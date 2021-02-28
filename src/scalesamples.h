/* -*- coding: utf-8; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Suika 2
 * Copyright (C) 2001-2016, TABATA Keiichi. All rights reserved.
 */

/*
 * [Changes]
 *  2016-06-11 作成
 */

/*
 * 下記のマクロを定義してインクルードする
 *  - SCALE_SAMPLES
 */

#if defined(LINUX) || defined(FREEBSD) || defined(NETBSD)

#include <math.h>

/* ボリュームを適用する */
void SCALE_SAMPLES(uint32_t *buf, int frames, float vol)
#ifdef PROTOTYPE_ONLY
;
#else
{
	float scale;
	uint32_t frame;
	int32_t il, ir;	/* intermediate L/R */
	int16_t sl, sr;	/* source L/R*/
	int16_t dl, dr;	/* destination L/R */
	int i;

	/* スケールファクタを指数関数にする */
	scale = (powf(10.0f, vol) - 1.0f) / (10.0f - 1.0f);

	/* 各サンプルをスケールする */
	for (i = 0; i < frames; i++) {
		frame = buf[i];

		sl = (int16_t)(uint16_t)frame;
		sr = (int16_t)(uint16_t)(frame >> 16);

		il = (int)(sl * scale);
		ir = (int)(sr * scale);

		il = il > 32767 ? 32767 : il;
		il = il < -32768 ? -32768 : il;
		ir = ir > 32767 ? 32767 : ir;
		ir = ir < -32768 ? -32768 : ir;

		dl = (int16_t)il;
		dr = (int16_t)ir;

		buf[i] = ((uint32_t)(uint16_t)dl) |
			 (((uint32_t)(uint16_t)dr) << 16);
	}
}
#endif

#undef SCALE_SAMPLES
#undef PROTOTYPE_ONLY

#endif
