/* -*- tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Suika 2
 * Copyright (C) 2001-2023, TABATA Keiichi. All rights reserved.
 */

/*
 * [Changes]
 *  2002-12-31 Created.
 *  2005-04-11 Updated. (style)
 *  2013-08-11 Updated. (fb)
 *  2014-06-12 Updated. (conskit)
 *  2016-05-27 Updated. (suika2)
 *  2022-07-22 Add OpenGL support.
 *  2023-07-17 Add capture/replay support.
 *  2023-12-11 Drop non-OpenGL code.
 */

/* Xlib */
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/xpm.h>
#include <X11/Xatom.h>
#include <X11/Xlocale.h>

/* POSIX */
#include <sys/types.h>
#include <sys/stat.h>	/* stat(), mkdir() */
#include <sys/time.h>	/* gettimeofday() */
#include <unistd.h>	/* usleep(), access() */

/* Suika2 Base */
#include "../suika.h"

/* Suika2 HAL implementation for sound output */
#if !defined(USE_REPLAY) && !defined(USE_CAPTURE)
#include "asound.h"
#include "gstplay.h"
#endif

/* Suika2 HAL implementation for graphics */
#include <GL/gl.h>
#include <GL/glx.h>
#include "../khronos/glrender.h"

/* Suika2 Capture */
#ifdef USE_CAPTURE
#include "capture.h"
#endif

/* Suika2 Replay */
#ifdef USE_REPLAY
#include "replay.h"
#endif

/* App Icon */
#include "icon.xpm"

/*
 * 色の情報
 */
#define DEPTH		(24)
#define BPP		(32)

/*
 * フレーム調整のための時間
 */
#define FRAME_MILLI	(33)	/* 1フレームの時間 */
#define SLEEP_MILLI	(5)	/* 1回にスリープする時間 */

/*
 * ログ1行のサイズ
 */
#define LOG_BUF_SIZE	(4096)

/*
 * OpenGLを利用するか
 */
static int is_opengl;

/*
 * GLXオブジェクト
 */
static GLXWindow glx_window = None;
static GLXContext glx_context = None;

/* OpenGL 3.2 API */
GLuint (APIENTRY *glCreateShader)(GLenum type);
void (APIENTRY *glShaderSource)(GLuint shader, GLsizei count,
				const GLchar *const*string,
				const GLint *length);
void (APIENTRY *glCompileShader)(GLuint shader);
void (APIENTRY *glGetShaderiv)(GLuint shader, GLenum pname, GLint *params);
void (APIENTRY *glGetShaderInfoLog)(GLuint shader, GLsizei bufSize,
				    GLsizei *length, GLchar *infoLog);
void (APIENTRY *glAttachShader)(GLuint program, GLuint shader);
void (APIENTRY *glLinkProgram)(GLuint program);
void (APIENTRY *glGetProgramiv)(GLuint program, GLenum pname, GLint *params);
void (APIENTRY *glGetProgramInfoLog)(GLuint program, GLsizei bufSize,
				     GLsizei *length, GLchar *infoLog);
GLuint (APIENTRY *glCreateProgram)(void);
void (APIENTRY *glUseProgram)(GLuint program);
void (APIENTRY *glGenVertexArrays)(GLsizei n, GLuint *arrays);
void (APIENTRY *glBindVertexArray)(GLuint array);
void (APIENTRY *glGenBuffers)(GLsizei n, GLuint *buffers);
void (APIENTRY *glBindBuffer)(GLenum target, GLuint buffer);
GLint (APIENTRY *glGetAttribLocation)(GLuint program, const GLchar *name);
void (APIENTRY *glVertexAttribPointer)(GLuint index, GLint size,
				       GLenum type, GLboolean normalized,
				       GLsizei stride, const void *pointer);
void (APIENTRY *glEnableVertexAttribArray)(GLuint index);
GLint (APIENTRY *glGetUniformLocation)(GLuint program, const GLchar *name);
void (APIENTRY *glUniform1i)(GLint location, GLint v0);
void (APIENTRY *glBufferData)(GLenum target, GLsizeiptr size, const void *data,
			      GLenum usage);
