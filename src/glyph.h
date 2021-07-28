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

#endif
