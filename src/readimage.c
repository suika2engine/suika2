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
static bool is_png_ext(const char *str);
static bool is_jpg_ext(const char *str);
#if !defined(NO_WEBP)
static bool is_webp_ext(const char *str);
#endif

/*
 * イメージをファイルから読み込む
 */
struct image *create_image_from_file(const char *dir, const char *file)
{
	char fname[128];
	struct image *img;
	int y, x;

	/* JPEGファイルの場合 */
	if (is_jpg_ext(file)) {
		img = create_image_from_file_jpeg(dir, file);
		if (img == NULL)
			return NULL;
		return img;
	}
#if !defined(NO_WEBP)
	/* WebPファイルの場合 */
	else if (is_webp_ext(file)) {
		img = create_image_from_file_webp(dir, file);
		if (img == NULL)
			return NULL;
	}
#endif
	/* PNGファイルの場合 */
	else if (is_png_ext(file)) {
		img = create_image_from_file_png(dir, file);
		if (img == NULL)
			return NULL;
	} else {
		do {
			/* 自動拡張子付与(.png)でチェックする */
			snprintf(fname, sizeof(fname), "%s.png", file);
			if (check_file_exist(dir, fname)) {
				img = create_image_from_file_png(dir, fname);
				if (img == NULL)
					return NULL;
				break;
			}

			/* 自動拡張子付与(.PNG)でチェックする */
			snprintf(fname, sizeof(fname), "%s.PNG", file);
			if (check_file_exist(dir, fname)) {
				img = create_image_from_file_png(dir, fname);
				if (img == NULL)
					return NULL;
				break;
			}

			/* 自動拡張子付与(.jpg)でチェックする */
			snprintf(fname, sizeof(fname), "%s.jpg", file);
			if (check_file_exist(dir, fname)) {
				img = create_image_from_file_jpeg(dir, fname);
				if (img == NULL)
					return NULL;
				break;
			}

			/* 自動拡張子付与(.JPG)でチェックする */
			snprintf(fname, sizeof(fname), "%s.JPG", file);
			if (check_file_exist(dir, fname)) {
				img = create_image_from_file_jpeg(dir, fname);
				if (img == NULL)
					return NULL;
				break;
			}

#if !defined(NO_WEBP)
			/* 自動拡張子付与(.webp)でチェックする */
			snprintf(fname, sizeof(fname), "%s.webp", file);
			if (check_file_exist(dir, fname)) {
				img = create_image_from_file_webp(dir, fname);
				if (img == NULL)
					return NULL;
				break;
			}

			/* 自動拡張子付与(.WEBP)でチェックする */
			snprintf(fname, sizeof(fname), "%s.WEBP", file);
			if (check_file_exist(dir, fname)) {
				img = create_image_from_file_webp(dir, fname);
				if (img == NULL)
					return NULL;
				break;
			}
#endif

			/* その他の場合はPNGとして開いてみる */
			img = create_image_from_file_png(dir, file);
			if (img == NULL)
				return NULL;
		} while (0);
	}

	/* 完全に透明なピクセルのRGB値を0にする */
	for (y = 0; y < img->height; y++)
		for (x = 0; x < img->width; x++)
			if (get_pixel_a(img->pixels[y * img->width + x]) == 0)
				img->pixels[y * img->width + x] = make_pixel(0, 0, 0, 0);

	return img;
}

/* 拡張子がPNGであるかチェックする */
static bool is_png_ext(const char *str)
{
	size_t len1 = strlen(str);
	size_t len2 = 4;
	if (len1 >= len2) {
		if (strcmp(str + len1 - len2, ".png") == 0)
			return true;
		if (strcmp(str + len1 - len2, ".PNG") == 0)
			return true;
	}
	return false;
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
