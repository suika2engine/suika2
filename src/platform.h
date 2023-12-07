/* -*- coding: utf-8; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Suika2
 * Copyright (C) 2001-2023, Keiichi Tabata. All rights reserved.
 */

/*
 * HAL API definition of Suika2
 *
 * [HAL]
 *  Hardware Abstraction Layer is a thin layer to port Suika2 to
 *  various platforms. For a porting, only the basic functions listed in this
 *  header excluding extensions need to be implemented.
 *
 * [Rules]
 *  - We use the utf-8 encoding for all "const char *" and "char *" string pointers
 *  - Return values of type "char *" must be freed by callers
 */

#ifndef SUIKA_PLATFORM_H
#define SUIKA_PLATFORM_H

#include "types.h"

/***********************************************************************
 * HAL API                                                             *
 ***********************************************************************/

/**************
 * Structures *
 **************/

/*
 * Image:
 *  - We use "struct image *" for image objects
 *  - An actual definition of "struct image" can vary:
 *    - Currently we have the only implementation in image.c
 *    - In the past there was an implementation for Android's OpenGL wrapper
 */
struct image;

/*
 * Sound stream:
 *  - We use "struct wave *" for PCM streams
 *  - Currently We have two implementations:
 *    - wave.c: a generic OggVorbis decoder that uses libvorbisfile
 *    - ndkwave.c: a backend implementation with Android's MediaPlayer class
 */
struct wave;

/***********
 * Logging *
 ***********/

/*
 * Put a info log with printf formats.
 *  - An "info" level log will be put into log.txt file
 *  - Note that sound threads cannot use logging
 */
bool log_info(const char *s, ...);

/*
 * Put a warn log with printf formats.
 *  - A "warn" level log will be put into log.txt file and a dialog will be shown
 *  - Note that sound threads cannot use logging
 */
bool log_warn(const char *s, ...);

/*
 * Put a error log with printf formats.
 *  - An "error" level log will be put into log.txt file and a dialog will be shown
 *  - Note that sound threads cannot use logging
 */
bool log_error(const char *s, ...);

/*********************
 * Path Manipulation *
 *********************/

/*
 * Create a save directory if it is does not exist.
 */
bool make_sav_dir(void);

/*
 * Create an effective path string from a directory name and a file name.
 *  - This function absorbs OS-specific path handling
 *  - Resulting strings can be passed to open_rfile() and open_wfile()
 *  - Resulting strings must be freed by caller
 */
char *make_valid_path(const char *dir, const char *fname);

/**************
 * GPU Checks *
 **************/

/*
 * Return whether OpenGL is enabled.
 *  - When we use OpenGL, we reorder pixels from RGBA to BGRA in readimage.c
 *  - A HAL can have multiple rendering backends and choose one on a startup time
 */
bool is_opengl_enabled(void);

/************************
 * Texture Manipulation *
 ************************/

/*
 * Lock a texture object.
 *  - Here, "texture object" is a VRAM backend of "struct image *"
 *  - This function abstracts a texture lock operation for a rendering backend in use
 *  - While a texture of an image is locked, stage.c can use draw_image() to the image
 *    - draw_image() to non-locked images are not allowed
 *  - A lock operation can be failed and in the case draw_image() renders to CPU memory
 *    - Pixels will be transfered on a later lock or unlock operation
 */
bool
lock_texture(int width,			/* image width */
	     int height,		/* image height */
	     pixel_t *pixels,		/* [IN] image pixels on CPU memory */
	     pixel_t **locked_pixels,	/* [OUT] temporary pixel pointer */
	     void **texture);		/* [OUT] a backend-dependent texture object */

/*
 * Unlock a texture object.
 *  - This function abstracts a texture unlock operation for a rendering backend in use
 *  - Currently all GPU rendering subsystem upload pixels to VRAM on unlock operations
 */
void
unlock_texture(int width,		/* image width */
	       int height,		/* image height */
	       pixel_t *pixels,		/* [IN] image pixels on CPU memory */
	       pixel_t **locked_pixels,	/* [OUT] temporary pixel pointer */
	       void **texture);		/* [IN/OUT] a backend-dependent texture object */

/*
 * Destroy a texture object.
 *  - This function abstracts a texture destruction for a rendering backend in use
 *  - It is called from destroy_image() only
 */
void destroy_texture(void *texture);

