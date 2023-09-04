/* -*- tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Suika 2
 * Copyright (C) 2001-2023, TABATA Keiichi. All rights reserved.
 */

/*
 * [Changes]
 *  2022-07-17 作成
 *  2022-08-26 Windows/Mac/Linux共通化
 */

#include "suika.h"
#include "png.h"

#ifdef OSX
#define GL_SILENCE_DEPRECATION
#include <OpenGL/gl3.h>
#else
#include <GL/gl.h>
#endif

/* リプレイ出力ディレクトリ */
#define REC_DIR		"record"
#define REC_DIR_L	L"record"
#define REP_DIR		"replay"
#define CSV_FILE	"main.csv"
#define CSV_FILE_L	L"main.csv"

/* 定期的にフレームをチェックする時間(3fps) */
#define FRAME_MILLI	333

/* CSVファイル */
static FILE *csv_fp;

/* 開始時刻 */
static uint64_t start_time;

/* 次のシミュレーションイベント */
uint64_t sim_time;
static bool sim_exit;
static bool sim_execute;

/* フレームバッファコピー用 */
static char *frame_buf;

/* PNG書き出し用の行ポインタ */
static png_bytep *row_pointers;

/* マウス状態 */
static bool prev_left_pressed;
static bool prev_right_pressed;
static int prev_mouse_x;
static int prev_mouse_y;

/* 前方参照 */
static bool read_csv_line(void);

#ifdef WIN
/* winmain.c */
const char *conv_utf16_to_utf8(const wchar_t *utf16_message);
#endif

/*
 * リプレイモジュールを初期化する
 */
#ifdef WIN
bool init_replay(int argc, wchar_t *argv[])
#else
bool init_replay(int argc, char *argv[])
#endif
{
#ifdef WIN
	wchar_t path[1024];
#else
	char path[1024];
#endif
	char line[1024];
	int y;

	/* CSVファイルを開く */
#ifdef WIN
	if (argc < 2)
		swprintf(path, sizeof(path) / sizeof(wchar_t), L"%s/%s", REC_DIR, CSV_FILE);
	else
		swprintf(path, sizeof(path) / sizeof(wchar_t), L"%ls/%s", argv[1], CSV_FILE);
	csv_fp = _wfopen(path, L"r");
	if (csv_fp == NULL) {
		log_file_open(conv_utf16_to_utf8(path));
		return false;
	}
#else
	if (argc < 2)
		snprintf(path, sizeof(path), "%s/%s", REC_DIR, CSV_FILE);
	else
		snprintf(path, sizeof(path), "%s/%s", argv[1], CSV_FILE);
	csv_fp = fopen(path, "r");
	if (csv_fp == NULL) {
		log_file_open(path);
		return false;
	}
#endif

	/* CSVのヘッダ行を読み込む */
	if (fgets(line, sizeof(line), csv_fp) == NULL) {
		log_error("Failed to read csv file header.");
		return false;
	}

	/* リプレイ出力データのディレクトリを作成する */
	reconstruct_dir(REP_DIR);

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
 * リプレイモジュールを終了する
 */
void cleanup_replay(void)
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
 * 入力をリプレイする
 */
bool replay_input(void)
{
	if (sim_exit)
		return false;

	/* CSVの行を読み込む */
	if (!read_csv_line()) {
		sim_exit = true;
		return true;
	}

	return true;
}

/*
 * リプレイ結果の出力をキャプチャする
 */
bool replay_output(void)
{
	char fname[256];
	png_structp png;
	png_infop info;
	FILE *png_fp;

	/* キャプチャが行われたフレームでなければ処理しない */
	if (!sim_execute)
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
	snprintf(fname,
		 sizeof(fname),
#if (defined(WIN) && !defined(__WIN64)) || defined(OSX)
		 "%s/%llu.png",
#else
		 "%s/%lu.png",
 #endif
		 REP_DIR,
		 sim_time);

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

/* CSVファイルの行を読み込む */
static bool read_csv_line(void)
{
	int ret;
	int b_sim_execute;
	int i_mouse_x;
	int i_mouse_y;
	int b_is_left_button_pressed;
	int b_is_right_button_pressed;
	int b_is_left_clicked;
	int b_is_right_clicked;
	int b_is_return_pressed;
	int b_is_space_pressed;
	int b_is_escape_pressed;
	int b_is_up_pressed;
	int b_is_down_pressed;
	int b_is_page_up_pressed;
	int b_is_page_down_pressed;
	int b_is_control_pressed;

	/* fscanf()はboolを受け取れないので、intで受け取ってコピーする */
	ret = fscanf(csv_fp,
#if (defined(WIN) && !defined(__WIN64)) || defined(OSX)
		     "%llu,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",
#else
		     "%lu,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",
#endif		     
		     &sim_time,
		     &b_sim_execute,
		     &i_mouse_x,
		     &i_mouse_y,
		     &b_is_left_button_pressed,
		     &b_is_right_button_pressed,
		     &b_is_left_clicked,
		     &b_is_right_clicked,
		     &b_is_return_pressed,
		     &b_is_space_pressed,
		     &b_is_escape_pressed,
		     &b_is_up_pressed,
		     &b_is_down_pressed,
		     &b_is_page_up_pressed,
		     &b_is_page_down_pressed,
		     &b_is_control_pressed);
	if (ret != 16)
		sim_exit = true;

	/* シミュレーション時刻を更新する */
	sim_execute = b_sim_execute;

	/* マウス左クリックを発生させる */
	if (b_is_left_button_pressed)
		on_event_mouse_press(MOUSE_LEFT, i_mouse_x, i_mouse_y);

	/* マウス右クリックを発生させる */
	if (b_is_right_button_pressed)
		on_event_mouse_press(MOUSE_RIGHT, i_mouse_x, i_mouse_y);

	/* 左マウスリリースを発生させる */
	if (prev_left_pressed && !b_is_left_button_pressed)
		on_event_mouse_release(MOUSE_LEFT, i_mouse_x, i_mouse_y);

	/* 右マウスリリースを発生させる */
	if (prev_right_pressed && !b_is_right_button_pressed)
		on_event_mouse_release(MOUSE_RIGHT, i_mouse_x, i_mouse_y);

	/* マウス移動を発生させる */
	if (!b_is_left_button_pressed && !b_is_right_button_pressed &&
	    (i_mouse_x != prev_mouse_x || i_mouse_y != prev_mouse_y))
		on_event_mouse_move(i_mouse_x, i_mouse_y);

	/* クリック状態を反映する (同一フレーム内で押下/解放があった場合のため) */
	is_left_clicked = b_is_left_clicked;
	is_right_clicked = b_is_right_clicked;

	/* キー状態を反映する */
	is_return_pressed = b_is_return_pressed;
	is_space_pressed = b_is_space_pressed;
	is_escape_pressed = b_is_escape_pressed;
	is_up_pressed = b_is_up_pressed;
	is_down_pressed = b_is_down_pressed;
	is_page_up_pressed = b_is_page_up_pressed;
	is_page_down_pressed = b_is_page_down_pressed;
	is_control_pressed = b_is_control_pressed;

	/* このフレームのマウス状態を保存する */
	prev_left_pressed = b_is_left_button_pressed;
	prev_right_pressed = b_is_right_button_pressed;
	prev_mouse_x = i_mouse_x;
	prev_mouse_y = i_mouse_y;

	return true;
}
