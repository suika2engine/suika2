/* -*- tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Suika 2
 * Copyright (C) 2001-2022, TABATA Keiichi. All rights reserved.
 */

/*
 * This is a platform independent part (PIP) of Suika2 for SDL2 on X11.
 *
 * [Changes]
 *  2022-11-08 Created
 */

/* SDL */
#include <SDL.h>
#include <SDL_syswm.h>

/* OpenGL (GLEW) */
#include <GL/glew.h>

/* Standard C */
#include <locale.h>

/* POSIX */
#include <sys/types.h>
#include <sys/stat.h>	/* stat(), mkdir() */
#include <sys/time.h>	/* gettimeofday() */
#include <pthread.h>

/* Suika2 */
#include "suika.h"
#include "glrender.h"
#include "gstplay.h"
#ifdef SSE_VERSIONING
#include "x86.h"
#endif

/*
 * Sound Config
 */
#define SAMPLING_RATE   (44100)
#define CHANNELS        (2)
#define FRAME_SIZE      (4)
#define TMP_SAMPLES     (512)

/*
 * The Log File
 */
static FILE *log_fp;

/*
 * SDL2 Objects
 */
static SDL_Window *window;
static SDL_GLContext *context;
static SDL_AudioDeviceID audio;

/*
 * Sound Objects
 */
static struct wave *wave[MIXER_STREAMS];
static float volume[MIXER_STREAMS];
static bool finish[MIXER_STREAMS];
static uint32_t snd_buf[TMP_SAMPLES];
static pthread_mutex_t mutex;

/* for sound mixing */
void mul_add_pcm(uint32_t *dst, uint32_t *src, float vol, int samples);

/*
 * Video Objects
 */
static bool is_gst_playing;
static bool is_gst_skippable;

/*
 * forward declaration
 */
static bool init(int argc, char *argv[]);
static void cleanup(void);
static void run_game_loop(void);
static bool dispatch_event(SDL_Event *ev);
static int get_keycode(int sym);
static bool show_video_frame(void);
static bool open_log_file(void);
static void close_log_file(void);
static bool init_sound(void);
static void cleanup_sound(void);
static void audio_callback(void *userdata, Uint8 *stream, int len);

/*
 * Main
 */
int main(int argc, char *argv[])
{
	int ret;

	ret = 1;
	do {
		/* Do lower layer initialization. */
		if (!init(argc, argv))
			break;

		/* Do upper layer initialization. */
		if (!on_event_init())
			break;
		
		/* Run game loop. */
		run_game_loop();

		/* Do upper layer cleanup. */
		on_event_cleanup();

		/* Succeeded. */
		ret = 0;
	} while (0);

	/* Show error message. */
	if  (ret != 0 && log_fp != NULL)
		printf("Check " LOG_FILE "\n");

	/* Do lower layer cleanup. */
	cleanup();

	return ret;
}

/* Do lower layer initialization. */
static bool init(int argc, char *argv[])
{
#ifdef SSE_VERSIONING
	/* Check the vector extensions. */
	x86_check_cpuid_flags();
#endif

	/* Initialize the SDL2. */
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
		log_error("Failed to initialize SDL: %s", SDL_GetError());
		return false;
	}

	/* We'll use the environment variable for locale. */
	setlocale(LC_ALL, "");

	/* Initialize locale. */
	init_locale_code();

	/* Initialize file I/O. */
	if (!init_file())
		return false;

	/* Initialize config. */
	if (!init_conf())
		return false;

	/* Create a window. */
	window = SDL_CreateWindow(conf_window_title,
				  SDL_WINDOWPOS_CENTERED,
				  SDL_WINDOWPOS_CENTERED,
				  conf_window_width,
				  conf_window_height,
				  SDL_WINDOW_OPENGL);

	/* Create an OpenGL context. */
	context = SDL_GL_CreateContext(window);
	SDL_GL_SetSwapInterval(1);

	/* Initialize GLEW. */
	if (glewInit() != GLEW_OK) {
		log_api_error("glewInit()");
		return false;
	}
	
	/* Initialize OpenGL. */
	if (!init_opengl())
		return false;

	/* Initialize sound. */
	if (!init_sound())
		return false;

	/* Initialize video. */
	gstplay_init(argc, argv);

	/* Succeeded. */
	return true;
}

/* Do lawer layer cleanup. */
static void cleanup(void)
{
	/* Cleanup sound. */
	cleanup_sound();

	/* Cleanup OpenGL. */
	cleanup_opengl();

	/* Cleanup config. */
	cleanup_conf();

	/* Cleanup file I/O. */
	cleanup_file();

	/* Close the log file. */
	close_log_file();
}

