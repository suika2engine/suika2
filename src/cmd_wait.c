/* -*- coding: utf-8; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Suika 2
 * Copyright (C) 2001-2016, TABATA Keiichi. All rights reserved.
 */

/*
 * [Changes]
 *  - 2016/06/22 作成
 */

#include "suika.h"

static float span;
static stop_watch_t sw;

/*
 * waitコマンド
 */
bool wait_command(void)
{
	/* 初期化処理を行う */
	if (!is_in_command_repetition()) {
		start_command_repetition();

		/* パラメータを取得する */
		span = get_float_param(WAIT_PARAM_SPAN);

		/* 時間の計測を開始する */
		reset_stop_watch(&sw);
	}

	/* 描画を行う(GPU用) */
	draw_stage_keep();

	/* 時間が経過した場合か、入力があった場合 */
	if ((float)get_stop_watch_lap(&sw) / 1000.0f >= span ||
	    (is_skip_mode() && !is_non_interruptible()) ||
	    (!is_auto_mode() && !is_non_interruptible() &&
	     (is_control_pressed || is_return_pressed || is_down_pressed ||
	      is_left_clicked))) {
		stop_command_repetition();

		/* 次のコマンドへ移動する */
		return move_to_next_command();
	}

	/* waitコマンドを継続する */
	return true;
}
