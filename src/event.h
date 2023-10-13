/* -*- coding: utf-8; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Suika 2
 * Copyright (C) 2016, TABATA Keiichi. All rights reserved.
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
 *  - platform.hの実装から呼び出される
 */
bool on_event_init(void);
void on_event_cleanup(void);
bool on_event_frame(int *x, int *y, int *w, int *h);
void on_event_key_press(int key);
void on_event_key_release(int key);
void on_event_mouse_press(int button, int x, int y);
void on_event_mouse_release(int button, int x, int y);
void on_event_mouse_move(int x, int y);
void on_event_mouse_scroll(int n);

#endif
