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
 */

#define _CRT_SECURE_NO_WARNINGS
#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#include "suika.h"
#include "dsound.h"
#include "dsvideo.h"
#include "resource.h"

#ifdef USE_DIRECT3D
#include "d3drender.h"
#endif

#ifdef SSE_VERSIONING
#include "x86.h"
#endif

/* VC6対応 */
#ifndef WM_MOUSEWHEEL
#define WM_MOUSEWHEEL	0x020A
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

/* Windowsオブジェクト */
static HWND hWndMain;
static HDC hWndDC;
#ifndef USE_DIRECT3D
static HDC hBitmapDC;
static HBITMAP hBitmap;
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

/* UTF-8からSJISへの変換バッファ */
static char szNativeMessage[NATIVE_MESSAGE_SIZE];

/* 前方参照 */
static BOOL InitApp(HINSTANCE hInstance, int nCmdShow);
static void CleanupApp(void);
static BOOL OpenLogFile(void);
static BOOL InitWindow(HINSTANCE hInstance, int nCmdShow);
static void GameLoop(void);
static BOOL SyncEvents(void);
static BOOL WaitForNextFrame(void);
static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam,
								LPARAM lParam);
static int ConvertKeyCode(int nVK);
static void ToggleFullScreen(void);
static void ChangeDisplayMode(void);
static void ResetDisplayMode(void);
static void OnPaint(void);
#ifndef USE_DIRECT3D
static BOOL CreateBackImage(void);
static void SyncBackImage(int x, int y, int w, int h);
#endif

/*
 * デバッガを使う場合
 */
#ifdef USE_DEBUGGER
/* バージョン文字列 */
static char szVersion[] =
	"Suika Studio 0.6 (under development)\n"
	"Copyright (c) 2022, LUXION SOFT. All rights reserved.";

/* 実行状態 */
BOOL bRunning;

/* メニュー */
static HMENU hMenu;

/* デバッガウィンドウ */
static HWND hWndDebug;

/* デバッガウィンドウのコンポーネント */
static HWND hWndBtnResume;
static HWND hWndBtnNext;
static HWND hWndBtnPause;
static HWND hWndLabelScript;
static HWND hWndTextboxScript;
static HWND hWndBtnChangeScript;
static HWND hWndLabelLine;
static HWND hWndTextboxLine;
static HWND hWndBtnChangeLine;
static HWND hWndLabelCommand;
static HWND hWndTextboxCommand;
static HWND hWndListbox;

/* ボタンが押下されたか */
static BOOL bResumePressed;
static BOOL bNextPressed;
static BOOL bPausePressed;
static BOOL bChangeScriptPressed;
static BOOL bChangeLinePressed;
static BOOL bReadPressed;
static BOOL bWritePressed;

/* 前方参照 */
static VOID InitMenu(void);
static BOOL InitDebugger(HINSTANCE hInstance, int nCmdShow);
static LRESULT CALLBACK WndProcDebug(HWND hWnd, UINT message, WPARAM wParam,
									 LPARAM lParam);
static VOID OnClickListBox(void);
#endif

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

#ifndef USE_DIRECT3D
	/* Sleep()の分解能を設定する */
	timeBeginPeriod(1);
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

#ifndef USE_DIRECT3D
	/* Sleep()の分解能を元に戻す */
	timeEndPeriod(1);
#endif

	return result;
}

/* 基盤レイヤの初期化処理を行う */
static BOOL InitApp(HINSTANCE hInstance, int nCmdShow)
{
#ifdef SSE_VERSIONING
	/* ベクトル命令の対応を確認する */
	x86_check_cpuid_flags();
#endif

	/* COMの初期化を行う */
	CoInitialize(0);

	/* ログファイルをオープンする */
	if(!OpenLogFile())
		return FALSE;

	/* パッケージの初期化処理を行う */
	if(!init_file())
		return FALSE;

	/* コンフィグの初期化処理を行う */
	if(!init_conf())
		return FALSE;

	/* ウィンドウを作成する */
	if(!InitWindow(hInstance, nCmdShow))
		return FALSE;

#ifdef USE_DIRECT3D
	/* Direct3Dを初期化する */
	if(!D3DInitialize(hWndMain))
		return FALSE;
#endif

	/* DirectSoundを初期化する */
	if(!DSInitialize(hWndMain))
		return FALSE;

#ifndef USE_DIRECT3D 
	/* バックイメージを作成する */
	if(!CreateBackImage())
		return FALSE;
#endif

#ifdef USE_DEBUGGER
	/* デバッガを初期化する */
	if(!InitDebugger(hInstance, nCmdShow))
		return FALSE;
#endif

	return TRUE;
}

