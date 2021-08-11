/* -*- tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Suika 2
 * Copyright (C) 2001-2021, TABATA Keiichi. All rights reserved.
 */

/*
 * [Changes]
 *  2021-06-26 Created.
 */

/* デバッグ用にデフォルトのシェルを使うか */
#undef DEFAULT_SHELL

#include <emscripten/emscripten.h>
#include <emscripten/html5.h>

#include <sys/types.h>
#include <sys/stat.h>	/* stat(), mkdir() */
#include <sys/time.h>	/* gettimeofday() */

#include "suika.h"
#include "emopenal.h"

#ifdef USE_OPENGL
#include "glesrender.h"
#endif

#ifndef USE_OPENGL
/*
 * 背景イメージ
 */
struct image *back_image;
#endif

/*
 * タッチのY座標
 */
static int touch_start_x;
static int touch_start_y;
static int touch_last_y;

/*
 * 前方参照
 */
static bool create_back_image(void);
static EM_BOOL loop_iter(double time, void * userData);
static EM_BOOL cb_mousemove(int eventType,
			    const EmscriptenMouseEvent *mouseEvent,
			    void *userData);
static EM_BOOL cb_mousedown(int eventType,
			    const EmscriptenMouseEvent *mouseEvent,
			    void *userData);
static EM_BOOL cb_mouseup(int eventType,
			  const EmscriptenMouseEvent *mouseEvent,
			  void *userData);
static EM_BOOL cb_wheel(int eventType,
			const EmscriptenWheelEvent *wheelEvent,
			void *userData);
static EM_BOOL cb_keydown(int eventType,
			  const EmscriptenKeyboardEvent *keyEvent,
			  void *userData);
static EM_BOOL cb_keyup(int eventType,
			const EmscriptenKeyboardEvent *keyEvent,
			void *userData);
static int get_keycode(const char *key);
static EM_BOOL cb_touchstart(int eventType,
			     const EmscriptenTouchEvent *touchEvent,
			     void *userData);
static EM_BOOL cb_touchmove(int eventType,
			    const EmscriptenTouchEvent *touchEvent,
			    void *userData);
static EM_BOOL cb_touchend(int eventType,
			   const EmscriptenTouchEvent *touchEvent,
			   void *userData);

int main(void)
{
	/* パッケージの初期化処理を行う */
	if(!init_file())
		return 1;

	/* コンフィグの初期化処理を行う */
	if(!init_conf())
		return 1;
	
	/* サウンドの初期化処理を行う */
	if (!init_openal())
		return 1;

	/* キャンバスサイズを設定する */
	emscripten_set_canvas_element_size("canvas", conf_window_width, conf_window_height);
#ifndef DEFAULT_SHELL
	EM_ASM_({resizeWindow(null);});
#endif

#ifndef USE_OPENGL
	/* バックイメージを作成する */
	if (!create_back_image())
		return 1;
#else
	/* OpenGLレンダを初期化する */
	EmscriptenWebGLContextAttributes attr;
	EMSCRIPTEN_WEBGL_CONTEXT_HANDLE context;
	emscripten_webgl_init_context_attributes(&attr);
	attr.majorVersion = 2;
	attr.minorVersion = 0;
	context = emscripten_webgl_create_context("canvas", &attr);
	emscripten_webgl_make_context_current(context);
	if (!init_opengl())
		return 1;
#endif

	/* 初期化イベントを処理する */
	if(!on_event_init())
		return 1;

	/* イベントの登録をする */
	emscripten_set_mousedown_callback("canvas", 0, true, cb_mousedown);
	emscripten_set_mouseup_callback("canvas", 0, true, cb_mouseup);
	emscripten_set_mousemove_callback("canvas", 0, true, cb_mousemove);
	emscripten_set_wheel_callback("canvas", 0, true, cb_wheel);
	emscripten_set_keydown_callback("canvas", 0, true, cb_keydown);
	emscripten_set_keyup_callback("canvas", 0, true, cb_keyup);
	emscripten_set_touchstart_callback("canvas", 0, true, cb_touchstart);
	emscripten_set_touchmove_callback("canvas", 0, true, cb_touchmove);
	emscripten_set_touchend_callback("canvas", 0, true, cb_touchend);

	/* アニメーションの処理を開始する */
	emscripten_request_animation_frame_loop(loop_iter, 0);

	return 0;
}

