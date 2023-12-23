/* -*- coding: utf-8; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Suika2
 * Copyright (C) 2001-2023, Keiichi Tabata. All rights reserved.
 */

/*
 * Suika2 Pro for Mobile (Android NDK)
 */

/* Android NDK */
#include <android/log.h>

/* Suika2 Base */
#include "suika.h"

/* Suika2 Pro */
#include "pro.h"

/* HAL */
#include "ndkmain.h"
#include "glrender.h"

/* Standard C */
#include <locale.h>	/* setlocale() */

/* POSIX */
#include <sys/types.h>
#include <sys/stat.h>	/* stat(), mkdir() */
#include <sys/time.h>	/* gettimeofday() */
#include <unistd.h>	/* usleep(), access() */

/*
 * Constants
 */

#define LOG_BUF_SIZE		(1024)
#define SCROLL_DOWN_MARGIN	(5)

/*
 * Variables
 */

/*
 * JNI関数呼び出しの間だけ有効なJNIEnvへの参照
 */
JNIEnv *jni_env;

/*
 * MainActivityのインスタンスへの参照
 */
jobject main_activity;

/* 状態 */
static bool is_running;
static bool is_continue_pressed;
static bool is_next_pressed;
static bool is_stop_pressed;
static bool is_line_changed;
static int changed_line;
static bool is_opened;
static char *opened_file;

/*
 * ビデオ再生状態
 */
static bool video_playing_flag;

/*
 * ゲーム初期化処理を行います。
 */
JNIEXPORT void JNICALL
Java_jp_luxion_suika_MainActivity_initGame(
	JNIEnv *env,
	jobject instance)
{
	video_playing_flag = false;

	/* この関数呼び出しの間だけenvをグローバル変数で参照する */
	jni_env = env;

	/* Activityを保持する */
	main_activity = (*env)->NewGlobalRef(env, instance);

	/* ロケールを初期化する */
	init_locale_code();

	/* コンフィグの初期化処理を行う */
	if (!init_conf()) {
		log_error("Failed to initialize config.");
		return;
	}

	/* OpenGL ESの初期化を行う */
	if (!init_opengl()) {
		log_error("Failed to initialize OpenGL.");
		return;
	}

	/* アプリケーション本体の初期化処理を行う */
	if (!on_event_init()) {
		log_error("Failed to initialize event loop.");
		return;
	}

	is_running = false;
	is_continue_pressed = false;
	is_next_pressed = false;
	is_stop_pressed = false;
	is_line_changed = false;
	changed_line = 0;
	is_opened = false;
	if (opened_file != NULL)
		free(opened_file);
	opened_file = NULL;

	/* envをグローバル変数で参照するのを終了する */
	jni_env = NULL;
}

/*
 * 再初期化処理を行います。ビデオ再生処理からの復帰を行います。
 */
JNIEXPORT void JNICALL
Java_jp_luxion_suika_MainActivity_reinitGame(
        JNIEnv *env,
        jobject instance)
{
	video_playing_flag = false;

	/* この関数呼び出しの間だけenvをグローバル変数で参照する */
	jni_env = env;

	/* Activityを保持する */
	main_activity = (*env)->NewGlobalRef(env, instance);

	/* OpenGL ESの初期化を行う */
	if (!init_opengl()) {
		log_error("Failed to initialize OpenGL.");
		return;
	}

	/* envをグローバル変数で参照するのを終了する */
	jni_env = NULL;
}

/*
 * 終了処理を行います。
 */
JNIEXPORT void JNICALL
Java_jp_luxion_suika_MainActivity_cleanupGame(
	JNIEnv *env,
	jobject instance)
{
	/* envをグローバル変数で参照する */
	jni_env = env;

	/* アプリケーション本体の終了処理を行う */
	on_event_cleanup();

	/* コンフィグの終了処理を行う */
	cleanup_conf();

	/* MainActivityの保持を終了する */
	if (main_activity != NULL) {
		(*env)->DeleteGlobalRef(env, main_activity);
		main_activity = NULL;
	}

	/* envをグローバル変数で参照するのを終了する */
	jni_env = NULL;
}

/*
 * フレーム処理を行います。
 */
