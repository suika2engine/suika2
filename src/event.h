/* -*- coding: utf-8; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Suika2
 * Copyright (C) 2016-2023, Keiichi Tabata. All rights reserved.
 */

/*
 * [Changes]
 *  - 2016/05/27 作成
 *  - 2016/06/22 分割
 */

#ifndef SUIKA_EVENT_H
#define SUIKA_EVENT_H

#include "types.h"

/*
 * キーコード
 */
enum key_code {
	KEY_CONTROL,
	KEY_SPACE,
	KEY_RETURN,
	KEY_UP,
	KEY_DOWN,
	KEY_LEFT,
	KEY_RIGHT,
	KEY_ESCAPE,
	KEY_C,
	KEY_S,
	KEY_L,
	KEY_H,
};

/*
 * マウスボタン
 */
enum mouse_button {
	MOUSE_LEFT,
	MOUSE_RIGHT,
};

/*
 * イベントハンドラ
 */
bool on_event_init(void);
void on_event_cleanup(void);
bool on_event_frame(void);
void on_event_key_press(int key);
void on_event_key_release(int key);
void on_event_mouse_press(int button, int x, int y);
void on_event_mouse_release(int button, int x, int y);
void on_event_mouse_move(int x, int y);
void on_event_touch_cancel(void);
void on_event_swipe_down(void);
void on_event_swipe_up(void);

/*
 * タッチ対応(右スワイプでスキップ動作を行うか)
 */
void set_horizontal_swipe_enabled(bool is_enabled);

#endif