void (APIENTRY *glDeleteShader)(GLuint shader);
void (APIENTRY *glDeleteProgram)(GLuint program);
void (APIENTRY *glDeleteVertexArrays)(GLsizei n, const GLuint *arrays);
void (APIENTRY *glDeleteBuffers)(GLsizei n, const GLuint *buffers);
/*void (APIENTRY *glActiveTexture)(GLenum texture);*/

struct API
{
	void **func;
	const char *name;
};
static struct API api[] =
{
	{(void **)&glCreateShader, "glCreateShader"},
	{(void **)&glShaderSource, "glShaderSource"},
	{(void **)&glCompileShader, "glCompileShader"},
	{(void **)&glGetShaderiv, "glGetShaderiv"},
	{(void **)&glGetShaderInfoLog, "glGetShaderInfoLog"},
	{(void **)&glAttachShader, "glAttachShader"},
	{(void **)&glLinkProgram, "glLinkProgram"},
	{(void **)&glGetProgramiv, "glGetProgramiv"},
	{(void **)&glGetProgramInfoLog, "glGetProgramInfoLog"},
	{(void **)&glCreateProgram, "glCreateProgram"},
	{(void **)&glUseProgram, "glUseProgram"},
	{(void **)&glGenVertexArrays, "glGenVertexArrays"},
	{(void **)&glBindVertexArray, "glBindVertexArray"},
	{(void **)&glGenBuffers, "glGenBuffers"},
	{(void **)&glBindBuffer, "glBindBuffer"},
	{(void **)&glGetAttribLocation, "glGetAttribLocation"},
	{(void **)&glVertexAttribPointer, "glVertexAttribPointer"},
	{(void **)&glEnableVertexAttribArray, "glEnableVertexAttribArray"},
	{(void **)&glGetUniformLocation, "glGetUniformLocation"},
	{(void **)&glUniform1i, "glUniform1i"},
	{(void **)&glBufferData, "glBufferData"},
	{(void **)&glDeleteShader, "glDeleteShader"},
	{(void **)&glDeleteProgram, "glDeleteProgram"},
	{(void **)&glDeleteVertexArrays, "glDeleteVertexArrays"},
	{(void **)&glDeleteBuffers, "glDeleteBuffers"},
/*	{(void **)&glActiveTexture, "glActiveTexture"}, */
};

/*
 * X11オブジェクト
 */
static Display *display;
static Window window = BadAlloc;
static Pixmap icon = BadAlloc;
static Pixmap icon_mask = BadAlloc;
static XImage *ximage;
static Atom delete_message = BadAlloc;

/*
 * 背景イメージ
 */
static struct image *back_image;

/*
 * フレーム開始時刻
 */
static struct timeval tv_start;

/*
 * ログファイル
 */
static FILE *log_fp;

/*
 * ウィンドウタイトルのバッファ
 */
#define TITLE_BUF_SIZE	(1024)
static char title_buf[TITLE_BUF_SIZE];

/*
 * 動画を再生中かどうか
 */
static bool is_gst_playing;

/*
 * 動画のスキップ可能かどうか
 */
#if !defined(USE_REPLAY) && !defined(USE_CAPTURE)
static bool is_gst_skippable;
#endif

/*
 * forward declaration
 */
static bool init(int argc, char *argv[]);
static void cleanup(void);
static bool open_log_file(void);
static void close_log_file(void);
static bool open_display(void);
static void close_display(void);
static bool create_window(void);
static void destroy_window(void);
static bool create_icon_image(void);
static void destroy_icon_image(void);
static bool init_glx(void);
static void cleanup_glx(void);
static void run_game_loop(void);
static bool run_frame(void);
static bool wait_for_next_frame(void);
static bool next_event(void);
static void event_key_press(XEvent *event);
static void event_key_release(XEvent *event);
static int get_key_code(XEvent *event);
static void event_button_press(XEvent *event);
static void event_button_release(XEvent *event);
static void event_motion_notify(XEvent *event);
static void event_expose(XEvent *event);

/*
 * メイン
 */
