/* -*- coding: utf-8; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Suika 2
 * Copyright (C) 2001-2023, TABATA Keiichi. All rights reserved.
 */

/*
 * [Changes]
 *  - 2021/06/10 作成
 *  - 2023/01/06 日本語の位置名に対応
 */

#include "suika.h"

#define INVALID_ACCEL_TYPE	(0)

enum cha_accel_type {
	CHA_ACCEL_UNIFORM,
	CHA_ACCEL_ACCEL,
	CHA_ACCEL_DEACCEL,
};

static stop_watch_t sw;
static float span;
static int accel;

static bool init(void);
static bool get_position(int *chpos, const char *pos);
static bool get_accel(const char *accel_s);
static int get_alpha(const char *alpha);
static void draw(void);
static bool cleanup(void);

/*
 * キャラクタアニメコマンド
 */
bool cha_command(int *x, int *y, int *w, int *h)
{
	if (!is_in_command_repetition())
		if (!init())
			return false;

	draw();

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
	const char *pos, *alpha_s, *accel_s;
	int chpos, ofs_x, ofs_y, alpha, x, y;

	/* パラメータを取得する */
	pos = get_string_param(CHA_PARAM_POS);
	span = get_float_param(CHA_PARAM_SPAN);
	accel_s = get_string_param(CHA_PARAM_ACCEL);
	ofs_x = get_int_param(CHA_PARAM_OFFSET_X);
	ofs_y = get_int_param(CHA_PARAM_OFFSET_Y);
	alpha_s = get_string_param(CHA_PARAM_ALPHA);

	/* キャラの位置を取得する */
	if (!get_position(&chpos, pos))
		return false;

	/* 加速を取得する */
	if (!get_accel(accel_s))
		return false;

	/* アルファ値を求める */
	alpha = get_alpha(alpha_s);

	/* キャラのオフセットを絶対座標に変換する */
	get_ch_position(chpos, &x, &y);
	x += ofs_x;
	y += ofs_y;

	/* Controlが押されているか、フェードしない場合 */
	if ((span == 0)
	    ||
	    (!is_non_interruptible() && is_skip_mode())
	    ||
	    (!is_non_interruptible() && !is_auto_mode() && is_control_pressed)) {
		/* フェードせず、すぐに切り替える */
		change_ch_attributes(chpos, x, y, alpha);
	} else {
		/* 繰り返し動作を開始する */
		start_command_repetition();

		/* キャラフェードモードを有効にする */
		start_ch_anime(chpos, x, y, alpha);

		/* 時間計測を開始する */
		reset_stop_watch(&sw);
	}

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
static bool get_accel(const char *accel_s)
{
	if (strcmp(accel_s, "move") == 0 ||
	    strcmp(accel_s, U8("なし")) == 0) {
		accel = CHA_ACCEL_UNIFORM;
		return true;
	} else if (strcmp(accel_s, "accel") == 0 ||
		   strcmp(accel_s, U8("あり")) == 0) {
		accel = CHA_ACCEL_ACCEL;
		return true;
	} else if (strcmp(accel_s, "brake") == 0 ||
		   strcmp(accel_s, U8("減速")) == 0) {
		accel = CHA_ACCEL_DEACCEL;
		return true;
	} else {
		/* スクリプト実行エラー */
		log_script_cha_accel(accel_s);
		log_script_exec_footer();
		return false;
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
static void draw(void)
{
	float lap, progress;
	int x, y, w, h;

	/* 経過時間を取得する */
	lap = (float)get_stop_watch_lap(&sw) / 1000.0f;
	if (lap >= span)
		lap = span;

	/* 加速を処理する */
	progress = lap / span;
	switch (accel) {
	case CHA_ACCEL_UNIFORM:
		break;
	case CHA_ACCEL_ACCEL:
		progress = progress * progress;
		break;
	case CHA_ACCEL_DEACCEL:
		progress = sqrtf(progress);
		break;
	default:
		assert(INVALID_ACCEL_TYPE);
		break;
	}

	/* 進捗を設定する */
	if (is_in_command_repetition()) {
		/*
		 * 経過時間が一定値を超えた場合と、
		 * 入力によりスキップされた場合
		 */
		if ((lap >= span)
		    ||
		    (!is_non_interruptible() && !is_auto_mode() &&
		     (is_control_pressed || is_return_pressed ||
		      is_left_clicked || is_down_pressed))) {
			/* 繰り返し動作を終了する */
			stop_command_repetition();

			/* キャラフェードモードを終了する */
			stop_ch_anime();
		} else {
			/* 進捗を設定する */
			set_ch_anime_progress(progress);
		}
	}

	/* ステージを描画する */
	if (is_in_command_repetition())
		draw_stage();	/* TODO: なんらかの最適化 */
	else
		draw_stage();

	/* 折りたたみシステムメニューを描画する */
	if (conf_sysmenu_transition && !is_non_interruptible()) {
		x = y = w = h = 0;
		draw_stage_collapsed_sysmenu(false, &x, &y, &w, &h);
	}
}

/* 終了処理を行う */
static bool cleanup(void)
{
	/* 次のコマンドに移動する */
	if (!move_to_next_command())
		return false;

	return true;
}
