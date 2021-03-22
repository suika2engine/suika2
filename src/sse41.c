/* -*- tab-width: 8; indent-tabs-mode: t; -*- */

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

/* SSE4.1版の描画関数を定義する */
#define DRAW_BLEND_NONE			draw_blend_none_sse41
#define DRAW_BLEND_FAST			draw_blend_fast_sse41
#define DRAW_BLEND_NORMAL		draw_blend_normal_sse41
#define DRAW_BLEND_ADD			draw_blend_add_sse41
#define DRAW_BLEND_SUB			draw_blend_sub_sse41
#include "drawimage.h"

/* SSE4.1版convert_to_integer()を定義する */
#define SCALE_SAMPLES scale_samples_sse41
#include "scalesamples.h"

/* SSE4.1版draw_glyph_func()を定義する */
#define DRAW_GLYPH_FUNC draw_glyph_func_sse41
#include "drawglyph.h"

/* SSE4.1版mul_add_pcm()を定義する */
#define MUL_ADD_PCM mul_add_pcm_sse41
#include "muladdpcm.h"

#endif /* SSE_VERSIONING */
#endif /* _MSC_VER */
