/* -*- coding: utf-8-with-signature; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Suika 2
 * Copyright (C) 2001-2016, TABATA Keiichi. All rights reserved.
 */

/*
 * [Changes]
 *  - 2016/05/27 作成
 *  - 2017/08/13 スイッチに対応
 */

#ifndef SUIKA_MAIN_H
#define SUIKA_MAIN_H

#include "types.h"

/*
 * 入力の状態
 */
extern bool is_left_button_pressed;
extern bool is_right_button_pressed;
extern bool is_return_pressed;
extern bool is_space_pressed;
extern bool is_escape_pressed;
extern bool is_up_pressed;
extern bool is_down_pressed;
extern bool is_page_up_pressed;
extern bool is_page_down_pressed;
extern bool is_control_pressed;
extern int mouse_pos_x;
extern int mouse_pos_y;

/*
 * ゲームループの中身
 */

void init_game_loop();
bool game_loop_iter();
void cleanup_game_loop();

/*
 * コマンドの実装
 */

bool message_command(int *x, int *y, int *w, int *h);
bool bg_command(int *x, int *y, int *w, int *h);
bool bgm_command(void);
bool ch_command(int *x, int *y, int *w, int *h);
bool click_command(void);
bool wait_command(void);
bool goto_command(void);
bool load_command(void);
bool vol_command(void);
bool set_command(void);
bool if_command(void);
bool select_command(int *x, int *y, int *w, int *h);
bool se_command(void);
bool menu_command(int *x, int *y, int *w, int *h);
bool retrospect_command(int *x, int *y, int *w, int *h);
bool switch_command(int *x, int *y, int *w, int *h);

/*
 * コマンドが終了した直後であるかのチェック
 */

bool check_menu_finish_flag(void);
bool check_retrospect_finish_flag(void);

#endif