JNIEXPORT jboolean JNICALL
Java_jp_luxion_suika_MainActivity_frame(
	JNIEnv *env,
	jobject instance)
{
	jclass cls;
	jmethodID mid;
	jboolean ret;
	bool draw;

	/* この関数呼び出しの間だけenvをグローバル変数で参照する */
	jni_env = env;

	/* ビデオ再生の処理を行う */
	draw = true;
	if (video_playing_flag) {
		cls = (*jni_env)->FindClass(jni_env, "jp/luxion/suika/MainActivity");
		mid = (*jni_env)->GetMethodID(jni_env, cls, "isVideoPlaying", "()Z");
		if ((*jni_env)->CallBooleanMethod(jni_env, main_activity, mid))
			draw = false;
		else
            video_playing_flag = false;
	}

	/* レンダリングを開始する */
	if (draw)
		opengl_start_rendering();

	/* フレームのコマンド実行を行う */
	if (!on_event_frame())
		ret = JNI_FALSE;
	else
		ret = JNI_TRUE;

	/* レンダリングを終了する */
	if (draw)
		opengl_end_rendering();

	/* envをグローバル変数で参照するのを終了する */
	jni_env = NULL;

	return ret;
}

/*
 * タッチ(左押下)を処理します。
 */
JNIEXPORT void JNICALL
Java_jp_luxion_suika_MainActivity_touchLeftDown(
        JNIEnv *env,
        jobject instance,
        jint x,
        jint y)
{
	/* この関数呼び出しの間だけenvをグローバル変数で参照する */
	jni_env = env;

	on_event_mouse_press(MOUSE_LEFT, x, y);

	/* envをグローバル変数で参照するのを終了する */
	jni_env = NULL;
}

/*
 * タッチ(右押下)を処理します。
 */
JNIEXPORT void JNICALL
Java_jp_luxion_suika_MainActivity_touchRightDown(
        JNIEnv *env,
        jobject instance,
        jint x,
        jint y)
{
	/* この関数呼び出しの間だけenvをグローバル変数で参照する */
	jni_env = env;

	on_event_mouse_press(MOUSE_RIGHT, x, y);

	/* envをグローバル変数で参照するのを終了する */
	jni_env = NULL;
}

/*
 * タッチ(移動)を処理します。
 */
JNIEXPORT void JNICALL
Java_jp_luxion_suika_MainActivity_touchMove(
	JNIEnv *env,
	jobject instance,
	jint x,
	jint y)
{
	on_event_mouse_move(x, y);
}

/*
 * タッチ(上スクロール)を処理します。
 */
JNIEXPORT void JNICALL
Java_jp_luxion_suika_MainActivity_touchScrollUp(
	JNIEnv *env,
	jobject instance)
{
        on_event_key_press(KEY_UP);
        on_event_key_release(KEY_UP);
}

/*
 * タッチ(下スクロール)を処理します。
 */
JNIEXPORT void JNICALL
Java_jp_luxion_suika_MainActivity_touchScrollDown(
	JNIEnv *env,
	jobject instance)
{
        on_event_key_press(KEY_DOWN);
        on_event_key_release(KEY_DOWN);
}

/*
 * タッチ(左解放)を処理します。
 */
JNIEXPORT void JNICALL
Java_jp_luxion_suika_MainActivity_touchLeftUp(
	JNIEnv *env,
	jobject instance,
	jint x,
	jint y)
{
        on_event_mouse_release(MOUSE_LEFT, x, y);
}

/*
 * タッチ(右解放)を処理します。
 */
JNIEXPORT void JNICALL
Java_jp_luxion_suika_MainActivity_touchRightUp(
	JNIEnv *env,
	jobject instance,
	jint x,
	jint y)
{
        on_event_mouse_release(MOUSE_RIGHT, x, y);
}

/*
 * マウス左押下を処理します。
 */
JNIEXPORT void JNICALL
Java_jp_luxion_suika_MainActivity_clickLeftDown(
	JNIEnv *env,
	jobject instance,
	jint x,
	jint y)
{
        on_event_mouse_press(MOUSE_LEFT, x, y);
}

/*
 * マウス左開放を処理します。
 */
JNIEXPORT void JNICALL
Java_jp_luxion_suika_MainActivity_clickLeftUp(
	JNIEnv *env,
	jobject instance,
	jint x,
	jint y)
{
        on_event_mouse_release(MOUSE_LEFT, x, y);
}

