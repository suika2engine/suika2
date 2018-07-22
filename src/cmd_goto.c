/* -*- coding: utf-8-with-signature; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Suika 2
 * Copyright (C) 2001-2016, TABATA Keiichi. All rights reserved.
 */

/*
 * [Changes]
 *  - 2016/06/27 作成
 */

#include "suika.h"

/* ロード画面を呼び出す特殊なラベル */
#define LOAD_LABEL	"$LOAD"

/*
 * gotoコマンド
 */
bool goto_command(void)
{
	const char *label;

	/* パラメータからラベルを取得する */
	label = get_string_param(GOTO_PARAM_LABEL);

	/* ロード画面への遷移を処理する */
	if (strcmp(label, LOAD_LABEL) == 0) {
		/* ロード専用モードでセーブ画面を開始する */
		start_save_mode(true);

		/* キャンセルされた場合のために次のコマンドへ移動しておく */
		return move_to_next_command();
	}

	/* ラベルの次のコマンドへ移動する */
	if (!move_to_label(label))
		return false;

	return true;
}