int main(int argc, char *argv[])
{
	int ret;

	setlocale(LC_ALL, "");

	/* 互換レイヤの初期化処理を行う */
	if (init(argc, argv)) {
		/* アプリケーション本体の初期化処理を行う */
		if (on_event_init()) {
			/* ゲームループを実行する */
			run_game_loop();

			/* 成功 */
			ret = 0;
		} else {
			/* 失敗 */
			ret = 1;
		}

		/* アプリケーション本体の終了処理を行う */
		on_event_cleanup();
	} else {
		/* エラーメッセージを表示する */
		if (log_fp != NULL)
			printf("Check " LOG_FILE "\n");

		/* 失敗 */
		ret = 1;
	}

	/* 互換レイヤの終了処理を行う */
	cleanup();

	return ret;
}

/* 互換レイヤの初期化処理を行う */
static bool init(int argc, char *argv[])
{
#ifdef SSE_VERSIONING
	/* ベクトル命令の対応を確認する */
	x86_check_cpuid_flags();
#endif

	/* ロケールを初期化する */
	init_locale_code();

	/* ログファイルを開く */
	if (!open_log_file())
		return false;

	/* ファイル読み書きの初期化処理を行う */
	if (!init_file())
		return false;

	/* コンフィグの初期化処理を行う */
	if (!init_conf())
		return false;

#if !defined(USE_REPLAY) && !defined(USE_CAPTURE)
	/* ALSAの使用を開始する */
	if (!init_asound())
		log_warn("Can't initialize sound.\n");
#endif

	/* ディスプレイをオープンする */
	if (!open_display()) {
		log_error("Can't open display.\n");
		return false;
	}

	/* OpenGLを初期化する */
	if (!init_glx()) {
		log_error("Failed to initialize OpenGL.");
		return false;
	}

	/* ウィンドウを作成する */
	if (!create_window()) {
		log_error("Can't open window.\n");
		return false;
	}

	/* アイコンを作成する */
	if (!create_icon_image()) {
		log_error("Can't create icon.\n");
		return false;
	}


#if !defined(USE_REPLAY) && !defined(USE_CAPTURE)
	gstplay_init(argc, argv);
#endif

#ifdef USE_CAPTURE
	UNUSED_PARAMETER(argc);
	UNUSED_PARAMETER(argv);
	if (!init_capture())
		return false;
#endif

#ifdef USE_REPLAY
	if (!init_replay(argc, argv))
		return false;
#endif

	return true;
}

/* 互換レイヤの終了処理を行う */
static void cleanup(void)
{
#if !defined(USE_REPLAY) && !defined(USE_CAPTURE)
	/* ALSAの使用を終了する */
	cleanup_asound();
#endif

	/* OpenGLの利用を終了する */
	cleanup_glx();

	/* ウィンドウを破棄する */
	destroy_window();

	/* アイコンを破棄する */
	destroy_icon_image();

	/* ディスプレイをクローズする */
	close_display();

	/* コンフィグの終了処理を行う */
	cleanup_conf();

	/* ファイル読み書きの終了処理を行う */
	cleanup_file();

	/* ログファイルを閉じる */
	close_log_file();
}

/*
 * ログ
 */

/* ログをオープンする */
static bool open_log_file(void)
{
	if (log_fp == NULL) {
		log_fp = fopen(LOG_FILE, "w");
		if (log_fp == NULL) {
			printf("Can't open log file.\n");
			return false;
		}
	}
	return true;
}

/* ログをクローズする */
static void close_log_file(void)
{
	if (log_fp != NULL)
		fclose(log_fp);
}

/*
 * X11
 */

/* ディスプレイをオープンする */
static bool open_display(void)
{
	setlocale(LC_ALL, "");

	display = XOpenDisplay(NULL);
	if (display == NULL) {
		log_api_error("XOpenDisplay");
		return false;
	}
	return true;
}

/* ディスプレイをクローズする */
static void close_display(void)
{
	if (display != NULL)
		XCloseDisplay(display);
}

