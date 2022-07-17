/* -*- coding: utf-8-with-signature; indent-tabs-mode: t; tab-width: 4; c-basic-offset: 4; -*- */

/*
 * Suika 2
 * Copyright (C) 2001-2021, TABATA Keiichi. All rights reserved.
 */

/*
 * [Changes]
 *  2014-05-24 作成 (conskit)
 *  2016-05-29 作成 (suika)
 *  2017-11-07 フルスクリーンで解像度変更するように修正
 *  2022-06-08 デバッガ対応
 */

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif

#include <windows.h>
#include <io.h>

#include "suika.h"
#include "dsound.h"
#include "dsvideo.h"
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

/* UTF-8からSJISへの変換バッファサイズ */
#define NATIVE_MESSAGE_SIZE	(65536)

/* ウィンドウクラス名 */
static const char szWindowClass[] = "suika";

/* ウィンドウタイトル(ShiftJISに変換後) */
static char mbszTitle[TITLE_BUF_SIZE];

/* ウィンドウタイトル(UTF-16) */
static wchar_t wszTitle[TITLE_BUF_SIZE];

/* メッセージ変換バッファ */
static wchar_t wszMessage[NATIVE_MESSAGE_SIZE];

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
#ifdef USE_DEBUGGER
#endif

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

/* フルスクリーンモード時の描画オフセット */
static int nOffsetX;
static int nOffsetY;

/* DirectShowでビデオを再生中か */
static BOOL bDShowMode;

/* DirectShow再生中にクリックでスキップするか */
static BOOL bDShowSkippable;

/* UTF-8からSJISへの変換バッファ */
static char szNativeMessage[NATIVE_MESSAGE_SIZE];

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
static void CleanupApp(void);
static BOOL InitWindow(HINSTANCE hInstance, int nCmdShow);
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
static BOOL CreateBackImage(void);
static void SyncBackImage(int x, int y, int w, int h);
static BOOL OpenLogFile(void);

/*
 * WinMain
 */
int WINAPI WinMain(
	HINSTANCE hInstance,
	UNUSED(HINSTANCE hPrevInstance),
	UNUSED(LPSTR lpszCmd),
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
#endif

	/* 描画エンジンを初期化する */
	do {
		if (_access("no-direct3d.txt", 0) != 0)
		{
			/* Direct3Dを初期化する */
			if (D3DInitialize(hWndMain))
			{
				bD3D = TRUE;
				break;
			}
			log_info(conf_language == NULL ?
					 "Direct3Dはサポートされません。" :
					 "Direct3D is not supported.");
		}

		if (_access("no-opengl.txt", 0) != 0)
		{
			/* OpenGLを初期化する */
			if(InitOpenGL())
			{
				bOpenGL = TRUE;
				break;
			}
			log_info(conf_language == NULL ?
					 "OpenGLはサポートされません。" :
					 "OpenGL is not supported.");
		}

		/* Direct3DとOpenGLが利用できない場合はGDIを利用する */
	} while (0);

	/* DirectSoundを初期化する */
	if(!DSInitialize(hWndMain))
		return FALSE;

	if(!bD3D && !bOpenGL)
	{
		/* バックイメージを作成する */
		if(!CreateBackImage())
			return FALSE;
	}

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

	if (!bOpenGL)
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
}

