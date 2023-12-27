/* -*- coding: utf-8; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Suika2
 * Copyright (C) 2001-2022, Keiichi Tabata. All rights reserved.
 */

/*
 * [Changes]
 *  - 2016/06/09 作成
 */

#include "suika.h"

/*
 * bgmコマンド
 *  - 2022/5/10 onceオプションを追加
 */
bool bgm_command(void)
{
	struct wave *w;
	const char *fname;
	const char *once;
	bool loop, stop;

	/* パラメータを取得する */
	fname = get_string_param(BGM_PARAM_FILE);
	once = get_string_param(BGM_PARAM_ONCE);

	/* 停止の指示かをチェックする */
	if (strcmp(fname, "stop") == 0 || strcmp(fname, U8("停止")) == 0)
		stop = true;
	else
		stop = false;

	/* ループしないかをチェックする */
	if (strcmp(once, "once") == 0)
		loop = false;
	else
		loop = true;

	/* 停止の指示でない場合 */
	if (!stop) {
		/* PCMストリームをオープンする */
		w = create_wave_from_file(BGM_DIR, fname, loop);
		if (w == NULL) {
			log_script_exec_footer();
			return false;
		}
	} else {
		w = NULL;
	}

	/* 再生を開始する */
	set_mixer_input(BGM_STREAM, w);

	/* BGMのファイル名を設定する */
	if (!set_bgm_file_name(stop ? NULL : fname))
		return false;

	/* 次のコマンドへ移動する */
	return move_to_next_command();
}
