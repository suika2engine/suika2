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

/*
 * loadコマンド
 */
bool load_command(void)
{
	char *file;

	/* パラメータからファイル名を取得する */
	file = strdup(get_string_param(LOAD_PARAM_FILE));
	if (file == NULL) {
		log_memory();
		return false;
	}

	/* 既読フラグをセーブする */
	save_seen();

	/* スクリプトをロードする */
	if (!load_script(file)) {
		free(file);
		return false;
	}
	free(file);

	return true;
}
