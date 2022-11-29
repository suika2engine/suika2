/* -*- coding: utf-8; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Suika 2
 * Copyright (C) 2001-2021, TABATA Keiichi. All rights reserved.
 */

/*
 * [Changes]
 *  - 2016/06/28 作成
 *  - 2021/06/03 マスターボリュームを追加
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

/* ローカルセーブデータに書き込まれるべきボリュームの値 */
static float vol_local[MIXER_STREAMS];

/* グローバルボリューム */
static float vol_global[MIXER_STREAMS];

/* キャラクタごとのボリューム */
static float vol_character[CH_VOL_SLOTS];

/* どのキャラクタのボリュームを適用するか */
static int ch_vol_index;

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
		vol_local[n] = 1.0f;

		/* vol_global[n] はセーブデータからロード済み */

		set_sound_volume(n, vol_global[n]);

		/* Androidでは再利用されるので初期化する */
		is_fading[n] = false;
	}

	for (n = 0; n < CH_VOL_SLOTS; n++)
		vol_character[n] = 1.0f;

	ch_vol_index = CH_VOL_SLOT_DEFAULT;
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
		vol_local[n] = vol;
		reset_stop_watch(&sw[n]);
	} else {
		is_fading[n] = false;
		vol_cur[n] = vol;
		vol_local[n] = vol;
		set_sound_volume(n, vol_global[n] * vol);
	}
}

/*
 * ボリュームを取得する
 *  - ローカルセーブデータ用
 */
float get_mixer_volume(int n)
{
	assert(n < MIXER_STREAMS);

	return vol_local[n];
}

/*
 * グローバルボリュームを設定する
 */
void set_mixer_global_volume(int n, float vol)
{
	assert(n < MIXER_STREAMS);
	assert(vol >= 0 && vol <= 1.0f);

	vol_global[n] = vol;

	set_sound_volume(n, vol_global[n] * vol_cur[n]);
}

/*
 * グローバルボリュームを取得する
 *  - グローバルセーブデータ用
 */
float get_mixer_global_volume(int n)
{
	assert(n < MIXER_STREAMS);

	return vol_global[n];
}

/*
 * キャラ別ボリュームを設定する
 */
void set_character_volume(int n, float vol)
{
	assert(n >= 0 && n < CH_VOL_SLOTS);

	vol_character[n] = vol;
}

/*
 * キャラ別ボリュームを取得する
 */
float get_character_volume(int n)
{
	assert(n >= 0 && n < CH_VOL_SLOTS);

	return vol_character[n];
}

/*
 * キャラクタボリュームを適用する
 */
void apply_character_volume(int index)
{
	float vol;

	assert(index >= 0 && index < CH_VOL_SLOTS);

	ch_vol_index = index;

	vol = vol_global[VOICE_STREAM] * vol_local[VOICE_STREAM] *
	      vol_character[ch_vol_index];

	set_sound_volume(VOICE_STREAM, vol);
}

/*
 * サウンドを再生し終わったかを取得する
*/
bool is_mixer_sound_finished(int n)
{
	if (is_sound_finished(n))
		return true;

	return false;
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

		/* キャラクタボリュームが適用される場合 */
		if (n == VOICE_STREAM && ch_vol_index != -1)
			vol *= vol_character[ch_vol_index];

		/* ボリュームを設定する */
		vol_cur[n] = vol;
		set_sound_volume(n, vol_global[n] * vol_cur[n]);
	}
}
