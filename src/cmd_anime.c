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
static bool opt_layer_all;
static bool opt_layer_tbl[STAGE_LAYERS];
static bool opt_xy;
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
	{"layer-all", &opt_layer_all},
	{"layer-bg", &opt_layer_tbl[LAYER_BG]},
	{"layer-bg2", &opt_layer_tbl[LAYER_BG2]},
	{"layer-effect5", &opt_layer_tbl[LAYER_EFFECT5]},
	{"layer-effect6", &opt_layer_tbl[LAYER_EFFECT5]},
	{"layer-effect7", &opt_layer_tbl[LAYER_EFFECT5]},
	{"layer-effect8", &opt_layer_tbl[LAYER_EFFECT5]},
	{"layer-chb", &opt_layer_tbl[LAYER_CHB]},
	{"layer-chb-eye", &opt_layer_tbl[LAYER_CHB_EYE]},
	{"layer-chl", &opt_layer_tbl[LAYER_CHL]},
	{"layer-chl-eye", &opt_layer_tbl[LAYER_CHL_EYE]},
	{"layer-chlc", &opt_layer_tbl[LAYER_CHL]},
	{"layer-chlc-eye", &opt_layer_tbl[LAYER_CHL_EYE]},
	{"layer-chr", &opt_layer_tbl[LAYER_CHR]},
	{"layer-chr-eye", &opt_layer_tbl[LAYER_CHR_EYE]},
	{"layer-chrc", &opt_layer_tbl[LAYER_CHRC]},
	{"layer-chrc-eye", &opt_layer_tbl[LAYER_CHRC_EYE]},
	{"layer-chc", &opt_layer_tbl[LAYER_CHC]},
	{"layer-chc-eye", &opt_layer_tbl[LAYER_CHC_EYE]},
	{"layer-effect1", &opt_layer_tbl[LAYER_EFFECT1]},
	{"layer-effect2", &opt_layer_tbl[LAYER_EFFECT2]},
	{"layer-effect3", &opt_layer_tbl[LAYER_EFFECT3]},
	{"layer-effect4", &opt_layer_tbl[LAYER_EFFECT4]},
	{"layer-msg", &opt_layer_tbl[LAYER_MSG]},
	{"layer-name", &opt_layer_tbl[LAYER_NAME]},
	{"layer-chf", &opt_layer_tbl[LAYER_CHF]},
	{"layer-chf-eye", &opt_layer_tbl[LAYER_CHF_EYE]},
	{"layer-click", &opt_layer_tbl[LAYER_CLICK]},
	{"layer-auto", &opt_layer_tbl[LAYER_AUTO]},
	{"layer-skip", &opt_layer_tbl[LAYER_SKIP]},
	{"layer-text1", &opt_layer_tbl[LAYER_TEXT1]},
	{"layer-text2", &opt_layer_tbl[LAYER_TEXT2]},
	{"layer-text3", &opt_layer_tbl[LAYER_TEXT3]},
	{"layer-text4", &opt_layer_tbl[LAYER_TEXT4]},
	{"layer-text5", &opt_layer_tbl[LAYER_TEXT5]},
	{"layer-text6", &opt_layer_tbl[LAYER_TEXT6]},
	{"layer-text7", &opt_layer_tbl[LAYER_TEXT7]},
	{"layer-text8", &opt_layer_tbl[LAYER_TEXT8]},
	{"xy", &opt_xy},
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
static void do_reset(void);
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
		do_reset();
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

static void do_reset(void)
{
	int i;

	for (i = 0; i < STAGE_LAYERS; i++) {
		if (opt_layer_tbl[i] || opt_layer_all) {
			if (opt_xy)
				set_layer_position(i, 0, 0);
			if (opt_scale)
				set_layer_scale(i, 1.0f, 1.0f);
			if (opt_rotate)
				set_layer_rotate(i, 0);
		}
	}
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
