/* -*- coding: utf-8-with-signature; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Suika 2
 * Copyright (C) 2001-2021, TABATA Keiichi. All rights reserved.
 */

/*
 * [Changes]
 *  2001-10-03 作成 [KImage@VNStudio]
 *  2002-03-10 アルファブレンド対応, 乗算テーブル [VNImage@西瓜Studio]
 *  2004-01-05 MMX対応 [LXImage@Texture Alpha Maker]
 *  2006-09-05 Cに書き換え [VImage@V]
 *  2016-05-27 gcc5.3.1のベクトル化に対応, 浮動小数点 [image@Suika]
 *  2016-06-11 SSEバージョニングを実装
 *  2016-06-16 OSX対応
 *  2021-03-22 Visual Studio 2019対応
 *  2021-06-05 色指定のイメージ作成に対応
 *  2021-06-10 マスクつき描画に対応
 *  2021-08-04 Direct3Dに対応
 */

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "suika.h"

#ifdef SSE_VERSIONING
#include "x86.h"
#endif

#if defined(SSE_VERSIONING) && defined(WIN)
#include <malloc.h>
#endif

/*
 * image構造体
 */
struct image {
	int width;			/* 水平方向のピクセル数 */
	int height;			/* 垂直方向のピクセル数 */
#ifndef SSE_VERSIONING
	pixel_t * RESTRICT pixels;	/* ピクセル列 */
#else
	ALIGN_DECL(SSE_ALIGN, pixel_t * RESTRICT pixels);
#endif
	bool need_free;			/* pixelsを解放する必要があるか */
	pixel_t *locked_pixels;		/* ロック済みのピクセル列 */
	void *texture;			/* テクスチャへのポインタ */
};

/*
 * 前方参照
 */

static struct image *create_image_helper(int w, int h);
static void draw_blend_none(struct image * RESTRICT dst_image, int dst_left,
			    int dst_top, struct image * RESTRICT src_image,
			    int width, int height, int src_left, int src_top);
static void draw_blend_fast(struct image * RESTRICT dst_image, int dst_left,
			    int dst_top, struct image * RESTRICT src_image,
			    int width, int height, int src_left, int src_top,
			    int alpha);
static void draw_blend_normal(struct image * RESTRICT dst_image, int dst_left,
			      int dst_top, struct image * RESTRICT src_image,
			      int width, int height, int src_left, int src_top,
			      int alpha);
static void draw_blend_add(struct image * RESTRICT dst_image, int dst_left,
			   int dst_top, struct image * RESTRICT src_image,
			   int width, int height, int src_left, int src_top,
			   int alpha);
static void draw_blend_sub(struct image * RESTRICT dst_image, int dst_left,
			   int dst_top, struct image * RESTRICT src_image,
			   int width, int height, int src_left, int src_top,
			   int alpha);

/*
 * 初期化
 */

/*
 * イメージを作成する
 */
struct image *create_image(int w, int h)
{
	struct image *img;

	/* イメージを作成する */
	img = create_image_helper(w, h);
	if (img == NULL)
		return NULL;

	/* 成功 */
	return img;
}

/* イメージを作成する */
static struct image *create_image_helper(int w, int h)
{
	struct image *img;
	pixel_t *pixels;

	assert(w > 0 && h > 0);

	/* イメージ構造体のメモリを確保する */
	img = malloc(sizeof(struct image));
	if (img == NULL) {
		log_memory();
		return NULL;
	}

	/* ピクセル列のメモリを確保する */
#if !defined(SSE_VERSIONING)
	pixels = malloc((size_t)w * (size_t)h * sizeof(pixel_t));
	if(pixels == NULL) {
		log_memory();
		free(img);
		return NULL;
	}
#elif defined(WIN)
	pixels = _aligned_malloc((size_t)w * (size_t)h * sizeof(pixel_t),
				 SSE_ALIGN);
	if (pixels == NULL) {
		log_memory();
		free(img);
		return NULL;
	}
#else
	if (posix_memalign((void **)&pixels, SSE_ALIGN, (size_t)w * (size_t)h *
			   sizeof(pixel_t)) != 0) {
		log_memory();
		free(img);
		return NULL;
	}
#endif

