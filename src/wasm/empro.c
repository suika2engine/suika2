/* -*- tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Suika2
 * Copyright (C) 2001-2023, Keiichi Tabata. All rights reserved.
 */

/*
 * HAL and Pro HAL for Emscripten
 *
 * [Changes]
 *  2023-11-12 Created.
 */

/* Suika2 Base */
#include "../suika.h"

/* Suika2 Graphics HAL for OpenGL */
#include "../khronos/glrender.h"

/* Suika2 Sound HAL for Emscripten OpenAL */
#include "alsound.h"

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
static bool is_continuous_swipe_enabled;

/* Debugger Status */
static bool is_running;
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
static EM_BOOL cb_touchcancel(int eventType, const EmscriptenTouchEvent *touchEvent, void *userData);
static void update_script_model_from_text(void);
static void update_script_model_from_current_line_text(void);
static void update_text_from_script_model(void);

/*
 * Main
 */
int main(void)
{
	/* Keep the thread alive and will receive events. */
	emscripten_exit_with_live_runtime();
	return 0;
}

/*
 * Startup
 */
EMSCRIPTEN_KEEPALIVE
void start_engine(void)
{
	/* Initialize the locale. */
	init_locale_code();

	/* Initialize the data01.arc package. */
	if(!init_file())
		return;

	/* Initialize the config. */
	if(!init_conf())
		return;

	/* Initialize the OpenAL sound subsystem. */
	if (!init_openal())
		return;

	/* Set the rendering canvas size. */
	emscripten_set_canvas_element_size("canvas", conf_window_width, conf_window_height);
	EM_ASM_({ onResizeWindow(); });

	/* Initialize the OpenGL rendering subsystem. */
	EmscriptenWebGLContextAttributes attr;
	EMSCRIPTEN_WEBGL_CONTEXT_HANDLE context;
	emscripten_webgl_init_context_attributes(&attr);
	attr.majorVersion = 2;
	attr.minorVersion = 0;
	context = emscripten_webgl_create_context("canvas", &attr);
	emscripten_webgl_make_context_current(context);
	if (!init_opengl())
		return;

	/* Execute the startup event. */
	if(!on_event_init())
		return;

	/* Register input events. */
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

	/* Register other events. */
	EM_ASM_({
		window.addEventListener('resize', onResizeWindow);
		document.addEventListener('visibilitychange', function () {
			if(document.visibilityState === 'visible') {
				Module.ccall('setVisible');
				document.getElementById('canvas').focus();
			} else if(document.visibilityState === 'hidden') {
				Module.ccall('setHidden');
			}
		});
		document.getElementById('canvas').addEventListener('mouseleave', function () {
			Module.ccall('mouseLeave');
		});
	});

	/* Reserve the first frame callback. */
	emscripten_async_call(loop_iter, NULL, FRAME_MILLI);
}

EM_JS(void, onResizeWindow, (void),
{
	var canvas = document.getElementById('canvas');
	var cw = canvas.width;
	var ch = canvas.height;
	var aspect = cw / ch;
	var winw = window.innerWidth;
	var winh = window.innerHeight;
	var w = winw;
	var h = winw / aspect;
	if(h > winh) {
		h = winh;
		w = winh * aspect;
	}
	canvas.style.width = w + 'px';
	canvas.style.height = h + 'px';
	canvas.focus();
});

/* Run a frame. */
static void loop_iter(void *userData)
{
	static bool is_flip_pending = false;

	/* Do sound buffer filling. */
	fill_sound_buffer();

	/*
	 * Start a rendering.
	 *  - On Chrome, glFlush() seems to be called on "await" for a file I/O
	 *  - This causes flickering
	 *  - We avoid calling glClear() through opengl_start_rendering() here
	 *  - See also finish_frame_io()
	 */
	/* opengl_start_rendering(); */

	/* Do a frame event. */
	on_event_frame();

	/* Finish a rendering. */
	opengl_end_rendering();

	/* Reserve the next frame callback. */
	emscripten_async_call(loop_iter, NULL, FRAME_MILLI);
}

