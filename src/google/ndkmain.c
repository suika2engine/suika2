/* -*- coding: utf-8; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Suika 2
 * Copyright (C) 2001-2022, TABATA Keiichi. All rights reserved.
 */

/*
 * NDKメインモジュール
 */

#include "suika.h"
#include "ndkmain.h"
#include "glrender.h"

#include <android/log.h>

#define LOG_BUF_SIZE		(1024)
#define SCROLL_DOWN_MARGIN	(5)

/*
 * JNI関数呼び出しの間だけ有効なJNIEnvへの参照
 */
JNIEnv *jni_env;

/*
 * MainActivityのインスタンスへの参照 
 */
jobject main_activity;

/*
 * ビデオ再生状態
 */
static bool video_playing_flag;

/*
 * 初期化処理を行います。
 */
JNIEXPORT void JNICALL
Java_jp_luxion_suika_MainActivity_init(
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

	/* envをグローバル変数で参照するのを終了する */
	jni_env = NULL;
}

/*
 * 再初期化処理を行います。ビデオ再生処理からの復帰を行います。
 */
JNIEXPORT void JNICALL
Java_jp_luxion_suika_MainActivity_reinit(
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
Java_jp_luxion_suika_MainActivity_cleanup(
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

/*
 * イメージをレンダリングする
 */
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
				width,
				height,
				alpha);
}

/*
 * イメージを暗くレンダリングする
 */
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
	return true;
}

/*
 * データのディレクトリ名とファイル名を指定して有効なパスを取得する
 */
char *make_valid_path(const char *dir, const char *fname)
{
	/* NDKでは使用しない */
	assert(0);
	return NULL;
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
	jclass cls;
	jmethodID mid;
	const char *file;
	bool loop;

	file = get_wave_file_name(w);
	loop = is_wave_looped(w);

	/* サウンドの再生を開始する */
	cls = (*jni_env)->FindClass(jni_env, "jp/luxion/suika/MainActivity");
	mid = (*jni_env)->GetMethodID(jni_env, cls, "playSound", "(ILjava/lang/String;Z)V");
	(*jni_env)->CallVoidMethod(jni_env, main_activity, mid, n, (*jni_env)->NewStringUTF(jni_env, file), loop ? JNI_TRUE : JNI_FALSE);
	return true;
}

/*
 * サウンドの再生を停止する
 */
bool stop_sound(int n)
{
	jclass cls;
	jmethodID mid;

	/* サウンドの再生を停止する */
	cls = (*jni_env)->FindClass(jni_env, "jp/luxion/suika/MainActivity");
	mid = (*jni_env)->GetMethodID(jni_env, cls, "stopSound", "(I)V");
	(*jni_env)->CallVoidMethod(jni_env, main_activity, mid, n);
	return true;
}

/*
 * サウンドのボリュームを設定する
 */
bool set_sound_volume(int n, float vol)
{
	jclass cls;
	jmethodID mid;

	/* サウンドの再生を停止する */
	cls = (*jni_env)->FindClass(jni_env, "jp/luxion/suika/MainActivity");
	mid = (*jni_env)->GetMethodID(jni_env, cls, "setVolume", "(IF)V");
	(*jni_env)->CallVoidMethod(jni_env, main_activity, mid, n, vol);
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
	return "other";
}

void speak_text(const char *text)
{
	UNUSED_PARAMETER(text);
}
