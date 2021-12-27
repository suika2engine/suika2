/* -*- tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Suika 2
 * Copyright (C) 2001-2016, TABATA Keiichi. All rights reserved.
 */

/*
 * [Changes]
 *  2016-06-06 作成
 */

#include "suika.h"

#include <pthread.h>
#include <alsa/asoundlib.h>

#ifdef SSE_VERSIONING
#include "x86.h"
#endif

/*
 * フォーマット
 */
#define SAMPLING_RATE	(44100)
#define CHANNELS	(2)
#define DEPTH		(16)
#define FRAME_SIZE	(4)

/*
 * 再生バッファ
 */
#define BUF_FRAMES		(SAMPLING_RATE / 8)
#define BUF_SIZE		(BUF_FRAMES * FRAME_SIZE)
#define PERIODS			(4)
#define PERIOD_FRAMES		(BUF_FRAMES / PERIODS)
#define PERIOD_SIZE		(BUF_SIZE / PERIODS)
#define PERIOD_FRAMES_PAD	((PERIOD_SIZE + 63) / 64 * 64 - PERIOD_SIZE)

/*
 * ストリームごとのデータ
 */

/* ALSAデバイス */
static snd_pcm_t *pcm[MIXER_STREAMS];

/* 入力ストリーム */
static struct wave *wave[MIXER_STREAMS];

/* サウンドスレッド */
static pthread_t thread[MIXER_STREAMS];

/* メインスレッドとサウンドスレッドの排他制御用ミューテックス */
static pthread_mutex_t mutex[MIXER_STREAMS];

/* メインスレッドからサウンドスレッドへの要求用条件変数 */
static pthread_cond_t req[MIXER_STREAMS];

/* サウンドスレッドからメインスレッドへの応答用条件変数 */
static pthread_cond_t ack[MIXER_STREAMS];

/* バッファ */
#ifndef SSE_VERSIONING
static uint32_t period_buf[MIXER_STREAMS][PERIOD_FRAMES + PERIOD_FRAMES_PAD];
#else
ALIGN_DECL(SSE_ALIGN, static uint32_t period_buf[MIXER_STREAMS][PERIOD_FRAMES +
							PERIOD_FRAMES_PAD]);
#endif

/* 使用終了の要求に使うフラグ */
static bool quit[MIXER_STREAMS];

/* ボリューム */
static float volume[MIXER_STREAMS];

/* 再生終了フラグ */
static bool finish[MIXER_STREAMS];

/*
 * 前方参照
 */
static bool init_pcm(int n);
static void *sound_thread(void *p);
static bool playback_period(int n);

/*
 * ALSAの初期化処理を行う
 */
bool init_asound(void)
{
	int n, ret;

	for (n = 0; n < MIXER_STREAMS; n++) {
		/* ストリームごとのデータを初期化する */
		pcm[n] = NULL;
		wave[n] = NULL;
		quit[n] = false;
		volume[n] = 1.0f;
		finish[n] = false;

		/* デバイスを初期化する */
		if (!init_pcm(n))
			return false;

		/* ミューテックスを作成する */
		pthread_mutex_init(&mutex[n], NULL);

		/* 条件変数を作成する */
		pthread_cond_init(&req[n], NULL);
		pthread_cond_init(&ack[n], NULL);

		pthread_mutex_lock(&mutex[n]);
		{
			/* スレッドを開始する */
			ret = pthread_create(&thread[n], NULL, sound_thread,
					     (void *)(intptr_t)n);
			if (ret != 0) {
				pthread_mutex_unlock(&mutex[n]);
				return false;
			}

			/* 待ち状態に入ったことの応答を受信する */
			pthread_cond_wait(&ack[n], &mutex[n]);
		}
		pthread_mutex_unlock(&mutex[n]);
	}

	return true;
}

/*
 * ALSAの終了処理を行う
 */
void cleanup_asound(void)
{
	void *p1;
	int n;

	for (n = 0; n < MIXER_STREAMS; n++) {
		/* 再生を終了する */
		stop_sound(n);

		pthread_mutex_lock(&mutex[n]);
		{
			/* 使用終了の通知を行う */
			quit[n] = true;
			pthread_cond_signal(&req[n]);
		}
		pthread_mutex_unlock(&mutex[n]);

		/* スレッドの終了を待つ */
		pthread_join(thread[n], &p1);

		/* デバイスをクローズする */
		if (pcm[n] != NULL)
			snd_pcm_close(pcm[n]);

		/* 条件変数を破棄する */
		pthread_cond_destroy(&req[n]);

		/* ミューテックスを破棄する */
		pthread_mutex_destroy(&mutex[n]);
	}
}

/*
 * サウンドを再生を開始する
 */
