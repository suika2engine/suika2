/* -*- coding: utf-8; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Suika2
 * Copyright (C) 2001-2021, Keiichi Tabata. All rights reserved.
 */

/*
 * Scnan conversion buffer
 * 走査変換バッファ
 *
 * [Changes]
 *  2021-06-16 作成
 */

#ifndef SUIKA_SCBUF_H
#define SUIKA_SCBUF_H

#include "types.h"

/*
 * 初期化
 */

/* 走査変換バッファの初期化処理をする */
bool init_scbuf(void);

/* 走査変換バッファの終了処理を行う */
void cleanup_scbuf(void);

/*
 * 走査変換
 */

/* 走査変換バッファをクリアする */
void clear_scbuf(void);

/* エッジをスキャンして最小値を設定する */
void scan_edge_min(int x1, int y1, int x2, int y2);

/* エッジをスキャンして最大値を設定する */
void scan_edge_max(int x1, int y1, int x2, int y2);

/* 指定した走査線の最小値と最大値を取得する */
void get_scan_line(int y, int *min, int *max);

#endif /* SUIKA_SCBUF_H */
