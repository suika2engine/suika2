/* -*- tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Suika 2
 * Copyright (C) 2001-2021, TABATA Keiichi. All rights reserved.
 */

/*
 * [Changes]
 *  2002-12-31 作成
 *  2005-04-11 更新 (style)
 *  2013-08-11 更新 (fb)
 *  2014-06-12 更新 (conskit)
 *  2016-05-27 更新 (suika2)
 */

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/xpm.h>
#include <X11/Xatom.h>
#include <X11/Xlocale.h>

#include <sys/types.h>
#include <sys/stat.h>	/* stat(), mkdir() */
#include <sys/time.h>	/* gettimeofday() */
#include <unistd.h>	/* usleep(), access() */

#include "suika.h"
#include "asound.h"
#include "gstplay.h"

#ifdef USE_X11_OPENGL
#include <GL/gl.h>
#include <GL/glx.h>
#include "glrender.h"
#endif

#ifdef SSE_VERSIONING
#include "x86.h"
#endif

#include "icon.xpm"	/* アイコン */

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

#ifdef USE_X11_OPENGL
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
#endif

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
static bool is_gst_skippable;

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
#ifdef USE_X11_OPENGL
static bool init_glx(void);
static void cleanup_glx(void);
#endif
static bool create_back_image(void);
static void destroy_back_image(void);
static void run_game_loop(void);
static bool wait_for_next_frame(void);
static void sync_back_image(int x, int y, int w, int h);
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

	/* ALSAの使用を開始する */
	if (!init_asound()) {
		log_error("Can't initialize sound.\n");
		return false;
	}

	/* ディスプレイをオープンする */
	if (!open_display()) {
		log_error("Can't open display.\n");
		return false;
	}

#ifdef USE_X11_OPENGL
	if (access("no-opengl.txt", F_OK) == 0) {
		log_info("Force 2D mode.");
	} else {
		/* OpenGLを初期化する */
		if (init_glx()) {
			is_opengl = true;
		} else {
			log_info("Failed to initialize OpenGL.");			log_info("Fall back to 2D mode.");
		}
	}
#endif

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

	if (!is_opengl) {
		/* バックイメージを作成する */
		if (!create_back_image()) {
			log_error("Can't create back image.\n");
			return false;
		}
	}

	gstplay_init(argc, argv);

	return true;
}

