/* -*- coding: utf-8; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Suika 2
 * Copyright (C) 2001-2021, TABATA Keiichi. All rights reserved.
 */

#ifndef SUIKA_EMOPENAL_H
#define SUIKA_EMOPENAL_H

#include "types.h"

/* OpenALの初期化処理を行う */
bool init_openal(void);

/* OpenALの終了処理を行う */
void cleanup_openal(void);

/* サウンドのバッファを埋める */
void fill_sound_buffer(void);

#endif
