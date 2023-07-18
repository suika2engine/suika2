// -*- tab-width: 4; c-basic-offset: 4; indent-tabs-mode: nil; -*-

/*
 * Suika 2
 * Copyright (C) 2001-2023, TABATA Keiichi. All rights reserved.
 */

/*
 * [Changed]
 *  - 2023/07/18 Created.
 */

#define GL_SILENCE_DEPRECATION
#import <OpenGL/gl3.h>

#import <Cocoa/Cocoa.h>

#import "suika.h"
#import "png.h"
#import "nsmain.h"

/* リプレイ出力ディレクトリ */
#define REP_DIR		"replay"

/* 定期的にフレームをチェックする時間(3fps) */
#define FRAME_MILLI	333

// replayフォルダのパス
static NSString *replayPath;

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
png_bytep *row_pointers;

/* 前方参照 */
static bool read_csv_line(void);
static uint64_t get_tick_count64(void);

/*
 * リプレイモジュールを初期化する
 */
bool init_replay(int argc, char *argv[])
{
    const char *fname;
    char buf[1024];
	int y;

	/* CSVファイルを開く TODO: recordの名前を可変にする */
    NSString *bundlePath = [[NSBundle mainBundle] bundlePath];
    NSString *basePath = [bundlePath stringByDeletingLastPathComponent];
    fname = [[NSString stringWithFormat:@"%@/record/main.csv", basePath] UTF8String];
	csv_fp = fopen(fname, "r");
	if (csv_fp == NULL) {
		log_file_open(fname);
		return false;
	}
	if (fgets(buf, sizeof(buf), csv_fp) == NULL) {
		log_error("Failed to read csv file header.");
		return false;
	}

	// replayフォルダを作成しなおす
    replayPath = [NSString stringWithFormat:@"%@/%s", basePath, REP_DIR];
    [[NSFileManager defaultManager] removeItemAtPath:replayPath error:nil];
    [[NSFileManager defaultManager] createDirectoryAtPath:replayPath
                              withIntermediateDirectories:YES
                                               attributes:nil
                                                    error:NULL];

	/* フレームバッファのコピー用メモリを確保する */
	frame_buf = malloc((size_t)(conf_window_width * conf_window_height * 3));
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
	const char *cpath;
	png_structp png;
	png_infop info;
	FILE *png_fp;

	/* キャプチャが行われたフレームでなければ処理しない */
	if (!sim_execute)
		return true;

	/* フレームバッファの内容を取得する */
	glReadBuffer(GL_FRONT);
	glReadPixels(0, 0, conf_window_width, conf_window_height, GL_RGB,
		     GL_UNSIGNED_BYTE, frame_buf);

	/* ファイル名を決める */
    cpath = [[NSString stringWithFormat:@"%@/%llu.png", replayPath, sim_time] UTF8String];

	/* PNGファイルをオープンする */
	png_fp = fopen(cpath, "wb");
	if (png_fp == NULL) {
		log_file_open(cpath);
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

	ret = fscanf(csv_fp,
		     "%llu,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",
		     &sim_time,
		     &b_sim_execute,
		     &mouse_pos_x,
		     &mouse_pos_y,
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

	/* fscanf()はboolを受け取れないので、intで受け取ってコピーする */
	sim_execute = b_sim_execute;
	is_left_button_pressed = b_is_left_button_pressed;
	is_right_button_pressed = b_is_right_button_pressed;
	is_left_clicked = b_is_left_clicked;
	is_right_clicked = b_is_right_clicked;
	is_return_pressed = b_is_return_pressed;
	is_space_pressed = b_is_space_pressed;
	is_escape_pressed = b_is_escape_pressed;
	is_up_pressed = b_is_up_pressed;
	is_down_pressed = b_is_down_pressed;
	is_page_up_pressed = b_is_page_up_pressed;
	is_page_down_pressed = b_is_page_down_pressed;
	is_control_pressed = b_is_control_pressed;

	return true;
}

/* ミリ秒の時刻を取得する */
static uint64_t get_tick_count64(void)
{
	struct timeval tv;

	gettimeofday(&tv, NULL);

	return (uint64_t)tv.tv_sec * 1000LL + (uint64_t)tv.tv_usec / 1000LL;
}
