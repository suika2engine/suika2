/* -*- coding: utf-8; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Suika2
 * Copyright (C) 2001-2016, Keiichi Tabata. All rights reserved.
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
	char *file, *label;

	/* パラメータからファイル名を取得する */
	file = strdup(get_string_param(LOAD_PARAM_FILE));
	if (file == NULL) {
		log_memory();
		return false;
	}

	/* パラメータからラベル名を取得する */
	label = strdup(get_string_param(LOAD_PARAM_LABEL));
	if (label == NULL) {
		log_memory();
		return false;
	}

	/* 既読フラグをセーブする */
	save_seen();

#ifdef USE_EDITOR
	save_script();
#endif

	/* スクリプトをロードする */
	if (!load_script(file)) {
		free(file);
		return false;
	}

	/* ラベルへジャンプする */
	if (strcmp(label, "") != 0) {
		if (!move_to_label(label)) {
			free(file);
			free(label);
			return false;
		}
	}

	free(file);
	free(label);

	/* 既読フラグをロードする */
	load_seen();

	return true;
}
