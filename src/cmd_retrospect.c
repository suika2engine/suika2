/* -*- coding: utf-8-with-signature; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Suika
 * Copyright (C) 2001-2017, TABATA Keiichi. All rights reserved.
 */

/*
 * @retrospectコマンド
 *
 * [Changes]
 *  - 2016/07/13 作成
 *  - 2017/08/19 キャンセル時の動作を修正
 *  - 2021/06/10 キャラクタのアルファ値に対応
 */

#include "suika.h"

/* false assertion */
#define NEVER_COME_HERE	(0)

/* サムネイルの最大数 */
#define THUMBNAIL_SIZE	(12)

/* パラメータのインデックス */
#define PARAM_LABEL(n)	(RETROSPECT_PARAM_LABEL1 + 4 * n)
#define PARAM_FLAG(n)	(RETROSPECT_PARAM_LABEL1 + 4 * n + 1)
#define PARAM_X(n)	(RETROSPECT_PARAM_LABEL1 + 4 * n + 2)
#define PARAM_Y(n)	(RETROSPECT_PARAM_LABEL1 + 4 * n + 3)

/* サムネイルの数 */
static int thumbnail_count;

/* サムネイルを隠す色 */
static pixel_t hide_color;

/* サムネイルのサイズ */
static int thumbnail_width;
static int thumbnail_height;

/* サムネイルごとの情報 */
static struct thumbnail {
	const char *label;
	bool enabled;
	int x;
	int y;
} thumbnail[THUMBNAIL_SIZE];

/* 最初の描画であるか */
static bool is_first_frame;

/* ポイントされている項目のインデックス */
static int pointed_index;

/* retrospectコマンドが完了したばかりであるかのフラグ */
static bool retrospect_finished_flag;

/* 前方参照 */
static bool init(void);
static bool load_images(void);
static bool load_hide_color(void);
static bool load_thumbnail_size(void);
static bool load_thumbnail_params(void);
static void fill_thumbnail_rects(void);
static void draw_frame(int *x, int *y, int *w, int *h);
static int get_pointed_index(void);
static bool cleanup(void);
static bool cancel(void);

/*
 * retrospectコマンド
 */
bool retrospect_command(int *x, int *y, int *w, int *h)
{
	/* 初期化処理を行う */
	if (!is_in_command_repetition())
		if (!init())
			return false;

	/* 右クリックによるキャンセルを処理する */
	if (is_right_button_pressed)
		return cancel();

	/* 繰り返し動作を行う */
	draw_frame(x, y, w, h);

	/* 終了処理を行う */
	if (!is_in_command_repetition())
		if (!cleanup())
			return false;

	return true;
}

/* コマンドの初期化処理を行う */
bool init(void)
{
	/* 画像を読み込む */
	if (!load_images())
		return false;

	/* サムネイルを隠す色を取得する */
	if (!load_hide_color())
		return false;

	/* サムネイルのサイズを取得する */
	if (!load_thumbnail_size())
		return false;

	/* サムネイルの情報を取得する */
	if (!load_thumbnail_params())
		return false;

	/* フラグの値が0であるサムネイルを色で塗り潰す */
	fill_thumbnail_rects();

	/* 繰り返し動作を開始する */
	start_command_repetition();

	/* ポイントされているボタンを初期化する */
	pointed_index = -1;

	/* 初回の描画であることを記録する */
	is_first_frame = true;

	return true;
}

/* 画像を読み込む */
static bool load_images(void)
{
	const char *file;
	struct image *img;

	/* 背景を読み込んでFOレイヤに描画する */
	file = get_string_param(RETROSPECT_PARAM_BG_FILE);
	img = create_image_from_file(BG_DIR, file);
	if (img == NULL) {
		log_script_exec_footer();
		return false;
	}
	draw_image_to_fo(img);
	destroy_image(img);

	/* 前景を読み込んでFIレイヤに描画する */
	file = get_string_param(MENU_PARAM_FG_FILE);
	img = create_image_from_file(BG_DIR, file);
	if (img == NULL) {
		log_script_exec_footer();
		return false;
	}
	draw_image_to_fi(img);
	destroy_image(img);

	return true;
}

