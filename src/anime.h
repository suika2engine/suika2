/* -*- coding: utf-8; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Suika 2
 * Copyright (C) 2001-2021, TABATA Keiichi. All rights reserved.
 */

#ifndef SUIKA_ANIME_H
#define SUIKA_ANIME_H

#include "types.h"
#include "image.h"

/* アニメーションレイヤーのインデックス */
enum layer {
	ANIME_LAYER_BG,
	ANIME_LAYER_CHB,
	ANIME_LAYER_CHL,
	ANIME_LAYER_CHR,
	ANIME_LAYER_CHC,
	ANIME_LAYER_MSG,
	ANIME_LAYER_NAME,
	ANIME_LAYER_CHF,
	ANIME_LAYER_COUNT
};

/* アニメーションサブシステムに関する初期化処理を行う */
bool init_anime(void);

/* アニメーションサブシステムに関する終了処理を行う */
void cleanup_anime(void);

/* アニメーションシーケンスを開始する */
bool start_anime_sequence(void);

/* アニメーションシーケンスにプロパティを追加する */
bool add_anime_sequence_property(const char *key, float val);

/* 指定したレイヤのアニメーションを開始する */
bool start_layer_anime(const char *layer);

/* 実行中のアニメーションがあるか調べる */
bool is_anime_running(void);

/* アニメーションのフレームを更新する */
void update_anime_frame(void);

/* レイヤのパラメータを取得する */
void
get_anime_layer_params(
	int layer,
	struct image **image,
	char **file,
	int *x,
	int *y,
	int *alpha);

#endif
