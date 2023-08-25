/* -*- coding: utf-8; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Suika 2
 * Copyright (C) 2001-2021, TABATA Keiichi. All rights reserved.
 */

/*
 * [Changes]
 *  2016-06-16 作成
 */

#include "suika.h"

#ifdef SSE_VERSIONING
#ifndef _MSC_VER

/* SSE4.2版の描画関数を定義する */
#define DRAW_BLEND_NONE			draw_blend_none_sse42
#define DRAW_BLEND_FAST			draw_blend_fast_sse42
#define DRAW_BLEND_NORMAL		draw_blend_normal_sse42
#define DRAW_BLEND_ADD			draw_blend_add_sse42
#define DRAW_BLEND_SUB			draw_blend_sub_sse42
#include "drawimage.h"

/* SSE4.2版scale_samples()を定義する */
#define SCALE_SAMPLES scale_samples_sse42
#include "scalesamples.h"

/* SSE4.2版draw_glyph_func()を定義する */
#define DRAW_GLYPH_FUNC draw_glyph_func_sse42
#define DRAW_GLYPH_DIM_FUNC draw_glyph_dim_func_sse42
#include "drawglyph.h"

/* SSE4.2版mul_add_pcm()を定義する */
#define MUL_ADD_PCM mul_add_pcm_sse42
#include "muladdpcm.h"

#endif /* SSE_VERSIONING */
#endif /* _MSC_VER */

