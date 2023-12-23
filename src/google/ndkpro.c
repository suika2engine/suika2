/* -*- coding: utf-8; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Suika2
 * Copyright (C) 2001-2023, Keiichi Tabata. All rights reserved.
 */

/*
 * Suika2 Pro for Mobile (Android NDK)
 */

/* Suika2 Base */
#include "suika.h"

/* Suika2 Pro */
#include "pro.h"

/* HAL */
#include "glrender.h"

/* Standard C */
#include <locale.h>	/* setlocale() */

/* POSIX */
#include <sys/types.h>
#include <sys/stat.h>	/* stat(), mkdir() */
#include <sys/time.h>	/* gettimeofday() */
#include <unistd.h>	/* usleep(), access() */

/* JNI */
#include <jni.h>

/* Android NDK */
#include <android/log.h>

/*
 * Constants
 */

#define LOG_BUF_SIZE		(1024)
#define SCROLL_DOWN_MARGIN	(5)

/*
 * Variables
 */

/* The reference to the MainActivity instance. */
static jobject main_activity;

/* JNIEnv pointer that is only effective in a JNI call and used by some HAL functions. */
static JNIEnv *jni_env;

/* Control status. */
static bool is_running;
static bool is_continue_pressed;
static bool is_next_pressed;
static bool is_stop_pressed;
static bool is_line_changed;
static int changed_line;
static bool is_opened;
static char *opened_file;
static bool video_playing_flag;

/*
 * Initialize the game.
 */
JNIEXPORT void JNICALL
Java_jp_luxion_suikapro_MainActivity_nativeInitGame(
	JNIEnv *env,
	jobject instance,
	jstring basePath)
{
	video_playing_flag = false;

	/* Retain the main activity. */
	main_activity = (*env)->NewGlobalRef(env, instance);

	/* Save the env pointer to a global variable until the end of this call. */
	jni_env = env;

	/* Change the working directory. */
	const char *cstr = (*env)->GetStringUTFChars(env, basePath, 0);
	chdir(cstr);
	(*env)->ReleaseStringUTFChars(env, basePath, cstr);

	/* Initialize the locale. */
	init_locale_code();

	/* Initialize the config. */
	if (!init_conf()) {
		log_error("Failed to initialize config.");
		exit(1);
	}

	/* Initialize the graphics subsystem for OpenGL ES. */
	if (!init_opengl()) {
		log_error("Failed to initialize OpenGL.");
		exit(1);
	}

	/* Initialize the game. */
	if (!on_event_init()) {
		log_error("Failed to initialize event loop.");
		exit(1);
	}

	/* Reset flags. */
	is_running = false;
	is_continue_pressed = false;
	is_next_pressed = false;
	is_stop_pressed = false;
	is_line_changed = false;
	changed_line = 0;
	is_opened = false;
	if (opened_file != NULL) {
		free(opened_file);
		opened_file = NULL;
	}

	/* Finish referencing the env pointer. */
	jni_env = NULL;
}

/*
 * Re-initialze after a video playback.
 */
JNIEXPORT void JNICALL
Java_jp_luxion_suikapro_MainActivity_nativeReinitOpenGL(
        JNIEnv *env,
        jobject instance)
{
	video_playing_flag = false;

	/* Save the env pointer to a global variable until the end of this call. */
	jni_env = env;

	/* Retain the main activity. (FIXME: not needed?) */
	main_activity = (*env)->NewGlobalRef(env, instance);

	/* Re-initialize OpenGL ES. */
	if (!init_opengl()) {
		log_error("Failed to initialize OpenGL.");
		exit(1);
	}

	/* Finish referencing the env pointer. */
	jni_env = NULL;
}

/*
 * フレーム処理を行います。
 */
JNIEXPORT void JNICALL
Java_jp_luxion_suikapro_MainActivity_nativeRunFrame(
	JNIEnv *env,
	jobject instance)
{
	/* Save the env pointer to a global variable until the end of this call. */
	jni_env = env;

	/* Process video playback. */
	bool draw = true;
	if (video_playing_flag) {
		jclass cls = (*jni_env)->FindClass(jni_env, "jp/luxion/suika/MainActivity");
		jmethodID mid = (*jni_env)->GetMethodID(jni_env, cls, "isVideoPlaying", "()Z");
		if ((*jni_env)->CallBooleanMethod(jni_env, main_activity, mid))
			draw = false;
		else
			video_playing_flag = false;
	}

	/* Start rendering. */
	if (draw)
		opengl_start_rendering();

	/* Do a frame. */
	if (!on_event_frame())
		exit(1);

	/* End rendering. */
	if (draw)
		opengl_end_rendering();

	/* Finish referencing the env pointer. */
	jni_env = NULL;
}

/*
 * Called on one-finger touch.
 */
JNIEXPORT void JNICALL
Java_jp_luxion_suikapro_MainActivity_nativeOnTouchOneDown(
        JNIEnv *env,
        jobject instance,
        jint x,
        jint y)
{
	jni_env = env;
	on_event_mouse_press(MOUSE_LEFT, x, y);
	jni_env = NULL;
}