	/* 構造体を初期化する */
	img->width = w;
	img->height = h;
	img->pixels = pixels;
	img->need_free = true;
	img->locked_pixels = NULL;
	img->texture = NULL;

	return img;
}

/*
 * バッファを指定してイメージを作成する
 */
struct image *create_image_with_pixels(int w, int h, pixel_t *buf)
{
	struct image *img;

	assert(w > 0 && h > 0);
	assert(buf != NULL);

	/* イメージ構造体のメモリを確保する */
	img = malloc(sizeof(struct image));
	if (img == NULL) {
		log_memory();
		return NULL;
	}

	/* 構造体を初期化する */
	img->width = w;
	img->height = h;
	img->pixels = buf;
	img->need_free = false;
	img->locked_pixels = NULL;

	/* 成功 */
	return img;
}

/*
 * 文字列で色を指定してイメージを作成する
 */
struct image *create_image_from_color_string(int w, int h, const char *color)
{
	struct image *img;
	uint32_t r, g, b;
	pixel_t cl;
	int rgb;

	/* イメージを作成する */
	img = create_image_helper(w, h);
	if (img == NULL)
		return NULL;

	/* 色指定文字列を読み込む */
	sscanf(color, "%x", &rgb);
	r = (rgb >> 16) & 0xff;
	g = (rgb >> 8) & 0xff;
	b = rgb & 0xff;
	cl = make_pixel_slow(0xff, r, g, b);

	/* イメージを塗り潰す */
	lock_image(img);
	clear_image_color(img, cl);
	unlock_image(img);

	return img;
}

/*
 * イメージを削除する
 */
void destroy_image(struct image *img)
{
	assert(img != NULL);
	assert(img->width > 0 && img->height > 0);
	assert(img->pixels != NULL);
	assert(img->locked_pixels == NULL);

	/* テクスチャを削除する */
	destroy_texture(img->texture);

	/* ピクセル列のメモリを解放する */
	if (img->need_free) {
#if defined(SSE_VERSIONING) && defined(WIN)
		_aligned_free(img->pixels);
#else
		free(img->pixels);
#endif		
	}
	img->pixels = NULL;

	/* イメージ構造体のメモリを解放する */
	free(img);
}

/*
 * ロック操作
 */

/*
 * イメージをロックする
 */
bool lock_image(struct image *img)
{
	if (!lock_texture(img->width, img->height, img->pixels,
			  &img->locked_pixels, &img->texture))
		return false;

	return true;
}

/*
 * イメージをアンロックする
 */
void unlock_image(struct image *img)
{
	unlock_texture(img->width, img->height, img->pixels,
		       &img->locked_pixels, &img->texture);
}

/*
 * 情報の取得
 */

/*
 * ピクセルへのポインタを取得する
 */
pixel_t *get_image_pixels(struct image *img)
{
	if (img->locked_pixels != NULL)
		return img->locked_pixels;

	return img->pixels;
}

/*
 * イメージの幅を取得する
 */
int get_image_width(struct image *img)
{
	return img->width;
}

/*
 * イメージの高さを取得する
 */
int get_image_height(struct image *img)
{
	assert(img != NULL);

	return img->height;
}

/*
 * テクスチャを取得する
 */
void *get_texture_object(struct image *img)
{
	return img->texture;
}

/*
 * クリア
 */

/*
 * イメージを黒色でクリアする
 */
void clear_image_black(struct image *img)
{
	assert(img->locked_pixels != NULL);

	clear_image_color_rect(img, 0, 0, img->width, img->height,
			       make_pixel_fast(0xff, 0, 0, 0));
}

/*
 * イメージの矩形を黒色でクリアする
 */
void clear_image_black_rect(struct image *img, int x, int y, int w, int h)
{
	assert(img->locked_pixels != NULL);

	clear_image_color_rect(img, x, y, w, h,
			       make_pixel_fast(0xff, 0, 0, 0));
}

/*
 * イメージを白色でクリアする
 */
void clear_image_white(struct image *img)
{
	assert(img->locked_pixels != NULL);

	clear_image_color_rect(img, 0, 0, img->width, img->height,
			       make_pixel_fast(0xff, 0xff, 0xff, 0xff));
}

/*
 * イメージの矩形を白色でクリアする
 */
