/* -*- coding: utf-8-with-signature; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Suika 2
 * Copyright (C) 2001-2018, TABATA Keiichi. All rights reserved.
 */

/*
 * [Changes]
 *  - 2018/07/22 作成
 */

#include "suika.h"

/*
 * returnコマンド
 */
bool return_command(void)
{
	int rp;

	/* リターンポイントを取得する */
	rp = pop_return_point();

	/* リターンポイントが無効な場合、エラーとする */
	if (rp == -1) {
		/* エラーを出力する */
		log_script_return_error();
		log_script_exec_footer();
		return false;
	}

	/* リターンポイントの次の行に復帰する */
	if (!move_to_command_index(rp + 1)) {
		/* エラーを出力する */
		log_script_return_error();
		log_script_exec_footer();
		return false;
	}

	return true;
}
