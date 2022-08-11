/* -*- coding: utf-8-with-signature; indent-tabs-mode: t; tab-width: 8; c-basic-offset: 8; -*- */

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
#define CHAR_SPACE		(0x0020)
#define CHAR_COMMA 		(0x002c)
#define CHAR_PERIOD		(0x002e)
#define CHAR_COLON		(0x003a)
#define CHAR_SEMICOLON  	(0x003b)
#define CHAR_TOUTEN		(0x3001)
#define CHAR_KUTEN		(0x3002)
#define CHAR_BACKSLASH		(0x005c)
#define CHAR_YENSIGN		(0x00a5)
#define CHAR_SMALLN		(0x006e)

/* フォントレンダラの初期化処理を行う */
bool init_glyph(void);

/* フォントレンダラの終了処理を行う */
void cleanup_glyph(void);

/* utf-8文字列の先頭文字をutf-32文字に変換する */
int utf8_to_utf32(const char *mbs, uint32_t *wc);

/* utf-8文字列の文字数を返す */
int utf8_chars(const char *mbs);

/* 文字を描画した際の幅を取得する */
int get_glyph_width(uint32_t codepoint);

/* utf-8文字列を描画した際の幅を取得する */
int get_utf8_width(const char *mbs);

/* 文字の描画を行う */
bool draw_glyph(struct image *img, int x, int y, pixel_t color,
		pixel_t outline_color, uint32_t codepoint, int *w, int *h);

/* フォントファイル名を設定する */
bool set_font_file_name(const char *file);

/* フォントファイル名を取得する */
const char *get_font_file_name(void);

/* サポートされているアルファベットか調べる */
bool isgraph_extended(const char **mbs, uint32_t *wc);

#endif
