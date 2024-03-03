/* -*- coding: utf-8; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Suika2
 * Copyright (C) 2001-2023, Keiichi Tabata. All rights reserved.
 */

#define PNG_DEBUG 3
#include <png.h>

#include "suika.h"

/*
 * ファイルスコープ変数
 */
static struct rfile *rf;
static png_structp png_ptr;
static png_infop info_ptr;
static png_bytep *rows;
static int width;
static int height;
static struct image *image;

/*
 * 前方参照
 */
static struct image *cleanup(void);
static bool read_png(const char *dir, const char *file);
static bool read_body(void);
static bool check_signature(void);
static bool read_header(void);
static void read_callback(png_structp png_ptr, png_bytep buf, png_size_t len);

/*
 * イメージをファイルから読み込む
 */
struct image *create_image_from_file_png(const char *dir, const char *file)
{
	/* PNGを読み込む */
	if (!read_png(dir, file))
		return NULL;

	/* イメージを返す */
	return cleanup();
}

/* ファイルスコープ変数のクリーンアップを行う */
static struct image *cleanup(void)
{
	struct image *result;

	result = NULL;
	if (rf != NULL) {
		close_rfile(rf);
		rf = NULL;
	}
	if (rows != NULL) {
		free(rows);
		rows = NULL;
	}
	if (png_ptr != NULL) {
		png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
		png_ptr = NULL;
		info_ptr = NULL;
	}
	if (image != NULL) {
		result = image;
		image = NULL;
	}
	return result;
}

/* イメージファイルを読み込む */
static bool read_png(const char *dir, const char *file)
{
	rf = open_rfile(dir, file, false);
	if (rf == NULL)
		return false;

	if (!check_signature()) {
		log_image_file_error(dir, file);
		return false;
	}

	if (!read_header()) {
		log_image_file_error(dir, file);
		return false;
	}

	image = create_image(width, height);
	if (image == NULL)
		return false;

	if (!read_body()) {
		log_image_file_error(dir, file);
		return false;
	}

	notify_image_update(image);

	return true;
}

/* シグネチャをチェックする */
static bool check_signature(void)
{
	png_byte buf[8];
	size_t len;

	len = read_rfile(rf, buf, 8);
	if (len == 0)
		return false;

	if (png_sig_cmp(buf, 0, len))
		return false;

	return true;
}

/* ヘッダを読み込む */
static bool read_header(void)
{
	png_byte color_type, bit_depth;

	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL,
					 NULL);
	if (png_ptr == NULL)
		return false;

	info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == NULL) {
		png_destroy_read_struct(&png_ptr, NULL, NULL);
		return false;
	}

	if (setjmp(png_jmpbuf(png_ptr))) {
		png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
		return false;
	}

	png_set_read_fn(png_ptr, rf, read_callback);
	png_set_sig_bytes(png_ptr, 8);
	png_read_info(png_ptr, info_ptr);

	/* サイズ、カラータイプ、ビット幅を取得する */
	width = (int)png_get_image_width(png_ptr, info_ptr);
	height = (int)png_get_image_height(png_ptr, info_ptr);
	color_type = png_get_color_type(png_ptr, info_ptr);
	bit_depth = png_get_bit_depth(png_ptr, info_ptr);

	/* パレットの場合はRGBに変換する */
	switch(color_type) {
	case PNG_COLOR_TYPE_GRAY:
		png_set_gray_to_rgb(png_ptr);
		png_set_add_alpha(png_ptr, 0xff, PNG_FILLER_AFTER);
		png_read_update_info(png_ptr, info_ptr);
		break;
	case PNG_COLOR_TYPE_PALETTE:
		if (!is_opengl_byte_order())
			png_set_bgr(png_ptr);

		png_set_palette_to_rgb(png_ptr);
		png_set_add_alpha(png_ptr, 0xff, PNG_FILLER_AFTER);
		png_read_update_info(png_ptr, info_ptr);
		break;
	case PNG_COLOR_TYPE_RGB:
		if (!is_opengl_byte_order())
			png_set_bgr(png_ptr);

		if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)) {
			png_set_tRNS_to_alpha(png_ptr);
		} else {
			png_set_add_alpha(png_ptr, 0xff, PNG_FILLER_AFTER);
			png_read_update_info(png_ptr, info_ptr);
		}
		break;
	case PNG_COLOR_TYPE_RGB_ALPHA:
		if (!is_opengl_byte_order())
			png_set_bgr(png_ptr);
		break;
	case PNG_COLOR_TYPE_GRAY_ALPHA:
		png_set_gray_to_rgb(png_ptr);
		png_set_add_alpha(png_ptr, 0xff, PNG_FILLER_AFTER);
		png_read_update_info(png_ptr, info_ptr);
		break;
	default:
		return false;
	}

	/* 16bit幅なら8bit幅に変換する */
	if (bit_depth == 16)
		png_set_strip_16(png_ptr);

	return true;
}

/* ファイル読み込みコールバック */
static void read_callback(png_structp png_ptr, png_bytep buf, png_size_t len)
{
	struct rfile *rf;

	rf = png_get_io_ptr(png_ptr);
	
	read_rfile(rf, buf, len);
}

/* イメージ本体を読み込む */
static bool read_body(void)
{
	int y;
	pixel_t *pixels;
	
	if (setjmp(png_jmpbuf(png_ptr))) {
		png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
		return false;
	}

	rows = malloc(sizeof(png_bytep) * (size_t)height);
	if (rows == NULL)
		return false;

	assert(png_get_rowbytes(png_ptr, info_ptr) == (size_t)(width*4));

#ifdef _MSC_VER
#pragma warning(disable:6386)
#endif
	pixels = image->pixels;
	for (y = 0; y < height; y++)
		rows[y] = (png_bytep)&pixels[width * y];

	png_read_image(png_ptr, rows);

	return true;
}
