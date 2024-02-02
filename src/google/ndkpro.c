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
#include "slsound.h"

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

/* A flag that indicates if the "continue" button is pressed. */
static bool flag_continue;

/* A flag that indicates if the "next" button is pressed. */
static bool flag_next;

/* A flag that indicates if the "stop" button is pressed. */
static bool flag_stop;

/* A flag that indicates if the "open" button is pressed. */
static bool flag_open;

/* A new file name for the flag_open state. */
static char *new_file;

/* A flag that indicates if the execution line is going to be changed. */
static bool flag_line;

/* A new line number for the the flag_line state. */
static int new_line;

/* The video playback state. */
static bool state_video;

/*
 * Forward declarations
 */

static jstring make_script_jstring(void);

/*
 * Exports
 */

JNIEXPORT void JNICALL
Java_jp_luxion_suikapro_MainActivity_nativeInitGame(
	JNIEnv *env,
	jobject instance,
	jstring basePath)
{
	/* Retain the main activity instance globally. */
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
		log_error("Failed to initialize OpenGL ES.");
		exit(1);
	}

	/* Initialize the sound subsystem for ALSA. */
	init_opensl_es();

	/* Initialize the game. */
	if (!on_event_init()) {
		log_error("Failed to initialize event loop.");
		exit(1);
	}

	/* Our process will be recycled and we have to clear the .bss section. */
	flag_continue = false;
	flag_next = false;
	flag_stop = false;
	flag_open = false;
	if (new_file != NULL) {
		free(new_file);
		new_file = NULL;
	}
	flag_line = false;
	new_line = 0;
	state_video = false;

	/* Finish referencing the env pointer. */
	jni_env = NULL;
}

JNIEXPORT void JNICALL
Java_jp_luxion_suikapro_MainActivity_nativeReinitOpenGL(
	JNIEnv *env,
	jobject instance)
{
	jni_env = env;

	/* Make sure to retain the main activity instance globally. */
	main_activity = (*env)->NewGlobalRef(env, instance);

	/* Re-initialize OpenGL ES. */
	if (!init_opengl()) {
		log_error("Failed to initialize OpenGL.");
		exit(1);
	}

	/* Make sure state_video is false. */
	state_video = false;

	jni_env = NULL;
}

JNIEXPORT void JNICALL
Java_jp_luxion_suikapro_MainActivity_nativeRunFrame(
	JNIEnv *env,
	jobject instance)
{
	jni_env = env;

	/* Process video playback. */
	bool draw = true;
	if (state_video) {
		jclass cls = (*jni_env)->FindClass(jni_env, "jp/luxion/suikapro/MainActivity");
		jmethodID mid = (*jni_env)->GetMethodID(jni_env, cls, "isVideoPlaying", "()Z");
		if ((*jni_env)->CallBooleanMethod(jni_env, main_activity, mid))
			draw = false;
		else
			state_video = false;
	}

	/* Start a rendering. */
	if (draw)
		opengl_start_rendering();

	/* Run a frame. */
	if (!on_event_frame())
		exit(1);

	/* End a rendering. */
	if (draw)
		opengl_end_rendering();

	jni_env = NULL;
}

JNIEXPORT void JNICALL
Java_jp_luxion_suikapro_MainActivity_nativeOnPause(
        JNIEnv *env,
        jobject instance)
{
	jni_env = env;
	sl_pause_sound();
	jni_env = NULL;
}

JNIEXPORT void JNICALL
Java_jp_luxion_suikapro_MainActivity_nativeOnResume(
        JNIEnv *env,
        jobject instance)
{
	jni_env = env;
	sl_resume_sound();
	jni_env = NULL;
}

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

JNIEXPORT void JNICALL
Java_jp_luxion_suikapro_MainActivity_nativeOnMouseMove(
	JNIEnv *env,
	jobject instance,
	jint x,
	jint y)
{
	jni_env = env;
	on_event_mouse_move(x, y);
	jni_env = NULL;
}

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

JNIEXPORT void JNICALL
Java_jp_luxion_suikapro_MainActivity_nativeSetContinueFlag(
	JNIEnv *env,
	jobject instance)
{
	flag_continue = true;
}

JNIEXPORT void JNICALL
Java_jp_luxion_suikapro_MainActivity_nativeSetNextFlag(
	JNIEnv *env,
	jobject instance)
{
	flag_next = true;
}

JNIEXPORT void JNICALL
Java_jp_luxion_suikapro_MainActivity_nativeSetStopFlag(
	JNIEnv *env,
	jobject instance)
{
	flag_stop = true;
}

