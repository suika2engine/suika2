/* -*- coding: utf-8-with-signature; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Suika 2
 * Copyright (C) 2001-2021, TABATA Keiichi. All rights reserved.
 */

/*
 * [Changes]
 *  - 2016/06/28 作成
 *  - 2021/06/03 マスターボリュームを追加
 */

#ifndef SUIKA_MIXER_H
#define SUIKA_MIXER_H

#include "wave.h"

/*
 * ミキサのストリーム
 */
#define MIXER_STREAMS	(3)
#define BGM_STREAM	(0)
#define VOICE_STREAM	(1)
#define SE_STREAM	(2)

/* ミキサーモジュールの初期化処理を行う */
void init_mixer(void);

/* ミキサーモジュールの初期化処理を行う */
void cleanup_mixer(void);

/* BGMのファイル名を設定する */
bool set_bgm_file_name(const char *file);

/* BGMのファイル名を取得する */
const char *get_bgm_file_name(void);

/* サウンドを再生・停止する */
void set_mixer_input(int n, struct wave *w);

/* ボリュームを設定する */
void set_mixer_volume(int n, float vol, float span);

/* ボリュームを取得する */
float get_mixer_volume(int n);

/* マスターボリュームを設定する */
void set_mixer_master_volume(int n, float vol);

/* マスターボリュームを取得する */
float get_mixer_master_volume(int n);

/* サウンドのフェード処理を実行する */
void process_sound_fading(void);

#endif
