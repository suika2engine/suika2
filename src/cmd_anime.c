/* -*- coding: utf-8; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Suika2
 * Copyright (C) 2001-2023, Keiichi Tabata. All rights reserved.
 */

/*
 * [Changes]
 *  - 2023/08/20 作成
 */

#include "suika.h"

static bool is_async;
static bool is_sysmenu;

/*
 * 前方参照
 */
static bool init(void);
static void draw(void);
static bool cleanup(void);

/*
 * animeコマンド
 */
bool anime_command(void)
{
	if (!is_in_command_repetition())
		if (!init())
			return false;

	draw();

	if (!is_in_command_repetition())
		if (!cleanup())
			return false;

	return true;
}

/* 初期化処理を行う */
static bool init(void)
{
	const char *fname;
	const char *spec;
	int i;
	int reg_index;

	/* パラメータを取得する */
	fname = get_string_param(ANIME_PARAM_FILE);
	spec = get_string_param(ANIME_PARAM_SPEC);

	/* 修飾を処理する */
	is_async = false;
	is_sysmenu = true;
	reg_index = -1;
	if (strstr(spec, "async") != NULL)
		is_async = true;
	if (strstr(spec, "nosysmenu") != NULL)
		is_sysmenu = false;
	if (!is_async && strstr(spec, "showmsgbox") == NULL) {
		show_namebox(false);
		show_msgbox(false);
		show_click(false);
	}
	if (strstr(spec, "forcemsgbox") != NULL)
		show_msgbox(true);
	if (strstr(spec, "forcenamebox") != NULL)
		show_namebox(true);
	if (strstr(spec, "reg00") != NULL)
		reg_index = 0;
	if (strstr(spec, "reg01") != NULL)
		reg_index = 1;
	if (strstr(spec, "reg02") != NULL)
		reg_index = 2;
	if (strstr(spec, "reg03") != NULL)
		reg_index = 3;
	if (strstr(spec, "reg04") != NULL)
		reg_index = 4;
	if (strstr(spec, "reg05") != NULL)
		reg_index = 5;
	if (strstr(spec, "reg06") != NULL)
		reg_index = 6;
	if (strstr(spec, "reg07") != NULL)
		reg_index = 7;

	/* アニメファイル名を処理する */
	if (strcmp(fname, "finish-all") == 0) {
		/*
		 * アニメファイルはロードしない
		 *  - 全レイヤのアニメ完了を待つ
		 */
	} else if (strcmp(fname, "stop-all") == 0) {
		/*
		 * アニメファイルはロードしない
		 *  - 全レイヤのアニメを強制的に完了する
		 */
		for (i = 0; i < STAGE_LAYERS; i++)
			finish_layer_anime(i);
	} else if (strcmp(fname, "unregister") == 0) {
		unregister_anime(reg_index);
	} else {
		/* アニメファイルをロードする */
		if (!load_anime_from_file(fname, reg_index)) {
			log_script_exec_footer();
			return false;
		}
	}

	/* 繰り返し動作を開始する */
	if (!is_async)
		start_command_repetition();

	return true;
}

/* 描画を行う */
static void draw(void)
{
	render_stage();

	/* システムメニューを表示する場合 */
	if (is_sysmenu) {
		/* 折りたたみシステムメニューを描画する */
		if (conf_sysmenu_transition && !is_non_interruptible())
			render_collapsed_sysmenu(false);
	}

	/* 同期処理の場合 */
	if (!is_async) {
		/* すべてのアニメーションが完了した場合 */
		if (!is_anime_running()) {
			/* 繰り返し動作を終了する */
			stop_command_repetition();
		}
	}
}

/* 終了処理を行う */
static bool cleanup(void)
{
	int i;

	/* 同期処理の場合、アニメシーケンスをクリアする */
	if (!is_async) {
		for (i = 0; i < STAGE_LAYERS; i++)
			clear_anime_sequence(i);
	}

	/* 次のコマンドに移動する */
	if (!move_to_next_command())
		return false;

	return true;
}
