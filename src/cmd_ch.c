/* -*- coding: utf-8-with-signature; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Suika 2
 * Copyright (C) 2001-2021, TABATA Keiichi. All rights reserved.
 */

/*
 * [Changes]
 *  - 2016/06/09 作成
 *  - 2021/06/10 マスクつき描画の対応
 */

#include "suika.h"

static stop_watch_t sw;
static float span;
static int fade_method;

static bool init(void);
static bool get_position(int *xpos, int *ypos, int *chpos, const char *pos,
			 struct image *img);
static int get_alpha(const char *alpha);
static void draw(void);
static bool cleanup(void);

/*
 * キャラクタコマンド
 */
bool ch_command(int *x, int *y, int *w, int *h)
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
	struct image *img;
	const char *fname;
	const char *pos;
	const char *method;
	const char *alpha_s;
	int xpos, ypos, chpos, ofs_x, ofs_y, alpha;

	/* パラメータを取得する */
	pos = get_string_param(CH_PARAM_POS);
	fname = get_string_param(CH_PARAM_FILE);
	span = get_float_param(CH_PARAM_SPAN);
	method = get_string_param(CH_PARAM_METHOD);
	ofs_x = get_int_param(CH_PARAM_OFFSET_X);
	ofs_y = get_int_param(CH_PARAM_OFFSET_Y);
	alpha_s = get_string_param(CH_PARAM_ALPHA);

	/* イメージが指定された場合 */
	if (strcmp(fname, "none") != 0) {
		/* イメージを読み込む */
		img = create_image_from_file(CH_DIR, fname);
		if (img == NULL) {
			log_script_exec_footer();
			return false;
		}
	} else {
		/* イメージが指定されなかった場合(消す) */
		img = NULL;
	}

	/* 位置を取得する */
	if (!get_position(&xpos, &ypos, &chpos, pos, img))
		return false;

	xpos += ofs_x;
	ypos += ofs_y;

	/* フェードの種類を求める */
	fade_method = get_fade_method(method);
	if (fade_method == FADE_METHOD_INVALID) {
		/* スクリプト実行エラー */
		log_script_fade_method(method);
		log_script_exec_footer();
		return false;
	}

	/* アルファ値を求める */
	alpha = get_alpha(alpha_s);

	/* キャラのファイル名を設定する */
	if (!set_ch_file_name(chpos, strcmp(fname, "none") == 0 ? NULL :
			      fname))
	    return false;

	/* Controlが押されているか、フェードしない場合 */
	if (is_skip_mode() || is_control_pressed || span == 0) {
		/* フェードせず、すぐに切り替える */
		change_ch_immediately(chpos, img, xpos, ypos, alpha);
	} else {
		/* 繰り返し動作を開始する */
		start_command_repetition();

		/* キャラフェードモードを有効にする */
		start_ch_fade(chpos, img, xpos, ypos, alpha);

		/* 時間計測を開始する */
		reset_stop_watch(&sw);
	}

	/* メッセージボックスを消す */
	show_namebox(false);
	show_msgbox(false);
	show_click(false);

	return true;
}

/* キャラの横方向の位置を取得する */
static bool get_position(int *xpos, int *ypos, int *chpos, const char *pos,
			 struct image *img)
{
	*xpos = 0;

	if (strcmp(pos, "back") == 0 || strcmp(pos, "b") == 0 ) {
		/* 中央に配置する */
		*chpos = CH_BACK;
		if (img != NULL)
			*xpos = (conf_window_width - get_image_width(img)) / 2;
	} else if (strcmp(pos, "left") == 0 || strcmp(pos, "l") == 0) {
		/* 左に配置する */
		*chpos = CH_LEFT;
		*xpos = 0;
	} else if (strcmp(pos, "right") == 0 || strcmp(pos, "r") == 0) {
		/* 右に配置する */
		*chpos = CH_RIGHT;
		if (img != NULL)
			*xpos = conf_window_width - get_image_width(img);
	} else if (strcmp(pos, "center") == 0 || strcmp(pos, "c") == 0) {
		/* 中央に配置する */
		*chpos = CH_CENTER;
		if (img != NULL)
			*xpos = (conf_window_width - get_image_width(img)) / 2;
	} else {
		/* スクリプト実行エラー */
		log_script_ch_position(pos);
		log_script_exec_footer();
		return false;
	}

	/* 縦方向の位置を求める */
	*ypos = img != NULL ? conf_window_height - get_image_height(img) : 0;
	return true;
}

/* アルファ値を取得する */
static int get_alpha(const char *alpha)
{
	if (strcmp(alpha, "show") == 0 ||
	    strcmp(alpha, "") == 0)
		return 255;
	else if (strcmp(alpha, "hide") == 0)
		return 0;
	else
		return atoi(alpha);
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
		if (lap >= span || is_control_pressed || is_return_pressed ||
		    is_left_button_pressed) {
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
	/* 次のコマンドに移動する */
	if (!move_to_next_command())
		return false;

	return true;
}
