/* -*- coding: utf-8; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Suika2
 * Copyright (C) 2001-2021, Keiichi Tabata. All rights reserved.
 */

/*
 * [Changes]
 *  2001-10-03 Created [KImage@VNStudio] (masked BitBlt() with HBURUSH)
 *  2002-03-10 Supported the alpha blending using a 256x256 multiply table [VNImage@Suika Studio]
 *  2004-01-05 Supported MMX intrinsics [LXImage@Texture Alpha Maker]
 *  2006-09-05 Ported to C from C++ [VImage@V]
 *  2016-05-27 Supported the gcc 5 tree vectorizer, changed to use float [image@Suika]
 *  2016-06-11 Added the dynamic SSE dispatching
 *  2016-06-16 Supported Mac OS X
 *  2021-03-22 Supported Visual Studio 2019
 *  2021-06-05 Added create_image_from_color_string()
 *  2021-06-10 Added a masked drawing such as BitBlt() with HBRUSH (later removed and 
 *  2021-08-04 Supported Direct3D 9
 *  2022-06-28 Added 1-bit universal transition (and removed the masked drawing)
 *  2022-11-15 Added 8-bit universal transition
 *  2023-12-08 Totally refactored for Apple Metal support
 */

#include "suika.h"

#if defined(SUIKA_TARGET_WIN32)
#include <malloc.h>	/* _aligned_mallo() */
#endif

/* 512-bit alignment */
#define ALIGN_BYTES	(64)

/*
 * 前方参照
 */

static bool check_draw_image(struct image *dst_image, int *dst_left, int *dst_top,
			     struct image *src_image, int *width, int *height,
			     int *src_left, int *src_top, int alpha);

/*
 * 初期化
 */

/*
 * イメージを作成する
 */
struct image *create_image(int w, int h)
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
#if defined(SUIKA_TARGET_WIN32)
	pixels = _aligned_malloc((size_t)w * (size_t)h * sizeof(pixel_t), 64);
	if (pixels == NULL) {
		log_memory();
		free(img);
		return NULL;
	}
#else
	if (posix_memalign((void **)&pixels, 64, (size_t)w * (size_t)h * sizeof(pixel_t)) != 0) {
		log_memory();
		free(img);
		return NULL;
	}
#endif

	/* 構造体を初期化する */
	img->width = w;
	img->height = h;
	img->pixels = pixels;
	img->texture = NULL;
	img->need_upload = false;

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
	img = create_image(w, h);
	if (img == NULL)
		return NULL;

	/* 色指定文字列を読み込む */
	rgb = 0;
	sscanf(color, "%x", &rgb);
	r = (rgb >> 16) & 0xff;
	g = (rgb >> 8) & 0xff;
	b = rgb & 0xff;
	cl = make_pixel(0xff, r, g, b);

	/* イメージを塗り潰す */
	clear_image_color(img, cl);

	return img;
}

/*
 * イメージを作成する
 */
struct image *create_image_with_pixels(int w, int h, pixel_t *pixels)
{
	struct image *img;

	assert(w > 0 && h > 0);

	/* イメージ構造体のメモリを確保する */
	img = malloc(sizeof(struct image));
	if (img == NULL) {
		log_memory();
		return NULL;
	}

	/* 構造体を初期化する */
	img->width = w;
	img->height = h;
	img->pixels = pixels;
	img->texture = NULL;
	img->need_upload = false;

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

	/* テクスチャを削除する */
	notify_image_free(img);

	/* ピクセル列のメモリを解放する */
#if defined(SUIKA_TARGET_WIN32)
	_aligned_free(img->pixels);
#else
	free(img->pixels);
#endif
	img->pixels = NULL;

	/* イメージ構造体のメモリを解放する */
	free(img);
}

/*
 * クリア
 */

/*
 * イメージを黒色でクリアする
 */
