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

/* Windows */
#include <windows.h>
#include <shlobj.h> /* SHGetFolderPath() to obtain "AppData" directory */

#ifdef _MSC_VER
#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif

/* msvcrt  */
#include <io.h> /* _access() */

/* Suika2 Base */
#include "suika.h"

/* Suika2 HAL Implementaion */
#include "dsound.h"
#include "dsvideo.h"
#include "tts_sapi.h"
#include "uimsg.h"
#include "resource.h"

/* Suika2 HAL Implementaion (Direct3D) */
#include "d3drender.h"

/* Suika2 HAL Implementaion (OpenGL) */
#include <GL/gl.h>
#include "glrender.h"
#include "glhelper.h"

/* Suika2 Pro */
#ifdef USE_DEBUGGER
#include <commctrl.h>
#include "windebug.h"
#include "package.h"
#endif

/* Suika2 Capture */
#ifdef USE_CAPTURE
#include "capture.h"
#endif

/* Suika2 Replay */
#ifdef USE_REPLAY
#include "replay.h"
#endif

/* x86 SSE/AVX Dispatch */
#ifdef SSE_VERSIONING
#include "x86.h"
#endif

/*
 * Constants
 */

/* A message to disable "Aero Snap" */
#define WM_RESTOREORIGINALSTYLE	(WM_USER + 1)

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
static const wchar_t wszWindowClassMain[] = L"SuikaMain";
#ifdef USE_DEBUGGER
static const wchar_t wszWindowClassGame[] = L"SuikaGame";
#endif

/*
 * Variables
 */

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
static HWND hWndGame;
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

/* フルスクリーンモードか */
static BOOL bFullScreen;

/* フルスクリーンモードに移行する必要があるか */
static BOOL bNeedFullScreen;

/* ウィンドウモードに移行する必要があるか */
static BOOL bNeedWindowed;

/* ウィンドウモードでのスタイル */
static DWORD dwStyle;

/* ウィンドウモードでの位置 */
static RECT rcWindow;

/* 最後に設定されたウィンドウサイズ */
static int nLastClientWidth, nLastClientHeight;

/* 最後に設定されたDPI */
#ifdef USE_DEBUGGER
static int nLastDpi;
#endif

/* RunFrame()が描画してよいか */
static BOOL bRunFrameAllow;

/* ストップウォッチの停止した時間 */
DWORD dwStopWatchOffset;

/* フルスクリーンモード時の描画オフセット */
static int nOffsetX;
static int nOffsetY;
static float fMouseScale;

/* DirectShowでビデオを再生中か */
static BOOL bDShowMode;

/* DirectShow再生中にクリックでスキップするか */
static BOOL bDShowSkippable;

/*
 * OpenGL Function Pointers
 */

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

/* A table to map OpenGL API names to addresses of function pointers. */
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

/*
 * Forward Declaration
 */

/* static */
static BOOL InitApp(HINSTANCE hInstance, int nCmdShow);
static BOOL InitRenderingEngine(void);
static void CleanupApp(void);
static BOOL InitWindow(HINSTANCE hInstance, int nCmdShow);
#ifndef USE_DEBUGGER
static VOID InitGameMenu(void);
#endif
static BOOL InitOpenGL(void);
static void GameLoop(void);
static BOOL RunFrame(void);
static BOOL SyncEvents(void);
static BOOL WaitForNextFrame(void);
static int ConvertKeyCode(int nVK);
static void OnPaint(HWND hWnd);
static void OnCommand(WPARAM wParam, LPARAM lParam);
static void OnSizing(int edge, LPRECT lpRect);
static void OnSize(void);
static void OnDpiChanged(HWND hWnd, UINT nDpi, LPRECT lpRect);
static void UpdateScreenOffsetAndScale(int nClientWidth, int nClientHeight);
static BOOL CreateBackImage(void);
static void SyncBackImage(int x, int y, int w, int h);
static BOOL OpenLogFile(void);

/* extern */
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
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

#ifdef USE_DEBUGGER
	DoPackagingIfArgExists();
	InitCommonControls();
#endif

	/* Sleep()の分解能を設定する */
	timeBeginPeriod(1);

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

	/* 基盤レイヤの終了処理を行う */
	CleanupApp();

	/* Sleep()の分解能を元に戻す */
	timeEndPeriod(1);

	return result;
}

/* 基盤レイヤの初期化処理を行う */
static BOOL InitApp(HINSTANCE hInstance, int nCmdShow)
{
	RECT rcClient;
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
	/* スタートアップファイル/ラインを取得する */
	if (!GetStartupPosition())
		return FALSE;
#endif

	/* 描画エンジンを初期化する */
	if (!InitRenderingEngine())
		return FALSE;

#ifdef USE_DEBUGGER
	/* ゲームパネルを再配置して描画サブシステムに反映する */
	GetClientRect(hWndMain, &rcClient);
	nLastClientWidth = nLastClientHeight = 0;
	UpdateScreenOffsetAndScale(rcClient.right, rcClient.bottom);
#else
	/* アスペクト比を補正する */
	GetClientRect(hWndMain, &rcClient);
	if (rcClient.right != conf_window_width || rcClient.bottom != conf_window_height)
		UpdateScreenOffsetAndScale(rcClient.right, rcClient.bottom);
#endif

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

	if (conf_tts_enable)
		InitSAPI();

#if defined(USE_CAPTURE)
	if (!init_capture())
		return FALSE;
#elif defined(USE_REPLAY)
	if (!init_replay(__argc, __wargv))
		return FALSE;
#endif

	return TRUE;
}

