/* -*- coding: utf-8; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Suika 2
 * Copyright (C) 2001-2023, TABATA Keiichi. All rights reserved.
 */

/*
 * [Changes]
 *  - 2021/06/10 作成
 *  - 2023/01/06 日本語の位置名に対応
 *  - 2023/08/20 新アニメーションシステムへ移行
 */

#include "suika.h"

static int anime_layer;

static bool init(void);
static bool get_position(int *chpos, const char *pos);
static int get_accel(const char *accel_s);
static int get_alpha(const char *alpha);
static void draw(int *x, int *y, int *w, int *h);
static void process_finish(void);
static bool cleanup(void);

/*
 * キャラクタアニメコマンド
 */
bool cha_command(int *x, int *y, int *w, int *h)
{
	if (!is_in_command_repetition())
		if (!init())
			return false;

	draw(x, y, w, h);

	if (!is_in_command_repetition())
		if (!cleanup())
			return false;

	*x = 0;
	*y = 0;
	*w = conf_window_width;
	*h = conf_window_height;

	return true;
}

/*
 * コマンドの初期化処理を行う
 */
static bool init(void)
{
	const char *pos_s, *alpha_s, *accel_s;
	float span;
	int chpos, ofs_x, ofs_y, alpha, from_x, from_y, to_x, to_y, accel;
	int stage_layer;

	/* パラメータを取得する */
	pos_s = get_string_param(CHA_PARAM_POS);
	span = get_float_param(CHA_PARAM_SPAN);
	accel_s = get_string_param(CHA_PARAM_ACCEL);
	ofs_x = get_int_param(CHA_PARAM_OFFSET_X);
	ofs_y = get_int_param(CHA_PARAM_OFFSET_Y);
	alpha_s = get_string_param(CHA_PARAM_ALPHA);

	/* キャラの位置を取得する */
	if (!get_position(&chpos, pos_s))
		return false;

	/* キャラの位置からアニメレイヤーを求める */
	stage_layer = chpos_to_layer(chpos);
	if (get_layer_file_name(stage_layer) == NULL) {
		log_script_cha_no_image(pos_s);
		log_script_exec_footer();
		return false;
	}

	/* 加速を取得する */
	accel = get_accel(accel_s);
	if (accel == -1)
		return false;

	/* アルファ値を求める */
	alpha = get_alpha(alpha_s);

	/* キャラのfrom位置を取得する */
	from_x = get_layer_x(stage_layer);
	from_y = get_layer_y(stage_layer);

	/* キャラのto位置を計算する */
	to_x = from_x + ofs_x;
	to_y = from_y + ofs_y;

	/* アニメ定義を行う */
	anime_layer = chpos_to_anime_layer(chpos);
	clear_anime_sequence(anime_layer);
	new_anime_sequence(anime_layer);
	add_anime_sequence_property_f("start", 0);
	add_anime_sequence_property_f("end", span);
	add_anime_sequence_property_i("from-x", from_x);
	add_anime_sequence_property_i("from-y", from_y);
	add_anime_sequence_property_i("from-a", get_layer_alpha(anime_layer));
	add_anime_sequence_property_i("to-x", to_x);
	add_anime_sequence_property_i("to-y", to_y);
	add_anime_sequence_property_i("to-a", alpha);
	add_anime_sequence_property_i("accel", accel);

	/* アニメを開始する */
	start_layer_anime(anime_layer);

	/* 繰り返し動作を開始する */
	start_command_repetition();

	/* メッセージボックスを消す */
	show_namebox(false);
	show_msgbox(false);
	show_click(false);

	return true;
}

/* キャラの位置指定を取得する */
static bool get_position(int *chpos, const char *pos)
{
	if (strcmp(pos, "back") == 0 || strcmp(pos, "b") == 0 ||
	    strcmp(pos, U8("背面")) == 0) {
		/* 中央背面に配置する */
		*chpos = CH_BACK;
	} else if (strcmp(pos, "left") == 0 || strcmp(pos, "l") == 0 ||
		   strcmp(pos, U8("左")) == 0) {
		/* 左に配置する */
		*chpos = CH_LEFT;
	} else if (strcmp(pos, "right") == 0 || strcmp(pos, "r") == 0 ||
		   strcmp(pos, U8("右")) == 0) {
		/* 右に配置する */
		*chpos = CH_RIGHT;
	} else if (strcmp(pos, "center") == 0 || strcmp(pos, "centre") == 0 ||
		   strcmp(pos, "c") == 0 || strcmp(pos, U8("右")) == 0) {
		/* 中央に配置する */
		*chpos = CH_CENTER;
	} else if (strcmp(pos, "face") == 0 || strcmp(pos, "f") == 0 ||
		   strcmp(pos, U8("顔")) == 0) {
		/* 顔に配置する */
		*chpos = CH_FACE;
	} else {
		/* スクリプト実行エラー */
		log_script_ch_position(pos);
		log_script_exec_footer();
		return false;
	}
	return true;
}

/* 加速を取得する */
static int get_accel(const char *accel_s)
{
	if (strcmp(accel_s, "move") == 0 ||
	    strcmp(accel_s, U8("なし")) == 0) {
		return ANIME_ACCEL_UNIFORM;
	} else if (strcmp(accel_s, "accel") == 0 ||
		   strcmp(accel_s, U8("あり")) == 0) {
		return ANIME_ACCEL_ACCEL;
	} else if (strcmp(accel_s, "brake") == 0 ||
		   strcmp(accel_s, U8("減速")) == 0) {
		return ANIME_ACCEL_DEACCEL;
	} else {
		/* スクリプト実行エラー */
		log_script_cha_accel(accel_s);
		log_script_exec_footer();
		return -1;
	}
}

/* アルファ値を取得する */
static int get_alpha(const char *alpha)
{
	if (strcmp(alpha, "show") == 0 ||
	    strcmp(alpha, "") == 0)
		return 255;
	else if (strcmp(alpha, "hide") == 0)
		return 0;
	else
		return atoi(alpha);
}

/* 描画を行う */
static void draw(int *x, int *y, int *w, int *h)
{
	/* アニメ終了の場合を処理する */
	process_finish();

	/* ステージを描画する */
	draw_stage();
	*x = 0;
	*y = 0;
	*w = conf_window_width;
	*h = conf_window_height;

	/* 折りたたみシステムメニューを描画する */
	if (conf_sysmenu_transition && !is_non_interruptible())
		draw_stage_collapsed_sysmenu(false, x, y, w, h);
}

/* アニメ終了を処理する */
static void process_finish(void)
{
	/* 繰り返し動作をしていない場合 */
	if (!is_in_command_repetition())
		return;

	/* アニメが終了した場合 */
	if (is_anime_finished_for_layer(anime_layer)) {
		/* 繰り返し動作を終了する */
		stop_command_repetition();
		return;
	}	

	/* 入力によりスキップされた場合 */
	if (!is_non_interruptible() && !is_auto_mode() &&
	    (is_control_pressed || is_return_pressed ||
	     is_left_clicked || is_down_pressed)) {
		/* アニメを終了する */
		finish_layer_anime(anime_layer);

		/* 繰り返し動作を終了する */
		stop_command_repetition();
		return;
	}
}

/* 終了処理を行う */
static bool cleanup(void)
{
	/* アニメが終了した場合は削除する */
	if (is_anime_finished_for_layer(anime_layer))
		clear_anime_sequence(anime_layer);

	/* 次のコマンドに移動する */
	if (!move_to_next_command())
		return false;

	return true;
}
