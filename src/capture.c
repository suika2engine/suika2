/* -*- coding: utf-8; indent-tabs-mode: t; tab-width: 8; c-basic-offset: 8; -*- */

/*
 * Suika2
 * Copyright (C) 2001-2023, Keiich Tabata. All rights reserved.
 */

/*
 * [Changes]
 *  2023-07-17 作成
 *  2022-08-26 Windows/Mac/Linux共通化
 */

#include "suika.h"
#include "capture.h"
#include "png.h"

#ifdef OSX
#define GL_SILENCE_DEPRECATION
#include <OpenGL/gl3.h>
#else
#include <GL/gl.h>
#endif

/* ディレクトリとファイルの名前 */
#define CAP_DIR		"record"
#define CSV_FILE	"main.csv"

/* 定期的にフレームを出力する時間(3fps) */
#define FRAME_MILLI	333

/* CSVファイル */
static FILE *csv_fp;

/* 現在時刻 */
uint64_t cap_cur_time;

/* 開始時刻 */
static uint64_t start_time;

/* 前回のPNG出力時刻 */
static uint64_t last_png_time;

/* このフレームで入力が変化したか */
static bool is_input_changed;

/* 前回のマウス座標 */
static int prev_mouse_x = -1, prev_mouse_y = -1;

/* フレームバッファコピー用 */
static char *frame_buf;

/* PNG書き出し用の行ポインタ */
static png_bytep *row_pointers;

/*
 * キャプチャモジュールを初期化する
 */
bool init_capture(void)
{
	int y;

	/* recordフォルダを作成しなおす */
	reconstruct_dir(CAP_DIR);

	/* CSVファイルを開く */
#ifdef MAC
	csv_fp = open_csv_file(CAP_DIR, CSV_FILE, "wb");
#else
	csv_fp = fopen(CAP_DIR "\\" CSV_FILE, "wb");
#endif
	if (csv_fp == NULL) {
		log_error("Failed to create record file.");
		return false;
	}
	fprintf(csv_fp, "%s", CSV_HEADER);

	/* フレームバッファのコピー用メモリを確保する */
	frame_buf = malloc((size_t)(conf_window_width * conf_window_height *
				    3));
	if (frame_buf == NULL) {
		log_memory();
		return false;
	}

	/* libpngに渡す行ポインタを作成する */
	row_pointers = malloc(sizeof(png_bytep) * (size_t)conf_window_height);
	if (row_pointers == NULL) {
		log_memory();
		return false;
	}
	for (y = 0; y < conf_window_height; y++) {
		row_pointers[y] = 
			(png_bytep)&frame_buf[conf_window_width * 3 *
					      (conf_window_height - 1 - y)];
	}

	/* 開始時刻を取得する */
	start_time = get_tick_count64();

	return true;
}

/*
 * キャプチャモジュールを終了する
 */
void cleanup_capture(void)
{
	if (row_pointers != NULL) {
		free(row_pointers);
		row_pointers = NULL;
	}

	if (frame_buf != NULL) {
		free(frame_buf);
		frame_buf = NULL;
	}

	if (csv_fp != NULL) {
		fclose(csv_fp);
		csv_fp = NULL;
	}
}

/*
 * 入力をキャプチャする
 */
bool capture_input(void)
{
	/* 時刻を更新する */
	cap_cur_time = get_tick_count64() - start_time;

	/* 入力に変化があったか調べる */
	is_input_changed = false;
	if (is_left_button_pressed ||
	    is_right_button_pressed ||
	    is_left_clicked ||
	    is_right_clicked ||
	    is_return_pressed ||
	    is_space_pressed ||
	    is_escape_pressed ||
	    is_up_pressed ||
	    is_down_pressed ||
	    is_page_up_pressed ||
	    is_page_down_pressed ||
	    is_control_pressed) {
		is_input_changed = true;
	}
	if (mouse_pos_x != prev_mouse_x || mouse_pos_y != prev_mouse_y) {
		is_input_changed = true;
		prev_mouse_x = mouse_pos_x;
		prev_mouse_y = mouse_pos_y;
	}
	if (cap_cur_time - last_png_time >= FRAME_MILLI)
		is_input_changed = true;

	/* フレームの時刻と入力の状態を出力する */
	fprintf(csv_fp,
#if (defined(WIN) && !defined(__WIN64)) || defined(OSX)
		"%lld,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
#else
		"%ld,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
#endif
		cap_cur_time,
		is_input_changed,
		mouse_pos_x,
		mouse_pos_y,
		is_left_button_pressed,
		is_right_button_pressed,
		is_left_clicked,
		is_right_clicked,
		is_return_pressed,
		is_space_pressed,
		is_escape_pressed,
		is_up_pressed,
		is_down_pressed,
		is_page_up_pressed,
		is_page_down_pressed,
		is_control_pressed);
	fflush(csv_fp);

	return true;
}

/*
 * 出力をキャプチャする
 */
bool capture_output(void)
{
	char fname[256];
	png_structp png;
	png_infop info;
	FILE *png_fp;

	/* 入力に変化のなかったフレームの場合 */
	if (!is_input_changed)
		return true;

	/* フレームバッファの内容を取得する */
#if defined(LINUX)
	glReadBuffer(GL_BACK);
#else
	glReadBuffer(GL_FRONT);
#endif
	glReadPixels(0, 0, conf_window_width, conf_window_height, GL_RGB,
		     GL_UNSIGNED_BYTE, frame_buf);

	/* ファイル名を決める */
	snprintf(fname, sizeof(fname),
#if (defined(WIN) && !defined(__WIN64)) || defined(OSX)
		 "%s\\%lld.png",
#else
		 "%s\\%ld.png",
#endif
		 CAP_DIR, cap_cur_time);

	/* PNGファイルをオープンする */
	png_fp = fopen(fname, "wb");
	if (png_fp == NULL) {
		log_file_open(fname);
		return false;
	}

	/* PNGを書き出す */
	png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (png == NULL) {
		log_api_error("png_create_write_struct");
		fclose(png_fp);
		return false;
	}
	info = png_create_info_struct(png);
	if (info == NULL) {
		log_api_error("png_create_info_struct");
		png_destroy_write_struct(&png, NULL);
		return false;
	}
	if (setjmp(png_jmpbuf(png))) {
		log_error("Failed to write png file.");
		png_destroy_write_struct(&png, &info);
		return false;
	}
	png_init_io(png, png_fp);
	png_set_IHDR(png, info,
		     (png_uint_32)conf_window_width,
		     (png_uint_32)conf_window_height,
		     8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
		     PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
	png_write_info(png, info);
	png_write_image(png, row_pointers);
	png_write_end(png, NULL);
	png_destroy_write_struct(&png, &info);

	/* PNGファイルをクローズする */
	fclose(png_fp);

	/* 前回の画像出力時刻を記録する */
	last_png_time = cap_cur_time;

	return true;
}