/* 描画エンジンを初期化する */
static BOOL InitRenderingEngine(void)
{
	/*
	 * Step.0: Use OpenGL for capture/replay apps.
	 */
#if defined(USE_CAPTURE) || defined(USE_REPLAY)
	if (InitOpenGL())
	{
		bOpenGL = TRUE;

		/* We disable window resizing for capture-replay apps. */
		dwStyle = (DWORD)GetWindowLong(hWndMain, GWL_STYLE);
		dwStyle ^= WS_THICKFRAME;
		SetWindowLong(hWndGame, GWL_STYLE, (LONG)dwStyle);
		conf_window_resize = 0;

		/* Set OpenGL screen size. */
		GetClientRect(hWndGame, &rcClient);
		opengl_set_screen(nOffsetX, nOffsetY, rc.right, rc.bottom);

		return TRUE;
	}
	log_info(conv_utf16_to_utf8(get_ui_message(UIMSG_WIN_NO_OPENGL)));
	return FALSE;
#endif	/* defined(USE_CAPTURE) || defined(USE_REPLAY) */

	/*
	 * Step.1: Try initializing Direct3D if there isn't "no-direct3d.txt" file.
	 */
	if (_access("no-direct3d.txt", 0) != 0)
	{
		/* Direct3Dを初期化する */
		if (D3DInitialize(hWndGame))
		{
			bD3D = TRUE;
			return TRUE;
		}

		/* Put error log. */
		log_info(conv_utf16_to_utf8(get_ui_message(UIMSG_WIN_NO_DIRECT3D)));
	}
	else
	{
		log_info("Fallback from Direct3D to OpenGL.");
	}

	/*
	 * Step.2: Try initializing OpenGL if there isn't "no-opengl.txt" file.
	 */
	if (_access("no-opengl.txt", 0) != 0)
	{
		/* OpenGLを初期化する */
		if(InitOpenGL())
		{
			bOpenGL = TRUE;
			return TRUE;
		}

		/* Put error log. */
		log_info(conv_utf16_to_utf8(get_ui_message(UIMSG_WIN_NO_OPENGL)));
	}

	/*
	 * Step.3: Use GDI instead of accelerations.
	 */

	/* Disable window resizing on GDI. */
	dwStyle = (DWORD)GetWindowLong(hWndMain, GWL_STYLE);
	dwStyle ^= WS_THICKFRAME;
	SetWindowLong(hWndMain, GWL_STYLE, (LONG)dwStyle);
	conf_window_resize = 0;

	/* Put error log. */
	log_info("Fallback from OpenGL to GDI.");

	return TRUE;
}

/* 基盤レイヤの終了処理を行う */
static void CleanupApp(void)
{
	/* コンフィグの終了処理を行う */
	cleanup_conf();

	/* ファイルの使用を終了する */
    cleanup_file();

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

	/* ログファイルをクローズする */
	if(pLogFile != NULL)
		fclose(pLogFile);

#ifdef USE_CAPTURE
	cleanup_capture();
#endif
#ifdef USE_REPLAY
	cleanup_replay();
#endif
}

/* ウィンドウを作成する */
static BOOL InitWindow(HINSTANCE hInstance, int nCmdShow)
{
	WNDCLASSEX wcex;
	RECT rc;
	int nVirtualScreenWidth, nVirtualScreenHeight;
	int nFrameAddWidth, nFrameAddHeight;
	int nMonitors;
	int nGameWidth, nGameHeight;
	int nWinWidth, nWinHeight;
	int nPosX, nPosY;
	int i;

	/* ウィンドウクラスを登録する */
	ZeroMemory(&wcex, sizeof(wcex));
	wcex.cbSize			= sizeof(WNDCLASSEX);
	wcex.lpfnWndProc    = WndProc;
	wcex.hInstance      = hInstance;
	wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SUIKA));
	wcex.hCursor        = LoadCursor(NULL, IDC_ARROW);
#ifndef USE_DEBUGGER
	wcex.hbrBackground  = (HBRUSH)GetStockObject(conf_window_white ? WHITE_BRUSH : BLACK_BRUSH);
#endif
	wcex.lpszClassName  = wszWindowClassMain;
	wcex.hIconSm		= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SMALL));
	if (!RegisterClassEx(&wcex))
		return FALSE;

	/* ウィンドウのスタイルを決める */
	if (!conf_window_fullscreen_disable && !conf_window_maximize_disable) {
		dwStyle = WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX |
			      WS_OVERLAPPED;
	} else {
		dwStyle = WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_OVERLAPPED;
	}
	if (conf_window_resize)
		dwStyle |= WS_THICKFRAME;

	/* フレームのサイズを取得する */
	nFrameAddWidth = GetSystemMetrics(SM_CXFIXEDFRAME) * 2;
	nFrameAddHeight = GetSystemMetrics(SM_CYCAPTION) +
					  GetSystemMetrics(SM_CYMENU) +
					  GetSystemMetrics(SM_CYFIXEDFRAME) * 2;

	/* ウィンドウのタイトルをUTF-8からUTF-16に変換する */
	MultiByteToWideChar(CP_UTF8, 0, conf_window_title, -1, wszTitle,
						TITLE_BUF_SIZE - 1);

	/* モニタの数を取得する */
	nMonitors = GetSystemMetrics(SM_CMONITORS);

	/* ウィンドウのサイズをコンフィグから取得する */
	if (conf_window_resize &&
		conf_window_default_width > 0 &&
		conf_window_default_height > 0)
	{
		nGameWidth = conf_window_default_width;
		nGameHeight = conf_window_default_height;
	}
	else
	{
		nGameWidth = conf_window_width;
		nGameHeight = conf_window_height;
	}

	/* ディスプレイのサイズを取得する */
	nVirtualScreenWidth = GetSystemMetrics(SM_CXVIRTUALSCREEN);
	nVirtualScreenHeight = GetSystemMetrics(SM_CYVIRTUALSCREEN);

