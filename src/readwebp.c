/* -*- coding: utf-8; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Suika2
 * Copyright (C) 2001-2023, Keiichi Tabata. All rights reserved.
 */

#if !defined(NO_WEBP)

#include "suika.h"

#include <webp/decode.h>

/*
 * イメージをWebPファイルから読み込む
 */
struct image *create_image_from_file_webp(const char *dir, const char *file)
{
	struct image *img;
	struct rfile *rf;
	pixel_t *p;
	uint8_t *pixels, *raw_data;
	size_t file_size;
	int width, height;
	int x, y;

	/* ファイルを開く */
	rf = open_rfile(dir, file, false);
	if (rf == NULL)
		return NULL;

	/* ファイルのサイズを取得する */
	file_size = get_rfile_size(rf);

	/* ファイル全体を読み込むメモリを確保する */
	raw_data = malloc(file_size);
	if (raw_data == NULL) {
		log_memory();
		close_rfile(rf);
		return NULL;
	}

	/* ファイル全体を読み込む */
	if (read_rfile(rf, raw_data, file_size) < file_size) {
		log_image_file_error(dir, file);
		free(raw_data);
		close_rfile(rf);
		return NULL;
	}
	close_rfile(rf);

	/* 画像の幅と高さを取得する */
	if (!WebPGetInfo(raw_data, file_size, &width, &height)) {
		log_image_file_error(dir, file);
		free(raw_data);
		return false;
	}

	/* 画像を作成する */
	img = create_image(width, height);
	if (img == NULL) {
		log_memory();
		free(raw_data);
		return NULL;
	}

	/* デコードを行う */
	pixels = WebPDecodeRGBA(raw_data, file_size, &width, &height);
	if (pixels == NULL) {
		log_image_file_error(dir, file);
		free(raw_data);
		return false;
	}

	/* ピクセルのコピーを行う */
	p = img->pixels;
	for (y = 0; y < height; y++) {
		for (x = 0; x < width; x++) {
			*p++ = make_pixel(pixels[y * width * 4 + x * 4 + 3],
					  pixels[y * width * 4 + x * 4 + 0],
					  pixels[y * width * 4 + x * 4 + 1],
					  pixels[y * width * 4 + x * 4 + 2]);
		}
	}
	notify_image_update(img);

	/* メモリを解放する */
	WebPFree(pixels);
	free(raw_data);

	return img;
}

#endif /* !defined(NO_WEBP) */
