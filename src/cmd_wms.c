/* -*- coding: utf-8; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Suika2
 * Copyright (C) 2001-2022, Keiichi Tabata. All rights reserved.
 */

/*
 * [Changes]
 *  - 2022/12/25 Created.
 */

#include "suika.h"
#include "wms.h"

static char *script;
static struct wms_runtime *rt;

static bool init(void);
static bool run(void);
static bool cleanup(void);

bool register_s2_functions(struct wms_runtime *rt);

/*
 * wmsコマンド
 */
bool wms_command(void)
{
	if (!is_in_command_repetition())
		if (!init())
			return false;

	if (!run())
		return false;

	/*
	 * TODO:
	 * 別途作成するリピートフラグが立っていればset_command_repetition()を呼ぶ。
	 * その場合、ここでリターンする。
	 * 現状では、command repetitionは実行されないこととする。
	 */
	assert(!is_in_command_repetition());

	if (!cleanup())
		return false;

	return true;
}

static bool init(void)
{
	struct rfile *rf;
	const char *file;
	size_t len;

	/* 引数を取得する */
	file = get_string_param(WMS_PARAM_FILE);

	/* スクリプトファイルを開いてすべて読み込む */
	rf = open_rfile(WMS_DIR, file, false);
	if (rf == NULL)
		return false;
	len = get_rfile_size(rf);
	script = malloc(len + 1);
	if (script == NULL) {
		log_memory();
		return false;
	}
	if (read_rfile(rf, script, len) != len) {
		log_file_read(WMS_DIR, file);
		return false;
	}
	close_rfile(rf);
	script[len] = '\0';

	/* パースしてランタイムを作成する */
	rt = wms_make_runtime(script);
	if (rt == NULL) {
		log_wms_syntax_error(file, wms_get_parse_error_line(),
				     wms_get_parse_error_column());
		return false;
	}

	/* ランタイムにFFI関数を登録する */
	if (!register_s2_functions(rt))
		return false;

	return true;
}

static bool run(void)
{
	/* WMSを実行する */
	if (!wms_run(rt)) {
		log_wms_runtime_error(get_string_param(WMS_PARAM_FILE),
				      wms_get_runtime_error_line(rt),
				      wms_get_runtime_error_message(rt));
		return false;
	}

	return true;
}

static bool cleanup(void)
{
	if (rt != NULL) {
		wms_free_runtime(rt);
		rt = NULL;
	}
	if (script != NULL) {
		free(script);
		script = NULL;
	}

	/* 次のコマンドに移動する */
	return move_to_next_command();
}