/* ウィンドウを作成する */
static bool create_window(void)
{
	Window root;
	XSizeHints *sh;
	XTextProperty tp;
	unsigned long black, white;
	int screen, ret;

	/* ディスプレイの情報を取得する */
	screen = DefaultScreen(display);
	root  = RootWindow(display, screen);
	black = BlackPixel(display, screen);
	white = WhitePixel(display, screen);

	/* ウィンドウを作成する (OpenGLを使用する場合ウィンドウは作成済み) */
	if (!is_opengl) {
		window = XCreateSimpleWindow(display, root, 0, 0,
					     (unsigned int)conf_window_width,
					     (unsigned int)conf_window_height,
					     1, black, white);
		if (window == BadAlloc || window == BadMatch ||
		    window == BadValue || window == BadWindow) {
			log_api_error("XCreateSimpleWindow");
			return false;
		}
	}

	/* ウィンドウのタイトルを設定する */
	ret = XmbTextListToTextProperty(display, &conf_window_title, 1,
					XCompoundTextStyle, &tp);
	if (ret == XNoMemory || ret == XLocaleNotSupported) {
		log_api_error("XmbTextListToTextProperty");
		return false;
	}
	XSetWMName(display, window, &tp);
	XFree(tp.value);

	/* ウィンドウを表示する */
	ret = XMapWindow(display, window);
	if (ret == BadWindow) {
		log_api_error("XMapWindow");
		return false;
	}

	/* ウィンドウのサイズを固定する */
	sh = XAllocSizeHints();
	sh->flags = PMinSize | PMaxSize;
	sh->min_width = conf_window_width;
	sh->min_height = conf_window_height;
	sh->max_width = conf_window_width;
	sh->max_height = conf_window_height;
	XSetWMSizeHints(display, window, sh, XA_WM_NORMAL_HINTS);
	XFree(sh);

	/* イベントマスクを設定する */
	ret = XSelectInput(display, window, KeyPressMask | ExposureMask |
		     ButtonPressMask | ButtonReleaseMask | KeyReleaseMask |
		     PointerMotionMask);
	if (ret == BadWindow) {
		log_api_error("XSelectInput");
		return false;
	}

	/* 可能なら閉じるボタンのイベントを受け取る */
	delete_message = XInternAtom(display, "WM_DELETE_WINDOW", True);
	if (delete_message != None && delete_message != BadAlloc &&
	    delete_message != BadValue)
		XSetWMProtocols(display, window, &delete_message, 1);

	return true;
}

/* ウィンドウを破棄する */
static void destroy_window(void)
{
	if (display != NULL)
		if (window != BadAlloc)
			XDestroyWindow(display, window);
}

/* アイコンを作成する */
static bool create_icon_image(void)
{
	XWMHints *win_hints;
	XpmAttributes attr;
	Colormap cm;
	int ret;

	/* カラーマップを作成する */
	cm = XCreateColormap(display, window,
			     DefaultVisual(display, DefaultScreen(display)),
			     AllocNone);
	if (cm == BadAlloc || cm == BadMatch || cm == BadValue ||
	    cm == BadWindow) {
		log_api_error("XCreateColorMap");
		return false;
	}

	/* Pixmapを作成する */
	attr.valuemask = XpmColormap;
	attr.colormap = cm;
	ret = XpmCreatePixmapFromData(display, window, icon_xpm, &icon,
				      &icon_mask, &attr);
	if (ret != XpmSuccess) {
		log_api_error("XpmCreatePixmapFromData");
		XFreeColormap(display, cm);
		return false;
	}

	/* WMHintsを確保する */
	win_hints = XAllocWMHints();
	if (!win_hints) {
		XFreeColormap(display, cm);
		return false;
	}

	/* アイコンを設定する */
	win_hints->flags = IconPixmapHint | IconMaskHint;
	win_hints->icon_pixmap = icon;
	win_hints->icon_mask = icon_mask;
	XSetWMHints(display, window, win_hints);

	/* オブジェクトを解放する */
	XFree(win_hints);
	XFreeColormap(display,cm);
	return true;
}

/* アイコンを破棄する */
static void destroy_icon_image(void)
{
	if (display != NULL) {
		if (icon != BadAlloc)
			XFreePixmap(display, icon);
		if (icon_mask != BadAlloc)
			XFreePixmap(display, icon_mask);
	}
}

/*
 * OpenGL
 */