void clear_image_white_rect(struct image *img, int x, int y, int w, int h)
{
	assert(img->locked_pixels != NULL);

	clear_image_color_rect(img, x, y, w, h,
			       make_pixel_fast(0xff, 0xff, 0xff, 0xff));
}

/*
 * イメージを色でクリアする
 */
void clear_image_color(struct image *img, pixel_t color)
{
	assert(img->locked_pixels != NULL);

	clear_image_color_rect(img, 0, 0, img->width, img->height, color);
}

/*
 * イメージの矩形を色でクリアする
 */
void clear_image_color_rect(struct image *img, int x, int y, int w, int h,
			    pixel_t color)
{
	pixel_t *pixels;
	int i, j, sx, sy;

	assert(img != NULL);
	assert(img->width > 0 && img->height > 0);
	assert(img->pixels != NULL);

	/* 描画の必要があるか判定する */
	if(w == 0 || h == 0)
		return;	/* 描画の必要がない*/
	sx = sy = 0;
	if(!clip_by_dest(img->width, img->height, &w, &h, &x, &y, &sx, &sy))
		return;	/* 描画範囲外 */

	assert(x >= 0 && x < img->width);
	assert(w >= 0 && x + w <= img->width);
	assert(y >= 0 && y < img->height);
	assert(h >= 0 && y + h <= img->height);
	assert(img->locked_pixels != NULL);

	pixels = img->locked_pixels;

	/* ピクセル列の矩形をクリアする */
	for (i = y; i < y + h; i++)
		for (j = x; j < x + w; j++)
			pixels[img->width * i + j] = color;
}

/*
 * 描画
 */

/*
 * イメージを描画する
 */
void draw_image(struct image * RESTRICT dst_image, int dst_left, int dst_top,
		struct image * RESTRICT src_image, int width, int height,
		int src_left, int src_top, int alpha, int bt)
{
	/* 引数をチェックする */
	assert(dst_image != NULL);
	assert(dst_image != src_image);
	assert(dst_image->width > 0 && dst_image->height > 0);
	assert(dst_image->pixels != NULL);
	assert(src_image != NULL);
	assert(src_image->width > 0 && src_image->height > 0);
	assert(src_image->pixels != NULL);
	assert(width >= 0 && height >= 0);
	assert(bt == BLEND_NONE || bt == BLEND_FAST || bt == BLEND_NORMAL ||
	       bt == BLEND_ADD || bt == BLEND_SUB);
	assert(dst_image->locked_pixels != NULL);

	/* 描画の必要があるか判定する */
	if(alpha == 0 || width == 0 || height == 0)
		return;	/* 描画の必要がない*/
	if(!clip_by_source(src_image->width, src_image->height, &width,
			   &height, &dst_left, &dst_top, &src_left, &src_top))
		return;	/* 描画範囲外 */
	if(!clip_by_dest(dst_image->width, dst_image->height, &width, &height,
			 &dst_left, &dst_top, &src_left, &src_top))
		return;	/* 描画範囲外 */

	/* 描画を行う */
	switch(bt) {
	case BLEND_NONE:
		draw_blend_none(dst_image, dst_left, dst_top, src_image, width,
				height, src_left, src_top);
		break;
	case BLEND_FAST:
		draw_blend_fast(dst_image, dst_left, dst_top, src_image, width,
				height, src_left, src_top, alpha);
		break;
	case BLEND_NORMAL:
		draw_blend_normal(dst_image, dst_left, dst_top, src_image,
				  width, height, src_left, src_top, alpha);
		break;
	case BLEND_ADD:
		draw_blend_add(dst_image, dst_left, dst_top, src_image, width,
			       height, src_left, src_top, alpha);
		break;
	case BLEND_SUB:
		draw_blend_sub(dst_image, dst_left, dst_top, src_image, width,
			       height, src_left, src_top, alpha);
		break;
	default:
		assert(0);
		break;
	}
}

/*
 * クリッピング
 */

/*
 * 転送元領域のサイズを元に転送矩形のクリッピングを行う
 *  - 転送元領域の有効な座標範囲を(0,0)-(src_cx-1,src_cy-1)とし、
 *    転送矩形のクリッピングを行う
 *  - 転送矩形は幅と高さのほかに、転送元領域と転送先領域それぞれにおける
 *    左上XY座標を持ち、これらすべての値がクリッピングにより補正される
 *  - 転送矩形が転送元領域の有効な座標範囲から完全に外れている場合、偽を返す。
 *    それ以外の場合、真を返す。
 */
