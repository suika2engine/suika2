/* -*- coding: utf-8; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Suika2
 * Copyright (C) 2001-2023, Keiichi Tabata. All rights reserved.
 */

/*
 * [Changes]
 *  - 2021/06/12 作成
 *  - 2023/01/06 日本語の指定に対応
 */

#include "suika.h"

#define INVALID_MOVE_TYPE	(0)

#define PI	(3.1415926f)

enum shake_move_type {
	SHAKE_MOVE_HORIZONTAL,
	SHAKE_MOVE_VERTICAL,
};

static uint64_t sw;
static int move;
static float span;
static int times;
static int amount;

static bool init(void);
static bool get_move(const char *move_s);
static void draw(void);
static bool cleanup(void);

/*
 * 画面揺らしコマンド
 */
bool shake_command(void)
{
	if (!is_in_command_repetition())
		if (!init())
			return false;

	draw();

	if (!is_in_command_repetition())
		if (!cleanup())
			return false;

	return true;
}

/*
 * コマンドの初期化処理を行う
 */
static bool init(void)
{
	const char *move_s;

	/* パラメータを取得する */
	move_s = get_string_param(SHAKE_PARAM_MOVE);
	span = get_float_param(SHAKE_PARAM_SPAN);
	times = get_int_param(SHAKE_PARAM_TIMES);
	amount = get_int_param(SHAKE_PARAM_AMOUNT);

	/* 移動方法を取得する */
	if (!get_move(move_s))
		return false;

	/* 画面揺らしモードを有効にする */
	start_fade_for_shake();

	/* 繰り返し動作を開始する */
	start_command_repetition();

	/* 時間計測を開始する */
	reset_lap_timer(&sw);

	/* メッセージボックスを消す */
	show_namebox(false);
	show_msgbox(false);
	show_click(false);

	return true;
}

/* 移動方法を取得する */
static bool get_move(const char *move_s)
{
	if (strcmp(move_s, "horizontal") == 0 || strcmp(move_s, "h") == 0 ||
	    strcmp(move_s, U8("横")) == 0) {
		move = SHAKE_MOVE_HORIZONTAL;
		return true;
	} else if (strcmp(move_s, "vertical") == 0 ||
		   strcmp(move_s, "v") == 0 || strcmp(move_s, U8("縦")) == 0) {
		move = SHAKE_MOVE_VERTICAL;
		return true;
	} else {
		/* スクリプト実行エラー */
		log_script_shake_move(move_s);
		log_script_exec_footer();
		return false;
	}
}

/* 描画を行う */
static void draw(void)
{
	float lap, t, s;

	/* 経過時間を取得する */
	lap = (float)get_lap_timer_millisec(&sw) / 1000.0f;
	if (lap >= span)
		lap = span;

	/* 加速を処理する */
	t = lap / span * 2 * PI;
	s = (float)amount * sinf((float)times * t);

	/* 進捗を設定する */
	if (is_in_command_repetition()) {
		/*
		 * 経過時間が一定値を超えた場合と、
		 * 入力によりスキップされた場合
		 */
		if ((lap >= span)
		    ||
		    (!is_non_interruptible() &&
		     (is_control_pressed || is_return_pressed ||
		      is_left_clicked))) {
			/* 繰り返し動作を終了する */
			stop_command_repetition();

			/* 画面揺らしモードを終了する */
			finish_fade();
		} else {
			/* 進捗を設定する */
			if (move == SHAKE_MOVE_HORIZONTAL)
				set_shake_offset((int)s, 0);
			else if (move == SHAKE_MOVE_VERTICAL)
				set_shake_offset(0, (int)s);
		}
	}

	/* ステージを描画する */
	if (is_in_command_repetition())
		draw_fade();
	else
		render_stage();

	/* 折りたたみシステムメニューを描画する */
	if (conf_sysmenu_transition && !is_non_interruptible())
		render_collapsed_sysmenu(false);
}

/* 終了処理を行う */
static bool cleanup(void)
{
	/* 次のコマンドに移動する */
	if (!move_to_next_command())
		return false;

	return true;
}