/* サムネイルを隠す色をロードする */
static bool load_hide_color(void)
{
	int r, g, b;

	r = get_int_param(RETROSPECT_PARAM_HIDE_R);
	if (r < 0) {
		log_script_rgb_negative(r);
		log_script_exec_footer();
		return false;
	}

	g = get_int_param(RETROSPECT_PARAM_HIDE_G);
	if (g < 0) {
		log_script_rgb_negative(g);
		log_script_exec_footer();
		return false;
	}

	b = get_int_param(RETROSPECT_PARAM_HIDE_B);
	if (b < 0) {
		log_script_rgb_negative(b);
		log_script_exec_footer();
		return false;
	}

	hide_color = make_pixel(0xff, (uint8_t)r, (uint8_t)g, (uint8_t)b);

	return true;
}

/* サムネイルのサイズをロードする */
static bool load_thumbnail_size(void)
{
	thumbnail_width = get_int_param(RETROSPECT_PARAM_WIDTH);
	if (thumbnail_width <= 0) {
		log_script_non_positive_size(thumbnail_width);
		log_script_exec_footer();
		return false;
	}

	thumbnail_height = get_int_param(RETROSPECT_PARAM_HEIGHT);
	if (thumbnail_width <= 0) {
		log_script_non_positive_size(thumbnail_width);
		log_script_exec_footer();
		return false;
	}

	return true;
}

/* サムネイルごとの情報をロードする */
static bool load_thumbnail_params(void)
{
	int32_t flag_val;
	int i;

	/* 各サムネイルの情報を取得する */
	thumbnail_count = 0;
	for(i = 0; i < THUMBNAIL_SIZE; i++) {
		/* ラベルを取得する */
		thumbnail[i].label = get_string_param(PARAM_LABEL(i));
		if (strcmp(thumbnail[i].label, "") == 0)
			break;	/* すべてのサムネイルを処理した*/

		/* 変数の値を求める */
		if (!get_variable_by_string(get_string_param(PARAM_FLAG(i)),
					    &flag_val))
			return false;

		/* サムネイルを有効にするかを求める */
		thumbnail[i].enabled = flag_val != 0;

		/* 表示位置を取得する */
		thumbnail[i].x = get_int_param(PARAM_X(i));
		thumbnail[i].y = get_int_param(PARAM_Y(i));

		thumbnail_count++;
	}

	for(i = thumbnail_count; i < THUMBNAIL_SIZE; i++)
		thumbnail[i].enabled = false;

	return true;
}

/* 隠すサムネールを色で塗り潰す */
static void fill_thumbnail_rects(void)
{
	int i;

	for(i = 0; i < thumbnail_count; i++) {
		if (thumbnail[i].enabled)
			continue;

		draw_rect_to_fo(thumbnail[i].x, thumbnail[i].y,
				thumbnail_width, thumbnail_height,
				hide_color);
	}
}

