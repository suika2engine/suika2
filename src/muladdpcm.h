/* -*- coding: utf-8-unix; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Suika 2
 * Copyright (C) 2001-2016, TABATA Keiichi. All rights reserved.
 */

/*
 * [Changes]
 *  2016-07-03 作成
 */

/*
 * 下記のマクロを定義してインクルードする
 *  - MUL_ADD_PCM
 *  - PROTOTYPE_ONLY
 */

#if defined(OSX) || defined(IOS) || defined(USE_SDL2) || defined(SWITCH)

/* ミキシングを行う */
void MUL_ADD_PCM(uint32_t *dst, uint32_t *src, float vol, int samples)
#ifdef PROTOTYPE_ONLY
	;
#else
{
    float scale;
    int i;
    int32_t il, ir; /* intermediate L/R */
    int16_t sl, sr; /* source L/R*/
    int16_t dl, dr; /* destination L/R */

    /* スケールファクタを指数関数にする */
    scale = (powf(10.0f, vol) - 1.0f) / (10.0f - 1.0f);

    /* 各サンプルを合成する */
    for (i = 0; i < samples; i++) {
        dl = (int16_t)(uint16_t)dst[i];
        dr = (int16_t)(uint16_t)(dst[i] >> 16);

        sl = (int16_t)(uint16_t)src[i];
        sr = (int16_t)(uint16_t)(src[i] >> 16);

        il = (int32_t)dl + (int32_t)(sl * scale);
        ir = (int32_t)dr + (int32_t)(sr * scale);

        il = il > 32767 ? 32767 : il;
        il = il < -32768 ? -32768 : il;
        ir = ir > 32767 ? 32767 : ir;
        ir = ir < -32768 ? -32768 : ir;

        dst[i] = ((uint32_t)(uint16_t)(int16_t)il) |
                 (((uint32_t)(uint16_t)(int16_t)ir) << 16);
    }
}
#endif

#undef MUL_ADD_PCM
#undef PROTOTYPE_ONLY

#endif
