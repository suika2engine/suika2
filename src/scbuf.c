/* -*- coding: utf-8; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Suika 2
 * Copyright (C) 2001-2021, TABATA Keiichi. All rights reserved.
 */

/*
 * Scnan conversion buffer
 *
 * 走査変換バッファ
 *  - Suika2は2D/3Dアクセラレータを用いないので、自前で用意している
 *  - 現状では塗り潰し専用で、Z座標とテクスチャ座標を保持していない
 *
 * [Changes]
 *  2021-06-16 作成
 */

#include "suika.h"

/*
 * 走査変換バッファ
 */
struct sc_line {
	int min, max;
} *sc_line;

/*
 * 初期化
 */

/*
 * 走査変換バッファの初期化をする
 */
bool init_scbuf(void)
{
	if (sc_line != NULL) {
		free(sc_line);
		sc_line = NULL;
	}

	/* 走査変換バッファを画面の高さだけ取得する */
	sc_line = malloc(sizeof(struct sc_line) * (size_t)conf_window_height);
	if (sc_line == NULL) {
		log_memory();
		return false;
	}

	return true;
}

/*
 * 走査変換バッファの終了処理を行う
 */
void cleanup_scbuf(void)
{
	if (sc_line != NULL) {
		free(sc_line);
		sc_line = NULL;
	}
}

/*
 * 走査変換
 */

/*
 * 走査変換バッファをクリアする
 */
void clear_scbuf(void)
{
	int i;
	for (i = 0; i < conf_window_height; i++) {
		sc_line[i].min = conf_window_width;
		sc_line[i].max = -1;
	}
}

static INLINE void int_swap(int *a, int *b)
{
	int tmp;
	tmp = *a;
	*a = *b;
	*b = tmp;
}

/*
 * エッジをスキャンして最小値を設定する
 */
void scan_edge_min(int x1, int y1, int x2, int y2)
{
	float x, delta_x;
	int y;

	/* X軸と平行な場合: 未対応 */
	if (y1 == y2)
		return;

	/*
	 * 傾きがあるかY軸に平行な場合
	 */

	if (y1 > y2) {
		int_swap(&y1, &y2);
		int_swap(&x1, &x2);
	}

	delta_x = (float)(x2 - x1) / (float)(y2 - y1);
	for (x = (float)x1, y = y1; y <= y2; x += delta_x, y++)
		if (y >= 0 && y < conf_window_height)
			sc_line[y].min = (int)x;
}

/*
 * エッジをスキャンして最大値を設定する
 */
void scan_edge_max(int x1, int y1, int x2, int y2)
{
	float x, delta_x;
	int y;

	/* X軸と平行な場合: 未対応 */
	if (y1 == y2)
		return;

	/*
	 * 傾きがあるかY軸に平行な場合
	 */

	if (y1 > y2) {
		int_swap(&y1, &y2);
		int_swap(&x1, &x2);
	}

	delta_x = (float)(x2 - x1) / (float)(y2 - y1);
	for (x = (float)x1, y = y1; y <= y2; x += delta_x, y++)
		if (y >= 0 && y < conf_window_height)
			sc_line[y].max = (int)x;
}

/*
 * 指定した走査線の最小値と最大値を取得する
 */
void get_scan_line(int y, int *min, int *max)
{
	assert(y >= 0 && y < conf_window_height);

	*min = sc_line[y].min;
	*max = sc_line[y].max;
}