/* mousemove callback */
static EM_BOOL cb_mousemove(int eventType,
			    const EmscriptenMouseEvent *mouseEvent,
			    void *userData)
{
	double w, h, scale;
	int x, y;

	/*
	 * Get the "CSS" size of the rendering canvas
	 *  - It's not a visible size of the canvas
	 */
	emscripten_get_element_css_size("canvas", &w, &h);

	/* Scale a mouse position. */
	scale = w / (double)conf_window_width;
	x = (int)((double)mouseEvent->targetX / scale);
	y = (int)((double)mouseEvent->targetY / scale);

	/* Call the event handler. */
	on_event_mouse_move(x, y);

	return EM_TRUE;
}

/* mousedown callback */
static EM_BOOL cb_mousedown(int eventType,
			    const EmscriptenMouseEvent *mouseEvent,
			    void *userData)
{
	double w, h, scale;
	int x, y, button;

	/* Scale a mouse position. */
	emscripten_get_element_css_size("canvas", &w, &h);
	scale = w / (double)conf_window_width;
	x = (int)((double)mouseEvent->targetX / scale);
	y = (int)((double)mouseEvent->targetY / scale);

	if (mouseEvent->button == 0)
		button = MOUSE_LEFT;
	else
		button = MOUSE_RIGHT;

	/* Call the event handler. */
	on_event_mouse_press(button, x, y);

	return EM_TRUE;
}

/* mouseup callback */
static EM_BOOL cb_mouseup(int eventType,
			    const EmscriptenMouseEvent *mouseEvent,
			    void *userData)
{
	double w, h, scale;
	int x, y, button;

	/* Scale a mouse position. */
	emscripten_get_element_css_size("canvas", &w, &h);
	scale = w / (double)conf_window_width;
	x = (int)((double)mouseEvent->targetX / scale);
	y = (int)((double)mouseEvent->targetY / scale);

	if (mouseEvent->button == 0)
		button = MOUSE_LEFT;
	else
		button = MOUSE_RIGHT;

	/* Call the event handler. */
	on_event_mouse_release(button, x, y);

	return EM_TRUE;
}

/* wheel callback */
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

/* keydown callback */
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

/* keyup callback */
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

/* Get a keycode from a keysym. */
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

/* touchstart callback */
static EM_BOOL cb_touchstart(
	int eventType,
	const EmscriptenTouchEvent *touchEvent,
	void *userData)
{
	double w, h, scale;
	int x, y;

	touch_start_x = touchEvent->touches[0].targetX;
	touch_start_y = touchEvent->touches[0].targetY;
	touch_last_y = touchEvent->touches[0].targetY;

	/* Scale a mouse position. */
	emscripten_get_element_css_size("canvas", &w, &h);
	scale = w / (double)conf_window_width;
	x = (int)((double)touchEvent->touches[0].targetX / scale);
	y = (int)((double)touchEvent->touches[0].targetY / scale);

	/* Call the event handler. */
	on_event_mouse_press(MOUSE_LEFT, x, y);

	return EM_TRUE;
}

/* touchmoveのコールバック */
static EM_BOOL cb_touchmove(
	int eventType,
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

	/* Scale a mouse position. */
	emscripten_get_element_css_size("canvas", &w, &h);
	scale = w / (double)conf_window_width;
	x = (int)((double)touchEvent->touches[0].targetX / scale);
	y = (int)((double)touchEvent->touches[0].targetY / scale);

	/* Call the event handler. */
	on_event_mouse_move(x, y);

	return EM_TRUE;
}

/* touchendのコールバック */
static EM_BOOL cb_touchend(
	int eventType,
	const EmscriptenTouchEvent *touchEvent,
	void *userData)
{
	const int OFS = 10;
	double w, h, scale;
	int x, y;

	/* Scale a mouse position. */
	emscripten_get_element_css_size("canvas", &w, &h);
	scale = w / (double)conf_window_width;
	x = (int)((double)touchEvent->touches[0].targetX / scale);
	y = (int)((double)touchEvent->touches[0].targetY / scale);

	/* Call the event handler. */
	on_event_mouse_move(x, y);

	/* Consider a two-finger tap as a right-click. */
	if (touchEvent->numTouches == 2) {
		on_event_mouse_press(MOUSE_RIGHT, x, y);
		on_event_mouse_release(MOUSE_RIGHT, x, y);
		return EM_TRUE;
	}

	/* Consider a one-finger tap as a left-click. */
	if (abs(touchEvent->touches[0].targetX - touch_start_x) < OFS &&
	    abs(touchEvent->touches[0].targetY - touch_start_y) < OFS) {
		on_event_mouse_release(MOUSE_LEFT, x, y);
		return EM_TRUE;
	}
	
	return EM_TRUE;
}

