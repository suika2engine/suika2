/* -*- coding: utf-8; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Suika 2
 * Copyright (C) 2001-2023, TABATA Keiichi. All rights reserved.
 */

/*
 * [Changes]
 *  - 2021/07/19 作成
 *  - 2022/06/26 テンプレートに対応
 *  - 2023/01/06 日本語の指定に対応
 *  - 2023/08/31 @chsxに対応
 */

#include "suika.h"

/* キャラクタレイヤ数+背景レイヤの配列 */
#define PARAM_SIZE	(CH_BASIC_LAYERS + 1)
#define BG_INDEX	(CH_BASIC_LAYERS)

static uint64_t sw;
static float span;
static int fade_method;

static bool init(void);
static int get_alpha(const char *alpha_s);
static void get_position(int *xpos, int *ypos, int chpos, struct image *img);
static void focus_character(int chpos, const char *fname);
static void draw(void);
static bool cleanup(void);

/*
 * 場面転換コマンド
 */
bool chs_command(void)
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

/*
 * コマンドの初期化処理を行う
 */
static bool init(void)
{
	struct image *img[PARAM_SIZE], *rule_img;
	const char *fname[PARAM_SIZE];
	bool stay[PARAM_SIZE];
	int ofs_x[PARAM_SIZE];
	int ofs_y[PARAM_SIZE];
	int alpha[PARAM_SIZE - 1];
	int x[PARAM_SIZE];
	int y[PARAM_SIZE];
	const char *method;
	int i, layer;

	/* パラメータを取得する */
	if (get_command_type() == COMMAND_CHS) {
		fname[CH_CENTER] = get_string_param(CHS_PARAM_CENTER);
		fname[CH_RIGHT] = get_string_param(CHS_PARAM_RIGHT);
		fname[CH_LEFT] = get_string_param(CHS_PARAM_LEFT);
		fname[CH_BACK] = get_string_param(CHS_PARAM_BACK);
		fname[BG_INDEX] = get_string_param(CHS_PARAM_BG);
		fname[CH_LEFT_CENTER] = "";
		fname[CH_RIGHT_CENTER] = "";
		ofs_x[CH_CENTER] = 0;
		ofs_y[CH_CENTER] = 0;
		ofs_x[CH_RIGHT] = 0;
		ofs_y[CH_RIGHT] = 0;
		ofs_x[CH_LEFT] = 0;
		ofs_y[CH_LEFT] = 0;
		ofs_x[CH_BACK] = 0;
		ofs_y[CH_BACK] = 0;
		ofs_x[BG_INDEX] = 0;
		ofs_y[BG_INDEX] = 0;
		ofs_x[CH_RIGHT_CENTER] = 0;
		ofs_y[CH_RIGHT_CENTER] = 0;
		ofs_x[CH_LEFT_CENTER] = 0;
		ofs_y[CH_LEFT_CENTER] = 0;
		alpha[CH_CENTER] = 255;
		alpha[CH_RIGHT] = 255;
		alpha[CH_LEFT] = 255;
		alpha[CH_RIGHT_CENTER] = 255;
		alpha[CH_LEFT_CENTER] = 255;
		alpha[CH_BACK] = 255;
		span = get_float_param(CHS_PARAM_SPAN);
		method = get_string_param(CHS_PARAM_METHOD);
	} else {
		fname[CH_CENTER] = get_string_param(CHSX_PARAM_CENTER);
		fname[CH_RIGHT] = get_string_param(CHSX_PARAM_RIGHT);
		fname[CH_RIGHT_CENTER] = get_string_param(CHSX_PARAM_RIGHT_CENTER);
		fname[CH_LEFT] = get_string_param(CHSX_PARAM_LEFT);
		fname[CH_LEFT_CENTER] = get_string_param(CHSX_PARAM_LEFT_CENTER);
		fname[CH_BACK] = get_string_param(CHSX_PARAM_BACK);
		fname[BG_INDEX] = get_string_param(CHSX_PARAM_BG);
		ofs_x[CH_CENTER] = get_int_param(CHSX_PARAM_CX);
		ofs_y[CH_CENTER] = get_int_param(CHSX_PARAM_CY);
		ofs_x[CH_RIGHT] = get_int_param(CHSX_PARAM_RX);
		ofs_y[CH_RIGHT] = get_int_param(CHSX_PARAM_RY);
		ofs_x[CH_LEFT] = get_int_param(CHSX_PARAM_LX);
		ofs_y[CH_LEFT] = get_int_param(CHSX_PARAM_LY);
		ofs_x[CH_RIGHT_CENTER] = get_int_param(CHSX_PARAM_RCX);
		ofs_y[CH_RIGHT_CENTER] = get_int_param(CHSX_PARAM_RCY);
		ofs_x[CH_LEFT_CENTER] = get_int_param(CHSX_PARAM_LCX);
		ofs_y[CH_LEFT_CENTER] = get_int_param(CHSX_PARAM_LCY);
		ofs_x[CH_BACK] = get_int_param(CHSX_PARAM_BX);
		ofs_y[CH_BACK] = get_int_param(CHSX_PARAM_BY);
		ofs_x[BG_INDEX] = get_int_param(CHSX_PARAM_BGX);
		ofs_y[BG_INDEX] = get_int_param(CHSX_PARAM_BGY);
		alpha[CH_CENTER] = get_alpha(get_string_param(CHSX_PARAM_CA));
		alpha[CH_RIGHT] = get_alpha(get_string_param(CHSX_PARAM_RA));
		alpha[CH_LEFT] = get_alpha(get_string_param(CHSX_PARAM_LA));
		alpha[CH_BACK] = get_alpha(get_string_param(CHSX_PARAM_BA));
		alpha[CH_RIGHT_CENTER] = get_alpha(get_string_param(CHSX_PARAM_RCA));
		alpha[CH_LEFT_CENTER] = get_alpha(get_string_param(CHSX_PARAM_LCA));
		span = get_float_param(CHSX_PARAM_SPAN);
		method = get_string_param(CHSX_PARAM_METHOD);
	}

	/* 描画メソッドを識別する */
	fade_method = get_fade_method(method);
	if (fade_method == FADE_METHOD_INVALID) {
		log_script_fade_method(method);
		log_script_exec_footer();
		return false;
	}

	/* 各キャラと背景について */
	for (i = 0; i < PARAM_SIZE; i++) {
		stay[i] = false;
		img[i] = NULL;
		x[i] = 0;
		y[i] = 0;

		if (i != BG_INDEX)
			layer = chpos_to_layer(i);
		else
			layer = LAYER_BG;

		/* 変更なしが指定された場合 */
		if (i != BG_INDEX) {
			if (strcmp(fname[i], "stay") == 0 ||
			    strcmp(fname[i], U8("変更なし")) == 0 ||
			    (get_command_type() == COMMAND_CHSX && strcmp(fname[i], "") == 0)) {
				/* 変更なしフラグをセットする */
				stay[i] = true;
				x[i] = get_layer_x(layer);
				y[i] = get_layer_y(layer);
				continue;
			}
		} else {
			if (strcmp(fname[i], "stay") == 0 ||
			    strcmp(fname[i], U8("変更なし")) == 0 ||
			    strcmp(fname[i], "") == 0) {
				/* 変更なしフラグをセットする */
				stay[i] = true;
				x[i] = get_layer_x(layer);
				y[i] = get_layer_y(layer);
				continue;
			}
		}

		/* イメージの消去が指定された場合 */
		if (i != BG_INDEX &&
		    (strcmp(fname[i], "none") == 0 ||
		     strcmp(fname[i], U8("消す")) == 0 ||
		     (get_command_type() == COMMAND_CHS && strcmp(fname[i], "") == 0))) {
			fname[i] = NULL;
			x[i] = get_layer_x(layer);
			y[i] = get_layer_y(layer);
			continue;
		}

		/* 背景の色指定の場合 */
		if (i == BG_INDEX && fname[i][0] == '#') {
			/* 色を指定してイメージを作成する */
			img[i] = create_image_from_color_string(
				conf_window_width,
				conf_window_height,
				&fname[i][1]);
		} else {
			/* イメージを読み込む */
			img[i] = create_image_from_file(
				i != BG_INDEX ? CH_DIR : BG_DIR, fname[i]);
		}
		if (img[i] == NULL) {
			log_script_exec_footer();
			return false;
		}

		/* ファイル名を設定する */
		if (!set_layer_file_name(layer, fname[i]))
			return false;

		/* 表示位置を取得する */
		if (i != BG_INDEX) {
			get_position(&x[i], &y[i], i, img[i]);
			x[i] += ofs_x[i];
			y[i] += ofs_y[i];
		} else {
			x[i] = ofs_x[i];
			y[i] = ofs_y[i];
		}

		/* キャラを暗くしない */
		if (i != BG_INDEX) {
			if (conf_character_focus == 1)
				focus_character(i, fname[i]);
		}
	}

	/* キャラの発話中状態を更新する */
	if (conf_character_focus == 1)
		set_ch_talking(-1);
	update_ch_dim();

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
	} else {
		rule_img = NULL;
	}

	/* 繰り返し動作を開始する */
	start_command_repetition();

	/* キャラフェードモードを有効にする */
	if (!start_fade_for_chs(stay, fname, img, x, y, alpha, fade_method,
				rule_img)) {
		log_script_exec_footer();
		return false;
	}

	/* 時間計測を開始する */
	reset_lap_timer(&sw);

	/* メッセージボックスを消す */
	if (!conf_msgbox_show_on_ch) {
		show_namebox(false);
		show_msgbox(false);
	}
	show_click(false);

	return true;
}