/*
 * Called on two-finger touch.
 */
JNIEXPORT void JNICALL
Java_jp_luxion_suikapro_MainActivity_nativeOnTouchTwoDown(
        JNIEnv *env,
        jobject instance,
        jint x,
        jint y)
{
	jni_env = env;
	on_event_mouse_press(MOUSE_RIGHT, x, y);
	jni_env = NULL;
}

/*
 * Called on touch-move.
 */
JNIEXPORT void JNICALL
Java_jp_luxion_suikapro_MainActivity_nativeOnTouchMove(
	JNIEnv *env,
	jobject instance,
	jint x,
	jint y)
{
	jni_env = env;
	on_event_mouse_move(x, y);
	jni_env = NULL;
}

/*
 * Called on touch-scroll-up.
 */
JNIEXPORT void JNICALL
Java_jp_luxion_suikapro_MainActivity_nativeOnTouchScrollUp(
	JNIEnv *env,
	jobject instance)
{
	jni_env = env;
        on_event_key_press(KEY_UP);
        on_event_key_release(KEY_UP);
	jni_env = NULL;
}

/*
 * Called on touch-scroll-down.
 */
JNIEXPORT void JNICALL
Java_jp_luxion_suikapro_MainActivity_nativeOnTouchScrollDown(
	JNIEnv *env,
	jobject instance)
{
	jni_env = env;
        on_event_key_press(KEY_DOWN);
        on_event_key_release(KEY_DOWN);
	jni_env = NULL;
}

/*
 * Called on a release of one-finger touch.
 */
JNIEXPORT void JNICALL
Java_jp_luxion_suikapro_MainActivity_nativeOnTouchOneUp(
	JNIEnv *env,
	jobject instance,
	jint x,
	jint y)
{
	jni_env = env;
        on_event_mouse_release(MOUSE_LEFT, x, y);
	jni_env = NULL;
}

/*
 * Called on a release of two-finger touch.
 */