void clear_image_black(struct image *img)
{
	clear_image_color_rect(img, 0, 0, img->width, img->height, make_pixel(0xff, 0, 0, 0));
}

/*
 * イメージを白色でクリアする
 */
void clear_image_white(struct image *img)
{
	clear_image_color_rect(img, 0, 0, img->width, img->height, make_pixel(0xff, 0xff, 0xff, 0xff));
}

/*
 * イメージを色でクリアする
 */
void clear_image_color(struct image *img, pixel_t color)
{
	clear_image_color_rect(img, 0, 0, img->width, img->height, color);
}

/*
 * イメージの矩形を色でクリアする
 */
void clear_image_color_rect(struct image *img, int x, int y, int w, int h, pixel_t color)
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

	/* ピクセル列の矩形をクリアする */
	pixels = img->pixels;
	for (i = y; i < y + h; i++)
		for (j = x; j < x + w; j++)
			pixels[img->width * i + j] = color;

	/* Request a texture update. */
	notify_image_update(img);
}

/*
 * イメージのアルファチャンネルを255でクリアする
 */
void fill_image_alpha(struct image *img)
{
	pixel_t *p;
	int y, x;

	assert(img != NULL);

	p = img->pixels;
	for (y = 0; y < img->height; y++) {
		for (x = 0; x < img->width; x++) {
			*p = (*p) | 0xff000000;
			p++;
		}
	}
}

/*
 * 描画
 */

/*
 * イメージを描画する
 */
void draw_image_copy(struct image *dst_image,
		int dst_left,
		int dst_top,
		struct image *src_image,
		int width,
		int height,
		int src_left,
		int src_top)
{
	pixel_t * RESTRICT src_ptr;
	pixel_t * RESTRICT dst_ptr;
	int x, y, sw, dw;

	if (!check_draw_image(dst_image, &dst_left, &dst_top, src_image, &width, &height, &src_left, &src_top, 255))
		return;

	sw = src_image->width;
	dw = dst_image->width;
	src_ptr = src_image->pixels + sw * src_top + src_left;
	dst_ptr = dst_image->pixels + dw * dst_top + dst_left;

	for(y = 0; y < height; y++) {
		for(x = 0; x < width; x++)
			*(dst_ptr + x) = *(src_ptr + x);
		src_ptr += sw;
		dst_ptr += dw;
	}

	notify_image_update(dst_image);
}

/*
 * イメージを描画する
 */
void draw_image_normal(struct image *dst_image,
		       int dst_left,
		       int dst_top,
		       struct image *src_image,
		       int width,
		       int height,
		       int src_left,
		       int src_top,
		       int alpha)
{
	pixel_t * RESTRICT src_ptr;
	pixel_t * RESTRICT dst_ptr;
	float a, src_r, src_g, src_b, src_a, dst_r, dst_g, dst_b, dst_a;
	uint32_t src_pix, dst_pix;
	int src_line_inc, dst_line_inc, x, y, sw, dw;

	if (!check_draw_image(dst_image, &dst_left, &dst_top, src_image, &width, &height, &src_left, &src_top, alpha))
		return;

	sw = src_image->width;
	dw = dst_image->width;
	src_ptr = src_image->pixels + sw * src_top + src_left;
	dst_ptr = dst_image->pixels + dw * dst_top + dst_left;
	src_line_inc = sw - width;
	dst_line_inc = dw - width;
	a = (float)alpha / 255.0f;

	for(y = 0; y < height; y++) {
		for(x = 0; x < width; x++) {
			/* 転送元と転送先のピクセルを取得する */
			src_pix	= *src_ptr++;
			dst_pix	= *dst_ptr;

			/* アルファ値を計算する */
			src_a = a * ((float)get_pixel_a(src_pix) / 255.0f);
			dst_a = 1.0f - src_a;

			/* 転送元ピクセルにアルファ値を乗算する */
			src_r = src_a * (float)get_pixel_r(src_pix);
			src_g = src_a * (float)get_pixel_g(src_pix);
			src_b = src_a * (float)get_pixel_b(src_pix);

			/* 転送先ピクセルにアルファ値を乗算する */
			dst_r = dst_a * (float)get_pixel_r(dst_pix);
			dst_g = dst_a * (float)get_pixel_g(dst_pix);
			dst_b = dst_a * (float)get_pixel_b(dst_pix);

			/* 転送先に格納する */
			*dst_ptr++ = make_pixel(0xff,
						(uint32_t)(src_r + dst_r),
						(uint32_t)(src_g + dst_g),
						(uint32_t)(src_b + dst_b));
		}
		src_ptr += src_line_inc;
		dst_ptr += dst_line_inc;
	}

	notify_image_update(dst_image);
}

