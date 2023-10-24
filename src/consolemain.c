/* -*- tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Suika2
 * Copyright (C) 2001-2023, Keiichi Tabata. All rights reserved.
 */

/*
 * This is just an example HAL for consoles that support SDL2.
 *
 * Note that:
 *  - The original author does not involved in this particular port
 *  - This port should not be used for anything beyond personal interest
 *  - Actual console ports should not be OSS due to SDK license issues
 *
 * [Changes]
 *  2023-03-23 Created by devseed, support Handheld game console
 */

/* Suika2 Base */
#define AVOID_UTF8_TO_UTF32
#include "suika.h"

/* Suika2 HAL for OpenGL */
#include "glrender.h"

/* SDL2 */
#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>

/* Standard C */
#include <locale.h>

/* POSIX */
#include <sys/types.h>
#include <sys/stat.h>	/* stat(), mkdir() */
#include <sys/time.h>	/* gettimeofday() */
#include <pthread.h>

/* Import homebrew; note that this is just an example for demonstration. */
#include <switch.h>

/*
 * Constants
 */

/* Sound */
#define SAMPLING_RATE   (44100)
#define CHANNELS        (2)
#define FRAME_SIZE      (4)
#define TMP_SAMPLES     (4096) /* Note: (512) produces sound glitch */

/* Click time threshould */
#define LONG_CLICK_TIME 800

/* Right ABXY button */
#define JOY_A     0
#define JOY_B     1
#define JOY_X     2
#define JOY_Y     3

/* Stick button */
#define JOY_LS    4
#define JOY_RS    5

/* Shoulder button */
#define JOY_L     6
#define JOY_R     7
#define JOY_ZL    8
#define JOY_ZR    9

/* Menu button */
#define JOY_PLUS  10
#define JOY_MINUS 11

/* Dpad button*/
#define JOY_LEFT  12
#define JOY_UP    13
#define JOY_RIGHT 14
#define JOY_DOWN  15

/*
 * Variables
 */

/* The Log File */
static FILE *log_fp;

/* Path of the game */
static char gamedir[FS_MAX_PATH] = {0};
static char savedir[FS_MAX_PATH] = {0};

/* SDL2 Objects */
static SDL_Window *window;
static SDL_GLContext *context;
static SDL_AudioDeviceID audio;
static SDL_Joystick *joystick;

/* Sound Objects */
static struct wave *wave[MIXER_STREAMS];
static float volume[MIXER_STREAMS];
static bool finish[MIXER_STREAMS];
static uint32_t snd_buf[TMP_SAMPLES];
static pthread_mutex_t mutex;

/* Video Objects */
static bool is_gst_playing;
static bool is_gst_skippable;

/*
 * Forward Declaration
 */

static bool init(int argc, char *argv[]);
static void init_underlying_sdk(int argc, char *argv[]);
static void cleanup(void);
static void run_game_loop(void);
static bool dispatch_event(SDL_Event *ev);
static int get_keycode(int button);
static void get_screen_position(double x, double y, int* screenx, int* screeny);
static bool show_video_frame(void);
static bool open_log_file(void);
static void close_log_file(void);
static bool init_sound(void);
static void cleanup_sound(void);
static void audio_callback(void *userdata, Uint8 *stream, int len);

/*
 * main(): called with args "-gamedir:dirpath -savedir:dirpath"
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

	/* Show an error message. */
	if  (ret != 0 && log_fp != NULL)
		printf("Check " LOG_FILE "\n");

	/* Do lower layer cleanup. */
	cleanup();

	return ret;
}

/* Do lower layer initialization. */
static bool init(int argc, char *argv[])
{
	/* Init an underlying SDK. */
	init_underlying_sdk(argc, argv);

	/* Initialize the SDL2. */
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS) != 0) {
		log_error("Failed to initialize SDL: %s", SDL_GetError());
		return false;
	}
	if (SDL_InitSubSystem(SDL_INIT_JOYSTICK) < 0) {
		log_error("Failed to initialize SDL JOYSTICK: %s", SDL_GetError());
		return false;
	}
	SDL_JoystickEventState(SDL_ENABLE);
	joystick = SDL_JoystickOpen(0);

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
				  SDL_WINDOW_OPENGL | SDL_WINDOW_BORDERLESS);

	/* Create an OpenGL context. */
	context = SDL_GL_CreateContext(window);
	SDL_GL_SetSwapInterval(1);
	
	/* Initialize OpenGL. */
	if (!init_opengl())
		return false;

	/* Initialize sound. */
	if (!init_sound())
		return false;

	/* TODO: support video rendering. */

	/* Succeeded. */
	return true;
}

