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

/* テクスチャを更新する */
void opengl_notify_image_update(struct image *img);

/* テクスチャを破棄する */
void opengl_notify_image_free(struct image *img);

/* 画面にイメージをレンダリングする */
void opengl_render_image_copy(int dst_left, int dst_top,
			      struct image *src_image, int width,
			      int height, int src_left, int src_top);

/* 画面にイメージをレンダリングする */
void opengl_render_image_normal(int dst_left, int dst_top,
				struct image *src_image, int width,
				int height, int src_left, int src_top, int alpha);

/* 画面にイメージをレンダリングする */
void opengl_render_image_add(int dst_left, int dst_top,
			     struct image *src_image, int width,
			     int height, int src_left, int src_top, int alpha);

/* 画面にイメージを暗くレンダリングする */
void opengl_render_image_dim(int dst_left, int dst_top,
			     struct image *src_image, int width,
			     int height, int src_left, int src_top);

/* 画面にイメージをルール付きでレンダリングする */
void opengl_render_image_rule(struct image *src_image,
			      struct image *rule_image,
			      int threshold);

/* 画面にイメージをルール付き(メルト)でレンダリングする */
void opengl_render_image_melt(struct image *src_image,
			      struct image *rule_image,
			      int progress);

/* 全画面表示のときのスクリーンオフセットを指定する */
void opengl_set_screen(int x, int y, int w, int h);

#endif
