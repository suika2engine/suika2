/* -*- tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Suika2
 * Copyright (C) 2001-2023, Keiichi Tabata. All rights reserved.
 */

/*
 * HAL for Wasm [Emscripten)
 *
 * [Changes]
 *  2021-06-26 Created.
 */

#include <emscripten/emscripten.h>
#include <emscripten/html5.h>

#include <sys/types.h>
#include <sys/stat.h>	/* stat(), mkdir() */
#include <sys/time.h>	/* gettimeofday() */

/*
 * Suika2 Base
 */
#include "../suika.h"

/*
 * HAL
 */
#include "../khronos/glrender.h"	/* Graphics HAL */
#include "alsound.h"			/* Sound HAL */

/*
 * タッチのY座標
 */
static int touch_start_x;
static int touch_start_y;

/*
 * 前方参照
 */
static EM_BOOL loop_iter(double time, void *userData);
static EM_BOOL cb_mousemove(int eventType, const EmscriptenMouseEvent *mouseEvent, void *userData);
static EM_BOOL cb_mousedown(int eventType, const EmscriptenMouseEvent *mouseEvent, void *userData);
static EM_BOOL cb_mouseup(int eventType, const EmscriptenMouseEvent *mouseEvent, void *userData);
static EM_BOOL cb_wheel(int eventType, const EmscriptenWheelEvent *wheelEvent, void *userData);
static EM_BOOL cb_keydown(int eventType, const EmscriptenKeyboardEvent *keyEvent, void *userData);
static EM_BOOL cb_keyup(int eventType, const EmscriptenKeyboardEvent *keyEvent, void *userData);
static int get_keycode(const char *key);
static EM_BOOL cb_touchstart(int eventType, const EmscriptenTouchEvent *touchEvent, void *userData);
static EM_BOOL cb_touchmove(int eventType, const EmscriptenTouchEvent *touchEvent, void *userData);
static EM_BOOL cb_touchend(int eventType, const EmscriptenTouchEvent *touchEvent, void *userData);
static EM_BOOL cb_touchcancel(int eventType, const EmscriptenTouchEvent *touchEvent, void *userData);

/*
 * メイン
 */
int main(void)
{
	/* ロケールを初期化する */
	init_locale_code();

	/* パッケージの初期化処理を行う */
	if(!init_file())
		return 1;

	/* コンフィグの初期化処理を行う */
	if(!init_conf())
		return 1;

	/* IDBFSのセーブデータをマウントする */
	EM_ASM_({
		FS.mkdir("suika2-sav");
		FS.mount(IDBFS, {}, "suika2-sav");
		FS.syncfs(true, function (err) { Module.ccall('main_continue', 'v'); });
	});

	/* 読み込みは非同期で、main_continue()に継続される */
	emscripten_exit_with_live_runtime();
}

/*
 * メインの続き
 */
EMSCRIPTEN_KEEPALIVE void main_continue(void)
{
	/* サウンドの初期化処理を行う */
	if (!init_openal())
		return;

	/* ウィンドウのタイトルを初期化する */
	update_window_title();

	/* キャンバスサイズを設定する */
	emscripten_set_canvas_element_size("canvas", conf_window_width, conf_window_height);
	EM_ASM_({ resizeWindow(); });

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

	/* イベントハンドラを設定する */
	EM_ASM_({
		function visibilityChange() {
			if(document.visibilityState === 'visible') {
				Module.ccall('setVisible');
				document.getElementById('canvas').focus();
			} else if(document.visibilityState === 'hidden') {
				Module.ccall('setHidden');
			}
		}
		function preventDefault(e) {
			e.preventDefault();
		}
		function onMouseLeave() {
			Module.ccall('mouseLeave');
		}
		window.ontouchmove = preventDefault;
		window.onwheel = preventDefault;
		document.ontouchmove = preventDefault;
		document.onwheel = preventDefault;
		document.body.addEventListener('touchmove', preventDefault, { passive: false });
		document.body.addEventListener('wheel', preventDefault, { passive: false });
		window.addEventListener('resize', resizeWindow);
		document.addEventListener('visibilitychange', visibilityChange);
		document.getElementById('canvas').addEventListener('mouseleave', onMouseLeave);
	});

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
	emscripten_set_touchcancel_callback("canvas", 0, true, cb_touchcancel);

	/* アニメーションの処理を開始する */
	emscripten_request_animation_frame_loop(loop_iter, 0);
}

