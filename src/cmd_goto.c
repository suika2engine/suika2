/* -*- coding: utf-8-with-signature; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Suika 2
 * Copyright (C) 2001-2021, TABATA Keiichi. All rights reserved.
 */

/*
 * [Changes]
 *  - 2016/06/27 作成
 *  - 2021/07/07 セーブ画面追加
 */

#include "suika.h"

/* ロード画面を呼び出す特殊なラベル */
#define LOAD_LABEL	"$LOAD"

/* セーブ画面を呼び出す特殊なラベル */
#define SAVE_LABEL	"$SAVE"

/*
 * gotoコマンド
 */
bool goto_command(bool *cont)
{
	const char *label;

	/* パラメータからラベルを取得する */
	label = get_string_param(GOTO_PARAM_LABEL);

	/* ロード画面への遷移を処理する */
	if (strcmp(label, LOAD_LABEL) == 0) {
		/* コマンドを連続実行しない */
		*cont = false;

		/* 画面を描画する */
		draw_stage();

		/* 最後のコマンドを実行中なら、ロードできない */
		if (is_final_command()) {
			log_script_final_command();
			return false;
		}

		/* セーブロードを有効にする */
		set_save_load(true);

		/* ロード画面を開始する */
		if (!prepare_gui_mode(LOAD_GUI_FILE, true, false)) {
			log_script_exec_footer();
			return false;
		}
		start_gui_mode();

		/* キャンセルされた場合のために次のコマンドへ移動しておく */
		return move_to_next_command();
	}

	/* セーブ画面への遷移を処理する */
	if (strcmp(label, SAVE_LABEL) == 0) {
		/* コマンドを連続実行しない */
		*cont = false;

		/* 画面を描画する */
		draw_stage();

		/* 最後のメッセージを空白にする */
		set_last_message("");

		/* 最後のコマンドを実行中なら、セーブできない */
		if (is_final_command()) {
			log_script_final_command();
			return false;
		}

		/* セーブロードを有効にする */
		set_save_load(true);

		/* セーブ画面を開始する */
		if (!prepare_gui_mode(SAVE_GUI_FILE, true, false)) {
			log_script_exec_footer();
			return false;
		}
		start_gui_mode();

		/*
		 * キャンセルかセーブされた場合のために
		 * 次のコマンドへ移動しておく
		 */
		return move_to_next_command();
	}

	/* コマンドを連続実行する */
	*cont = true;

	/* ラベルの次のコマンドへ移動する */
	if (!move_to_label(label))
		return false;

	return true;
}
