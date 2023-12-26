/* -*- tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Suika2
 * Copyright (C) 2001-2023, Keiichi Tabata. All rights reserved.
 */

/*
 * [Changes]
 *  - 2016-06-06 Created.
 *  - 2023-12-25 Refactored.
 */

#include "../suika.h"

#include <pthread.h>

#if !defined(__ANDROID__)
/* GNU/Linux */
#include <alsa/asoundlib.h>
#else
/* Android NDK with tinyalsa */
#include <tinyalsa/asoundlib.h>
#include <unistd.h>	/* sleep() */
#define snd_pcm_t			struct pcm
#define snd_pcm_close			pcm_close
#define snd_config_update_free_global()
#define snd_pcm_writei			pcm_writei
#define snd_pcm_prepare			pcm_prepare
#define snd_pcm_drop(s)			pcm_stop(s)
#endif

/*
 * Format
 */
#define SAMPLING_RATE	(44100)
#define CHANNELS	(2)
#define DEPTH		(16)
#define FRAME_SIZE	(4)

/*
 * Sound Buffer
 */
#define BUF_FRAMES		(SAMPLING_RATE / 8)
#define BUF_SIZE		(BUF_FRAMES * FRAME_SIZE)
#define PERIODS			(4)
#define PERIOD_FRAMES		(BUF_FRAMES / PERIODS)
#define PERIOD_SIZE		(BUF_SIZE / PERIODS)
#define PERIOD_FRAMES_PAD	((PERIOD_SIZE + 63) / 64 * 64 - PERIOD_SIZE)

/*
 * Stream Data
 */

/* ALSA Devices */
static snd_pcm_t *pcm[MIXER_STREAMS];

/* Input Streams */
static struct wave *wave[MIXER_STREAMS];

/* Sound Threads */
static pthread_t thread[MIXER_STREAMS];

/* Mutex Objects (mutually exclude between the main thread and a sound thread) */
static pthread_mutex_t mutex[MIXER_STREAMS];

/* Conditional Variables (for requests from the main thread to a sound thread) */
static pthread_cond_t req[MIXER_STREAMS];

/* Conditional Variables (for renponses from a sound thread to the main thread */
static pthread_cond_t ack[MIXER_STREAMS];

/* Buffers */
SIMD_ALIGNED_MEMBER(static uint32_t period_buf[MIXER_STREAMS][PERIOD_FRAMES + PERIOD_FRAMES_PAD]);

/* Flags of Quit Requests */
static bool quit[MIXER_STREAMS];

/* Volume Values per Stream */
static float volume[MIXER_STREAMS];

/* Finish Flags */
static bool finish[MIXER_STREAMS];

/*
 * Forward Declarations
 */
static bool init_pcm(int n);
static void *sound_thread(void *p);
static bool playback_period(int n);
static void scale_samples(uint32_t *buf, int frames, float vol);

/*
 * Initialize ALSA.
 */
bool init_asound(void)
{
	int n, ret;

	for (n = 0; n < MIXER_STREAMS; n++) {
		/* Initialize per stream data. */
		pcm[n] = NULL;
		wave[n] = NULL;
		quit[n] = false;
		volume[n] = 1.0f;
		finish[n] = false;

		/* Initialize a device. */
		if (!init_pcm(n))
			return false;

		/* Create a mutex object. */
		pthread_mutex_init(&mutex[n], NULL);

		/* Create conditional variables. */
		pthread_cond_init(&req[n], NULL);
		pthread_cond_init(&ack[n], NULL);

		pthread_mutex_lock(&mutex[n]);
		{
			/* Start a sound thread. */
			ret = pthread_create(&thread[n], NULL, sound_thread, (void *)(intptr_t)n);
			if (ret != 0) {
				pthread_mutex_unlock(&mutex[n]);
				return false;
			}

			/* Receive a response that indicates the sound thread is in a wait. */
			pthread_cond_wait(&ack[n], &mutex[n]);
		}
		pthread_mutex_unlock(&mutex[n]);
	}

	return true;
}

