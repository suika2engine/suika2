/* -*- coding: utf-8; indent-tabs-mode: t; tab-width: 4; c-basic-offset: 4; -*- */

/*
 * Suika 2
 * Copyright (C) 2001-2023, TABATA Keiichi. All rights reserved.
 */

/*
 * [Changes]
 *  2014-05-24 作成 (conskit)
 *  2016-05-29 作成 (suika)
 *  2017-11-07 フルスクリーンで解像度変更するように修正
 *  2022-06-08 デバッガ対応
 *  2023-07-17 キャプチャ対応
 */

#ifdef _MSC_VER
#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif

#include <windows.h>
#include <shlobj.h>
#include <io.h>

#include "suika.h"
#include "dsound.h"
#include "dsvideo.h"
#include "uimsg.h"
#include "resource.h"

#ifdef USE_DEBUGGER
#include <commctrl.h>
#include "windebug.h"
#endif

#include "d3drender.h"

#include <GL/gl.h>
#include "glhelper.h"
#include "glrender.h"

#ifdef SSE_VERSIONING
#include "x86.h"
#endif

/* ウィンドウタイトルのバッファサイズ */
#define TITLE_BUF_SIZE	(1024)

/* ログ1行のサイズ */
#define LOG_BUF_SIZE	(4096)

/* フレームレート */
#define FPS				(30)

/* 1フレームの時間 */
#define FRAME_MILLI		(33)

/* 1回にスリープする時間 */
#define SLEEP_MILLI		(5)

/* UTF-8/UTF-16の変換バッファサイズ */
#define CONV_MESSAGE_SIZE	(65536)

/* ウィンドウクラス名 */
static const wchar_t wszWindowClass[] = L"suika";

/* ウィンドウタイトル(UTF-16) */
static wchar_t wszTitle[TITLE_BUF_SIZE];

/* メッセージ変換バッファ */
static wchar_t wszMessage[CONV_MESSAGE_SIZE];
static char szMessage[CONV_MESSAGE_SIZE];

/* Direct3Dを利用するか */
static BOOL bD3D;

/* OpenGLを利用するか */
static BOOL bOpenGL;

/* Windowsオブジェクト */
static HWND hWndMain;
static HDC hWndDC;
static HDC hBitmapDC;
static HBITMAP hBitmap;
static HGLRC hGLRC;
static HMENU hMenu;

/* アクセラレータ */
static HACCEL hAccel;

/* イメージオブジェクト */
static struct image *BackImage;

/* WaitForNextFrame()の時間管理用 */
static DWORD dwStartTime;

/* ログファイル */
static FILE *pLogFile;

/* フルスクリーンモードであるか */
static BOOL bFullScreen;

/* ディスプレイセッティングを変更中か */
static BOOL bDisplaySettingsChanged;

/* ウィンドウモードでの座標 */
static RECT rectWindow;

/* ストップウォッチの停止した時間 */
DWORD dwStopWatchOffset;

/* フルスクリーンモード時の描画オフセット */
static int nOffsetX;
static int nOffsetY;

/* DirectShowでビデオを再生中か */
static BOOL bDShowMode;

/* DirectShow再生中にクリックでスキップするか */
static BOOL bDShowSkippable;

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
void (APIENTRY *glActiveTexture)(GLenum texture);

struct GLExtAPITable
{
	void **func;
	const char *name;
} APITable[] =
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
	{(void **)&glActiveTexture, "glActiveTexture"},
};

/* 前方参照 */
static BOOL InitApp(HINSTANCE hInstance, int nCmdShow);
static BOOL InitRenderingEngine(void);
static void CleanupApp(void);
static BOOL InitWindow(HINSTANCE hInstance, int nCmdShow);
#ifndef USE_DEBUGGER
static VOID InitGameMenu(void);
#endif
static BOOL InitOpenGL(void);
static void GameLoop(void);
static BOOL SyncEvents(void);
static BOOL WaitForNextFrame(void);
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam,
						 LPARAM lParam);
static int ConvertKeyCode(int nVK);
static void ToggleFullScreen(void);
static void ChangeDisplayMode(void);
static void ResetDisplayMode(void);
static void OnPaint(void);
static void OnCommand(UINT nID);
static BOOL CreateBackImage(void);
static void SyncBackImage(int x, int y, int w, int h);
static BOOL OpenLogFile(void);
const wchar_t *conv_utf8_to_utf16(const char *utf8_message);
const char *conv_utf16_to_utf8(const wchar_t *utf16_message);

/*
 * WinMain
 */
int WINAPI wWinMain(
	HINSTANCE hInstance,
	UNUSED(HINSTANCE hPrevInstance),
	UNUSED(LPWSTR lpszCmd),
	int nCmdShow)
{
	int result = 1;

	/* Sleep()の分解能を設定する */
	timeBeginPeriod(1);

#ifdef USE_DEBUGGER
	InitCommonControls();
#endif

	/* 基盤レイヤの初期化処理を行う */
	if(InitApp(hInstance, nCmdShow))
	{
		/* アプリケーション本体の初期化を行う */
		if(on_event_init())
		{
			/* イベントループを実行する */
			GameLoop();

			/* アプリケーション本体の終了処理を行う */
			on_event_cleanup();

			result = 0;
		}
	}

	/* 互換レイヤの終了処理を行う */
	CleanupApp();

	/* Sleep()の分解能を元に戻す */
	timeEndPeriod(1);

	return result;
}

/* 基盤レイヤの初期化処理を行う */
static BOOL InitApp(HINSTANCE hInstance, int nCmdShow)
{
	HRESULT hRes;

#ifdef SSE_VERSIONING
	/* ベクトル命令の対応を確認する */
	x86_check_cpuid_flags();
#endif

	/* COMの初期化を行う */
	hRes = CoInitialize(0);
	if (hRes != S_OK)
		return FALSE;

	/* ロケールを初期化する */
	init_locale_code();

	/* パッケージの初期化処理を行う */
	if (!init_file())
		return FALSE;

	/* コンフィグの初期化処理を行う */
	if (!init_conf())
		return FALSE;

	/* ウィンドウを作成する */
	if (!InitWindow(hInstance, nCmdShow))
		return FALSE;

#ifdef USE_DEBUGGER
	/* デバッガウィンドウを作成する */
	if (!InitDebuggerWindow(hInstance, nCmdShow))
		return FALSE;

	/* スタートアップファイル/ラインを取得する */
	if (!GetStartupPosition())
		return FALSE;
#endif

	/* 描画エンジンを初期化する */
	if (!InitRenderingEngine())
		return FALSE;

	/* DirectSoundを初期化する */
	if (!DSInitialize(hWndMain))
	{
		log_error(conv_utf16_to_utf8(get_ui_message(UIMSG_NO_SOUND_DEVICE)));
		return FALSE;
	}

	if (!bD3D && !bOpenGL)
	{
		/* バックイメージを作成する */
		if(!CreateBackImage())
			return FALSE;
	}

#ifdef USE_CAPTURE
	if (!init_capture())
		return FALSE;
#endif

	return TRUE;
}

