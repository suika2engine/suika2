/* -*- tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Suika 2
 * Copyright (C) 2001-2016, TABATA Keiichi. All rights reserved.
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
#include <unistd.h>	/* usleep() */

#include "suika.h"
#include "asound.h"

#ifdef SSE_VERSIONING
#include "cpuid.h"
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
struct image *back_image;

/*
 * フレーム開始時刻
 */
struct timeval tv_start;

/*
 * ログファイル
 */
FILE *log_fp;

/*
 * forward declaration
 */
static bool init(void);
static void cleanup(void);
static bool open_log_file(void);
static void close_log_file(void);
static bool open_display(void);
static void close_display(void);
static bool create_window(void);
static void destroy_window(void);
static bool create_icon_image(void);
static void destroy_icon_image(void);
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
int main()
{
	int ret;

	/* 互換レイヤの初期化処理を行う */
	if (init()) {
		/* アプリケーション本体の初期化処理を行う */
		if (on_event_init()) {
			/* ゲームループを実行する */
			run_game_loop();

			/* 成功 */
			ret = 0;
		}

		/* アプリケーション本体の終了処理を行う */
		on_event_cleanup();
	} else {
		/* エラーメッセージを表示する */
		if (log_fp != NULL)
			printf(LOG_FILE "を確認してください。\n");

		/* 失敗 */
		ret = 1;
	}

	/* 互換レイヤの終了処理を行う */
	cleanup();

	return ret;
}

/* 互換レイヤの初期化処理を行う */
static bool init(void)
{
#ifdef SSE_VERSIONING
	/* ベクトル命令の対応を確認する */
	x86_check_cpuid_flags();
#endif

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
		log_error("サウンドを初期化できません。\n");
		return false;
	}

	/* ディスプレイをオープンする */
	if (!open_display()) {
		log_error("ディスプレイをオープンできません。\n");
		return false;
	}

	/* ウィンドウを作成する */
	if (!create_window()) {
		log_error("ウィンドウを作成できません。\n");
		return false;
	}

	/* アイコンを作成する */
	if (!create_icon_image()) {
		log_error("アイコンを作成できません。\n");
		return false;
	}

	/* バックイメージを作成する */
	if (!create_back_image()) {
		log_error("バックイメージを作成できません。\n");
		return false;
	}

	return true;
}

/* 互換レイヤの終了処理を行う */
static void cleanup(void)
{
	/* ALSAの使用を終了する */
	cleanup_asound();

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
		printf("Can't open display.\n");
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

	/* ウィンドウを作成する */
	window = XCreateSimpleWindow(display, root, 0, 0,
				     (unsigned int)conf_window_width,
				     (unsigned int)conf_window_height,
				     1, black, white);
	if (window == BadAlloc || window == BadMatch || window == BadValue ||
	    window == BadWindow) {
		log_api_error("XCreateSimpleWindow");
		return false;
	}

	/* ウィンドウのタイトルを設定する */
	ret = XmbTextListToTextProperty(display, &conf_window_title, 1,
					XCompoundTextStyle, &tp);
	if (ret == XNoMemory || ret == XLocaleNotSupported) {
		log_api_error("XmbTextListToTextProperty");
		return false;
	}
	XSetWMName(display, window, &tp);

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

	/* イベントマスクを設定する */
	XSelectInput(display, window, KeyPressMask | ExposureMask |
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
	if (ret != XpmSuccess || ret < 0) {
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

	/* XDestroyImage()がピクセル列を解放してしまうので手動で確保する */
#ifndef SSE_VERSIONING
	pixels = malloc(WINDOW_WIDTH * WINDOW_HEIGHT * BPP / 8);
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

/*
 * X11のイベント処理
 */

/* イベントループ */
static void run_game_loop(void)
{
	int x, y, w, h;

	/* フレームの開始時刻を取得する */
	gettimeofday(&tv_start, NULL);

	while (1) {
		/* フレームイベントを呼び出す */
		x = y = w = h = 0;
		if (!on_event_frame(&x, &y, &w, &h))
			break;	/* スクリプトの終端に達した */

		/* 描画を行う */
		if (w != 0 && h != 0)
			sync_back_image(x, y, w, h);

		/* フレームの描画を行う */
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
	uint32_t lap, wait;

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
		if (lap > FRAME_MILLI) {
			tv_start = tv_end;
			break;
		}

		/* スリープする時間を求める */
		wait = (FRAME_MILLI - lap > SLEEP_MILLI) ? SLEEP_MILLI :
			FRAME_MILLI - lap;

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
		GC gc = XCreateGC(display, window, 0, 0);
		XPutImage(display, window, gc, ximage, 0, 0, 0, 0,
			  (unsigned int)conf_window_width,
			  (unsigned int)conf_window_height);
		XFreeGC(display, gc);
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
		fprintf(stderr, "%s", buf);
		fprintf(log_fp, "%s", buf);
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
		fprintf(stderr, "%s", buf);
		fprintf(log_fp, "%s", buf);
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
		fprintf(stderr, "%s", buf);
		fprintf(log_fp, "%s", buf);
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