/*
 * イメージを描画する
 */
void draw_image_add(struct image *dst_image,
		    int dst_left,
		    int dst_top,
		    struct image *src_image,
		    int width,
		    int height,
		    int src_left,
		    int src_top,
		    int alpha)
{
	pixel_t * RESTRICT src_ptr;
	pixel_t * RESTRICT dst_ptr;
	float a, src_a;
	uint32_t src_pix, src_r, src_g, src_b;
	uint32_t dst_pix, dst_r, dst_g, dst_b;
	uint32_t add_r, add_g, add_b;
	int src_line_inc, dst_line_inc, x, y, sw, dw;

	if (!check_draw_image(dst_image, &dst_left, &dst_top, src_image, &width, &height, &src_left, &src_top, alpha))
		return;

	sw = src_image->width;
	dw = dst_image->width;
	src_ptr = src_image->pixels + sw * src_top + src_left;
	dst_ptr = dst_image->pixels + dw * dst_top + dst_left;
	src_line_inc = sw - width;
	dst_line_inc = dw - width;
	a = (float)alpha / 255.0f;

	for(y = 0; y < height; y++) {
		for(x = 0; x < width; x++) {
			/* 転送元と転送先のピクセルを取得する */
			src_pix	= *src_ptr++;
			dst_pix	= *dst_ptr;

			/* アルファ値を計算する */
			src_a = a * ((float)get_pixel_a(src_pix) / 255.0f);

			/* 転送元ピクセルにアルファ値を乗算する */
			src_r = (uint32_t)(src_a * ((float)get_pixel_r(src_pix) / 255.0f) * 255.0f);
			src_g = (uint32_t)(src_a * ((float)get_pixel_g(src_pix) / 255.0f) * 255.0f);
			src_b = (uint32_t)(src_a * ((float)get_pixel_b(src_pix) / 255.0f) * 255.0f);

			/* 転送先ピクセルを取得する */
			dst_r = get_pixel_r(dst_pix);
			dst_g = get_pixel_g(dst_pix);
			dst_b = get_pixel_b(dst_pix);

			/* 飽和加算する */
			add_r = src_r + dst_r;
			if (add_r > 255)
				add_r = 255;
			add_g = src_g + dst_g;
			if (add_g > 255)
				add_g = 255;
			add_b = src_b + dst_b;
			if (add_b > 255)
				add_b = 255;

			/* 転送先に格納する */
			*dst_ptr++ = make_pixel(0xff,
						add_r,
						add_g,
						add_b);
		}
		src_ptr += src_line_inc;
		dst_ptr += dst_line_inc;
	}

	notify_image_update(dst_image);
}

/*
 * イメージを描画する
 */
