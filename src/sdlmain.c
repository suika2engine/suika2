/* -*- tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Suika 2
 * Copyright (C) 2001-2022, TABATA Keiichi. All rights reserved.
 */

/*
 * [Changes]
 *  2022-11-08 Created
 */

#include <SDL.h>
#include <GL/glew.h>

#include <locale.h>

#include <sys/types.h>
#include <sys/stat.h>	/* stat(), mkdir() */
#include <sys/time.h>	/* gettimeofday() */

#include "suika.h"
#include "glrender.h"

#ifdef SSE_VERSIONING
#include "x86.h"
#endif

/*
 * The Log File
 */
static FILE *log_fp;

/*
 * SDL2 Objects
 */
static SDL_Window *window;
static SDL_GLContext *context;

/*
 * forward declaration
 */
static bool init(void);
static void cleanup(void);
static void run_game_loop(void);
static bool dispatch_event(SDL_Event *ev);
static int get_keycode(int sym);
static bool open_log_file(void);
static void close_log_file(void);

/*
 * Main
 */
int main(int argc, char *argv[])
{
	int ret;

	UNUSED_PARAMETER(argc);
	UNUSED_PARAMETER(argv);

	ret = 1;
	do {
		/* Do lower layer initialization. */
		if (!init())
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

	/* Do lower layer initialization. */
	cleanup();

	return ret;
}

/* Do lower layer initialization. */
static bool init(void)
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
				  SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);

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

	/* Succeeded. */
	return true;
}

/* Do lawer layer cleanup. */
static void cleanup(void)
{
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

/*
 * Implementation of platform.h functions
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
	if (!opengl_lock_texture(width, height, pixels, locked_pixels,
				 texture))
		return false;

	return true;
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

	/* Create a path. */
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
	UNUSED_PARAMETER(fname);
	UNUSED_PARAMETER(is_skippable);

	/* TODO */
	return true;
}

/*
 * Stop video playback.
 */
void stop_video(void)
{
	/* TODO */
}

/*
 * Check if video is playing.
 */
bool is_video_playing(void)
{
	/* TODO */
	return false;
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

/*
 * Start the sound playback on the specified stream.
 */
bool play_sound(int stream, struct wave *w)
{
	UNUSED_PARAMETER(stream);
	UNUSED_PARAMETER(w);
	/* TODO */
	return true;
}

/*
 * Stop the sound playback on the specified stream.
 */
bool stop_sound(int stream)
{
	UNUSED_PARAMETER(stream);
	/* TODO */
	return true;
}

/*
 * Set the sound volume of the specified stream.
 */
bool set_sound_volume(int stream, float vol)
{
	UNUSED_PARAMETER(stream);
	UNUSED_PARAMETER(vol);
	/* TODO */
	return true;
}

/*
 * Check if the sound playback is finished on the specified stream.
 */
bool is_sound_finished(int stream)
{
	UNUSED_PARAMETER(stream);
	/* TODO */
	return true;
}