/* touchcancelのコールバック */
static EM_BOOL cb_touchcancel(int eventType,
			      const EmscriptenTouchEvent *touchEvent,
			      void *userData)
{
	on_event_mouse_move(-1, -1);

	return EM_TRUE;
}

/*
 * Callback from JavaScript
 */

/* Resize the canvas. */
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

/* Callback when a project is loaded. */
EMSCRIPTEN_KEEPALIVE void onLoadProject(void)
{
	start_engine();
}

/* Callback when the continue button is pressed. */
EMSCRIPTEN_KEEPALIVE void onClickContinue(void)
{
	flag_continue_pushed = true;
}

/* Callback when the next button is pressed. */
EMSCRIPTEN_KEEPALIVE void onClickNext(void)
{
	flag_next_pushed = true;
}

/* Callback when the stop button is pressed. */
EMSCRIPTEN_KEEPALIVE void onClickStop(void)
{
	flag_stop_pushed = true;
}

/* Callback when a tab is shown. */
EMSCRIPTEN_KEEPALIVE void setVisible(void)
{
	resume_sound();
}

/* Callback when a tab is hidden. */
EMSCRIPTEN_KEEPALIVE void setHidden(void)
{
	pause_sound();
}

/* Callback when a selected range is changed. */
EMSCRIPTEN_KEEPALIVE void onEditorRangeChange(void)
{
	update_script_model_from_text();
}

/* Callback when Ctrl+Return is pressed. */
EMSCRIPTEN_KEEPALIVE void onEditorCtrlReturn(void)
{
	update_script_model_from_current_line_text();

	changed_exec_line = EM_ASM_INT({
		return editor.getSelection().getRange().start.row;
	});
	flag_exec_line_changed = true;
	flag_next_pushed = true;
}

/*
 * HAL API
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

void notify_image_update(struct image *img)
{
	fill_sound_buffer();
	opengl_notify_image_update(img);
	fill_sound_buffer();
}

void notify_image_free(struct image *img)
{
	fill_sound_buffer();
	opengl_notify_image_free(img);
	fill_sound_buffer();
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
	opengl_render_image_normal(dst_left, dst_top, dst_width, dst_height, src_image, src_left, src_top, src_width, src_height, alpha);
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
	opengl_render_image_add(dst_left, dst_top, dst_width, dst_height, src_image, src_left, src_top, src_width, src_height, alpha);
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
	opengl_render_image_dim(dst_left, dst_top, dst_width, dst_height, src_image, src_left, src_top, src_width, src_height, alpha);
}

void render_image_rule(
	struct image *src_img,
	struct image *rule_img,
	int threshold)
{
	opengl_render_image_rule(src_img, rule_img, threshold);
}

void render_image_melt(
	struct image *src_img,
	struct image *rule_img,
	int threshold)
{
	opengl_render_image_melt(src_img, rule_img, threshold);
}

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

bool make_sav_dir(void)
{
	return true;
}

char *make_valid_path(const char *dir, const char *fname)
{
	/* stub */
	return strdup("");
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

bool is_video_playing(void)
{
	int ended;

	ended = EM_ASM_INT({
		var v = document.getElementById("video");
		return v.ended;
	});

	return !ended;
}

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
	/* stub */
}

void leave_full_screen_mode(void)
{
	/* stub */
}

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

void finish_frame_io(void)
{
	opengl_start_rendering();
}

/*
 * Pro HAL API
 */

/*
 * Returns whether the "continue" botton is pressed.
 */
bool is_continue_pushed(void)
{
	bool ret;
	ret = flag_continue_pushed;
	flag_continue_pushed = false;
	return ret;
}

/*
 * Returns whether the "next" button is pressed.
 */
bool is_next_pushed(void)
{
	bool ret;
	ret = flag_next_pushed;
	flag_next_pushed = false;
	return ret;
}

/*
 * Returns whether the "stop" button is pressed.
 */
bool is_stop_pushed(void)
{
	bool ret;
	ret = flag_stop_pushed;
	flag_stop_pushed = false;
	return ret;
}

/*
 * Returns whether the "open" button is pressed.
 */
bool is_script_opened(void)
{
	bool ret;
	ret = flag_script_opened;
	flag_script_opened = false;
	return ret;
}

/*
 * Returns a script file name when the "open" button is pressed.
 */
const char *get_opened_script(void)
{
	return opened_script_name;
}

