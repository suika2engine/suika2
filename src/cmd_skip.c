/* -*- coding: utf-8; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Suika2
 * Copyright (C) 2001-2023, Keiichi Tabata. All rights reserved.
 */

/*
 * [Changes]
 *  - 2022/06/05 作成
 *  - 2023/01/06 オートモードとスキップモードを終了するよう変更
 */

#include "suika.h"

/*
 * スキップ許可コマンド
 */
bool skip_command(void)
{
	const char *param;

	param = get_string_param(SKIP_PARAM_MODE);

	if (strcmp(param, "disable") == 0 ||
	    strcmp(param, U8("不許可")) == 0) {
		/* オートモードを終了する */
		if (is_auto_mode()) {
			stop_auto_mode();
			show_automode_banner(false);
		}

		/* スキップモードを終了する */
		if (is_skip_mode()) {
			stop_skip_mode();
			show_skipmode_banner(false);
		}

		/* スキップできないモードにする */
		set_non_interruptible(true);
	} else if (strcmp(param, "enable") == 0 ||
		   strcmp(param, U8("許可")) == 0) {
		/* スキップできるモードにする */
		set_non_interruptible(false);
	} else {
		log_script_enable_disable(param);
		log_script_exec_footer();
		return false;
	}

	return move_to_next_command();
}
