/* -*- coding: utf-8-with-signature; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Suika 2
 * Copyright (C) 2001-2021, TABATA Keiichi. All rights reserved.
 */

/*
 * [Changes]
 *  - 2016/06/09 作成
 */

#include "suika.h"

static stop_watch_t sw;
static float span;

static bool init(void);
static void draw(void);
static bool cleanup(void);

/*
 * 背景コマンド
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
	int xpos, ypos, chpos;

	/* パラメータを取得する */
	pos = get_string_param(CH_PARAM_POS);
	fname = get_string_param(CH_PARAM_FILE);
	span = get_float_param(CH_PARAM_SPAN);

	/* イメージが指定された場合 */
	if (strcmp(fname, "none") != 0) {
		/* イメージを読み込む */
		img = create_image_from_file(CH_DIR, fname);
		if (img == NULL) {
			log_script_exec_footer();
			return false;
		}
	} else {
		/* イメージが指定されなかった場合 */
		img = NULL;
	}

	/* 位置を取得する */
	xpos = 0;
	switch (*pos) {
	case 'b':
		/* 中央に配置する */
		chpos = CH_BACK;
		if (img != NULL)
			xpos = (conf_window_width - get_image_width(img)) / 2;
		break;
	case 'l':
		/* 左に配置する */
		chpos = CH_LEFT;
		xpos = 0;
		break;
	case 'r':
		/* 右に配置する */
		chpos = CH_RIGHT;
		if (img != NULL)
			xpos = conf_window_width - get_image_width(img);
		break;
	case 'c':
		/* 中央に配置する */
		chpos = CH_CENTER;
		if (img != NULL)
			xpos = (conf_window_width - get_image_width(img)) / 2;
		break;
	default:
		/* スクリプト実行エラー */
		log_script_ch_position(pos);
		log_script_exec_footer();
		return false;
	}

	/* 縦方向の位置を求める */
	ypos = img != NULL ? conf_window_height - get_image_height(img) : 0;

	/* キャラのファイル名を設定する */
	if (!set_ch_file_name(chpos, strcmp(fname, "none") == 0 ? NULL :
			      fname))
	    return false;

	/* Controlが押されているか、フェードしない場合 */
	if (is_control_pressed || span == 0) {
		/* フェードせず、すぐに切り替える */
		change_ch_immediately(chpos, img, xpos, ypos);
	} else {
		/* 繰り返し動作を開始する */
		start_command_repetition();

		/* キャラフェードモードを有効にする */
		start_ch_fade(chpos, img, xpos, ypos);

		/* 時間計測を開始する */
		reset_stop_watch(&sw);
	}

	/* メッセージボックスを消す */
	show_namebox(false);
	show_msgbox(false);
	show_click(false);

	return true;
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
		draw_stage_ch_fade();
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
