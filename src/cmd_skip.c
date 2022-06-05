/* -*- coding: utf-8-with-signature; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Suika 2
 * Copyright (C) 2001-2022, TABATA Keiichi. All rights reserved.
 */

/*
 * [Changes]
 *  - 2022/06/05 作成
 */

#include "suika.h"

/*
 * スキップ許可コマンド
 */
bool skip_command(void)
{
	const char *param;

	param = get_string_param(SKIP_PARAM_MODE);

	if (strcmp(param, "disable") == 0) {
		set_non_interruptible(true);
	} else if (strcmp(param, "enable") == 0) {
		set_non_interruptible(false);
	} else {
		log_script_enable_disable(param);
		log_script_exec_footer();
		return false;
	}

	return move_to_next_command();
}
