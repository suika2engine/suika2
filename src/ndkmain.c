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
 * 背景イメージ
 */
static struct image *back_image;

/*
 * 初期化処理を行います。
 */
JNIEXPORT jobject JNICALL
Java_jp_luxion_suika_MainActivity_init(
	JNIEnv *env,
	jobject instance,
	jobject assetManager)
{
	jobject back_bitmap;

	/* Activityを保持する */
	main_activity = (*env)->NewGlobalRef(env, instance);

	/* この関数呼び出しの間だけenvをグローバル変数で参照する */
	jni_env = env;

	/* コンフィグの初期化処理を行う */
	if (!init_conf())
		return NULL;

	/* 背景イメージを作成する */
	back_image = create_image(1280, 720);
	back_bitmap = (jobject)get_image_object(back_image);

	/* アプリケーション本体の初期化処理を行う */
	on_event_init();

	/* envをグローバル変数で参照するのを終了する */
	jni_env = NULL;

	return back_bitmap;
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

	/* 背景イメージの破棄を行う */
	if (back_image != NULL) {
		destroy_image(back_image);
		back_image = NULL;
	}

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

	if (!on_event_frame(&x, &y, &w, &h)) {
		ret = JNI_FALSE;
	} else {
		ret = JNI_TRUE;

		// 再描画を行う
		cls = (*jni_env)->FindClass(jni_env, "jp/luxion/suika/MainActivity");
		mid = (*jni_env)->GetMethodID(jni_env, cls, "invalidateView", "()V");
		(*jni_env)->CallVoidMethod(jni_env, main_activity, mid);
	}

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
	mouse_pos_x = (int)x;
	mouse_pos_y = (int)y;
}

/*
 * タッチ(解放)を処理します。
 */
JNIEXPORT void JNICALL
Java_jp_luxion_suika_MainActivity_touchUp(
	JNIEnv *env,
	jobject instance,
	jint x,
	jint y)
{
	mouse_pos_x = (int)x;
	mouse_pos_y = (int)y;
	is_left_button_pressed = true;
}

/*
 * タッチ(ホールド)を処理します。
 */
JNIEXPORT void JNICALL
Java_jp_luxion_suika_MainActivity_touchHold(
	JNIEnv *env,
	jobject instance,
	jint x,
	jint y)
{
	mouse_pos_x = (int)x;
	mouse_pos_y = (int)y;
	is_right_button_pressed = true;
}

/*
 * タッチ(スクロール)を処理します。
 */
JNIEXPORT void JNICALL
Java_jp_luxion_suika_MainActivity_touchScroll(
	JNIEnv *env,
	jobject instance,
	jint x,
	jint y)
{
	int dy;

	dy = y - mouse_pos_y;

	mouse_pos_x = (int)x;
	mouse_pos_y = (int)y;

	if (dy < 0)
		is_up_pressed = true;
	else if (dy > SCROLL_DOWN_MARGIN)
		is_down_pressed = true;
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
 * テクスチャをロックする
 */
bool lock_texture(int width, int height, pixel_t *pixels,
				  pixel_t **locked_pixels, void **texture)
{
	assert(*locked_pixels == NULL);

	UNUSED_PARAMETER(width);
	UNUSED_PARAMETER(height);
	UNUSED_PARAMETER(texture);

	*locked_pixels = pixels;

	return true;
}

/*
 * テクスチャをアンロックする
 */
void unlock_texture(int width, int height, pixel_t *pixels,
					pixel_t **locked_pixels, void **texture)
{
	assert(*locked_pixels != NULL);

	UNUSED_PARAMETER(width);
	UNUSED_PARAMETER(height);
	UNUSED_PARAMETER(pixels);
	UNUSED_PARAMETER(texture);

	*locked_pixels = NULL;
}

/*
 * テクスチャを破棄する
 */
void destroy_texture(void *texture)
{
	UNUSED_PARAMETER(texture);
}

/*
 * イメージをレンダリングする
 */
void render_image(int dst_left, int dst_top, struct image * RESTRICT src_image,
                  int width, int height, int src_left, int src_top, int alpha,
                  int bt)
{
	draw_image(back_image, dst_left, dst_top, src_image, width, height,
			   src_left, src_top, alpha, bt);
}

/*
 * イメージをマスク描画でレンダリングする
 */
void render_image_mask(int dst_left, int dst_top,
                       struct image * RESTRICT src_image,
                       int width, int height, int src_left, int src_top,
                       int mask)
{
	draw_image_mask(back_image, dst_left, dst_top, src_image, width,
			height, src_left, src_top, mask);
}

/*
 * 画面をクリアする
 */
void render_clear(int left, int top, int width, int height, pixel_t color)
{
	clear_image_color_rect(back_image, left, top, width, height, color);
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
 * バックイメージを取得する
 */
struct image *get_back_image(void)
{
	return back_image;
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
/*
	cls = (*jni_env)->FindClass(jni_env, "jp/luxion/suika/MainActivity");
	mid = (*jni_env)->GetMethodID(jni_env, cls, "playSound", "(ILjava/lang/String;Z)V");
	(*jni_env)->CallVoidMethod(jni_env, main_activity, mid, n, (*jni_env)->NewStringUTF(jni_env, file), loop ? JNI_TRUE : JNI_FALSE);
*/
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
/*
	cls = (*jni_env)->FindClass(jni_env, "jp/luxion/suika/MainActivity");
	mid = (*jni_env)->GetMethodID(jni_env, cls, "stopSound", "(I)V");
	(*jni_env)->CallVoidMethod(jni_env, main_activity, mid, n);
*/
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
/*
	cls = (*jni_env)->FindClass(jni_env, "jp/luxion/suika/MainActivity");
	mid = (*jni_env)->GetMethodID(jni_env, cls, "setVolume", "(IF)V");
	(*jni_env)->CallVoidMethod(jni_env, main_activity, mid, n, vol);
*/
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

/* サウンドが再生終了したか調べる */
bool is_sound_finished(int stream)
{
	return false;
}