/* フレームを描画する */
static void draw_frame(int *x, int *y, int *w, int *h)
{
	int new_pointed_index;

	/* ポイントされている項目を取得する */
	new_pointed_index = get_pointed_index();

	/* 初回描画の場合 */
	if (is_first_frame) {
		if (new_pointed_index != -1) {
			/* 背景全体とボタンを1つ描画する */
			draw_stage_with_buttons(thumbnail[new_pointed_index].x,
						thumbnail[new_pointed_index].y,
						thumbnail_width,
						thumbnail_height,
						0, 0, 0, 0);
		} else {
			/* 背景全体を描画する */
			draw_stage_with_buttons(0, 0, 0, 0, 0, 0, 0, 0);
		}

		/* ウィンドウ全体を更新する */
		*w = conf_window_width;
		*h = conf_window_height;

		/* 初回の描画でないことを記録する */
		is_first_frame = false;

		/* ポイントされているサムネイルを変更する */
		pointed_index = new_pointed_index;
		return;
	}

	/* クリックされた場合 */
	if (new_pointed_index != -1 && is_left_button_pressed) {
		/* 繰り返し動作を終了する */
		pointed_index = new_pointed_index;
		stop_command_repetition();
		return;
	}

	/* ポイントされている項目が変更された場合 */
	if (new_pointed_index != -1 && pointed_index != -1) {
		/* 古いサムネイルを消して新しいサムネイルを描画する */
		draw_stage_rect_with_buttons(thumbnail[pointed_index].x,
					     thumbnail[pointed_index].y,
					     thumbnail_width,
					     thumbnail_height,
					     thumbnail[new_pointed_index].x,
					     thumbnail[new_pointed_index].y,
					     thumbnail_width,
					     thumbnail_height);

		/* ウィンドウの更新領域を求める */
		union_rect(x, y, w, h,
			   thumbnail[pointed_index].x,
			   thumbnail[pointed_index].y,
			   thumbnail_width,
			   thumbnail_height,
			   thumbnail[new_pointed_index].x,
			   thumbnail[new_pointed_index].y,
			   thumbnail_width,
			   thumbnail_height);

		/* ポイントされているサムネイルを変更する */
		pointed_index = new_pointed_index;
		return;
	}

	/* ポイントされてい状態からポイントされている状態に変化した場合 */
	if (new_pointed_index != -1 && pointed_index == -1) {
		/* 新しいサムネイルを描画する */
		draw_stage_rect_with_buttons(0, 0, 0, 0,
					     thumbnail[new_pointed_index].x,
					     thumbnail[new_pointed_index].y,
					     thumbnail_width,
					     thumbnail_height);

		/* ウィンドウの更新領域を求める */
		*x = thumbnail[new_pointed_index].x;
		*y = thumbnail[new_pointed_index].y;
		*w = thumbnail_width;
		*h = thumbnail_height;

		/* ポイントされているサムネイルを変更する */
		pointed_index = new_pointed_index;
		return;
	}

	/* ポイントされている状態からポイントされていない状態に変化した場合 */
	if (new_pointed_index == -1 && pointed_index != -1) {
		/* 古いサムネイルを消す */
		draw_stage_rect_with_buttons(thumbnail[pointed_index].x,
					     thumbnail[pointed_index].y,
					     thumbnail_width,
					     thumbnail_height,
					     0, 0, 0, 0);

		/* ウィンドウの更新領域を求める */
		*x = thumbnail[pointed_index].x;
		*y = thumbnail[pointed_index].y;
		*w = thumbnail_width;
		*h = thumbnail_height;

		/* ポイントされているサムネイルを変更する */
		pointed_index = new_pointed_index;
		return;
	}

	/* 選択に変更がない */
	assert(new_pointed_index == pointed_index);
}

/* ポイントされているサムネイルを取得する */
static int get_pointed_index(void)
{
	int i;

	for (i = 0; i < thumbnail_count; i++) {
		if (!thumbnail[i].enabled)
			continue;
		if (mouse_pos_x >= thumbnail[i].x &&
		    mouse_pos_x < thumbnail[i].x + thumbnail_width &&
		    mouse_pos_y >= thumbnail[i].y &&
		    mouse_pos_y < thumbnail[i].y + thumbnail_height)
			return i;
	}
	return -1;
}

/* コマンドを終了する */
static bool cleanup(void)
{
	assert(pointed_index != -1);

	/* ステージの画像を無効にする */
	change_bg_immediately(NULL);
	change_ch_immediately(CH_BACK, NULL, 0, 0, 0);
	change_ch_immediately(CH_LEFT, NULL, 0, 0, 0);
	change_ch_immediately(CH_RIGHT, NULL, 0, 0, 0);
	change_ch_immediately(CH_CENTER, NULL, 0, 0, 0);

	/* メニューコマンドが完了したばかりであることを記録する */
	retrospect_finished_flag = true;

	/* ラベルにジャンプする */
	return move_to_label(thumbnail[pointed_index].label);
}

/* コマンドをキャンセルする */
static bool cancel(void)
{
	assert(is_right_button_pressed);

	/* ステージの画像を無効にする */
	change_bg_immediately(NULL);
	change_ch_immediately(CH_BACK, NULL, 0, 0, 0);
	change_ch_immediately(CH_LEFT, NULL, 0, 0, 0);
	change_ch_immediately(CH_RIGHT, NULL, 0, 0, 0);
	change_ch_immediately(CH_CENTER, NULL, 0, 0, 0);

	/* メニューコマンドが完了したばかりであることを記録する */
	retrospect_finished_flag = true;

	/* 繰り返し動作を終了する */
	stop_command_repetition();

	/* ラベルにジャンプする */
	return move_to_next_command();
}

/*
 * メニューコマンドが完了したばかりであるかをチェックする
 */
bool check_retrospect_finish_flag(void)
{
	bool ret;

	ret = retrospect_finished_flag;
	retrospect_finished_flag = false;

	return ret;
}