/* 描画エンジンを初期化する */
static BOOL InitRenderingEngine(void)
{
#ifdef USE_CAPTURE
	/*
	 * キャプチャアプリではOpenGLを利用する
	 *  - リプレイアプリはLinuxで動き、OpenGLを利用するため
	 */
	if (InitOpenGL())
	{
		bOpenGL = TRUE;
		return TRUE;
	}
	log_info(conv_utf16_to_utf8(get_ui_message(UIMSG_WIN_NO_OPENGL)));
	return FALSE;
#endif

	/* まずDirect3Dを初期化してみる */
	if (_access("no-direct3d.txt", 0) != 0)
	{
		/* Direct3Dを初期化する */
		if (D3DInitialize(hWndMain))
		{
			bD3D = TRUE;
			return TRUE;
		}
		log_info(conv_utf16_to_utf8(get_ui_message(UIMSG_WIN_NO_DIRECT3D)));
	}

	/* 次にOpenGLを初期化してみる */
	if (_access("no-opengl.txt", 0) != 0)
	{
		/* OpenGLを初期化する */
		if(InitOpenGL())
		{
			bOpenGL = TRUE;
			return TRUE;
		}
		log_info(conv_utf16_to_utf8(get_ui_message(UIMSG_WIN_NO_OPENGL)));
	}

	/* Direct3DとOpenGLが利用できない場合はGDIを利用する */
	return TRUE;
}

/* 基盤レイヤの終了処理を行う */
static void CleanupApp(void)
{
	/* フルスクリーンモードであれば解除する */
	if (bFullScreen)
		ToggleFullScreen();

	/* Direct3Dの終了処理を行う */
	if (bD3D)
		D3DCleanup();

	/* OpenGLコンテキストを破棄する */
	if (bOpenGL && hGLRC != NULL)
	{
		cleanup_opengl();
		wglMakeCurrent(NULL, NULL);
		wglDeleteContext(hGLRC);
		hGLRC = NULL;
	}

	/* ウィンドウのデバイスコンテキストを破棄する */
	if (hWndDC != NULL)
	{
		ReleaseDC(hWndMain, hWndDC);
		hWndDC = NULL;
	}

	if (!bOpenGL && !bD3D)
	{
		/* バックイメージのビットマップを破棄する */
		if (hBitmap != NULL)
		{
			DeleteObject(hBitmap);
			hBitmap = NULL;
		}

		/* バックイメージのデバイスコンテキストを破棄する */
		if (hBitmapDC != NULL)
		{
			DeleteDC(hBitmapDC);
			hBitmapDC = NULL;
		}

		/* バックイメージを破棄する */
		if (BackImage != NULL)
		{
			destroy_image(BackImage);
			BackImage = NULL;
		}
	}

	/* DirectSoundの終了処理を行う */
	DSCleanup();

	/* コンフィグの終了処理を行う */
	cleanup_conf();

	/* ログファイルをクローズする */
	if(pLogFile != NULL)
		fclose(pLogFile);

#ifdef USE_CAPTURE
	cleanup_capture();
#endif
}

/* ウィンドウを作成する */
static BOOL InitWindow(HINSTANCE hInstance, int nCmdShow)
{
	WNDCLASSEX wcex;
	RECT rc;
	DWORD style;
	int dw, dh, i;

	/* ディスプレイのサイズが足りない場合 */
	if (GetSystemMetrics(SM_CXVIRTUALSCREEN) < conf_window_width ||
		GetSystemMetrics(SM_CYVIRTUALSCREEN) < conf_window_height)
	{
		MessageBox(NULL,
				   get_ui_message(UIMSG_WIN_SMALL_DISPLAY),
				   get_ui_message(UIMSG_ERROR),
				   MB_OK | MB_ICONERROR);
		return FALSE;
	}

	/* ウィンドウクラスを登録する */
	wcex.cbSize			= sizeof(WNDCLASSEX);
	wcex.style          = 0;
	wcex.lpfnWndProc    = WndProc;
	wcex.cbClsExtra     = 0;
	wcex.cbWndExtra     = 0;
	wcex.hInstance      = hInstance;
	wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SUIKA));
	wcex.hCursor        = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground  = (HBRUSH)GetStockObject(conf_window_white ?
												 WHITE_BRUSH : BLACK_BRUSH);
	wcex.lpszMenuName   = NULL;
	wcex.lpszClassName  = wszWindowClass;
	wcex.hIconSm		= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SMALL));
	if (!RegisterClassEx(&wcex))
		return FALSE;

	/* ウィンドウのスタイルを決める */
	if (!conf_window_fullscreen_disable && !conf_window_maximize_disable) {
		style = WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX |
			WS_OVERLAPPED;
	} else {
		style = WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_OVERLAPPED;
	}

	/* フレームのサイズを取得する */
	dw = GetSystemMetrics(SM_CXFIXEDFRAME) * 2;
	dh = GetSystemMetrics(SM_CYCAPTION) +
		GetSystemMetrics(SM_CYMENU) +
		GetSystemMetrics(SM_CYFIXEDFRAME) * 2;

	/* ウィンドウのタイトルをUTF-8からUTF-16に変換する */
	MultiByteToWideChar(CP_UTF8, 0, conf_window_title, -1, wszTitle,
						TITLE_BUF_SIZE - 1);

	/* ウィンドウを作成する */
	hWndMain = CreateWindowEx(0, wszWindowClass, wszTitle, style,
#ifdef USE_DEBUGGER
							  10, 10,
#else
							  (int)CW_USEDEFAULT, (int)CW_USEDEFAULT,
#endif
							  conf_window_width + dw, conf_window_height + dh,
							  NULL, NULL, hInstance, NULL);
	if (hWndMain == NULL)
		return FALSE;

	/* ウィンドウのサイズを調整する */
	SetRectEmpty(&rc);
	rc.right = conf_window_width;
	rc.bottom = conf_window_height;
	AdjustWindowRectEx(&rc, (DWORD)GetWindowLong(hWndMain, GWL_STYLE),
#ifdef USE_DEBUGGER
					   TRUE,
#else
					   conf_window_menubar,
#endif
					   (DWORD)GetWindowLong(hWndMain, GWL_EXSTYLE));
	SetWindowPos(hWndMain, NULL, 0, 0, rc.right - rc.left, rc.bottom - rc.top,
				 SWP_NOZORDER | SWP_NOMOVE);

