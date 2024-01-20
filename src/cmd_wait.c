/* -*- coding: utf-8; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Suika2
 * Copyright (C) 2001-2016, Keiichi Tabata. All rights reserved.
 */

/*
 * [Changes]
 *  - 2016/06/22 作成
 */

#include "suika.h"

static float span;
static uint64_t sw;
static bool show_sysmenu;

/*
 * waitコマンド
 */
bool wait_command(void)
{
	const char *opt;

	/* 初期化処理を行う */
	if (!is_in_command_repetition()) {
		start_command_repetition();

		/* パラメータを取得する */
		span = get_float_param(WAIT_PARAM_SPAN);
		opt = get_string_param(WAIT_PARAM_OPT);

		show_sysmenu = false;
		if (strstr(opt, "showsysmenu") != NULL)
			show_sysmenu = true;
		if (strstr(opt, "showmsgbox") != NULL)
			show_msgbox(true);
		if (strstr(opt, "hidemsgbox") != NULL)
			show_msgbox(false);
		if (strstr(opt, "shownamebox") != NULL)
			show_namebox(true);
		if (strstr(opt, "hidenamebox") != NULL)
			show_namebox(false);

		/* 時間の計測を開始する */
		reset_lap_timer(&sw);
	}

	/* 描画を行う */
	render_stage();
	if (show_sysmenu)
		render_collapsed_sysmenu(false);

	/* 時間が経過した場合か、入力があった場合 */
	if ((float)get_lap_timer_millisec(&sw) / 1000.0f >= span ||
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