/* OpenGLを初期化する */
static bool init_glx(void)
{
	int pix_attr[] = {
		GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
		GLX_RENDER_TYPE, GLX_RGBA_BIT,
		GLX_DOUBLEBUFFER, True,
		GLX_RED_SIZE, 1,
		GLX_GREEN_SIZE, 1,
		GLX_BLUE_SIZE, 1,
		None
	};
	int ctx_attr[]= {
		GLX_CONTEXT_MAJOR_VERSION_ARB, 2,
		GLX_CONTEXT_MINOR_VERSION_ARB, 0,
		GLX_CONTEXT_FLAGS_ARB, 0,
		GLX_CONTEXT_PROFILE_MASK_ARB, GLX_CONTEXT_CORE_PROFILE_BIT_ARB,
		None
	};
	GLXContext (*glXCreateContextAttribsARB)(Display *dpy,
						 GLXFBConfig config,
						 GLXContext share_context,
						 Bool direct,
						 const int *attrib_list);
	GLXFBConfig *config;
	XVisualInfo *vi;
	XSetWindowAttributes swa;
	XEvent event;
	int i, n;

	/* フレームバッファの形式を選択する */
	config = glXChooseFBConfig(display, DefaultScreen(display), pix_attr,
				   &n);
	if (config == NULL)
		return false;
	vi = glXGetVisualFromFBConfig(display, config[0]);

	/* ウィンドウを作成する */
	swa.border_pixel = 0;
	swa.event_mask = StructureNotifyMask;
	swa.colormap = XCreateColormap(display, RootWindow(display, vi->screen),
				       vi->visual, AllocNone);
	window = XCreateWindow(display, RootWindow(display, vi->screen), 0, 0,
			       (unsigned int)conf_window_width,
			       (unsigned int)conf_window_height,
			       0, vi->depth, InputOutput, vi->visual,
			       CWBorderPixel | CWColormap | CWEventMask, &swa);
	XFree(vi);

	/* GLXコンテキストを作成する */
	glXCreateContextAttribsARB = (void *)glXGetProcAddress(
			(const unsigned char *)"glXCreateContextAttribsARB");
	if (glXCreateContextAttribsARB == NULL) {
		XDestroyWindow(display, window);
		return false;
	}
	glx_context = glXCreateContextAttribsARB(display, config[0], 0, True,
						 ctx_attr);
	if (glx_context == NULL) {
		XDestroyWindow(display, window);
		return false;
	}

	/* GLXウィンドウを作成する */
	glx_window = glXCreateWindow(display, config[0], window, NULL);
	XFree(config);

	/* ウィンドウをスクリーンにマップして表示されるのを待つ */
	XMapWindow(display, window);
	XNextEvent(display, &event);

	/* GLXコンテキストをウィンドウにバインドする */
	glXMakeContextCurrent(display, glx_window, glx_window, glx_context);

	/* APIのポインタを取得する */
	for (i = 0; i < (int)(sizeof(api)/sizeof(struct API)); i++) {
		*api[i].func = (void *)glXGetProcAddress(
			(const unsigned char *)api[i].name);
		if(*api[i].func == NULL) {
			log_info("Failed to get %s", api[i].name);
			glXMakeContextCurrent(display, None, None, None);
			glXDestroyContext(display, glx_context);
			glXDestroyWindow(display, glx_window);
			glx_context = None;
			glx_window = None;
			return false;
		}
	}

	/* OpenGLの初期化を行う */
	if (!init_opengl()) {
		glXMakeContextCurrent(display, None, None, None);
		glXDestroyContext(display, glx_context);
		glXDestroyWindow(display, glx_window);
		glx_context = None;
		glx_window = None;
		return false;
	}

	return true;
}

/* OpenGLの終了処理を行う */
static void cleanup_glx(void)
{
	glXMakeContextCurrent(display, None, None, None);
	if (glx_context != None) {
		glXDestroyContext(display, glx_context);
		glx_context = None;
	}
	if (glx_window != None) {
		glXDestroyWindow(display, glx_window);
		glx_window = None;
	}

	cleanup_opengl();
}

/*
 * X11のイベント処理
 */