/* 文字列のアルファ値を整数に変換する */
static int get_alpha(const char *alpha_s)
{
	int ret;

	/* 省略された場合は255にする */
	if (strcmp(alpha_s, "") == 0)
		return 255;

	ret = atoi(alpha_s);
	if (ret < 0)
		ret = 0;
	if (ret > 255)
		ret = 255;
	return ret;
}

/* キャラの横方向の位置を取得する */
static void get_position(int *xpos, int *ypos, int chpos, struct image *img)
{
	*xpos = 0;

	switch (chpos) {
	case CH_BACK:
	case CH_CENTER:
		/* 中央に配置する */
		if (img != NULL)
			*xpos = (conf_window_width - img->width) / 2;
		break;
	case CH_LEFT:
		/* 左に配置する */
		*xpos = conf_stage_ch_margin_left;
		break;
	case CH_LEFT_CENTER:
		/* 左中に配置する */
		*xpos = (conf_window_width - img->width) / 4;
		break;
	case CH_RIGHT:
		/* 右に配置する */
		if (img != NULL)
			*xpos = conf_window_width - img->width - conf_stage_ch_margin_right;
		break;
	case CH_RIGHT_CENTER:
		/* 右に配置する */
		if (img != NULL)
			*xpos = (conf_window_width - img->width) - img->width / 2;
		break;
	}

	/* 縦方向の位置を求める */
	*ypos = img != NULL ? conf_window_height - img->height : 0;
}

