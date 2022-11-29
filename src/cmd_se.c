/* -*- coding: utf-8; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Suika 2
 * Copyright (C) 2001-2021, TABATA Keiichi. All rights reserved.
 */

/*
 * [Changes]
 *  - 2016/07/02 作成
 *  - 2021/06/06 ボイスストリームでの再生に対応
 */

#include "suika.h"

/*
 * seコマンド
 *  - 2022/5/10 loopオプションを追加
 */
bool se_command(void)
{
	struct wave *w;
	const char *fname;
	const char *option;
	int stream;
	bool loop;

	/* パラメータを取得する */
	fname = get_string_param(SE_PARAM_FILE);
	option = get_string_param(SE_PARAM_OPTION);

	/*
	 * 1. voice指示の有無を確認する
	 *  - マスターボリュームのフィードバック再生を行う際、テキスト表示なし
	 *    でボイスを再生できるようにするための指示
	 * 2. ループ再生するかを確認する
	 */
	if (strcmp(option, "voice") == 0) {
		stream = VOICE_STREAM;
		loop = false;
	} else if (strcmp(option, "loop") == 0) {
		stream = SE_STREAM;
		loop = true;
	} else {
		stream = SE_STREAM;
		loop = false;
	}

	/* 停止の指示でない場合 */
	w = NULL;
	if (strcmp(fname, "stop") != 0) {
		/* PCMストリームをオープンする */
		w = create_wave_from_file(SE_DIR, fname, loop);
		if (w == NULL) {
			log_script_exec_footer();
			return false;
		}
	}

	/* 再生を開始する */
	set_mixer_input(stream, w);

	/* 次のコマンドへ移動する */
	return move_to_next_command();
}
