/* -*- coding: utf-8-with-signature; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Suika
 * Copyright (C) 2001-2021, TABATA Keiichi. All rights reserved.
 */

/*
 * @menuコマンド
 *
 * [Changes]
 *  - 2016/07/04 作成
 *  - 2021/07/31 SEに対応
 */

#include "suika.h"

/* false assertion */
#define NEVER_COME_HERE	(0)

/* ボタンの数 */
#define BUTTON_COUNT	(16)

/* コマンドの引数 */
#define PARAM_LABEL(n)	(MENU_PARAM_LABEL1 + 5 * n)
#define PARAM_X(n)	(MENU_PARAM_LABEL1 + 5 * n + 1)
#define PARAM_Y(n)	(MENU_PARAM_LABEL1 + 5 * n + 2)
#define PARAM_W(n)	(MENU_PARAM_LABEL1 + 5 * n + 3)
#define PARAM_H(n)	(MENU_PARAM_LABEL1 + 5 * n + 4)

/* ボタン */
static struct button {
	const char *label;
	int x;
	int y;
	int w;
	int h;
} button[BUTTON_COUNT];

/* 最初の描画であるか */
static bool is_first_frame;

/* ポイントされている項目のインデックス */
static int pointed_index;

/* 前方参照 */
static bool init(void);
static void draw_frame(int *x, int *y, int *w, int *h);
static int get_pointed_index(void);
static void play_se(const char *file);
static bool cleanup(void);

/*
 * menuコマンド
 */