#ifndef USE_DEBUGGER
	nWinWidth = nGameWidth + nFrameAddWidth;
	nWinHeight = nGameHeight + nFrameAddHeight;
#else
	nWinWidth = nGameWidth + nFrameAddWidth + DEBUGGER_PANEL_WIDTH;
	nWinHeight = nGameHeight + nFrameAddHeight;
#endif

	/* ディスプレイのサイズが足りない場合 */
	if (nVirtualScreenWidth < conf_window_width ||
		nVirtualScreenHeight < conf_window_height)
	{
#ifndef USE_DEBUGGER
		log_error(conv_utf16_to_utf8(get_ui_message(UIMSG_WIN_SMALL_DISPLAY)),
				  nVirtualScreenWidth, nVirtualScreenHeight);
		return FALSE;
#else
		nWinWidth = nVirtualScreenWidth;
		nWinHeight = nVirtualScreenHeight;
		nGameWidth = nWinWidth - DEBUGGER_PANEL_WIDTH;
		nGameHeight = nWinHeight;
#endif
	}

	/* マルチモニタでなければセンタリングする */
	if (nMonitors == 1)
	{
		nPosX = (nVirtualScreenWidth - nWinWidth) / 2;
		nPosY = (nVirtualScreenHeight - nWinHeight) / 2;
	}
	else
	{
		nPosX = CW_USEDEFAULT;
		nPosY = CW_USEDEFAULT;
	}

	/* メインウィンドウを作成する */
	hWndMain = CreateWindowEx(0,
							  wszWindowClassMain,
							  wszTitle,
							  dwStyle,
							  nPosX,
							  nPosY,
							  nWinWidth,
							  nWinHeight,
							  NULL,
							  NULL,
							  hInstance,
							  NULL);
	if (hWndMain == NULL)
	{
		log_api_error("CreateWindowEx");
		return FALSE;
	}

	/* ウィンドウのサイズを調整する */
	SetRectEmpty(&rc);
	rc.right = nGameWidth;
	rc.bottom = nGameHeight;
	AdjustWindowRectEx(
		&rc,
		dwStyle,
#ifdef USE_DEBUGGER
		TRUE,
#else
		conf_window_menubar,
#endif
		(DWORD)GetWindowLong(hWndMain, GWL_EXSTYLE));
	SetWindowPos(hWndMain,
				 NULL,
				 0,
				 0,
				 rc.right - rc.left,
				 rc.bottom - rc.top,
				 SWP_NOZORDER | SWP_NOMOVE);
	GetWindowRect(hWndMain, &rcWindow);

#ifndef USE_DEBUGGER
	hWndGame = hWndMain;

	/* ゲーム用メニューを作成する */
	if(conf_window_menubar)
		InitGameMenu();
#else
	/* ゲーム領域の子ウィンドウを作成する */
	ZeroMemory(&wcex, sizeof(wcex));
	wcex.cbSize			= sizeof(WNDCLASSEX);
	wcex.lpfnWndProc    = WndProc;
	wcex.hInstance      = hInstance;
	wcex.hbrBackground  = (HBRUSH)GetStockObject(conf_window_white ? WHITE_BRUSH : BLACK_BRUSH);
	wcex.lpszClassName  = wszWindowClassGame;
	if (!RegisterClassEx(&wcex))
		return FALSE;
	hWndGame = CreateWindowEx(0,
							  wszWindowClassGame,
							  NULL,
							  WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS,
							  0,
							  0,
							  nGameWidth,
							  nGameHeight,
							  hWndMain,
							  NULL,
							  hInstance,
							  NULL);
	if (hWndGame == NULL)
	{
		log_api_error("CreateWindowEx");
		return FALSE;
	}

	/* アクセラレータをロードする */
	hAccel = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDR_ACCEL));

	/* デバッガパネルを作成する */
	if (!InitDebuggerPanel(hWndMain, hWndGame, WndProc))
		return FALSE;
#endif

	/* ウィンドウを表示する */
	ShowWindow(hWndMain, nCmdShow);
	UpdateWindow(hWndMain);

	/* デバイスコンテキストを取得する */
	hWndDC = GetDC(hWndGame);
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
	pixels = NULL;
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
	BOOL bBreak;

	/* WM_PAINTでの描画を許可する */
	bRunFrameAllow = TRUE;

	/* ゲームループ */
	bBreak = FALSE;
	while (!bBreak)
	{
		/* イベントを処理する */
		if(!SyncEvents())
			break;	/* 閉じるボタンが押された */

		/* 次の描画までスリープする */
		if(!WaitForNextFrame())
			break;	/* 閉じるボタンが押された */

		/* フレームの開始時刻を取得する */
		dwStartTime = GetTickCount();

#if defined(USE_CAPTURE) || defined(USE_REPLAY)
		/* 入力のキャプチャ/エミュレートを行う */
		if (!capture_input())
			break;
#endif

		/* フレームを実行する */
		if (!RunFrame())
			bBreak = TRUE;

#if defined(USE_CAPTURE) || defined(USE_REPLAY)
		/* 出力のキャプチャを行う */
		if (!bDShowMode && !capture_output())
			bBreak = TRUE;
#endif
	}
}

