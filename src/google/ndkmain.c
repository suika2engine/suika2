/* -*- coding: utf-8; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Suika2
 * Copyright (C) 2001-2023, Keiichi Tabata. All rights reserved.
 */

/*
 * JNI code for Android NDK.
 *
 * [Changes]
 *  - 2016-08-06 Created.
 *  - 2023-12-26 Refactored.
 */

/* Suika2 Base */
#include "suika.h"

/* HAL */
#include "ndkmain.h"
#include "glrender.h"
#include "slsound.h"

/* Standard C */
#include <locale.h>

/* NDK */
#include <android/log.h>

/*
 * Constants
 */

#define LOG_BUF_SIZE		(1024)
#define SCROLL_DOWN_MARGIN	(5)

/*
 * Variables
 */

/*
 * The global reference to the MainActivity instance.
 */
jobject main_activity;

/*
 * A temporal reference to a JNIEnv.
 */
JNIEnv *jni_env;

/* A flag that indicates if a video is playing back. */
static bool state_video;

JNIEXPORT void JNICALL
Java_jp_luxion_suika_MainActivity_nativeInitGame(
	JNIEnv *env,
	jobject instance)
{
	jni_env = env;

	/* Retain the main activity instance globally. */
	main_activity = (*env)->NewGlobalRef(env, instance);

	/* Clear the video state. */
	state_video = false;

	/* Init Suika2 engine. */
	init_locale_code();
	if (!init_conf()) {
		log_error("Failed to initialize config.");
		return;
	}
	if (!init_opengl()) {
		log_error("Failed to initialize OpenGL.");
		return;
	}
	init_opensl_es();
	if (!on_event_init()) {
		log_error("Failed to initialize event loop.");
		return;
	}

	jni_env = NULL;
}

JNIEXPORT void JNICALL
Java_jp_luxion_suika_MainActivity_nativeReinitOpenGL(
        JNIEnv *env,
        jobject instance)
{
	jni_env = env;

	/* Make sure to retain the main activity instance. */
	main_activity = (*env)->NewGlobalRef(env, instance);

	/* Re-initialize OpenGL. */
	if (!init_opengl()) {
		log_error("Failed to initialize OpenGL.");
		return;
	}

	state_video = false;

	jni_env = NULL;
}

JNIEXPORT void JNICALL
Java_jp_luxion_suika_MainActivity_nativeCleanup(
	JNIEnv *env,
	jobject instance)
{
	jni_env = env;

	on_event_cleanup();
	cleanup_conf();

	/* Delete the global reference to the main activity instance. */
	if (main_activity != NULL) {
		(*env)->DeleteGlobalRef(env, main_activity);
		main_activity = NULL;
	}

	jni_env = NULL;
}

JNIEXPORT jboolean JNICALL
Java_jp_luxion_suika_MainActivity_nativeRunFrame(
	JNIEnv *env,
	jobject instance)
{
	bool do_render;

	jni_env = env;

	/* Process a video playback. */
	bool draw = true;
	if (state_video) {
		jclass cls = (*jni_env)->FindClass(jni_env, "jp/luxion/suika/MainActivity");
		jmethodID mid = (*jni_env)->GetMethodID(jni_env, cls, "bridgeIsVideoPlaying", "()Z");
		if ((*jni_env)->CallBooleanMethod(jni_env, main_activity, mid))
			do_render = false;
		else
			state_video = false;
	}

	/* Start a rendering. */
	if (do_render)
		opengl_start_rendering();

	/* Run a frame. */
	jboolean ret = JNI_TRUE;
	if (!on_event_frame())
		ret = JNI_FALSE;

	/* Finish a rendering. */
	if (do_render)
		opengl_end_rendering();

	jni_env = NULL;

	return ret;
}

JNIEXPORT void JNICALL
Java_jp_luxion_suika_MainActivity_nativeOnPause(
        JNIEnv *env,
        jobject instance,
        jint x,
        jint y)
{
	jni_env = env;
	sl_pause_sound();
	jni_env = NULL;
}

JNIEXPORT void JNICALL
Java_jp_luxion_suika_MainActivity_nativeOnResume(
        JNIEnv *env,
        jobject instance,
        jint x,
        jint y)
{
	jni_env = env;
	sl_resume_sound();
	jni_env = NULL;
}