/*
 * マウス右押下を処理します。
 */
JNIEXPORT void JNICALL
Java_jp_luxion_suika_MainActivity_clickRightDown(
	JNIEnv *env,
	jobject instance,
	jint x,
	jint y)
{
        on_event_mouse_press(MOUSE_RIGHT, x, y);
}

/*
 * マウス左開放を処理します。
 */
JNIEXPORT void JNICALL
Java_jp_luxion_suika_MainActivity_clickRightUp(
	JNIEnv *env,
	jobject instance,
	jint x,
	jint y)
{
        on_event_mouse_release(MOUSE_RIGHT, x, y);
}

/*
 * HAL
 */

/*
 * infoログを出力する
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

/*
 * warnログを出力する
 */
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

/*
 * errorログを出力する
 */
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

/*
 * テクスチャを更新する
 */
void notify_image_update(struct image *img)
{
	opengl_notify_image_update(img);
}

/*
 * テクスチャを破棄する
 */
void notify_image_free(struct image *img)
{
	opengl_notify_image_free(img);
}

/*
 * イメージをレンダリングする
 */
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

/*
 * イメージをレンダリングする
 */
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

/*
 * イメージを暗くレンダリングする
 */
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

/*
 * イメージをルール付きでレンダリングする
 */
void render_image_rule(struct image *src_img, struct image *rule_img, int threshold)
{
	opengl_render_image_rule(src_img, rule_img, threshold);
}

/*
 * イメージをルール付き(メルト)でレンダリングする
 */
void render_image_melt(struct image *src_img, struct image *rule_img, int progress)
{
	opengl_render_image_melt(src_img, rule_img, progress);
}

/*
 * セーブディレクトリを作成する
 */
bool make_sav_dir(void)
{
	struct stat st = {0};

	if (stat(SAVE_DIR, &st) == -1)
		mkdir(SAVE_DIR, 0700);

	return true;
}

/*
 * データのディレクトリ名とファイル名を指定して有効なパスを取得する
 */
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

/*
 * タイマをリセットする
 */
void reset_lap_timer(uint64_t *t)
{
	jclass cls;
	jmethodID mid;
	long ret;

	/* 現在の時刻を取得する */
	cls = (*jni_env)->FindClass(jni_env, "java/lang/System");
	mid = (*jni_env)->GetStaticMethodID(jni_env, cls, "currentTimeMillis", "()J");
	ret = (*jni_env)->CallStaticLongMethod(jni_env, cls, mid);

	/* 時刻を格納する */
	*t = (uint64_t)ret;
}

/*
 * タイマのラップをミリ秒単位で取得する
 */
uint64_t get_lap_timer_millisec(uint64_t *t)
{
	jclass cls;
	jmethodID mid;
	long ret;

	/* 現在の時刻を取得する */
	cls = (*jni_env)->FindClass(jni_env, "java/lang/System");
	mid = (*jni_env)->GetStaticMethodID(jni_env, cls, "currentTimeMillis", "()J");
	ret = (*jni_env)->CallStaticLongMethod(jni_env, cls, mid);

	return (uint64_t)(ret - (long)*t);
}

/*
 * サウンドを再生を開始する
 */
bool play_sound(int n, struct wave *w)
{
	/* stub */
	return true;
}

/*
 * サウンドの再生を停止する
 */
bool stop_sound(int n)
{
	/* stub */
	return true;
}

/*
 * サウンドのボリュームを設定する
 */
bool set_sound_volume(int n, float vol)
{
	/* stub */
	return true;
}

/*
 * 終了ダイアログを表示する
 */
bool exit_dialog(void)
{
	/* stub */
	return true;
}

/*
 * タイトルに戻るダイアログを表示する
 */
bool title_dialog(void)
{
	/* stub */
	return true;
}

/*
 * 削除ダイアログを表示する
 */
bool delete_dialog(void)
{
	/* stub */
	return true;
}

/*
 * 上書きダイアログを表示する
 */
bool overwrite_dialog(void)
{
	/* stub */
	return true;
}

/*
 * 初期設定ダイアログを表示する
 */
bool default_dialog(void)
{
	/* stub */
	return true;
}

/*
 * サウンドが再生終了したか調べる
 */
bool is_sound_finished(int stream)
{
	return false;
}

/*
 * ビデオを再生する
 */
