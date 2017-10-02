/* -*- coding: utf-8-with-signature; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Suika 2
 * Copyright (C) 2001-2016, TABATA Keiichi. All rights reserved.
 */

#ifndef SUIKA_PLATFORM_H
#define SUIKA_PLATFORM_H

#include "types.h"

/* イメージ */
struct image;

/* PCMストリーム */
struct wave;

/*
 * ログを出力する
 *  - ログを出力してよいのはメインスレッドのみとする
 *  - ネイティブの文字コードを渡すこととする
 */
bool log_info(const char *s, ...);
bool log_warn(const char *s, ...);
bool log_error(const char *s, ...);

/*
 * UTF-8のメッセージをネイティブの文字コードに変換する
 *  - 変換の必要のない場合は引数をそのまま返す
 */
const char *conv_utf8_to_native(const char *utf8_message);

/* セーブディレクトリを作成する */
bool make_sav_dir(void);

/* データのディレクトリ名とファイル名を指定して有効なパスを取得する */
char *make_valid_path(const char *dir, const char *fname);

/* バックイメージを取得する */
struct image *get_back_image(void);

/* タイマをリセットする */
void reset_stop_watch(stop_watch_t *t);

/* タイマのラップをミリ秒単位で取得する */
int get_stop_watch_lap(stop_watch_t *t);

/* サウンドを再生を開始する */
bool play_sound(int n, struct wave *w);

/* サウンドの再生を停止する */
bool stop_sound(int n);

/* サウンドのボリュームを設定する */
bool set_sound_volume(int n, float vol);

#endif