bool clip_by_source(
	int src_cx,	/* 転送元領域の幅 */
	int src_cy,	/* 転送元領域の高さ */
	int *cx,	/* 転送矩形の幅 */
	int *cy,	/* 転送矩形の高さ */
	int *dst_x,	/* 転送先領域における転送矩形の左上X座標 */
	int *dst_y,	/* 転送先領域における転送矩形の左上Y座標 */
	int *src_x,	/* 転送元領域における転送矩形の左上X座標 */
	int *src_y)	/* 転送元領域における転送矩形の左上Y座標 */
{
	/*
	 * 転送矩形が転送元領域の有効な座標範囲から
	 * 完全に外れている場合を検出する
	 */
	if(*src_x < 0 && -*src_x >= *cx)
		return false;	/* 左方向に完全にはみ出している */
	if(*src_y < 0 && -*src_y >= *cy)
		return false;	/* 上方向に完全にはみ出している */
	if(*src_x > 0 && *src_x >= src_cx)
		return false;	/* 右方向に完全にはみ出している */
	if(*src_y > 0 && *src_y >= src_cy)
		return false;	/* 右方向に完全にはみ出している */

	/*
	 * 左方向のはみ出しをカットする
	 * (転送元領域での左上X座標が負の場合)
	 */
	if(*src_x < 0) {
		*cx += *src_x;
		*dst_x -= *src_x;
		*src_x = 0;
	}

	/*
	 * 上方向のはみ出しをカットする
	 * (転送元領域での左上Y座標が負の場合)
	 */
	if(*src_y < 0) {
		*cy += *src_y;
		*dst_y -= *src_y;
		*src_y = 0;
	}

	/*
	 * 右方向のはみ出しをカットする
	 * (転送元領域での右端X座標が幅を超える場合)
	 */
	if(*src_x + *cx > src_cx)
		*cx = src_cx - *src_x;

	/*
	 * 下方向のはみ出しをカットする
	 * (転送元領域での下端Y座標が高さを超える場合)
	 */
	if(*src_y + *cy > src_cy)
		*cy = src_cy - *src_y;

	assert(*cx > 0 && *cy > 0);

	/* 成功 */
	return true;
}

/*
 * 転送先領域のサイズを元に矩形のクリッピングを行う
 *  - 転送先領域の有効な座標範囲を(0,0)-(src_cx-1,src_cy-1)とし、
 *    転送矩形のクリッピングを行う
 *  - 転送矩形は幅と高さのほかに、転送元領域と転送先領域それぞれにおける
 *    左上XY座標を持ち、これらすべての値がクリッピングにより補正される
 *  - 転送矩形が転送先領域の有効な座標範囲から完全に外れている場合、偽を返す。
 *    それ以外の場合、真を返す
 */
bool clip_by_dest(
	int dst_cx,	/* 転送先領域の幅 */
	int dst_cy,	/* 転送先領域の高さ */
	int *cx,	/* [IN/OUT] 転送矩形の幅 */
	int *cy,	/* [IN/OUT] 転送矩形の高さ */
	int *dst_x,	/* [IN/OUT] 転送先領域における転送矩形の左上X座標 */
	int *dst_y,	/* [IN/OUT] 転送先領域における転送矩形の左上Y座標 */
	int *src_x,	/* [IN/OUT] 転送元領域における転送矩形の左上X座標 */
	int *src_y)	/* [IN/OUT] 転送元領域における転送矩形の左上Y座標 */
{
	/*
	 * 転送矩形が転送先領域の有効な座標範囲から
	 * 完全に外れている場合を検出する
	 */
	if(*dst_x < 0 && -*dst_x >= *cx)
		return false;	/* 左方向に完全にはみ出している */
	if(*dst_y < 0 && -*dst_y >= *cy)
		return false;	/* 上方向に完全にはみ出している */
	if(*dst_x > 0 && *dst_x >= dst_cx)
		return false;	/* 右方向に完全にはみ出している */
	if(*dst_y > 0 && *dst_y >= dst_cy)
		return false;	/* 右方向に完全にはみ出している */

	/*
	 * 左方向のはみ出しをカットする
	 * (転送先領域での左端X座標が負の場合)
	 */
	if(*dst_x < 0) {
		*cx += *dst_x;
		*src_x -= *dst_x;
		*dst_x = 0;
	}

	/*
	 * 上方向のはみ出しをカットする
	 * (転送先領域での上端Y座標が負の場合)
	 */
	if(*dst_y < 0) {
		*cy += *dst_y;
		*src_y -= *dst_y;
		*dst_y = 0;
	}

	/*
	 * 右方向のはみ出しをカットする
	 * (転送先領域での右端X座標が幅を超える場合)
	 */
	if(*dst_x + *cx > dst_cx)
		*cx = dst_cx - *dst_x;

	/*
	 * 下方向のはみ出しをカットする
	 * (転送先領域での下端Y座標が高さを超える場合)
	 */
	if(*dst_y + *cy > dst_cy)
		*cy = dst_cy - *dst_y;

	assert(*cx > 0 && *cy > 0);

	/* 成功 */
	return true;
}

