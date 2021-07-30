/* -*- coding: utf-8-with-signature; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Suika 2
 * Copyright (C) 2001-2016, TABATA Keiichi. All rights reserved.
 */

/*
 * [Changes]
 *  - 2016/06/29 作成
 *  - 2021/06/15 @setsave対応
 */

#include "suika.h"

/* false assertion */
#define INVALID_SELECTION	(0)

/* 最初の描画であるか */
static bool is_first_frame;

/* テキスト */
static const char *text[3];

/* 選択肢の矩形 */
static int rect_x;
static int rect_y[3];
static int rect_w;
static int rect_h;

/* 選択中の項目 */
static int selected_item;

/* 前方参照 */
static bool init(void);
static bool cleanup(void);
static int get_selected_item(void);
static void draw_frame(int *x, int *y, int *w, int *h);
static void draw_selbox(int sel);
static void draw_text(int y, const char *t);
static void play_se(const char *file);

/*
 * selectコマンドの実装
 */
bool select_command(int *x, int *y, int *w, int *h)
{
	if (!is_in_command_repetition())
		if (!init())
			return false;

	/* セーブ画面への遷移を確認する */
	if (is_right_button_pressed && is_save_load_enabled()) {
		start_save_mode(false);
		stop_command_repetition();
		return true;
	}

	/* ヒストリ画面への遷移を確認する */
	if (is_up_pressed) {
		if (is_up_pressed)
			play_se(conf_msgbox_history_se);
		start_history_mode();
		stop_command_repetition();
		return true;
	}

	/* 繰り返し動作を行う */
	draw_frame(x, y, w, h);

	if (!is_in_command_repetition())
		if (!cleanup())
			return false;

	return true;
}

/*
 * 初期化処理を行う
 */
static bool init(void)
{
	/* パラメータを取得する */
	text[0] = get_string_param(SELECT_PARAM_TEXT1);
	text[1] = get_string_param(SELECT_PARAM_TEXT2);
	text[2] = get_string_param(SELECT_PARAM_TEXT3);

	/* 選択肢ボックスの矩形を取得する */
	get_selbox_rect(&rect_x, &rect_y[0], &rect_w, &rect_h);

	/* 選択項目のY座標を求める */
	rect_y[1] = rect_y[0] + rect_h / 3;
	rect_y[2] = rect_y[0] + rect_h / 3 * 2;

	/* 現在の選択項目を無効値にして再描画させる */
	selected_item = -1;

	/* 繰り返し動作を開始する */
	start_command_repetition();

	/* 選択肢ボックスを表示する */
	show_selbox(true);

	/* 最初のフレームであることを記録する */
	is_first_frame = true;

	/* 名前ボックス、メッセージボックスを消す */
	show_namebox(false);
	show_msgbox(false);

	return true;
}

/*
 * 終了処理を行う
 */
static bool cleanup(void)
{
	const char *label;

	/* 選択された項目のラベルを取得する */
	switch(selected_item) {
	case 0:
		label = get_string_param(SELECT_PARAM_LABEL1);
		break;
	case 1:
		label = get_string_param(SELECT_PARAM_LABEL2);
		break;
	case 2:
		label = get_string_param(SELECT_PARAM_LABEL3);
		break;
	default:
		assert(INVALID_SELECTION);
		return false;
	}

	/* ラベルへ移動する */
	return move_to_label(label);
}

/*
 * 繰り返し動作の描画を行う
 */
static void draw_frame(int *x, int *y, int *w, int *h)
{
	int new_selected_item;

	/* セーブ画面かヒストリ画面から復帰した場合のフラグをクリアする */
	check_restore_flag();
	check_history_flag();

	/* 終了する場合 */
	if (!is_first_frame && selected_item != -1 && is_left_button_pressed) {
		/* 選択肢ボックスを非表示する */
		show_selbox(false);

		/* ステージを描画する */
		draw_stage();

		/* ウィンドウの更新領域を設定する */
		get_selbox_rect(x, y, w, h);

		/* 繰り返し動作を終了する */
		stop_command_repetition();
		return;
	}

	/* 選択項目が変わったか調べる */
	new_selected_item = get_selected_item();
	if (!is_first_frame && new_selected_item == selected_item)
		return;
	if (new_selected_item != -1 && new_selected_item != selected_item)
		play_se(conf_selbox_change_se);

	/* 選択項目を変更する */
	selected_item = new_selected_item;
	
	/* 描画を行う */
	draw_selbox(selected_item);

	/* 更新領域を設定する */
	if (is_first_frame) {
		*x = 0;
		*y = 0;
		*w = conf_window_width;
		*h = conf_window_height;
	} else {
		get_selbox_rect(x, y, w, h);
	}

	/* 最初のフレームでないことを記録する */
	is_first_frame = false;
}

/*
 * マウス座標を元に選択項目を求める
 */
static int get_selected_item(void)
{
	if (mouse_pos_x < rect_x || mouse_pos_x >= rect_x + rect_w)
		return -1;
	if (mouse_pos_y < rect_y[0] || mouse_pos_y >= rect_y[0] + rect_h)
		return -1;

	if (mouse_pos_y < rect_y[1])
		return 0;
	if (mouse_pos_y >= rect_y[2])
		return 2;
	return 1;
}

/*
 * 選択肢ボックスを描画する
 */
static void draw_selbox(int sel)
{
	/* 選択肢ボックスのイメージを描画する */
	switch(sel) {
	case -1:
		clear_selbox(0, 0, 0, 0);
		break;
	case 0:
		clear_selbox(0, 0, rect_w, rect_h / 3);
		break;
	case 1:
		clear_selbox(0, rect_h / 3, rect_w, rect_h / 3);
		break;
	case 2:
		clear_selbox(0, rect_h / 3 * 2, rect_w, rect_h / 3);
		break;
	default:
		assert(INVALID_SELECTION);
		break;
	}

	/* 選択肢のテキストを描画する */
	draw_text(0, text[0]);
	draw_text(rect_h / 3, text[1]);
	draw_text(rect_h / 3 * 2, text[2]);

	draw_stage();
}

/*
 * 選択肢のテキストを描画する
 */
static void draw_text(int y, const char *t)
{
	uint32_t c;
	int mblen, x;

	/* 描画位置を決める */
	x = (rect_w - get_utf8_width(t)) / 2;
	y += conf_selbox_margin_y;

	/* 1文字ずつ描画する */
	while (*t != '\0') {
		/* 描画する文字を取得する */
		mblen = utf8_to_utf32(t, &c);
		if (mblen == -1)
			return;

		/* 描画する */
		x += draw_char_on_selbox(x, y, c);

		/* 次の文字へ移動する */
		t += mblen;
	}
}

/* SEを再生する */
static void play_se(const char *file)
{
	struct wave *w;

	if (file == NULL || strcmp(file, "") == 0)
		return;

	w = create_wave_from_file(SE_DIR, file, false);
	if (w == NULL)
		return;

	set_mixer_input(SE_STREAM, w);
}
