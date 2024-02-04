/* -*- coding: utf-8; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Suika2
 * Copyright (C) 2001-2023, Keiichi Tabata. All rights reserved.
 */

/*
 * [Changes]
 *  - 2016/06/29 作成
 *  - 2017/08/17 グローバル変数に対応
 *  - 2022/06/11 名前変数に対応
 */

#ifndef SUIKA_VARS_H
#define SUIKA_VARS_H

#include "types.h"

/*
 * 変数の数
 */
#define LOCAL_VAR_SIZE	(10000)
#define GLOBAL_VAR_SIZE	(1000)
#define VAR_SIZE	(LOCAL_VAR_SIZE + GLOBAL_VAR_SIZE)
#define NAME_VAR_SIZE	(26)

/*
 * グローバル変数のインデックスのオフセット
 */
#define GLOBAL_VAR_OFFSET	LOCAL_VAR_SIZE

/*
 * 名前変数のインデックス
 */
#define NAME_VAR_FAMILY	(0)
#define NAME_VAR_GIVEN	(1)

/* 変数の初期化処理を行う */
void init_vars(void);

/* 変数の終了処理を行う */
void cleanup_vars(void);

/* 変数を取得する */
int32_t get_variable(int index);

/* 変数を設定する */
void set_variable(int index, int32_t val);

/* 変数を文字列で指定して取得する */
bool get_variable_by_string(const char *var, int32_t *val);

/* 変数を文字列で指定して設定する */
bool set_variable_by_string(const char *var, int32_t val);

/* 名前変数を取得する */
const char *get_name_variable(int index);

/* 名前変数を設定する */
bool set_name_variable(int index, const char *val);

/* 名前変数の最後の文字を消去する */
void truncate_name_variable(int index);

/* 文字列の中の変数を展開して返す */
const char *expand_variable(const char *msg);

/* 文字列の中の変数を展開して返す(変数のインクリメントも行う) */
const char *expand_variable_with_increment(const char *msg, int inc);

/* ローカル変数テーブルへのポインタを取得する */
int32_t *get_local_variables_pointer(void);

/* ローカル変数テーブルへのポインタを取得する */
int32_t *get_global_variables_pointer(void);

#ifdef USE_DEBUGGER
/* 変数の値が更新されたかをチェックする */
bool check_variable_updated(void);

/* 更新された変数のインデックスを取得する */
int get_updated_variable_index(void);

/* 変数が初期値から更新されているかを調べる */
bool is_variable_changed(int index);

/* 変数の更新状態をクリアする */
void clear_variable_changed(void);
#endif

#endif