/* The event loop. */
static void run_game_loop(void)
{
	SDL_Event ev;
	int x, y, w, h;
	bool cont;

	do {
		/* Process events. */
		while (SDL_PollEvent(&ev))
			if (!dispatch_event(&ev))
				return;

		/* Process video playback. */
		if (is_gst_playing) {
			if (!show_video_frame())
				return;
			continue;
		}

		/* Start rendering. */
		opengl_start_rendering();

		/* Do a frame event. */
		cont = on_event_frame(&x, &y, &w, &h);

		/* End rendering. */
		opengl_end_rendering();

		/* Show a frame */
		SDL_GL_SwapWindow(window);
	} while (cont);
}

/* The event dispatcher. */
static bool dispatch_event(SDL_Event *ev)
{
	if (ev->type == SDL_QUIT) {
		/* Quit the main loop. */
		return false;
	}

	switch (ev->type) {
	case SDL_MOUSEMOTION:
		on_event_mouse_move(ev->motion.x, ev->motion.y);
		break;
	case SDL_MOUSEBUTTONDOWN:
		if (ev->button.button == SDL_BUTTON_LEFT) {
			on_event_mouse_press(MOUSE_LEFT,
					     ev->button.x,
					     ev->button.y);
		} else if (ev->button.button == SDL_BUTTON_RIGHT) {
			on_event_mouse_press(MOUSE_RIGHT,
					     ev->button.x,
					     ev->button.y);
		}
		break;
	case SDL_MOUSEWHEEL:
		if (ev->wheel.y > 0)
			on_event_mouse_scroll(1);
		else
			on_event_mouse_scroll(-1);
		break;
	case SDL_KEYDOWN:
		if (ev->key.repeat)
			break;
		on_event_key_press(get_keycode(ev->key.keysym.sym));
		break;
	case SDL_KEYUP:
		on_event_key_release(get_keycode(ev->key.keysym.sym));
		break;
	}

	/* Continue the main loop. */
	return true;
}

/* Convert the keycode. */
static int get_keycode(int sym)
{
	switch (sym) {
	case SDLK_UP:
		return KEY_UP;
	case SDLK_DOWN:
		return KEY_DOWN;
	case SDLK_LCTRL:
	case SDLK_RCTRL:
		return KEY_CONTROL;
	case SDLK_RETURN:
		return KEY_RETURN;
	case SDLK_ESCAPE:
		return KEY_ESCAPE;
	case 'c':
		return KEY_C;
	}
	return -1;
}

/* Show a video frame. */
static bool show_video_frame(void)
{
	int x, y, w, h;

	assert(is_gst_playing);

	/* Show a frame. */
	gstplay_loop_iteration();

	/* Check if playback is finished. */
	if (!gstplay_is_playing()) {
		/* Do post-process. */
		gstplay_stop();
		is_gst_playing = false;
	}

	/* Do a frame event. */
	if (!on_event_frame(&x, &y, &w, &h)) {
		/* Break the main loop. */
		return false;
	}

	/* Continue the main loop. */
	return true;
}

/*
 * Implementation of "platform.h" functions (HAL API)
 */

/*
 * Logging
 */

/*
 * Record INFO log.
 */
bool log_info(const char *s, ...)
{
	va_list ap;

	open_log_file();

	va_start(ap, s);
	if (log_fp != NULL) {
		vfprintf(stderr, s, ap);
		vfprintf(log_fp, s, ap);
		fflush(log_fp);
		if (ferror(log_fp))
			return false;
	}
	va_end(ap);
	return true;
}

/*
 * Record WARN log.
 */
bool log_warn(const char *s, ...)
{
	va_list ap;

	open_log_file();

	va_start(ap, s);
	if (log_fp != NULL) {
		vfprintf(stderr, s, ap);
		vfprintf(log_fp, s, ap);
		fflush(log_fp);
		if (ferror(log_fp))
			return false;
	}
	va_end(ap);
	return true;
}

/*
 * Record ERROR log.
 */
bool log_error(const char *s, ...)
{
	va_list ap;

	open_log_file();

	va_start(ap, s);
	if (log_fp != NULL) {
		vfprintf(stderr, s, ap);
		vfprintf(log_fp, s, ap);
		fflush(log_fp);
		if (ferror(log_fp))
			return false;
	}
	va_end(ap);
	return true;
}

/*
 * Convert UTF-8 log to the native encoding.
 */
const char *conv_utf8_to_native(const char *utf8_message)
{
	assert(utf8_message != NULL);

	/* Just use the UTF-8 log. */
	return utf8_message;
}

