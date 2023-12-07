/* -*- coding: utf-8; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Suika 2
 * Copyright (C) 2001-2016, TABATA Keiichi. All rights reserved.
 */

/*
 * [Changes]
 *  - 2016/06/21 作成
 */

#include "suika.h"

/* オートモードのとき待ち時間 */
#define AUTO_MODE_WAIT	(2000)

static stop_watch_t sw;

/*
 * clickコマンド
 */
bool click_command(void)
{
	/* 初期化処理を行う */
	if (!is_in_command_repetition()) {
		/* メッセージボックスを非表示にする */
		show_msgbox(false);
		show_namebox(false);

		/* スキップモードを終了する */
		if (is_skip_mode()) {
			stop_skip_mode();
			show_skipmode_banner(false);
		}

		/* 時間の計測を開始する */
		reset_stop_watch(&sw);

		/* 繰り返し動作を開始する */
		start_command_repetition();
	}

	/*
	 * 入力があった場合か、オートモード中に一定時間経過したら、
	 * 繰り返し動作を終了する
	 */
	if ((!is_auto_mode() &&
	     (is_control_pressed || is_return_pressed || is_down_pressed ||
	      is_left_clicked))
	    ||
	    (is_auto_mode() &&
	      (float)get_stop_watch_lap(&sw) >= AUTO_MODE_WAIT))
		stop_command_repetition();

	/* ステージの描画を行う */
	draw_stage();

	/* 後処理を行う */
	if (!is_in_command_repetition()) {
		/* 次のコマンドへ移動する */
		return move_to_next_command();
	}

	/* コマンドの実行を継続する */
	return true;
}
