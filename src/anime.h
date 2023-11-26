/* -*- coding: utf-8; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Suika 2
 * Copyright (C) 2001-2023, TABATA Keiichi. All rights reserved.
 */

#ifndef SUIKA_ANIME_H
#define SUIKA_ANIME_H

#include "types.h"
#include "image.h"

/*
 * アニメーションレイヤーのインデックス
 */
enum anime_layer {
	ANIME_LAYER_BG,
	ANIME_LAYER_BG2,
	ANIME_LAYER_EFFECT5,
	ANIME_LAYER_EFFECT6,
	ANIME_LAYER_EFFECT7,
	ANIME_LAYER_EFFECT8,
	ANIME_LAYER_CHB,
	ANIME_LAYER_CHL,
	ANIME_LAYER_CHLC,
	ANIME_LAYER_CHR,
	ANIME_LAYER_CHRC,
	ANIME_LAYER_CHC,
	ANIME_LAYER_EFFECT1,
	ANIME_LAYER_EFFECT2,
	ANIME_LAYER_EFFECT3,
	ANIME_LAYER_EFFECT4,
	ANIME_LAYER_MSG,
	ANIME_LAYER_NAME,
	ANIME_LAYER_CHF,
	ANIME_LAYER_TEXT1,
	ANIME_LAYER_TEXT2,
	ANIME_LAYER_TEXT3,
	ANIME_LAYER_TEXT4,
	ANIME_LAYER_TEXT5,
	ANIME_LAYER_TEXT6,
	ANIME_LAYER_TEXT7,
	ANIME_LAYER_TEXT8,
	ANIME_LAYER_COUNT
};

/*
 * アニメーションの加速
 */
enum anime_accel {
	ANIME_ACCEL_UNIFORM,
	ANIME_ACCEL_ACCEL,
	ANIME_ACCEL_DEACCEL,
};

/* アニメーションサブシステムに関する初期化処理を行う */
bool init_anime(void);

/* アニメーションサブシステムに関する終了処理を行う */
void cleanup_anime(void);

/* アニメーションファイルを読み込む */
bool load_anime_from_file(const char *fname);

/* アニメーションシーケンスをクリアする */
void clear_anime_sequence(int layer);

/* アニメーションシーケンスを開始する */
bool new_anime_sequence(int layer);

/* アニメーションシーケンスにプロパティを追加する */
bool add_anime_sequence_property_f(const char *key, float val);
bool add_anime_sequence_property_i(const char *key, int val);

/* 指定したレイヤのアニメーションを開始する */
bool start_layer_anime(int layer);

/* 指定したレイヤのアニメーションを完了する */
bool finish_layer_anime(int layer);

/* 実行中のアニメーションがあるか調べる */
bool is_anime_running(void);

/* 実行中のアニメーションがあるか調べる */
bool is_anime_running_for_layer(int layer);

/* 完了したアニメーションがあるか調べる */
bool is_anime_finished_for_layer(int layer);

/* アニメーションのフレーム時刻を更新する */
void update_anime_time(void);

/* レイヤのパラメータを取得する */
bool
get_anime_layer_params(
	int layer,
	struct image **image,
	char **file,
	int *x,
	int *y,
	int *alpha);

/* アニメレイヤの座標を更新する */
void set_anime_layer_position(int anime_layer, int x, int y);

#endif
