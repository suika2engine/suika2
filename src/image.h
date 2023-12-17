/* -*- coding: utf-8; tab-width: 8; indent-tabs-mode: t; -*- */

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
 *  2023-12-08 リファクタリング
 */

#ifndef SUIKA_IMAGE_H
#define SUIKA_IMAGE_H

#include "types.h"

/* RGBAカラー形式のピクセル値 */
typedef uint32_t pixel_t;

/*
 * image構造体
 */
struct image {
	/* 水平方向のピクセル数 */
	int width;

	/* 垂直方向のピクセル数 */
	int height;

	/* ピクセル列 */
	SIMD_ALIGNED_MEMBER(pixel_t *pixels);

	/* (HAL internal) テクスチャへのポインタ */
	void *texture;

	/* (HAL internal) テクスチャのアップロードが必要か */
	bool need_upload;
};

/*
 * Direct3D, Metal の場合は RGBA形式
 */
#if defined(SUIKA_TARGET_WIN32) || defined(SUIKA_TARGET_MACOS) || defined(SUIKA_TARGET_IOS)

/* ピクセル値を合成する */
static INLINE pixel_t make_pixel(uint32_t a, uint32_t r, uint32_t g, uint32_t b)
{
	return (((pixel_t)a) << 24) | (((pixel_t)r) << 16) | (((pixel_t)g) << 8) | ((pixel_t)b);
}

/* ピクセル値のアルファチャンネルを取得する */
static INLINE uint32_t get_pixel_a(pixel_t p)
{
	return (p >> 24) & 0xff;
}

/* ピクセル値の赤チャンネルを取得する */
static INLINE uint32_t get_pixel_r(pixel_t p)
{
	return (p >> 16) & 0xff;
}

/* ピクセル値の緑チャンネルを取得する */
static INLINE uint32_t get_pixel_g(pixel_t p)
{
	return (p >> 8) & 0xff;
}

/* ピクセル値の青チャンネルを取得する */
static INLINE uint32_t get_pixel_b(pixel_t p)
{
	return p & 0xff;
}

/*
 * OpenGLの場合はBGRA形式
 */
#else

/* ピクセル値を合成する */
static INLINE pixel_t make_pixel(uint32_t a, uint32_t r, uint32_t g, uint32_t b)
{
	return (((pixel_t)a) << 24) | (((pixel_t)b) << 16) | (((pixel_t)g) << 8) | ((pixel_t)r);
}

/* ピクセル値のアルファチャンネルを取得する */
static INLINE uint32_t get_pixel_a(pixel_t p)
{
	return (p >> 24) & 0xff;
}

/* ピクセル値の赤チャンネルを取得する */
static INLINE uint32_t get_pixel_r(pixel_t p)
{
	return p & 0xff;
}

/* ピクセル値の緑チャンネルを取得する */
static INLINE uint32_t get_pixel_g(pixel_t p)
{
	return (p >> 8) & 0xff;
}

/* ピクセル値の青チャンネルを取得する */
static INLINE uint32_t get_pixel_b(pixel_t p)
{
	return (p >> 16) & 0xff;
}

#endif

/* イメージを作成する */
struct image *create_image(int w, int h);

/* ファイル名を指定してイメージを作成する */
struct image *create_image_from_file(const char *dir, const char *file);

/* 文字列で色を指定してイメージを作成する */
struct image *create_image_from_color_string(int w, int h, const char *color);

/* イメージを削除する */
void destroy_image(struct image *img);

/* イメージのテクスチャが更新されたことを記録する */
void set_image_updated(struct image *img);

/* イメージを黒色でクリアする */
void clear_image_black(struct image *img);

/* イメージを白色でクリアする */
void clear_image_white(struct image *img);

/* イメージを色でクリアする */
void clear_image_color(struct image *img, pixel_t color);

/* イメージを描画する(コピー) */
void draw_image_copy(struct image *dst_image,
		     int dst_left,
		     int dst_top,
		     struct image *src_image,
		     int width,
		     int height,
		     int src_left,
		     int src_top);

/* イメージを描画する(アルファブレンド) */
void draw_image_normal(struct image *dst_image,
		       int dst_left,
		       int dst_top,
		       struct image *src_image,
		       int width,
		       int height,
		       int src_left,
		       int src_top,
		       int alpha);

/* イメージを描画する(50%暗くする) */
void draw_image_dim(struct image *dst_image,
		    int dst_left,
		    int dst_top,
		    struct image *src_image,
		    int width,
		    int height,
		    int src_left,
		    int src_top);

/* イメージをルール付きで描画する */
void draw_image_rule(struct image *dst_image,
		     struct image *src_image,
		     struct image *rule_image,
		     int threshold);

/* イメージをルール付き(メルト)で描画する */
void draw_image_melt(struct image *dst_image,
		     struct image *src_image,
		     struct image *rule_image,
		     int threshold);

/* イメージをスケールして描画する */
void draw_image_scale(struct image *dst_image,
		      int virtual_dst_width,
		      int virtual_dst_height,
		      int virtual_dst_left,
		      int virtual_dst_top,
		      struct image *src_image);

/*
 * Helpers for rendering HALs.
 */

/* 転送元領域のサイズを元に矩形のクリッピングを行う */
bool clip_by_source(int src_cx, int src_cy, int *cx, int *cy, int *dst_x,
		    int *dst_y, int *src_x, int *src_y);

/* 転送先領域のサイズを元に矩形のクリッピングを行う */
bool clip_by_dest(int dst_cx, int dst_cy, int *cx, int *cy, int *dst_x,
		  int *dst_y, int *src_x, int *src_y);

#endif /* SUIKA_IMAGE_H */
