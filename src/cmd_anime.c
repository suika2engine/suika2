/* -*- coding: utf-8; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Suika 2
 * Copyright (C) 2001-2023, TABATA Keiichi. All rights reserved.
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
static void draw(int *x, int *y, int *w, int *h);
static bool cleanup(void);

/*
 * animeコマンド
 */
bool anime_command(int *x, int *y, int *w, int *h)
{
	if (!is_in_command_repetition())
		if (!init())
			return false;

	draw(x, y, w, h);

	if (!is_in_command_repetition())
		if (!cleanup())
			return false;

	*x = 0;
	*y = 0;
	*w = conf_window_width;
	*h = conf_window_height;

	return true;
}

/* 初期化処理を行う */
static bool init(void)
{
	const char *fname;
	const char *spec;

	/* パラメータを取得する */
	fname = get_string_param(ANIME_PARAM_FILE);
	spec = get_string_param(ANIME_PARAM_SPEC);

	/* アニメーションファイルをロードする */
	if (!load_anime_from_file(fname)) {
		log_script_exec_footer();
		return false;
	}

	/* 修飾を処理する */
	is_async = false;
	is_sysmenu = true;
	if (strstr(spec, "async") != NULL)
		is_async = true;
	if (strstr(spec, "nosysmenu") != NULL)
		is_sysmenu = false;
	if (strstr(spec, "showmsgbox") == NULL) {
		show_namebox(false);
		show_msgbox(false);
		show_click(false);
	}

	/* 繰り返し動作を開始する */
	start_command_repetition();

	return true;
}

/* 描画を行う */
static void draw(int *x, int *y, int *w, int *h)
{
	draw_stage();

	/* システムメニューを表示する場合 */
	if (is_sysmenu) {
		/* 折りたたみシステムメニューを描画する */
		if (conf_sysmenu_transition && !is_non_interruptible())
			draw_stage_collapsed_sysmenu(false, x, y, w, h);
	}

	/* すべてのアニメーションが完了した場合 */
	if (!is_anime_running()) {
		/* 繰り返し動作を終了する */
		stop_command_repetition();
	}
}

/* 終了処理を行う */
static bool cleanup(void)
{
	/* 次のコマンドに移動する */
	if (!move_to_next_command())
		return false;

	return true;
}
