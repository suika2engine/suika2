/* -*- coding: utf-8; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Suika 2
 * Copyright (C) 2001-2016, TABATA Keiichi. All rights reserved.
 */

/*
 * イベントハンドラ
 *  - platform.hの実装から呼び出される
 *
 * [Changes]
 *  - 2016/05/27 作成
 *  - 2016/06/22 分割
 *  - 2021/06/16 走査変換バッファに対応
 */

#include "suika.h"

/* False assertion */
#define INVALID_KEYCODE	(0)

/*
 * プラットフォーム非依存な初期化処理を行う
 */
bool on_event_init(void)
{
	int w, h;

	/* 変数の初期化処理を行う */
	init_vars();

	/* セーブデータの初期化処理を行う */
	if (!init_save())
		return false;

	/* 文字レンダリングエンジンの初期化処理を行う */
	if (!init_glyph())
		return false;

	/* ミキサの初期化処理を行う */
	init_mixer();

	/* ステージの初期化処理を行う */
	if (!init_stage())
		return false;

	/* 走査変換バッファの初期化処理を行う */
	if (!init_scbuf())
		return false;

	/* 初期スクリプトをロードする */
	if (!init_script())
		return false;

	/* 既読フラグをロードする */
	if (!init_seen())
		return false;

	/* GUIを初期化する */
	if (!init_gui())
		return false;

	/* ゲームループの初期化処理を行う */
	init_game_loop();

	/* フォントのプリロードを行う */
	draw_glyph(NULL, 0, 0, 0, 0, 'A', &w, &h);

	return true;
}

/*
 * プラットフォーム非依存な終了処理を行う
 */
void on_event_cleanup(void)
{
	/* ゲームループの終了処理を行う */
	cleanup_game_loop();

	/* GUIの終了処理を行う */
	cleanup_gui();

	/* 既読フラグ管理の終了処理を行う */
	cleanup_seen();

	/* スクリプトの終了処理を行う */
	cleanup_script();

	/* デーブデータ関連の終了処理を行う */
	cleanup_save();

	/* 走査変換バッファの終了処理を行う */
	cleanup_scbuf();

	/* ステージの終了処理を行う */
	cleanup_stage();

	/* ミキサの終了処理を行う */
	cleanup_mixer();

	/* 文字レンダリングエンジンの終了処理を行う */
	cleanup_glyph();

	/* 変数の終了処理を行う */
	cleanup_vars();
}

/*
 * 再描画時に呼び出される
 */
bool on_event_frame(int *x, int *y, int *w, int *h)
{
	/* デフォルトの書き換え領域をなしとする */
	*x = *y = *w = *h = 0;
	
	/* ゲームループの中身を実行する */
	if (!game_loop_iter(x, y, w, h)) {
		/* アプリケーションを終了する */
		return false;
	}

	/* アプリケーションを続行する */
	return true;
}

/*
 * キー押下時に呼び出される
 */
void on_event_key_press(int key)
{
	switch(key) {
	case KEY_CONTROL:
		is_control_pressed = true;
		break;
	case KEY_SPACE:
		is_space_pressed = true;
		break;
	case KEY_RETURN:
		is_return_pressed = true;
		break;
	case KEY_UP:
		is_up_pressed = true;
		break;
	case KEY_DOWN:
		is_down_pressed = true;
		break;
	case KEY_ESCAPE:
		is_escape_pressed = true;
		break;
	case KEY_C:
		conf_click_disable = conf_click_disable ? 0 : 1;
		break;
	default:
		assert(INVALID_KEYCODE);
		break;
	}
}

/*
 * キー解放時に呼び出される
 */
void on_event_key_release(int key)
{
	switch(key) {
	case KEY_CONTROL:
		is_control_pressed = false;
		break;
	case KEY_SPACE:
		is_space_pressed = false;
		break;
	case KEY_RETURN:
	case KEY_UP:
	case KEY_DOWN:
	case KEY_ESCAPE:
		/* これらのキーはフレームごとに解放されたことにされる */
		break;
	case KEY_C:
		/* このキーには解放処理を行わない */
		break;
	default:
		assert(0);
		break;
	}
}

/*
 * マウス押下時に呼び出される
 */
void on_event_mouse_press(int button, int x, int y)
{
	mouse_pos_x = x;
	mouse_pos_y = y;

	if (button == MOUSE_LEFT) {
		is_left_button_pressed = true;
		is_mouse_dragging = true;
	} else {
		is_right_button_pressed = true;
	}
}

/*
 * マウス解放時に呼び出される
 */
void on_event_mouse_release(UNUSED(int button), int x, int y)
{
	/* 1フレーム内の解放を無視する */
	mouse_pos_x = x;
	mouse_pos_y = y;

	is_mouse_dragging = false;
}

/*
 * マウス移動時に呼び出される
 */
void on_event_mouse_move(int x, int y)
{
	mouse_pos_x = x;
	mouse_pos_y = y;
}

/*
 * マウススクロール時に呼び出される
 */
void on_event_mouse_scroll(UNUSED(int n))
{
	/* FIXME: このイベントはいらないのでは？ */
}
