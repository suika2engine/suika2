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
#include "glrender.h"
#include "emopenal.h"

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

/*
 * メイン
 */
int main(void)
{
	/* セーブデータを読み込む */
	EM_ASM_(
		FS.mkdir('/sav');
		FS.mount(IDBFS, {}, '/sav');
		FS.syncfs(true, function (err) { ccall('main_continue', 'v'); });
	);

	/* 読み込みは非同期で、main_continue()に継続される */
	emscripten_exit_with_live_runtime();
	return 0;
}

/*
 * メインの続き
 */
EMSCRIPTEN_KEEPALIVE void main_continue(void)
{
	/* ロケールを初期化する */
	init_locale_code();

	/* パッケージの初期化処理を行う */
	if(!init_file())
		return;

	/* コンフィグの初期化処理を行う */
	if(!init_conf())
		return;
	
	/* サウンドの初期化処理を行う */
	if (!init_openal())
		return;

	/* キャンバスサイズを設定する */
	emscripten_set_canvas_element_size("canvas", conf_window_width, conf_window_height);
#ifndef DEFAULT_SHELL
	EM_ASM_({resizeWindow(null);});
#endif

	/* OpenGLレンダを初期化する */
	EmscriptenWebGLContextAttributes attr;
	EMSCRIPTEN_WEBGL_CONTEXT_HANDLE context;
	emscripten_webgl_init_context_attributes(&attr);
	attr.majorVersion = 2;
	attr.minorVersion = 0;
	context = emscripten_webgl_create_context("canvas", &attr);
	emscripten_webgl_make_context_current(context);
	if (!init_opengl())
		return;

	/* 初期化イベントを処理する */
	if(!on_event_init())
		return;

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
}

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

	/* フレームのレンダリングを開始する */
	opengl_start_rendering();

	/* フレームイベントを呼び出す */
	x = y = w = h = 0;
	if (!on_event_frame(&x, &y, &w, &h)) {
		stop = true;

		/* グローバルデータを保存する */
		save_global_data();

		/* スクリプトの終端に達した */
		EM_FALSE;
	}

	/* フレームのレンダリングを終了する */
	opengl_end_rendering();

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

/* mousedownのコールバック */
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

	on_event_mouse_press(MOUSE_LEFT, x, y);

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
	fill_sound_buffer();
	if (!opengl_lock_texture(width, height, pixels, locked_pixels,
				 texture))
		return false;
	fill_sound_buffer();
	return true;
}

/*
 * テクスチャをアンロックする
 */
void unlock_texture(int width, int height, pixel_t *pixels,
					pixel_t **locked_pixels, void **texture)
{
	fill_sound_buffer();
	opengl_unlock_texture(width, height, pixels, locked_pixels, texture);
	fill_sound_buffer();
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

/* イメージをルール付きでレンダリングする */
void render_image_rule(struct image * RESTRICT src_img,
		       struct image * RESTRICT rule_img,
		       int threshold)
{
	opengl_render_image_rule(src_img, rule_img, threshold);
}

/* イメージをルール付き(メルト)でレンダリングする */
void render_image_melt(struct image * RESTRICT src_img,
		       struct image * RESTRICT rule_img,
		       int threshold)
{
	opengl_render_image_melt(src_img, rule_img, threshold);
}

/*
 * セーブディレクトリを作成する
 */
bool make_sav_dir(void)
{
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
	if (strcmp(dir, SAVE_DIR) == 0)
		dir = "/sav";

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
 * ビデオを再生する
 */
bool play_video(const char *fname, bool is_skippable)
{
	EM_ASM_({
		document.getElementById("canvas").style.display = "none";

		var v = document.getElementById("video");
		v.style.display = "block";
		v.src = Module.UTF8ToString($0, $1);
		v.load();
		v.addEventListener('ended', function() {
			document.getElementById("canvas").style.display = "block";
			document.getElementById("video").style.display = "none";
		});
		v.play();
	}, fname, strlen(fname));

	return true;
}

/*
 * ビデオを停止する
 */
void stop_video(void)
{
	EM_ASM_(
		var c = document.getElementById("canvas");
		c.style.display = "block";

		var v = document.getElementById("video");
		v.style.display = "none";
		v.pause();
		v.src = "";
		v.load();
	);
}

/*
 * ビデオが再生中か調べる
 */
bool is_video_playing(void)
{
	int ended;

	ended = EM_ASM_INT(
		var v = document.getElementById("video");
		return v.ended;
	);

	return !ended;
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
	/* stub */
	return "other";
}