void draw_image_dim(struct image *dst_image,
		    int dst_left,
		    int dst_top,
		    struct image *src_image,
		    int width,
		    int height,
		    int src_left,
		    int src_top,
		    int alpha)
{
	pixel_t * RESTRICT src_ptr, * RESTRICT dst_ptr;
	float a, src_r, src_g, src_b, src_a, dst_r, dst_g, dst_b, dst_a;
	uint32_t src_pix, dst_pix;
	int src_line_inc, dst_line_inc, x, y, sw, dw;

	if (!check_draw_image(dst_image, &dst_left, &dst_top, src_image, &width, &height, &src_left, &src_top, 255))
		return;

	sw = src_image->width;
	dw = dst_image->width;
	src_ptr = src_image->pixels + sw * src_top + src_left;
	dst_ptr = dst_image->pixels + dw * dst_top + dst_left;
	src_line_inc = sw - width;
	dst_line_inc = dw - width;
	a = (float)alpha / 255.0f;

	for(y = 0; y < height; y++) {
		for(x = 0; x < width; x++) {
			/* 転送元と転送先のピクセルを取得する */
			src_pix	= *src_ptr++;
			dst_pix	= *dst_ptr;

			/* アルファ値を計算する */
			src_a = a * ((float)get_pixel_a(src_pix) / 255.0f);
			dst_a = 1.0f - src_a;

			/* 転送元ピクセルにアルファ値とdim係数を乗算する */
			src_r = src_a * 0.5f * (float)get_pixel_r(src_pix);
			src_g = src_a * 0.5f * (float)get_pixel_g(src_pix);
			src_b = src_a * 0.5f * (float)get_pixel_b(src_pix);

			/* 転送先ピクセルにアルファ値を乗算する */
			dst_r = dst_a * (float)get_pixel_r(dst_pix);
			dst_g = dst_a * (float)get_pixel_g(dst_pix);
			dst_b = dst_a * (float)get_pixel_b(dst_pix);

			/* 転送先に格納する */
			*dst_ptr++ = make_pixel(0xff,
						(uint32_t)(src_r + dst_r),
						(uint32_t)(src_g + dst_g),
						(uint32_t)(src_b + dst_b));
		}
		src_ptr += src_line_inc;
		dst_ptr += dst_line_inc;
	}

	notify_image_update(dst_image);
}

/*
 * イメージをルール付きで描画する
 */
void draw_image_rule(struct image *dst_image,
		     struct image *src_image,
		     struct image *rule_image,
		     int threshold)
{
	pixel_t * RESTRICT src_ptr;
	pixel_t * RESTRICT dst_ptr;
	pixel_t * RESTRICT rule_ptr;
	int x, y, dw, sw, rw, w, dh, sh, rh, h;

	assert(dst_image != NULL);
	assert(src_image != NULL);
	assert(rule_image != NULL);
	assert(rule_image->width == conf_window_width);
	assert(rule_image->height == conf_window_height);

	/* 幅を取得する */
	dw = dst_image->width;
	sw = src_image->width;
	rw = rule_image->width;
	w = dw;
	if (sw < w)
		w = sw;
	if (rw < w)
		w = rw;
	
	/* 高さを取得する */
	dh = dst_image->height;
	sh = src_image->height;
	rh = rule_image->height;
	h = dh;
	if (sh < h)
		h = sh;
	if (rh < h)
		h = rh;

	/* 描画する */
	dst_ptr = dst_image->pixels;
	src_ptr = src_image->pixels;
	rule_ptr = rule_image->pixels;
	for (y = 0; y < h; y++) {
		for (x = 0; x < w; x++)
			if (get_pixel_b(*(rule_ptr + x)) <= (unsigned char)threshold)
				*(dst_ptr + x) = *(src_ptr + x);
		dst_ptr += dw;
		src_ptr += sw;
		rule_ptr += rw;
	}

	notify_image_update(dst_image);
}

/*
 * イメージをルール付き(メルト)で描画する
 */