bool menu_command(int *x, int *y, int *w, int *h)
{
	/* 初期化処理を行う */
	if (!is_in_command_repetition())
		if (!init())
			return false;

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
	const char *file;
	struct image *img;
	int i;

	/* FO/FIレイヤをロックする */
	lock_fo_fi_for_menu();

	/* 背景を読み込んでFOレイヤに描画する */
	file = get_string_param(MENU_PARAM_BG_FILE);
	img = create_image_from_file(BG_DIR, file);
	if (img == NULL) {
		log_script_exec_footer();
		unlock_fo_fi_for_menu();
		return false;
	}
	draw_image_to_fo(img);
	destroy_image(img);

	/* 前景を読み込んでFIレイヤに描画する */
	file = get_string_param(MENU_PARAM_FG_FILE);
	img = create_image_from_file(BG_DIR, file);
	if (img == NULL) {
		log_script_exec_footer();
		unlock_fo_fi_for_menu();
		return false;
	}
	draw_image_to_fi(img);
	destroy_image(img);

	/* FO/FIレイヤをアンロックする */
	unlock_fo_fi_for_menu();

	/* ボタンのラベルと座標をロードする */
	for (i = 0; i < BUTTON_COUNT; i++) {
		button[i].label = get_string_param(PARAM_LABEL(i));
		button[i].x = get_int_param(PARAM_X(i));
		button[i].y = get_int_param(PARAM_Y(i));
		button[i].w = get_int_param(PARAM_W(i));
		button[i].h = get_int_param(PARAM_H(i));
	}

	/* 繰り返し動作を開始する */
	start_command_repetition();

	/* ポイントされているボタンを初期化する */
	pointed_index = -1;

	/* 初回の描画であることを記録する */
	is_first_frame = true;

	/* オートモードを終了する */
	if (is_auto_mode())
		stop_auto_mode();

	/* スキップモードを終了する */
	if (is_skip_mode())
		stop_skip_mode();

	return true;
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
			draw_stage_with_buttons(button[new_pointed_index].x,
						button[new_pointed_index].y,
						button[new_pointed_index].w,
						button[new_pointed_index].h,
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

		/* ポイントされているボタンを変更する */
		pointed_index = new_pointed_index;
		return;
	}

	/* クリックされた場合 */
	if (new_pointed_index != -1 && is_left_button_pressed) {
		/* 繰り返し動作を終了する */
		pointed_index = new_pointed_index;
		stop_command_repetition();

		/* 背景全体とボタンを1つ描画する */
		draw_stage_with_buttons_keep(button[new_pointed_index].x,
					     button[new_pointed_index].y,
					     button[new_pointed_index].w,
					     button[new_pointed_index].h,
					     0, 0, 0, 0);
		return;
	}

	/* ポイントされている項目が変更された場合 */
	if (new_pointed_index != -1 && pointed_index != -1 &&
	    new_pointed_index != pointed_index) {
		/* SEを再生する */
		play_se(conf_menu_change_se);

		/* 古いボタンを消して新しいボタンを描画する */
		draw_stage_rect_with_buttons(button[pointed_index].x,
					     button[pointed_index].y,
					     button[pointed_index].w,
					     button[pointed_index].h,
					     button[new_pointed_index].x,
					     button[new_pointed_index].y,
					     button[new_pointed_index].w,
					     button[new_pointed_index].h);

		/* ウィンドウの更新領域を求める */
		union_rect(x, y, w, h,
			   button[pointed_index].x,
			   button[pointed_index].y,
			   button[pointed_index].w,
			   button[pointed_index].h,
			   button[new_pointed_index].x,
			   button[new_pointed_index].y,
			   button[new_pointed_index].w,
			   button[new_pointed_index].h);

		/* ポイントされているボタンを変更する */
		pointed_index = new_pointed_index;
		return;
	}

	/* ポイントされていない状態からポイントされている状態に変化した場合 */
	if (new_pointed_index != -1 && pointed_index == -1) {
		/* SEを再生する */
		play_se(conf_menu_change_se);

		/* 新しいボタンを描画する */
		draw_stage_rect_with_buttons(0, 0, 0, 0,
					     button[new_pointed_index].x,
					     button[new_pointed_index].y,
					     button[new_pointed_index].w,
					     button[new_pointed_index].h);

		/* ウィンドウの更新領域を求める */
		*x = button[new_pointed_index].x;
		*y = button[new_pointed_index].y;
		*w = button[new_pointed_index].w;
		*h = button[new_pointed_index].h;

		/* ポイントされているボタンを変更する */
		pointed_index = new_pointed_index;
		return;
	}

	/* ポイントされている状態からポイントされていない状態に変化した場合 */
	if (new_pointed_index == -1 && pointed_index != -1) {
		/* 古いボタンを消す */
		draw_stage_rect_with_buttons(button[pointed_index].x,
					     button[pointed_index].y,
					     button[pointed_index].w,
					     button[pointed_index].h,
					     0, 0, 0, 0);

		/* ウィンドウの更新領域を求める */
		*x = button[pointed_index].x;
		*y = button[pointed_index].y;
		*w = button[pointed_index].w;
		*h = button[pointed_index].h;

		/* ポイントされているボタンを変更する */
		pointed_index = new_pointed_index;
		return;
	}

	/* 背景全体とボタンを1つ描画する */
	draw_stage_with_buttons_keep(button[new_pointed_index].x,
				     button[new_pointed_index].y,
				     button[new_pointed_index].w,
				     button[new_pointed_index].h,
				     0, 0, 0, 0);
	
	/* 選択に変更がない */
	assert(new_pointed_index == pointed_index);
}

/* ポイントされているボタンを取得する */
static int get_pointed_index(void)
{
	int i;

	for (i = 0; i < BUTTON_COUNT; i++) {
		if (mouse_pos_x >= button[i].x &&
		    mouse_pos_x < button[i].x + button[i].w &&
		    mouse_pos_y >= button[i].y &&
		    mouse_pos_y < button[i].y + button[i].h)
			return i;
	}
	return -1;
}

/* SEを再生する */
static void play_se(const char *file)
{
	struct wave *w;

	if (file == NULL || strcmp(file, "") == 0)
		return;

	w = create_wave_from_file(SE_DIR, file, false);
	if (w == NULL)
		return;

	set_mixer_input(SE_STREAM, w);
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
	set_menu_finish_flag();
	
	/* ラベルにジャンプする */
	return move_to_label(button[pointed_index].label);
}