/* ウィンドウを作成する */
static BOOL InitWindow(HINSTANCE hInstance, int nCmdShow)
{
	WNDCLASSEX wcex;
	RECT rc;
	DWORD style;
	int dw, dh, i, cch;

	/* ディスプレイのサイズが足りない場合 */
	if (GetSystemMetrics(SM_CXVIRTUALSCREEN) < conf_window_width ||
		GetSystemMetrics(SM_CYVIRTUALSCREEN) < conf_window_height)
	{
		MessageBox(NULL, conf_language == NULL ?
				   "ディスプレイのサイズが足りません。" :
				   "Display size too small.",
				   conf_language == NULL ? "エラー" : "Error",
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
	wcex.lpszClassName  = szWindowClass;
	wcex.hIconSm		= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SMALL));
	if (!RegisterClassEx(&wcex))
		return FALSE;

	/* ウィンドウのスタイルを決める */
	if (!conf_window_fullscreen_disable) {
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

#ifdef USE_DEBUGGER
	/* デバッガを使う場合、ウィンドウタイトルの先頭にProのタイトルを付ける */
	strcpy(mbszTitle, MSGBOX_TITLE);
	strcat(mbszTitle, " - ");
#else
	mbszTitle[0] = '\0';
#endif

	/* ウィンドウのタイトルをUTF-8からShiftJISに変換する */
	cch = MultiByteToWideChar(CP_UTF8, 0, conf_window_title, -1, wszTitle,
							  TITLE_BUF_SIZE - 1);
	wszTitle[cch] = L'\0';
	WideCharToMultiByte(CP_THREAD_ACP, 0, wszTitle, (int)wcslen(wszTitle),
						mbszTitle + strlen(mbszTitle),
						TITLE_BUF_SIZE - (int)strlen(mbszTitle) - 1,
						NULL, NULL);

	/* ウィンドウを作成する */
	hWndMain = CreateWindowEx(0, szWindowClass, mbszTitle, style,
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
					   FALSE,
#endif
					   (DWORD)GetWindowLong(hWndMain, GWL_EXSTYLE));
	SetWindowPos(hWndMain, NULL, 0, 0, rc.right - rc.left, rc.bottom - rc.top,
				 SWP_NOZORDER | SWP_NOMOVE);

#ifdef USE_DEBUGGER
	/* メニューを作成する */
	InitMenu(hWndMain);

	/* アクセラレータをロードする */
	hAccel = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDR_ACCEL));
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
		log_info(conf_language == NULL ?
				 "ChoosePixelFormat() の呼び出しに失敗しました。" :
				 "Failed to call ChoosePixelFormat()");
		return FALSE;
	}
	SetPixelFormat(hWndDC, pixelFormat, &pfd);

	/* OpenGLコンテキストを作成する */
	hGLRC = wglCreateContext(hWndDC);
	if (hGLRC == NULL)
	{
		log_info(conf_language == NULL ?
				 "wglCreateContext() の呼び出しに失敗しました。" :
				 "Failed to call wglCreateContext()");
		return FALSE;
	}
	wglMakeCurrent(hWndDC, hGLRC);

	/* wglCreateContextAttribsARB()へのポインタを取得する */
	wglCreateContextAttribsARB =
		(void *)wglGetProcAddress("wglCreateContextAttribsARB");
	if (wglCreateContextAttribsARB == NULL)
	{
		log_info(conf_language == NULL ?
				 "API wglCreateContextAttribsARB がみつかりません。" :
				 "API wglCreateContextAttribsARB not found.");
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
		log_info(conf_language == NULL ?
				 "仮想環境を検出しました。" :
				 "Detected virtual environment.");
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
			log_info(conf_language == NULL ?
					 "API %s がみつかりません。" :
					 "API %s not found.", APITable[i].name);
			wglMakeCurrent(NULL, NULL);
			wglDeleteContext(hGLRC);
			hGLRC = NULL;
			return FALSE;
		}
	}

	/* レンダラを初期化する */
	if (!init_opengl())
	{
		log_info(conf_language == NULL ?
				 "OpenGLの初期化に失敗しました。" :
				 "Failed to initialize OpenGL.");
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
	MSG msg;

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
		if (MessageBox(hWnd, conf_language == NULL ?
					   "終了しますか？" : "Quit?",
					   mbszTitle, MB_OKCANCEL) == IDOK)
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
		if(wParam == VK_ESCAPE && bFullScreen)
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
	int cx, cy;

#ifdef USE_DEBUGGER
	return;
#endif

	if(!bFullScreen)
	{
		bFullScreen = TRUE;

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

		nOffsetX = 0;
		nOffsetY = 0;
		SetWindowLong(hWndMain, GWL_STYLE, (LONG)(WS_CAPTION |
												  WS_SYSMENU |
												  WS_MINIMIZEBOX |
												  WS_MAXIMIZEBOX |
												  WS_OVERLAPPED));
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
	MessageBox(hWndMain, buf, conf_language == NULL ? "情報" : "Info",
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
	MessageBox(hWndMain, buf, conf_language == NULL ? "警告" : "Warning",
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
	MessageBox(hWndMain, buf, conf_language == NULL ? "エラー" : "Error",
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
	/* すでにオープンされていれば成功とする */
	if(pLogFile != NULL)
		return TRUE;

	/* オープンする */
	pLogFile = fopen(LOG_FILE, "w");
	if (pLogFile == NULL)
	{
		/* 失敗 */
		MessageBox(NULL,
				   conf_language == NULL ?
				   "ログファイルをオープンできません。" :
				   "Cannot open log file.",
				   conf_language == NULL ? "エラー" : "Error",
				   MB_OK | MB_ICONWARNING);
		return FALSE;
	}

	/* 成功 */
	return TRUE;
#endif
}

/*
 * UTF-8のメッセージをネイティブの文字コードに変換する
 */
const char *conv_utf8_to_native(const char *utf8_message)
{
	int cch;

	assert(utf8_message != NULL);

	/* UTF-8からワイド文字に変換する */
	cch = MultiByteToWideChar(CP_UTF8, 0, utf8_message, -1, wszMessage,
							  NATIVE_MESSAGE_SIZE - 1);
	wszMessage[cch] = L'\0';

	/* ワイド文字からSJISに変換する */
	WideCharToMultiByte(CP_THREAD_ACP, 0, wszMessage, -1, szNativeMessage,
						NATIVE_MESSAGE_SIZE - 1, NULL, NULL);

	return szNativeMessage;
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
		if (D3DLockTexture(width, height, pixels, locked_pixels, texture))
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
 * 画面にイメージをテンプレート指定でレンダリングする
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
 * セーブディレクトリを作成する
 */
bool make_sav_dir(void)
{
	CreateDirectory(SAVE_DIR, NULL);
	return true;
}

/*
 * データのディレクトリ名とファイル名を指定して有効なパスを取得する
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
	if (buf == NULL)
		return NULL;

	strcpy(buf, dir);
	if (strlen(dir) != 0)
		strcat(buf, "\\");
	strcat(buf, fname);

	return buf;
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
	*t = GetTickCount();
}

/*
 * タイマのラップをミリ秒単位で取得する
 */
int get_stop_watch_lap(stop_watch_t *t)
{
	DWORD dwCur = GetTickCount();
	return (int32_t)(dwCur - *t);
}

/*
 * 終了ダイアログを表示する
 */
bool exit_dialog(void)
{
	const char *pszMsg = conf_language == NULL ? "終了しますか？" : "Quit?";
	if (MessageBox(hWndMain, pszMsg, mbszTitle, MB_OKCANCEL) == IDOK)
		return true;
	return false;
}

/*
 * タイトルに戻るダイアログを表示する
 */
bool title_dialog(void)
{
	const char *pszMsg = conf_language == NULL ?
		"タイトルに戻りますか？" :
		"Are you sure you want to go to title?";
	if (MessageBox(hWndMain, pszMsg, mbszTitle, MB_OKCANCEL) == IDOK)
		return true;
	return false;
}

/*
 * 削除ダイアログを表示する
 */
bool delete_dialog(void)
{
	const char *pszMsg = conf_language == NULL ?
		"セーブデータを削除しますか？" :
		"Are you sure you want to delete the save data?";
	if (MessageBox(hWndMain, pszMsg, mbszTitle, MB_OKCANCEL) == IDOK)
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
	int cch1, cch2, cch3;

#ifdef USE_DEBUGGER
	/* デバッガを使う場合、先頭にProのタイトルを付ける */
	strcpy(mbszTitle, MSGBOX_TITLE);
	strcat(mbszTitle, " - ");
#else
	mbszTitle[0] = '\0';
#endif

	/* コンフィグのウィンドウタイトルをUTF-8からUTF-16に変換する */
	cch1 = MultiByteToWideChar(CP_UTF8, 0, conf_window_title, -1, wszTitle,
							   TITLE_BUF_SIZE - 1);
	cch1--;
	cch2 = MultiByteToWideChar(CP_UTF8, 0, " | ", -1, wszTitle + cch1,
							   TITLE_BUF_SIZE - cch1 - 1);
	cch2--;
	cch3 = MultiByteToWideChar(CP_UTF8, 0, get_chapter_name(), -1,
							   wszTitle + cch1 + cch2,
							   TITLE_BUF_SIZE - cch1 - cch2 - 1);
	cch3--;
	wszTitle[cch1 + cch2 + cch3] = L'\0';

	/* UTF-16から実行環境の文字コードに変換する */
	WideCharToMultiByte(CP_THREAD_ACP, 0, wszTitle, (int)wcslen(wszTitle),
						mbszTitle + strlen(mbszTitle),
						TITLE_BUF_SIZE - (int)strlen(mbszTitle) - 1,
						NULL, NULL);

	/* ウィンドウのタイトルを設定する */
	SetWindowText(hWndMain, mbszTitle);
}