#ifndef USE_OPENGL
/* 背景イメージを作成する */
static bool create_back_image(void)
{
	/* 背景イメージを作成する */
	back_image = create_image(conf_window_width, conf_window_height);
	if (back_image == NULL)
		return false;

	/* 初期状態でバックイメージを白く塗り潰す */
	if (conf_window_white) {
		lock_image(back_image);
		clear_image_white(back_image);
		unlock_image(back_image);
	}

	return true;
}
#endif

/* フレームを処理する */
static EM_BOOL loop_iter(double time, void * userData)
{
	int x, y, w, h;
	static bool stop = false;

	/* 停止済みであれば */
	if (stop)
		return EM_FALSE;

	/* サウンドの処理を行う */
	fill_sound_buffer();

#ifndef USE_OPENGL
	/* バックイメージをロックする */
	lock_image(back_image);
#else
	/* フレームのレンダリングを開始する */
	opengl_start_rendering();
#endif

	/* フレームイベントを呼び出す */
	x = y = w = h = 0;
	if (!on_event_frame(&x, &y, &w, &h)) {
		stop = true;

		/* グローバルデータを保存する */
		save_global_data();

		/* スクリプトの終端に達した */
		EM_FALSE;
	}

#ifndef USE_OPENGL
	/* バックイメージをアンロックする */
	unlock_image(back_image);

	/* 描画を行わない場合 */
	if (w == 0 || h == 0)
		return EM_TRUE;

	/* 描画を行う */
	pixel_t *p;
	p = get_image_pixels(back_image);
	EM_ASM_({
		let data = Module.HEAPU8.slice($0, $0 + $1 * $2 * 4);
		let context = Module['canvas'].getContext('2d');
		let imageData = context.getImageData(0, 0, $1, $2);
		imageData.data.set(data);
		context.putImageData(imageData, 0, 0);
	}, p, conf_window_width, conf_window_height);
#else
	/* フレームのレンダリングを終了する */
	opengl_end_rendering();
#endif

	return EM_TRUE;
}

/* mousemoveのコールバック */
static EM_BOOL cb_mousemove(int eventType,
			    const EmscriptenMouseEvent *mouseEvent,
			    void *userData)
{
	double w, h, scale;
	int x, y;

	/*
	 * canvasのCSS上の大きさを取得する
	 *  - canvasの描画領域のサイズではない
	 *  - CSS上の大きさにスケールされて表示される
	 */
	emscripten_get_element_css_size("canvas", &w, &h);

	/* マウス座標をスケーリングする */
	scale = w / (double)conf_window_width;
	x = (int)((double)mouseEvent->targetX / scale);
	y = (int)((double)mouseEvent->targetY / scale);

	on_event_mouse_move(x, y);
	return EM_TRUE;
}

/* Mousedownのコールバック */
static EM_BOOL cb_mousedown(int eventType,
			    const EmscriptenMouseEvent *mouseEvent,
			    void *userData)
{
	double w, h, scale;
	int x, y, button;

	/* マウス座標をスケーリングする */
	emscripten_get_element_css_size("canvas", &w, &h);
	scale = w / (double)conf_window_width;
	x = (int)((double)mouseEvent->targetX / scale);
	y = (int)((double)mouseEvent->targetY / scale);

	if (mouseEvent->button == 0)
		button = MOUSE_LEFT;
	else
		button = MOUSE_RIGHT;

	on_event_mouse_press(button, x, y);
	return EM_TRUE;
}

/* mouseupのコールバック */
static EM_BOOL cb_mouseup(int eventType,
			    const EmscriptenMouseEvent *mouseEvent,
			    void *userData)
{
	double w, h, scale;
	int x, y, button;

	/* マウス座標をスケーリングする */
	emscripten_get_element_css_size("canvas", &w, &h);
	scale = w / (double)conf_window_width;
	x = (int)((double)mouseEvent->targetX / scale);
	y = (int)((double)mouseEvent->targetY / scale);

	if (mouseEvent->button == 0)
		button = MOUSE_LEFT;
	else
		button = MOUSE_RIGHT;

	on_event_mouse_release(button, x, y);
	return EM_TRUE;
}

/* wheelのコールバック */
static EM_BOOL cb_wheel(int eventType,
			const EmscriptenWheelEvent *wheelEvent,
			void *userData)
{
	if (wheelEvent->deltaY > 0) {
		on_event_key_press(KEY_DOWN);
		on_event_key_release(KEY_DOWN);
	} else {
		on_event_key_press(KEY_UP);
		on_event_key_release(KEY_UP);
	}
	return EM_TRUE;
}