/* Initialize an underlying SDK. */
static void parse_args(int argc, char *argv[])
{
	char *name;
	int i;

	socketInitializeDefault();
	nxlinkStdio();

	name = strrchr(argv[0], '/');
	for (i = 1; i < argc;i++) {
		if (strncmp(argv[i], "gamedir:", 8) == 0)
			strcpy(gamedir, argv[i] + 8);
		else if (strncmp(argv[i], "savedir:", 8)==0)
			strcpy(savedir, argv[i] + 8);
	}

	if (strstr(argv[0], ".nsp") != NULL ||
	    strstr(argv[0], ".NSP") != NULL) {
		romfsInit();
		chdir("romfs:/");
		if (savedir[0] != '\0') {
			strcpy(savedir, "/switch/");
			strcpy(savedir, name);
			savedir[strlen(savedir) - 4] = 0;
		}
	}
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

	/* Close SDL Object */
	SDL_JoystickClose(joystick);
	SDL_DestroyWindow(window);

	/* Close the debug socket */
	socketExit(); 
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
	static Uint32 lastclick;
	int key, screenx, screeny;

	switch (ev->type) {
	case SDL_QUIT:
		/* Quit the main loop. */
		return false;
	case SDL_FINGERMOTION:
		lastclick = ev->tfinger.timestamp;
		get_screen_position(ev->tfinger.x, ev->tfinger.y, &screenx, &screeny);
		on_event_mouse_move(screenx, screeny);
		break;
	case SDL_FINGERDOWN:
		lastclick = ev->tfinger.timestamp;
		get_screen_position(ev->tfinger.x, ev->tfinger.y, &screenx, &screeny);
		on_event_mouse_press(MOUSE_LEFT, screenx, screeny);
		break;
	case SDL_FINGERUP:
		get_screen_position(ev->tfinger.x, ev->tfinger.y, &screenx, &screeny);
		
		/* recognize long touch or multi fingers as right click */
		key = MOUSE_LEFT;
		if(ev->tfinger.timestamp - lastclick > LONG_CLICK_TIME 
			|| SDL_GetNumTouchFingers(ev->tfinger.touchId) >= 2) {
			lastclick = ev->tfinger.timestamp;
			key = MOUSE_RIGHT;
		}
		on_event_mouse_release(key, screenx, screeny);
		break;
	case SDL_JOYAXISMOTION:
		/* Todo: render mouse pointer in screen */
		break;
	case SDL_JOYBUTTONDOWN:
		/* homebrew use menu_plus to exit*/
		if(ev->jbutton.button == JOY_PLUS) return false;

		key = get_keycode(ev->jbutton.button);
		if(key>=0) on_event_key_press(key);
		break;
	case SDL_JOYBUTTONUP:
		key = get_keycode(ev->jbutton.button);
		if(key>=0) on_event_key_release(key);
		break;
	}

	/* Continue the main loop. */
	return true;
}

/* Convert the keycode. */
static int get_keycode(int button)
{
	switch (button) {
	case JOY_UP:
		return KEY_UP;
	case JOY_DOWN:
		return KEY_DOWN;
	case JOY_R:
		return KEY_CONTROL;
	case JOY_A:
		return KEY_RETURN;
	case JOY_X:
		return KEY_ESCAPE;
	case JOY_B:
		return KEY_C;
	}
	return -1;
}

static void get_screen_position(double x, double y, int* screenx, int* screeny)
{
	*screenx = (int)(x * conf_window_width);
	*screeny = (int)(y * conf_window_height);
}

/* TODO: Show a video frame. */
static bool show_video_frame(void)
{
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
	char buf[LOG_BUF_SIZE];
	va_list ap;

	open_log_file();

	va_start(ap, s);
	vsnprintf(buf, sizeof(buf), s, ap);
	va_end(ap);

	printf("%s", buf);
	if (log_fp != NULL) {
		fprintf(log_fp, "%s", buf);
		fflush(log_fp);
	}

	return true;
}

/*
 * Record WARN log.
 */
bool log_warn(const char *s, ...)
{
	char buf[LOG_BUF_SIZE];
	va_list ap;

	open_log_file();

	va_start(ap, s);
	vsnprintf(buf, sizeof(buf), s, ap);
	va_end(ap);

	printf("%s", buf);
	if (log_fp != NULL) {
		fprintf(log_fp, "%s", buf);
		fflush(log_fp);
	}

	return true;
}

/*
 * Record ERROR log.
 */
bool log_error(const char *s, ...)
{
	char buf[LOG_BUF_SIZE];
	va_list ap;

	open_log_file();

	va_start(ap, s);
	vsnprintf(buf, sizeof(buf), s, ap);
	va_end(ap);

	printf("%s", buf);
	if (log_fp != NULL) {
		fprintf(log_fp, "%s", buf);
		fflush(log_fp);
	}

	return true;
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
 * Render an image to the screen with dimming.
 */
void render_image_dim(int dst_left, int dst_top,
		      struct image * RESTRICT src_image,
		      int width, int height, int src_left, int src_top)
{
	/* See also glrender.c */
	opengl_render_image_dim(dst_left, dst_top, src_image, width, height,
				src_left, src_top);
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

	if(savedir[0]){
		if (stat(savedir, &st) == -1)
			mkdir(savedir, 0700);
	}
	else {
		if (stat(SAVE_DIR, &st) == -1)
			mkdir(SAVE_DIR, 0700);
	}

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
	len = strlen(gamedir) + 1 + strlen(dir) + 1 + strlen(fname) + 1;
	buf = malloc(len);
	if (buf == NULL) {
		log_memory();
		return NULL;
	}

	/* Create the path. */
	strcpy(buf, gamedir);
	if (strlen(gamedir) != 0) strcat(buf, "/");
	strcat(buf, dir);
	if (strlen(dir) != 0) strcat(buf, "/");
	strcat(buf, fname);
	return buf;
}

/*
 * Measuring Time Laps
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
		/* Return 0 when overflow. */
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
 * TODO: Start video playback (use libavcodec)
 */
bool play_video(const char *fname, bool is_skippable)
{
	UNUSED_PARAMETER(fname);
	UNUSED_PARAMETER(is_skippable);
	return true;
}

/*
 * TODO: Stop video playback.
 */
void stop_video(void)
{
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
 * Sound HAL implementaion
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

/* Define mul_add_pcm(). */
#define MUL_ADD_PCM mul_add_pcm
#include "muladdpcm.h"

/* Audio callback. */
static void audio_callback(UNUSED(void *userdata), Uint8 *stream, int len)
{
	uint32_t *sample_ptr;
	int n, ret, remain, read_samples;

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
				mul_add_pcm(sample_ptr, snd_buf, volume[n], read_samples);
			}
		
			/* Increment the write position. */
			sample_ptr += read_samples;
			remain -= read_samples;
		}
	}
	pthread_mutex_unlock(&mutex);
}
