/* -*- coding: utf-8; indent-tabs-mode: t; tab-width: 8; c-basic-offset: 8; -*- */

/*
 * Suika 2
 * Copyright (C) 2001-2016, TABATA Keiichi. All rights reserved.
 */

/*
 * freetype2 レンダリング
 *
 * [Changes]
 *  - 2016/06/18 作成
 */

#ifndef SUIKA_GLYPH_H
#define SUIKA_GLYPH_H

#include "image.h"

/* Unicodeコードポイント */
#define CHAR_TOUTEN		(0x3001)
#define CHAR_KUTEN		(0x3002)
#define CHAR_YENSIGN		(0x00a5)

/* フォントレンダラの初期化処理を行う */
bool init_glyph(void);

/* フォントレンダラの終了処理を行う */
void cleanup_glyph(bool no_free_file_names);

/* utf-8文字列の先頭文字をutf-32文字に変換する */
#ifndef SWITCH /* switch.h にutf8_to_utf32は既に存在します　*/
int utf8_to_utf32(const char *mbs, uint32_t *wc);
#endif

/* utf-8文字列の文字数を返す */
int utf8_chars(const char *mbs);

/* 文字を描画した際の幅を取得する */
int get_glyph_width(uint32_t codepoint);

/* 文字を描画した際の高さを取得する */
int get_glyph_height(uint32_t codepoint);

/* utf-8文字列を描画した際の幅を取得する */
int get_utf8_width(const char *mbs);

/* 文字の描画を行う */
bool draw_glyph(struct image *img, int x, int y, pixel_t color,
		pixel_t outline_color, uint32_t codepoint, int *w, int *h,
		int base_font_size, bool is_dim);

/* グローバルのフォントファイル名を設定する */
bool set_global_font_file_name(const char *file);

/* グローバルのフォントファイル名を取得する */
const char *get_global_font_file_name(void);

/* ローカルのフォントファイル名を設定する */
bool set_local_font_file_name(const char *file);

/* 現在のフォントファイル名を取得する */
const char *get_font_file_name(void);

/* サポートされているアルファベットか調べる */
bool isgraph_extended(const char **mbs, uint32_t *wc);

/* フォントサイズを変更する */
bool set_font_size(int size);

/* フォントサイズを取得する */
int get_font_size(void);

#endif