/* イベントループ */
static void run_game_loop(void)
{
	bool cont;

	/* フレームの開始時刻を取得する */
	gettimeofday(&tv_start, NULL);

	cont = true;
	while (1) {
#if !defined(USE_REPLAY) && !defined(USE_CAPTURE)
		if (is_gst_playing) {
			gstplay_loop_iteration();
			if (!gstplay_is_playing()) {
				gstplay_stop();
				is_gst_playing = false;
			}
		}
#endif

#if defined(USE_CAPTURE) || defined(USE_REPLAY)
		/* 入力のキャプチャ/リプレイを行う */
		if (!capture_input())
			break;
#endif

		/* Run a frame. */
		if (!run_frame())
			cont = false;

#if defined(USE_CAPTURE) || defined(USE_REPLAY)
		/* 出力のキャプチャを行う */
		if (!capture_output())
			break;
#endif

		/* スクリプトの終端に達した */
		if (!cont)
			break;

		/* 次のフレームを待つ */
		if (!wait_for_next_frame())
			break;	/* 閉じるボタンが押された */

		/* フレームの開始時刻を取得する */
		gettimeofday(&tv_start, NULL);
	}
}

/* Run a frame. */
static bool run_frame(void)
{
	bool cont;

	/* レンダリングを開始する */
	if (!is_gst_playing) {
		opengl_start_rendering();
	}

	/* フレームイベントを呼び出す */
	cont = on_event_frame();

	/* レンダリングを終了する */
	if (!is_gst_playing) {
		opengl_end_rendering();
		glXSwapBuffers(display, glx_window);
	}

	return cont;
}

/* フレームの描画を行う */
static bool wait_for_next_frame(void)
{
	struct timeval tv_end;
	uint32_t lap, wait, span;

	span = is_opengl ? FRAME_MILLI / 2 : FRAME_MILLI;

	/* 次のフレームの開始時刻になるまでイベント処理とスリープを行う */
	do {
		/* イベントがある場合は処理する */
		while (XEventsQueued(display, QueuedAfterFlush) > 0)
			if (!next_event())
				return false;

#ifdef USE_REPLAY
		UNUSED_PARAMETER(tv_end);
		UNUSED_PARAMETER(lap);
		UNUSED_PARAMETER(span);
		usleep(1);
		break;
#else
		/* 経過時刻を取得する */
		gettimeofday(&tv_end, NULL);
		lap = (uint32_t)((tv_end.tv_sec - tv_start.tv_sec) * 1000 +
				 (tv_end.tv_usec - tv_start.tv_usec) / 1000);

		/* 次のフレームの開始時刻になった場合はスリープを終了する */
		if (lap > span) {
			tv_start = tv_end;
			break;
		}

		/* スリープする時間を求める */
		wait = (span - lap > SLEEP_MILLI) ? SLEEP_MILLI : span - lap;

		/* スリープする */
		usleep(wait * 1000);
#endif
	} while(wait > 0);

	return true;
}

/* イベントを1つ処理する */
static bool next_event(void)
{
	XEvent event;

	XNextEvent(display, &event);
	switch (event.type) {
	case KeyPress:
		event_key_press(&event);
		break;
	case KeyRelease:
		event_key_release(&event);
		break;
	case ButtonPress:
		event_button_press(&event);
		break;
	case ButtonRelease:
		event_button_release(&event);
		break;
	case MotionNotify:
		event_motion_notify(&event);
		break;
	case Expose:
		event_expose(&event);
		break;
	case MappingNotify:
		XRefreshKeyboardMapping(&event.xmapping);
		break;
	case ClientMessage:
		/* 閉じるボタンが押された */
		if ((Atom)event.xclient.data.l[0] == delete_message)
			return false;
		break;
	}
	return true;
}

/* KeyPressイベントを処理する */
static void event_key_press(XEvent *event)
{
	int key;

	/* キーコードを取得する */
	key = get_key_code(event);
	if (key == -1)
		return;

	/* イベントハンドラを呼び出す */
	on_event_key_press(key);
}

/* KeyReleaseイベントを処理する */
static void event_key_release(XEvent *event)
{
	int key;

	/* オートリピートのイベントを無視する */
	if (XEventsQueued(display, QueuedAfterReading) > 0) {
		XEvent next;
		XPeekEvent(display, &next);
		if (next.type == KeyPress &&
		    next.xkey.keycode == event->xkey.keycode &&
		    next.xkey.time == event->xkey.time) {
			XNextEvent(display, &next);
			return;
		}
	}

	/* キーコードを取得する */
	key = get_key_code(event);
	if (key == -1)
		return;

	/* イベントハンドラを呼び出す */
	on_event_key_release(key);
}