/* Open the log file. */
static bool open_log_file(void)
{
	if (log_fp == NULL) {
		log_fp = fopen(LOG_FILE, "w");
		if (log_fp == NULL) {
			printf("Can't open log file.\n");
			return false;
		}
	}
	return true;
}

/* Close the log file. */
static void close_log_file(void)
{
	if (log_fp != NULL)
		fclose(log_fp);
}

/*
 * Graphics
 */

/*
 * Check if we use GPU acceleration.
 * The result will affect whether entire screen is rewritten every frame.
 */
bool is_gpu_accelerated(void)
{
	/* We use OpenGL, so just return true. */
	return true;
}

/*
 * Check if we use OpenGL.
 * The result will affect to the byte-order of the images.
 */
bool is_opengl_enabled(void)
{
	/* We use OpenGL, so just return true. */
	return true;
}

/*
 * Lock the texture object for an image.
 * While the texture for the image is locked, the image can be drawn.
 */
bool lock_texture(int width, int height, pixel_t *pixels,
		  pixel_t **locked_pixels, void **texture)
{
	/* See also glrender.c */
	return opengl_lock_texture(width, height, pixels, locked_pixels,
				   texture);
}

/*
 * Unlock the texture object for an image.
 * When the texture is unlocked, the pixels are uploaded to VRAM.
 */
void unlock_texture(int width, int height, pixel_t *pixels,
		    pixel_t **locked_pixels, void **texture)
{
	/* See also glrender.c */
	opengl_unlock_texture(width, height, pixels, locked_pixels, texture);
}

/*
 * Destroy the texture object for the image.
 */
void destroy_texture(void *texture)
{
	/* See also glrender.c */
	opengl_destroy_texture(texture);
}

/*
 * Render an image to the screen.
 */
void render_image(int dst_left, int dst_top, struct image * RESTRICT src_image,
                  int width, int height, int src_left, int src_top, int alpha,
                  int bt)
{
	/* See also glrender.c */
	opengl_render_image(dst_left, dst_top, src_image, width, height,
			    src_left, src_top, alpha, bt);
}

/*
 * Render an image to the screen with a rule image.
 */
void render_image_rule(struct image * RESTRICT src_img,
		       struct image * RESTRICT rule_img,
		       int threshold)
{
	/* See also glrender.c */
	opengl_render_image_rule(src_img, rule_img, threshold);
}

/*
 * Render an image to the screen with a rule image, using the melt effect.
 */
void render_image_melt(struct image * RESTRICT src_img,
		       struct image * RESTRICT rule_img,
		       int threshold)
{
	/* See also glrender.c */
	opengl_render_image_melt(src_img, rule_img, threshold);
}

/*
 * File manipulation
 */

/*
 * Create a save directory.
 */
bool make_sav_dir(void)
{
	struct stat st = {0};

	if (stat(SAVE_DIR, &st) == -1)
		mkdir(SAVE_DIR, 0700);

	return true;
}

/*
 * Create a valid path by directory and file names.
 */
char *make_valid_path(const char *dir, const char *fname)
{
	char *buf;
	size_t len;

	if (dir == NULL)
		dir = "";

	/* Allocate a buffer for the path. */
	len = strlen(dir) + 1 + strlen(fname) + 1;
	buf = malloc(len);
	if (buf == NULL) {
		log_memory();
		return NULL;
	}

	/* Create the path. */
	strcpy(buf, dir);
	if (strlen(dir) != 0)
		strcat(buf, "/");
	strcat(buf, fname);
	return buf;
}

/*
 * Stop watch
 */

/*
 * Reset a stop watch.
 */
void reset_stop_watch(stop_watch_t *t)
{
	struct timeval tv;

	gettimeofday(&tv, NULL);

	*t = (stop_watch_t)(tv.tv_sec * 1000 + tv.tv_usec / 1000);
}

/*
 * Get a lap time of stop watch in milli seconds.
 */
int get_stop_watch_lap(stop_watch_t *t)
{
	struct timeval tv;
	stop_watch_t end;
	
	gettimeofday(&tv, NULL);

	end = (stop_watch_t)(tv.tv_sec * 1000 + tv.tv_usec / 1000);

	if (end < *t) {
		/* オーバーフローの場合、タイマをリセットして0を返す */
		reset_stop_watch(t);
		return 0;
	}

	return (int)(end - *t);
}

/*
 * Dialog
 */

/*
 * Show the exit dialog.
 */
bool exit_dialog(void)
{
	/* stub, always YES */
	return true;
}

