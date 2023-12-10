/* -*- coding: utf-8; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Suika 2
 * Copyright (C) 2001-2023, TABATA Keiichi. All rights reserved.
 */

/*
 * [Changes]
 *  - 2022/07/29 作成
 *  - 2023/01/06 日本語の指定に対応
 */

#include "suika.h"

static bool init(void);
static bool cleanup(void);

/*
 * GUIコマンド
 */
bool gui_command(void)
{
	/* 最初のフレームの場合 */
	if (!is_in_command_repetition()) {
		/* 初期化を行う */
		if (!init())
			return false;

		/* 最初のフレームの描画を行う */
		if (!run_gui_mode())
			return false;

		/* 次のフレーム以降はmain.cでrun_gui_mode()が呼ばれる */
		return true;
	}

	/*
	 * GUIモードが終了した際にここに到達する
	 *  - タイトルへ戻るが選択された場合、ここに到達しない
	 */
	if (!cleanup())
		return false;

	return true;
}

/* 初期化を行う */
static bool init(void)
{
	const char *file, *opt;
	bool opt_cancel;
	bool opt_nofadein;
	bool opt_nofadeout;
	bool opt_msgbox;

	/* GUIファイル名を取得する */
	file = get_string_param(GUI_PARAM_FILE);

	/* オプションを取得する */
	opt_cancel = false;
	opt_nofadein = false;
	opt_nofadeout = false;
	opt_msgbox = false;
	opt = get_string_param(GUI_PARAM_OPTIONS);
	if (strstr(opt, "cancel") != NULL ||
	    strstr(opt, U8("キャンセル許可")) != NULL)
		opt_cancel = true;
	if (strstr(opt, "nofadein") != NULL ||
	    strstr(opt, "フェードインなし") != NULL)
		opt_nofadein = true;
	if (strstr(opt, "nofadeout") != NULL ||
	    strstr(opt, "フェードアウトなし") != NULL)
		opt_nofadeout = true;
	if (strstr(opt, "msgbox") != NULL ||
	    strstr(opt, U8("メッセージボックスあり")) != NULL)
		opt_msgbox = true;

	/* セーブに備えてサムネイルを作成する */
	draw_stage_to_thumb();

	/* GUIファイルと指定された画像の読み込みを行う */
	if (!prepare_gui_mode(file, false)) {
		log_script_exec_footer();
		return false;
	}
	set_gui_options(opt_cancel, opt_nofadein, opt_nofadeout);

	/* 背景以外を消す */
	if (!opt_msgbox) {
		show_namebox(false);
		show_msgbox(false);
	}

	/* 繰り返し処理を開始する */
	start_command_repetition();

	/* 繰り返し処理中のままGUIモードに移行する */
	start_gui_mode();

	return true;
}

/* 終了処理を行う */
static bool cleanup(void)
{
	const char *label;
	bool ret;

	ret = true;

	/* 繰り返し処理を終了する */
	stop_command_repetition();

	/* ラベルジャンプボタンが押下された場合 */
	label = get_gui_result_label();
	if (label != NULL) {
		ret = move_to_label(label);
		cleanup_gui();
		return ret;
	}

	/* 終了ボタンが押下された場合 */
	if (is_gui_result_exit()) {
		cleanup_gui();
		return false;
	}

	/*
	 * キャンセルボタンが押下された場合と、
	 * 右クリックでキャンセルされた場合で、
	 * セーブされていなければ、次のコマンドへ移動する
	 */
	if (!is_gui_saved())
		ret = move_to_next_command();

	cleanup_gui();
	return ret;
}
