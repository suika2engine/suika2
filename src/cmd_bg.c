/* -*- coding: utf-8; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Suika 2
 * Copyright (C) 2001-2023, TABATA Keiichi. All rights reserved.
 */

/*
 * [Changes]
 *  - 2016/06/09 作成
 *  - 2021/06/05 フェードの種類を追加
 *  - 2021/06/10 マスクつき描画の対応
 *  - 2021/06/10 キャラクタのアルファ値に対応
 *  - 2021/06/16 時計描画の対応
 *  - 2023/09/14 オフセットの追加
 */

#include "suika.h"

/* CGディレクトリ */
#define CG_DIR_PREFIX "cg/"

/* コマンドの経過時刻を表すストップウォッチ */
static stop_watch_t sw;

/* コマンドの長さ(秒) */
static float span;

/* フェードメソッド */
static int fade_method;

/*
 * 前方参照
 */
static bool init(void);
static void draw(void);
static bool cleanup(void);

/*
 * bgコマンド
 */
bool bg_command(int *x, int *y, int *w, int *h)
{
	if (!is_in_command_repetition())
		if (!init())
			return false;

	draw();

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
	static struct image *img, *rule_img;
	const char *fname, *method;
	int ofs_x, ofs_y;

	/* パラメータを取得する */
	fname = get_string_param(BG_PARAM_FILE);
	span = get_float_param(BG_PARAM_SPAN);
	method = get_string_param(BG_PARAM_METHOD);
	ofs_x = get_int_param(BG_PARAM_X);
	ofs_y = get_int_param(BG_PARAM_Y);

 	/* 描画メソッドを識別する */
	fade_method = get_fade_method(method);
	if (fade_method == FADE_METHOD_INVALID) {
		log_script_fade_method(method);
		log_script_exec_footer();
		return false;
	}

	/* ルールが使用される場合 */
	if (fade_method == FADE_METHOD_RULE ||
	    fade_method == FADE_METHOD_MELT) {
		/* ルールファイルが指定されていない場合 */
		if (strcmp(&method[5], "") == 0) {
			log_script_rule();
			log_script_exec_footer();
			return false;
		}

		/* イメージを読み込む */
		rule_img = create_image_from_file(RULE_DIR, &method[5]);
		if (rule_img == NULL) {
			log_script_exec_footer();
			return false;
		}
		set_rule_image(rule_img);
	}

	/* 色指定の場合 */
	if (fname[0] == '#') {
		/* 色を指定してイメージを作成する */
		img = create_image_from_color_string(conf_window_width,
						     conf_window_height,
						     &fname[1]);
	} else {
		if (strncmp(fname, CG_DIR_PREFIX, strlen(CG_DIR_PREFIX)) == 0) {
			/* cgからイメージを読み込む */
			img = create_image_from_file(CG_DIR, &fname[strlen(CG_DIR_PREFIX)]);
		} else {
			/* bgからイメージを読み込む */
			img = create_image_from_file(BG_DIR, fname);
		}
	}
	if (img == NULL) {
		log_script_exec_footer();
		return false;
	}

	/* 背景・キャラクタファイル名を設定する */
	if (!set_layer_file_name(LAYER_BG, fname)) {
		log_script_exec_footer();
		return false;
	}
	set_layer_file_name(LAYER_CHB, NULL);
	set_layer_file_name(LAYER_CHL, NULL);
	set_layer_file_name(LAYER_CHR, NULL);
	set_layer_file_name(LAYER_CHC, NULL);
	set_layer_position(LAYER_BG, ofs_x, ofs_y);
	set_layer_alpha(LAYER_BG, 255);
	set_anime_layer_position(ANIME_LAYER_BG, ofs_x, ofs_y);

	/* メッセージボックスを消す (msgbox.show.on.bg=2) */
	if (conf_msgbox_show_on_bg == 2) {
		show_namebox(false);
		show_msgbox(false);
	}
	show_click(false);

	/* 繰り返し動作を開始する */
	start_command_repetition();

	/* 背景フェードモードを有効にする */
	start_bg_fade(img);

	/* 時間計測を開始する */
	reset_stop_watch(&sw);

	/* メッセージボックスを消す (msgbox.show.on.bg=0 or 2) */
	if (conf_msgbox_show_on_bg == 0 ||
	    conf_msgbox_show_on_bg == 2) {
		show_namebox(false);
		show_msgbox(false);
	}

	return true;
}

/* 描画を行う */
static void draw(void)
{
	float lap;
	int x, y, w, h;

	/* 経過時間を取得する */
	lap = (float)get_stop_watch_lap(&sw) / 1000.0f;
	if (lap >= span)
		lap = span;

	/* 経過時間が一定値を超えた場合と、入力によりスキップされた場合 */
	if (is_in_command_repetition()) {
		if ((lap >= span)
		    ||
		    (!is_non_interruptible() &&
		     !is_auto_mode() &&
		     (is_control_pressed || is_return_pressed ||
		      is_left_clicked || is_down_pressed))) {
			/* 繰り返し動作を停止する */
			stop_command_repetition();

			/* フェードを完了する */
			stop_bg_fade();
		} else {
			/* フェーディングを行う */
			set_bg_fade_progress(lap / span);
		}
	}

	/* ステージを描画する */
	if (is_in_command_repetition())
		draw_stage_bg_fade(fade_method);
	else
		draw_stage();

	/* 折りたたみシステムメニューを描画する */
	if (conf_sysmenu_transition && !is_non_interruptible()) {
		x = y = w = h = 0;
		draw_stage_collapsed_sysmenu(false, &x, &y, &w, &h);
	}
}

/* 終了処理を行う */
static bool cleanup(void)
{
	/* ルール画像を破棄する */
	set_rule_image(NULL);

	/* 次のコマンドに移動する */
	if (!move_to_next_command())
		return false;

	return true;
}