/*
 * Returns whether the "execution line number" is changed.
 */
bool is_exec_line_changed(void)
{
	bool ret;
	ret = flag_exec_line_changed;
	flag_exec_line_changed = false;
	return ret;
}

/*
 * Returns the "execution line number" if it is changed.
 */
int get_changed_exec_line(void)
{
	return changed_exec_line;
}

/*
 * Updates UI elements when the running state is changed.
 */
void on_change_running_state(bool running, bool request_stop)
{
	is_running = running;
	if (is_running) {
		EM_ASM({
			editor.session.clearBreakpoints();
			editor.session.setBreakpoint($0, 'currentExecLine');
			editor.scrollToLine($0, true, true);
		}, get_line_num());
	} else {
		EM_ASM({
			editor.session.clearBreakpoints();
			editor.session.setBreakpoint($0, 'nextExecLine');
			editor.scrollToLine($0, true, true);
		}, get_line_num());
	}

	if(request_stop) {
		/* Running but stop-requested. */
		EM_ASM({
			document.getElementById('runningStatus').innerHTML = '実行中(停止待ち)';
			document.getElementById('btnContinue').disabled = 'disabled';
			document.getElementById('btnNext').disabled = 'disabled';
			document.getElementById('btnStop').disabled = 'disabled';
			document.getElementById('btnOpen').disabled = 'disabled';
			document.getElementById('btnMove').disabled = 'disabled';
		});
	} else if(running) {
		/* Running. */
		EM_ASM({
			document.getElementById('runningStatus').innerHTML = '実行中';
			document.getElementById('btnContinue').disabled = 'disabled';
			document.getElementById('btnNext').disabled = 'disabled';
			document.getElementById('btnStop').disabled = "";
			document.getElementById('btnOpen').disabled = 'disabled';
			document.getElementById('btnMove').disabled = 'disabled';
		});
	} else {
		/* Stopped. */
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
	if (is_running) {
		EM_ASM({
			editor.session.clearBreakpoints();
			editor.session.setBreakpoint($0, 'currentExecLine');
			editor.scrollToLine($0, true, true);
		}, get_line_num());
	} else {
		EM_ASM({
			editor.session.clearBreakpoints();
			editor.session.setBreakpoint($0, 'nextExecLine');
			editor.scrollToLine($0, true, true);
		}, get_line_num());
	}
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

/* Updates the script model from the text of the editview. */
static void update_script_model_from_text(void)
{
	char line_buf[4096];
	int lines, i;

	/* Reset parse errors and will notify a first error. */
	dbg_reset_parse_error_count();

	/* Get the line count on the editor. */
	lines = EM_ASM_INT({ return editor.session.getLength(); });

	/* Update for each line. */
	for (i = 0; i < lines; i++) {
		/* Get the line text. */
		EM_ASM({
			stringToUTF8(editor.session.getLine($0), $1, $2);
		}, i, line_buf, sizeof(line_buf));

		/* Update the line on the script model. */
		if (i < get_line_count())
			update_script_line(i, line_buf);
		else
			insert_script_line(i, line_buf);
	}

	/* Process the removed tailing lines. */
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

	/* If there are parse errors: */
	if (dbg_get_parse_error_count() > 0) {
		/* Set the editview text in order to apply heading '!'. */
		update_text_from_script_model();
	}
}

/* Updates the script model from the current line text of the editview. */
static void update_script_model_from_current_line_text(void)
{
	char line_buf[4096];
	int line;

	/* Reset parse errors and will notify a first error. */
	dbg_reset_parse_error_count();

	/* Get the cursor line number on the editor. */
	line = EM_ASM_INT({
		return editor.getSelection().getRange().start.row;
	});

	/* Get the line text on the editor. */
	EM_ASM({
		stringToUTF8(editor.session.getLine($0), $1, $2);
	}, line, line_buf, sizeof(line_buf));

	/* Update a line. */
	update_script_line(line, line_buf);

	/* If there are parse errors: */
	if (dbg_get_parse_error_count() > 0) {
		/* Set the editview text in order to apply heading '!'. */
		update_text_from_script_model();
	}
}

/* Set the editview text from the script model. */
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

void speak_text(const char *text)
{
	UNUSED_PARAMETER(text);
}

void set_continuous_swipe_enabled(bool is_enabled)
{
	is_continuous_swipe_enabled = is_enabled;
}
