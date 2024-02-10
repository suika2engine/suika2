/* -*- coding: utf-8; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Suika2
 * Copyright (C) 2001-2022, Keiichi Tabata. All rights reserved.
 */

#include "suika.h"

#include <jpeglib.h>

/*
 * イメージをJPEGファイルから読み込む
 */
struct image *create_image_from_file_jpeg(const char *dir, const char *file)
{
	struct jpeg_decompress_struct jpeg;
	struct jpeg_error_mgr jerr;
	struct rfile *rf;
	struct image *img;
	pixel_t *p;
	unsigned char *raw_data;
	unsigned char *line;
	size_t file_size;
	unsigned int width, height, x, y;
	int components;

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

	/* デコードを開始する */
	jpeg_create_decompress(&jpeg);
	jpeg_mem_src(&jpeg, raw_data, file_size);
	jpeg.err = jpeg_std_error(&jerr);
	jpeg_read_header(&jpeg, TRUE);
	jpeg_start_decompress(&jpeg);

	/* 画像サイズとチャネル数を取得する */
	width = jpeg.output_width;
	height = jpeg.output_height;
	components = jpeg.out_color_components;
	if (components != 3) {
		log_image_file_error(dir, file);
		free(raw_data);
		jpeg_destroy_decompress(&jpeg);
		return NULL;
	}

	/* デコード結果のピクセル列を1行格納するメモリを確保する */
	line = malloc(width * height * 3);
	if (line == NULL) {
		log_memory();
		free(raw_data);
		jpeg_destroy_decompress(&jpeg);
		return NULL;
	}

	/* 画像を作成する */
	img = create_image((int)width, (int)height);
	if (img == NULL) {
		log_memory();
		free(line);
		free(raw_data);
		jpeg_destroy_decompress(&jpeg);
		return NULL;
	}

	/* 行ごとにデコードする */
	p = img->pixels;
	for (y = 0; y < height; y++) {
		/* 1行デコードする */
		jpeg_read_scanlines(&jpeg, &line, 1);

		/* イメージにコピーする */
		if (!is_rgba_reverse_needed()) {
			for (x = 0; x < width; x++) {
				*p++ = make_pixel(255,
						  line[x * 3],
						  line[x * 3 + 1],
						  line[x * 3 + 2]);
			}
		} else {
			for (x = 0; x < width; x++) {
				*p++ = make_pixel(255,
						  line[x * 3 + 2],
						  line[x * 3 + 1],
						  line[x * 3]);
			}
		}
	}
	notify_image_update(img);

	/* 終了処理を行う */
	free(line);
	free(raw_data);
	jpeg_destroy_decompress(&jpeg);

	return img;
}