/* KeySymからenum key_codeに変換する */
static int get_key_code(XEvent *event)
{
	char text[255];
	KeySym keysym;

	/* キーシンボルを取得する */
	XLookupString(&event->xkey, text, sizeof(text), &keysym, 0);

	/* キーコードに変換する */
	switch (keysym) {
	case XK_Return:
	case XK_KP_Enter:
		return KEY_RETURN;
	case XK_space:
		return KEY_SPACE;
		break;
	case XK_Control_L:
	case XK_Control_R:
		return KEY_CONTROL;
	case XK_Down:
		return KEY_DOWN;
	case XK_Up:
		return KEY_UP;
	case XK_Left:
		return KEY_LEFT;
	case XK_Right:
		return KEY_RIGHT;
	}
	return -1;
}

/* ButtonPressイベントを処理する */
static void event_button_press(XEvent *event)
{
	/* ボタンの種類ごとにディスパッチする */
	switch (event->xbutton.button) {
	case Button1:
		on_event_mouse_press(MOUSE_LEFT, event->xbutton.x,
				     event->xbutton.y);
		break;
	case Button3:
		on_event_mouse_press(MOUSE_RIGHT, event->xbutton.x,
				     event->xbutton.y);
		break;
	case Button4:
		on_event_key_press(KEY_UP);
		on_event_key_release(KEY_UP);
		break;
	case Button5:
		on_event_key_press(KEY_DOWN);
		on_event_key_release(KEY_DOWN);
		break;
	default:
		break;
	}
}

/* ButtonPressイベントを処理する */
static void event_button_release(XEvent *event)
{
	/* ボタンの種類ごとにディスパッチする */
	switch (event->xbutton.button) {
	case Button1:
		on_event_mouse_release(MOUSE_LEFT, event->xbutton.x,
				       event->xbutton.y);
		break;
	case Button3:
		on_event_mouse_release(MOUSE_RIGHT, event->xbutton.x,
				       event->xbutton.y);
		break;
	}
}

/* MotionNotifyイベントを処理する */
static void event_motion_notify(XEvent *event)
{
	/* イベントをディスパッチする */
	on_event_mouse_move(event->xmotion.x, event->xmotion.y);
}

/* Exposeイベントを処理する */
static void event_expose(XEvent *event)
{
	if (event->xexpose.count == 0) {
		if (!is_opengl) {
			GC gc = XCreateGC(display, window, 0, 0);
			XPutImage(display, window, gc, ximage, 0, 0, 0, 0,
				  (unsigned int)conf_window_width,
				  (unsigned int)conf_window_height);
			XFreeGC(display, gc);
		}
	}
}

/*
 * HAL
 */

/*
 * INFOログを出力する
 */
bool log_info(const char *s, ...)
{
	char buf[LOG_BUF_SIZE];
	va_list ap;

	va_start(ap, s);
	if (log_fp != NULL) {
		vsnprintf(buf, sizeof(buf), s, ap);
		fprintf(stderr, "%s\n", buf);
		fprintf(log_fp, "%s\n", buf);
		fflush(log_fp);
		if (ferror(log_fp))
			return false;
	}
	va_end(ap);
	return true;
}

/*
 * WARNログを出力する
 */
bool log_warn(const char *s, ...)
{
	char buf[LOG_BUF_SIZE];
	va_list ap;

	va_start(ap, s);
	if (log_fp != NULL) {
		vsnprintf(buf, sizeof(buf), s, ap);
		fprintf(stderr, "%s\n", buf);
		fprintf(log_fp, "%s\n", buf);
		fflush(log_fp);
		if (ferror(log_fp))
			return false;
	}
	va_end(ap);
	return true;
}

/*
 * ERRORログを出力する
 */