#ifdef USE_DEBUGGER
	/* デバッガ用メニューを作成する */
	InitDebuggerMenu(hWndMain);

	/* アクセラレータをロードする */
	hAccel = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDR_ACCEL));
#else
	/* ゲーム用メニューを作成する */
	if(conf_window_menubar)
		InitGameMenu();
#endif

	/* ウィンドウを表示する */
	ShowWindow(hWndMain, nCmdShow);
	UpdateWindow(hWndMain);

	/* デバイスコンテキストを取得する */
	hWndDC = GetDC(hWndMain);
	if(hWndDC == NULL)
		return FALSE;

	/* 0.1秒間(3フレーム)でウィンドウに関連するイベントを処理してしまう */
	dwStartTime = GetTickCount();
	for(i = 0; i < FPS / 10; i++)
		WaitForNextFrame();

	return TRUE;
}

#ifndef USE_DEBUGGER
/* ゲームウィンドウのメニューを初期化する */
static VOID InitGameMenu(void)
{
	HMENU hMenuFile = CreatePopupMenu();
	HMENU hMenuView = CreatePopupMenu();
    MENUITEMINFO mi;

	/* メニューを作成する */
	hMenu = CreateMenu();

	/* 1階層目を作成する準備を行う */
	ZeroMemory(&mi, sizeof(MENUITEMINFOW));
	mi.cbSize = sizeof(MENUITEMINFOW);
	mi.fMask = MIIM_TYPE | MIIM_SUBMENU;
	mi.fType = MFT_STRING;
	mi.fState = MFS_ENABLED;

	/* ファイル(F)を作成する */
	mi.hSubMenu = hMenuFile;
	mi.dwTypeData = (wchar_t *)get_ui_message(UIMSG_WIN_MENU_FILE);
	InsertMenuItem(hMenu, 0, TRUE, &mi);

	/* 表示(V)を作成する */
	mi.hSubMenu = hMenuView;
	mi.dwTypeData = (wchar_t *)get_ui_message(UIMSG_WIN_MENU_VIEW);
	InsertMenuItem(hMenu, 1, TRUE, &mi);

	/* 2階層目を作成する準備を行う */
	mi.fMask = MIIM_TYPE | MIIM_ID;

	/* 終了(Q)を作成する */
	mi.wID = ID_QUIT;
	mi.dwTypeData = (wchar_t *)get_ui_message(UIMSG_WIN_MENU_QUIT);
	InsertMenuItem(hMenuFile, 0, TRUE, &mi);

	/* フルスクリーン(S)を作成する */
	mi.wID = ID_FULLSCREEN;
	mi.dwTypeData = (wchar_t *)get_ui_message(UIMSG_WIN_MENU_FULLSCREEN);
	InsertMenuItem(hMenuView, 0, TRUE, &mi);

	/* メニューをセットする */
	SetMenu(hWndMain, hMenu);
}
#endif

/* OpenGLを初期化する */
static BOOL InitOpenGL(void)
{
	PIXELFORMATDESCRIPTOR pfd = {
		sizeof(PIXELFORMATDESCRIPTOR),
		1,
		PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
		PFD_TYPE_RGBA,
		24, /* 24-bit color */
		0, 0, 0, 0, 0, 0,
		0,
		0,
		0,
		0, 0, 0, 0,
		0, /* no z-buffer */
		0,
		0,
		PFD_MAIN_PLANE,
		0,
		0, 0, 0
	};
	static const int  contextAttibs[]= {
		WGL_CONTEXT_MAJOR_VERSION_ARB, 2,
		WGL_CONTEXT_MINOR_VERSION_ARB, 0,
		WGL_CONTEXT_FLAGS_ARB, 0,
		WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
		0
	};
	HGLRC (WINAPI *wglCreateContextAttribsARB)(HDC hDC, HGLRC hShareContext,
											   const int *attribList);
	HGLRC hGLRCOld;
	int pixelFormat;
	int i;

	/* ピクセルフォーマットを選択する */
	pixelFormat = ChoosePixelFormat(hWndDC, &pfd);
	if (pixelFormat == 0)
	{
		log_info("Failed to call ChoosePixelFormat()");
		return FALSE;
	}
	SetPixelFormat(hWndDC, pixelFormat, &pfd);

	/* OpenGLコンテキストを作成する */
	hGLRC = wglCreateContext(hWndDC);
	if (hGLRC == NULL)
	{
		log_info("Failed to call wglCreateContext()");
		return FALSE;
	}
	wglMakeCurrent(hWndDC, hGLRC);

	/* wglCreateContextAttribsARB()へのポインタを取得する */
	wglCreateContextAttribsARB =
		(void *)wglGetProcAddress("wglCreateContextAttribsARB");
	if (wglCreateContextAttribsARB == NULL)
	{
		log_info("API wglCreateContextAttribsARB not found.");
		wglMakeCurrent(NULL, NULL);
		wglDeleteContext(hGLRC);
		hGLRC = NULL;
		return FALSE;
	}

	/* 新しい HGLRC の作成 */
	hGLRCOld = hGLRC;
	hGLRC = wglCreateContextAttribsARB(hWndDC, NULL, contextAttibs);
	if (hGLRC == NULL)
	{
		log_info("Failed to call wglCreateContextAttribsARB()");
		wglMakeCurrent(NULL, NULL);
		wglDeleteContext(hGLRCOld);
		return FALSE;
	}
	wglMakeCurrent(hWndDC, hGLRC);
	wglDeleteContext(hGLRCOld);

	/* 仮想マシンを検出したらOpenGLを使わない */
	if (strcmp((const char *)glGetString(GL_VENDOR), "VMware, Inc.") == 0) {
		log_info("Detected virtual machine environment.");
		wglMakeCurrent(NULL, NULL);
		wglDeleteContext(hGLRC);
		hGLRC = NULL;
		return FALSE;
	}

	/* APIのポインタを取得する */
	for (i = 0; i < (int)(sizeof(APITable) / sizeof(struct GLExtAPITable)); i++)
	{
		*APITable[i].func = (void *)wglGetProcAddress(APITable[i].name);
		if (*APITable[i].func == NULL)
		{
			log_info("API %s not found.", APITable[i].name);
			wglMakeCurrent(NULL, NULL);
			wglDeleteContext(hGLRC);
			hGLRC = NULL;
			return FALSE;
		}
	}

	/* レンダラを初期化する */
	if (!init_opengl())
	{
		log_info("Failed to initialize OpenGL.");
		wglMakeCurrent(NULL, NULL);
		wglDeleteContext(hGLRC);
		hGLRC = NULL;
		return FALSE;
	}

	return TRUE;
}

