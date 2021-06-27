/* -*- tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Suika 2
 * Copyright (C) 2001-2021, TABATA Keiichi. All rights reserved.
 */

/*
 * OpenAL 1.1 sound module for Emscripten
 *
 * [Changes]
 *  2021-06-27 作成
 */

#include "suika.h"

#include <AL/al.h>
#include <AL/alc.h>

/*
 * サンプリングレートとバッファのサンプル数
 */
#define SAMPLING_RATE	(44100)
#define SAMPLES		SAMPLING_RATE

/*
 * バッファ数
 */
#define BUFFER_COUNT	(4)

/*
 * サウンドデバイス
 */
static ALCdevice *device;

/*
 * サウンドコンテキスト
 */
static ALCcontext *context;

/*
 * バッファ
 */
static ALuint buffer[BUFFER_COUNT];

/*
 * ソース
 */
static ALuint source;

/*
 * PCMストリーム
 */
static struct wave *stream;

/*
 * サンプルの一時格納場所
 */
static uint32_t sample_buf[SAMPLES];

/*
 * サウンドの初期化を行う
 */
bool init_openal(void)
{
	ALenum error;

	/* デバイスを開く */
	device = alcOpenDevice(NULL);
	if (device == NULL) {
		log_api_error("alcOpenDevice");
		return false;
	}

	/* コンテキストを作成する */
	context = alcCreateContext(device, NULL);
	if (context == NULL) {
		log_api_error("alcCreateContext");
		return false;
	}
	alcMakeContextCurrent(context);
	alGetError();

	/* バッファを作成する */
	alGenBuffers(BUFFER_COUNT, buffer);
	error = alGetError();
	if (error != AL_NO_ERROR) {
		log_api_error("alGenBuffers");
		return false;
	}

	/* ソースを作成する */
	alGenSources(1, &source);
	alSourcef(source, AL_GAIN, 1);
	alSource3f(source, AL_POSITION, 0, 0, 0);

	return true;
}

/*
 * サウンドの終了処理を行う
 */
void cleanup_openal(void)
{
}

/*
 * サウンドを再生を開始する
 */
bool play_sound(int n, struct wave *w)
{
	ALuint buf[BUFFER_COUNT];
	int i, samples;

	if (n != 0)
		return true;

	if (stream != NULL) {
		alSourceStop(source);
		alSourceUnqueueBuffers(source, BUFFER_COUNT, buf);
	}

	stream = w;

	for (i = 0; i < BUFFER_COUNT; i++) {
		samples = get_wave_samples(stream, sample_buf, SAMPLES);
		alBufferData(buffer[i], AL_FORMAT_STEREO16, sample_buf,
			     samples * sizeof(uint32_t), SAMPLING_RATE);
	}

	alSourceQueueBuffers(source, BUFFER_COUNT, buffer);
	alSourcePlay(source);

	return true;
}

/*
 * サウンドの再生を停止する
 */
bool stop_sound(int n)
{
	ALuint buf[BUFFER_COUNT];

	if (n != 0)
		return true;

	if (stream == NULL) {
		alSourceStop(source);
		alSourceUnqueueBuffers(source, BUFFER_COUNT, buf);
		stream = NULL;
	}

	return true;
}

/*
 * サウンドのボリュームを設定する
 */
bool set_sound_volume(int n, float vol)
{
	if (n != 0)
		return true;

	alSourcef(source, AL_GAIN, vol);

	return true;
}

/*
 * サウンドのバッファを埋める
 */
void fill_sound_buffer(void)
{
	int processed, samples;
	ALuint buf;

	if (stream == NULL)
		return;

	alGetSourcei(source, AL_BUFFERS_PROCESSED, &processed);
	while (processed-- > 0) {
		alSourceUnqueueBuffers(source, 1, &buf);
		samples = get_wave_samples(stream, sample_buf, SAMPLES);
		alBufferData(buf, AL_FORMAT_STEREO16, sample_buf,
			     samples * sizeof(uint32_t), SAMPLING_RATE);
		alSourceQueueBuffers(source, 1, &buf);
	}
}
