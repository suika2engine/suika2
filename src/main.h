/* -*- coding: utf-8-with-signature; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Suika 2
 * Copyright (C) 2001-2021, TABATA Keiichi. All rights reserved.
 */

/*
 * [Changes]
 *  - 2016/05/27 作成
 *  - 2017/08/13 スイッチに対応
 *  - 2018/07/21 gosubに対応
 *  - 2021/06/10 chaに対応
 *  - 2021/06/12 shakeに対応
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

void init_game_loop(void);
bool game_loop_iter(int *x, int *y, int *w, int *h);
void cleanup_game_loop(void);

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
bool gosub_command(void);
bool return_command(void);
bool cha_command(int *x, int *y, int *w, int *h);
bool shake_command(int *x, int *y, int *w, int *h);

/*
 * 複数のイテレーションに渡るコマンドの実行中であるかの設定
 */

void start_command_repetition(void);
void stop_command_repetition(void);
bool is_in_command_repetition(void);

/*
 * 現在表示中のメッセージがヒストリに登録済みであるかの設定
 */

void set_message_registered(void);
void clear_message_registered(void);
bool is_message_registered(void);

/*
 * コマンドが終了した直後であるかのチェック
 */

bool check_menu_finish_flag(void);
bool check_retrospect_finish_flag(void);

#endif