JNIEXPORT void JNICALL
Java_jp_luxion_suikapro_MainActivity_nativeSetOpenFlag(
	JNIEnv *env,
	jobject instance,
	jstring file)
{
	if (new_file != NULL) {
		free(new_file);
		new_file = NULL;
	}

	const char *cstr = (*env)->GetStringUTFChars(env, file, 0);
	new_file = strdup(cstr);
	(*env)->ReleaseStringUTFChars(env, file, cstr);

	flag_open = true;
}

JNIEXPORT void JNICALL
Java_jp_luxion_suikapro_MainActivity_nativeSetLineFlag(
	JNIEnv *env,
	jobject instance,
	jint line)
{
	new_line = line;
	flag_line = true;
}

JNIEXPORT void JNICALL
Java_jp_luxion_suikapro_MainActivity_nativeSaveScript(
	JNIEnv *env,
	jobject instance,
	jint line)
{
	save_script();
}

JNIEXPORT jboolean JNICALL
Java_jp_luxion_suikapro_MainActivity_nativeUpdateScriptModel(
	JNIEnv *env,
	jobject instance,
	jstring script)
{
	/* Copy the script text. */
	const char *cstr = (*env)->GetStringUTFChars(env, script, 0);
	char *text = strdup(cstr);
	(*env)->ReleaseStringUTFChars(env, script, cstr);

	/* Reset parse errors and will notify a first error. */
	dbg_reset_parse_error_count();

	/* Update the script model. */
	int total = (int)strlen(cstr);
	int line_num = 0;
	int line_start = 0;
	while (line_start < total) {
		/* Cut the current line by replacing LF to NUL. */
		const char *line_text = text + line_start;
		char *lf = strstr(text + line_start, "\n");
		if (lf != NULL)
			*lf = '\0';

		/* 行を更新する */
		if (line_num < get_line_count())
			update_script_line(line_num, line_text);
		else
			insert_script_line(line_num, line_text);

		/* Get the line length. */
		int line_len = (lf != NULL) ?
			(int)(lf - (text + line_start)) :
			(int)strlen(text + line_start);

		/* Increment the line number and the line start position. */
		line_num++;
		line_start += line_len + 1; /* +1 for LF */
	}
	free(text);

	/* Delete the lines that have been shortened. */
	for (int i = get_line_count() - 1; i >= line_num; i--)
		delete_script_line(line_num);

	/* Reparse for "<<<" syntax. */
	reparse_script_for_structured_syntax();

	/* If there are syntax errors, return false. */
	if (dbg_get_parse_error_count() > 0)
		return JNI_FALSE;

	/* No syntax error, so return true. */
	return JNI_TRUE;
}

JNIEXPORT jstring JNICALL
Java_jp_luxion_suikapro_MainActivity_nativeGetScript(
	JNIEnv *env,
	jobject instance)
{
	jni_env = env;
	jstring ret = make_script_jstring();
	jni_env = NULL;

	return ret;
}

static jstring make_script_jstring(void)
{
	/* Get the total bytes of the script string. */
	int total = 0;
	int lines = get_line_count();
	for (int i = 0; i < lines; i++) {
		const char *line_string = get_line_string_at_line_num(i);
		total += strlen(line_string) + 1; /* +1 for '\n' */
	}
	total++; /* +1 for '\0' */

	/* Allocate a buffer for the string.  */
	char *buf = malloc(total);
	if (buf == NULL)
		return NULL;

	/* Copy the script to buf. */
	char *cur = buf;
	for (int i = 0; i < lines; i++) {
		const char *line_string = get_line_string_at_line_num(i);
		int line_len = strlen(line_string);
		memcpy(cur, line_string, line_len);
		*(cur + line_len) = '\n';
		cur += line_len + 1;
	}
	*cur = '\0';

	/* Create a Java String. */
	jstring ret = (*jni_env)->NewStringUTF(jni_env, buf);
	free(buf);

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

	jclass cls = (*jni_env)->FindClass(jni_env, "jp/luxion/suikapro/MainActivity");
	jmethodID mid = (*jni_env)->GetMethodID(jni_env, cls, "bridgeAlert", "(Ljava/lang/String;)V");
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

	jclass cls = (*jni_env)->FindClass(jni_env, "jp/luxion/suikapro/MainActivity");
	jmethodID mid = (*jni_env)->GetMethodID(jni_env, cls, "bridgeAlert", "(Ljava/lang/String;)V");
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

	jclass cls = (*jni_env)->FindClass(jni_env, "jp/luxion/suikapro/MainActivity");
	jmethodID mid = (*jni_env)->GetMethodID(jni_env, cls, "bridgeAlert", "(Ljava/lang/String;)V");
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

	jclass cls = (*jni_env)->FindClass(jni_env, "jp/luxion/suikapro/MainActivity");
	jmethodID mid = (*jni_env)->GetMethodID(jni_env, cls, "playVideo", "(Ljava/lang/String;Z)V");
	(*jni_env)->CallVoidMethod(jni_env, main_activity, mid, (*jni_env)->NewStringUTF(jni_env, fname), is_skippable ? JNI_TRUE : JNI_FALSE);

	return true;
}

