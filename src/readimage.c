/* -*- coding: utf-8; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Suika2
 * Copyright (C) 2001-2023, Keiichi Tabata. All rights reserved.
 */

#include "suika.h"

struct image *create_image_from_file_png(const char *dir, const char *file);
struct image *create_image_from_file_jpeg(const char *dir, const char *file);
#if !defined(NO_WEBP)
struct image *create_image_from_file_webp(const char *dir, const char *file);
#endif

/*
 * 前方参照
 */
static bool is_jpg_ext(const char *str);
#if !defined(NO_WEBP)
static bool is_webp_ext(const char *str);
#endif

/*
 * イメージをファイルから読み込む
 */
struct image *create_image_from_file(const char *dir, const char *file)
{
	struct image *img;
	int y, x;

	/* JPEGファイルの場合 */
	if (is_jpg_ext(file)) {
		img = create_image_from_file_jpeg(dir, file);
		if (img == NULL)
			return false;
		return img;
	}

#if !defined(NO_WEBP)
	/* WebPファイルの場合 */
	if (is_webp_ext(file)) {
		img = create_image_from_file_webp(dir, file);
		if (img == NULL)
			return false;
	}
#endif

	/* その他の場合はPNGファイルとみなす */
	img = create_image_from_file_png(dir, file);
	if (img == NULL)
		return false;

	/* 完全に透明なピクセルのRGB値を0にする */
	for (y = 0; y < img->height; y++)
		for (x = 0; x < img->width; x++)
			if (get_pixel_a(img->pixels[y * img->width + x]) == 0)
				img->pixels[y * img->width + x] = make_pixel(0, 0, 0, 0);

	return img;
}

/* 拡張子がJPGであるかチェックする */
static bool is_jpg_ext(const char *str)
{
	size_t len1 = strlen(str);
	size_t len2 = 4;
	if (len1 >= len2) {
		if (strcmp(str + len1 - len2, ".jpg") == 0)
			return true;
		if (strcmp(str + len1 - len2, ".JPG") == 0)
			return true;
	}
	return false;
}

#if !defined(NO_WEBP)
/* 拡張子がWebPであるかチェックする */
static bool is_webp_ext(const char *str)
{
	size_t len1 = strlen(str);
	size_t len2 = 5;
	if (len1 >= len2) {
		if (strcmp(str + len1 - len2, ".webp") == 0)
			return true;
		if (strcmp(str + len1 - len2, ".WEBP") == 0)
			return true;
	}
	return false;
}
#endif
