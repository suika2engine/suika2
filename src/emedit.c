/* -*- tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Suika2
 * Copyright (C) 2001-2023, Keiichi Tabata. All rights reserved.
 */

/*
 * Suika2 Studio HAL for WASM (Emscripten)
 *
 * [Changes]
 *  2023-11-12 Created.
 */

/* Suika2 Base */
#include "suika.h"

/* Suika2 Graphics HAL for OpenGL */
#include "glrender.h"

/* Suika2 Sound HAL for Emscripten OpenAL */
#include "emopenal.h"

/* Emscripten Core */
#include <emscripten/emscripten.h>
#include <emscripten/html5.h>

/* Emscripten POSIX Emulation */
#include <sys/types.h>
#include <sys/stat.h>	/* stat(), mkdir() */
#include <sys/time.h>	/* gettimeofday() */

/*
 * Constants
 */

/* Frame Milli Seconds */
#define FRAME_MILLI	33

/*
 * Variables
 */

/* Touch Position */
static int touch_start_x;
static int touch_start_y;
static int touch_last_y;

/* Debugger Status */
static bool flag_continue_pushed;
static bool flag_next_pushed;
static bool flag_stop_pushed;
static bool flag_script_opened;
static char *opened_script_name;
static bool flag_exec_line_changed;
static int changed_exec_line;

/*
 * Forward Declarations
 */
static void loop_iter(void *userData);
static EM_BOOL cb_mousemove(int eventType, const EmscriptenMouseEvent *mouseEvent, void *userData);
static EM_BOOL cb_mousedown(int eventType, const EmscriptenMouseEvent *mouseEvent, void *userData);
static EM_BOOL cb_mouseup(int eventType, const EmscriptenMouseEvent *mouseEvent, void *userData);
static EM_BOOL cb_wheel(int eventType, const EmscriptenWheelEvent *wheelEvent, void *userData);
static EM_BOOL cb_keydown(int eventType, const EmscriptenKeyboardEvent *keyEvent, void *userData);
static EM_BOOL cb_keyup(int eventType, const EmscriptenKeyboardEvent *keyEvent, void *userData);
static int get_keycode(const char *key);
static EM_BOOL cb_touchstart(int eventType, const EmscriptenTouchEvent *touchEvent, void *userData);
static EM_BOOL cb_touchmove(int eventType, const EmscriptenTouchEvent *touchEvent,void *userData);
static EM_BOOL cb_touchend(int eventType, const EmscriptenTouchEvent *touchEvent, void *userData);
static void update_script_model_from_text(void);
static void update_script_model_from_current_line_text(void);
static void update_text_from_script_model(void);

/*
 * メイン
 */
int main(void)
{
	emscripten_exit_with_live_runtime();
	return 0;
}

/*
 * エンジン起動
 */
EMSCRIPTEN_KEEPALIVE void start_engine(void)
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
	EM_ASM_({resizeWindow(null);});

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

	/* フレームを予約する */
	emscripten_async_call(loop_iter, NULL, FRAME_MILLI);
}

