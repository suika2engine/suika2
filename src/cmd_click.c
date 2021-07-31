/* -*- coding: utf-8-with-signature; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Suika 2
 * Copyright (C) 2001-2016, TABATA Keiichi. All rights reserved.
 */

/*
 * [Changes]
 *  - 2016/06/21 作成
 */

#include "suika.h"

/*
 * clickコマンド
 */
bool click_command(void)
{
	/* 入力がない場合はclickコマンドを継続する */
	if (!is_control_pressed && !is_return_pressed && !is_down_pressed &&
	    !is_left_button_pressed) {
		show_msgbox(false);
		return true;
	}

	/* 次のコマンドへ移動する */
	return move_to_next_command();
}
