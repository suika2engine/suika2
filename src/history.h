/* -*- coding: utf-8-with-signature; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Suika 2
 * Copyright (C) 2001-2016, TABATA Keiichi. All rights reserved.
 */

/*
 * セーブ画面とセーブ・ロード実行
 *
 * [Changes]
 *  - 2016/07/10 作成
 */

#ifndef SUIKA_HISTORY_H
#define SUIKA_HISTORY_H

#include "types.h"

/* ヒストリに関する初期化処理を行う */
bool init_history(void);

/* ヒストリに関する終了処理を行う */
void cleanup_history(void);

/* メッセージを登録する */
bool register_message(const char *name, const char *msg, const char *voice);

/* ヒストリをクリアする(ロード時) */
void clear_history(void);

/* ヒストリ画面から復帰したかばかりかを確認する */
bool check_history_flag(void);

/* ヒストリが空であるかを返す */
bool is_history_empty(void);

/* ヒストリ画面を開始する */
void start_history_mode(void);

/* ヒストリ画面が有効であるかを返す */
bool is_history_mode(void);

/* ヒストリ画面を実行する */
void run_history_mode(int *x, int *y, int *w, int *h);

#endif
