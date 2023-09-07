/* -*- coding: utf-8; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Suika 2
 * Copyright (C) 2001-2021, TABATA Keiichi. All rights reserved.
 */

/*
 * [Changes]
 *  2021-08-06 Created.
 */

#ifndef SUIKA_EMGLRENDER_H
#define SUIKA_EMGLRENDER_H

#include "suika.h"

/* OpenGLの初期化処理を行う */
bool init_opengl(void);

/* OpenGLの終了処理を行う */
void cleanup_opengl(void);

/* フレームのレンダリングを開始する */
void opengl_start_rendering(void);

/* フレームのレンダリングを終了する */
void opengl_end_rendering(void);

/* テクスチャをロックする */
bool opengl_lock_texture(int width, int height, pixel_t *pixels,
			 pixel_t **locked_pixels, void **texture);

/* テクスチャをアンロックする */
void opengl_unlock_texture(int width, int height, pixel_t *pixels,
			   pixel_t **locked_pixels, void **texture);

/* テクスチャを破棄する */
void opengl_destroy_texture(void *texture);

/* 画面にイメージをレンダリングする */
void opengl_render_image(int dst_left, int dst_top,
			 struct image * RESTRICT src_image, int width,
			 int height, int src_left, int src_top, int alpha,
			 int bt);

/* 画面にイメージを暗くレンダリングする */
void opengl_render_image_dim(int dst_left, int dst_top,
			     struct image * RESTRICT src_image, int width,
			     int height, int src_left, int src_top);

/* 画面にイメージをルール付きでレンダリングする */
void opengl_render_image_rule(struct image * RESTRICT src_image,
			      struct image * RESTRICT rule_image,
			      int threshold);

/* 画面にイメージをルール付き(メルト)でレンダリングする */
void opengl_render_image_melt(struct image * RESTRICT src_image,
			      struct image * RESTRICT rule_image,
			      int threshold);

#ifdef WIN
/* 全画面表示のときのスクリーンオフセットを指定する */
void opengl_set_screen(int x, int y, int w, int h);
#endif

#endif