JNIEXPORT void JNICALL
Java_jp_luxion_suika_MainActivity_nativeOnTouchOneDown(
        JNIEnv *env,
        jobject instance,
        jint x,
        jint y)
{
	jni_env = env;
	on_event_mouse_press(MOUSE_LEFT, x, y);
	jni_env = NULL;
}

JNIEXPORT void JNICALL
Java_jp_luxion_suika_MainActivity_nativeOnTouchTwoDown(
        JNIEnv *env,
        jobject instance,
        jint x,
        jint y)
{
	jni_env = env;
	on_event_mouse_press(MOUSE_RIGHT, x, y);
	jni_env = NULL;
}

JNIEXPORT void JNICALL
Java_jp_luxion_suika_MainActivity_nativeOnTouchMove(
	JNIEnv *env,
	jobject instance,
	jint x,
	jint y)
{
	jni_env = env;
	on_event_mouse_move(x, y);
	jni_env = NULL;
}

JNIEXPORT void JNICALL
Java_jp_luxion_suika_MainActivity_nativeOnTouchScrollUp(
	JNIEnv *env,
	jobject instance)
{
	jni_env = env;
        on_event_key_press(KEY_UP);
        on_event_key_release(KEY_UP);
	jni_env = NULL;
}

JNIEXPORT void JNICALL
Java_jp_luxion_suika_MainActivity_nativeOnTouchScrollDown(
	JNIEnv *env,
	jobject instance)
{
	jni_env = env;
        on_event_key_press(KEY_DOWN);
        on_event_key_release(KEY_DOWN);
	jni_env = NULL;
}

JNIEXPORT void JNICALL
Java_jp_luxion_suika_MainActivity_nativeOnTouchOneUp(
	JNIEnv *env,
	jobject instance,
	jint x,
	jint y)
{
	jni_env = env;
        on_event_mouse_release(MOUSE_LEFT, x, y);
	jni_env = NULL;
}

JNIEXPORT void JNICALL
Java_jp_luxion_suika_MainActivity_nativeOnTouchTwoUp(
	JNIEnv *env,
	jobject instance,
	jint x,
	jint y)
{
	jni_env = env;
        on_event_mouse_release(MOUSE_RIGHT, x, y);
	jni_env = NULL;
}

/*
 * HAL
 */

bool log_info(const char *s, ...)
{
	char buf[LOG_BUF_SIZE];
	va_list ap;

	va_start(ap, s);
	vsnprintf(buf, sizeof(buf), s, ap);
	__android_log_print(ANDROID_LOG_INFO, "Suika", "%s", buf);
	va_end(ap);
	return true;
}

bool log_warn(const char *s, ...)
{
	char buf[LOG_BUF_SIZE];
	va_list ap;

	va_start(ap, s);
	vsnprintf(buf, sizeof(buf), s, ap);
	__android_log_print(ANDROID_LOG_WARN, "Suika", "%s", buf);
	va_end(ap);
	return true;
}

bool log_error(const char *s, ...)
{
	char buf[LOG_BUF_SIZE];
	va_list ap;

	va_start(ap, s);
	vsnprintf(buf, sizeof(buf), s, ap);
	__android_log_print(ANDROID_LOG_ERROR, "Suika", "%s", buf);
	va_end(ap);
	return true;
}

void notify_image_update(struct image *img)
{
	opengl_notify_image_update(img);
}

void notify_image_free(struct image *img)
{
	opengl_notify_image_free(img);
}

void render_image_normal(int dst_left,
			 int dst_top,
			 int dst_width,
			 int dst_height,
			 struct image *src_image,
			 int src_left,
			 int src_top,
			 int src_width,
			 int src_height,
			 int alpha)
{
	opengl_render_image_normal(dst_left,
				   dst_top,
				   dst_width,
				   dst_height,
				   src_image,
				   src_left,
				   src_top,
				   src_width,
				   src_height,
				   alpha);
}

void render_image_add(int dst_left,
		      int dst_top,
		      int dst_width,
		      int dst_height,
		      struct image *src_image,
		      int src_left,
		      int src_top,
		      int src_width,
		      int src_height,
		      int alpha)
{
	opengl_render_image_add(dst_left,
				dst_top,
				dst_width,
				dst_height,
				src_image,
				src_left,
				src_top,
				src_width,
				src_height,
				alpha);
}

