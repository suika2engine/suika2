/* -*- coding: utf-8-with-signature; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Suika 2
 * Copyright (C) 2001-2016, TABATA Keiichi. All rights reserved.
 */

/*
 * [Changes]
 *  - 2016/06/09 作成
 */

#include "suika.h"

/* 繰り返し動作中であるか */
static bool repeatedly;

/* コマンドの経過時刻を表すストップウォッチ */
static stop_watch_t sw;

/* コマンドの長さ(秒) */
static float span;

/* カーテンフェードであるか */
static bool is_curtain;

/* フェードイン中のイメージ */
static struct image *img;

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
	if (!repeatedly)
		if (!init())
			return false;

	draw();

	if (!repeatedly)
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
	const char *fname, *method;

	/* パラメータを取得する */
	fname = get_string_param(BG_PARAM_FILE);
	span = get_float_param(BG_PARAM_SPAN);
	method = get_string_param(BG_PARAM_METHOD);

	/* 描画メソッドを識別する */
	switch(method[0]) {
	case 'c':	/* curtain */
		is_curtain = true;
		break;
	default:
		is_curtain = false;
		break;
	}

	/* イメージを読み込む */
	img = create_image_from_file(BG_DIR, fname);
	if (img == NULL) {
		log_dir_file_open(BG_DIR, fname);
		log_script_exec_footer();
		return false;
	}

	/* 背景・キャラクタファイル名を設定する */
	if (!set_bg_file_name(fname))
		return false;
	if (!set_ch_file_name(CH_BACK, NULL))
		return false;
	if (!set_ch_file_name(CH_RIGHT, NULL))
		return false;
	if (!set_ch_file_name(CH_LEFT, NULL))
		return false;
	if (!set_ch_file_name(CH_CENTER, NULL))
		return false;

	/* フェードしない場合か、Controlが押されている場合 */
	if (span == 0 || is_control_pressed) {
		/* フェードせず、すぐに切り替える */
		change_bg_immediately(img);
		change_ch_immediately(CH_BACK, NULL, 0, 0);
		change_ch_immediately(CH_LEFT, NULL, 0, 0);
		change_ch_immediately(CH_RIGHT, NULL, 0, 0);
		change_ch_immediately(CH_CENTER, NULL, 0, 0);
		return true;
	} else {
		/* bgコマンドが繰り返し呼び出されるようにする */
		repeatedly = true;

		/* 背景フェードモードを有効にする */
		start_bg_fade(img);

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

	/* 経過時間が一定値を超えた場合と、入力によりスキップされた場合 */
	if (repeatedly) {
		if (lap >= span || is_control_pressed || is_return_pressed ||
		    is_left_button_pressed) {
			/* 繰り返し動作を停止する */
			repeatedly = false;

			/* フェードを完了する */
			stop_bg_fade();
		} else {
			/* フェーディングを行う */
			set_bg_fade_progress(lap / span);
		}
	}

	/* ステージを描画する */
	if (repeatedly)
		draw_stage_bg_fade(is_curtain);
	else
		draw_stage();
}

/* 終了処理を行う */
static bool cleanup(void)
{
	/* bgコマンドを非動作状態にする */
	repeatedly = false;

	/* 次のコマンドに移動する */
	if (!move_to_next_command())
		return false;

	return true;
}