/* キャラクタのフォーカスを行う */
static void focus_character(int chpos, const char *fname)
{
	int i;

	/* 名前が登録されているキャラクタであるかチェックする */
	for (i = 0; i < CHARACTER_MAP_COUNT; i++) {
		if (conf_character_name[i] == NULL)
			continue;
		if (conf_character_file[i] == NULL)
			continue;
		if (fname == NULL)
			continue;
		if (strncmp(conf_character_file[i], fname, strlen(conf_character_file[i])) == 0)
			break;
	}
	if (i == CHARACTER_MAP_COUNT)
		i = -1;

	set_ch_name_mapping(chpos, i);
}

/* 描画を行う */
static void draw(void)
{
	float lap;

	/* 経過時間を取得する */
	lap = (float)get_lap_timer_millisec(&sw) / 1000.0f;
	if (lap >= span)
		lap = span;

	/* 入力に反応する */
	if (is_auto_mode() &&
	    (is_control_pressed || is_return_pressed ||
	     is_left_clicked || is_down_pressed)) {
		/* 入力によりオートモードを終了する */
		stop_auto_mode();
		show_automode_banner(false);

		/* 繰り返し動作を停止する */
		stop_command_repetition();

		/* フェードを完了する */
		finish_fade();
	} else if (is_skip_mode() &&
		   (is_control_pressed || is_return_pressed ||
		    is_left_clicked || is_down_pressed)) {
		/* 入力によりスキップモードを終了する */
		stop_skip_mode();
		show_skipmode_banner(false);

		/* 繰り返し動作を停止する */
		stop_command_repetition();

		/* フェードを完了する */
		finish_fade();
	} else if ((lap >= span)
		   ||
		   is_skip_mode()
		   ||
		   (!is_non_interruptible() &&
		    (is_control_pressed || is_return_pressed ||
		     is_left_clicked || is_down_pressed))) {
		/*
		 * 経過時間が一定値を超えた場合と、
		 * スキップモードの場合と、
		 * 入力により省略された場合
		 */

		/* 繰り返し動作を終了する */
		stop_command_repetition();

		/* フェードを終了する */
		finish_fade();

		/* 入力があればスキップとオートを終了する */
		if (is_control_pressed || is_return_pressed ||
		    is_left_clicked || is_down_pressed) {
			if (is_skip_mode()) {
				stop_skip_mode();
				show_skipmode_banner(false);
			} else if (is_auto_mode()) {
				stop_auto_mode();
				show_automode_banner(false);
			}
		}
	} else {
		/* 進捗を設定する */
		set_fade_progress(lap / span);
	}

	/* ステージを描画する */
	if (is_in_command_repetition())
		draw_fade();
	else
		draw_stage();

	/* 折りたたみシステムメニューを描画する */
	if (conf_sysmenu_transition && !is_non_interruptible())
		draw_stage_collapsed_sysmenu(false);
}

/* 終了処理を行う */
static bool cleanup(void)
{
	/* 次のコマンドに移動する */
	if (!move_to_next_command())
		return false;

	return true;
}