/*************
 * Rendering *
 *************/

/*
 * Render an image to the screen with the "normal" shader.
 *  - The "normal" shader renders pixels with alpha blending
 */
void
render_image(int dst_left,	/* X coordinate of the screen */
	     int dst_top,	/* Y coordinate of the screen */
	     struct image * RESTRICT src_image, /* [IN] an image to be rendered */
	     int width,		/* width of a drawing rectangle */
	     int height		/* height of a drawing rectangle */,
	     int src_left,	/* X coordinate of a source image */
	     int src_top,	/* Y coordinate of a source image */
	     int alpha,		/* alpha value (0 to 255) */
	     int bt);		/* blend type (see also "enum blend_type" in image.h) */

/*
 * Render an image to the screen with the "dimming" shader.
 *  - The "dimming" shader renders pixels at 50% values for each RGB component
 */
void
render_image_dim(int dst_left,	/* X coordinate of the screen */
		 int dst_top,	/* Y coordinate of the screen */
		 struct image * RESTRICT src_image, /* [IN] an image to be rendered */
		 int width,	/* width of a drawing rectangle */
		 int height,	/* height of a drawing rectangle */
		 int src_left,	/* X coordinate of a source image */
		 int src_top);	/* X coordinate of a source image */

/*
 * Render an image to the screen with the "rule" shader.
 *  - The "rule" shader is a variation of "universal transition" with **threshold**
 *  - A rule image must have the same size as the screen
 */
void
render_image_rule(struct image * RESTRICT src_img,	/* [IN] a source image */
		  struct image * RESTRICT rule_img,	/* [IN] a rule image */
		  int threshold);			/* threshold (0 to 255) */

/*
 * Render an image to the screen with the "melt" shader.
 *  - The "melt" shader is a variation of "universal transition" with a progress value
 *  - A rule image must have the same size as the screen
 */
void render_image_melt(struct image * RESTRICT src_img,		/* [IN] a source image */
		       struct image * RESTRICT rule_img,	/* [IN] a rule image */
		       int progress);				/* progress (0 to 255) */

/*************
 * Lap Timer *
 *************/

/*
 * Reset a lap timer and initialize it with a current time.
 */
void reset_stop_watch(stop_watch_t *t);

/*
 * Get a timer lap in milliseconds.
 *  - FIXME: use uint64_t.
 */
int get_stop_watch_lap(stop_watch_t *t);

/******************
 * Sound Playback *
 ******************/

/*
 * Note: we have the following sound streams:
 *  - BGM_STREAM: background music
 *  - SE_STREAM: sound effect
 *  - VOICE_STREAM: character voice
 *  - SYS_STREAM: system sound
 */

/*
 * Start playing a sound file on a track.
 */
bool
play_sound(int stream,		/* A sound stream index */
	   struct wave *w);	/* [IN] A sound object, delegate to callee */

/*
 * Stop playing a sound track.
 */
bool stop_sound(int stream);

/*
 * Set sound volume.
 */
bool set_sound_volume(int stream, float vol);

/*
 * Return whether a sound playback for a stream is already finished.
 *  - This is mainly to detect voice playback finish
 */
bool is_sound_finished(int stream);

/*******************************
 * Confirmation Prompt Dialogs *
 *******************************/

/*
 * Show a quit dialog.
 */
bool exit_dialog(void);

/*
 * Show a go-back-to-title dialog.
 */
bool title_dialog(void);

/*
 * Show a save data deletion dialog.
 */
bool delete_dialog(void);

/*
 * Show a overwrite save data dialog.
 */
bool overwrite_dialog(void);

/*
 * Show a reset settings dialog.
 */
bool default_dialog(void);

/******************
 * Video Playback *
 ******************/

/*
 * Start playing a video file.
 */
bool play_video(const char *fname,	/* file name */
		bool is_skippable);	/* allow skip for a unseen video */

/*
 * Stop playing music stream.
 */
void stop_video(void);

/*
 * Returns whether a video playcack is running.
 */
bool is_video_playing(void);

/***********************
 * Window Manipulation *
 ***********************/

/*
 * Set a window title.
 */
void update_window_title(void);

/*
 * Return whether the current HAL supports the "full screen mode".
 *  - The "full screen mode" includes docking of some game consoles
 *  - A HAL can implement the "full screen mode" but it is optional
 */
bool is_full_screen_supported(void);