/* keydownのコールバック */
static EM_BOOL cb_keydown(int eventType,
			  const EmscriptenKeyboardEvent *keyEvent,
			  void *userData)
{
	int keycode;

	keycode = get_keycode(keyEvent->key);
	if (keycode == -1)
		return EM_TRUE;

	on_event_key_press(keycode);
	return EM_TRUE;
}

/* keyupのコールバック */
static EM_BOOL cb_keyup(int eventType,
			const EmscriptenKeyboardEvent *keyEvent,
			void *userData)
{
	int keycode;

	keycode = get_keycode(keyEvent->key);
	if (keycode == -1)
		return EM_TRUE;

	on_event_key_release(keycode);
	return EM_TRUE;
}

/* キーコードを取得する */
static int get_keycode(const char *key)
{
	if (strcmp(key, "Enter") == 0) {
		return KEY_RETURN;
	} else if (strcmp(key, " ") == 0) {
		return KEY_SPACE;
	} else if (strcmp(key, "Control") == 0) {
		return KEY_CONTROL;
	} else if (strcmp(key, "ArrowUp") == 0) {
		return KEY_UP;
	} else if (strcmp(key, "ArrowDown") == 0) {
		return KEY_DOWN;
	}
	return -1;
}

/* touchstartのコールバック */
static EM_BOOL cb_touchstart(int eventType,
			     const EmscriptenTouchEvent *touchEvent,
			     void *userData)
{
	double w, h, scale;
	int x, y;

	touch_start_x = touchEvent->touches[0].targetX;
	touch_start_y = touchEvent->touches[0].targetY;
	touch_last_y = touchEvent->touches[0].targetY;

	/* マウス座標をスケーリングする */
	emscripten_get_element_css_size("canvas", &w, &h);
	scale = w / (double)conf_window_width;
	x = (int)((double)touchEvent->touches[0].targetX / scale);
	y = (int)((double)touchEvent->touches[0].targetY / scale);
	on_event_mouse_move(x, y);

	return EM_TRUE;
}

/* touchmoveのコールバック */
static EM_BOOL cb_touchmove(int eventType,
			    const EmscriptenTouchEvent *touchEvent,
			    void *userData)
{
	const int LINE_HEIGHT = 10;
	double w, h, scale;
	int delta, x, y;

	delta = touchEvent->touches[0].targetY - touch_last_y;
	touch_last_y = touchEvent->touches[0].targetY;

	if (delta > LINE_HEIGHT) {
		on_event_key_press(KEY_DOWN);
		on_event_key_release(KEY_DOWN);
	} else if (delta < -LINE_HEIGHT) {
		on_event_key_press(KEY_UP);
		on_event_key_release(KEY_UP);
	}

	/* マウス座標をスケーリングする */
	emscripten_get_element_css_size("canvas", &w, &h);
	scale = w / (double)conf_window_width;
	x = (int)((double)touchEvent->touches[0].targetX / scale);
	y = (int)((double)touchEvent->touches[0].targetY / scale);
	on_event_mouse_move(x, y);

	return EM_TRUE;
}

/* touchendのコールバック */
static EM_BOOL cb_touchend(int eventType,
			   const EmscriptenTouchEvent *touchEvent,
			   void *userData)
{
	const int OFS = 10;
	double w, h, scale;
	int x, y;

	/* マウス座標をスケーリングする */
	emscripten_get_element_css_size("canvas", &w, &h);
	scale = w / (double)conf_window_width;
	x = (int)((double)touchEvent->touches[0].targetX / scale);
	y = (int)((double)touchEvent->touches[0].targetY / scale);
	on_event_mouse_move(x, y);

	/* 2本指でタップした場合、右クリックとする */
	if (touchEvent->numTouches == 2) {
		on_event_mouse_press(MOUSE_RIGHT, x, y);
		on_event_mouse_release(MOUSE_RIGHT, x, y);
		return EM_TRUE;
	}

	/* 1本指でタップした場合、左クリックとする */
	if (abs(touchEvent->touches[0].targetX - touch_start_x) < OFS &&
	    abs(touchEvent->touches[0].targetY - touch_start_y) < OFS) {
		on_event_mouse_press(MOUSE_LEFT, x, y);
		on_event_mouse_release(MOUSE_LEFT, x, y);
		return EM_TRUE;
	}
	
	return EM_TRUE;
}