/* 基盤レイヤの終了処理を行う */
static void CleanupApp(void)
{
	/* フルスクリーンモードであれば解除する */
	if (bFullScreen)
		ToggleFullScreen();

	/* ウィンドウのデバイスコンテキストを破棄する */
	if(hWndDC != NULL)
		ReleaseDC(hWndMain, hWndDC);

#ifndef USE_DIRECT3D
	/* バックイメージのビットマップを破棄する */
	if(hBitmap != NULL)
		DeleteObject(hBitmap);

	/* バックイメージのデバイスコンテキストを破棄する */
	if(hBitmapDC != NULL)
		DeleteDC(hBitmapDC);

	/* バックイメージを破棄する */
	if(BackImage != NULL)
		destroy_image(BackImage);
#endif

#ifdef USE_DIRECT3D
	D3DCleanup();
#endif

	/* DirectSoundの終了処理を行う */
	DSCleanup();

	/* コンフィグの終了処理を行う */
	cleanup_conf();

	/* ログファイルをクローズする */
	if(pLogFile != NULL)
		fclose(pLogFile);
}

/* ログをオープンする */
static BOOL OpenLogFile(void)
{
	if (pLogFile == NULL)
	{
		pLogFile = fopen(LOG_FILE, "w");
		if (pLogFile == NULL)
		{
			MessageBox(NULL,
					   conf_language == NULL ?
					   "ログファイルをオープンできません。" :
					   "Cannot open log file.",
					   conf_language == NULL ? "エラー" : "Error",
					   MB_OK | MB_ICONWARNING);
			return FALSE;
		}
	}
	return TRUE;
}

