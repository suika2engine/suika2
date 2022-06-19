/* -*- coding: utf-8-with-signature; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Suika 2
 * Copyright (C) 2001-2022, TABATA Keiichi. All rights reserved.
 */

/*
 * [Changes]
 *  - 2022/5/11 作成
 */

#include "suika.h"

/* 動画をスキップ可能か */
static bool is_skippable;

/* 前方参照 */
static bool init(void);
static void run(void);

/*
 * videoコマンド
 */
bool video_command(void)
{
	/* 最初のフレームの場合、初期化する */
	if (!is_in_command_repetition())
		if (!init())
			return false;

	/* 毎フレーム処理する */
	run();

	/* 再生が終了した場合 */
	if (!is_in_command_repetition()) {
		/* 次のコマンドへ移動する */
		return move_to_next_command();
	}

	/* 再生が継続される場合 */
	return true;
}

/* 初期化する */
static bool init(void)
{
	const char *fname;

	/* パラメータを取得する */
	fname = get_string_param(VIDEO_PARAM_FILE);

	/* 再生しない場合を検出する */
	if ((!is_non_interruptible() && get_seen() && is_control_pressed) ||
	    (is_skip_mode() && get_seen())) {
		/* 次のコマンドへ移動する */
		return move_to_next_command();
	}

	/* スキップモードなら未読なので解除する */
	if (is_skip_mode())
		stop_skip_mode();

	/* クリックでスキップ可能かを決定する */
	is_skippable = get_seen() && !is_non_interruptible();
#ifdef USE_DEBUGGER
	is_skippable = true;
#endif

	/* 既読フラグを設定する */
	set_seen();

	/* 動画を再生する */
	if (!play_video(fname, is_skippable))
		return false;

	/* 繰り返し実行を開始する */
	start_command_repetition();
	return true;
}

/* 各フレームの処理を行う */
static void run(void)
{
	/* スキップ可能なとき、入力があれば再生を終了する */
	if (is_skippable &&
	   (is_left_button_pressed || is_right_button_pressed ||
	    is_control_pressed || is_return_pressed || is_down_pressed)) {
		stop_video();
		stop_command_repetition();
		return;
	}

	/* 再生が末尾まで終了した場合 */
	if (!is_video_playing()) {
		stop_command_repetition();
		return;
	}
}