void draw_image_melt(struct image *dst_image,
		     struct image *src_image,
		     struct image *rule_image,
		     int threshold)
{
	pixel_t * RESTRICT src_ptr;
	pixel_t * RESTRICT dst_ptr;
	pixel_t * RESTRICT rule_ptr;
	pixel_t src_pix, dst_pix, rule_pix;
	float src_a, src_r, src_g, src_b, dst_a, dst_r, dst_g, dst_b, rule_a;
	int x, y, dw, sw, rw, w, dh, sh, rh, h;

	assert(dst_image != NULL);
	assert(src_image != NULL);
	assert(rule_image != NULL);
	assert(rule_image->width == conf_window_width);
	assert(rule_image->height == conf_window_height);

	/* 幅を取得する */
	dw = dst_image->width;
	sw = src_image->width;
	rw = rule_image->width;
	w = dw;
	if (sw < w)
		w = sw;
	if (rw < w)
		w = rw;
	
	/* 高さを取得する */
	dh = dst_image->height;
	sh = src_image->height;
	rh = rule_image->height;
	h = dh;
	if (sh < h)
		h = sh;
	if (rh < h)
		h = rh;

	/* 描画する */
	dst_ptr = dst_image->pixels;
	src_ptr = src_image->pixels;
	rule_ptr = rule_image->pixels;
	for (y = 0; y < h; y++) {
		for (x = 0; x < w; x++) {
			/* 描画元のピクセルを取得する */
			src_pix = src_ptr[x];

			/* 描画先のピクセルを取得する */
			dst_pix = dst_ptr[x];

			/* ルール画像のピクセルを取得する */
			rule_pix = rule_ptr[x];

			/* アルファ値を計算する */
			rule_a = (float)get_pixel_b(rule_pix) / 255.0f;
			src_a = 2.0f * ((float)threshold / 255.0f) - rule_a;
			src_a = src_a < 0 ? 0 : src_a;
			src_a = src_a > 1.0f ? 1.0f : src_a;
			dst_a = 1.0f - src_a;

			/* 描画元ピクセルにアルファ値を乗算する */
			src_r = src_a * (float)get_pixel_r(src_pix);
			src_g = src_a * (float)get_pixel_g(src_pix);
			src_b = src_a * (float)get_pixel_b(src_pix);

			/* 描画先ピクセルにアルファ値を乗算する */
			dst_r = dst_a * (float)get_pixel_r(dst_pix);
			dst_g = dst_a * (float)get_pixel_g(dst_pix);
			dst_b = dst_a * (float)get_pixel_b(dst_pix);

			/* 描画先に格納する */
			dst_ptr[x] = make_pixel(0xff,
						(uint32_t)(src_r + dst_r),
						(uint32_t)(src_g + dst_g),
						(uint32_t)(src_b + dst_b));
		}
		dst_ptr += dw;
		src_ptr += sw;
		rule_ptr += rw;
	}

	notify_image_update(dst_image);
}

/*
 * イメージをスケールして描画する (nearest-neighbor)
 */
void draw_image_scale(struct image *dst_image,
		      int virtual_dst_width,
		      int virtual_dst_height,
		      int virtual_dst_left,
		      int virtual_dst_top,
		      struct image *src_image)
{
	pixel_t * RESTRICT dst_ptr;
	pixel_t * RESTRICT src_ptr;
	float scale_x, scale_y;
	pixel_t src_pix, dst_pix;
	float src_a, src_r, src_g, src_b, dst_a, dst_r, dst_g, dst_b;
	int real_dst_width, real_dst_height;
	int real_src_width, real_src_height;
	int real_draw_left, real_draw_top, real_draw_width, real_draw_height;
	int virtual_x, virtual_y;
	int i, j;

	assert(dst_image != NULL);
	assert(src_image != NULL);

	/* 実際の描画先のサイズを取得する */
	real_dst_width = dst_image->width;
	real_dst_height = dst_image->height;

	/* 縮尺を計算する */
	scale_x = (float)real_dst_width / (float)virtual_dst_width;
	scale_y = (float)real_dst_height / (float)virtual_dst_height;

	/* 実際の描画元のサイズを取得する */
	real_src_width = src_image->width;
	real_src_height = src_image->height;

