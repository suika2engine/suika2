/* -*- coding: utf-8; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Suika 2
 * Copyright (C) 2001-2016, TABATA Keiichi. All rights reserved.
 */

#ifndef SUIKA_WAVE_H
#define SUIKA_WAVE_H

#include "types.h"

/*
 * 44.1kHz 16bit stereoのPCMストリーム
 */
struct wave;

/* ファイルからPCMストリームを作成する */
struct wave *create_wave_from_file(const char *dir, const char *file,
				   bool loop);

/* PCMストリームを破棄する */
void destroy_wave(struct wave *w);

/* PCMストリームのループ回数を設定する */
void set_wave_repeat_times(struct wave *w, int n);

/* PCMストリームが終端に達しているか取得する */
bool is_wave_eos(struct wave *w);

/* PCMストリームからサンプルを取得する */
int get_wave_samples(struct wave *w, uint32_t *, int samples);

/* PCMバッファにボリュームを適用する */
void scale_samples(uint32_t *buf, int frames, float vol);

/* PCMストリームのファイル名を取得する(NDK) */
const char *get_wave_file_name(struct wave *w);

/* PCMストリームがループ再生されるかを取得する(NDK) */
bool is_wave_looped(struct wave *w);

#endif