/*
 * アルファブレンドの描画
 *  - ベクトル化が見込めるため、同一のソース(drawimage.h)をインクルードして
 *    異なるコンパイルオプションでコンパイルするために次のファイルがある
 *  -- novec.c ... 最悪のケースでSSEが使えなかった場合(MMX Pentiumまで)
 *  -- sse.c ... Pentium III
 *  -- sse2.c ... Pentium 4
 *  -- sse3.c ... Pentium 4 (Prescott)
 *  -- sse41.c ... Core 2 (Penryn)
 *  -- sse42.c ... Core i (Nehalem)
 *  -- avx.c ... Core i (Sandy Bridge)
 *  -- avx2.c ... Core i (Haswell)
 *  - MMXの最適化はgcc/clangにないし、需要もないので、ベクトル化なしとした
 *  - Mac向けではIntelでSSE3, Apple SiliconでNEONが固定で使われる
 */

/*
 * SSEバージョニングを行わない場合
 */
#ifndef SSE_VERSIONING

/* 描画関数を定義する */
#define DRAW_BLEND_NONE			draw_blend_none
#define DRAW_BLEND_FAST			draw_blend_fast
#define DRAW_BLEND_NORMAL		draw_blend_normal
#define DRAW_BLEND_ADD			draw_blend_add
#define DRAW_BLEND_SUB			draw_blend_sub
#include "drawimage.h"

/*
 * SSEバージョニングを行う場合
 */
#else

/* AVX-512版の描画関数を宣言する */
#define PROTOTYPE_ONLY
#define DRAW_BLEND_NONE			draw_blend_none_avx512
#define DRAW_BLEND_FAST			draw_blend_fast_avx512
#define DRAW_BLEND_NORMAL		draw_blend_normal_avx512
#define DRAW_BLEND_ADD			draw_blend_add_avx512
#define DRAW_BLEND_SUB			draw_blend_sub_avx512
#include "drawimage.h"

/* AVX2版の描画関数を宣言する */
#define PROTOTYPE_ONLY
#define DRAW_BLEND_NONE			draw_blend_none_avx2
#define DRAW_BLEND_FAST			draw_blend_fast_avx2
#define DRAW_BLEND_NORMAL		draw_blend_normal_avx2
#define DRAW_BLEND_ADD			draw_blend_add_avx2
#define DRAW_BLEND_SUB			draw_blend_sub_avx2
#include "drawimage.h"

/* AVX版の描画関数を宣言する */
#define PROTOTYPE_ONLY
#define DRAW_BLEND_NONE			draw_blend_none_avx
#define DRAW_BLEND_FAST			draw_blend_fast_avx
#define DRAW_BLEND_NORMAL		draw_blend_normal_avx
#define DRAW_BLEND_ADD			draw_blend_add_avx
#define DRAW_BLEND_SUB			draw_blend_sub_avx
#include "drawimage.h"

#if !defined(_MSC_VER)

/* SSE4.2版の描画関数を宣言する */
#define PROTOTYPE_ONLY
#define DRAW_BLEND_NONE			draw_blend_none_sse42
#define DRAW_BLEND_FAST			draw_blend_fast_sse42
#define DRAW_BLEND_NORMAL		draw_blend_normal_sse42
#define DRAW_BLEND_ADD			draw_blend_add_sse42
#define DRAW_BLEND_SUB			draw_blend_sub_sse42
#include "drawimage.h"

