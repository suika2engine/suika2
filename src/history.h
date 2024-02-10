/* -*- coding: utf-8; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Suika2
 * Copyright (C) 2001-2012, Keiichi Tabata. All rights reserved.
 */

/*
 * ヒストリ管理
 *
 * [Changes]
 *  - 2016/07/10 作成
 *  - 2022/08/08 GUIに機能を移管
 */

#ifndef SUIKA_HISTORY_H
#define SUIKA_HISTORY_H

#include "types.h"

/* ヒストリに関する初期化処理を行う */
bool init_history(void);

/* ヒストリに関する終了処理を行う */
void cleanup_history(void);

/* メッセージを登録する */
bool register_message(const char *name, const char *msg, const char *voice,
		      pixel_t body_color, pixel_t body_outline_color,
		      pixel_t name_color, pixel_t name_outline_color);

/* メッセージを末尾に追記する */
bool append_message(const char *msg);

/* ヒストリをクリアする(ロード時) */
void clear_history(void);

/* ヒストリの数を取得する */
int get_history_count(void);

/* ヒストリを取得する */
const char *get_history_message(int index);

/* ヒストリのボイスを取得する */
const char *get_history_voice(int index);

/* セリフがカッコで始まりカッコで終わるかチェックする */
bool is_quoted_serif(const char *msg);

#endif