/* バックイメージを作成する */
static BOOL CreateBackImage(void)
{
	BITMAPINFO bi;
	pixel_t *pixels;

	/* ビットマップを作成する */
	memset(&bi, 0, sizeof(BITMAPINFO));
	bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bi.bmiHeader.biWidth = conf_window_width;
	bi.bmiHeader.biHeight = -conf_window_height; /* Top-down */
	bi.bmiHeader.biPlanes = 1;
	bi.bmiHeader.biBitCount = 32;
	bi.bmiHeader.biCompression = BI_RGB;
	hBitmapDC = CreateCompatibleDC(NULL);
	if(hBitmapDC == NULL)
		return FALSE;

	/* DIBを作成する */
	hBitmap = CreateDIBSection(hBitmapDC, &bi, DIB_RGB_COLORS,
							   (VOID **)&pixels, NULL, 0);
	if(hBitmap == NULL || pixels == NULL)
		return FALSE;
	SelectObject(hBitmapDC, hBitmap);

	/* イメージを作成する */
	BackImage = create_image_with_pixels(conf_window_width, conf_window_height,
										 pixels);
	if(BackImage == NULL)
		return FALSE;
	if(conf_window_white) {
		lock_image(BackImage);
		clear_image_white(BackImage);
		unlock_image(BackImage);
	}

	return TRUE;
}

/* ゲームループを実行する */
static void GameLoop(void)
{
	int x, y, w, h;
	BOOL bBreak;

	/* 最初にイベントを処理してしまう */
	if(!SyncEvents())
		return;

	/* 最初のフレームの開始時刻を取得する */
	dwStartTime = GetTickCount();

	while(TRUE)
	{
#ifdef USE_CAPTURE
		/* 入力のキャプチャを行う */
		capture_input();
#endif

		/* DirectShowで動画を再生中の場合 */
		if(bDShowMode)
		{
			/* ウィンドウイベントを処理する */
			if(!SyncEvents())
				break;

			/* @videoコマンドを実行する */
			if(!on_event_frame(&x, &y, &w, &h))
				break;

			continue;
		}

		if (bD3D)
		{
			/* フレームの描画を開始する */
			D3DStartFrame();
		}
		else if (bOpenGL)
		{
			/* フレームの描画を開始する */
			opengl_start_rendering();
		}
		else
		{
			/* バックイメージのロックを行う */
			lock_image(BackImage);
		}

		/* フレームの実行と描画を行う */
		bBreak = FALSE;
		if(!on_event_frame(&x, &y, &w, &h))
		{
			/* スクリプトの終端に達した */
			bBreak = TRUE;
		}

		if (bD3D)
		{
			/* フレームの描画を終了する */
			D3DEndFrame();

			assert(get_image_lock_count() == 0);
		}
		else if(bOpenGL)
		{
			/* フレームの描画を終了する */
			opengl_end_rendering();

			/* 描画を反映する */
			SwapBuffers(hWndDC);
		}
		else
		{
			/* バックイメージのアンロックを行う */
			unlock_image(BackImage);

			/* 描画範囲をウィンドウに転送する */
			if(w !=0 && h !=0)
				SyncBackImage(x, y, w, h);
		}

#ifdef USE_CAPTURE
		/* 出力のキャプチャを行う */
		if (!capture_output())
			break;
#endif

		if(bBreak)
			break;

		/* イベントを処理する */
		if(!SyncEvents())
			break;	/* 閉じるボタンが押された */

		/* 次の描画までスリープする */
		if(!WaitForNextFrame())
			break;	/* 閉じるボタンが押された */

		/* 次のフレームの開始時刻を取得する */
		dwStartTime = GetTickCount();
	}
}

/* ウィンドウにバックイメージを転送する */
static void SyncBackImage(int x, int y, int w, int h)
{
	BitBlt(hWndDC, x + nOffsetX, y + nOffsetY, w, h, hBitmapDC, x, y, SRCCOPY);
}