/* フレームを実行する */
static BOOL RunFrame(void)
{
	int x, y, w, h;
	BOOL bRet;

	/* 実行許可前の場合 */
	if (!bRunFrameAllow)
		return TRUE;

	/* DirectShowで動画を再生中の場合は特別に処理する */
	if(bDShowMode)
	{
		/* ウィンドウイベントを処理する */
		if(!SyncEvents())
			return FALSE;

		/* @videoコマンドを実行する */
		if(!on_event_frame(&x, &y, &w, &h))
			return FALSE;

		return TRUE;
	}

	/* フレームの描画を開始する */
	if (bD3D)
		D3DStartFrame();
	else if (bOpenGL)
		opengl_start_rendering();

	/* フレームの実行と描画を行う */
	bRet = TRUE;
	if(!on_event_frame(&x, &y, &w, &h))
	{
		/* スクリプトの終端に達した */
		bRet = FALSE;
		bRunFrameAllow = FALSE;
	}

	/* フレームの描画を終了する */
	if (bD3D)
	{
		D3DEndFrame();
		assert(get_image_lock_count() == 0);
	}
	else if(bOpenGL)
	{
		opengl_end_rendering();
		SwapBuffers(hWndDC);
	}
	else
	{
		SyncBackImage(x, y, w, h);
	}

	return bRet;
}

/* ウィンドウにバックイメージを転送する */
static void SyncBackImage(int x, int y, int w, int h)
{
	if (w == 0 || h == 0)
		return;

	BitBlt(hWndDC, x + nOffsetX, y + nOffsetY, w, h, hBitmapDC, x, y, SRCCOPY);
}