/*
 * Return whether the current HAL is in the "full screen mode".
 */
bool is_full_screen_mode(void);

/*
 * Enter the full screen mode.
 *  - A HAL can ignore this call
 */
void enter_full_screen_mode(void);

/*
 * Leave the full screen mode.
 *  - A HAL can ignore this call
 */
void leave_full_screen_mode(void);

/**********
 * Locale *
 **********/

/*
 * Get the system language.
 *  - Return value can be:
 *    - "ja": Japanese
 *    - "en": English
 *    - "zh": Simplified Chinese
 *    - "tw": Traditional Chinese
 *    - "fr": French
 *    - "it": Italian
 *    - "es": Spanish (Castellano)
 *    - "de": Deutsch
 *    - "el": Greek
 *    - "ru": Russian
 *    - "other": Other (must fallback to English)
 *  - To add a language, please make changes to the following:
 *    - "enum language_code" in conf.h
 *    - init_locale_code() in conf.c
 *    - set_locale_mapping() in conf.c
 *    - get_ui_message() in uimsg.c
 *  - Note that:
 *    - Currently we don't have a support for right-to-left writing systems
 *      - It should be implemented in draw_msg_common() in glyph.c
 *    - Glyphs that are composed from multiple Unicode codepoints are not supported
 *      - Currently we need pre-composed texts
 */
const char *get_system_locale(void);

/******************
 * Text-To-Speech *
 ******************/

/* Currently we support TTS on Windows only. */
#if defined(WIN)
/*
 * Do TTS.
 *  - text: Specify NULL to stop speaking.
 */
void speak_text(const char *text);
#else
static inline void speak_text(UNUSED(const char *text)) { }
#endif

/***********************************************************************
 * HAL-DBG API                                                         *
 ***********************************************************************/
/*
 * For Suika2 Pro (of debugger versions):
 *  - HAL-DBG is for debuggers and it cannot change script models
 *  - A script view is expected to be a list view that is not edited by a user
 */
#if defined(USE_DEBUGGER) || defined(USE_EDITOR)

/*
 * Return whether the "continue" botton is pressed.
 */
bool is_continue_pushed(void);

/*
 * Return whether the "next" button is pressed.
 */
bool is_next_pushed(void);

/*
 * Return whether the "stop" button is pressed.
 */
bool is_stop_pushed(void);

/*
 * Return whether the "open" button is pressed.
 */
bool is_script_opened(void);

/*
 * Return a script file name when the "open" button is pressed.
 */
const char *get_opened_script(void);

/*
 * Return whether the "execution line number" is changed.
 */
bool is_exec_line_changed(void);

/*
 * Return the "execution line number" if it is changed.
 */
int get_changed_exec_line(void);

/*
 * Update UI elements when the running state is changed.
 */
void on_change_running_state(bool running, bool request_stop);

/*
 * Update UI elements when the main engine changes the script to be executed.
 */
void on_load_script(void);

/*
 * Update UI elements when the main engine changes the command position to be executed.
 */
void on_change_position(void);

/*
 * Update UI elements when the main engine changes variables.
 */
void on_update_variable(void);

/*
 * The following are not used in the newer versions of Suika2 Pro:
 */
#if !defined(USE_EDITOR)
/*
 * Return whether "current command string" is updated.
 */
bool is_command_updated(void);

/*
 * Return the "current command string" if it is changed.
 */
const char *get_updated_command(void);

/*
 * Return whether the "reload" button is pressed.
 */
bool is_script_reloaded(void);
#endif

#endif /* defined(USE_DEBUGGER) || defined(USE_EDITOR) */

/***********************************************************************
 * HAL-CAP API                                                         *
 ***********************************************************************/
/*
 * For Suika2 Capture and Suika2 Replay
 */
#if defined(USE_CAPTURE) || defined(USE_REPLAY)

/*
 * Remove and re-create an output directory.
 */
bool reconstruct_dir(const char *dir);

/*
 * Get a clock in a millisecond precise.
 *  - For replay: a clock value is one from cvs
 */
uint64_t get_tick_count64(void);

/*
 * A wrapper for fopen().
 */
FILE *fopen_wrapper(const char *dir, const char *file, const char *mode);
#endif /* defined(USE_CAPTURE) || defined(USE_REPLAY) */

#endif /* SUIKA_PLATFORM_H */
