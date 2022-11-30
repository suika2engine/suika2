/* -*- coding: utf-8; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Suika 2
 * Copyright (C) 2001-2018, TABATA Keiichi. All rights reserved.
 */

/*
 * [Changes]
 *  - 2018/07/21 作成
 */

#include "suika.h"

/*
 * gosubコマンド
 */
bool gosub_command(void)
{
	const char *label;

	/* リターンポイントを記録する */
	push_return_point();

	/* パラメータからラベルを取得する */
	label = get_string_param(GOSUB_PARAM_LABEL);

	/* ラベルの次のコマンドへ移動する */
	if (!move_to_label(label))
		return false;

	return true;
}