bool play_video(const char *fname, bool is_skippable)
{
	jclass cls;
	jmethodID mid;

	video_playing_flag = true;

	/* ビデオの再生を開始する */
	cls = (*jni_env)->FindClass(jni_env, "jp/luxion/suika/MainActivity");
	mid = (*jni_env)->GetMethodID(jni_env, cls, "playVideo", "(Ljava/lang/String;Z)V");
	(*jni_env)->CallVoidMethod(jni_env, main_activity, mid, (*jni_env)->NewStringUTF(jni_env, fname), is_skippable ? JNI_TRUE : JNI_FALSE);

	return true;
}

/*
 * ビデオを停止する
 */
void stop_video(void)
{
	jclass cls;
	jmethodID mid;

	video_playing_flag = false;

	/* ビデオの再生を開始する */
	cls = (*jni_env)->FindClass(jni_env, "jp/luxion/suika/MainActivity");
	mid = (*jni_env)->GetMethodID(jni_env, cls, "stopVideo", "()V");
	(*jni_env)->CallVoidMethod(jni_env, main_activity, mid);
}

/*
 * ビデオが再生中か調べる
 */
bool is_video_playing(void)
{
	return video_playing_flag;
}

/*
 * ウィンドウタイトルを更新する
 */
void update_window_title(void)
{
}

/*
 * フルスクリーンモードがサポートされるか調べる
 */
bool is_full_screen_supported(void)
{
	return false;
}

/*
 * フルスクリーンモードであるか調べる
 */
bool is_full_screen_mode(void)
{
	return false;
}

/*
 * フルスクリーンモードを開始する
 */
void enter_full_screen_mode(void)
{
	/* stub */
}

/*
 * フルスクリーンモードを終了する
 */
void leave_full_screen_mode(void)
{
	/* stub */
}

/*
 * システムのロケールを取得する
 */
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

/*
 * 続けるボタンが押されたか調べる
 */
bool is_continue_pushed(void)
{
	bool ret = is_continue_pressed;
	is_continue_pressed = false;
	return ret;
}

/*
 * 次へボタンが押されたか調べる
 */
bool is_next_pushed(void)
{
	bool ret = is_next_pressed;
	is_next_pressed = false;
	return ret;
}

/*
 * 停止ボタンが押されたか調べる
 */
bool is_stop_pushed(void)
{
	bool ret = is_stop_pressed;
	is_stop_pressed = false;
	return ret;
}

/*
 * 実行するスクリプトファイルが変更されたか調べる
 */
bool is_script_opened(void)
{
	bool ret = is_opened;
	is_opened = false;
	return ret;
}

/*
 * 変更された実行するスクリプトファイル名を取得する
 */
const char *get_opened_script(void)
{
	return "init.txt";
}

/*
 * 実行する行番号が変更されたか調べる
 */
bool is_exec_line_changed(void)
{
	bool ret = is_line_changed;
	is_line_changed = false;
	return ret;
}

/*
 * 変更された実行する行番号を取得する
 */
int get_changed_exec_line(void)
{
	return changed_line;
}

/*
 * コマンドの実行中状態を設定する
 */
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

/*
 * スクリプトがロードされたときのコールバック
 */
void on_load_script(void)
{
	/* スクリプトファイル名を設定する */
	/* スクリプトテキストビューを設定する */
}

/*
 * 実行位置が変更されたときのコールバック
 */
void on_change_position(void)
{
	/* 実行行のハイライトを行う */
	/* スクロールする */
}

/*
 * 変数が変更されたときのコールバック
 */
void on_update_variable(void)
{
	/* TODO */
}

/*
 * Stubs
 */

struct wave {
	int dummy;
} wave_instance;

struct wave *create_wave_from_file(const char *dir, const char *file, bool loop)
{
	return &wave_instance;
}

/*
 * PCMストリームのループ回数を設定する
 */
void set_wave_repeat_times(struct wave *w, int n)
{
}

/*
 * PCMストリームを破棄する
 */
void destroy_wave(struct wave *w)
{
}

/*
 * PCMストリームのファイル名を取得する(NDK)
 */
const char *get_wave_file_name(struct wave *w)
{
	return "";
}

/*
 * PCMストリームがループ再生されるかを取得する(NDK)
 */
bool is_wave_looped(struct wave *w)
{
	return false;
}
