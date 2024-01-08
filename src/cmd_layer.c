/* -*- coding: utf-8; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Suika2
 * Copyright (C) 2001-2023, Keiichi Tabata. All rights reserved.
 */

/*
 * [Changes]
 *  - 2023/11/27 作成
 */

#include "suika.h"

/* レイヤー名とレイヤーのインデックスのマップ */
struct layer_name_map {
	const char *name;
	int index;
};
static struct layer_name_map layer_name_map[] = {
	{"bg", LAYER_BG}, {U8("背景"), LAYER_BG},
	{"bg2", LAYER_BG2},
	{"effect5", LAYER_EFFECT5},
	{"effect6", LAYER_EFFECT6},
	{"effect7", LAYER_EFFECT7},
	{"effect8", LAYER_EFFECT8},
	{"chb", LAYER_CHB}, {U8("背面キャラ"), LAYER_CHB},
	{"chl", LAYER_CHL}, {U8("左キャラ"), LAYER_CHL},
	{"chlc", LAYER_CHL}, {U8("左中キャラ"), LAYER_CHLC},
	{"chr", LAYER_CHR}, {U8("右キャラ"), LAYER_CHR},
	{"chrc", LAYER_CHRC}, {U8("右中キャラ"), LAYER_CHRC},
	{"chc", LAYER_CHC}, {U8("中央キャラ"), LAYER_CHC},
	{"effect1", LAYER_EFFECT1},
	{"effect2", LAYER_EFFECT2},
	{"effect3", LAYER_EFFECT3},
	{"effect4", LAYER_EFFECT4},
	{"text1", LAYER_TEXT1},
	{"text2", LAYER_TEXT2},
	{"text3", LAYER_TEXT3},
	{"text4", LAYER_TEXT4},
	{"text5", LAYER_TEXT5},
	{"text6", LAYER_TEXT6},
	{"text7", LAYER_TEXT7},
	{"text8", LAYER_TEXT8},
};

/* 前方参照 */
static int name_to_layer(const char *name);

/*
 * レイヤコマンド
 */
bool layer_command(void)
{
	struct image *img;
	const char *name;
	const char *file;
	const char *dir;
	int x, y, a, layer;

	/* パラメータを取得する */
	name = get_string_param(LAYER_PARAM_NAME);
	file = get_string_param(LAYER_PARAM_FILE);
	x = get_int_param(LAYER_PARAM_X);
	y = get_int_param(LAYER_PARAM_Y);
	a = get_int_param(LAYER_PARAM_A);

	/* キャラの消去が指定されているかチェックする */
	if (strcmp(file, "none") == 0 || strcmp(file, U8("消去")) == 0)
		file = NULL;

	/* レイヤ名からレイヤインデックスを求める */
	layer = name_to_layer(name);
	if (layer == -1) {
		log_invalid_layer_name(name);
		return false;
	}

	/* イメージが指定された場合 */
	if (file != NULL) {
		switch (layer) {
		case LAYER_BG:
		case LAYER_BG2:
			dir = BG_DIR;
			break;
		case LAYER_CHB:
		case LAYER_CHL:
		case LAYER_CHLC:
		case LAYER_CHR:
		case LAYER_CHRC:
		case LAYER_CHC:
		case LAYER_CHF:
			dir = CH_DIR;
			break;
		default:
			dir = CG_DIR;
			break;
		}

		/* イメージを読み込む */
		img = create_image_from_file(dir, file);
		if (img == NULL) {
			log_script_exec_footer();
			return false;
		}
	} else {
		/* イメージが指定されなかった場合(消す) */
		img = NULL;
	}

	/* レイヤを設定する */
	set_layer_file_name(layer, file);
	set_layer_image(layer, img);
	set_layer_position(layer, x, y);
	set_layer_alpha(layer, a);
	set_layer_scale(layer, 1.0f, 1.0f);

	/* 次のコマンドへ */
	if (!move_to_next_command())
		return false;

	return true;
}

static int name_to_layer(const char *name)
{
	int i;

	for (i = 0;
	     i < (int)(sizeof(layer_name_map) / sizeof(struct layer_name_map));
	     i++) {
		if (strcmp(layer_name_map[i].name, name) == 0)
			return layer_name_map[i].index;
	}
	return -1;
}