bool log_error(const char *s, ...)
{
	va_list ap;
	char buf[LOG_BUF_SIZE];

	va_start(ap, s);
	if (log_fp != NULL) {
		vsnprintf(buf, sizeof(buf), s, ap);
		fprintf(stderr, "%s\n", buf);
		fprintf(log_fp, "%s\n", buf);
		fflush(log_fp);
		if (ferror(log_fp))
			return false;
	}
	va_end(ap);
	return true;
}

/*
 * イメージの更新を通知する
 */
void notify_image_update(struct image *img)
{
	opengl_notify_image_update(img);
}

/*
 * イメージの破棄を通知する
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
				src_width,
				src_height,
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
 * 画面にイメージをルール付きでレンダリングする
 */
void render_image_rule(struct image *src_img,
		       struct image *rule_img,
		       int threshold)
{
	opengl_render_image_rule(src_img, rule_img, threshold);
}

/*
 * 画面にイメージをルール付き(メルト)でレンダリングする
 */
void render_image_melt(struct image *src_img,
		       struct image *rule_img,
		       int progress)
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
void reset_lap_timer(uint64_t *t)
{
#ifndef USE_REPLAY
	struct timeval tv;

	gettimeofday(&tv, NULL);

	*t = (uint64_t)(tv.tv_sec * 1000 + tv.tv_usec / 1000);
#else
	extern uint64_t sim_time;
	*t = sim_time;
#endif
}

/*
 * タイマのラップをミリ秒単位で取得する
 */
uint64_t get_lap_timer_millisec(uint64_t *t)
{
#ifndef USE_REPLAY
	struct timeval tv;
	uint64_t end;
	
	gettimeofday(&tv, NULL);

	end = (uint64_t)(tv.tv_sec * 1000 + tv.tv_usec / 1000);

	return (uint64_t)(end - *t);
#else
	extern uint64_t sim_time;
	return (uint64_t)(sim_time - *t);
#endif
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
#if !defined(USE_REPLAY) && !defined(USE_CAPTURE)
	char *path;

	path = make_valid_path(MOV_DIR, fname);

	is_gst_playing = true;
	is_gst_skippable = is_skippable;

	gstplay_play(path, window);

	free(path);
#else
	UNUSED_PARAMETER(fname);
	UNUSED_PARAMETER(is_skippable);
#endif

	return true;
}

/*
 * ビデオを停止する
 */
void stop_video(void)
{
#if !defined(USE_REPLAY) && !defined(USE_CAPTURE)
	gstplay_stop();
#endif
	is_gst_playing = false;
}

/*
 * ビデオが再生中か調べる
 */
bool is_video_playing(void)
{
	return is_gst_playing;
}

/*
 * ウィンドウタイトルを更新する
 */
void update_window_title(void)
{
	XTextProperty tp;
	char *buf;
	const char *sep;
	int ret;

	/* セパレータを取得する */
	sep = conf_window_title_separator;
	if (sep == NULL)
		sep = " ";

	/* タイトルを作成する */
	strncpy(title_buf, conf_window_title, TITLE_BUF_SIZE - 1);
	strncat(title_buf, sep, TITLE_BUF_SIZE - 1);
	strncat(title_buf, get_chapter_name(), TITLE_BUF_SIZE - 1);
	title_buf[TITLE_BUF_SIZE - 1] = '\0';

	/* ウィンドウのタイトルを設定する */
	buf = title_buf;
	ret = XmbTextListToTextProperty(display, &buf, 1,
					XCompoundTextStyle, &tp);
	if (ret == XNoMemory || ret == XLocaleNotSupported) {
		log_api_error("XmbTextListToTextProperty");
		return;
	}
	XSetWMName(display, window, &tp);
	XFree(tp.value);
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

void speak_text(const char *msg)
{
	UNUSED_PARAMETER(msg);
}

#if defined(USE_CAPTURE) || defined(USE_REPLAY)
/*
 * ミリ秒の時刻を取得する
 */
uint64_t get_tick_count64(void)
{
	struct timeval tv;

	gettimeofday(&tv, NULL);

	return (uint64_t)tv.tv_sec * 1000LL + (uint64_t)tv.tv_usec / 1000LL;
}

/*
 * 出力データのディレクトリを作り直す
 */
bool reconstruct_dir(const char *dir)
{
	remove(dir);
	mkdir(dir, 0700); 
	return true;
}
#endif
