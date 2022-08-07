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
 *  - 2022/08/07 GUIをgui.cに分離
 */

#ifndef SUIKA_SAVE_H
#define SUIKA_SAVE_H

#include "types.h"

/*
 * グローバル変数のセーブファイル
 */
#define GLOBAL_VARS_FILE	"g000.sav"

/*
 * セーブGUIファイル
 */
#define SAVE_GUI_FILE		"save.txt"

/*
 * ロードGUIファイル
 */
#define LOAD_GUI_FILE		"load.txt"

/* セーブデータに関する初期化処理を行う */
bool init_save(void);

/* セーブデータに関する終了処理を行う */
void cleanup_save(void);

/* セーブを実行する */
bool execute_save(int index);

/* ロードを実行する */
bool execute_load(int index);

/* グローバルデータを保存する */
void save_global_data(void);

/* クイックセーブデータがあるか */
bool have_quick_save_data(void);

/* クイックセーブを行う */
bool quick_save(void);

/* クイックロードを行う */
bool quick_load(void);

/* セーブデータの削除を処理する */
void delete_save_data(int index);

/* セーブデータの日付を取得する */
time_t get_save_date(int index);

/* セーブデータの章タイトルを取得する */
const char *get_save_chapter_name(int index);

/* セーブデータの最後のメッセージを取得する */
const char *get_save_last_message(int index);

/* セーブデータのサムネイルを取得する */
struct image *get_save_thumbnail(int index);

/* 章題を設定する */
bool set_chapter_name(const char *name);

/* 章題を取得する */
const char *get_chapter_name(void);

/* 最後のメッセージを設定する */
bool set_last_message(const char *msg);

/* テキストスピードを設定する */
void set_text_speed(float val);

/* テキストスピードを取得する */
float get_text_speed(void);

/* オートスピードを設定する */
void set_auto_speed(float val);

/* オートスピードを取得する */
float get_auto_speed(void);

#endif
