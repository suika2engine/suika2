/* -*- coding: utf-8-with-signature; tab-width: 8; indent-tabs-mode: t; -*- */

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

/* AVX版の描画関数を定義する */
#define DRAW_BLEND_NONE			draw_blend_none_avx
#define DRAW_BLEND_FAST			draw_blend_fast_avx
#define DRAW_BLEND_NORMAL		draw_blend_normal_avx
#define DRAW_BLEND_ADD			draw_blend_add_avx
#define DRAW_BLEND_SUB			draw_blend_sub_avx
#include "drawimage.h"

/* AVX版scale_samples()を定義する */
#define SCALE_SAMPLES scale_samples_avx
#include "scalesamples.h"

/* AVX版draw_glyph_func()を定義する */
#define DRAW_GLYPH_FUNC draw_glyph_func_avx
#include "drawglyph.h"

/* AVX版mul_add_pcm()を定義する */
#define MUL_ADD_PCM mul_add_pcm_avx
#include "muladdpcm.h"

#endif /* SSE_VERSIONING */

