/* -*- coding: utf-8-with-signature; tab-width: 8; indent-tabs-mode: t; -*- */

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

/* SSE3版の描画関数を定義する */
#define DRAW_BLEND_NONE			draw_blend_none_sse3
#define DRAW_BLEND_FAST			draw_blend_fast_sse3
#define DRAW_BLEND_NORMAL		draw_blend_normal_sse3
#define DRAW_BLEND_ADD			draw_blend_add_sse3
#define DRAW_BLEND_SUB			draw_blend_sub_sse3
#include "drawimage.h"

/* SSE3版scale_samples()を定義する */
#define SCALE_SAMPLES scale_samples_sse3
#include "scalesamples.h"

/* SSE3版draw_glyph_func()を定義する */
#define DRAW_GLYPH_FUNC draw_glyph_func_sse3
#include "drawglyph.h"

/* SSE3版mul_add_pcm()を定義する */
#define MUL_ADD_PCM mul_add_pcm_sse3
#include "muladdpcm.h"

#endif /* SSE_VERSIONING */
#endif /* _MSC_VER */