/*
 * Show the "back to the title" dialog.
 */
bool title_dialog(void)
{
	/* stub, always YES */
	return true;
}

/*
 * Show the delete dialog.
 */
bool delete_dialog(void)
{
	/* stub, always YES */
	return true;
}

/*
 * Show the overwrite dialog.
 */
bool overwrite_dialog(void)
{
	/* stub, always YES */
	return true;
}

/*
 * Show the "reset settings" dialog.
 */
bool default_dialog(void)
{
	/* stub, always YES */
	return true;
}

/*
 * Video playback
 */

/*
 * Start video playback.
 */
bool play_video(const char *fname, bool is_skippable)
{
	SDL_SysWMinfo info;
	char *path;

	path = make_valid_path(MOV_DIR, fname);

	is_gst_playing = true;
	is_gst_skippable = is_skippable;

	SDL_VERSION(&info.version);
	SDL_GetWindowWMInfo(window, &info);

	gstplay_play(path, info.info.x11.window);

	free(path);

	return true;
}

/*
 * Stop video playback.
 */
void stop_video(void)
{
	gstplay_stop();
	is_gst_playing = false;
}

/*
 * Check if video is playing.
 */
bool is_video_playing(void)
{
	return is_gst_playing;
}

/*
 * Window and Full Screen mode
 */

/*
 * Update window title (chapter name).
 */
void update_window_title(void)
{
	/* TODO */
}

/*
 * Check if full screen mode is supported.
 */
bool is_full_screen_supported(void)
{
	/* TODO */
	return false;
}

/*
 * Check if we are in full screen mode.
 */
bool is_full_screen_mode(void)
{
	/* TODO */
	return false;
}

/*
 * Start full screen mode.
 */
void enter_full_screen_mode(void)
{
	/* stub */
}

/*
 * Exit full screen mode.
 */
void leave_full_screen_mode(void)
{
	/* stub */
}

/*
 * Locale
 */

/*
 * Get the system locale.
 */
const char *get_system_locale(void)
{
	const char *locale;

	locale = setlocale(LC_ALL, "");
	if (locale[0] == '\0' || locale[1] == '\0')
		return "en";
	else if (strncmp(locale, "en", 2) == 0)
		return "en";
	else if (strncmp(locale, "fr", 2) == 0)
		return "fr";
	else if (strncmp(locale, "de", 2) == 0)
		return "fr";
	else if (strncmp(locale, "it", 2) == 0)
		return "it";
	else if (strncmp(locale, "es", 2) == 0)
		return "es";
	else if (strncmp(locale, "el", 2) == 0)
		return "el";
	else if (strncmp(locale, "ru", 2) == 0)
		return "ru";
	else if (strncmp(locale, "zh_CN", 5) == 0)
		return "zh";
	else if (strncmp(locale, "zh_TW", 5) == 0)
		return "tw";
	else if (strncmp(locale, "ja", 2) == 0)
		return "ja";

	return "other";
}

/*
 * Sound
 */

/* Initialize sound playback. */
static bool init_sound(void)
{
	SDL_AudioSpec desired, obtained;

	pthread_mutex_init(&mutex, NULL);

	SDL_zero(desired);
	desired.freq = SAMPLING_RATE;
	desired.format = AUDIO_S16;
	desired.channels = CHANNELS;
	desired.samples = TMP_SAMPLES;
	desired.callback = audio_callback;

	audio = SDL_OpenAudioDevice(NULL, 0, &desired, &obtained, 0);
	if (audio == 0) {
		log_api_error("SDL_OpenAudioDevice()");
		return false;
	}

	SDL_PauseAudioDevice(audio, 0);
	return true;
}

/* Cleanup sound playback. */
static void cleanup_sound(void)
{
	SDL_PauseAudioDevice(audio, 1);
	SDL_CloseAudioDevice(audio);
        pthread_mutex_destroy(&mutex);
	audio = 0;
}

/*
 * Start the sound playback on the specified stream.
 */
bool play_sound(int stream, struct wave *w)
{
	pthread_mutex_lock(&mutex);
	{
		/* Set the stream source. */
		wave[stream] = w;

		/* Reset the finish flag. */
		finish[stream] = false;
	}
	pthread_mutex_unlock(&mutex);

	return true;
}

/*
 * Stop the sound playback on the specified stream.
 */
bool stop_sound(int stream)
{
	pthread_mutex_lock(&mutex);
	{
		/* Remove the stream source. */
		wave[stream] = NULL;

		/* Set the finish flag. */
		finish[stream] = true;
	}
	pthread_mutex_unlock(&mutex);

	return true;
}