/* SSE4.1版の描画関数を宣言する */
#define PROTOTYPE_ONLY
#define DRAW_BLEND_NONE			draw_blend_none_sse41
#define DRAW_BLEND_FAST			draw_blend_fast_sse41
#define DRAW_BLEND_NORMAL		draw_blend_normal_sse41
#define DRAW_BLEND_ADD			draw_blend_add_sse41
#define DRAW_BLEND_SUB			draw_blend_sub_sse41
#include "drawimage.h"

/* SSE3版の描画関数を宣言する */
#define PROTOTYPE_ONLY
#define DRAW_BLEND_NONE			draw_blend_none_sse3
#define DRAW_BLEND_FAST			draw_blend_fast_sse3
#define DRAW_BLEND_NORMAL		draw_blend_normal_sse3
#define DRAW_BLEND_ADD			draw_blend_add_sse3
#define DRAW_BLEND_SUB			draw_blend_sub_sse3
#include "drawimage.h"

#endif /* !defined(_MSC_VER) */

/* SSE2版の描画関数を宣言する */
#define PROTOTYPE_ONLY
#define DRAW_BLEND_NONE			draw_blend_none_sse2
#define DRAW_BLEND_FAST			draw_blend_fast_sse2
#define DRAW_BLEND_NORMAL		draw_blend_normal_sse2
#define DRAW_BLEND_ADD			draw_blend_add_sse2
#define DRAW_BLEND_SUB			draw_blend_sub_sse2
#include "drawimage.h"

/* SSE版の描画関数を宣言する */
#define PROTOTYPE_ONLY
#define DRAW_BLEND_NONE			draw_blend_none_sse
#define DRAW_BLEND_FAST			draw_blend_fast_sse
#define DRAW_BLEND_NORMAL		draw_blend_normal_sse
#define DRAW_BLEND_ADD			draw_blend_add_sse
#define DRAW_BLEND_SUB			draw_blend_sub_sse
#include "drawimage.h"

/* 非ベクトル版の描画関数を宣言する */
#define PROTOTYPE_ONLY
#define DRAW_BLEND_NONE			draw_blend_none_novec
#define DRAW_BLEND_FAST			draw_blend_fast_novec
#define DRAW_BLEND_NORMAL		draw_blend_normal_novec
#define DRAW_BLEND_ADD			draw_blend_add_novec
#define DRAW_BLEND_SUB			draw_blend_sub_novec
#include "drawimage.h"

/*
 * 描画関数のディスパッチ
 */

static void draw_blend_none(struct image *dst_image, int dst_left, int dst_top,
			    struct image *src_image, int width, int height,
			    int src_left, int src_top)
{
	if (has_avx512) {
		draw_blend_none_avx512(dst_image, dst_left, dst_top, src_image,
				       width, height, src_left, src_top);
	} else if (has_avx2) {
		draw_blend_none_avx2(dst_image, dst_left, dst_top, src_image,
				     width, height, src_left, src_top);
	} else if (has_avx) {
		draw_blend_none_avx(dst_image, dst_left, dst_top, src_image,
				    width, height, src_left, src_top);
#if !defined(_MSC_VER)
	} else if (has_sse42) {
		draw_blend_none_sse42(dst_image, dst_left, dst_top, src_image,
				      width, height, src_left, src_top);
	} else if (has_sse41) {
		draw_blend_none_sse41(dst_image, dst_left, dst_top, src_image,
				      width, height, src_left, src_top);
	} else if (has_sse3) {
		draw_blend_none_sse3(dst_image, dst_left, dst_top, src_image,
				     width, height, src_left, src_top);
#endif
	} else if (has_sse2) {
		draw_blend_none_sse2(dst_image, dst_left, dst_top, src_image,
				     width, height, src_left, src_top);
	} else if (has_sse) {
		draw_blend_none_sse(dst_image, dst_left, dst_top, src_image,
				    width, height, src_left, src_top);
	} else {
		draw_blend_none_novec(dst_image, dst_left, dst_top, src_image,
				      width, height, src_left, src_top);
	}
}