/* キューにあるイベントを処理する */
static BOOL SyncEvents(void)
{
	/* DWORD dwStopWatchPauseStart; */
	MSG msg;

	/* イベント処理の開始時刻を求める */
	/* dwStopWatchPauseStart = GetTickCount(); */

	/* イベント処理を行う */
	while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
	{
		if (msg.message == WM_QUIT)
			return FALSE;
#ifdef USE_DEBUGGER
		if (PretranslateForDebugger(&msg))
			continue;
#endif
		if (!TranslateAccelerator(msg.hwnd, hAccel, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	/* イベント処理にかかった時間をストップウォッチから除外するようにする */
	/* dwStopWatchOffset += GetTickCount() - dwStopWatchPauseStart; */

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
		lap = end - dwStartTime;

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

	switch(message)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	case WM_SYSKEYDOWN:	/* Alt + Key */
#ifndef USE_DEBUGGER
		/* Alt + Enter */
		if (hWnd == NULL && (hWnd == hWndMain || hWnd == hWndGame))
		{
			if(wParam == VK_RETURN && (HIWORD(lParam) & KF_ALTDOWN))
			{
				if (!conf_window_fullscreen_disable)
				{
					SendMessage(hWndMain, WM_SYSCOMMAND,
								(WPARAM)SC_MAXIMIZE, (LPARAM)0);
				}
				return 0;
			}
		}
#endif
		/* Alt + F4 */
		if (hWnd == NULL && (hWnd == hWndMain || hWnd == hWndGame))
		{
			if(wParam == VK_F4)
			{
#ifndef USE_DEBUGGER
				if (MessageBox(hWnd,
							   get_ui_message(UIMSG_EXIT),
							   conv_utf8_to_utf16(conf_window_title),
							   MB_OKCANCEL) == IDOK)
#endif
				{
					DestroyWindow(hWnd);
					return 0;
				}
			}
		}
		break;
	case WM_CLOSE:
		if (hWnd != NULL && (hWnd == hWndMain || hWnd == hWndGame))
		{
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
		}
		break;
	case WM_LBUTTONDOWN:
		if (hWnd != NULL && hWnd == hWndGame)
		{
			on_event_mouse_press(
				MOUSE_LEFT,
				(int)((float)(LOWORD(lParam) - nOffsetX) / fMouseScale),
				(int)((float)(HIWORD(lParam) - nOffsetY) / fMouseScale));
			return 0;
		}
		break;
	case WM_LBUTTONUP:
		if (hWnd != NULL && hWnd == hWndGame)
		{
			on_event_mouse_release(
				MOUSE_LEFT,
				(int)((float)(LOWORD(lParam) - nOffsetX) / fMouseScale),
				(int)((float)(HIWORD(lParam) - nOffsetY) / fMouseScale));
			return 0;
		}
		break;
	case WM_RBUTTONDOWN:
		if (hWnd != NULL && hWnd == hWndGame)
		{
			on_event_mouse_press(
				MOUSE_RIGHT,
				(int)((float)(LOWORD(lParam) - nOffsetX) / fMouseScale),
				(int)((float)(HIWORD(lParam) - nOffsetY) / fMouseScale));
			return 0;
		}
		break;
	case WM_RBUTTONUP:
		if (hWnd != NULL && hWnd == hWndGame)
		{
			on_event_mouse_release(
				MOUSE_RIGHT,
				(int)((float)(LOWORD(lParam) - nOffsetX) / fMouseScale),
				(int)((float)(HIWORD(lParam) - nOffsetY) / fMouseScale));
			return 0;
		}
		break;
	case WM_KEYDOWN:
		if (hWnd != NULL && hWnd == hWndGame)
		{
			/* オートリピートの場合を除外する */
			if((HIWORD(lParam) & 0x4000) != 0)
				return 0;
			if((int)wParam == VK_ESCAPE && bFullScreen)
			{
				bNeedWindowed = TRUE;
				SendMessage(hWndMain, WM_SIZE, 0, 0);
				return 0;
			}
			if ((int)wParam == 'V')
			{
				if (!conf_tts_enable) {
					InitSAPI();
					conf_tts_enable = conf_tts_user;
					if (strcmp(get_system_locale(), "ja") == 0)
						SpeakSAPI(L"テキスト読み上げがオンです。");
					else
						SpeakSAPI(L"Text-to-speech is turned on.");
				} else {
					conf_tts_enable = 0;
					if (strcmp(get_system_locale(), "ja") == 0)
						SpeakSAPI(L"テキスト読み上げがオフです。");
					else
						SpeakSAPI(L"Text-to-speech is turned off.");
				}
				return 0;
			}
			kc = ConvertKeyCode((int)wParam);
			if(kc != -1)
				on_event_key_press(kc);
			return 0;
		}
		break;
	case WM_KEYUP:
		if (hWnd != NULL && (hWnd == hWndGame || hWnd == hWndMain))
		{
			kc = ConvertKeyCode((int)wParam);
			if(kc != -1)
				on_event_key_release(kc);
			return 0;
		}
		break;
	case WM_MOUSEMOVE:
		if (hWnd != NULL && hWnd == hWndGame)
		{
			on_event_mouse_move(
				(int)((float)(LOWORD(lParam) - nOffsetX) / fMouseScale),
				(int)((float)(HIWORD(lParam) - nOffsetY) / fMouseScale));
			return 0;
		}
		break;
	case WM_MOUSEWHEEL:
		if (hWnd != NULL && hWnd == hWndGame)
		{
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
		}
		break;
	case WM_KILLFOCUS:
		if (hWnd != NULL && (hWnd == hWndGame || hWnd == hWndMain))
		{
			on_event_key_release(KEY_CONTROL);
			return 0;
		}
		break;
	case WM_SYSCHAR:
		if (hWnd != NULL && (hWnd == hWndGame || hWnd == hWndMain))
		{
			return 0;
		}
		break;
	case WM_PAINT:
		if (hWnd != NULL && (hWnd == hWndGame || hWnd == hWndMain))
		{
			OnPaint(hWnd);
			return 0;
		}
		break;
	case WM_COMMAND:
		OnCommand(wParam, lParam);
		return 0;
	case WM_GRAPHNOTIFY:
		if(!DShowProcessEvent())
			bDShowMode = FALSE;
		break;
	case WM_SIZING:
		if (hWnd != NULL && hWnd == hWndMain)
		{
			if (conf_window_resize)
			{
				OnSizing((int)wParam, (LPRECT)lParam);
				return TRUE;
			}
			return FALSE;
		}
		break;
	case WM_SIZE:
		if (hWnd != NULL && hWnd == hWndMain)
		{
			OnSize();
			return 0;
		}
		break;
#ifndef USE_DEBUGGER
	/*
	 * デバッガでない場合だけウィンドウの最大化によるフルスクリーンを処理する
	 */
	case WM_SYSCOMMAND:
		/* Hook maximize and enter full screen mode. */
		if (wParam == SC_MAXIMIZE && !conf_window_fullscreen_disable)
		{
			bNeedFullScreen = TRUE;
			SendMessage(hWndMain, WM_SIZE, 0, 0);
			return 0;
		}

		/*
		 * Cancel Aero Snap:
		 *  - https://stackoverflow.com/questions/19661126/win32-prevent-window-snap
		 */
		if (wParam == (SC_MOVE | 2))
			wParam = SC_SIZE | 9;
		if ((wParam & 0xFFE0) == SC_SIZE && (wParam & 0x000F))
		{
			DWORD oldStyle = (DWORD)GetWindowLong(hWndMain, GWL_STYLE);
			PostMessage(hWndMain, WM_RESTOREORIGINALSTYLE, (WPARAM)GWL_STYLE, (LPARAM)oldStyle);
			SetWindowLong(hWndMain, GWL_STYLE, (LONG)(oldStyle & 0xFEFEFFFF));
			DefWindowProc(hWndMain, WM_SYSCOMMAND, wParam, lParam);
			return 0;
		}
		break;
	case WM_RESTOREORIGINALSTYLE:
		/* Restore Aero Snap. */
		if ((LONG)wParam == GWL_STYLE)
			SetWindowLong(hWndMain, GWL_STYLE, (LONG)lParam);
		return 0;
	case WM_NCLBUTTONDBLCLK:
		/* タイトルバーがダブルクリックされたとき */
		if(wParam == HTCAPTION)
		{
			if (!conf_window_fullscreen_disable)
			{
				bNeedFullScreen = TRUE;
				SendMessage(hWndMain, WM_SIZE, 0, 0);
			}
			return 0;
		}
		break;
#endif
#ifdef USE_DEBUGGER
	case WM_DPICHANGED:
		OnDpiChanged(hWnd, HIWORD(wParam), (LPRECT)lParam);
		return 0;
#endif
		default:
		break;
	}

	/* システムのウィンドウプロシージャにチェインする */
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
	case VK_LEFT:
		return KEY_LEFT;
	case VK_RIGHT:
		return KEY_RIGHT;
	case VK_ESCAPE:
		return KEY_ESCAPE;
	case 'C':
		return KEY_C;
	case 'S':
		return KEY_S;
	case 'L':
		return KEY_L;
	case 'H':
		return KEY_H;
	default:
		break;
	}
	return -1;
}

/* ウィンドウの内容を更新する */
static void OnPaint(HWND hWnd)
{
	HDC hDC;
	PAINTSTRUCT ps;

	hDC = BeginPaint(hWnd, &ps);
	if (hWnd == hWndGame)
		RunFrame();
	EndPaint(hWnd, &ps);

	UNUSED_PARAMETER(hDC);
}

/* WM_COMMANDを処理する */
static void OnCommand(WPARAM wParam, LPARAM lParam)
{
	UINT nID;

	nID = LOWORD(wParam);
	switch(nID)
	{
	case ID_QUIT:
		PostMessage(hWndMain,WM_CLOSE, 0, 0);
		break;
#ifndef USE_DEBUGGER
	case ID_FULLSCREEN:
		if (!conf_window_fullscreen_disable)
		{
			SendMessage(hWndMain, WM_SYSCOMMAND,
						(WPARAM)SC_MAXIMIZE, (LPARAM)0);
		}
		break;
	default:
		break;
#else
	default:
		OnCommandForDebugger(wParam, lParam);
		break;
#endif
	}
}

/* WM_SIZING */
static void OnSizing(int edge, LPRECT lpRect)
{
	RECT rcClient;
	float fPadX, fPadY, fWidth, fHeight, fAspect;
	int nOrigWidth, nOrigHeight;

	/* Get the rects before a size change. */
	GetWindowRect(hWndMain, &rcWindow);
	GetClientRect(hWndMain, &rcClient);

	/* Save the original window size. */
	nOrigWidth = rcWindow.right - rcWindow.left + 1;
	nOrigHeight = rcWindow.bottom - rcWindow.top + 1;

	/* Calc the paddings. */
	fPadX = (float)((rcWindow.right - rcWindow.left) -
		(rcClient.right - rcClient.left));
	fPadY = (float)((rcWindow.bottom - rcWindow.top) -
		(rcClient.bottom - rcClient.top));

	/* Calc the client size.*/
	fWidth = (float)(lpRect->right - lpRect->left + 1) - fPadX;
	fHeight = (float)(lpRect->bottom - lpRect->top + 1) - fPadY;

	/* Appky adjustments.*/
	if (conf_window_resize == 2)
	{
		fAspect = (float)conf_window_height / (float)conf_window_width;

		/* Adjust the window edges. */
		switch (edge)
		{
		case WMSZ_TOP:
			fWidth = fHeight / fAspect;
			lpRect->top = lpRect->bottom - (int)(fHeight + fPadY + 0.5);
			lpRect->right = lpRect->left + (int)(fWidth + fPadX + 0.5);
			break;
		case WMSZ_TOPLEFT:
			fHeight = fWidth * fAspect;
			lpRect->top = lpRect->bottom - (int)(fHeight + fPadY + 0.5);
			lpRect->left = lpRect->right - (int)(fWidth + fPadX + 0.5);
			break;
		case WMSZ_TOPRIGHT:
			fHeight = fWidth * fAspect;
			lpRect->top = lpRect->bottom - (int)(fHeight + fPadY + 0.5);
			lpRect->right = lpRect->left + (int)(fWidth + fPadX + 0.5);
			break;
		case WMSZ_BOTTOM:
			fWidth = fHeight / fAspect;
			lpRect->bottom = lpRect->top + (int)(fHeight + fPadY + 0.5);
			lpRect->right = lpRect->left + (int)(fWidth + fPadX + 0.5);
			break;
		case WMSZ_BOTTOMRIGHT:
			fHeight = fWidth * fAspect;
			lpRect->bottom = lpRect->top + (int)(fHeight + fPadY + 0.5);
			lpRect->right = lpRect->left + (int)(fWidth + fPadX + 0.5);
			break;
		case WMSZ_BOTTOMLEFT:
			fHeight = fWidth * fAspect;
			lpRect->bottom = lpRect->top + (int)(fHeight + fPadY + 0.5);
			lpRect->left = lpRect->right - (int)(fWidth + fPadX + 0.5);
			break;
		case WMSZ_LEFT:
			fHeight = fWidth * fAspect;
			lpRect->left = lpRect->right - (int)(fWidth + fPadX + 0.5);
			lpRect->bottom = lpRect->top + (int)(fHeight + fPadY + 0.5);
			break;
		case WMSZ_RIGHT:
			fHeight = fWidth * fAspect;
			lpRect->right = lpRect->left + (int)(fWidth + fPadX + 0.5);
			lpRect->bottom = lpRect->top + (int)(fHeight + fPadY + 0.5);
			break;
		default:
			/* Aero Snap? */
			fHeight = fWidth * fAspect;
			lpRect->bottom = lpRect->top + (int)(fHeight + fPadY + 0.5);
			lpRect->right = lpRect->left + (int)(fWidth + fPadX + 0.5);
			break;
		}
	}
	else
	{
#ifdef USE_DEBUGGER
		/* Apply the minimum window size. */
		if (fWidth < DEBUGGER_WIN_WIDTH_MIN)
			fWidth = DEBUGGER_WIN_WIDTH_MIN;
		if (fHeight < DEBUGGER_WIN_HEIGHT_MIN)
			fHeight = DEBUGGER_WIN_HEIGHT_MIN;

		/* Adjust the window edges. */
		switch (edge)
		{
		case WMSZ_TOP:
			lpRect->top = lpRect->bottom - (int)(fHeight + fPadY + 0.5);
			break;
		case WMSZ_TOPLEFT:
			lpRect->top = lpRect->bottom - (int)(fHeight + fPadY + 0.5);
			lpRect->left = lpRect->right - (int)(fWidth + fPadX + 0.5);
			break;
		case WMSZ_TOPRIGHT:
			lpRect->top = lpRect->bottom - (int)(fHeight + fPadY + 0.5);
			lpRect->right = lpRect->left + (int)(fWidth + fPadX + 0.5);
			break;
		case WMSZ_BOTTOM:
			lpRect->bottom = lpRect->top + (int)(fHeight + fPadY + 0.5);
			break;
		case WMSZ_BOTTOMRIGHT:
			lpRect->bottom = lpRect->top + (int)(fHeight + fPadY + 0.5);
			lpRect->right = lpRect->left + (int)(fWidth + fPadX + 0.5);
			break;
		case WMSZ_BOTTOMLEFT:
			lpRect->bottom = lpRect->top + (int)(fHeight + fPadY + 0.5);
			lpRect->left = lpRect->right - (int)(fWidth + fPadX + 0.5);
			break;
		case WMSZ_LEFT:
			lpRect->left = lpRect->right - (int)(fWidth + fPadX + 0.5);
			break;
		case WMSZ_RIGHT:
			lpRect->right = lpRect->left + (int)(fWidth + fPadX + 0.5);
			break;
		default:
			/* Aero Snap? */
			lpRect->bottom = lpRect->top + (int)(fHeight + fPadY + 0.5);
			lpRect->right = lpRect->left + (int)(fWidth + fPadX + 0.5);
			break;
		}
#endif
	}

	/* If there's a size change, update the screen size with the debugger panel size. */
	if (nOrigWidth != lpRect->right - lpRect->left + 1 ||
		nOrigHeight != lpRect->bottom - lpRect->top + 1)
	{
		UpdateScreenOffsetAndScale((int)(fWidth + 0.5f), (int)(fHeight + 0.5f));
	}
}

/* WM_SIZE */
static void OnSize(void)
{
	RECT rc;

	if(bNeedFullScreen)
	{
		HMONITOR monitor;
		MONITORINFOEX minfo;

		bNeedFullScreen = FALSE;
		bFullScreen = TRUE;

		monitor = MonitorFromWindow(hWndMain, MONITOR_DEFAULTTONEAREST);
		minfo.cbSize = sizeof(MONITORINFOEX);
		GetMonitorInfo(monitor, (LPMONITORINFO)&minfo);
		rc = minfo.rcMonitor;

		dwStyle = (DWORD)GetWindowLong(hWndMain, GWL_STYLE);

		SetMenu(hWndMain, NULL);
		SetWindowLong(hWndMain, GWL_STYLE, (LONG)(WS_POPUP | WS_VISIBLE));
		SetWindowLong(hWndMain, GWL_EXSTYLE, WS_EX_TOPMOST);
		SetWindowPos(hWndMain, NULL, 0, 0, 0, 0,
					 SWP_NOMOVE | SWP_NOSIZE |
					 SWP_NOZORDER | SWP_FRAMECHANGED);
		MoveWindow(hWndMain, 0, 0, rc.right, rc.bottom, TRUE);
		InvalidateRect(hWndMain, NULL, TRUE);
	}
	else if (bNeedWindowed)
	{
		bNeedWindowed = FALSE;
		bFullScreen = FALSE;
		if (hMenu != NULL)
			SetMenu(hWndMain, hMenu);
		SetWindowLong(hWndMain, GWL_STYLE, (LONG)dwStyle);
		SetWindowLong(hWndMain, GWL_EXSTYLE, 0);
		SetWindowPos(hWndMain, NULL, 0, 0, 0, 0,
					 SWP_NOMOVE | SWP_NOSIZE |
					 SWP_NOZORDER | SWP_FRAMECHANGED);
		MoveWindow(hWndMain, rcWindow.left, rcWindow.top,
				   rcWindow.right - rcWindow.left,
				   rcWindow.bottom - rcWindow.top, TRUE);
		InvalidateRect(hWndMain, NULL, TRUE);

		GetClientRect(hWndMain, &rc);
	}
	else
	{
		GetClientRect(hWndMain, &rc);
	}

	/* Update the screen offset and scale. */
	UpdateScreenOffsetAndScale(rc.right, rc.bottom);
}

/* スクリーンのオフセットとスケールを計算する */
static void UpdateScreenOffsetAndScale(int nClientWidth, int nClientHeight)
{
	float fAspect, fUseWidth, fUseHeight;

#ifdef USE_DEBUGGER
	int nDpi;
	int nDebugWidth;

	nDpi = Win11_GetDpiForWindow(hWndMain);

	/* If size and dpi are not changed, just return. */
	if (nClientWidth == nLastClientWidth && nClientHeight == nLastClientHeight && nLastDpi != nDpi)
		return;
	else
		nLastClientWidth = nClientWidth, nLastClientHeight = nClientHeight, nLastDpi = nDpi;

	nDebugWidth = MulDiv(DEBUGGER_PANEL_WIDTH, nDpi, 96);
	nClientWidth -= nDebugWidth;
#endif

	/* Calc the aspect ratio of the game. */
	fAspect = (float)conf_window_height / (float)conf_window_width;

	/* Set the height temporarily with "width-first". */
    fUseWidth = (float)nClientWidth;
    fUseHeight = fUseWidth * fAspect;
    fMouseScale = (float)nClientWidth / (float)conf_window_width;

	/* If height is not enough, determine width with "height-first". */
    if(fUseHeight > (float)nClientHeight)
	{
        fUseHeight = (float)nClientHeight;
        fUseWidth = (float)nClientHeight / fAspect;
        fMouseScale = (float)nClientHeight / (float)conf_window_height;
    }

	/* Calc the viewport origin. */
	nOffsetX = (int)((((float)nClientWidth - fUseWidth) / 2.0f) + 0.5);
	nOffsetY = (int)((((float)nClientHeight - fUseHeight) / 2.0f) + 0.5);

#ifdef USE_DEBUGGER
	/* Move the game window. */
	MoveWindow(hWndGame, 0, 0, nClientWidth, nClientHeight, TRUE);
	RearrangeDebuggerPanel(nClientWidth, nClientHeight);
#endif

	/* Update the screen offset and scale for drawing subsystem. */
	if (bD3D)
	{
		D3DResizeWindow(nOffsetX, nOffsetY, fMouseScale);
	}
	else if (bOpenGL)
	{
		opengl_set_screen(nOffsetX, nOffsetY,
						  (int)(fUseWidth + 0.5f),
						  (int)(fUseHeight + 0.5f));
	}
}

/* WM_DPICHANGED */
VOID OnDpiChanged(HWND hWnd, UNUSED(UINT nDpi), LPRECT lpRect)
{
	RECT rcClient;

	if (hWnd == hWndMain)
	{
		SetWindowPos(hWnd,
					   NULL,
					   lpRect->left,
					   lpRect->top,
					   lpRect->right - lpRect->left,
					   lpRect->bottom - lpRect->top,
					   SWP_NOZORDER | SWP_NOACTIVATE);
		GetClientRect(hWndMain, &rcClient);
		UpdateScreenOffsetAndScale(rcClient.right, rcClient.bottom);
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
		fprintf(pLogFile, "%s\n", buf);
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
		fprintf(pLogFile, "%s\n", buf);
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
		fprintf(pLogFile, "%s\n", buf);
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
 * イメージを暗くレンダリングする
 */
void render_image_dim(int dst_left, int dst_top,
					  struct image * RESTRICT src_image, int width, int height,
					  int src_left, int src_top)
{
	if (bD3D)
	{
		D3DRenderImageDim(dst_left, dst_top, src_image, width, height,
						  src_left, src_top);
	}
	else if (bOpenGL)
	{
		opengl_render_image_dim(dst_left, dst_top, src_image, width, height,
								src_left, src_top);
	}
	else
	{
		draw_image_dim(BackImage, dst_left, dst_top, src_image, width, height,
					   src_left, src_top);
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
#if defined(USE_CAPTURE)
	extern uint64_t cap_cur_time;
	*t = cap_cur_time;
#elif defined(USE_REPLAY)
	extern uint64_t sim_time;
	*t = sim_time;
#else
	*t = GetTickCount();
	dwStopWatchOffset = 0;
#endif
}

/*
 * タイマのラップをミリ秒単位で取得する
 */
int get_stop_watch_lap(stop_watch_t *t)
{
#if defined(USE_CAPTURE)
	extern uint64_t cap_cur_time;
	return (int32_t)(cap_cur_time - *t);
#elif defined(USE_REPLAY)
	extern uint64_t sim_time;
	return (int)(sim_time - *t);
#else
	DWORD dwCur = GetTickCount();
	return (int32_t)(dwCur - *t - dwStopWatchOffset);
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
	{
		bNeedFullScreen = TRUE;
		SendMessage(hWndMain, WM_SIZE, 0, 0);
	}
}

/*
 * フルスクリーンモードを終了する
 */
void leave_full_screen_mode(void)
{
	if (bFullScreen)
	{
		bNeedWindowed = TRUE;
		SendMessage(hWndMain, WM_SIZE, 0, 0);
	}
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

#if defined(USE_CAPTURE) || defined(USE_REPLAY)
/* ディレクトリを削除する */
static bool delete_directory(LPCWSTR pszDirName);

/*
 * ミリ秒の時刻を取得する
 */
uint64_t get_tick_count64(void)
{
	return GetTickCount64();
}

/*
 * 出力データのディレクトリを作り直す
 */
bool reconstruct_dir(const char *dir)
{
	if (!delete_directory(conv_utf8_to_utf16(dir))) {
		log_error("Failed to remove record directory.");
		return false;
	}
	if (!CreateDirectory(conv_utf8_to_utf16(dir), NULL)) {
		if (_access(dir, 0) != 0) {
			log_error("Failed to create record directory.");
			return false;
		}
	}
	return true;
}

/* ディレクトリを削除する */
static bool delete_directory(LPCWSTR pszDirName)
{
	wchar_t path[256];
	HANDLE hFind;
	WIN32_FIND_DATA wfd;

	/* ディレクトリの内容を取得する */
	_snwprintf(path, sizeof(path), L"%s\\*.*", pszDirName);
	hFind = FindFirstFile(path, &wfd);
	if(hFind == INVALID_HANDLE_VALUE)
		return true;

	/* 再帰的に削除する */
	do
	{
		if (wcscmp(wfd.cFileName, L".") == 0)
			continue;
		if (wcscmp(wfd.cFileName, L"..") == 0)
			continue;

		if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			_snwprintf(path, sizeof(path), L"%s\\%s", pszDirName,
				   wfd.cFileName);
			if (!delete_directory(path))
				return false;
		}
		else
		{
			_snwprintf(path, sizeof(path), L"%s\\%s", pszDirName,
				   wfd.cFileName);
			DeleteFile(path);
		}
    } while(FindNextFile(hFind, &wfd));

    FindClose(hFind);
    return true;
}
#endif

/*
 * Text-To-Speech
 */

/*
 * TTSによる読み上げを行う
 */
void speak_text(const char *text)
{
	wchar_t buf[4096];
	const wchar_t *s;
	wchar_t *d;

	/* 読み上げのキューに入っている文章をスキップする */
	if (text == NULL)
	{
		SpeakSAPI(NULL);
		return;
	}

	/* エスケープシーケンスを処理する */
	s = conv_utf8_to_utf16(text);
	d = &buf[0];
	while (*s) {
		/* エスケープシーケンスでない場合 */
		if (*s != '\\') {
			*d++ = *s++;
			continue;
		}

		/* エスケープシーケンスの場合 */
		switch (*(s + 1)) {
		case '\0':
			/* 文字列の末尾が'\\'なので無視する */
			s++;
			break;
		case 'n':
			/* "\\n"を処理する */
			s += 2;
			break;
		default:
			/* "\\.{.+}"のエスケープシーケンスをスキップする */
			if (*(s + 2) == '{') {
				s += 3;
				while (*s != '\0' && *s != '}')
					s++;
				s++;
			}
			break;
		}
	}
	*d = '\0';

	/* 読み上げる */
	SpeakSAPI(buf);
}
