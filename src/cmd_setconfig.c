/* -*- coding: utf-8; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Suika2
 * Copyright (C) 2001-2023, Keiichi Tabata. All rights reserved.
 */

/*
 * [Changes]
 *  - 2023/08/27 作成
 */

#include "suika.h"

/*
 * コンフィグ変更コマンド
 */
bool setconfig_command(void)
{
	const char *key, *val;

	key = get_string_param(SETCONFIG_PARAM_KEY);
	val = get_string_param(SETCONFIG_PARAM_VALUE);

	/* 特殊なキーを処理する */
	if (strcmp(key, "update-msgbox-and-namebox") == 0) {
		/* 互換性のため */
	} else {
		/* コンフィグを書き換える */
		if (!overwrite_config(key, val)) {
			log_script_config_not_found(key);
			log_script_exec_footer();
			return false;
		}
	}

	/* 次のコマンドに移動する */
	return move_to_next_command();
}
