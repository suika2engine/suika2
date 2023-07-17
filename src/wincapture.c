/* -*- coding: utf-8; indent-tabs-mode: t; tab-width: 8; c-basic-offset: 8; -*- */

/*
 * Suika 2
 * Copyright (C) 2001-2023, TABATA Keiichi. All rights reserved.
 */

/*
 * [Changes]
 *  2023-07-17 作成
 */

#include <GL/gl.h>
#include <windows.h>	/* File/directory manipultion */
#include <io.h>		/* _access() */
#include "png.h"

#include "suika.h"

/* ディレクトリとファイルの名前 */
#define CAP_DIR		"record"
#define CAP_DIR_L	L"record"
#define CSV_FILE	"main.csv"

/* CSVのヘッダ */
static char csv_header[] =
	"time,X,Y,left,right,return,space,escape,"
	"up,down,pageup,pagedown,control\n";

/* CSVファイル */
static FILE *csv_fp;

/* 開始時刻 */
static uint64_t start_time;

/* 現在時刻 */
static uint64_t cur_time;

/* このフレームで入力が変化したか */
static bool is_input_changed;

/* 前回のマウス座標 */
static int prev_mouse_x = -1, prev_mouse_y = -1;

/* フレームバッファコピー用 */
static char *frame_buf;

/* PNG書き出し用の行ポインタ */
png_bytep *row_pointers;

/* 前方参照 */
static bool delete_directory(LPCWSTR pszDirName);

/*
 * キャプチャモジュールを初期化する
 */
bool init_capture(void)
{
	int y;

	if (!delete_directory(CAP_DIR_L)) {
		log_error("Failed to remove record directory.");
		return false;
	}

	if (!CreateDirectory(CAP_DIR_L, NULL)) {
		if (_access(CAP_DIR, 0) != 0) {
			log_error("Failed to create record directory.");
			return false;
		}
	}

	csv_fp = fopen(CAP_DIR "\\" CSV_FILE, "wb");
	if (csv_fp == NULL) {
		log_error("Failed to create record file.");
		return false;
	}
	fprintf(csv_fp, "%s", csv_header);

	frame_buf = malloc((size_t)(conf_window_width * conf_window_height *
				    3));
	if (frame_buf == NULL) {
		log_memory();
		return false;
	}

	row_pointers = malloc(sizeof(png_bytep) * (size_t)conf_window_height);
	if (row_pointers == NULL) {
		log_memory();
		return false;
	}
	for (y = 0; y < conf_window_height; y++) {
		row_pointers[y] = 
			(png_bytep)&frame_buf[conf_window_width * 3 *
					      (conf_window_height - y)];
	}

	start_time = GetTickCount64();
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

/* ディレクトリを削除する */
static bool delete_directory(LPCWSTR pszDirName)
{
	wchar_t path[256];
	HANDLE hFind;
	WIN32_FIND_DATA wfd;

	/* ディレクトリの内容を取得する */
	_snwprintf(path, sizeof(path), L"%s\\*.*", pszDirName);
	hFind = FindFirstFile(path, &wfd);
	if(hFind == INVALID_HANDLE_VALUE)
		return true;

	/* 再帰的に削除する */
	do
	{
		if (wcscmp(wfd.cFileName, L".") == 0)
			continue;
		if (wcscmp(wfd.cFileName, L"..") == 0)
			continue;

		if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			_snwprintf(path, sizeof(path), L"%s\\%s", pszDirName,
				   wfd.cFileName);
			if (!delete_directory(path))
				return false;
		}
		else
		{
			_snwprintf(path, sizeof(path), L"%s\\%s", pszDirName,
				   wfd.cFileName);
			DeleteFile(path);
		}
    } while(FindNextFile(hFind, &wfd));

    FindClose(hFind);
    return true;
}

void capture_input(void)
{
	is_input_changed = false;
	if (is_left_button_pressed ||
	    is_right_button_pressed ||
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
	if (!is_input_changed)
		return;

	cur_time = GetTickCount64() - start_time;

	fprintf(csv_fp,
		"%lld,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
		cur_time,
		mouse_pos_x,
		mouse_pos_y,
		is_left_button_pressed,
		is_right_button_pressed,
		is_return_pressed,
		is_space_pressed,
		is_escape_pressed,
		is_up_pressed,
		is_down_pressed,
		is_page_up_pressed,
		is_page_down_pressed,
		is_control_pressed);
	fflush(csv_fp);
}

bool capture_output(void)
{
	char fname[256];
	png_structp png;
	png_infop info;
	FILE *png_fp;

	if (!is_input_changed)
		return true;

	/* フレームバッファの内容を取得する */
	glReadBuffer(GL_FRONT);
	glReadPixels(0, 0, conf_window_width, conf_window_height, GL_RGB,
		     GL_UNSIGNED_BYTE, frame_buf);

	/* ファイル名を決める */
	snprintf(fname, sizeof(fname), "%s\\%lld.png", CAP_DIR, cur_time);

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

	return true;
}
