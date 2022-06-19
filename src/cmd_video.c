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

/*
 * videoコマンド
 */
bool video_command(void)
{
	const char *fname;
	bool is_skippable;

	/* パラメータを取得する */
	fname = get_string_param(VIDEO_PARAM_FILE);

	/* 最後のコマンドを実行中なら、動画再生できない */
	if (is_final_command()) {
		log_script_final_command();
		return false;
	}

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

	/* 次のコマンドへ移動する */
	return move_to_next_command();
}