JNIEXPORT void JNICALL
Java_jp_luxion_suikapro_MainActivity_nativeOnTouchTwoUp(
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
 * Called on a left mouse button down.
 */
JNIEXPORT void JNICALL
Java_jp_luxion_suikapro_MainActivity_nativeOnMouseLeftDown(
	JNIEnv *env,
	jobject instance,
	jint x,
	jint y)
{
	jni_env = env;
        on_event_mouse_press(MOUSE_LEFT, x, y);
	jni_env = NULL;
}

/*
 * Called on a right mouse button down.
 */
JNIEXPORT void JNICALL
Java_jp_luxion_suikapro_MainActivity_nativeOnMouseLeftUp(
	JNIEnv *env,
	jobject instance,
	jint x,
	jint y)
{
	jni_env = env;
        on_event_mouse_release(MOUSE_LEFT, x, y);
	jni_env = NULL;
}

/*
 * Called on a right mouse button down.
 */
JNIEXPORT void JNICALL
Java_jp_luxion_suikapro_MainActivity_nativeOnMouseRightDown(
	JNIEnv *env,
	jobject instance,
	jint x,
	jint y)
{
	jni_env = env;
        on_event_mouse_press(MOUSE_RIGHT, x, y);
	jni_env = NULL;
}

/*
 * Called on a right mouse button up.
 */
JNIEXPORT void JNICALL
Java_jp_luxion_suikapro_MainActivity_nativeOnMouseRightUp(
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
 * Called on a mouse move.
 */
JNIEXPORT void JNICALL
Java_jp_luxion_suikapro_MainActivity_nativeOnMouseMove(
	JNIEnv *env,
	jobject instance,
	jint x,
	jint y)
{
        on_event_mouse_move(x, y);
}

/*
 * Get an integer config value.
 */
JNIEXPORT jint JNICALL
Java_jp_luxion_suikapro_MainActivity_nativeGetIntConfigForKey(
	JNIEnv *env,
	jobject instance,
	jstring key)
{
	jni_env = env;
	const char *cstr = (*env)->GetStringUTFChars(env, key, 0);
	jint ret = get_int_config_value_for_key(cstr);
	(*env)->ReleaseStringUTFChars(env, key, cstr);
	jni_env = NULL;
	return ret;
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

	jclass cls = (*jni_env)->FindClass(jni_env, "jp/luxion/suika/MainActivity");
	jmethodID mid = (*jni_env)->GetMethodID(jni_env, cls, "alert", "(Ljava/lang/String;)V");
	(*jni_env)->CallVoidMethod(jni_env, main_activity, mid, (*jni_env)->NewStringUTF(jni_env, buf));

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

	jclass cls = (*jni_env)->FindClass(jni_env, "jp/luxion/suika/MainActivity");
	jmethodID mid = (*jni_env)->GetMethodID(jni_env, cls, "alert", "(Ljava/lang/String;)V");
	(*jni_env)->CallVoidMethod(jni_env, main_activity, mid, (*jni_env)->NewStringUTF(jni_env, buf));

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

	jclass cls = (*jni_env)->FindClass(jni_env, "jp/luxion/suika/MainActivity");
	jmethodID mid = (*jni_env)->GetMethodID(jni_env, cls, "alert", "(Ljava/lang/String;)V");
	(*jni_env)->CallVoidMethod(jni_env, main_activity, mid, (*jni_env)->NewStringUTF(jni_env, buf));

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

void render_image_normal(
	int dst_left,
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

void render_image_add(
	int dst_left,
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

void render_image_dim(
	int dst_left,
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
	struct stat st = {0};

	if (stat(SAVE_DIR, &st) == -1)
		mkdir(SAVE_DIR, 0700);

	return true;
}

char *make_valid_path(const char *dir, const char *fname)
{
	char *buf;
	size_t len;

	if (dir == NULL)
		dir = "";

	/* パスのメモリを確保する */
	len = strlen(dir) + 1 + strlen(fname) + 1;
	buf = malloc(len);
	if (buf == NULL) {
		log_memory();
		return NULL;
	}

	strcpy(buf, dir);
	if (strlen(dir) != 0)
		strcat(buf, "/");
	strcat(buf, fname);

	return buf;
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

bool play_sound(int n, struct wave *w)
{
	/* stub */
	return true;
}

bool stop_sound(int n)
{
	/* stub */
	return true;
}

bool set_sound_volume(int n, float vol)
{
	/* stub */
	return true;
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

bool is_sound_finished(int stream)
{
	return false;
}

bool play_video(const char *fname, bool is_skippable)
{
	video_playing_flag = true;

	jclass cls = (*jni_env)->FindClass(jni_env, "jp/luxion/suika/MainActivity");
	jmethodID mid = (*jni_env)->GetMethodID(jni_env, cls, "playVideo", "(Ljava/lang/String;Z)V");
	(*jni_env)->CallVoidMethod(jni_env, main_activity, mid, (*jni_env)->NewStringUTF(jni_env, fname), is_skippable ? JNI_TRUE : JNI_FALSE);

	return true;
}

void stop_video(void)
{
	video_playing_flag = false;

	jclass cls = (*jni_env)->FindClass(jni_env, "jp/luxion/suika/MainActivity");
	jmethodID mid = (*jni_env)->GetMethodID(jni_env, cls, "stopVideo", "()V");
	(*jni_env)->CallVoidMethod(jni_env, main_activity, mid);
}

bool is_video_playing(void)
{
	return video_playing_flag;
}

void update_window_title(void)
{
	/* TODO: Does ChromeOS have window titles? */
}

bool is_full_screen_supported(void)
{
	return false;
}

bool is_full_screen_mode(void)
{
	return false;
}

void enter_full_screen_mode(void)
{
}

void leave_full_screen_mode(void)
{
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

/*
 * Pro HAL
 */

bool is_continue_pushed(void)
{
	bool ret = is_continue_pressed;
	is_continue_pressed = false;
	return ret;
}

bool is_next_pushed(void)
{
	bool ret = is_next_pressed;
	is_next_pressed = false;
	return ret;
}

bool is_stop_pushed(void)
{
	bool ret = is_stop_pressed;
	is_stop_pressed = false;
	return ret;
}

bool is_script_opened(void)
{
	bool ret = is_opened;
	is_opened = false;
	return ret;
}

const char *get_opened_script(void)
{
	/* stub */
	return "init.txt";
}

bool is_exec_line_changed(void)
{
	bool ret = is_line_changed;
	is_line_changed = false;
	return ret;
}

int get_changed_exec_line(void)
{
	return changed_line;
}

void on_change_running_state(bool running, bool request_stop)
{
	is_running = running;

	if(request_stop)
	{
		/*
		 * 実行中だが停止要求によりコマンドの完了を待機中のとき
		 *  - コントロールとメニューアイテムを無効にする
		 */

		/* 実行中の背景色を設定する */
	}
	else if(running)
	{
		/*
		 * 実行中のとき
		 *  - 「停止」だけ有効、他は無効にする
		 */

		/* 実行中の背景色を設定する */
	}
	else
	{
		/*
		 * 完全に停止中のとき
		 *  - 「停止」だけ無効、他は有効にする
		 */

		/* 次の実行される行の背景色を設定する */
	}
}

void on_load_script(void)
{
	/* スクリプトファイル名を設定する */
	/* スクリプトテキストビューを設定する */
}

void on_change_position(void)
{
	/* 実行行のハイライトを行う */
	/* スクロールする */
}

void on_update_variable(void)
{
	/* TODO */
}

/*
 * stubs for sound
 */

struct wave {
	int dummy;
} wave_instance;

struct wave *create_wave_from_file(const char *dir, const char *file, bool loop)
{
	return &wave_instance;
}

void set_wave_repeat_times(struct wave *w, int n)
{
}

void destroy_wave(struct wave *w)
{
}

const char *get_wave_file_name(struct wave *w)
{
	return "";
}

bool is_wave_looped(struct wave *w)
{
	return false;
}
