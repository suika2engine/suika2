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

/* テクスチャをロックする */
bool lock_texture(int width, int height, pixel_t *pixels,
		  pixel_t **locked_pixels, void **texture);

/* テクスチャをアンロックする */
void unlock_texture(int width, int height, pixel_t *pixels,
		    pixel_t **locked_pixels, void **texture);

/* テクスチャを破棄する */
void destroy_texture(void *texture);

/* 画面にイメージをレンダリングする */
void render_image(int dst_left, int dst_top, struct image * RESTRICT src_image,
		  int width, int height, int src_left, int src_top, int alpha,
		  int bt);

/* 画面にイメージをマスク描画でレンダリングする */
void render_image_mask(int dst_left, int dst_top,
		       struct image * RESTRICT src_image, int width,
		       int height, int src_left, int src_top, int mask);

/* 画面をクリアする */
void render_clear(int left, int top, int width, int height, pixel_t color);

/* タイマをリセットする */
void reset_stop_watch(stop_watch_t *t);

/* タイマのラップをミリ秒単位で取得する */
int get_stop_watch_lap(stop_watch_t *t);

/* サウンドを再生を開始する */
bool play_sound(int stream, struct wave *w);

/* サウンドの再生を停止する */
bool stop_sound(int stream);

/* サウンドのボリュームを設定する */
bool set_sound_volume(int stream, float vol);

/* サウンドが再生終了したか調べる */
bool is_sound_finished(int stream);

/* 終了ダイアログを表示する */
bool exit_dialog(void);

/* タイトルに戻るダイアログを表示する */
bool title_dialog(void);

/* ビデオを再生する */
bool play_video(const char *fname);

#endif
