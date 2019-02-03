/* -*- coding: utf-8-with-signature; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Suika 2
 * Copyright (C) 2001-2016, TABATA Keiichi. All rights reserved.
 */

/*
 * [Changes]
 *  - 2016/06/28 作成
 */

#include "suika.h"

/* PCMストリーム */
static struct wave *pcm[MIXER_STREAMS];

/* フェード中であるか */
static bool is_fading[MIXER_STREAMS];

/* 現在ボリューム */
static float vol_cur[MIXER_STREAMS];

/* 開始ボリューム */
static float vol_start[MIXER_STREAMS];

/* 終了ボリューム */
static float vol_end[MIXER_STREAMS];

/* フェードの時間 */
static float vol_span[MIXER_STREAMS];

/* フェードの開始時刻 */
static stop_watch_t sw[MIXER_STREAMS];

/* セーブデータに書き込まれるべきボリュームの値 */
static float vol_sav[MIXER_STREAMS];

/* BGMファイル名 */
static char *bgm_file_name;

/*
 * ミキサーモジュールの初期化処理を行う 
 */
void init_mixer(void)
{
	int n;

	for (n = 0; n < MIXER_STREAMS; n++) {
		vol_cur[n] = 1.0f;
		vol_sav[n] = 1.0f;
		set_sound_volume(n, 1.0f);

		/* Androidでは再利用されるので初期化する */
		is_fading[n] = false;
	}
}

/*
 * ミキサーモジュールの初期化処理を行う 
 */
void cleanup_mixer(void)
{
	int n;

	for (n = 0; n < MIXER_STREAMS; n++) {
		stop_sound(n);
		if (pcm[n] != NULL) {
			destroy_wave(pcm[n]);
			pcm[n] = NULL;
		}
	}

	free(bgm_file_name);
	bgm_file_name = NULL;
}

/*
 * BGMのファイル名を設定する
 */
bool set_bgm_file_name(const char *file)
{
	if (bgm_file_name != NULL)
		free(bgm_file_name);

	if (file == NULL) {
		bgm_file_name = NULL;
	} else {
		bgm_file_name = strdup(file);
		if (bgm_file_name == NULL) {
			log_memory();
			return false;
		}
	}

	return true;
}

/*
 * BGMのファイル名を取得する
 */
const char *get_bgm_file_name(void)
{
	return bgm_file_name;
}

/*
 * サウンドを再生・停止する
 *  - play_sound(), stop_sound() は直接呼び出さないこと
 */
void set_mixer_input(int n, struct wave *w)
{
	struct wave *old_pcm;

	assert(n < MIXER_STREAMS);

	old_pcm = pcm[n];
	if (old_pcm != NULL) {
		stop_sound(n);
		pcm[n] = NULL;
		destroy_wave(old_pcm);
	}

	if (w != NULL) {
		play_sound(n, w);
		pcm[n] = w;
	}
}

/*
 * ボリュームを設定する
 */
void set_mixer_volume(int n, float vol, float span)
{
	assert(n < MIXER_STREAMS);
	assert(vol >= 0 && vol <= 1.0f);

	if (span > 0) {
		is_fading[n] = true;
		vol_start[n] = vol_cur[n];
		vol_end[n] = vol;
		vol_span[n] = span;
		vol_sav[n] = vol;
		reset_stop_watch(&sw[n]);
	} else {
		is_fading[n] = false;
		vol_cur[n] = vol;
		vol_sav[n] = vol;
		set_sound_volume(n, vol);
	}
}

/*
 * ボリュームを取得する
 *  - セーブ用
 */
float get_mixer_volume(int n)
{
	assert(n < MIXER_STREAMS);

	return vol_sav[n];
}

/*
 * サウンドのフェード処理を実行する
 *  - 毎フレーム呼び出される
 */
void process_sound_fading(void)
{
	int n;
	float lap, vol;

	for (n = 0; n < MIXER_STREAMS; n++) {
		if (!is_fading[n])
			continue;

		/* 経過時刻を取得する */
		lap = (float)get_stop_watch_lap(&sw[n]) / 1000.0f;
		if (lap >= vol_span[n]) {
			lap = vol_span[n];
			is_fading[n] = false;
		}

		/* 現在のボリュームを求める */
		vol = vol_start[n] * (1.0f - lap / vol_span[n]) +
		      vol_end[n] * (lap / vol_span[n]);

		/* ボリュームを設定する */
		vol_cur[n] = vol;
		set_sound_volume(n, vol);
	}
}