/* キューにあるイベントを処理する */
static BOOL SyncEvents(void)
{
	DWORD dwStopWatchPauseStart;
	MSG msg;

	/* イベント処理の開始時刻を求める */
	dwStopWatchPauseStart = GetTickCount();

	/* イベント処理を行う */
	while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
	{
		if(msg.message == WM_QUIT)
			return FALSE;
		if(!TranslateAccelerator(msg.hwnd, hAccel, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	/* イベント処理にかかった時間をストップウォッチから除外するようにする */
	dwStopWatchOffset += GetTickCount() - dwStopWatchPauseStart;

	return TRUE;
}

/* 次のフレームの開始時刻までイベント処理とスリープを行う */
static BOOL WaitForNextFrame(void)
{
	DWORD end, lap, wait, span;

	/* 3Dのときは60FPSを目指し、GDIのときは30FPSを目指す */
	span = (bD3D || bOpenGL) ? FRAME_MILLI / 2 : FRAME_MILLI;

	/* 次のフレームの開始時刻になるまでイベント処理とスリープを行う */
	do {
		/* イベントがある場合は処理する */
		if(!SyncEvents())
			return FALSE;

		/* 経過時刻を取得する */
		end = GetTickCount();
		lap = dwStartTime - end;

		/* 次のフレームの開始時刻になった場合はスリープを終了する */
		if(lap >= span) {
			dwStartTime = end;
			break;
		}

		/* スリープする時間を求める */
		wait = (span - lap > SLEEP_MILLI) ? SLEEP_MILLI : span - lap;

		/* スリープする */
		Sleep(wait);
	} while(wait > 0);

	return TRUE;
}

/* ウィンドウプロシージャ */
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int kc;

#ifdef USE_DEBUGGER
	/* デバッグウィンドウと子ウィンドウ、あるいはWM_COMMANDの場合 */
	if(IsDebuggerHWND(hWnd) || message == WM_COMMAND)
		return WndProcDebugHook(hWnd, message, wParam, lParam);
#endif

	switch(message)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	case WM_SYSKEYDOWN:
		if(wParam == VK_RETURN && (HIWORD(lParam) & KF_ALTDOWN))
		{
			if (!conf_window_fullscreen_disable)
				ToggleFullScreen();
			return 0;
		}
		if(wParam != VK_F4)
			break;
		/* ALT+F4のとき */
		/* fall-thru */
	case WM_CLOSE:
#ifdef USE_DEBUGGER
		DestroyWindow(hWnd);
		return 0;
#else
		if (MessageBox(hWnd,
					   get_ui_message(UIMSG_EXIT),
					   conv_utf8_to_utf16(conf_window_title),
					   MB_OKCANCEL) == IDOK)
			DestroyWindow(hWnd);
		return 0;
#endif
	case WM_LBUTTONDOWN:
		on_event_mouse_press(MOUSE_LEFT, LOWORD(lParam) - nOffsetX,
							 HIWORD(lParam) - nOffsetY);
		return 0;
	case WM_LBUTTONUP:
		on_event_mouse_release(MOUSE_LEFT, LOWORD(lParam) - nOffsetX,
							   HIWORD(lParam) - nOffsetY);
		return 0;
	case WM_RBUTTONDOWN:
		on_event_mouse_press(MOUSE_RIGHT, LOWORD(lParam) - nOffsetX,
							 HIWORD(lParam) - nOffsetY);
		return 0;
	case WM_RBUTTONUP:
		on_event_mouse_release(MOUSE_RIGHT, LOWORD(lParam) - nOffsetX,
							   HIWORD(lParam) - nOffsetY);
		return 0;
	case WM_KEYDOWN:
		/* オートリピートの場合を除外する */
		if((HIWORD(lParam) & 0x4000) != 0)
			return 0;
		if((int)wParam == VK_ESCAPE && bFullScreen)
		{
			ToggleFullScreen();
			return 0;
		}
		kc = ConvertKeyCode((int)wParam);
		if(kc != -1)
			on_event_key_press(kc);
		return 0;
	case WM_KEYUP:
		kc = ConvertKeyCode((int)wParam);
		if(kc != -1)
			on_event_key_release(kc);
		return 0;
	case WM_MOUSEMOVE:
		on_event_mouse_move(LOWORD(lParam) - nOffsetX,
							HIWORD(lParam) - nOffsetY);
		return 0;
	case WM_MOUSEWHEEL:
		if((int)(short)HIWORD(wParam) > 0)
		{
			on_event_key_press(KEY_UP);
			on_event_key_release(KEY_UP);
		}
		else if((int)(short)HIWORD(wParam) < 0)
		{
			on_event_key_press(KEY_DOWN);
			on_event_key_release(KEY_DOWN);
		}
		return 0;
	case WM_KILLFOCUS:
		on_event_key_release(KEY_CONTROL);
		return 0;
	case WM_SYSCHAR:
		return 0;
	case WM_NCLBUTTONDBLCLK:
		if(wParam == HTCAPTION)
		{
			if (!conf_window_fullscreen_disable)
				ToggleFullScreen();
			return 0;
		}
		break;
	case WM_SYSCOMMAND:
		if(wParam == SC_MAXIMIZE)
		{
			if (!conf_window_fullscreen_disable)
				ToggleFullScreen();
			return TRUE;
		}
		break;
	case WM_PAINT:
		OnPaint();
		return 0;
	case WM_COMMAND:
		OnCommand(LOWORD(wParam));
		return 0;
	case WM_GRAPHNOTIFY:
		if(!DShowProcessEvent())
			bDShowMode = FALSE;
		break;
	default:
		break;
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}

/* キーコードの変換を行う */
static int ConvertKeyCode(int nVK)
{
	switch(nVK)
	{
	case VK_CONTROL:
		return KEY_CONTROL;
	case VK_SPACE:
		return KEY_SPACE;
	case VK_RETURN:
		return KEY_RETURN;
	case VK_UP:
		return KEY_UP;
	case VK_DOWN:
		return KEY_DOWN;
	case VK_ESCAPE:
		return KEY_ESCAPE;
	case 'C':
		return KEY_C;
	default:
		break;
	}
	return -1;
}

/* フルスクリーンモードの切り替えを行う */
static void ToggleFullScreen(void)
{
	LONG style;
	int cx, cy;

#ifdef USE_DEBUGGER
	return;
#endif

	assert(!conf_window_fullscreen_disable);

	if(!bFullScreen)
	{
		bFullScreen = TRUE;

		SetMenu(hWndMain, NULL);

		ChangeDisplayMode();

		cx = GetSystemMetrics(SM_CXSCREEN);
		cy = GetSystemMetrics(SM_CYSCREEN);
		nOffsetX = (cx - conf_window_width) / 2;
		nOffsetY = (cy - conf_window_height) / 2;
		GetWindowRect(hWndMain, &rectWindow);
		SetWindowLong(hWndMain, GWL_STYLE, (LONG)(WS_POPUP | WS_VISIBLE));
		SetWindowLong(hWndMain, GWL_EXSTYLE, WS_EX_TOPMOST);
		SetWindowPos(hWndMain, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE |
					 SWP_NOZORDER | SWP_FRAMECHANGED);
		MoveWindow(hWndMain, 0, 0, cx, cy, TRUE);
		ShowWindow(hWndMain, SW_SHOW);
		InvalidateRect(NULL, NULL, TRUE);

		D3DSetDisplayOffset(nOffsetX, nOffsetY);
	}
	else
	{
		bFullScreen = FALSE;

		ResetDisplayMode();

		if (hMenu != NULL)
			SetMenu(hWndMain, hMenu);

		nOffsetX = 0;
		nOffsetY = 0;

		if (!conf_window_fullscreen_disable && !conf_window_maximize_disable) {
			style = WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX |
				WS_OVERLAPPED;
		} else {
			style = WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_OVERLAPPED;
		}

		SetWindowLong(hWndMain, GWL_STYLE, style);
		SetWindowLong(hWndMain, GWL_EXSTYLE, 0);
		SetWindowPos(hWndMain, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE |
					 SWP_NOZORDER | SWP_FRAMECHANGED);
		MoveWindow(hWndMain, rectWindow.left, rectWindow.top,
				   rectWindow.right - rectWindow.left,
				   rectWindow.bottom - rectWindow.top, TRUE);
		ShowWindow(hWndMain, SW_SHOW);
		InvalidateRect(NULL, NULL, TRUE);

		D3DSetDisplayOffset(0, 0);
	}
}

/* 画面のサイズを変更する */
static void ChangeDisplayMode(void)
{
	DEVMODE dm, dmSmallest;
	HDC hDCDisp;
	DWORD n, size;
	int bpp;
	BOOL bFound;

	/* ウィンドウサイズと同じサイズに変更できるか試してみる */
	dm.dmSize = sizeof(DEVMODE);
	dm.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT;
	dm.dmPelsWidth = (DWORD)conf_window_width;
	dm.dmPelsHeight = (DWORD)conf_window_height;
	if(ChangeDisplaySettings(&dm, CDS_FULLSCREEN) == DISP_CHANGE_SUCCESSFUL)
	{
		bDisplaySettingsChanged = TRUE;
		return;
	}

	/* 現在の画面のBPPを取得する */
	hDCDisp = GetDC(NULL);
	bpp = GetDeviceCaps(hDCDisp, BITSPIXEL);
	ReleaseDC(NULL, hDCDisp);

	/* ウィンドウサイズ以上の最小の画面サイズを取得する */
	n = 0;
	bFound = FALSE;
	size = 0xffffffff;
	while(EnumDisplaySettings(NULL, n, &dm))
	{
		if(dm.dmPelsWidth >= (DWORD)conf_window_width &&
		   dm.dmPelsHeight >= (DWORD)conf_window_height &&
		   dm.dmBitsPerPel == (DWORD)bpp &&
		   dm.dmPelsWidth + dm.dmPelsHeight <= size)
		{
			bFound = TRUE;
			size = dm.dmPelsWidth + dm.dmPelsHeight;
			dmSmallest = dm;
		}
		n++;
	}

	/* 求めた画面サイズに変更する */
	if(bFound)
	{
		if(ChangeDisplaySettings(&dmSmallest, CDS_FULLSCREEN) ==
		   DISP_CHANGE_SUCCESSFUL)
		{
			bDisplaySettingsChanged = TRUE;
			return;
		}
	}
}

/* 画面のサイズを元に戻す */
static void ResetDisplayMode(void)
{
	if(bDisplaySettingsChanged)
	{
		ChangeDisplaySettings(NULL, 0);
		bDisplaySettingsChanged = FALSE;
	}
}

/* ウィンドウの内容を更新する */
static void OnPaint(void)
{
	HDC hDC;
	PAINTSTRUCT ps;

	hDC = BeginPaint(hWndMain, &ps);
	if(!bD3D && !bOpenGL && hBitmapDC != NULL)
	{
		BitBlt(hDC,
			   ps.rcPaint.left,
			   ps.rcPaint.top,
			   ps.rcPaint.right - ps.rcPaint.left,
			   ps.rcPaint.bottom - ps.rcPaint.top,
			   hBitmapDC,
			   ps.rcPaint.left - nOffsetX,
			   ps.rcPaint.top - nOffsetY,
			   SRCCOPY);
	}
	EndPaint(hWndMain, &ps);

	if (bD3D)
		D3DRedraw();
}

/* WM_COMMANDを処理する */
static void OnCommand(UINT nID)
{
	switch(nID)
	{
	case ID_QUIT:
		PostMessage(hWndMain,WM_CLOSE, 0, 0);
		break;
	case ID_FULLSCREEN:
		if (!conf_window_fullscreen_disable)
			ToggleFullScreen();
		break;
	default:
		break;
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

	/* 必要ならログファイルをオープンする */
	if(!OpenLogFile())
		return false;

	/* メッセージボックスを表示する */
	va_start(ap, s);
	vsnprintf(buf, sizeof(buf), s, ap);
	va_end(ap);
#ifdef USE_DEBUGGER
	MessageBox(hWndMain, conv_utf8_to_utf16(buf), wszTitle,
			   MB_OK | MB_ICONINFORMATION);
#endif

	/* ログファイルがオープンされている場合 */
	if(pLogFile != NULL)
	{
		/* ファイルへ出力する */
		fprintf(pLogFile, buf);
		fprintf(pLogFile, "\n");
		fflush(pLogFile);
		if(ferror(pLogFile))
			return false;
	}
	return true;
}

/*
 * WARNログを出力する
 */
bool log_warn(const char *s, ...)
{
	char buf[LOG_BUF_SIZE];
	va_list ap;

	/* 必要ならログファイルをオープンする */
	if(!OpenLogFile())
		return false;

	/* メッセージボックスを表示する */
	va_start(ap, s);
	vsnprintf(buf, sizeof(buf), s, ap);
	va_end(ap);
	MessageBox(hWndMain, conv_utf8_to_utf16(buf), wszTitle,
			   MB_OK | MB_ICONWARNING);

	/* ログファイルがオープンされている場合 */
	if(pLogFile != NULL)
	{
		/* ファイルへ出力する */
		fprintf(pLogFile, buf);
		fprintf(pLogFile, "\n");
		fflush(pLogFile);
		if(ferror(pLogFile))
			return false;
	}
	return true;
}

/*
 * ERRORログを出力する
 */
bool log_error(const char *s, ...)
{
	char buf[LOG_BUF_SIZE];
	va_list ap;

	/* 必要ならログファイルをオープンする */
	if(!OpenLogFile())
		return false;

	/* メッセージボックスを表示する */
	va_start(ap, s);
	vsnprintf(buf, sizeof(buf), s, ap);
	va_end(ap);
	MessageBox(hWndMain, conv_utf8_to_utf16(buf), wszTitle,
			   MB_OK | MB_ICONERROR);

	/* ログファイルがオープンされている場合 */
	if(pLogFile != NULL)
	{
		/* ファイルへ出力する */
		fprintf(pLogFile, buf);
		fprintf(pLogFile, "\n");
		fflush(pLogFile);
		if(ferror(pLogFile))
			return false;
	}
	return true;
}

/* ログをオープンする */
static BOOL OpenLogFile(void)
{
#ifdef USE_DEBUGGER
	return TRUE;
#else
	wchar_t path[MAX_PATH] = {0};

	/* すでにオープンされていれば成功とする */
	if(pLogFile != NULL)
		return TRUE;

	/* オープンする */
	if (!conf_release ||
		/* ウィンドウタイトルが空のエラー処理中の場合 */
		(conf_release && conf_window_title == NULL))
	{
		/* ゲームディレクトリに作成する */
		pLogFile = _wfopen(conv_utf8_to_utf16(LOG_FILE), L"w");
		if (pLogFile == NULL)
		{
			/* 失敗 */
			MessageBox(NULL, get_ui_message(UIMSG_CANNOT_OPEN_LOG), wszTitle,
					   MB_OK | MB_ICONWARNING);
			return FALSE;
		}
	}
	else
	{
		/* AppDataに作成する */
		SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, path);
		wcsncat(path, L"\\", MAX_PATH - 1);
		wcsncat(path, conv_utf8_to_utf16(conf_window_title), MAX_PATH - 1);
		wcsncat(path, L"\\", MAX_PATH - 1);
		wcsncat(path, conv_utf8_to_utf16(LOG_FILE), MAX_PATH - 1);
		pLogFile = _wfopen(path, L"w");
		if (pLogFile == NULL)
		{
			/* 失敗 */
			MessageBox(NULL, get_ui_message(UIMSG_CANNOT_OPEN_LOG),
					   get_ui_message(UIMSG_ERROR),
					   MB_OK | MB_ICONWARNING);
			return FALSE;
		}
	}

	/* 成功 */
	return TRUE;
#endif
}

/*
 * UTF-8のメッセージをUTF-16に変換する
 */
const wchar_t *conv_utf8_to_utf16(const char *utf8_message)
{
	assert(utf8_message != NULL);

	/* UTF8からUTF16に変換する */
	MultiByteToWideChar(CP_UTF8, 0, utf8_message, -1, wszMessage,
						CONV_MESSAGE_SIZE - 1);

	return wszMessage;
}

/*
 * UTF-16のメッセージをUTF-8に変換する
 */
const char *conv_utf16_to_utf8(const wchar_t *utf16_message)
{
	assert(utf16_message != NULL);

	/* ワイド文字からUTF-8に変換する */
	WideCharToMultiByte(CP_UTF8, 0, utf16_message, -1, szMessage,
						CONV_MESSAGE_SIZE - 1, NULL, NULL);

	return szMessage;
}

/*
 * GPUを使うか調べる
 */
bool is_gpu_accelerated(void)
{
	if (bD3D || bOpenGL)
		return TRUE;

	return FALSE;
}

/*
 * OpenGLが有効か調べる
 */
bool is_opengl_enabled(void)
{
	return bOpenGL;
}

/*
 * テクスチャをロックする
 */
bool lock_texture(int width, int height, pixel_t *pixels,
				  pixel_t **locked_pixels, void **texture)
{
	if (bD3D)
	{
		if (!D3DLockTexture(width, height, pixels, locked_pixels, texture))
			return false;
	}
	else if (bOpenGL)
	{
		if (!opengl_lock_texture(width, height, pixels, locked_pixels,
								 texture))
			return false;
	}
	else
	{
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
	if (bD3D)
	{
		D3DUnlockTexture(width, height, pixels, locked_pixels, texture);
	}
	else if (bOpenGL)
	{
		opengl_unlock_texture(width, height, pixels, locked_pixels, texture);
	}
	else
	{
		assert(*locked_pixels != NULL);
		*locked_pixels = NULL;
	}
}

/*
 * テクスチャを破棄する
 */
void destroy_texture(void *texture)
{
	if (bD3D)
		D3DDestroyTexture(texture);
	else if (bOpenGL)
		opengl_destroy_texture(texture);
}

/*
 * イメージをレンダリングする
 */
void render_image(int dst_left, int dst_top, struct image * RESTRICT src_image,
                  int width, int height, int src_left, int src_top, int alpha,
                  int bt)
{
	if (bD3D)
	{
		D3DRenderImage(dst_left, dst_top, src_image, width, height, src_left,
					   src_top, alpha, bt);
	}
	else if (bOpenGL)
	{
		opengl_render_image(dst_left, dst_top, src_image, width, height,
							src_left, src_top, alpha, bt);
	}
	else
	{
		draw_image(BackImage, dst_left, dst_top, src_image, width, height,
				   src_left, src_top, alpha, bt);
	}
}

/*
 * 画面にイメージをルール付きでレンダリングする
 */
void render_image_rule(struct image * RESTRICT src_img,
					   struct image * RESTRICT rule_img,
					   int threshold)
{
	if (bD3D)
		D3DRenderImageRule(src_img, rule_img, threshold);
	else if(bOpenGL)
		opengl_render_image_rule(src_img, rule_img, threshold);
	else
		draw_image_rule(BackImage, src_img, rule_img, threshold);
}

/*
 * 画面にイメージをルール付き(メルト)でレンダリングする
 */
void render_image_melt(struct image * RESTRICT src_img,
					   struct image * RESTRICT rule_img,
					   int threshold)
{
	if (bD3D)
		D3DRenderImageMelt(src_img, rule_img, threshold);
	else if(bOpenGL)
		opengl_render_image_melt(src_img, rule_img, threshold);
	else
		draw_image_melt(BackImage, src_img, rule_img, threshold);
}

/*
 * セーブディレクトリを作成する
 */
bool make_sav_dir(void)
{
	wchar_t path[MAX_PATH] = {0};

	if (conf_release) {
		/* AppDataに作成する */
		SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, path);
		wcsncat(path, L"\\", MAX_PATH - 1);
		wcsncat(path, conv_utf8_to_utf16(conf_window_title), MAX_PATH - 1);
		CreateDirectory(path, NULL);
	} else {
		/* ゲームディレクトリに作成する */
		CreateDirectory(conv_utf8_to_utf16(SAVE_DIR), NULL);
	}

	return true;
}

/*
 * データのディレクトリ名とファイル名を指定して有効なパスを取得する
 */
char *make_valid_path(const char *dir, const char *fname)
{
	wchar_t *buf;
	const char *result;
	size_t len;

	if (dir == NULL)
		dir = "";

	if (conf_release && strcmp(dir, SAVE_DIR) == 0) {
		/* AppDataを参照する場合 */
		wchar_t path[MAX_PATH] = {0};
		SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, path);
		wcsncat(path, L"\\", MAX_PATH - 1);
		wcsncat(path, conv_utf8_to_utf16(conf_window_title), MAX_PATH - 1);
		wcsncat(path, L"\\", MAX_PATH - 1);
		wcsncat(path, conv_utf8_to_utf16(fname), MAX_PATH - 1);
		return strdup(conv_utf16_to_utf8(path));
	}

	/* パスのメモリを確保する */
	len = strlen(dir) + 1 + strlen(fname) + 1;
	buf = malloc(sizeof(wchar_t) * len);
	if (buf == NULL)
		return NULL;

	/* パスを生成する */
	wcscpy(buf, conv_utf8_to_utf16(dir));
	if (strlen(dir) != 0)
		wcscat(buf, L"\\");
	wcscat(buf, conv_utf8_to_utf16(fname));

	result = conv_utf16_to_utf8(buf);
	free(buf);
	return strdup(result);
}

/*
 * バックイメージを取得する
 */
struct image *get_back_image(void)
{
	return BackImage;
}

/*
 * タイマをリセットする
 */
void reset_stop_watch(stop_watch_t *t)
{
#ifndef USE_CAPTURE
	*t = GetTickCount();
	dwStopWatchOffset = 0;
#else
	extern uint64_t cap_cur_time;
	*t = cap_cur_time;
#endif
}

/*
 * タイマのラップをミリ秒単位で取得する
 */
int get_stop_watch_lap(stop_watch_t *t)
{
#ifndef USE_CAPTURE
	DWORD dwCur = GetTickCount();
	return (int32_t)(dwCur - *t - dwStopWatchOffset);
#else
	extern uint64_t cap_cur_time;
	return (int32_t)(cap_cur_time - *t);
#endif

}

/*
 * 終了ダイアログを表示する
 */
bool exit_dialog(void)
{
	if (MessageBox(hWndMain,
				   get_ui_message(UIMSG_EXIT),
				   conv_utf8_to_utf16(conf_window_title),
				   MB_OKCANCEL) == IDOK)
		return true;
	return false;
}

/*
 * タイトルに戻るダイアログを表示する
 */
bool title_dialog(void)
{
	if (MessageBox(hWndMain,
				   get_ui_message(UIMSG_TITLE),
				   conv_utf8_to_utf16(conf_window_title),
				   MB_OKCANCEL) == IDOK)
		return true;
	return false;
}

/*
 * 削除ダイアログを表示する
 */
bool delete_dialog(void)
{
	if (MessageBox(hWndMain,
				   get_ui_message(UIMSG_DELETE),
  				   conv_utf8_to_utf16(conf_window_title),
				   MB_OKCANCEL) == IDOK)
		return true;
	return false;
}

/*
 * 上書きダイアログを表示する
 */
bool overwrite_dialog(void)
{
	if (MessageBox(hWndMain,
				   get_ui_message(UIMSG_OVERWRITE),
				   conv_utf8_to_utf16(conf_window_title),
				   MB_OKCANCEL) == IDOK)
		return true;
	return false;
}

/*
 * 初期設定ダイアログを表示する
 */
bool default_dialog(void)
{
	if (MessageBox(hWndMain,
				   get_ui_message(UIMSG_DEFAULT),
				   conv_utf8_to_utf16(conf_window_title),
				   MB_OKCANCEL) == IDOK)
		return true;
	return false;
}

/*
 * ビデオを再生する
 */
bool play_video(const char *fname, bool is_skippable)
{
	char *path;

	path = make_valid_path(MOV_DIR, fname);

	/* イベントループをDirectShow再生モードに設定する */
	bDShowMode = TRUE;

	/* クリックでスキップするかを設定する */
	bDShowSkippable = is_skippable;

	/* ビデオの再生を開始する */
	BOOL ret = DShowPlayVideo(hWndMain, path);
	if(!ret)
		bDShowMode = FALSE;

	free(path);
	return ret;
}

/*
 * ビデオを停止する
 */
void stop_video(void)
{
	DShowStopVideo();
	bDShowMode = FALSE;
}

/*
 * ビデオが再生中か調べる
 */
bool is_video_playing(void)
{
	return bDShowMode;
}

/*
 * ウィンドウタイトルを更新する
 */
void update_window_title(void)
{
	const char *separator;
	int cch1, cch2, cch3;

	/* セパレータを取得する */
	separator = conf_window_title_separator;
	if (separator == NULL)
		separator = " ";

	/* コンフィグのウィンドウタイトルをUTF-8からUTF-16に変換する */
	cch1 = MultiByteToWideChar(CP_UTF8, 0, conf_window_title, -1, wszTitle,
							   TITLE_BUF_SIZE - 1);
	cch1--;
	cch2 = MultiByteToWideChar(CP_UTF8, 0, separator, -1, wszTitle + cch1,
							   TITLE_BUF_SIZE - cch1 - 1);
	cch2--;
	cch3 = MultiByteToWideChar(CP_UTF8, 0, get_chapter_name(), -1,
							   wszTitle + cch1 + cch2,
							   TITLE_BUF_SIZE - cch1 - cch2 - 1);
	cch3--;
	wszTitle[cch1 + cch2 + cch3] = L'\0';

	/* ウィンドウのタイトルを設定する */
	SetWindowText(hWndMain, wszTitle);
}

/*
 * フルスクリーンモードがサポートされるか調べる
 */
bool is_full_screen_supported()
{
	return true;
}

/*
 * フルスクリーンモードであるか調べる
 */
bool is_full_screen_mode(void)
{
	return bFullScreen ? true : false;
}

/*
 * フルスクリーンモードを開始する
 */
void enter_full_screen_mode(void)
{
	if (!bFullScreen)
		ToggleFullScreen();
}

/*
 * フルスクリーンモードを終了する
 */
void leave_full_screen_mode(void)
{
	if (bFullScreen)
		ToggleFullScreen();
}

/*
 * システムのロケールを取得する
 */
const char *get_system_locale(void)
{
	switch (GetUserDefaultLCID()) {
	case 1033:	/* US */
	case 2057:	/* UK */
	case 3081:	/* オーストラリア */
	case 4105:	/* カナダ */
		return "en";
	case 1036:
		return "fr";
	case 1031:	/* ドイツ */
	case 2055:	/* スイス */
	case 3079:	/* オーストリア */
		return "de";
	case 3082:
		return "es";
	case 1040:
		return "it";
	case 1032:
		return "el";
	case 1049:
		return "ru";
	case 2052:
		return "zh";
	case 1028:
		return "tw";
	case 1041:
		return "ja";
	default:
		break;
	}
	return "other";
}