/* フレームを処理する */
static void loop_iter(void *userData)
{
	static bool is_flip_pending = false;
	int x, y, w, h;

	/* サウンドの処理を行う */
	fill_sound_buffer();

	/*
	 * フレームのレンダリングを開始する
	 *  - ChromeではファイルI/OでawaitするときにglFlush()されてしまうらしい
	 *  - これによるチラつきを避けるためにglClear()を呼ばない
	 */
	/* opengl_start_rendering(); */

	/* フレームのコマンドを実行する */
	x = y = w = h = 0;
	on_event_frame(&x, &y, &w, &h);

	/* フレームのレンダリングを終了する */
	opengl_end_rendering();

	/* 次のフレームを予約する */
	emscripten_async_call(loop_iter, NULL, FRAME_MILLI);
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

/* プロジェクトがロードされたときのコールバック */
EMSCRIPTEN_KEEPALIVE void onLoadProject(void)
{
	start_engine();
}

/* 続けるボタンがクリックされたときのコールバック */
EMSCRIPTEN_KEEPALIVE void onClickContinue(void)
{
	flag_continue_pushed = true;
}

/* 次へボタンがクリックされたときのコールバック */
EMSCRIPTEN_KEEPALIVE void onClickNext(void)
{
	flag_next_pushed = true;
}

/* 停止ボタンがクリックされたときのコールバック */
EMSCRIPTEN_KEEPALIVE void onClickStop(void)
{
	flag_stop_pushed = true;
}

/* タブが表示された際のコールバック */
EMSCRIPTEN_KEEPALIVE void setVisible(void)
{
	resume_sound();
}

/* タブが非表示にされた際のコールバック */
EMSCRIPTEN_KEEPALIVE void setHidden(void)
{
	pause_sound();
}

/* 範囲変更のコールバック */
EMSCRIPTEN_KEEPALIVE void onRangeChange(void)
{
	update_script_model_from_text();
}

/* Ctrl+Returnのコールバック */
EMSCRIPTEN_KEEPALIVE void onCtrlReturn(void)
{
	update_script_model_from_current_line_text();

	changed_exec_line = EM_ASM_INT({
		return editor.getSelection().getRange().start.row;
	});
	flag_exec_line_changed = true;
	flag_next_pushed = true;
}

/*
 * HAL-API for Suika2 Main Engine
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
bool lock_texture(int width, int height, pixel_t *pixels, pixel_t **locked_pixels, void **texture)
{
	fill_sound_buffer();
	if (!opengl_lock_texture(width, height, pixels, locked_pixels, texture))
		return false;
	fill_sound_buffer();
	return true;
}

/*
 * テクスチャをアンロックする
 */
void unlock_texture(int width, int height, pixel_t *pixels, pixel_t **locked_pixels, void **texture)
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

/*
 * イメージを暗くレンダリングする
 */
void render_image_dim(int dst_left, int dst_top,
		      struct image * RESTRICT src_image,
		      int width, int height, int src_left, int src_top)
{
	opengl_render_image_dim(dst_left, dst_top, src_image, width, height,
				src_left, src_top);
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
	return strdup("");
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
	char *path;

	path = make_valid_path(MOV_DIR, fname);

	EM_ASM_({
		document.getElementById("canvas").style.display = "none";

		var v = document.getElementById("video");
		v.style.display = "block";
		v.src = s2GetFileURL(Module.UTF8ToString($0));
		v.load();
		v.addEventListener('ended', function() {
			document.getElementById("canvas").style.display = "block";
			document.getElementById("video").style.display = "none";
		});
		v.play();
	}, path);

	free(path);

	return true;
}

/*
 * ビデオを停止する
 */
void stop_video(void)
{
	EM_ASM_({
		var c = document.getElementById("canvas");
		c.style.display = "block";

		var v = document.getElementById("video");
		v.style.display = "none";
		v.pause();
		v.src = "";
		v.load();
	});
}

/*
 * ビデオが再生中か調べる
 */
bool is_video_playing(void)
{
	int ended;

	ended = EM_ASM_INT({
		var v = document.getElementById("video");
		return v.ended;
	});

	return !ended;
}

/*
 * ウィンドウタイトルを更新する
 */
void update_window_title(void)
{
	const char *separator, *chapter;

	separator = conf_window_title_separator;
	if (separator == NULL)
		separator = " ";

	chapter = get_chapter_name();

	EM_ASM_({
		document.title = 'Suika2 Studio WASM - ' + Module.UTF8ToString($0) + Module.UTF8ToString($1) + Module.UTF8ToString($2);
	}, conf_window_title, separator, chapter);
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
	int lang_code;

	lang_code = EM_ASM_INT({
		if (window.navigator.language.startsWith("ja"))
			return 0;
		return 1;
	});

	if (lang_code == 0)
		return "ja";

	return "en";
}

/*
 * フレームのI/O完了時に呼ばれる
 */
void finish_frame_io(void)
{
	opengl_start_rendering();
}

/*
 * HAL-DBG API for Suika2 Studio
 */

/*
 * Return whether the "continue" botton is pressed.
 */
bool is_continue_pushed(void)
{
	bool ret;
	ret = flag_continue_pushed;
	flag_continue_pushed = false;
	return ret;
}

/*
 * Return whether the "next" button is pressed.
 */
bool is_next_pushed(void)
{
	bool ret;
	ret = flag_next_pushed;
	flag_next_pushed = false;
	return ret;
}

/*
 * Return whether the "stop" button is pressed.
 */
bool is_stop_pushed(void)
{
	bool ret;
	ret = flag_stop_pushed;
	flag_stop_pushed = false;
	return ret;
}

/*
 * Return whether the "open" button is pressed.
 */
bool is_script_opened(void)
{
	bool ret;
	ret = flag_script_opened;
	flag_script_opened = false;
	return ret;
}

/*
 * Return a script file name when the "open" button is pressed.
 */
const char *get_opened_script(void)
{
	return opened_script_name;
}

/*
 * Return whether the "execution line number" is changed.
 */
bool is_exec_line_changed(void)
{
	bool ret;
	ret = flag_exec_line_changed;
	flag_exec_line_changed = false;
	return ret;
}

/*
 * Return the "execution line number" if it is changed.
 */
int get_changed_exec_line(void)
{
	return changed_exec_line;
}

/*
 * Update UI elements when the running state is changed.
 */
void on_change_running_state(bool running, bool request_stop)
{
	if(request_stop) {
		/*
		 * 停止によりコマンドの完了を待機中のとき
		 *  - コントロールとメニューアイテムを無効にする
		 */
		EM_ASM({
			document.getElementById('runningStatus').innerHTML = '実行中(停止待ち)';
			document.getElementById('btnContinue').disabled = 'disabled';
			document.getElementById('btnNext').disabled = 'disabled';
			document.getElementById('btnStop').disabled = 'disabled';
			document.getElementById('btnOpen').disabled = 'disabled';
			document.getElementById('btnMove').disabled = 'disabled';
		});
	} else if(running) {
		/*
		 * 実行中のとき
		 *  - 「停止」だけ有効、他は無効にする
		 */
		EM_ASM({
			document.getElementById('runningStatus').innerHTML = '実行中';
			document.getElementById('btnContinue').disabled = 'disabled';
			document.getElementById('btnNext').disabled = 'disabled';
			document.getElementById('btnStop').disabled = "";
			document.getElementById('btnOpen').disabled = 'disabled';
			document.getElementById('btnMove').disabled = 'disabled';
		});
	} else {
		/*
		 * 完全に停止中のとき
		 *  - 「停止」だけ無効、他は有効にする
		 */
		EM_ASM({
			document.getElementById('runningStatus').innerHTML = '停止中';
			document.getElementById('btnContinue').disabled = "";
			document.getElementById('btnNext').disabled = "";
			document.getElementById('btnStop').disabled = 'disabled';
			document.getElementById('btnOpen').disabled = "";
			document.getElementById('btnMove').disabled = "";
		});
	}
}

/*
 * Update UI elements when the main engine changes the script to be executed.
 */
void on_load_script(void)
{
	const char *script_file;
	int i, line_count;

	script_file = get_script_file_name();

	update_text_from_script_model();
}

/*
 * Update UI elements when the main engine changes the command position to be executed.
 */
void on_change_position(void)
{
	EM_ASM({
		editor.session.clearBreakpoints();
		editor.session.setBreakpoint($0, 'execLine');
		editor.scrollToLine($0, true, true);
	}, get_line_num());
}

/*
 * Update UI elements when the main engine changes variables.
 */
void on_update_variable(void)
{
	/* TODO */
}

/*
 * Script Model
 */

/* テキストを元にスクリプトモデルを更新する */
static void update_script_model_from_text(void)
{
	char line_buf[4096];
	int lines, i;

	/* パースエラーをリセットして、最初のパースエラーで通知を行う */
	dbg_reset_parse_error_count();

	/* エディタの行数を求める */
	lines = EM_ASM_INT({ return editor.session.getLength(); }); /*  */

	/* 各行ごとにアップデートする */
	for (i = 0; i < lines; i++) {
		/* 行を取得する */
		EM_ASM({
			stringToUTF8(editor.session.getLine($0), $1, $2);
		}, i, line_buf, sizeof(line_buf));

		/* 行を更新する */
		if (i < get_line_count())
			update_script_line(i, line_buf);
		else
			insert_script_line(i, line_buf);
	}

	/* 削除された末尾の行を処理する */
	flag_exec_line_changed = false;
	for (i = get_line_count() - 1; i >= lines; i--)
		if (delete_script_line(i))
			flag_exec_line_changed = true;
	if (flag_exec_line_changed) {
		EM_ASM({
			editor.session.clearBreakpoints();
			editor.session.setBreakpoint($0, 'execLine');
			editor.scrollToLine($0, true, true);
			editor.gotoLine($0, 0, true);
		}, get_line_num());
	}

	/* コマンドのパースに失敗した場合 */
	if (dbg_get_parse_error_count() > 0) {
		/* 行頭の'!'を反映するためにテキストを再設定する */
		update_text_from_script_model();
	}
}

/* 現在の行のテキストを元にスクリプトモデルを更新する */
static void update_script_model_from_current_line_text(void)
{
	char line_buf[4096];
	int line;

	/* パースエラーをリセットして、最初のパースエラーで通知を行う */
	dbg_reset_parse_error_count();

	/* 現在のカーソル行番号を取得する */
	line = EM_ASM_INT({
		return editor.getSelection().getRange().start.row;
	});

	/* 行を取得する */
	EM_ASM({
		stringToUTF8(editor.session.getLine($0), $1, $2);
	}, line, line_buf, sizeof(line_buf));

	/* 行を更新する */
	update_script_line(line, line_buf);

	/* コマンドのパースに失敗した場合 */
	if (dbg_get_parse_error_count() > 0) {
		/* 行頭の'!'を反映するためにテキストを再設定する */
		update_text_from_script_model();
	}
}

/* スクリプトモデルを元にエディタのテキストを更新する */
static void update_text_from_script_model(void)
{
	int i, lines;

	lines = get_line_count();
	for (i = 0; i < lines; i++) {
		EM_ASM({
			if ($0 == 0)
				editorText = UTF8ToString($1);
			else
				editorText = editorText + '\n' + UTF8ToString($1);
		}, i, get_line_string_at_line_num(i));
	}

	EM_ASM({
		editor.setValue(editorText);
	});
}
