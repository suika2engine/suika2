/* -*- coding: utf-8; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Suika 2
 * Copyright (C) 2001-2022, TABATA Keiichi. All rights reserved.
 */

/*
 * GUI
 *
 * [Changes]
 *  - 2022/07/27 作成
 */

#ifndef SUIKA_GUI_H
#define SUIKA_GUI_H

#include "types.h"

/* GUIに関する初期化処理を行う */
bool init_gui(void);

/* GUIに関する終了処理を行う */
void cleanup_gui(void);

/* GUIから復帰した直後かどうかを確認する */
bool check_gui_flag(void);

/* GUIを準備する */
bool prepare_gui_mode(const char *file, bool sys);

/* GUIのオプションを指定する */
void set_gui_options(bool cancel, bool nofadein, bool nofadeout);

/* GUIを開始する */
void start_gui_mode(void);

/* GUIを停止する */
void stop_gui_mode(void);

/* GUIが有効であるかを返す */
bool is_gui_mode(void);

/* GUIがオーバレイであるかを返す */
bool is_gui_overlay(void);

/* GUIを実行する */
bool run_gui_mode(int *x, int *y, int *w, int *h);

/* GUIの実行結果のジャンプ先ラベルを取得する */
const char *get_gui_result_label(void);

/* GUIの実行結果がタイトルへ戻るであるかを調べる */
bool is_gui_result_title(void);

/* GUIの実行結果が終了であるかを取得する */
bool is_gui_result_exit(void);

/* GUIでセーブされたか */
bool is_gui_saved(void);

#endif
