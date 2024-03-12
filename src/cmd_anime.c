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

static const char *file;

#define REG_SIZE 8

static bool opt_async;
static bool opt_nosysmenu;
static bool opt_showmsgbox;
static bool opt_forcemsgbox;
static bool opt_forcenamebox;
static bool opt_all;
static bool opt_rotate;
static bool opt_scale;
static bool opt_reg[REG_SIZE];

struct option_table {
	const char *name;
	bool *flag;
} option_table[] = {
	{"async", &opt_async},
	{"nosysmenu", &opt_nosysmenu},
	{"showmsgbox", &opt_showmsgbox},
	{"forcemsgbox", &opt_forcemsgbox},
	{"forcenamebox", &opt_forcenamebox},
	{"all", &opt_all},
	{"rotate", &opt_rotate},
	{"scale", &opt_scale},
	{"reg00", &opt_reg[0]},
	{"reg01", &opt_reg[1]},
	{"reg02", &opt_reg[2]},
	{"reg03", &opt_reg[3]},
	{"reg04", &opt_reg[4]},
	{"reg05", &opt_reg[5]},
	{"reg06", &opt_reg[6]},
	{"reg07", &opt_reg[7]},
};

#define OPT_COUNT	((int)(sizeof(option_table) / sizeof(struct option_table)))

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
	const char *spec;
	int i, reg_index = -1;

	/* パラメータを取得する */
	file = get_string_param(ANIME_PARAM_FILE);
	spec = get_string_param(ANIME_PARAM_SPEC);

	/* オプションを初期化する */
	for (i = 0; i < OPT_COUNT; i++) {
		if (strstr(spec, option_table[i].name) != NULL)
			*(option_table[i].flag) = true;
		else
			*(option_table[i].flag) = false;
	}
	for (i = 0; i < REG_SIZE; i++) {
		if (opt_reg[i]) {
			reg_index = i;
			break;
		}
	}

	/* オプションの処理を行う */
	if (!opt_async && !opt_showmsgbox) {
		show_namebox(false);
		show_msgbox(false);
		show_click(false);
	}
	if (opt_forcemsgbox)
		show_msgbox(true);
	if (opt_forcenamebox)
		show_namebox(true);

	/* 特殊なファイル名を処理する */
	if (strcmp(file, "reset") == 0) {
		/* 全レイヤのアニメを強制的に完了する */
		for (i = 0; i < STAGE_LAYERS; i++) {
			finish_layer_anime(i);
			set_layer_scale(i, 1.0f, 1.0f);
			set_layer_rotate(i, 0);
		}
	} else if (strcmp(file, "finish-all") == 0) {
		/* 全レイヤのアニメ完了を待つ */
		opt_async = false;
	} else if (strcmp(file, "stop-all") == 0) {
		/* 全レイヤのアニメを強制的に完了する */
		for (i = 0; i < STAGE_LAYERS; i++)
			finish_layer_anime(i);
	} else if (strcmp(file, "unregister") == 0) {
		/* 登録解除を行う */
		unregister_anime(reg_index);
	} else {
		/* アニメファイルをロードする */
		if (!load_anime_from_file(file, reg_index)) {
			log_script_exec_footer();
			return false;
		}
	}

	/* 繰り返し動作を開始する */
	if (!opt_async)
		start_command_repetition();

	return true;
}

/* 描画を行う */
static void draw(void)
{
	render_stage();

	/* システムメニューを表示する場合 */
	if (!opt_nosysmenu) {
		/* 折りたたみシステムメニューを描画する */
		if (conf_sysmenu_transition && !is_non_interruptible())
			render_collapsed_sysmenu(false);
	}

	/* 同期処理の場合 */
	if (!opt_async) {
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
	if (!opt_async) {
		for (i = 0; i < STAGE_LAYERS; i++)
			clear_anime_sequence(i);
	}

	/* 次のコマンドに移動する */
	if (!move_to_next_command())
		return false;

	return true;
}
