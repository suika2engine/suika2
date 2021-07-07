/* -*- coding: utf-8-with-signature; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Suika 2
 * Copyright (C) 2001-2021, TABATA Keiichi. All rights reserved.
 */

/*
 * セーブ画面とセーブ・ロード実行
 *
 * [Changes]
 *  - 2016/06/30 作成
 *  - 2017/08/17 グローバル変数に対応
 *  - 2021/06/05 マスターボリュームに対応
 *  - 2021/07/07 セーブ専用画面に対応
 */

#ifndef SUIKA_SAVE_H
#define SUIKA_SAVE_H

#include "types.h"

/*
 * グローバル変数のセーブファイル
 */
#define GLOBAL_VARS_FILE	"g000.sav"

/* セーブデータに関する初期化処理を行う */
bool init_save(void);

/* セーブデータに関する終了処理を行う */
void cleanup_save(void);

/* コマンドがロードによって開始されたかを確認する */
bool check_load_flag(void);

/* コマンドがセーブ画面から復帰したかを確認する */
bool check_restore_flag(void);

/* セーブ画面を開始する */
void start_save_mode(bool allow_save, bool allow_load);

/* セーブ画面が有効であるかを返す */
bool is_save_mode(void);

/* セーブ画面を実行する */
bool run_save_mode(int *x, int *y, int *w, int *h);

/* グローバルデータを保存する */
void save_global_data(void);

#endif
