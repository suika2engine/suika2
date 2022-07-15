/* -*- coding: utf-8-with-signature; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Suika 2
 * Copyright (C) 2001-2022, TABATA Keiichi. All rights reserved.
 */

/*
 * [Changes]
 *  - 2022/07/04 作成
 */

#include "suika.h"

/*
 * 章題設定コマンド
 */
bool chapter_command(void)
{
	const char *param;

	param = get_string_param(CHAPTER_PARAM_NAME);

	/* 章題を設定する */
	if (!set_chapter_name(param)) {
		log_script_exec_footer();
		return false;
	}

	/* ウィンドウタイトルを更新する */
	update_window_title();

	return move_to_next_command();
}