static void draw_blend_fast(struct image *dst_image, int dst_left, int dst_top,
			    struct image *src_image, int width, int height,
			    int src_left, int src_top, int alpha)
{
	if (has_avx512) {
		draw_blend_fast_avx512(dst_image, dst_left, dst_top, src_image,
				       width, height, src_left, src_top,
				       alpha);
	} else if (has_avx2) {
		draw_blend_fast_avx2(dst_image, dst_left, dst_top, src_image,
				     width, height, src_left, src_top,
				     alpha);
	} else if (has_avx) {
		draw_blend_fast_avx(dst_image, dst_left, dst_top, src_image,
				    width, height, src_left, src_top, alpha);
#if !defined(_MSC_VER)
	} else if (has_sse42) {
		draw_blend_fast_sse42(dst_image, dst_left, dst_top, src_image,
				      width, height, src_left, src_top, alpha);
	} else if (has_sse41) {
		draw_blend_fast_sse41(dst_image, dst_left, dst_top, src_image,
				      width, height, src_left, src_top, alpha);
	} else if (has_sse3) {
		draw_blend_fast_sse3(dst_image, dst_left, dst_top, src_image,
				     width, height, src_left, src_top, alpha);
#endif
	} else if (has_sse2) {
		draw_blend_fast_sse2(dst_image, dst_left, dst_top, src_image,
				     width, height, src_left, src_top, alpha);
	} else if (has_sse) {
		draw_blend_fast_sse(dst_image, dst_left, dst_top, src_image,
				    width, height, src_left, src_top, alpha);
	} else {
		draw_blend_fast_novec(dst_image, dst_left, dst_top, src_image,
				      width, height, src_left, src_top, alpha);
	}
}

static void draw_blend_normal(struct image *dst_image, int dst_left,
			      int dst_top, struct image *src_image, int width,
			      int height, int src_left, int src_top, int alpha)
{
	if (has_avx512) {
		draw_blend_normal_avx512(dst_image, dst_left, dst_top,
					 src_image, width, height, src_left,
					 src_top, alpha);
	} else if (has_avx2) {
		draw_blend_normal_avx2(dst_image, dst_left, dst_top, src_image,
				       width, height, src_left, src_top,
				       alpha);
	} else if (has_avx) {
		draw_blend_normal_avx(dst_image, dst_left, dst_top, src_image,
				      width, height, src_left, src_top,
				      alpha);
#if !defined(_MSC_VER)
	} else if (has_sse42) {
		draw_blend_normal_sse42(dst_image, dst_left, dst_top,
					src_image, width, height, src_left,
					src_top, alpha);
	} else if (has_sse41) {
		draw_blend_normal_sse41(dst_image, dst_left, dst_top,
					src_image, width, height, src_left,
					src_top, alpha);
	} else if (has_sse3) {
		draw_blend_normal_sse3(dst_image, dst_left, dst_top, src_image,
				       width, height, src_left, src_top,
				       alpha);
#endif
	} else if (has_sse2) {
		draw_blend_normal_sse2(dst_image, dst_left, dst_top, src_image,
				       width, height, src_left, src_top,
				       alpha);
	} else if (has_sse) {
		draw_blend_normal_sse(dst_image, dst_left, dst_top, src_image,
				      width, height, src_left, src_top,
				      alpha);
	} else {
		draw_blend_normal_novec(dst_image, dst_left, dst_top,
					src_image, width, height, src_left,
					src_top, alpha);
	}
}