/*
 * JavaScriptからのコールバック
 */

/* タブが表示された際のコールバック */
void EMSCRIPTEN_KEEPALIVE setVisible(int argc, char *argv[])
{
	resume_sound();
}

/* タブが非表示にされた際のコールバック */
void EMSCRIPTEN_KEEPALIVE setHidden(int argc, char *argv[])
{
	pause_sound();
}

/*
 * platform.hの実装
 */

/*
 * INFOログを出力する
 */
bool log_info(const char *s, ...)
{
	va_list ap;
	va_start(ap, s);
	vprintf(s, ap);
	va_end(ap);
	return true;
}

/*
 * WARNログを出力する
 */
bool log_warn(const char *s, ...)
{
	va_list ap;
	va_start(ap, s);
	vprintf(s, ap);
	va_end(ap);
	return true;
}

/*
 * ERRORログを出力する
 */
bool log_error(const char *s, ...)
{
	va_list ap;
	va_start(ap, s);
	vprintf(s, ap);
	va_end(ap);
	return true;
}

/*
 * UTF-8のメッセージをネイティブの文字コードに変換する
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
#ifdef USE_OPENGL
	if (!opengl_lock_texture(width, height, pixels, locked_pixels,
				 texture))
		return false;

	return true;
#else
	assert(*locked_pixels == NULL);

	UNUSED_PARAMETER(width);
	UNUSED_PARAMETER(height);
	UNUSED_PARAMETER(texture);

	*locked_pixels = pixels;

	return true;
#endif
}

/*
 * テクスチャをアンロックする
 */
void unlock_texture(int width, int height, pixel_t *pixels,
					pixel_t **locked_pixels, void **texture)
{
#ifdef USE_OPENGL
	opengl_unlock_texture(width, height, pixels, locked_pixels, texture);
#else
	assert(*locked_pixels != NULL);

	UNUSED_PARAMETER(width);
	UNUSED_PARAMETER(height);
	UNUSED_PARAMETER(pixels);
	UNUSED_PARAMETER(texture);

	*locked_pixels = NULL;
#endif
}

/*
 * テクスチャを破棄する
 */
void destroy_texture(void *texture)
{
#ifdef USE_OPENGL
	opengl_destroy_texture(texture);
#else
	UNUSED_PARAMETER(texture);
#endif
}

/*
 * イメージをレンダリングする
 */
void render_image(int dst_left, int dst_top, struct image * RESTRICT src_image,
                  int width, int height, int src_left, int src_top, int alpha,
                  int bt)
{
#ifdef USE_OPENGL
	opengl_render_image(dst_left, dst_top, src_image, width, height,
			    src_left, src_top, alpha, bt);
#else
	draw_image(back_image, dst_left, dst_top, src_image, width, height,
		   src_left, src_top, alpha, bt);
#endif
}

/*
 * イメージをマスク描画でレンダリングする
 */
void render_image_mask(int dst_left, int dst_top,
                       struct image * RESTRICT src_image,
                       int width, int height, int src_left, int src_top,
                       int mask)
{
#ifdef USE_OPENGL
	opengl_render_image_mask(dst_left, dst_top, src_image, width,
				 height, src_left, src_top, mask);
#else
	draw_image_mask(back_image, dst_left, dst_top, src_image, width,
			height, src_left, src_top, mask);
#endif
}

/*
 * 画面をクリアする
 */
void render_clear(int left, int top, int width, int height, pixel_t color)
{
#ifdef USE_OPENGL
	opengl_render_clear(left, top, width, height, color);
#else
	clear_image_color_rect(back_image, left, top, width, height, color);
#endif
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
 * データファイルのディレクトリ名とファイル名を指定して有効なパスを取得する
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

#ifndef USE_OPENGL
/*
 * バックイメージを取得する
 */
struct image *get_back_image(void)
{
	return back_image;
}
#endif

/*
 * タイマをリセットする
 */
void reset_stop_watch(stop_watch_t *t)
{
	struct timeval tv;

	gettimeofday(&tv, NULL);

	*t = (stop_watch_t)(tv.tv_sec * 1000 + tv.tv_usec / 1000);
}

/*
 * タイマのラップをミリ秒単位で取得する
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