bool play_sound(int n, struct wave *w)
{
	assert(n < MIXER_STREAMS);
	assert(w != NULL);

	/* 再生中であれば停止する */
	stop_sound(n);

	pthread_mutex_lock(&mutex[n]);
	{
		/* PCMストリームを設定する */
		wave[n] = w;

		/* 再生終了状態をリセットする */
		finish[n] = false;

		/* 再生開始の要求を行う */
		pthread_cond_signal(&req[n]);
	}
	pthread_mutex_unlock(&mutex[n]);
	return true;
}

/*
 * サウンドの再生を停止する
 */
bool stop_sound(int n)
{
	assert(n < MIXER_STREAMS);

	pthread_mutex_lock(&mutex[n]);
	{
		if (wave[n] != NULL) {
			/* 再生状態を取り消す */
			wave[n] = NULL;

			/* バッファの再生を途中でやめる */
			snd_pcm_drop(pcm[n]);

			/* 待ち状態に入ったことの応答を受信する */
			pthread_cond_wait(&ack[n], &mutex[n]);
		}
	}
	pthread_mutex_unlock(&mutex[n]);
	return true;
}

/*
 * サウンドのボリュームを設定する
 */
bool set_sound_volume(int n, float vol)
{
	assert(n < MIXER_STREAMS);
	assert(vol >= 0 && vol <= 1.0f);

	/*
	 * FIXME: POSIXのセマンティクスに厳密であるためにはロックが必要だが、
	 *        現実のコンシステンシモデルでは不要である。ロックを取ると、
	 *        だいたい1/4秒待つことになるため、取らないことにした。
	 */
/*
 *	pthread_mutex_lock(&mutex[n]);
 *	{
 */
		volume[n] = vol;
/* 
 *	}
 *	pthread_mutex_unlock(&mutex[n]);
 */
	return true;
}

/*
 * サウンドが再生終了したか調べる
 */
bool is_sound_finished(int n)
{
	if (finish[n])
		return true;

	return false;
}

/* デバイスを初期化する */
static bool init_pcm(int n)
{
	snd_pcm_hw_params_t *params;
	snd_pcm_uframes_t frames;
	int ret;

	/* デバイスをオープンする */
	ret = snd_pcm_open(&pcm[n], "default", SND_PCM_STREAM_PLAYBACK, 0);
	if (ret < 0) {
		log_api_error("snd_pcm_open");
		return false;
	}

	/*
	 * フォーマットを設定する
	 */

	snd_pcm_hw_params_alloca(&params);
	ret = snd_pcm_hw_params_any(pcm[n], params);
	if (ret < 0) {
		log_api_error("snd_pcm_hw_params_any");
		return false;
	}

	if (snd_pcm_hw_params_set_access(pcm[n], params,
					 SND_PCM_ACCESS_RW_INTERLEAVED) < 0) {
		log_api_error("snd_pcm_hw_params_set_access");
		return false;
	}
	if (snd_pcm_hw_params_set_format(pcm[n], params,
					 SND_PCM_FORMAT_S16_LE) < 0) {
		log_api_error("snd_pcm_hw_params_set_format");
		return false;
	}
	if (snd_pcm_hw_params_set_rate(pcm[n], params, SAMPLING_RATE, 0) < 0) {
		log_api_error("snd_pcm_hw_params_set_rate");
		return false;
	}
	if (snd_pcm_hw_params_set_channels(pcm[n], params, 2) < 0) {
		log_api_error("snd_pcm_hw_params_set_channels");
		return false;
	}
	if (snd_pcm_hw_params_set_periods(pcm[n], params, PERIODS, 0) < 0) {
		log_api_error("snd_pcm_hw_params_set_periods");
		return false;
	}
#if defined(LINUX)
	if (snd_pcm_hw_params_set_buffer_size(pcm[n], params, BUF_FRAMES) <
	    0) {
		frames = BUF_FRAMES;
		if (snd_pcm_hw_params_set_buffer_size_near(pcm[n], params,
							   &frames) < 0) {
			log_api_error(
				"snd_pcm_hw_params_set_buffer_size_near");
			return false;
		}
	}
#endif
	if (snd_pcm_hw_params(pcm[n], params) < 0) {
		log_api_error("snd_pcm_hw_params");
		return false;
	}

	return true;
}

/*
 * サウンドスレッド
 */