/*
 * Set the sound volume of the specified stream.
 */
bool set_sound_volume(int stream, float vol)
{
	volume[stream] = vol;
	return true;
}

/*
 * Check if the sound playback is finished on the specified stream.
 */
bool is_sound_finished(int stream)
{
	return finish[stream];
}

/* Audio callback. */
static void audio_callback(void *userdata, Uint8 *stream, int len)
{
	uint32_t *sample_ptr;
	int n, ret, remain, read_samples;

	UNUSED_PARAMETER(userdata);

	/* Clear the buffer. */
	memset(stream, 0, (size_t)len);

	sample_ptr = (uint32_t *)stream;
	remain = len / FRAME_SIZE;

	pthread_mutex_lock(&mutex);
	{
		while (remain > 0) {
			/* Get the sample size for the read. */
			read_samples = remain > TMP_SAMPLES ?
				TMP_SAMPLES : remain;
		
			/* For each stream: */
			for (n = 0; n < MIXER_STREAMS; n++) {
				/* If not playing: */
				if (wave[n] == NULL)
					continue;
		
				/* Get samples from input stream. */
				ret = get_wave_samples(wave[n], snd_buf,
						       read_samples);
		
				/* When we reached EOS: */
				if(ret < read_samples) {
					/* Add zeros. */
					memset(snd_buf + ret,
					       0,
					       (size_t)(read_samples - ret) *
					       sizeof(uint32_t));
		
					/* Remove the input stream. */
					wave[n] = NULL;
		
					/* Set the finish flag. */
					finish[n] = true;
				}
		
				/* Do mixing. */
				mul_add_pcm(sample_ptr, snd_buf, volume[n],
					    read_samples);
			}
		
			/* Increment the write position. */
			sample_ptr += read_samples;
			remain -= read_samples;
		}
	}
	pthread_mutex_unlock(&mutex);
}

/*
 * Without SSE dispatch:
 */
#ifndef SSE_VERSIONING

/* Define mul_add_pcm(). */
#define MUL_ADD_PCM mul_add_pcm
#include "muladdpcm.h"

/*
 * With SSE dispatch:
 */
#else

/* AVX-512 */
#define PROTOTYPE_ONLY
#define MUL_ADD_PCM mul_add_pcm_avx512
#include "muladdpcm.h"

/* AVX2 */
#define PROTOTYPE_ONLY
#define MUL_ADD_PCM mul_add_pcm_avx2
#include "muladdpcm.h"

/* AVX */
#define PROTOTYPE_ONLY
#define MUL_ADD_PCM mul_add_pcm_avx
#include "muladdpcm.h"

/* SSE4.2 */
#define PROTOTYPE_ONLY
#define MUL_ADD_PCM mul_add_pcm_sse42
#include "muladdpcm.h"

/* SSE4.1 */
#define PROTOTYPE_ONLY
#define MUL_ADD_PCM mul_add_pcm_sse41
#include "muladdpcm.h"

/* SSE3 */
#define PROTOTYPE_ONLY
#define MUL_ADD_PCM mul_add_pcm_sse3
#include "muladdpcm.h"

/* SSE2 */
#define PROTOTYPE_ONLY
#define MUL_ADD_PCM mul_add_pcm_sse2
#include "muladdpcm.h"

/* SSE */
#define PROTOTYPE_ONLY
#define MUL_ADD_PCM mul_add_pcm_sse
#include "muladdpcm.h"

/* Non-vectorized */
#define PROTOTYPE_ONLY
#define MUL_ADD_PCM mul_add_pcm_novec
#include "muladdpcm.h"

/* Dispatch mul_add_pcm() functions. */
void mul_add_pcm(uint32_t *dst, uint32_t *src, float vol, int samples)
{
	if (has_avx512)
		mul_add_pcm_avx512(dst, src, vol, samples);
	else if (has_avx2)
		mul_add_pcm_avx2(dst, src, vol, samples);
	else if (has_avx)
		mul_add_pcm_avx(dst, src, vol, samples);
	else if (has_sse42)
		mul_add_pcm_sse42(dst, src, vol, samples);
	else if (has_sse41)
		mul_add_pcm_sse41(dst, src, vol, samples);
	else if (has_sse3)
		mul_add_pcm_sse3(dst, src, vol, samples);
	else if (has_sse2)
		mul_add_pcm_sse2(dst, src, vol, samples);
	else if (has_sse)
		mul_add_pcm_sse(dst, src, vol, samples);
	else
		mul_add_pcm_novec(dst, src, vol, samples);
}

#endif
