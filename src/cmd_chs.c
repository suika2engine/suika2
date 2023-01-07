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
 */

#include "suika.h"

/* キャラクタレイヤ数+背景レイヤの配列 */
#define PARAM_SIZE	(CH_BASIC_LAYERS + 1)
#define BG_INDEX	(CH_BASIC_LAYERS)

static stop_watch_t sw;
static float span;
static int fade_method;

static bool init(void);
static void get_position(int *xpos, int *ypos, int chpos, struct image *img);
static void draw(void);
static bool cleanup(void);

/*
 * キャラクタコマンド
 */
bool chs_command(int *x, int *y, int *w, int *h)
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

/*
 * コマンドの初期化処理を行う
 */
static bool init(void)
{
	struct image *img[PARAM_SIZE], *rule_img;
	const char *fname[PARAM_SIZE];
	bool stay[PARAM_SIZE];
	int x[PARAM_SIZE];
	int y[PARAM_SIZE];
	const char *method;
	int i;

	/* パラメータを取得する */
	fname[CH_CENTER] = get_string_param(CHS_PARAM_CENTER);
	fname[CH_RIGHT] = get_string_param(CHS_PARAM_RIGHT);
	fname[CH_LEFT] = get_string_param(CHS_PARAM_LEFT);
	fname[CH_BACK] = get_string_param(CHS_PARAM_BACK);
	fname[BG_INDEX] = get_string_param(CHS_PARAM_BG);
	span = get_float_param(CHS_PARAM_SPAN);
	method = get_string_param(CHS_PARAM_METHOD);

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

		/* 変更なしが指定された場合 */
		if (strcmp(fname[i], "stay") == 0 ||
		    strcmp(fname[i], U8("変更なし")) == 0 ||
		    strcmp(fname[i], "") == 0) {
			/* 変更なしフラグをセットする */
			stay[i] = true;
			continue;
		}

		/* イメージの消去が指定された場合 */
		if (i != BG_INDEX &&
		    (strcmp(fname[i], "none") == 0 ||
		     strcmp(fname[i], U8("消す")) == 0)) {
			/* 変更なしフラグをセットしない */
			stay[i] = false;

			/* ファイル名を設定する */
			if (!set_ch_file_name(i, NULL))
				return false;

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
		if (i != BG_INDEX) {
			if (!set_ch_file_name(i, fname[i]))
				return false;
		} else {
			if (!set_bg_file_name(fname[i]))
				return false;
		}

		/* 表示位置を取得する */
		if (i != BG_INDEX)
			get_position(&x[i], &y[i], i, img[i]);
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

	/* キーが押されているか、フェードしない場合 */
	if ((span == 0)
	    ||
	    (!is_non_interruptible() && is_skip_mode())
	    ||
	    (!is_non_interruptible() && !is_auto_mode() &&
	     is_control_pressed)) {
		/* フェードせず、すぐに切り替える */
		for (i = 0; i < PARAM_SIZE; i++) {
			if (stay[i])
				continue;

			/* フェードせず、すぐに切り替える */
			if (i != BG_INDEX)
				change_ch_immediately(i, img[i], x[i], y[i],
						      255);
			else
				change_bg_immediately(img[i]);
		}
	} else {
		/* 繰り返し動作を開始する */
		start_command_repetition();

		/* キャラフェードモードを有効にする */
		start_ch_fade_multi(stay, img, x, y);

		/* 時間計測を開始する */
		reset_stop_watch(&sw);
	}

	/* メッセージボックスを消す */
	if (!conf_msgbox_show_on_ch) {
		show_namebox(false);
		show_msgbox(false);
	}
	show_click(false);

	return true;
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
			*xpos = (conf_window_width - get_image_width(img)) / 2;
		break;
	case CH_LEFT:
		/* 左に配置する */
		*xpos = 0;
		break;
	case CH_RIGHT:
		/* 右に配置する */
		if (img != NULL)
			*xpos = conf_window_width - get_image_width(img);
		break;
	}

	/* 縦方向の位置を求める */
	*ypos = img != NULL ? conf_window_height - get_image_height(img) : 0;
}

/* 描画を行う */
static void draw(void)
{
	float lap;

	/* 経過時間を取得する */
	lap = (float)get_stop_watch_lap(&sw) / 1000.0f;
	if (lap >= span)
		lap = span;

	if (is_in_command_repetition()) {
		/*
		 * 経過時間が一定値を超えた場合と、
		 * 入力によりスキップされた場合
		 */
		if ((lap >= span)
		    ||
		    (!is_non_interruptible() && !is_auto_mode() &&
		     (is_control_pressed || is_return_pressed ||
		      is_left_clicked || is_down_pressed))) {
			/* 繰り返し動作を終了する */
			stop_command_repetition();

			/* キャラフェードモードを終了する */
			stop_ch_fade();
		} else {
			/* 進捗を設定する */
			set_ch_fade_progress(lap / span);
		}
	}

	/* ステージを描画する */
	if (is_in_command_repetition())
		draw_stage_ch_fade(fade_method);
	else
		draw_stage();
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
