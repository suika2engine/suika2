/* -*- coding: utf-8-with-signature; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Suika 2
 * Copyright (C) 2001-2021, TABATA Keiichi. All rights reserved.
 */

/*
 * [Changes]
 *  2001-10-03 作成 [KImage@VNStudio]
 *  2002-03-10 アルファブレンド対応 [VNImage@西瓜Studio]
 *  2004-01-05 MMX対応 [LXImage@Texture Alpha Maker]
 *  2006-09-05 Cに書き換え [VImage@V]
 *  2016-05-27 gcc5.3.1のベクトル化に対応, 浮動小数点 [image@Suika]
 *  2016-06-11 SSEバージョニングを実装
 *  2016-06-16 OSX対応
 *  2016-08-05 Android NDK対応
 *  2021-06-10 マスクつき描画対応
 */

#ifndef SUIKA_IMAGE_H
#define SUIKA_IMAGE_H

#include "types.h"

/* platform.h */
bool is_opengl_enabled(void);

/* ARGB/ABGRカラー形式のピクセル値 */
typedef uint32_t pixel_t;

/* イメージ構造体 */
struct image;

/* ブレンドタイプ */
enum blend_type {
	BLEND_NONE,			/* ソースをそのままコピー */
	BLEND_FAST,			/* 通常のブレンド */
	BLEND_NORMAL,			/* 通常のブレンド */
	BLEND_ADD,			/* 飽和加算ブレンド */
	BLEND_SUB,			/* 飽和減算ブレンド */
};

/* ピクセル値を合成する */
static INLINE pixel_t make_pixel_fast(uint32_t a, uint32_t c1, uint32_t c2,
				      uint32_t c3)
{
	return (((pixel_t)a) << 24) | (((pixel_t)c1) << 16) |
		(((pixel_t)c2) << 8) | ((pixel_t)c3);
}

/* ピクセル値のアルファチャンネルを取得する */
static INLINE uint32_t get_pixel_a(pixel_t p)
{
	return p >> 24;
}

/* ピクセル値のコンポーネント1(RかB)を取得する */
static INLINE uint32_t get_pixel_c1(pixel_t p)
{
	return (p >> 16) & 0xff;
}

/* ピクセル値のコンポーネント2(G)を取得する */
static INLINE uint32_t get_pixel_c2(pixel_t p)
{
	return (p >> 8) & 0xff;
}

/* ピクセル値の青チャンネル(BかR)を取得する */
static INLINE uint32_t get_pixel_c3(pixel_t p)
{
	return p & 0xff;
}

/*
 * WindowsとX11の場合はARGB形式(バイト順にBGRA)
 * OpenGLが有効な場合はBGRA形式(バイト順にRGBA)
 */
#if defined(WIN) || defined(LINUX) || defined(NETBSD) || defined(FREEBSD)

/* ピクセル値を合成する */
static INLINE pixel_t make_pixel_slow(uint32_t a, uint32_t r, uint32_t g,
				      uint32_t b)
{
	if (is_opengl_enabled()) {
		return (((pixel_t)a) << 24) | (((pixel_t)b) << 16) |
			(((pixel_t)g) << 8) | ((pixel_t)r);
	} else {
		return (((pixel_t)a) << 24) | (((pixel_t)r) << 16) |
			(((pixel_t)g) << 8) | ((pixel_t)b);
	}
}

/* ピクセル値の赤チャンネルを取得する */
static INLINE uint32_t get_pixel_r_slow(pixel_t p)
{
	if (is_opengl_enabled())
		return p & 0xff;
	else
		return (p >> 16) & 0xff;
}

/* ピクセル値の緑チャンネルを取得する */
static INLINE uint32_t get_pixel_g_slow(pixel_t p)
{
	return (p >> 8) & 0xff;
}

/* ピクセル値の青チャンネルを取得する */
static INLINE uint32_t get_pixel_b_slow(pixel_t p)
{
	if (is_opengl_enabled())
		return (p >> 16) & 0xff;
	else
		return p & 0xff;
}

/*
 * Macの場合は常にABGR形式(バイト順にRGBA)
 */
#else

/* ピクセル値を合成する */
static INLINE pixel_t make_pixel_slow(uint32_t a, uint32_t r, uint32_t g,
				      uint32_t b)
{
	return (((pixel_t)a) << 24) | (((pixel_t)b) << 16) |
		(((pixel_t)g) << 8) | ((pixel_t)r);
}

/* ピクセル値の赤チャンネルを取得する */
static INLINE uint32_t get_pixel_r_slow(pixel_t p)
{
	return p & 0xff;
}

/* ピクセル値の緑チャンネルを取得する */
static INLINE uint32_t get_pixel_g_slow(pixel_t p)
{
	return (p >> 8) & 0xff;
}

/* ピクセル値の青チャンネルを取得する */
static INLINE uint32_t get_pixel_b_slow(pixel_t p)
{
	return (p >> 16) & 0xff;
}

#endif

/* イメージを作成する */
struct image *create_image(int w, int h);

/* ピクセル列を指定してイメージを作成する */
struct image *create_image_with_pixels(int w, int h, pixel_t *ptr);

/* ファイル名を指定してイメージを作成する */
struct image *create_image_from_file(const char *dir, const char *file);

/* 文字列で色を指定してイメージを作成する */
struct image *create_image_from_color_string(int w, int h, const char *color);

/* イメージを削除する */
void destroy_image(struct image *img);

/* イメージをロックする */
bool lock_image(struct image *img);

/* イメージをアンロックする */
void unlock_image(struct image *img);

/* ピクセルへのポインタを取得する(for glyph.c) */
pixel_t *get_image_pixels(struct image *img);

/* イメージの幅を取得する */
int get_image_width(struct image *img);

/* イメージの高さを取得する */
int get_image_height(struct image *img);

/* テクスチャを取得する */
void *get_texture_object(struct image *img);

/* イメージに関連付けられたオブジェクトを取得する(for NDK, iOS) */
void *get_image_object(struct image *img);

/* イメージを黒色でクリアする */
void clear_image_black(struct image *img);

/* イメージの矩形を黒色でクリアする */
void clear_image_black_rect(struct image *img, int x, int y, int w, int h);

/* イメージを白色でクリアする */
void clear_image_white(struct image *img);

/* イメージの矩形を白色でクリアする */
void clear_image_white_rect(struct image *img, int x, int y, int w, int h);

/* イメージを色でクリアする */
void clear_image_color(struct image *img, pixel_t color);

/* イメージの矩形を色でクリアする */
void clear_image_color_rect(struct image *img, int x, int y, int w, int h,
			    pixel_t color);

/* イメージを描画する */
void draw_image(struct image * RESTRICT dst_image, int dst_left, int dst_top,
		struct image * RESTRICT src_image, int width, int height,
		int src_left, int src_top, int alpha, int bt);

/* イメージをルール付きで描画する */
void draw_image_rule(struct image * RESTRICT dst_image,
		     struct image * RESTRICT src_image,
		     struct image * RESTRICT rule_image,
		     int threshold);

/* 転送元領域のサイズを元に矩形のクリッピングを行う */
bool clip_by_source(int src_cx, int src_cy, int *cx, int *cy, int *dst_x,
		    int *dst_y, int *src_x, int *src_y);

/* 転送先領域のサイズを元に矩形のクリッピングを行う */
bool clip_by_dest(int dst_cx, int dst_cy, int *cx, int *cy, int *dst_x,
		  int *dst_y, int *src_x, int *src_y);

#endif /* SUIKA_IMAGE_H */
