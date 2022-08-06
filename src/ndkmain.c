/* -*- coding: utf-8; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Suika 2
 * Copyright (C) 2001-2016, TABATA Keiichi. All rights reserved.
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
 * 初期化処理を行います。
 */
JNIEXPORT void JNICALL
Java_jp_luxion_suika_MainActivity_init(
	JNIEnv *env,
	jobject instance)
{
	/* この関数呼び出しの間だけenvをグローバル変数で参照する */
	jni_env = env;

	/* Activityを保持する */
	main_activity = (*env)->NewGlobalRef(env, instance);

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
	int x, y, w, h;
	jboolean ret;

	/* この関数呼び出しの間だけenvをグローバル変数で参照する */
	jni_env = env;

	opengl_start_rendering();

	if (!on_event_frame(&x, &y, &w, &h))
		ret = JNI_FALSE;
	else
		ret = JNI_TRUE;

	opengl_end_rendering();

	/* envをグローバル変数で参照するのを終了する */
	jni_env = NULL;

	return ret;
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
 * タッチ(左クリック)を処理します。
 */
JNIEXPORT void JNICALL
Java_jp_luxion_suika_MainActivity_touchLeftClick(
	JNIEnv *env,
	jobject instance,
	jint x,
	jint y)
{
        on_event_mouse_press(MOUSE_LEFT, x, y);
}

/*
 * タッチ(右クリック)を処理します。
 */
JNIEXPORT void JNICALL
Java_jp_luxion_suika_MainActivity_touchRightClick(
	JNIEnv *env,
	jobject instance,
	jint x,
	jint y)
{
        on_event_mouse_press(MOUSE_RIGHT, x, y);
}

/*
 * platform.hの実装
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
 * UTF-8のメッセージをネイティブの文字コードに変換する
 *  - 変換の必要がないので引数をそのまま返す
 */
const char *conv_utf8_to_native(const char *utf8_message)
{
	return utf8_message;
}

/*
 * GPUを使うか調べる
 */
bool is_gpu_accelerated(void)
{
	return true;
}

/*
 * OpenGLが有効か調べる
 */
bool is_opengl_enabled(void)
{
	return true;
}

/*
 * テクスチャをロックする
 */
bool lock_texture(int width, int height, pixel_t *pixels,
				  pixel_t **locked_pixels, void **texture)
{
	assert(*locked_pixels == NULL);

	if (!opengl_lock_texture(width, height, pixels, locked_pixels,
				 texture))
		return false;

	return true;
}

/*
 * テクスチャをアンロックする
 */
void unlock_texture(int width, int height, pixel_t *pixels,
					pixel_t **locked_pixels, void **texture)
{
	assert(*locked_pixels != NULL);

	opengl_unlock_texture(width, height, pixels, locked_pixels, texture);
}

/*
 * テクスチャを破棄する
 */
void destroy_texture(void *texture)
{
	opengl_destroy_texture(texture);
}

/*
 * イメージをレンダリングする
 */
void render_image(int dst_left, int dst_top, struct image * RESTRICT src_image,
                  int width, int height, int src_left, int src_top, int alpha,
                  int bt)
{
	opengl_render_image(dst_left, dst_top, src_image, width, height,
			    src_left, src_top, alpha, bt);
}

/*
 * イメージをルールつきでレンダリングする
 */
void render_image_rule(struct image * RESTRICT src_img,
		       struct image * RESTRICT rule_img,
		       int threshold)
{
	opengl_render_image_rule(src_img, rule_img, threshold);
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
void reset_stop_watch(stop_watch_t *t)
{
	jclass cls;
	jmethodID mid;
	long ret;

	/* 現在の時刻を取得する */
	cls = (*jni_env)->FindClass(jni_env, "java/lang/System");
	mid = (*jni_env)->GetStaticMethodID(jni_env, cls, "currentTimeMillis", "()J");
	ret = (*jni_env)->CallStaticLongMethod(jni_env, cls, mid);

	/* 時刻を格納する */
	*t = (stop_watch_t)ret;
}

/*
 * タイマのラップをミリ秒単位で取得する
 */
int get_stop_watch_lap(stop_watch_t *t)
{
	jclass cls;
	jmethodID mid;
	long ret;

	/* 現在の時刻を取得する */
	cls = (*jni_env)->FindClass(jni_env, "java/lang/System");
	mid = (*jni_env)->GetStaticMethodID(jni_env, cls, "currentTimeMillis", "()J");
	ret = (*jni_env)->CallStaticLongMethod(jni_env, cls, mid);

	return (int)(ret - (long)*t);
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
	UNUSED_PARAMETER(fname);

	/* stub */
	return true;
}

/*
 * ビデオを停止する
 */
void stop_video(void)
{
	/* stub */
}

/*
 * ビデオが再生中か調べる
 */
bool is_video_playing(void)
{
	/* stub */
	return false;
}

/*
 * ウィンドウタイトルを更新する
 */
void update_window_title(void)
{
}
