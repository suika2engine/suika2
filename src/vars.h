/* -*- coding: utf-8-with-signature; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Suika 2
 * Copyright (C) 2001-2017, TABATA Keiichi. All rights reserved.
 */

/*
 * [Changes]
 *  - 2016/06/29 作成
 *  - 2017/08/17 グローバル変数に対応
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

/*
 * グローバル変数のインデックスのオフセット
 */
#define GLOBAL_VAR_OFFSET	LOCAL_VAR_SIZE

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

/* 文字列の中の変数を展開して返す */
char *expand_variable(const char *msg);

#ifdef USE_DEBUGGER
/* 変数の値が更新されたかをチェックする */
bool check_variable_updated(void);

/* 更新された変数のインデックスを取得する */
int get_updated_variable_index(void);

/* 変数が初期値から更新されているかを調べる */
bool is_variable_changed(int index);
#endif

#endif