/*
 * Cleanup ALSA.
 */
void cleanup_asound(void)
{
	void *p1;
	int n;

	for (n = 0; n < MIXER_STREAMS; n++) {
		if (pcm[n] == NULL)
			continue;

		stop_sound(n);

		pthread_mutex_lock(&mutex[n]);
		{
			/* Send a quit signal to a sound thread. */
			quit[n] = true;
			pthread_cond_signal(&req[n]);
		}
		pthread_mutex_unlock(&mutex[n]);

		/* Wait for an exit of the sound thread. */
		pthread_join(thread[n], &p1);

		/* Close a device. */
		if (pcm[n] != NULL)
			snd_pcm_close(pcm[n]);

		/* Destroy a conditional variable. */
		pthread_cond_destroy(&req[n]);

		/* Destroy a mutex. */
		pthread_mutex_destroy(&mutex[n]);
	}

	/* Free caches for Valgrind check. */
	snd_config_update_free_global();
}

/*
 * Start sound playback.
 */
bool play_sound(int n, struct wave *w)
{
	assert(n < MIXER_STREAMS);
	assert(w != NULL);

	/* If ALSA is not available, just return. */
	if (pcm[n] == NULL)
		return true;

	/* Stop an in-flight playback. */
	stop_sound(n);

	pthread_mutex_lock(&mutex[n]);
	{
		/* Set a PCM stream. */
		wave[n] = w;

		/* Reset a finish flag. */
		finish[n] = false;

		/* Send a signal of a playback start. */
		pthread_cond_signal(&req[n]);
	}
	pthread_mutex_unlock(&mutex[n]);
	return true;
}

/*
 * Stop sound playback on a stream.
 */
bool stop_sound(int n)
{
	assert(n < MIXER_STREAMS);

	/* If ALSA is not available, just return. */
	if (pcm[n] == NULL)
		return true;

	pthread_mutex_lock(&mutex[n]);
	{
		if (wave[n] != NULL) {
			/* Cancel playback status. */
			wave[n] = NULL;

			/* Stop an in-flight buffer. */
			snd_pcm_drop(pcm[n]);

			/* Receive a signal that indicates a sound thread is in a wait. */
			pthread_cond_wait(&ack[n], &mutex[n]);
		}
	}
	pthread_mutex_unlock(&mutex[n]);
	return true;
}

/*
 * Set a sound volume for a stream.
 */
bool set_sound_volume(int n, float vol)
{
	assert(n < MIXER_STREAMS);
	assert(vol >= 0 && vol <= 1.0f);

	volume[n] = vol;

	/* For relaxed consistencies. Not need for x86 and arm processors. */
	__sync_synchronize();

	return true;
}

/*
 * Check if a sound stream is finished.
 */
bool is_sound_finished(int n)
{
	/* If ALSA is not available, just return. */
	if (pcm[n] == NULL)
		return true;

	if (finish[n])
		return true;

	return false;
}