/* サウンドスレッドのエントリポイント */
static void *sound_thread(void *p)
{
	int n = (int)(intptr_t)p;

	while (1) {
		pthread_mutex_lock(&mutex[n]);
		{
			/* 待ち状態に入ったことの応答を送る */
			pthread_cond_signal(&ack[n]);

			/* 再生開始か使用終了の要求を待つ */
			pthread_cond_wait(&req[n], &mutex[n]);
			if (quit[n]) {
				pthread_mutex_unlock(&mutex[n]);
				break;
			}
		}
		pthread_mutex_unlock(&mutex[n]);

		/* 再生ループを実行する */
		while (playback_period(n)) {
#if defined(LINUX)
			/*
			 * [重要]
			 *  - コンテキストスイッチを明示的に行う
			 *  - これがないとメインスレッドが止まる
			 *  - linux 4.4.0で確認
			 *  - sched_yield()ではだめ
			 */
			sleep(0);
#elif defined(NETBSD)
			/*
			 * [重要]
			 *  - コンテキストスイッチを明示的に行う
			 *  - これがないとメインスレッドでmutexを取得できない
			 *  - NetBSD 9.1で確認
			 *  - sleep(0)ではだめ
			 */
			sched_yield();
#endif
		}
	}

	return (void *)0;
}

/* 再生を実行する */
static bool playback_period(int n)
{
	int size;

	pthread_mutex_lock(&mutex[n]);
	{
		/* メインスレッドで再生が停止された場合 */
		if (wave[n] == NULL) {
			pthread_mutex_unlock(&mutex[n]);
			return false;
		}

		/* PCMサンプルを取得する */
		size = get_wave_samples(wave[n], period_buf[n], PERIOD_FRAMES);

		/* 終端でサンプル数が足りない場合、ゼロで埋める */
		if (size < PERIOD_FRAMES)
			memset(period_buf[n] + size, 0,
			       (size_t)(PERIOD_FRAMES - size) * FRAME_SIZE);

		/* ボリュームの値でサンプルをスケールする */
		scale_samples(period_buf[n], PERIOD_FRAMES, volume[n]);

		/* デバイスに書き込む(アンダーランしている間繰り返す) */
		while (snd_pcm_writei(pcm[n], period_buf[n],
				      PERIOD_FRAMES) < 0)
			snd_pcm_prepare(pcm[n]);

		/* 終端まで再生した場合 */
		if (is_wave_eos(wave[n])) {
			wave[n] = NULL;
			finish[n] = true;
			pthread_mutex_unlock(&mutex[n]);
			return false;
		}
	}
	pthread_mutex_unlock(&mutex[n]);

	return true;
}

/*
 * SSEバージョニングを行わない場合
 */
#ifndef SSE_VERSIONING

/* scale_samples()を定義する */
#define SCALE_SAMPLES scale_samples
#include "scalesamples.h"

/*
 * SSEバージョニングを行う場合
 */
#else

/* AVX-512版scale_samples()を定義する */
#define PROTOTYPE_ONLY
#define SCALE_SAMPLES scale_samples_avx512
#include "scalesamples.h"

/* AVX2版scale_samples()を定義する */
#define PROTOTYPE_ONLY
#define SCALE_SAMPLES scale_samples_avx2
#include "scalesamples.h"

/* AVX版scale_samples()を定義する */
#define PROTOTYPE_ONLY
#define SCALE_SAMPLES scale_samples_avx
#include "scalesamples.h"

/* SSE4.2版scale_samples()を定義する */
#define PROTOTYPE_ONLY
#define SCALE_SAMPLES scale_samples_sse42
#include "scalesamples.h"

/* SSE4.1版scale_samples()を定義する */
#define PROTOTYPE_ONLY
#define SCALE_SAMPLES scale_samples_sse41
#include "scalesamples.h"

/* SSE3版scale_samples()を定義する */
#define PROTOTYPE_ONLY
#define SCALE_SAMPLES scale_samples_sse3
#include "scalesamples.h"

/* SSE2版scale_samples()を定義する */
#define PROTOTYPE_ONLY
#define SCALE_SAMPLES scale_samples_sse2
#include "scalesamples.h"

/* SSE版scale_samples()を定義する */
#define PROTOTYPE_ONLY
#define SCALE_SAMPLES scale_samples_sse
#include "scalesamples.h"

/* 非ベクトル版scale_samples()を定義する */
#define PROTOTYPE_ONLY
#define SCALE_SAMPLES scale_samples_novec
#include "scalesamples.h"

/* scale_samples()をディスパッチする */
void scale_samples(uint32_t *buf, int n, float vol)
{
	if (has_avx512)
		scale_samples_avx512(buf, n, vol);
	else if (has_avx2)
		scale_samples_avx2(buf, n, vol);
	else if (has_avx)
		scale_samples_avx(buf, n, vol);
	else if (has_sse42)
		scale_samples_sse42(buf, n, vol);
	else if (has_sse41)
		scale_samples_sse41(buf, n, vol);
	else if (has_sse3)
		scale_samples_sse3(buf, n, vol);
	else if (has_sse2)
		scale_samples_sse2(buf, n, vol);
	else if (has_sse)
		scale_samples_sse(buf, n, vol);
	else
		scale_samples_novec(buf, n, vol);
}

#endif