/* キャンバスをリサイズする */
EM_JS(void, resizeWindow, (void), {
	canvas = document.getElementById('canvas');
	cw = canvas.width;
	ch = canvas.height;
	aspect = cw / ch;
	winw = window.innerWidth;
	winh = window.innerHeight;
	w = winw;
	h = winw / aspect;
	if(h > winh) {
		h = winh;
		w = winh * aspect;
	}
	canvas.style.width = w + 'px';
	canvas.style.height = h + 'px';
	canvas.focus();
});

/* フレームを処理する */
static EM_BOOL loop_iter(double time, void *userData)
{
	static bool stop = false;

	/* 停止済みであれば */
	if (stop)
		return EM_FALSE;

	/* サウンドの処理を行う */
	fill_sound_buffer();

	/* フレームのレンダリングを開始する */
	opengl_start_rendering();

	/* フレームイベントを呼び出す */
	if (!on_event_frame()) {
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

	/* マウス座標をスケーリングする */
	emscripten_get_element_css_size("canvas", &w, &h);
	scale = w / (double)conf_window_width;
	x = (int)((double)touchEvent->touches[0].targetX / scale);
	y = (int)((double)touchEvent->touches[0].targetY / scale);

	on_event_mouse_release(MOUSE_LEFT, -1, -1);
	on_event_mouse_release(MOUSE_RIGHT, -1, -1);

	on_event_mouse_press(MOUSE_LEFT, x, y);

	return EM_TRUE;
}

/* touchmoveのコールバック */
static EM_BOOL cb_touchmove(int eventType,
			    const EmscriptenTouchEvent *touchEvent,
			    void *userData)
{
	double w, h, scale;
	int delta, x, y;

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
	const int FLICK_DISTANCE = 50;
	const int FINGER_DISTANCE = 10;
	double w, h, scale;
	int x, y, delta;

	delta = touchEvent->touches[0].targetY - touch_start_y;
	if (delta > FLICK_DISTANCE) {
		on_event_touch_cancel();
		on_event_swipe_down();
		return EM_TRUE;
	} else if (delta < -FLICK_DISTANCE) {
		on_event_touch_cancel();
		on_event_swipe_up();
		return EM_TRUE;
	}

	/* マウス座標をスケーリングする */
	emscripten_get_element_css_size("canvas", &w, &h);
	scale = w / (double)conf_window_width;
	x = (int)((double)touchEvent->touches[0].targetX / scale);
	y = (int)((double)touchEvent->touches[0].targetY / scale);

	/* 1本指でタップして、距離が誤差範囲内の場合、左クリックとする */
	if (touchEvent->numTouches == 1 &&
	    abs(touchEvent->touches[0].targetX - touch_start_x) < FINGER_DISTANCE &&
	    abs(touchEvent->touches[0].targetY - touch_start_y) < FINGER_DISTANCE) {
		on_event_mouse_press(MOUSE_LEFT, x, y);
		on_event_mouse_release(MOUSE_LEFT, x, y);
		return EM_TRUE;
	}

	/* 2本指でタップした場合、右クリックとする */
	if (touchEvent->numTouches == 2) {
		on_event_touch_cancel();
		on_event_mouse_press(MOUSE_RIGHT, x, y);
		on_event_mouse_release(MOUSE_RIGHT, x, y);
		return EM_TRUE;
	}

	/* その他の場合はタッチをキャンセルする */
	on_event_touch_cancel();

	return EM_TRUE;
}

/* touchcancelのコールバック */
static EM_BOOL cb_touchcancel(int eventType,
			      const EmscriptenTouchEvent *touchEvent,
			      void *userData)
{
	on_event_touch_cancel();

	return EM_TRUE;
}

/*
 * JavaScriptからのコールバック
 */

/* タブが表示された際のコールバック */
void EMSCRIPTEN_KEEPALIVE setVisible(void)
{
	resume_sound();
}

/* タブが非表示にされた際のコールバック */
void EMSCRIPTEN_KEEPALIVE setHidden(void)
{
	pause_sound();
}

/* ポインタがCanvasからはみ出た際のコールバック */
void EMSCRIPTEN_KEEPALIVE mouseLeave(void)
{
	on_event_touch_cancel();
}

/*
 * HAL
 */

/*
 * INFOログを出力する
 */
bool log_info(const char *s, ...)
{
	char buf[1024];

	va_list ap;
	va_start(ap, s);
	vsnprintf(buf, sizeof(buf), s, ap);
	va_end(ap);

	EM_ASM({
		alert(UTF8ToString($0));
	}, buf);

	return true;
}

/*
 * WARNログを出力する
 */
bool log_warn(const char *s, ...)
{
	char buf[1024];

	va_list ap;
	va_start(ap, s);
	vsnprintf(buf, sizeof(buf), s, ap);
	va_end(ap);

	EM_ASM({
		alert(UTF8ToString($0));
	}, buf);

	return true;
}

/*
 * ERRORログを出力する
 */
bool log_error(const char *s, ...)
{
	char buf[1024];

	va_list ap;
	va_start(ap, s);
	vsnprintf(buf, sizeof(buf), s, ap);
	va_end(ap);

	EM_ASM({
		alert(UTF8ToString($0));
	}, buf);

	return true;
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
void notify_image_update(struct image *img)
{
	fill_sound_buffer();
	opengl_notify_image_update(img);
	fill_sound_buffer();
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
	opengl_render_image_normal(dst_left, dst_top, dst_width, dst_height, src_image, src_left, src_top, src_width, src_height, alpha);
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
	opengl_render_image_add(dst_left, dst_top, dst_width, dst_height, src_image, src_left, src_top, src_width, src_height, alpha);
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
	opengl_render_image_dim(dst_left, dst_top, dst_width, dst_height, src_image, src_left, src_top, src_width, src_height, alpha);
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
 * Renders an image to the screen with the "normal" shader pipeline.
 */
void
render_image_3d_normal(
	float x1,
	float y1,
	float x2,
	float y2,
	float x3,
	float y3,
	float x4,
	float y4,
	struct image *src_image,
	int src_left,
	int src_top,
	int src_width,
	int src_height,
	int alpha)
{
	opengl_render_image_3d_normal(x1,
				      y1,
				      x2,
				      y2,
				      x3,
				      y3,
				      x4,
				      y4,
				      src_image,
				      src_left,
				      src_top,
				      src_width,
				      src_height,
				      alpha);
}

/*
 * Renders an image to the screen with the "add" shader pipeline.
 */
void
render_image_3d_add(
	float x1,
	float y1,
	float x2,
	float y2,
	float x3,
	float y3,
	float x4,
	float y4,
	struct image *src_image,
	int src_left,
	int src_top,
	int src_width,
	int src_height,
	int alpha)
{
	opengl_render_image_3d_add(x1,
				   y1,
				   x2,
				   y2,
				   x3,
				   y3,
				   x4,
				   y4,
				   src_image,
				   src_left,
				   src_top,
				   src_width,
				   src_height,
				   alpha);
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
	char buf[1204];
	char *ret;
	size_t len;

	/* If it is a save directory. */
	if (dir != NULL && strcmp(dir, SAVE_DIR) == 0) {
		if (conf_sav_name[0] == '/')
			conf_sav_name[0] = '_';

		snprintf(buf, sizeof(buf), "suika2-sav/%s-%s", conf_sav_name, fname);
		ret = strdup(buf);
		if (ret == NULL) {
			log_memory();
			return NULL;
		}
		return ret;
	}

	/* If it is a top level file. */
	if (dir == NULL) {
		ret = strdup(fname);
		if (ret == NULL) {
			log_memory();
			return NULL;
		}
		return ret;
	}

	/* If it is a file in the mov directory. */
	snprintf(buf, sizeof(buf), "%s/%s", dir, fname);
	ret = strdup(buf);
	if (ret == NULL) {
		log_memory();
		return NULL;
	}
	return ret;
}

/*
 * タイマをリセットする
 */
void reset_lap_timer(uint64_t *t)
{
	struct timeval tv;

	gettimeofday(&tv, NULL);

	*t = (uint64_t)(tv.tv_sec * 1000 + tv.tv_usec / 1000);
}

/*
 * タイマのラップをミリ秒単位で取得する
 */
uint64_t get_lap_timer_millisec(uint64_t *t)
{
	struct timeval tv;
	uint64_t end;
	
	gettimeofday(&tv, NULL);

	end = (uint64_t)(tv.tv_sec * 1000 + tv.tv_usec / 1000);

	return (uint64_t)(end - *t);
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
	char *path;

	path = make_valid_path(MOV_DIR, fname);

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
	}, path, strlen(path));

	free(path);

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
	EM_ASM_({
		document.title = Module.UTF8ToString($0);
	}, conf_window_title);
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
EM_JS(int, get_lang_code, (void), {
	if (window.navigator.language.startsWith("ja"))
		return 0;
	return 1;
});
const char *get_system_locale(void)
{
	int lang_code;

	/* FIXME */
	lang_code = get_lang_code();
	if (lang_code == 0)
		return "ja";
	else
		return "en";
}

void speak_text(const char *text)
{
	UNUSED_PARAMETER(text);
}