/* Initialize a device for a stream. */
static bool init_pcm(int n)
{
#if !defined(__ANDROID__)
	/* Normal GNU/Linux */

	/* Open a device. */
	int ret;
	ret = snd_pcm_open(&pcm[n], "default", SND_PCM_STREAM_PLAYBACK, 0);
	if (ret < 0) {
		log_api_error("snd_pcm_open");
		return false;
	}

	/* Set the format (44.1kHz, stereo, 16-bit signed little endian) */
	snd_pcm_hw_params_t *params;
	snd_pcm_uframes_t frames;
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
	if (snd_pcm_hw_params_set_format(pcm[n], params, SND_PCM_FORMAT_S16_LE) < 0) {
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
	if (snd_pcm_hw_params_set_buffer_size(pcm[n], params, BUF_FRAMES) < 0) {
		frames = BUF_FRAMES;
		if (snd_pcm_hw_params_set_buffer_size_near(pcm[n], params, &frames) < 0) {
			log_api_error("snd_pcm_hw_params_set_buffer_size_near");
			return false;
		}
	}
	if (snd_pcm_hw_params(pcm[n], params) < 0) {
		log_api_error("snd_pcm_hw_params");
		return false;
	}
#else
	/* tinyalsa */

	/* Describe the format (44.1kHz, stereo, 16-bit signed little endian) */
	struct pcm_config config;
	memset(&config, 0, sizeof(config));
	config.channels = 2;
	config.rate = 44100;
	config.period_size = BUF_FRAMES;
	config.period_count = PERIODS;
	config.format = PCM_FORMAT_S16_LE;

	/* Open a PCM device. */
	pcm[n] = pcm_open(0, 0, PCM_OUT, &config);
	if (pcm[n] == NULL) {
		log_api_error("pcm_open_by_name");
		return false;
	}
#endif
	return true;
}

/*
 * Sound Threads
 */

/* The entrypoint of a per-device sound thread. */
static void *sound_thread(void *p)
{
	int n = (int)(intptr_t)p;

	while (1) {
		pthread_mutex_lock(&mutex[n]);
		{
			/* Send a signal that indicates the sound thread is in a wait. */
			pthread_cond_signal(&ack[n]);

			/* Wait for a signal. (start or exit) */
			pthread_cond_wait(&req[n], &mutex[n]);
			if (quit[n]) {
				pthread_mutex_unlock(&mutex[n]);
				break;
			}
		}
		pthread_mutex_unlock(&mutex[n]);

		/* Run a playback loop. */
		while (playback_period(n)) {
#if defined(__linux__)
			/*
			 * [Important]
			 *  - We execute context-switch explicitly on Linux by sleep(0).
			 *  - If we do not call sleep(0), the main thread stops.
			 *  - We have confirmed this issue on Linux 4.4.0 x86_64.
			 *  - As we understand, sched_yield() does not solve this issue.
			 */
			sleep(0);
#elif defined(__NetBSD__)
			/*
			 * [Important]
			 *  - We execute context-switch explicitly on NetBSD by sched_yield().
			 *  - If we do not call sched_yield(), the main thread cannot acquire a mutex.
			 *  - We have confirmed this issue on NetBSD 9.1 amd64.
			 *  - As we understand, sleep(0) does not solve this issue.
			 */
			sched_yield();
#endif
		}
	}

	return (void *)0;
}

/* Write to a buffer. */
static bool playback_period(int n)
{
	int size;

	pthread_mutex_lock(&mutex[n]);
	{
		/* Return false if the main thread stopped a playback. */
		if (wave[n] == NULL) {
			pthread_mutex_unlock(&mutex[n]);
			return false;
		}

		/* Get PCM samples. */
		size = get_wave_samples(wave[n], period_buf[n], PERIOD_FRAMES);

		/* Fill the remaining samples by zeros for a case where we have reached an end-of-stream. */
		if (size < PERIOD_FRAMES)
			memset(period_buf[n] + size, 0, (size_t)(PERIOD_FRAMES - size) * FRAME_SIZE);

		/* Scale samples by a volume value. */
		scale_samples(period_buf[n], PERIOD_FRAMES, volume[n]);

		/* Write to the device (repeat while under-running) */
		while (snd_pcm_writei(pcm[n], period_buf[n], PERIOD_FRAMES) < 0)
			snd_pcm_prepare(pcm[n]);

		/* Return false if we reached an end-of-stream. */
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

/* Apply a volume value. */
static void scale_samples(uint32_t *buf, int frames, float vol)
{
	float scale;
	uint32_t frame;
	int32_t il, ir;	/* intermediate L/R */
	int16_t sl, sr;	/* source L/R*/
	int16_t dl, dr;	/* destination L/R */
	int i;

	/* For relaxed consistencies. Not need for x86 and arm processors. */
	__sync_synchronize();

	/* Convert a scale factor to an exponential value. */
	scale = (powf(10.0f, vol) - 1.0f) / (10.0f - 1.0f);

	/* Scale samples. */
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

		buf[i] = ((uint32_t)(uint16_t)dl) | (((uint32_t)(uint16_t)dr) << 16);
	}
}