/* ウィンドウを作成する */
static BOOL InitWindow(HINSTANCE hInstance, int nCmdShow)
{
	wchar_t wszTitle[TITLE_BUF_SIZE];
	WNDCLASSEX wcex;
	RECT rc;
	DWORD style;
	int dw, dh, i, cch;

	/* ディスプレイのサイズが足りない場合 */
	if(GetSystemMetrics(SM_CXVIRTUALSCREEN) < conf_window_width ||
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
	if(!RegisterClassEx(&wcex))
		return FALSE;

	/* ウィンドウのスタイルを決める */
	style = WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX |
			WS_OVERLAPPED;

	/* フレームのサイズを取得する */
	dw = GetSystemMetrics(SM_CXFIXEDFRAME) * 2;
	dh = GetSystemMetrics(SM_CYCAPTION) +
		 GetSystemMetrics(SM_CYMENU) +
		 GetSystemMetrics(SM_CYFIXEDFRAME) * 2;

	/* ウィンドウのタイトルをUTF-8からShiftJISに変換する */
	cch = MultiByteToWideChar(CP_UTF8, 0, conf_window_title, -1, wszTitle,
							  TITLE_BUF_SIZE - 1);
	wszTitle[cch] = L'\0';
	WideCharToMultiByte(CP_THREAD_ACP, 0, wszTitle, (int)wcslen(wszTitle),
						mbszTitle, TITLE_BUF_SIZE - 1, NULL, NULL);

	/* ウィンドウを作成する */
	hWndMain = CreateWindowEx(0, szWindowClass, mbszTitle, style,
							  (int)CW_USEDEFAULT, (int)CW_USEDEFAULT,
							  conf_window_width + dw, conf_window_height + dh,
							  NULL, NULL, hInstance, NULL);
	if(!hWndMain)
		return FALSE;

	/* ウィンドウのサイズを調整する(for Windows 10) */
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
	InitMenu();

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

#ifndef USE_DIRECT3D
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
#endif

/* ゲームループを実行する */
static void GameLoop(void)
{
	int x, y, w, h;
	BOOL bBreak;

	/* 最初にイベントを処理してしまう */
	if(!SyncEvents())
		return;

#if 1
	/* 最初のフレームの開始時刻を取得する */
	dwStartTime = GetTickCount();
#endif

	while(TRUE)
	{
		/* DirectShowで動画を再生中の場合 */
		if(bDShowMode)
		{
			/* イベントを処理する */
			if(!SyncEvents())
				break;

			continue;
		}

#ifdef USE_DIRECT3D
		/* フレームの描画を開始する */
		D3DStartFrame();
#else
		/* バックイメージのロックを行う */
		lock_image(BackImage);
#endif

		/* 描画を行う */
		bBreak = FALSE;
		if(!on_event_frame(&x, &y, &w, &h))
		{
			/* スクリプトの終端に達した */
			bBreak = TRUE;
		}

#ifdef USE_DIRECT3D
		/* フレームの描画を終了する */
		D3DEndFrame();
#else
		/* バックイメージのアンロックを行う */
		unlock_image(BackImage);

		/* 描画を反映する */
		if(w !=0 && h !=0)
			SyncBackImage(x, y, w, h);
#endif

		if(bBreak)
			break;

		/* イベントを処理する */
		if(!SyncEvents())
			break;

		/* 次の描画までスリープする */
#ifndef USE_DIRECT3D
		if(!WaitForNextFrame())
			break;	/* 閉じるボタンが押された */

		/* 次のフレームの開始時刻を取得する */
		dwStartTime = GetTickCount();
#endif
	}
}

#ifndef USE_DIRECT3D
/* ウィンドウにバックイメージを転送する */
static void SyncBackImage(int x, int y, int w, int h)
{
	BitBlt(hWndDC, x + nOffsetX, y + nOffsetY, w, h, hBitmapDC, x, y, SRCCOPY);
}
#endif

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
	DWORD end, lap, wait;

	/* 次のフレームの開始時刻になるまでイベント処理とスリープを行う */
	do {
		/* イベントがある場合は処理する */
		if(!SyncEvents())
			return FALSE;

		/* 経過時刻を取得する */
		end = GetTickCount();
		lap = dwStartTime - end;

		/* 次のフレームの開始時刻になった場合はスリープを終了する */
		if (lap > FRAME_MILLI) {
			dwStartTime = end;
			break;
		}

		/* スリープする時間を求める */
		wait = (FRAME_MILLI - lap > SLEEP_MILLI) ? SLEEP_MILLI :
		    FRAME_MILLI - lap;

		/* スリープする */
		Sleep(wait);
	} while(wait > 0);

	return TRUE;
}

/* ウィンドウプロシージャ */
static LRESULT CALLBACK WndProc(HWND hWnd,
								UINT message,
								WPARAM wParam,
								LPARAM lParam)
{
	int kc;

#ifdef USE_DEBUGGER
	/* デバッグウィンドウと子ウィンドウ、あるいはWM_COMMANDの場合 */
	if(hWnd == hWndDebug || hWnd == hWndBtnResume || hWnd == hWndBtnNext ||
	   hWnd == hWndBtnPause || hWnd == hWndLabelScript ||
	   hWnd == hWndTextboxScript || hWnd == hWndBtnChangeScript ||
	   hWnd == hWndLabelLine || hWnd == hWndTextboxLine ||
	   hWnd == hWndBtnChangeLine || hWnd == hWndLabelCommand ||
	   hWnd == hWndTextboxCommand || hWnd == hWndListbox ||
	   message == WM_COMMAND)
		return WndProcDebug(hWnd, message, wParam, lParam);
#endif

	switch(message)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	case WM_SYSKEYDOWN:
		if(wParam == VK_RETURN && (HIWORD(lParam) & KF_ALTDOWN))
		{
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
#endif
		if (MessageBox(hWnd, conf_language == NULL ?
					   "終了しますか？" : "Quit?",
					   mbszTitle, MB_OKCANCEL) == IDOK)
			DestroyWindow(hWnd);
		return 0;
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
	case WM_SYSCHAR:
		return 0;
	case WM_NCLBUTTONDBLCLK:
		if(wParam == HTCAPTION)
		{
			ToggleFullScreen();
			return 0;
		}
		break;
	case WM_SYSCOMMAND:
		if(wParam == SC_MAXIMIZE)
		{
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

#ifdef USE_DIRECT3D
		D3DSetDisplayOffset(nOffsetX, nOffsetY);
#endif
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

#ifdef USE_DIRECT3D
		D3DSetDisplayOffset(0, 0);
#endif
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
#ifndef USE_DIRECT3D
	BitBlt(hDC,
		   ps.rcPaint.left,
		   ps.rcPaint.top,
		   ps.rcPaint.right - ps.rcPaint.left,
		   ps.rcPaint.bottom - ps.rcPaint.top,
		   hBitmapDC,
		   ps.rcPaint.left - nOffsetX,
		   ps.rcPaint.top - nOffsetY,
		   SRCCOPY);
#else
	UNUSED_PARAMETER(hDC);
#endif
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

	va_start(ap, s);
	if(pLogFile != NULL)
	{
		/* メッセージボックスを表示する */
		vsnprintf(buf, sizeof(buf), s, ap);
		MessageBox(hWndMain, buf, conf_language == NULL ? "情報" : "Info",
				   MB_OK | MB_ICONINFORMATION);

		/* ファイルへ出力する */
		fprintf(pLogFile, buf);
		fflush(pLogFile);
		if(ferror(pLogFile))
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
	if(pLogFile != NULL)
	{
		/* メッセージボックスを表示する */
		vsnprintf(buf, sizeof(buf), s, ap);
		MessageBox(hWndMain, buf, conf_language == NULL ? "警告" : "Warning",
				   MB_OK | MB_ICONWARNING);

		/* ファイルへ出力する */
		fprintf(pLogFile, buf);
		fflush(pLogFile);
		if(ferror(pLogFile))
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
	char buf[LOG_BUF_SIZE];
	va_list ap;

	va_start(ap, s);
	if(pLogFile != NULL)
	{
		/* メッセージボックスを表示する */
		vsnprintf(buf, sizeof(buf), s, ap);
		MessageBox(hWndMain, buf, conf_language == NULL ? "エラー" : "Error",
				   MB_OK | MB_ICONERROR);

		/* ファイルへ出力する */
		fprintf(pLogFile, buf);
		fflush(pLogFile);
		if(ferror(pLogFile))
			return false;
	}
	va_end(ap);
	return true;
}

/*
 * UTF-8のメッセージをネイティブの文字コードに変換する
 */
const char *conv_utf8_to_native(const char *utf8_message)
{
	wchar_t wszMessage[NATIVE_MESSAGE_SIZE];
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
 * テクスチャをロックする
 */
bool lock_texture(int width, int height, pixel_t *pixels,
				  pixel_t **locked_pixels, void **texture)
{
#ifdef USE_DIRECT3D
	if (!D3DLockTexture(width, height, pixels, locked_pixels, texture))
		return false;

	return true;
#else
	assert(*locked_pixels == NULL);

	UNUSED_PARAMETER(width);
	UNUSED_PARAMETER(height);
	UNUSED_PARAMETER(texture);

	*locked_pixels = pixels;

	return true;
#endif
}

/*
 * テクスチャをアンロックする
 */
void unlock_texture(int width, int height, pixel_t *pixels,
					pixel_t **locked_pixels, void **texture)
{
#ifdef USE_DIRECT3D
	D3DUnlockTexture(width, height, pixels, locked_pixels, texture);
#else
	assert(*locked_pixels != NULL);

	UNUSED_PARAMETER(width);
	UNUSED_PARAMETER(height);
	UNUSED_PARAMETER(pixels);
	UNUSED_PARAMETER(texture);

	*locked_pixels = NULL;
#endif
}

/*
 * テクスチャを破棄する
 */
void destroy_texture(void *texture)
{
#ifdef USE_DIRECT3D
	D3DDestroyTexture(texture);
#else
	UNUSED_PARAMETER(texture);
#endif
}

/*
 * イメージをレンダリングする
 */
void render_image(int dst_left, int dst_top, struct image * RESTRICT src_image,
                  int width, int height, int src_left, int src_top, int alpha,
                  int bt)
{
#ifdef USE_DIRECT3D
	D3DRenderImage(dst_left, dst_top, src_image, width, height,
				   src_left, src_top, alpha, bt);
#else
	draw_image(BackImage, dst_left, dst_top, src_image, width, height,
			   src_left, src_top, alpha, bt);
#endif
}

/*
 * イメージをマスク描画でレンダリングする
 */
void render_image_mask(int dst_left, int dst_top,
                       struct image * RESTRICT src_image,
                       int width, int height, int src_left, int src_top,
                       int mask)
{
#ifdef USE_DIRECT3D
	D3DRenderImageMask(dst_left, dst_top, src_image, width, height,
					   src_left, src_top, mask);
#else
	draw_image_mask(BackImage, dst_left, dst_top, src_image, width, height,
					src_left, src_top, mask);
#endif
}

/*
 * 画面をクリアする
 */
void render_clear(int left, int top, int width, int height, pixel_t color)
{
#ifdef USE_DIRECT3D
	D3DRenderClear(left, top, width, height, color);
#else
	clear_image_color_rect(BackImage, left, top, width, height, color);
#endif
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
	if (MessageBox(hWndMain, pszMsg, mbszTitle, MB_OKCANCEL)
		== IDOK)
		return true;
	return false;
}

/*
 * ビデオを再生する
 */
bool play_video(const char *fname)
{
	char *path;

	path = make_valid_path(MOV_DIR, fname);

	/* イベントループをDirectShow再生モードに設定する */
	bDShowMode = TRUE;

	/* ビデオの再生を開始する */
	BOOL ret = DShowPlayVideo(hWndMain, path);
	if(!ret)
		bDShowMode = FALSE;

	free(path);
	return ret;
}

/*
 * デバッガの実装
 */
#ifdef USE_DEBUGGER

/* メニューを作成する */
static VOID InitMenu(void)
{
	BOOL bEnglish = conf_language == NULL ? FALSE : TRUE;
	HMENU hMenuFile = CreatePopupMenu();
	HMENU hMenuScript = CreatePopupMenu();
	HMENU hMenuHelp = CreatePopupMenu();
    MENUITEMINFO mi;

	/* メニューを作成する */
	hMenu = CreateMenu();

	/* 1階層目を作成する準備を行う */
	ZeroMemory(&mi, sizeof(MENUITEMINFO));
	mi.cbSize = sizeof(MENUITEMINFO);
	mi.fMask = MIIM_TYPE | MIIM_SUBMENU;
	mi.fType = MFT_STRING;
	mi.fState = MFS_ENABLED;

	/* ファイル(F)を作成する */
	mi.hSubMenu = hMenuFile;
	mi.dwTypeData = bEnglish ? "File(&F)": "ファイル(&F)";
	InsertMenuItem(hMenu, 0, TRUE, &mi);

	/* スクリプト(S)を作成する */
	mi.hSubMenu = hMenuScript;
	mi.dwTypeData = bEnglish ? "Script(&S)": "スクリプト(&S)";
	InsertMenuItem(hMenu, 1, TRUE, &mi);

	/* ヘルプ(H)を作成する */
	mi.hSubMenu = hMenuHelp;
	mi.dwTypeData = bEnglish ? "Help(&H)": "ヘルプ(&H)";
	InsertMenuItem(hMenu, 2, TRUE, &mi);

	/* 2階層目を作成する準備を行う */
	mi.fMask = MIIM_TYPE | MIIM_ID;

	/* 終了(Q)を作成する */
	mi.wID = ID_QUIT;
	mi.dwTypeData = bEnglish ? "Quit(&Q)\tAlt+Q" : "終了(&Q)\tAlt+Q";
	InsertMenuItem(hMenuFile, 0, TRUE, &mi);

	/* 続ける(C)を作成する */
	mi.wID = ID_RESUME;
	mi.dwTypeData = bEnglish ? "Resume(&R)\tAlt+R" : "続ける(&R)\tAlt+R";
	InsertMenuItem(hMenuScript, 0, TRUE, &mi);

	/* 次へ(N)を作成する */
	mi.wID = ID_NEXT;
	mi.dwTypeData = bEnglish ? "Next(&N)\tAlt+N" : "次へ(&N)\tAlt+N";
	InsertMenuItem(hMenuScript, 1, TRUE, &mi);

	/* 停止(P)を作成する */
	mi.wID = ID_PAUSE;
	mi.dwTypeData = bEnglish ? "Pause(&P)\tAlt+P" : "停止(&P)\tAlt+P";
	InsertMenuItem(hMenuScript, 2, TRUE, &mi);
	EnableMenuItem(hMenu, ID_PAUSE, MF_GRAYED);

	/* バージョン(V)を作成する */
	mi.wID = ID_VERSION;
	mi.dwTypeData = bEnglish ? "Version(&V)" : "バージョン(&V)\tAlt+V";
	InsertMenuItem(hMenuHelp, 0, TRUE, &mi);

	/* メニューをセットする */
	SetMenu(hWndMain, hMenu);
}

/* デバッガを初期化する */
static BOOL InitDebugger(HINSTANCE hInstance, int nCmdShow)
{
	const char szWndClass[] = "suikadebug";
	WNDCLASSEX wcex;
	RECT rc;
	DWORD style;
	int dw, dh;
	BOOL bEnglish;
	const int WIN_WIDTH = 440;
	const int WIN_HEIGHT = 720;

	bEnglish = conf_language == NULL ? FALSE : TRUE;

	/* ウィンドウクラスを登録する */
	wcex.cbSize			= sizeof(WNDCLASSEX);
	wcex.style          = 0;
	wcex.lpfnWndProc    = WndProc;
	wcex.cbClsExtra     = 0;
	wcex.cbWndExtra     = 0;
	wcex.hInstance      = hInstance;
	wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SUIKA));
	wcex.hCursor        = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground  = (HBRUSH)COLOR_BACKGROUND + 1;
	wcex.lpszMenuName   = NULL;
	wcex.lpszClassName  = szWndClass;
	wcex.hIconSm		= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SMALL));
	if(!RegisterClassEx(&wcex))
		return FALSE;

	/* ウィンドウのスタイルを決める */
	style = WS_CAPTION | WS_OVERLAPPED;

	/* フレームのサイズを取得する */
	dw = GetSystemMetrics(SM_CXFIXEDFRAME) * 2;
	dh = GetSystemMetrics(SM_CYCAPTION) +
		 GetSystemMetrics(SM_CYFIXEDFRAME) * 2;

	/* メインウィンドウの矩形を取得する */
	GetWindowRect(hWndMain, &rc);

	/* ウィンドウを作成する */
	hWndDebug = CreateWindowEx(0, szWndClass, bEnglish ?
							   "Stopped - Suika Studio" :
							   "停止中 - Suika Studio",
							   style,
							   rc.right + 10, rc.top,
							   WIN_WIDTH + dw, WIN_HEIGHT + dh,
							   NULL, NULL, GetModuleHandle(NULL), NULL);
	if(!hWndDebug)
		return FALSE;

	/* ウィンドウのサイズを調整する(for Windows 10) */
	SetRectEmpty(&rc);
	rc.right = WIN_WIDTH;
	rc.bottom = WIN_HEIGHT;
	AdjustWindowRectEx(&rc, (DWORD)GetWindowLong(hWndDebug, GWL_STYLE), FALSE,
					   (DWORD)GetWindowLong(hWndDebug, GWL_EXSTYLE));
	SetWindowPos(hWndDebug, NULL, 0, 0, rc.right - rc.left, rc.bottom - rc.top,
				 SWP_NOZORDER | SWP_NOMOVE);

	/* 英語モードかどうかチェックする */
	bEnglish = conf_language != NULL;

	/* 続けるボタンを作成する */
	hWndBtnResume = CreateWindow(
		"BUTTON",
		bEnglish ? "Resume" : "続ける",
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
		10, 10, 100, 80,
		hWndDebug, (HMENU)ID_RESUME,
		(HINSTANCE)GetWindowLongPtr(hWndDebug, GWLP_HINSTANCE), NULL);

	/* 次へボタンを作成する */
	hWndBtnNext = CreateWindow(
		"BUTTON",
		bEnglish ? "Next" : "次へ",
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
		120, 10, 100, 80,
		hWndDebug, (HMENU)ID_NEXT,
		(HINSTANCE)GetWindowLongPtr(hWndDebug, GWLP_HINSTANCE), NULL);

	/* 停止ボタンを作成する */
	hWndBtnPause = CreateWindow(
		"BUTTON",
		bEnglish ? "(Paused)" : "(停止中)",
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
		330, 10, 100, 80,
		hWndDebug, (HMENU)ID_PAUSE,
		(HINSTANCE)GetWindowLongPtr(hWndDebug, GWLP_HINSTANCE), NULL);
	EnableWindow(hWndBtnPause, FALSE);

	/* スクリプトラベルを作成する */
	hWndLabelScript = CreateWindow(
		"STATIC",
		bEnglish ? "Script:" : "スクリプト:",
		WS_VISIBLE | WS_CHILD,
		10, 110, 100, 30,
		hWndDebug, 0,
		(HINSTANCE)GetWindowLongPtr(hWndDebug, GWLP_HINSTANCE), NULL);

	/* スクリプト名のテキストボックスを作成する */
	hWndTextboxScript = CreateWindow(
		"EDIT",
		NULL,
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | WS_BORDER,
		10, 140, 300, 30,
		hWndDebug, 0,
		(HINSTANCE)GetWindowLongPtr(hWndDebug, GWLP_HINSTANCE), NULL);

	/* スクリプトの変更ボタンを作成する */
	hWndBtnChangeScript = CreateWindow(
		"BUTTON",
		bEnglish ? "Change" : "変更",
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
		320, 140, 80, 30,
		hWndDebug, (HMENU)ID_CHANGE_SCRIPT,
		(HINSTANCE)GetWindowLongPtr(hWndDebug, GWLP_HINSTANCE), NULL);

	/* 行番号ラベルを作成する */
	hWndLabelLine = CreateWindow(
		"STATIC",
		bEnglish ? "Line:" : "行番号:",
		WS_VISIBLE | WS_CHILD,
		10, 190, 100, 30,
		hWndDebug, 0,
		(HINSTANCE)GetWindowLongPtr(hWndDebug, GWLP_HINSTANCE), NULL);

	/* 行番号のテキストボックスを作成する */
	hWndTextboxLine = CreateWindow(
		"EDIT",
		NULL,
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | WS_BORDER,
		10, 220, 80, 30,
		hWndDebug, 0,
		(HINSTANCE)GetWindowLongPtr(hWndDebug, GWLP_HINSTANCE), NULL);

	/* 行番号の変更ボタンを作成する */
	hWndBtnChangeLine = CreateWindow(
		"BUTTON",
		bEnglish ? "Change" : "変更",
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
		100, 220, 80, 30,
		hWndDebug, (HMENU)ID_CHANGE_LINE,
		(HINSTANCE)GetWindowLongPtr(hWndDebug, GWLP_HINSTANCE), NULL);

	/* コマンドのラベルを作成する */
	hWndLabelCommand = CreateWindow(
		"STATIC",
		bEnglish ? "Command:" : "コマンド:",
		WS_VISIBLE | WS_CHILD,
		10, 270, 100, 30,
		hWndDebug, 0,
		(HINSTANCE)GetWindowLongPtr(hWndDebug, GWLP_HINSTANCE), NULL);

	/* コマンドのテキストボックスを作成する */
	hWndTextboxCommand = CreateWindow(
		"EDIT",
		NULL,
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | WS_BORDER | ES_MULTILINE,
		10, 300, 420, 100,
		hWndDebug, 0,
		(HINSTANCE)GetWindowLongPtr(hWndDebug, GWLP_HINSTANCE), NULL);

	/* スクリプトのテキストボックスを作成する */
	hWndListbox = CreateWindow(
		"LISTBOX",
		NULL,
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | WS_BORDER | WS_VSCROLL |
		LBS_NOTIFY | LBS_WANTKEYBOARDINPUT,
		10, 420, 420, 300,
		hWndDebug, 0,
		(HINSTANCE)GetWindowLongPtr(hWndDebug, GWLP_HINSTANCE), NULL);

	/* ウィンドウを表示する */
	ShowWindow(hWndDebug, nCmdShow);
	UpdateWindow(hWndDebug);

	return TRUE;
}

/* デバッガ関連のウィンドウプロシージャの処理 */
static LRESULT CALLBACK WndProcDebug(HWND hWnd,
									 UINT message,
									 WPARAM wParam,
									 LPARAM lParam)
{
	int nId;
	int nEvent;

	switch(message)
	{
	case WM_SYSKEYDOWN:
		if(wParam == VK_F4)
		{
			DestroyWindow(hWndMain);
			return 0;
		}
		break;
	case WM_CLOSE:
		DestroyWindow(hWndMain);
		return 0;
	case WM_VKEYTOITEM:
		if(hWnd == hWndDebug && LOWORD(wParam) == VK_RETURN)
		{
			OnClickListBox();
			return 0;
		}
		break;
	case WM_COMMAND:
		nId = LOWORD(wParam);
		nEvent = HIWORD(wParam);
		if(nEvent == LBN_DBLCLK)
		{
			OnClickListBox();
			return 0;
		}
		switch(nId)
		{
		case ID_QUIT:
			DestroyWindow(hWndMain);
			break;
		case ID_VERSION:
			MessageBox(hWndMain, szVersion, "About", MB_OK | MB_ICONINFORMATION);
			break;
		case ID_RESUME:
			bResumePressed = TRUE;
			break;
		case ID_NEXT:
			bNextPressed = TRUE;
			break;
		case ID_PAUSE:
			bPausePressed = TRUE;
			break;
		case ID_CHANGE_SCRIPT:
			bChangeScriptPressed = TRUE;
			break;
		case ID_CHANGE_LINE:
			bChangeLinePressed = TRUE;
			break;
		case ID_READ:
			bReadPressed = TRUE;
			break;
		case ID_WRITE:
			bWritePressed = TRUE;
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		return 0;
	default:
		break;
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}

/* リストボックスのクリックと改行キー入力を処理する */
static VOID OnClickListBox(void)
{
	char line[10];
	if(!bRunning)
	{
		snprintf(line, sizeof(line), "%d",
				 (int)SendMessage(hWndListbox, LB_GETCURSEL, 0, 0) +
				 1);
		SetWindowText(hWndTextboxLine, line);
		bChangeLinePressed = TRUE;
	}
}

/* 再開ボタンが押されたか調べる */
bool is_resume_pushed(void)
{
	bool ret = bResumePressed;
	bResumePressed = FALSE;
	return ret;
}

/* 次へボタンが押されたか調べる */
bool is_next_pushed(void)
{
	bool ret = bNextPressed;
	bNextPressed = FALSE;
	return ret;
}

/* 停止ボタンが押されたか調べる */
bool is_pause_pushed(void)
{
	bool ret = bPausePressed;
	bPausePressed = FALSE;
	return ret;
}

/* 実行するスクリプトファイルが変更されたか調べる */
bool is_script_changed(void)
{
	bool ret = bChangeScriptPressed;
	bChangeScriptPressed = FALSE;
	return ret;
}

/* 変更された実行するスクリプトファイル名を取得する */
const char *get_changed_script(void)
{
	static char script[256];

	GetWindowText(hWndTextboxScript, script, sizeof(script) - 1);

	return script;
}

/* 実行する行番号が変更されたか調べる */
bool is_line_changed(void)
{
	bool ret = bChangeLinePressed;
	bChangeLinePressed = FALSE;
	return ret;
}

/* 変更された実行するスクリプトファイル名を取得する */
int get_changed_line(void)
{
	static char text[256];
	int line;

	GetWindowText(hWndTextboxLine, text, sizeof(text) - 1);

	line = atoi(text) - 1;

	return line;
}

/*
 * コマンドの実行中状態を設定する
 */
void set_running_state(bool running, bool request_stop)
{
	BOOL bEnglish;

	bRunning = running;

	/* 英語モードかどうかチェックする */
	bEnglish = conf_language != NULL;

	/* 停止によりコマンドの完了を待機中のとき */
	if(request_stop)
	{
		/* ウィンドウのタイトルを設定する */
		SetWindowText(hWndDebug, bEnglish ?
					  "Waiting for finish command... - Suika Studio" :
					  "コマンドの完了を待機中... - Suika Studio");

		/* 続けるボタンを無効にする */
		EnableWindow(hWndBtnResume, FALSE);
		SetWindowText(hWndBtnResume, bEnglish ? "(Resume)" : "(続ける)");

		/* 次へボタンを無効にする */
		EnableWindow(hWndBtnNext, FALSE);
		SetWindowText(hWndBtnNext, bEnglish ? "(Next)" : "(次へ)");

		/* 停止ボタンを無効にする */
		EnableWindow(hWndBtnPause, FALSE);
		SetWindowText(hWndBtnPause, bEnglish ? "(Waiting)" : "(完了待ち)");

		/* スクリプトテキストボックスを無効にする */
		EnableWindow(hWndTextboxScript, FALSE);

		/* スクリプト変更ボタンを無効にする */
		EnableWindow(hWndBtnChangeScript, FALSE);

		/* 行番号テキストボックスを無効にする */
		EnableWindow(hWndTextboxLine, FALSE);

		/* 行番号変更ボタンを無効にする */
		EnableWindow(hWndBtnChangeLine, FALSE);

		/* コマンドテキストボックスを無効にする */
		EnableWindow(hWndTextboxCommand, FALSE);

		/* 続けるメニューを無効にする */
		EnableMenuItem(hMenu, ID_RESUME, MF_GRAYED);

		/* 次へメニューを無効にする */
		EnableMenuItem(hMenu, ID_NEXT, MF_GRAYED);

		/* 停止メニューを無効にする */
		EnableMenuItem(hMenu, ID_PAUSE, MF_GRAYED);
	}
	/* 実行中のとき */
	else if(running)
	{
		/* ウィンドウのタイトルを設定する */
		SetWindowText(hWndDebug, bEnglish ?
					  "Running... - Suika Studio" :
					  "実行中... - Suika Studio");

		/* 続けるボタンを無効にする */
		EnableWindow(hWndBtnResume, FALSE);
		SetWindowText(hWndBtnResume, bEnglish ? "(Resume)" : "(続ける)");

		/* 次へボタンを無効にする */
		EnableWindow(hWndBtnNext, FALSE);
		SetWindowText(hWndBtnNext, bEnglish ? "(Next)" : "(次へ)");

		/* 停止ボタンを有効にする */
		EnableWindow(hWndBtnPause, TRUE);
		SetWindowText(hWndBtnPause, bEnglish ? "Pause" : "停止");

		/* スクリプトテキストボックスを無効にする */
		EnableWindow(hWndTextboxScript, FALSE);

		/* スクリプト変更ボタンを無効にする */
		EnableWindow(hWndBtnChangeScript, FALSE);

		/* 行番号テキストボックスを無効にする */
		EnableWindow(hWndTextboxLine, FALSE);

		/* 行番号変更ボタンを無効にする */
		EnableWindow(hWndBtnChangeLine, FALSE);

		/* コマンドテキストボックスを無効にする */
		EnableWindow(hWndTextboxCommand, FALSE);

		/* 続けるメニューを無効にする */
		EnableMenuItem(hMenu, ID_RESUME, MF_GRAYED);

		/* 次へメニューを無効にする */
		EnableMenuItem(hMenu, ID_NEXT, MF_GRAYED);

		/* 停止メニューを有効にする */
		EnableMenuItem(hMenu, ID_PAUSE, MF_ENABLED);
	}
	/* 完全に停止中のとき */
	else
	{
		/* ウィンドウのタイトルを設定する */
		SetWindowText(hWndDebug, bEnglish ?
					  "Stopped - Suika Studio" :
					  "停止中 - Suika Studio");

		/* 続けるボタンを有効にする */
		EnableWindow(hWndBtnResume, TRUE);
		SetWindowText(hWndBtnResume, bEnglish ? "Resume" : "続ける");

		/* 次へボタンを有効にする */
		EnableWindow(hWndBtnNext, TRUE);
		SetWindowText(hWndBtnNext, bEnglish ? "Next" : "次へ");

		/* 停止ボタンを無効にする */
		EnableWindow(hWndBtnPause, FALSE);
		SetWindowText(hWndBtnPause, bEnglish ? "(Pausing)" : "(停止中)");

		/* スクリプトテキストボックスを有効にする */
		EnableWindow(hWndTextboxScript, TRUE);

		/* スクリプト変更ボタンを有効にする */
		EnableWindow(hWndBtnChangeScript, TRUE);

		/* 行番号テキストボックスを有効にする */
		EnableWindow(hWndTextboxLine, TRUE);

		/* 行番号変更ボタンを有効にする */
		EnableWindow(hWndBtnChangeLine, TRUE);

		/* コマンドテキストボックスを有効にする */
		EnableWindow(hWndTextboxCommand, TRUE);

		/* 続けるメニューを有効にする */
		EnableMenuItem(hMenu, ID_RESUME, MF_ENABLED);

		/* 次へメニューを有効にする */
		EnableMenuItem(hMenu, ID_NEXT, MF_ENABLED);

		/* 停止メニューを無効にする */
		EnableMenuItem(hMenu, ID_PAUSE, MF_GRAYED);
	}
}

/*
 * デバッグ情報を更新する
 */
void update_debug_info(bool script_changed)
{
	char line[10];
	const char *command;
	int line_num;
	int top;

	/* スクリプトファイル名を設定する */
	SetWindowText(hWndTextboxScript, get_script_file_name());

	/* 行番号を設定する */
	snprintf(line, sizeof(line), "%d", get_line_num() + 1);
	SetWindowText(hWndTextboxLine, line);

	/* コマンド文字列を設定する */
	SetWindowText(hWndTextboxCommand, conv_utf8_to_native(get_line_string()));

	/* スクリプトのリストボックスを設定する */
	if(script_changed)
	{
		SendMessage(hWndListbox, LB_RESETCONTENT, 0 , 0);
		for(line_num = 0; line_num < get_line_count(); line_num++)
		{
			command = get_line_string_at_line_num(line_num);
			command = conv_utf8_to_native(command);
			SendMessage(hWndListbox, LB_ADDSTRING, 0, (LPARAM)command);
		}
	}
	line_num = get_line_num();
	top = (line_num - 8 < 0) ? 0 : (line_num - 8);
	SendMessage(hWndListbox, LB_SETCURSEL, (WPARAM)line_num, 0);
	SendMessage(hWndListbox, LB_SETTOPINDEX, (WPARAM)top, 0);
}
#endif