void render_image_dim(int dst_left,
		      int dst_top,
		      int dst_width,
		      int dst_height,
		      struct image *src_image,
		      int src_left,
		      int src_top,
		      int src_width,
		      int src_height,
		      int alpha)
{
	opengl_render_image_dim(dst_left,
				dst_top,
				dst_width,
				dst_height,
				src_image,
				src_left,
				src_top,
				src_width,
				src_height,
				alpha);
}

void render_image_rule(struct image *src_img, struct image *rule_img, int threshold)
{
	opengl_render_image_rule(src_img, rule_img, threshold);
}

void render_image_melt(struct image *src_img, struct image *rule_img, int progress)
{
	opengl_render_image_melt(src_img, rule_img, progress);
}

bool make_sav_dir(void)
{
	/* Note: We don't create a sav directory for engine-android. */
	return true;
}

char *make_valid_path(const char *dir, const char *fname)
{
	/* Note: We don't use a POSIX path for engine-android. */
	assert(0);
	return NULL;
}

void reset_lap_timer(uint64_t *t)
{
	struct timeval tv;

	gettimeofday(&tv, NULL);

	*t = (uint64_t)(tv.tv_sec * 1000 + tv.tv_usec / 1000);
}

uint64_t get_lap_timer_millisec(uint64_t *t)
{
	struct timeval tv;
	uint64_t end;
	
	gettimeofday(&tv, NULL);

	end = (uint64_t)(tv.tv_sec * 1000 + tv.tv_usec / 1000);

	return (uint64_t)(end - *t);
}

bool exit_dialog(void)
{
	/* stub */
	return true;
}

bool title_dialog(void)
{
	/* stub */
	return true;
}

bool delete_dialog(void)
{
	/* stub */
	return true;
}

bool overwrite_dialog(void)
{
	/* stub */
	return true;
}

bool default_dialog(void)
{
	/* stub */
	return true;
}

bool play_video(const char *fname, bool is_skippable)
{
	state_video = true;

	jstring file = (*jni_env)->NewStringUTF(jni_env, fname);

	jclass cls = (*jni_env)->FindClass(jni_env, "jp/luxion/suika/MainActivity");
	jmethodID mid = (*jni_env)->GetMethodID(jni_env, cls, "bridgePlayVideo", "(Ljava/lang/String;Z)V");
	(*jni_env)->CallVoidMethod(jni_env,
				   main_activity,
				   mid,
				   file,
				   is_skippable ? JNI_TRUE : JNI_FALSE);

	return true;
}

void stop_video(void)
{
	state_video = false;

	jclass cls = (*jni_env)->FindClass(jni_env, "jp/luxion/suika/MainActivity");
	jmethodID mid = (*jni_env)->GetMethodID(jni_env, cls, "bridgeStopVideo", "()V");
	(*jni_env)->CallVoidMethod(jni_env, main_activity, mid);
}

bool is_video_playing(void)
{
	if (state_video) {
		jclass cls = (*jni_env)->FindClass(jni_env, "jp/luxion/suikapro/MainActivity");
		jmethodID mid = (*jni_env)->GetMethodID(jni_env, cls, "bridgeIsVideoPlaying", "()V");
		if (!(*jni_env)->CallBooleanMethod(jni_env, main_activity, mid)) {
			state_video = false;
			return false;
		}
	}
	return false;
}

void update_window_title(void)
{
	/* FIXME: Do we have a window name on ChromeOS? */
}

bool is_full_screen_supported(void)
{
	/* FIXME: Do we have a full screen support on ChromeOS? */
	return false;
}

bool is_full_screen_mode(void)
{
	/* FIXME: Do we have a full screen support on ChromeOS? */
	return false;
}

void enter_full_screen_mode(void)
{
	/* FIXME: Do we have a full screen support on ChromeOS? */
}

void leave_full_screen_mode(void)
{
	/* FIXME: Do we have a full screen support on ChromeOS? */
}

const char *get_system_locale(void)
{
	const char *locale;

	locale = setlocale(LC_ALL, "");
	if (locale == NULL)
		return "en";
	else if (locale[0] == '\0' || locale[1] == '\0')
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

void speak_text(const char *text)
{
	UNUSED_PARAMETER(text);
}
