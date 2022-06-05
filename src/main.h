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
 *  - 2021/06/15 setsaveに対応
 *  - 2022/05/11 動画再生に対応
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
bool goto_command(bool *cont);
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
bool setsave_command(void);
bool chs_command(int *x, int *y, int *w, int *h);
bool video_command(void);
bool skip_command(void);

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

void set_menu_finish_flag(void);
bool check_menu_finish_flag(void);
void set_retrospect_finish_flag(void);
bool check_retrospect_finish_flag(void);

/*
 * オートモードの設定
 */

void start_auto_mode(void);
void stop_auto_mode(void);
bool is_auto_mode(void);

/*
 * スキップモードの設定
 */

void start_skip_mode(void);
void stop_skip_mode(void);
bool is_skip_mode(void);

/*
 * セーブ・ロード画面の許可の設定
 */

void set_save_load(bool enable);
bool is_save_load_enabled(void);

/*
 * 割り込み不可モードの設定
 */

void set_non_interruptible(bool mode);
bool is_non_interruptible(void);

#endif
