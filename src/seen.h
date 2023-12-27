/* -*- coding: utf-8; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Suika2
 * Copyright (C) 2001-2021, Keiichi Tabata. All rights reserved.
 */

/*
 * [Changes]
 *  - 2021/07/31 作成
 */

#ifndef SUIKA_SEEN_H
#define SUIKA_SEEN_H

#include "types.h"

/*
 * 初期化
 */

/* 既読フラグ管理を初期化する */
bool init_seen(void);

/* 既読フラグ管理の終了処理を行う */
void cleanup_seen(void);

/*
 * 既読フラグへのアクセス
 */

/* 現在のスクリプトに対応する既読フラグをロードする */
bool load_seen(void);

/* 現在のスクリプトに対応する既読フラグをセーブする */
bool save_seen(void);

/* 現在のコマンドが既読かを返す */
bool get_seen(void);

/* 現在のコマンドを既読にする */
void set_seen(void);

#endif