void stop_video(void)
{
	state_video = false;

	jclass cls = (*jni_env)->FindClass(jni_env, "jp/luxion/suikapro/MainActivity");
	jmethodID mid = (*jni_env)->GetMethodID(jni_env, cls, "stopVideo", "()V");
	(*jni_env)->CallVoidMethod(jni_env, main_activity, mid);
}

bool is_video_playing(void)
{
	if (state_video) {
		jclass cls = (*jni_env)->FindClass(jni_env, "jp/luxion/suikapro/MainActivity");
		jmethodID mid = (*jni_env)->GetMethodID(jni_env, cls, "isVideoPlaying", "()Z");
		if (!(*jni_env)->CallBooleanMethod(jni_env, main_activity, mid)) {
			state_video = false;
			return false;
		}
	}
	return false;
}

void update_window_title(void)
{
	/*
	 * FIXME:
	 *  - Does ChromeOS have window titles?
	 *  - If so, I'll implement this function.
	 */
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
		return "de";
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
	assert(jni_env != NULL);

	bool ret = flag_continue;
	flag_continue = false;
	return ret;
}

bool is_next_pushed(void)
{
	assert(jni_env != NULL);

	bool ret = flag_next;
	flag_next = false;
	return ret;
}

bool is_stop_pushed(void)
{
	assert(jni_env != NULL);

	bool ret = flag_stop;
	flag_stop = false;
	return ret;
}

bool is_script_opened(void)
{
	assert(jni_env != NULL);

	bool ret = flag_open;
	flag_open = false;
	return ret;
}

const char *get_opened_script(void)
{
	assert(jni_env != NULL);

	return new_file;
}

bool is_exec_line_changed(void)
{
	assert(jni_env != NULL);

	bool ret = flag_line;
	flag_line = false;
	return ret;
}

int get_changed_exec_line(void)
{
	assert(jni_env != NULL);

	return new_line;
}

void on_change_running_state(bool running, bool request_stop)
{
	assert(jni_env != NULL);

	jclass cls = (*jni_env)->FindClass(jni_env, "jp/luxion/suikapro/MainActivity");
	jmethodID mid = (*jni_env)->GetMethodID(jni_env, cls, "bridgeChangeRunningState", "(ZZ)V");
	(*jni_env)->CallVoidMethod(jni_env,
				   main_activity,
				   mid,
				   running ? JNI_TRUE: JNI_FALSE,
				   request_stop ? JNI_TRUE : JNI_FALSE);
}

void on_load_script(void)
{
	assert(jni_env != NULL);

	const char *cfile = get_script_file_name();

	jstring file = (*jni_env)->NewStringUTF(jni_env, cfile);
	jstring content = make_script_jstring();

	jclass cls = (*jni_env)->FindClass(jni_env, "jp/luxion/suikapro/MainActivity");
	jmethodID mid = (*jni_env)->GetMethodID(jni_env, cls, "bridgeLoadScript", "(Ljava/lang/String;Ljava/lang/String;)V"); 
	(*jni_env)->CallVoidMethod(jni_env, main_activity, mid, file, content);

	(*jni_env)->DeleteLocalRef(jni_env, file);
	(*jni_env)->DeleteLocalRef(jni_env, content);
}

void on_change_position(void)
{
	assert(jni_env != NULL);

	int line = get_expanded_line_num();

	jclass cls = (*jni_env)->FindClass(jni_env, "jp/luxion/suikapro/MainActivity");
	jmethodID mid = (*jni_env)->GetMethodID(jni_env, cls, "bridgeChangePosition", "(I)V");
	(*jni_env)->CallVoidMethod(jni_env, main_activity, mid, line);
}

void on_update_variable(void)
{
	assert(jni_env != NULL);

	jclass cls = (*jni_env)->FindClass(jni_env, "jp/luxion/suikapro/MainActivity");
	jmethodID mid = (*jni_env)->GetMethodID(jni_env, cls, "bridgeUpdateVariables", "()V");
	(*jni_env)->CallVoidMethod(jni_env, main_activity, mid);
}
