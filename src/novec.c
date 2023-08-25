/* -*- coding: utf-8; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Suika 2
 * Copyright (C) 2001-2016, TABATA Keiichi. All rights reserved.
 */

/*
 * [Changes]
 *  2016-06-16 作成
 */

#include "suika.h"

#ifdef SSE_VERSIONING

/* 非ベクトル化版の描画関数を定義する */
#define DRAW_BLEND_NONE			draw_blend_none_novec
#define DRAW_BLEND_FAST			draw_blend_fast_novec
#define DRAW_BLEND_NORMAL		draw_blend_normal_novec
#define DRAW_BLEND_ADD			draw_blend_add_novec
#define DRAW_BLEND_SUB			draw_blend_sub_novec
#include "drawimage.h"

/* 非ベクトル化版scale_samples()を宣言する */
#define SCALE_SAMPLES scale_samples_novec
#include "scalesamples.h"

/* 非ベクトル化版draw_glyph_func()を定義する */
#define DRAW_GLYPH_FUNC draw_glyph_func_novec
#define DRAW_GLYPH_DIM_FUNC draw_glyph_dim_func_novec
#include "drawglyph.h"

/* 非ベクトル版mul_add_pcm()を定義する */
#define MUL_ADD_PCM mul_add_pcm_novec
#include "muladdpcm.h"

#endif /* SSE_VERSIONING */