/* 互換レイヤの終了処理を行う */
static void cleanup(void)
{
	/* ALSAの使用を終了する */
	cleanup_asound();

	/* OpenGLの利用を終了する */
#ifdef USE_X11_OPENGL
	cleanup_glx();
#endif

	/* ウィンドウを破棄する */
	destroy_window();

	/* アイコンを破棄する */
	destroy_icon_image();

	/* バックイメージを破棄する */
	destroy_back_image();

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

/* 背景イメージを作成する */
static bool create_back_image(void)
{
	pixel_t *pixels;
	XVisualInfo vi;
	int screen;

	assert(!is_opengl);

	/* XDestroyImage()がピクセル列を解放してしまうので手動で確保する */
#ifndef SSE_VERSIONING
	pixels = malloc((size_t)(conf_window_width * conf_window_height *
				 BPP / 8));
	if (pixels == NULL)
		return false;
#else
	if (posix_memalign((void **)&pixels, SSE_ALIGN,
			   (size_t)(conf_window_width * conf_window_height *
				    BPP / 8)) != 0)
		return false;
#endif

	/* 初期状態でバックイメージを白く塗り潰す */
	if (conf_window_white) {
		memset(pixels, 0xff, (size_t)(conf_window_width *
					      conf_window_height * BPP / 8));
	}

	/* 背景イメージを作成する */
	back_image = create_image_with_pixels(conf_window_width,
					      conf_window_height,
					      pixels);
	if (back_image == NULL) {
		free(pixels);
		return false;
	}

	/* 32bppのVisualを取得する */
	screen = DefaultScreen(display);
	if (!XMatchVisualInfo(display, screen, BPP, TrueColor, &vi)) {
		log_error("Your X server is not capable of 32bpp mode.\n");
		destroy_image(back_image);
		free(pixels);
		return false;
	}

	/* 背景イメージを持つXImageオブジェクトを作成する */
	ximage = XCreateImage(display, vi.visual, DEPTH, ZPixmap, 0,
			      (char *)pixels,
			      (unsigned int)conf_window_width,
			      (unsigned int)conf_window_height,
			      BPP,
			      conf_window_width * BPP / 8);
	if (ximage == NULL) {
		destroy_image(back_image);
		free(pixels);
		return false;
	}

	return true;
}

/* 背景イメージを破棄する */
static void destroy_back_image(void)
{
	if (ximage != NULL) {
		XDestroyImage(ximage);
		ximage = NULL;
	}

	if (back_image != NULL) {
		destroy_image(back_image);
		back_image = NULL;
	}
}

#ifdef USE_X11_OPENGL
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
#endif

/*
 * X11のイベント処理
 */

/* イベントループ */
static void run_game_loop(void)
{
	int x, y, w, h;
	bool cont;

	/* フレームの開始時刻を取得する */
	gettimeofday(&tv_start, NULL);

	while (1) {
		if (is_gst_playing) {
			gstplay_loop_iteration();
			if (!gstplay_is_playing()) {
				gstplay_stop();
				is_gst_playing = false;
			}
		}

		if (!is_gst_playing) {
			if (is_opengl) {
#ifdef USE_X11_OPENGL
				/* レンダリングを開始する */
				opengl_start_rendering();
#endif
			} else {
				/* バックイメージをロックする */
				lock_image(back_image);
			}
		}

		/* フレームイベントを呼び出す */
		x = y = w = h = 0;
		cont = on_event_frame(&x, &y, &w, &h);

		if (!is_gst_playing) {
			if (is_opengl) {
#ifdef USE_X11_OPENGL
				/* レンダリングを終了する */
				opengl_end_rendering();

				/* フレームの描画を行う */
				glXSwapBuffers(display, glx_window);
#endif
			} else {
				/* バックイメージをアンロックする */
				unlock_image(back_image);

				/* フレームの描画を行う */
				if (w != 0 && h != 0)
					sync_back_image(x, y, w, h);
			}
		}

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

/* ウィンドウにイメージを転送する */
static void sync_back_image(int x, int y, int w, int h)
{
	GC gc = XCreateGC(display, window, 0, 0);
	XPutImage(display, window, gc, ximage, x, y, x, y, (unsigned int)w,
		  (unsigned int)h);
	XFreeGC(display, gc);
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
 * platform.hの実装
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
 * UTF-8のメッセージをネイティブの文字コードに変換する
 *  - 変換の必要がないので引数をそのまま返す
 */
const char *conv_utf8_to_native(const char *utf8_message)
{
	assert(utf8_message != NULL);
	return utf8_message;
}

/*
 * GPUを使うか調べる
 */
bool is_gpu_accelerated(void)
{
	return is_opengl;
}

/*
 * OpenGLが有効か調べる
 */
bool is_opengl_enabled(void)
{
	return is_opengl;
}

/*
 * テクスチャをロックする
 */
bool lock_texture(int width, int height, pixel_t *pixels,
		  pixel_t **locked_pixels, void **texture)
{
	if (is_opengl) {
#ifdef USE_X11_OPENGL
		if (!opengl_lock_texture(width, height, pixels, locked_pixels,
					 texture))
			return false;
#endif
	} else {
		UNUSED_PARAMETER(width);
		UNUSED_PARAMETER(height);
		UNUSED_PARAMETER(texture);
		assert(*locked_pixels == NULL);
		*locked_pixels = pixels;
	}
	return true;
}

/*
 * テクスチャをアンロックする
 */
void unlock_texture(int width, int height, pixel_t *pixels,
		    pixel_t **locked_pixels, void **texture)
{
	if (is_opengl) {
#ifdef USE_X11_OPENGL
		opengl_unlock_texture(width, height, pixels, locked_pixels,
				      texture);
#endif
	} else {
		UNUSED_PARAMETER(width);
		UNUSED_PARAMETER(height);
		UNUSED_PARAMETER(texture);
		UNUSED_PARAMETER(pixels);
		assert(*locked_pixels != NULL);
		*locked_pixels = NULL;
	}
}

/*
 * テクスチャを破棄する
 */
void destroy_texture(void *texture)
{
	if (is_opengl) {
#ifdef USE_X11_OPENGL
		opengl_destroy_texture(texture);
#endif
	} else {
		UNUSED_PARAMETER(texture);
	}
}

/*
 * イメージをレンダリングする
 */
void render_image(int dst_left, int dst_top, struct image * RESTRICT src_image,
                  int width, int height, int src_left, int src_top, int alpha,
                  int bt)
{
	if (is_opengl) {
#ifdef USE_X11_OPENGL
		opengl_render_image(dst_left, dst_top, src_image, width, height,
				    src_left, src_top, alpha, bt);
#endif
	} else {
		draw_image(back_image, dst_left, dst_top, src_image, width,
			   height, src_left, src_top, alpha, bt);
	}
}

/*
 * 画面にイメージをテンプレート指定でレンダリングする
 */
void render_image_rule(struct image * RESTRICT src_img,
					   struct image * RESTRICT rule_img,
					   int threshold)
{
	if (is_opengl) {
#ifdef USE_X11_OPENGL
		opengl_render_image_rule(src_img, rule_img, threshold);
#endif
	} else {
		draw_image_rule(back_image, src_img, rule_img, threshold);
	}
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

	is_gst_playing = true;
	is_gst_skippable = is_skippable;

	gstplay_play(path, window);

	free(path);

	return true;
}

/*
 * ビデオを停止する
 */
void stop_video(void)
{
	gstplay_stop();
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
	if (locale[0] == '\0' || locale[1] == '\0')
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