static void draw_blend_add(struct image *dst_image, int dst_left, int dst_top,
			   struct image *src_image, int width, int height,
			   int src_left, int src_top, int alpha)
{
	if (has_avx512) {
		draw_blend_add_avx512(dst_image, dst_left, dst_top, src_image,
				      width, height, src_left, src_top, alpha);
	} else if (has_avx2) {
		draw_blend_add_avx2(dst_image, dst_left, dst_top, src_image,
				    width, height, src_left, src_top, alpha);
	} else if (has_avx) {
		draw_blend_add_avx(dst_image, dst_left, dst_top, src_image,
				   width, height, src_left, src_top, alpha);
#if !defined(_MSC_VER)
	} else if (has_sse42) {
		draw_blend_add_sse42(dst_image, dst_left, dst_top, src_image,
				     width, height, src_left, src_top, alpha);
	} else if (has_sse41) {
		draw_blend_add_sse41(dst_image, dst_left, dst_top, src_image,
				     width, height, src_left, src_top, alpha);
	} else if (has_sse3) {
		draw_blend_add_sse3(dst_image, dst_left, dst_top, src_image,
				    width, height, src_left, src_top, alpha);
#endif
	} else if (has_sse2) {
		draw_blend_add_sse2(dst_image, dst_left, dst_top, src_image,
				    width, height, src_left, src_top, alpha);
	} else if (has_sse) {
		draw_blend_add_sse(dst_image, dst_left, dst_top, src_image,
				   width, height, src_left, src_top, alpha);
	} else {
		draw_blend_add_novec(dst_image, dst_left, dst_top, src_image,
				     width, height, src_left, src_top, alpha);
	}
}

static void draw_blend_sub(struct image *dst_image, int dst_left, int dst_top,
			   struct image *src_image, int width, int height,
			   int src_left, int src_top, int alpha)
{
	if (has_avx512) {
		draw_blend_sub_avx512(dst_image, dst_left, dst_top, src_image,
				      width, height, src_left, src_top, alpha);
	} else if (has_avx2) {
		draw_blend_sub_avx2(dst_image, dst_left, dst_top, src_image,
				    width, height, src_left, src_top, alpha);
	} else if (has_avx) {
		draw_blend_sub_avx(dst_image, dst_left, dst_top, src_image,
				   width, height, src_left, src_top, alpha);
#if !defined(_MSC_VER)
	} else if (has_sse42) {
		draw_blend_sub_sse42(dst_image, dst_left, dst_top, src_image,
				     width, height, src_left, src_top, alpha);
	} else if (has_sse41) {
		draw_blend_sub_sse41(dst_image, dst_left, dst_top, src_image,
				     width, height, src_left, src_top, alpha);
	} else if (has_sse3) {
		draw_blend_sub_sse3(dst_image, dst_left, dst_top, src_image,
				    width, height, src_left, src_top, alpha);
#endif
	} else if (has_sse2) {
		draw_blend_sub_sse2(dst_image, dst_left, dst_top, src_image,
				    width, height, src_left, src_top, alpha);
	} else if (has_sse) {
		draw_blend_sub_sse(dst_image, dst_left, dst_top, src_image,
				    width, height, src_left, src_top, alpha);
	} else {
		draw_blend_sub_novec(dst_image, dst_left, dst_top, src_image,
				     width, height, src_left, src_top, alpha);
	}
}

#endif	/* SSE_VERSIONING */

/*
 * ルール付き描画
 *  - ベクトル化が見込めないのでここで定義する
 */

/*
 * イメージをルール付きで描画する
 */
void draw_image_rule(struct image * RESTRICT dst_image,
		     struct image * RESTRICT src_image,
		     struct image * RESTRICT rule_image,
		     int threshold)
{
	pixel_t * RESTRICT src_ptr, * RESTRICT dst_ptr, * RESTRICT rule_ptr;
	int x, y, dw, sw, rw, w, dh, sh, rh, h;

	assert(dst_image->locked_pixels != NULL);

	/* 幅を取得する */
	dw = get_image_width(dst_image);
	sw = get_image_width(src_image);
	rw = get_image_width(rule_image);
	w = dw;
	if (sw < w)
		w = sw;
	if (rw < w)
		w = rw;
	
	/* 高さを取得する */
	dh = get_image_height(dst_image);
	sh = get_image_height(src_image);
	rh = get_image_height(rule_image);
	h = dh;
	if (sh < h)
		h = sh;
	if (rh < h)
		h = rh;

	/* 描画する */
	dst_ptr = get_image_pixels(dst_image);
	src_ptr = get_image_pixels(src_image);
	rule_ptr = get_image_pixels(rule_image);
	for (y = 0; y < h; y++) {
		for (x = 0; x < w; x++) {
			if (get_pixel_c1(*(rule_ptr + x)) <=
			    (unsigned char)threshold)
				*(dst_ptr + x) = *(src_ptr + x);
		}
		dst_ptr += dw;
		src_ptr += sw;
		rule_ptr += rw;
	}
}
