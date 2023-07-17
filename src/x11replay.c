/* -*- tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Suika 2
 * Copyright (C) 2001-2021, TABATA Keiichi. All rights reserved.
 */

/*
 * [Changes]
 *  2022-07-17 作成
 */
#include <GL/gl.h>
#include <sys/stat.h>	/* mkdir() */
#include <sys/time.h>	/* gettimeofday() */
#include "png.h"

#include "suika.h"

/* リプレイ出力ディレクトリ */
#define REP_DIR		"replay"

/* CSVファイル */
static FILE *csv_fp;

/* 開始時刻 */
static uint64_t start_time;

/* 次のシミュレーションイベント */
uint64_t sim_time;
static int sim_mouse_pos_x = -1;
static int sim_mouse_pos_y = -1;
static bool sim_is_left_button_pressed;
static bool sim_is_right_button_pressed;
static bool sim_is_left_clicked;
static bool sim_is_right_clicked;
static bool sim_is_return_pressed;
static bool sim_is_space_pressed;
static bool sim_is_escape_pressed;
static bool sim_is_up_pressed;
static bool sim_is_down_pressed;
static bool sim_is_page_up_pressed;
static bool sim_is_page_down_pressed;
static bool sim_is_control_pressed;
static bool sim_exit;
static bool sim_executed;

/* マウス座標 */
static int prev_mouse_x = -1;
static int prev_mouse_y = -1;

/* フレームバッファコピー用 */
static char *frame_buf;

/* PNG書き出し用の行ポインタ */
png_bytep *row_pointers;

/* 前方参照 */
static bool read_csv_line(void);
static uint64_t get_tick_count64(void);

/*
 * リプレイモジュールを初期化する
 */
bool init_replay(int argc, char *argv[])
{
	char buf[1024];
	int y;

	/* CSVファイルを開く */
	if (argc < 2) {
		log_error("Csv file not specified.");
		return false;
	}
	snprintf(buf, sizeof(buf), "%s/main.csv", argv[1]);
	csv_fp = fopen(buf, "r");
	if (csv_fp == NULL) {
		log_file_open(argv[1]);
		return false;
	}
	if (fgets(buf, sizeof(buf), csv_fp) == NULL) {
		log_error("Failed to read csv file header.");
		return false;
	}

	/* 最初の行を読み込む */
	if (!read_csv_line()) {
		sim_exit = true;
		return true;
	}

	/* リプレイ出力データのディレクトリを作成する */
	mkdir(REP_DIR, 0700); 

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
					      (conf_window_height - y)];
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
	uint64_t cur_time;

	if (sim_exit)
		return false;

	cur_time = get_tick_count64() - start_time;
	if (cur_time < sim_time) {
		/* 入力はない */
		mouse_pos_x = prev_mouse_x;
		mouse_pos_y = prev_mouse_y;
		is_left_button_pressed = false;
		is_right_button_pressed = false;
		is_left_clicked = false;
		is_right_clicked = false;
		is_return_pressed = false;
		is_space_pressed = false;
		is_escape_pressed = false;
		is_up_pressed = false;
		is_down_pressed = false;
		is_page_up_pressed = false;
		is_page_down_pressed = false;
		is_control_pressed = false;
		sim_executed = false;
		return true;
	}

	/* 入力状態をセットアップする */
	mouse_pos_x = sim_mouse_pos_x;
	mouse_pos_y = sim_mouse_pos_y;
	is_left_button_pressed = sim_is_left_button_pressed;
	is_right_button_pressed = sim_is_right_button_pressed;
	is_left_clicked = sim_is_left_clicked;
	is_right_clicked = sim_is_right_clicked;
	is_return_pressed = sim_is_return_pressed;
	is_space_pressed = sim_is_space_pressed;
	is_escape_pressed = sim_is_escape_pressed;
	is_up_pressed = sim_is_up_pressed;
	is_down_pressed = sim_is_down_pressed;
	is_page_up_pressed = sim_is_page_up_pressed;
	is_page_down_pressed = sim_is_page_down_pressed;
	is_control_pressed = sim_is_control_pressed;
	prev_mouse_x = mouse_pos_x;
	prev_mouse_y = mouse_pos_y;
	sim_executed = true;

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

	/* リプレイ操作が行われたフレームでなければ処理しない */
	if (!sim_executed)
		return true;

	/* フレームバッファの内容を取得する */
	glReadBuffer(GL_BACK);
	glReadPixels(0, 0, conf_window_width, conf_window_height, GL_RGB,
		     GL_UNSIGNED_BYTE, frame_buf);

	/* ファイル名を決める */
	snprintf(fname, sizeof(fname), "%s/%lu.png", REP_DIR, sim_time);

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

	/* 次のCSV行を読み込む */
	if (!read_csv_line())
		sim_exit = true;

	return true;
}

/* CSVファイルの行を読み込む */
static bool read_csv_line(void)
{
	int ret;

	if (sim_exit)
		return true;

	ret = fscanf(csv_fp,
		     "%lu,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",
		     &sim_time,
		     &sim_mouse_pos_x,
		     &sim_mouse_pos_y,
		     (int *)&sim_is_left_button_pressed,
		     (int *)&sim_is_right_button_pressed,
		     (int *)&sim_is_left_clicked,
		     (int *)&sim_is_right_clicked,
		     (int *)&sim_is_return_pressed,
		     (int *)&sim_is_space_pressed,
		     (int *)&sim_is_escape_pressed,
		     (int *)&sim_is_up_pressed,
		     (int *)&sim_is_down_pressed,
		     (int *)&sim_is_page_up_pressed,
		     (int *)&sim_is_page_down_pressed,
		     (int *)&sim_is_control_pressed);
	if (ret != 15)
		sim_exit = true;

	return true;
}

/* ミリ秒の時刻を取得する */
static uint64_t get_tick_count64(void)
{
	struct timeval tv;

	gettimeofday(&tv, NULL);

	return (uint64_t)tv.tv_sec * 1000LL + (uint64_t)tv.tv_usec / 1000LL;
}

/*
 * ALSA
 */

/* サウンドを再生を開始する */
bool play_sound(int stream, struct wave *w)
{
	UNUSED_PARAMETER(stream);
	UNUSED_PARAMETER(w);
	return true;
}

/* サウンドの再生を停止する */
bool stop_sound(int stream)
{
	UNUSED_PARAMETER(stream);
	return true;
}

/* サウンドのボリュームを設定する */
bool set_sound_volume(int stream, float vol)
{
	UNUSED_PARAMETER(stream);
	UNUSED_PARAMETER(vol);
	return true;
}

/* サウンドが再生終了したか調べる */
bool is_sound_finished(int stream)
{
	UNUSED_PARAMETER(stream);
	return true;
}