	/* 実際の描画先の位置とサイズを計算する */
	real_draw_left = (int)((float)virtual_dst_left * scale_x);
	real_draw_top = (int)((float)virtual_dst_top * scale_y);
	real_draw_width = (int)((float)real_src_width * scale_x);
	real_draw_height = (int)((float)real_src_height * scale_y);

	/* ピクセルへのポインタを取得する */
	dst_ptr = dst_image->pixels;
	src_ptr = src_image->pixels;

	/* 描画する */
	for (i = real_draw_top; i < real_draw_top + real_draw_height; i++) {
		/* 描画先のY座標でクリッピングする */
		if (i < 0)
			continue;
		if (i >= real_dst_height)
			continue;

		/* 描画元のY座標を求め、クリッピングする */
		virtual_y = (int)((float)i / scale_y) - virtual_dst_top;
		if (virtual_y < 0)
			continue;
		if (virtual_y >= real_src_height)
			continue;

		for (j = real_draw_left; j < real_draw_left + real_draw_width;
		     j++) {
			/* 描画先のX座標でクリッピングする */
			if (j < 0)
				continue;
			if (j >= real_dst_width)
				continue;

			/* 描画元のX座標を求め、クリッピングする */
			virtual_x = (int)((float)j / scale_x) - virtual_dst_left;
			if (virtual_x < 0)
				continue;
			if (virtual_x >= real_src_width)
				continue;

			/* 描画元のピクセルを取得する */
			src_pix = src_ptr[real_src_width * virtual_y + virtual_x];

			/* 描画先のピクセルを取得する */
			dst_pix = dst_ptr[real_dst_width * i + j];

			/* アルファ値を計算する */
			src_a = (float)get_pixel_a(src_pix) / 255.0f;
			dst_a = 1.0f - src_a;

			/* 描画元ピクセルにアルファ値を乗算する */
			src_r = src_a * (float)get_pixel_r(src_pix);
			src_g = src_a * (float)get_pixel_g(src_pix);
			src_b = src_a * (float)get_pixel_b(src_pix);

			/* 描画先ピクセルにアルファ値を乗算する */
			dst_r = dst_a * (float)get_pixel_r(dst_pix);
			dst_g = dst_a * (float)get_pixel_g(dst_pix);
			dst_b = dst_a * (float)get_pixel_b(dst_pix);

			/* 描画先に格納する */
			dst_ptr[real_dst_width * i + j] = make_pixel(0xff,
								     (uint32_t)(src_r + dst_r),
								     (uint32_t)(src_g + dst_g),
								     (uint32_t)(src_b + dst_b));
		}
	}

	notify_image_update(dst_image);
}

/*
 * Clipping
 */

/* Check for draw_image_*() parameters. */
static bool check_draw_image(struct image *dst_image,
			     int *dst_left,
			     int *dst_top,
			     struct image *src_image,
			     int *width,
			     int *height,
			     int *src_left,
			     int *src_top,
			     int alpha)
{
	/* 引数をチェックする */
	assert(dst_image != NULL);
	assert(dst_image != src_image);
	assert(dst_image->width > 0 && dst_image->height > 0);
	assert(dst_image->pixels != NULL);
	assert(src_image != NULL);
	assert(src_image->width > 0 && src_image->height > 0);
	assert(src_image->pixels != NULL);

	/* 描画の必要があるか判定する */
	if(alpha == 0 || *width == 0 || *height == 0)
		return false;	/* 描画の必要がない*/
	if(!clip_by_source(src_image->width, src_image->height, width, height, dst_left, dst_top, src_left, src_top))
		return false;	/* 描画範囲外 */
	if(!clip_by_dest(dst_image->width, dst_image->height, width, height, dst_left, dst_top, src_left, src_top))
		return false;	/* 描画範囲外 */

	/* 描画する */
	return true;
}

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

	if (*cx <= 0 || *cy <= 0)
		return false;

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

	if (*cx <= 0 || *cy <= 0)
		return false;

	/* 成功 */
	return true;
}
