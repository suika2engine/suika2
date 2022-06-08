/* -*- Coding: utf-8-with-signature; indent-tabs-mode: t; tab-width: 4; c-basic-offset: 4; -*- */

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

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif

#include "suika.h"
#include "dsound.h"
#include "dsvideo.h"
#include "resource.h"

#ifdef USE_DEBUGGER
#include <commctrl.h>
#endif

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

/* SJISからUTF-8への変換バッファ */
static char szUtf8Message[NATIVE_MESSAGE_SIZE];

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
const char *ConvNativeToUtf8(const char *lpszNativeMessage);
#endif

/*
 * デバッガを使う場合
 */
#ifdef USE_DEBUGGER
/* メッセージボックスのタイトル */
#define MSGBOX_TITLE		"Suika2 Pro"

/* 変数テキストボックスのテキストの最大長(形: "$00001=12345678901\r\n") */
#define VAR_TEXTBOX_MAX		(11000 * (1 + 5 + 1 + 11 + 2))

/* バージョン文字列 */
static char szVersion[] =
	"Suika2 Pro\n"
	"Copyright (c) 2022, LUXION SOFT. All rights reserved.\n"
	"\n"
	"Contributors:\n"
	"TABATA Keiichi (Programmer)\n"
	"MATSUNO Seiji (Supervisor)\n";

/* 実行状態 */
BOOL bRunning;

/* メニュー */
static HMENU hMenu;

/* ステータスバー */
static HWND hWndStatus;

/* デバッガウィンドウ */
static HWND hWndDebug;

/* デバッガウィンドウのコンポーネント */
static HWND hWndBtnResume;
static HWND hWndBtnNext;
static HWND hWndBtnPause;
static HWND hWndLabelScript;
static HWND hWndTextboxScript;
static HWND hWndBtnChangeScript;
static HWND hWndBtnSelectScript;
static HWND hWndLabelLine;
static HWND hWndTextboxLine;
static HWND hWndBtnChangeLine;
static HWND hWndLabelCommand;
static HWND hWndTextboxCommand;
static HWND hWndBtnUpdate;
static HWND hWndBtnReset;
static HWND hWndLabelContent;
static HWND hWndListbox;
static HWND hWndBtnError;
static HWND hWndBtnSave;
static HWND hWndBtnReload;
static HWND hWndLabelVar;
static HWND hWndTextboxVar;
static HWND hWndBtnVar;

/* ボタンが押下されたか */
static BOOL bResumePressed;
static BOOL bNextPressed;
static BOOL bPausePressed;
static BOOL bChangeScriptPressed;
static BOOL bChangeLinePressed;
static BOOL bUpdatePressed;
static BOOL bReloadPressed;

/* 変数のテキストボックスの内容 */
static char szTextboxVar[VAR_TEXTBOX_MAX + 1];

/* 前方参照 */
static VOID InitMenu(void);
static VOID InitStatusBar(void);
static BOOL InitDebugger(HINSTANCE hInstance, int nCmdShow);
static HWND CreateTooltip(HWND hWndBtn, const char *pszTextEnglish,
						  const char *pszTextJapanese);
static LRESULT CALLBACK WndProcDebug(HWND hWnd, UINT message, WPARAM wParam,
									 LPARAM lParam);
static VOID OnClickListBox(void);
static VOID OnSelectScript(void);
static VOID OnPressReset(void);
static VOID OnPressSave(void);
static VOID OnPressError(void);
static VOID UpdateVariableTextBox(void);
static VOID OnPressWriteVars(void);
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
#ifndef USE_DEBUGGER
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
#endif
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
#ifdef USE_DEBUGGER
	strcpy(mbszTitle, MSGBOX_TITLE);
	strcat(mbszTitle, " - ");
#endif
	cch = MultiByteToWideChar(CP_UTF8, 0, conf_window_title, -1, wszTitle,
							  TITLE_BUF_SIZE - 1);
	wszTitle[cch] = L'\0';
	WideCharToMultiByte(CP_THREAD_ACP, 0, wszTitle, (int)wcslen(wszTitle),
#ifdef USE_DEBUGGER
						mbszTitle + strlen(MSGBOX_TITLE) + 3,
						TITLE_BUF_SIZE - strlen(MSGBOX_TITLE)  - 3 - 1,
#else
						mbszTitle,
						TITLE_BUF_SIZE - 1,
#endif
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

	/* ステータスバーを作成する */
	InitStatusBar();

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
	   hWnd == hWndTextboxCommand || hWnd == hWndBtnUpdate ||
	   hWnd == hWndBtnReset || hWnd == hWndListbox || hWnd == hWndBtnError ||
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
		return 0;
#else
		if (MessageBox(hWnd, conf_language == NULL ?
					   "終了しますか？" : "Quit?",
					   mbszTitle, MB_OKCANCEL) == IDOK)
			DestroyWindow(hWnd);
		return 0;
#endif
#ifdef USE_DEBUGGER
	case WM_SIZE:
		/* ステータスバーの位置を正しくする */
		SendMessage(hWndStatus, WM_SIZE, wParam, lParam);
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
 * ネイティブ文字コードのメッセージをUtf-8文字コードに変換する
 */
const char *ConvNativeToUtf8(const char *lpszNativeMessage)
{
	wchar_t wszMessage[NATIVE_MESSAGE_SIZE];
	int cch;

	assert(lpszNativeMessage != NULL);

	/* ネイティブ文字コードからワイド文字に変換する */
	cch = MultiByteToWideChar(CP_THREAD_ACP, 0, lpszNativeMessage, -1,
							  wszMessage, NATIVE_MESSAGE_SIZE - 1);
	wszMessage[cch] = L'\0';

	/* ワイド文字からSJISに変換する */
	WideCharToMultiByte(CP_UTF8, 0, wszMessage, -1, szUtf8Message,
						NATIVE_MESSAGE_SIZE - 1, NULL, NULL);

	return szUtf8Message;
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

	/* メッセージボックスを表示する */
	vsnprintf(buf, sizeof(buf), s, ap);
	MessageBox(hWndMain, buf, conf_language == NULL ? "情報" : "Info",
			   MB_OK | MB_ICONINFORMATION);

	if(pLogFile != NULL)
	{
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

	/* メッセージボックスを表示する */
	vsnprintf(buf, sizeof(buf), s, ap);
	MessageBox(hWndMain, buf, conf_language == NULL ? "警告" : "Warning",
			   MB_OK | MB_ICONWARNING);

	if(pLogFile != NULL)
	{
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

	/* メッセージボックスを表示する */
	vsnprintf(buf, sizeof(buf), s, ap);
	MessageBox(hWndMain, buf, conf_language == NULL ? "エラー" : "Error",
			   MB_OK | MB_ICONERROR);

	if(pLogFile != NULL)
	{
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

	/* スクリプトを開く(Q)を作成する */
	mi.wID = ID_SELECT_SCRIPT;
	mi.dwTypeData = bEnglish ?
		"Open script(&Q)\tAlt+O" :
		"スクリプトを開く(&O)\tAlt+O";
	InsertMenuItem(hMenuFile, 0, TRUE, &mi);

	/* スクリプトを上書き保存する(S)を作成する */
	mi.wID = ID_SAVE;
	mi.dwTypeData = bEnglish ?
		"Overwrite script(&S)\tAlt+S" :
		"スクリプトを上書き保存する(&S)\tAlt+S";
	InsertMenuItem(hMenuFile, 1, TRUE, &mi);

	/* 終了(Q)を作成する */
	mi.wID = ID_QUIT;
	mi.dwTypeData = bEnglish ? "Quit(&Q)\tAlt+Q" : "終了(&Q)\tAlt+Q";
	InsertMenuItem(hMenuFile, 2, TRUE, &mi);

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

	/* 次のエラー箇所へ移動(E)を作成する */
	mi.wID = ID_ERROR;
	mi.dwTypeData = bEnglish ?
		"Go to next error(&E)\tAlt+E" :
		"次のエラー箇所へ移動(&E)\tAlt+E";
	InsertMenuItem(hMenuScript, 3, TRUE, &mi);

	/* 再読み込み(R)を作成する */
	mi.wID = ID_RELOAD;
	mi.dwTypeData = bEnglish ? "Reload(&R)\tF5" : "再読み込み(&R)\tF5";
	InsertMenuItem(hMenuScript, 4, TRUE, &mi);

	/* バージョン(V)を作成する */
	mi.wID = ID_VERSION;
	mi.dwTypeData = bEnglish ? "Version(&V)" : "バージョン(&V)\tAlt+V";
	InsertMenuItem(hMenuHelp, 0, TRUE, &mi);

	/* メニューをセットする */
	SetMenu(hWndMain, hMenu);
}

/* ステータスバーを作成する */
VOID InitStatusBar(void)
{
	int sizes[] = {200, 400, -1};
	RECT rcBar, rcMain;

	/* ステータスバーを作成する */
	hWndStatus = CreateWindowEx(0, STATUSCLASSNAME, NULL,
								WS_CHILD | CCS_BOTTOM |
								WS_VISIBLE,
								0, 0, 0, 0,
								hWndMain, (HMENU)ID_STATUS,
								GetModuleHandle(NULL), NULL);
//	SendMessage(hWndStatus, SB_SIMPLE, TRUE, 0);
//	SendMessage(hWndStatus, SB_SETTEXT, (WPARAM)(SB_SIMPLEID | 0),
	SendMessage(hWndStatus, SB_SETPARTS, (WPARAM)3, (LPARAM)(LPINT)sizes);
	SendMessage(hWndStatus, SB_SETTEXT, (WPARAM)(0 | 0),
				(LPARAM)(conf_language == NULL ? "停止中" : "Stopped"));
	SendMessage(hWndStatus, SB_SETTEXT, (WPARAM)(0 | 1), (LPARAM)"init.txt");
	SendMessage(hWndStatus, SB_SETTEXT, (WPARAM)(0 | 2), (LPARAM)"1");
	GetWindowRect(hWndStatus, &rcBar);

	/* ウィンドウのサイズを調整する */
	SetRectEmpty(&rcMain);
	rcMain.right = conf_window_width;
	rcMain.bottom = conf_window_height + (rcBar.bottom - rcBar.top);
	AdjustWindowRectEx(&rcMain, (DWORD)GetWindowLong(hWndMain, GWL_STYLE),
					   TRUE, (DWORD)GetWindowLong(hWndMain, GWL_EXSTYLE));
	SetWindowPos(hWndMain, NULL, 0, 0, rcMain.right - rcMain.left,
				 rcMain.bottom - rcMain.top, SWP_NOZORDER | SWP_NOMOVE);
}

/* デバッガを初期化する */
static BOOL InitDebugger(HINSTANCE hInstance, int nCmdShow)
{
	const char szWndClass[] = "suikadebug";
	WNDCLASSEX wcex;
	RECT rc;
	HFONT hFont, hFontFixed;
	DWORD style;
	int dw, dh;
	BOOL bEnglish;
	const int WIN_WIDTH = 440;
	const int WIN_HEIGHT = 905;

	/* 英語モードかどうかチェックする */
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
	hWndDebug = CreateWindowEx(0, szWndClass, bEnglish ? "Stopped" : "停止中",
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

	/* フォントを作成する */
	hFont = CreateFont(16, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE,
					   ANSI_CHARSET, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS,
					   DEFAULT_QUALITY,
					   DEFAULT_PITCH | FF_DONTCARE, "Yu Gothic UI");
	hFontFixed = CreateFont(16, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE,
							ANSI_CHARSET, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS,
							PROOF_QUALITY,
							DEFAULT_PITCH | FF_DONTCARE, "Yu Gothic");

	/* 続けるボタンを作成する */
	hWndBtnResume = CreateWindow(
		"BUTTON",
		bEnglish ? "Resume" : "続ける",
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
		10, 10, 100, 80,
		hWndDebug, (HMENU)ID_RESUME,
		(HINSTANCE)GetWindowLongPtr(hWndDebug, GWLP_HINSTANCE), NULL);
	SendMessage(hWndBtnResume, WM_SETFONT, (WPARAM)hFont, (LPARAM)TRUE);
	CreateTooltip(hWndBtnResume,
				  "Start executing script and run continuosly.",
				  "スクリプトの実行を開始し、継続して実行します。");

	/* 次へボタンを作成する */
	hWndBtnNext = CreateWindow(
		"BUTTON",
		bEnglish ? "Next" : "次へ",
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
		120, 10, 100, 80,
		hWndDebug, (HMENU)ID_NEXT,
		(HINSTANCE)GetWindowLongPtr(hWndDebug, GWLP_HINSTANCE), NULL);
	SendMessage(hWndBtnNext, WM_SETFONT, (WPARAM)hFont, (LPARAM)TRUE);
	CreateTooltip(hWndBtnNext,
				  "Run only one command and stop after it.",
				  "コマンドを1個だけ実行し、停止します。");

	/* 停止ボタンを作成する */
	hWndBtnPause = CreateWindow(
		"BUTTON",
		bEnglish ? "(Paused)" : "(停止中)",
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
		330, 10, 100, 80,
		hWndDebug, (HMENU)ID_PAUSE,
		(HINSTANCE)GetWindowLongPtr(hWndDebug, GWLP_HINSTANCE), NULL);
	EnableWindow(hWndBtnPause, FALSE);
	SendMessage(hWndBtnPause, WM_SETFONT, (WPARAM)hFont, (LPARAM)TRUE);
	CreateTooltip(hWndBtnPause,
				  "Stop script execution.",
				  "コマンドの実行を停止します。");

	/* スクリプトラベルを作成する */
	hWndLabelScript = CreateWindow(
		"STATIC",
		bEnglish ? "Script file name:" : "スクリプトファイル名:",
		WS_VISIBLE | WS_CHILD,
		10, 110, 100, 16,
		hWndDebug, 0,
		(HINSTANCE)GetWindowLongPtr(hWndDebug, GWLP_HINSTANCE), NULL);
	SendMessage(hWndLabelScript, WM_SETFONT, (WPARAM)hFont, (LPARAM)TRUE);

	/* スクリプト名のテキストボックスを作成する */
	hWndTextboxScript = CreateWindow(
		"EDIT",
		NULL,
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL,
		10, 130, 300, 30,
		hWndDebug, 0,
		(HINSTANCE)GetWindowLongPtr(hWndDebug, GWLP_HINSTANCE), NULL);
	SendMessage(hWndTextboxScript, WM_SETFONT, (WPARAM)hFont, (LPARAM)TRUE);
	CreateTooltip(hWndTextboxScript,
				  "Write script file name to be jumped to.",
				  "ジャンプしたいスクリプトファイル名を書きます。");

	/* スクリプトの変更ボタンを作成する */
	hWndBtnChangeScript = CreateWindow(
		"BUTTON",
		bEnglish ? "Change" : "変更",
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
		320, 130, 80, 30,
		hWndDebug, (HMENU)ID_CHANGE_SCRIPT,
		(HINSTANCE)GetWindowLongPtr(hWndDebug, GWLP_HINSTANCE), NULL);
	SendMessage(hWndBtnChangeScript, WM_SETFONT, (WPARAM)hFont, (LPARAM)TRUE);
	CreateTooltip(hWndBtnChangeScript,
				  "Jump to the script written in the left text box.",
				  "左のテキストボックスに書いたスクリプトにジャンプします。");

	/* スクリプトの選択ボタンを作成する */
	hWndBtnSelectScript = CreateWindow(
		"BUTTON", "...",
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
		405, 130, 25, 30,
		hWndDebug, (HMENU)ID_SELECT_SCRIPT,
		(HINSTANCE)GetWindowLongPtr(hWndDebug, GWLP_HINSTANCE), NULL);
	SendMessage(hWndBtnSelectScript, WM_SETFONT, (WPARAM)hFont, (LPARAM)TRUE);
	CreateTooltip(hWndBtnSelectScript,
				  "Select a script file and jump to it.",
				  "スクリプトファイルを選択してジャンプします。");

	/* 行番号ラベルを作成する */
	hWndLabelLine = CreateWindow(
		"STATIC",
		bEnglish ? "Next line to be executed:" : "次に実行される行番号:",
		WS_VISIBLE | WS_CHILD,
		10, 170, 300, 16,
		hWndDebug, 0,
		(HINSTANCE)GetWindowLongPtr(hWndDebug, GWLP_HINSTANCE), NULL);
	SendMessage(hWndLabelLine, WM_SETFONT, (WPARAM)hFont, (LPARAM)TRUE);

	/* 行番号のテキストボックスを作成する */
	hWndTextboxLine = CreateWindow(
		"EDIT",
		NULL,
		WS_TABSTOP | ES_NUMBER | WS_VISIBLE | WS_CHILD | WS_BORDER,
		10, 190, 80, 30,
		hWndDebug, 0,
		(HINSTANCE)GetWindowLongPtr(hWndDebug, GWLP_HINSTANCE), NULL);
	SendMessage(hWndTextboxLine, WM_SETFONT, (WPARAM)hFont, (LPARAM)TRUE);
	CreateTooltip(hWndTextboxLine,
				  "Write line number to be jumped to.",
				  "ジャンプしたい行番号を書きます。");

	/* 行番号の変更ボタンを作成する */
	hWndBtnChangeLine = CreateWindow(
		"BUTTON",
		bEnglish ? "Change" : "変更",
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
		100, 190, 80, 30,
		hWndDebug, (HMENU)ID_CHANGE_LINE,
		(HINSTANCE)GetWindowLongPtr(hWndDebug, GWLP_HINSTANCE), NULL);
	SendMessage(hWndBtnChangeLine, WM_SETFONT, (WPARAM)hFont, (LPARAM)TRUE);
	CreateTooltip(hWndBtnChangeLine,
				  "Jump to the line written in the left text box.",
				  "左のテキストボックスに書いた行にジャンプします。");

	/* コマンドのラベルを作成する */
	hWndLabelCommand = CreateWindow(
		"STATIC",
		bEnglish ? "Next command to be executed:" : "次に実行されるコマンド:",
		WS_VISIBLE | WS_CHILD,
		10, 230, 300, 16,
		hWndDebug, 0,
		(HINSTANCE)GetWindowLongPtr(hWndDebug, GWLP_HINSTANCE), NULL);
	SendMessage(hWndLabelCommand, WM_SETFONT, (WPARAM)hFont, (LPARAM)TRUE);

	/* コマンドのテキストボックスを作成する */
	hWndTextboxCommand = CreateWindow(
		"EDIT",
		NULL,
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOVSCROLL |
		ES_MULTILINE | ES_WANTRETURN, // ES_READONLY
		10, 250, 330, 100,
		hWndDebug, 0,
		(HINSTANCE)GetWindowLongPtr(hWndDebug, GWLP_HINSTANCE), NULL);
	SendMessage(hWndTextboxCommand, WM_SETFONT, (WPARAM)hFontFixed,
				(LPARAM)TRUE);
	CreateTooltip(hWndTextboxCommand,
				  "This is a command text to be executed next.",
				  "次に実行されるコマンドのテキストです。"
				  "編集することで書き換えることができます。");

	/* コマンドアップデートのボタンを作成する */
	hWndBtnUpdate = CreateWindow(
		"BUTTON",
		bEnglish ? "Update" : "更新",
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
		WIN_WIDTH - 10 - 80, 250, 80, 30,
		hWndDebug, (HMENU)ID_UPDATE_COMMAND,
		(HINSTANCE)GetWindowLongPtr(hWndDebug, GWLP_HINSTANCE), NULL);
	SendMessage(hWndBtnUpdate, WM_SETFONT, (WPARAM)hFont, (LPARAM)TRUE);
	CreateTooltip(hWndBtnUpdate,
				  "Reflect command text for execution. "
				  "Script file will not be overwritten at this point.",
				  "編集したコマンドを実行に反映します。"
				  "ここで反映しただけではファイルには書き込まれません。");

	/* コマンドリセットのボタンを作成する */
	hWndBtnReset = CreateWindow(
		"BUTTON",
		bEnglish ? "Reset" : "リセット",
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
		WIN_WIDTH - 10 - 80, 290, 80, 30,
		hWndDebug, (HMENU)ID_RESET_COMMAND,
		(HINSTANCE)GetWindowLongPtr(hWndDebug, GWLP_HINSTANCE), NULL);
	SendMessage(hWndBtnReset, WM_SETFONT, (WPARAM)hFont, (LPARAM)TRUE);
	CreateTooltip(hWndBtnReset,
				  "Reset the command text.",
				  "編集したコマンドを元に戻します。");

	/* スクリプト内容のラベルを作成する */
	hWndLabelContent = CreateWindow(
		"STATIC",
		bEnglish ? "Script content:" : "スクリプトの内容:",
		WS_VISIBLE | WS_CHILD,
		10, 360, 100, 16,
		hWndDebug, 0,
		(HINSTANCE)GetWindowLongPtr(hWndDebug, GWLP_HINSTANCE), NULL);
	SendMessage(hWndLabelContent, WM_SETFONT, (WPARAM)hFont, (LPARAM)TRUE);

	/* スクリプトのリストボックスを作成する */
	hWndListbox = CreateWindow(
		"LISTBOX",
		NULL,
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | WS_BORDER | WS_VSCROLL |
		LBS_NOTIFY | LBS_WANTKEYBOARDINPUT,
		10, 380, 420, 310,
		hWndDebug, 0,
		(HINSTANCE)GetWindowLongPtr(hWndDebug, GWLP_HINSTANCE), NULL);
	SendMessage(hWndListbox, WM_SETFONT, (WPARAM)hFontFixed, (LPARAM)TRUE);
	CreateTooltip(hWndListbox,
				  "Current script content.",
				  "実行中のスクリプトの内容です。");

	/* エラーを探すを有効にする */
	hWndBtnError = CreateWindow(
		"BUTTON",
		bEnglish ? "Search for error" : "エラーを探す",
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
		10, 735 - 10 - 30, 120, 30,
		hWndDebug, (HMENU)ID_ERROR,
		(HINSTANCE)GetWindowLongPtr(hWndDebug, GWLP_HINSTANCE), NULL);
	SendMessage(hWndBtnError, WM_SETFONT, (WPARAM)hFont, (LPARAM)TRUE);
	CreateTooltip(hWndBtnError,
				  "Search for a next error.",
				  "次のエラー箇所を探します。");

	/* 上書き保存ボタンを作成する */
	hWndBtnSave = CreateWindow(
		"BUTTON",
		bEnglish ? "Overwrite" : "上書き保存",
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
		WIN_WIDTH - 10 - 80 - 10 - 80, 735 - 10 - 30, 80, 30,
		hWndDebug, (HMENU)ID_SAVE,
		(HINSTANCE)GetWindowLongPtr(hWndDebug, GWLP_HINSTANCE), NULL);
	SendMessage(hWndBtnSave, WM_SETFONT, (WPARAM)hFont, (LPARAM)TRUE);
	CreateTooltip(hWndBtnSave,
				  "Overwrite the contents of the modified script.",
				  "スクリプトの内容をファイルに上書き保存します。");

	/* 再読み込みボタンを作成する */
	hWndBtnReload = CreateWindow(
		"BUTTON",
		bEnglish ? "Reload" : "再読み込み",
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
		WIN_WIDTH - 10 - 80, 735 - 10 - 30, 80, 30,
		hWndDebug, (HMENU)ID_RELOAD,
		(HINSTANCE)GetWindowLongPtr(hWndDebug, GWLP_HINSTANCE), NULL);
	SendMessage(hWndBtnReload, WM_SETFONT, (WPARAM)hFont, (LPARAM)TRUE);
	CreateTooltip(hWndBtnReload,
				  "Reload the script from file.",
				  "スクリプトファイルの内容を再読み込みします。");

	/* 変数のラベルを作成する */
	hWndLabelVar = CreateWindow(
		"STATIC",
		bEnglish ?
		"Variables (non-initial values):" :
		"変数 (初期値でない):",
		WS_VISIBLE | WS_CHILD,
		10, 735, 200, 16,
		hWndDebug, 0,
		(HINSTANCE)GetWindowLongPtr(hWndDebug, GWLP_HINSTANCE), NULL);
	SendMessage(hWndLabelVar, WM_SETFONT, (WPARAM)hFont, (LPARAM)TRUE);

	/* 変数のテキストボックスを作成する */
	hWndTextboxVar = CreateWindow(
		"EDIT",
		NULL,
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOVSCROLL |
		ES_MULTILINE | ES_WANTRETURN, // ES_READONLY
		10, 755, 420, 100,
		hWndDebug, 0,
		(HINSTANCE)GetWindowLongPtr(hWndDebug, GWLP_HINSTANCE), NULL);
	SendMessage(hWndTextboxVar, WM_SETFONT, (WPARAM)hFontFixed, (LPARAM)TRUE);
	CreateTooltip(hWndTextboxVar,
				  "List of variables which have non-initial values.",
				  "初期値から変更された変数の一覧です。");

	/* 値を書き込むボタンを作成する */
	hWndBtnVar = CreateWindow(
		"BUTTON",
		bEnglish ? "Write values" : "値を書き込む",
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
		WIN_WIDTH - 10 - 120, WIN_HEIGHT - 10 - 30, 120, 30,
		hWndDebug, (HMENU)ID_WRITE,
		(HINSTANCE)GetWindowLongPtr(hWndDebug, GWLP_HINSTANCE), NULL);
	SendMessage(hWndBtnVar, WM_SETFONT, (WPARAM)hFont, (LPARAM)TRUE);
	CreateTooltip(hWndBtnVar,
				  "Write to the variables.",
				  "変数の内容を書き込みます。");

	/* ウィンドウを表示する */
	ShowWindow(hWndDebug, nCmdShow);
	UpdateWindow(hWndDebug);

	return TRUE;
}

/* ツールチップを作成する */
static HWND CreateTooltip(HWND hWndBtn, const char *pszTextEnglish,
				   const char *pszTextJapanese)
{
	TOOLINFO ti;

	/* ツールチップを作成する */
	HWND hWndTip = CreateWindowEx(0, TOOLTIPS_CLASS, NULL, TTS_ALWAYSTIP,
								  CW_USEDEFAULT, CW_USEDEFAULT,
								  CW_USEDEFAULT, CW_USEDEFAULT,
								  hWndDebug, NULL, GetModuleHandle(NULL),
								  NULL);

	/* ツールチップをボタンに紐付ける */
	ZeroMemory(&ti, sizeof(ti));
	ti.cbSize = sizeof(ti);
	ti.uFlags = TTF_SUBCLASS;
	ti.hwnd = hWndBtn;
	ti.lpszText = (char *)(conf_language == NULL ?
						   pszTextJapanese : pszTextEnglish);
	GetClientRect(hWndBtn, &ti.rect);
	SendMessage(hWndTip, TTM_ADDTOOL, 0, (LPARAM)&ti);

	return hWndTip;
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
			MessageBox(hWndMain, szVersion, MSGBOX_TITLE,
					   MB_OK | MB_ICONINFORMATION);
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
		case ID_SELECT_SCRIPT:
			OnSelectScript();
			break;
		case ID_CHANGE_LINE:
			bChangeLinePressed = TRUE;
			break;
		case ID_UPDATE_COMMAND:
			bUpdatePressed = TRUE;
			break;
		case ID_RESET_COMMAND:
			OnPressReset();
			break;
		case ID_ERROR:
			OnPressError();
			break;
		case ID_SAVE:
			OnPressSave();
			break;
		case ID_RELOAD:
			bReloadPressed = TRUE;
			break;
		case ID_WRITE:
			OnPressWriteVars();
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

/* スクリプト選択ボタンが押された場合の処理を行う */
static VOID OnSelectScript(void)
{
	OPENFILENAME ofn;
	char szPath[1024];

	szPath[0] = '\0';

	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = hWndMain;
	ofn.lpstrFilter = conf_language == NULL ?
		"テキストファイル\0*.txt;\0すべてのファイル(*.*)\0*.*\0\0" :
		"Text Files\0*.txt;\0All Files(*.*)\0*.*\0\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFile = szPath;
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrInitialDir = SCRIPT_DIR;
	ofn.Flags = OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
	ofn.lpstrDefExt = "txt";
	ofn.nMaxFileTitle = 255;
	GetOpenFileName(&ofn);
	if(ofn.lpstrFile[0])
	{
		char file[1024];
		char *file_ptr;
		GetFullPathName(szPath, sizeof(file), file, &file_ptr);
		SetWindowText(hWndTextboxScript, file_ptr);
		bChangeScriptPressed = TRUE;
	}
}

/* コマンドリセットボタンが押下された場合の処理を行う */
static VOID OnPressReset(void)
{
	/* コマンド文字列を設定する */
	SetWindowText(hWndTextboxCommand, conv_utf8_to_native(get_line_string()));
}

/* 上書き保存ボタンが押された場合の処理を行う */
static VOID OnPressSave(void)
{
	FILE *fp;
	char *path;
	const char *scr;
	int i;

	scr = get_script_file_name();
	if(strcmp(scr, "DEBUG") == 0)
		return;

	if (MessageBox(hWndMain, conf_language == NULL ?
				   "スクリプトファイルを上書き保存します。\n"
				   "よろしいですか？" :
				   "Are you sure you want to overwrite the script file?",
				   MSGBOX_TITLE, MB_ICONWARNING | MB_OKCANCEL) != IDOK)
		return;

	path = make_valid_path(SCRIPT_DIR, scr);

	fp = fopen(path, "w");
	if (fp == NULL)
	{
		free(path);
		MessageBox(hWndMain, conf_language == NULL ?
				   "ファイルに書き込めません。" :
				   "Cannot write to file.",
				   MSGBOX_TITLE, MB_OK | MB_ICONERROR);
		return;
	}
	free(path);

	for(i=0; i<get_line_count(); i++)
	{
		int body = fputs(get_line_string_at_line_num(i), fp);
		int crlf = fputs("\n", fp);
		if(body < 0 || crlf < 0)
		{
			MessageBox(hWndMain, conf_language == NULL ?
					   "ファイルに書き込めません。" :
					   "Cannot write to file.",
					   MSGBOX_TITLE, MB_OK | MB_ICONERROR);
			fclose(fp);
			return;
		}			
	}
	fclose(fp);
}

/* 次のエラー箇所へ移動ボタンが押下されたとき */
static VOID OnPressError(void)
{
	const char *text;
	int start, lines;
	int i;

	lines = get_line_count();
	start = (int)SendMessage(hWndListbox, LB_GETCURSEL, 0, 0);

	for(i=start+1; i<lines; i++)
	{
		text = get_line_string_at_line_num(i);
		if(text[0] == '!')
		{
			SendMessage(hWndListbox, LB_SETCURSEL, (WPARAM)i, 0);
			return;
		}
	}
	if(start != 0)
	{
		for(i=0; i<start; i++)
		{
			text = get_line_string_at_line_num(i);
			if(text[0] == '!')
			{
				SendMessage(hWndListbox, LB_SETCURSEL, (WPARAM)i, 0);
				return;
			}
			
		}
	}

	MessageBox(hWndDebug, conf_language == NULL ?
			   "エラーはありません。" : "No error.",
			   MSGBOX_TITLE, MB_OK | MB_ICONINFORMATION);
}

/* 変数の書き込みボタンが押下された場合を処理する */
static VOID OnPressWriteVars(void)
{
	char *p, *next_line;
	int index, val;

	/* テキストボックスの内容を取得する */
	GetWindowText(hWndTextboxVar, szTextboxVar, sizeof(szTextboxVar) - 1);

	/* パースする */
	p = szTextboxVar;
	while(*p)
	{
		/* 空行を読み飛ばす */
		if(*p == '\n')
		{
			p++;
			continue;
		}

		/* 次の行の開始文字を探す */
		next_line = p;
		while(*next_line)
		{
			if(*next_line == '\r')
			{
				*next_line = '\0';
				next_line++;
				break;
			}
			next_line++;
		}

		/* パースする */
		if(sscanf(p, "$%d=%d", &index, &val) != 2)
			index = -1, val = -1;
		if(index >= LOCAL_VAR_SIZE + GLOBAL_VAR_SIZE)
			index = -1;

		/* 変数を設定する */
		if(index != -1)
			set_variable(index, val);

		/* 次の行へポインタを進める */
		p = next_line;
	}

	UpdateVariableTextBox();
}

/*
 * 再開ボタンが押されたか調べる
 */
bool is_resume_pushed(void)
{
	bool ret = bResumePressed;
	bResumePressed = FALSE;
	return ret;
}

/*
 * 次へボタンが押されたか調べる
 */
bool is_next_pushed(void)
{
	bool ret = bNextPressed;
	bNextPressed = FALSE;
	return ret;
}

/*
 * 停止ボタンが押されたか調べる
 */
bool is_pause_pushed(void)
{
	bool ret = bPausePressed;
	bPausePressed = FALSE;
	return ret;
}

/*
 * 実行するスクリプトファイルが変更されたか調べる
 */
bool is_script_changed(void)
{
	bool ret = bChangeScriptPressed;
	bChangeScriptPressed = FALSE;
	return ret;
}

/*
 * 変更された実行するスクリプトファイル名を取得する
 */
const char *get_changed_script(void)
{
	static char script[256];

	GetWindowText(hWndTextboxScript, script, sizeof(script) - 1);

	return script;
}

/*
 * 実行する行番号が変更されたか調べる
 */
bool is_line_changed(void)
{
	bool ret = bChangeLinePressed;
	bChangeLinePressed = FALSE;
	return ret;
}

/*
 * 変更された実行するスクリプトファイル名を取得する
 */
int get_changed_line(void)
{
	static char text[256];
	int line;

	GetWindowText(hWndTextboxLine, text, sizeof(text) - 1);

	line = atoi(text) - 1;

	return line;
}

/*
 * スクリプトがアップデートされたかを調べる
 */
bool is_script_updated(void)
{
	bool ret = bReloadPressed;
	bReloadPressed = FALSE;
	return ret;
}

/*
 * コマンドがアップデートされたかを調べる
 */
bool is_command_updated(void)
{
	bool ret = bUpdatePressed;
	bUpdatePressed = FALSE;
	return ret;
}

/*
 * アップデートされたコマンド文字列を取得する
 */
const char *get_updated_command()
{
	static char text[4096];
	char *p;

	GetWindowText(hWndTextboxCommand, text, sizeof(text) - 1);

	/* 改行をスペースに置き換える */
	p = text;
	while(*p)
	{
		if(*p == '\r' || *p == '\n')
			*p = ' ';
		p++;
	}

	return ConvNativeToUtf8(text);
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
		/* ステータスバーを設定します */
		SendMessage(hWndStatus, SB_SETTEXT, (WPARAM)(0 | 0),
					(LPARAM)(bEnglish ?
							 "Waiting for command finish..." :
							 "コマンドの完了を待機中..."));

		/* ウィンドウのタイトルを設定する */
		SetWindowText(hWndDebug, bEnglish ?
					  "Waiting for command finish..." :
					  "コマンドの完了を待機中...");

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

		/* スクリプト選択ボタンを無効にする */
		EnableWindow(hWndBtnSelectScript, FALSE);

		/* 行番号ラベルを設定する */
		SetWindowText(hWndLabelLine, bEnglish ?
					  "Current waiting line:" :
					  "現在完了待ちの行番号:");

		/* 行番号テキストボックスを無効にする */
		EnableWindow(hWndTextboxLine, FALSE);

		/* 行番号変更ボタンを無効にする */
		EnableWindow(hWndBtnChangeLine, FALSE);

		/* コマンドラベルを設定する */
		SetWindowText(hWndLabelCommand, bEnglish ?
					  "Current waiting command:" :
					  "現在完了待ちのコマンド:");

		/* コマンドテキストボックスを無効にする */
		EnableWindow(hWndTextboxCommand, FALSE);

		/* コマンドアップデートボタンを無効にする */
		EnableWindow(hWndBtnUpdate, FALSE);

		/* コマンドリセットボタンを無効にする */
		EnableWindow(hWndBtnReset, FALSE);

		/* リストボックスを有効にする */
		EnableWindow(hWndListbox, FALSE);

		/* エラーを探すを有効にする */
		EnableWindow(hWndBtnError, TRUE);

		/* 上書き保存ボタンを無効にする */
		EnableWindow(hWndBtnSave, FALSE);

		/* 再読み込みボタンを無効にする */
		EnableWindow(hWndBtnReload, FALSE);

		/* 変数のテキストボックスを無効にする */
		EnableWindow(hWndTextboxVar, FALSE);

		/* 変数の書き込みボタンを無効にする */
		EnableWindow(hWndBtnVar, FALSE);

		/* スクリプトを開くメニューを無効にする */
		EnableMenuItem(hMenu, ID_SELECT_SCRIPT, MF_GRAYED);

		/* 上書き保存メニューを無効にする */
		EnableMenuItem(hMenu, ID_SAVE, MF_GRAYED);

		/* 続けるメニューを無効にする */
		EnableMenuItem(hMenu, ID_RESUME, MF_GRAYED);

		/* 次へメニューを無効にする */
		EnableMenuItem(hMenu, ID_NEXT, MF_GRAYED);

		/* 停止メニューを無効にする */
		EnableMenuItem(hMenu, ID_PAUSE, MF_GRAYED);

		/* 次のエラー箇所へメニューを無効にする */
		EnableMenuItem(hMenu, ID_ERROR, MF_GRAYED);

		/* 再読み込みメニューを無効にする */
		EnableMenuItem(hMenu, ID_RELOAD, MF_GRAYED);
	}
	/* 実行中のとき */
	else if(running)
	{
		/* ステータスバーを設定します */
		SendMessage(hWndStatus, SB_SETTEXT, (WPARAM)(0 | 0),
					(LPARAM)(bEnglish ?
							 "Running command..." :
							 "コマンドを実行中..."));

		/* ウィンドウのタイトルを設定する */
		SetWindowText(hWndDebug, bEnglish ? "Running..." : "実行中...");

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

		/* スクリプト選択ボタンを無効にする */
		EnableWindow(hWndBtnSelectScript, FALSE);

		/* 行番号ラベルを設定する */
		SetWindowText(hWndLabelLine, bEnglish ?
					  "Current running line:" :
					  "現在実行中の行番号:");

		/* 行番号テキストボックスを無効にする */
		EnableWindow(hWndTextboxLine, FALSE);

		/* 行番号変更ボタンを無効にする */
		EnableWindow(hWndBtnChangeLine, FALSE);

		/* コマンドラベルを設定する */
		SetWindowText(hWndLabelCommand, bEnglish ?
					  "Current running command:" :
					  "現在実行中のコマンド:");

		/* コマンドテキストボックスを無効にする */
		EnableWindow(hWndTextboxCommand, FALSE);

		/* コマンドアップデートボタンを無効にする */
		EnableWindow(hWndBtnUpdate, FALSE);

		/* コマンドリセットボタンを無効にする */
		EnableWindow(hWndBtnReset, FALSE);

		/* リストボックスを有効にする */
		EnableWindow(hWndListbox, FALSE);

		/* エラーを探すを無効にする */
		EnableWindow(hWndBtnError, FALSE);

		/* 上書きボタンを無効にする */
		EnableWindow(hWndBtnSave, FALSE);

		/* 再読み込みボタンを無効にする */
		EnableWindow(hWndBtnReload, FALSE);

		/* 変数のテキストボックスを無効にする */
		EnableWindow(hWndTextboxVar, FALSE);

		/* 変数の書き込みボタンを無効にする */
		EnableWindow(hWndBtnVar, FALSE);

		/* スクリプトを開くメニューを無効にする */
		EnableMenuItem(hMenu, ID_SELECT_SCRIPT, MF_GRAYED);

		/* 上書き保存メニューを無効にする */
		EnableMenuItem(hMenu, ID_SAVE, MF_GRAYED);

		/* 続けるメニューを無効にする */
		EnableMenuItem(hMenu, ID_RESUME, MF_GRAYED);

		/* 次へメニューを無効にする */
		EnableMenuItem(hMenu, ID_NEXT, MF_GRAYED);

		/* 停止メニューを有効にする */
		EnableMenuItem(hMenu, ID_PAUSE, MF_ENABLED);

		/* 次のエラー箇所へメニューを無効にする */
		EnableMenuItem(hMenu, ID_ERROR, MF_GRAYED);

		/* 再読み込みメニューを無効にする */
		EnableMenuItem(hMenu, ID_RELOAD, MF_GRAYED);
	}
	/* 完全に停止中のとき */
	else
	{
		/* ステータスバーを設定します */
		SendMessage(hWndStatus, SB_SETTEXT, (WPARAM)(0 | 0),
					(LPARAM)(bEnglish ? "Stopped" : "停止中"));

		/* ウィンドウのタイトルを設定する */
		SetWindowText(hWndDebug, bEnglish ? "Stopped" : "停止中");

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

		/* スクリプト選択ボタンを有効にする */
		EnableWindow(hWndBtnSelectScript, TRUE);

		/* 行番号ラベルを設定する */
		SetWindowText(hWndLabelLine, bEnglish ?
					  "Next line to be executed:" :
					  "次に実行される行番号:");

		/* 行番号テキストボックスを有効にする */
		EnableWindow(hWndTextboxLine, TRUE);

		/* 行番号変更ボタンを有効にする */
		EnableWindow(hWndBtnChangeLine, TRUE);

		/* コマンドラベルを設定する */
		SetWindowText(hWndLabelCommand, bEnglish ?
					  "Next command to be executed:" :
					  "次に実行されるコマンド:");

		/* コマンドテキストボックスを有効にする */
		EnableWindow(hWndTextboxCommand, TRUE);

		/* コマンドアップデートボタンを有効にする */
		EnableWindow(hWndBtnUpdate, TRUE);

		/* コマンドリセットボタンを有効にする */
		EnableWindow(hWndBtnReset, TRUE);

		/* リストボックスを有効にする */
		EnableWindow(hWndListbox, TRUE);

		/* エラーを探すを有効にする */
		EnableWindow(hWndBtnError, TRUE);

		/* 上書き保存ボタンを有効にする */
		EnableWindow(hWndBtnSave, TRUE);

		/* 再読み込みボタンを有効にする */
		EnableWindow(hWndBtnReload, TRUE);

		/* 変数のテキストボックスを有効にする */
		EnableWindow(hWndTextboxVar, TRUE);

		/* 変数の書き込みボタンを有効にする */
		EnableWindow(hWndBtnVar, TRUE);

		/* スクリプトを開くメニューを有効にする */
		EnableMenuItem(hMenu, ID_SELECT_SCRIPT, MF_ENABLED);

		/* 上書き保存メニューを有効にする */
		EnableMenuItem(hMenu, ID_SAVE, MF_ENABLED);

		/* 続けるメニューを有効にする */
		EnableMenuItem(hMenu, ID_RESUME, MF_ENABLED);

		/* 次へメニューを有効にする */
		EnableMenuItem(hMenu, ID_NEXT, MF_ENABLED);

		/* 停止メニューを無効にする */
		EnableMenuItem(hMenu, ID_PAUSE, MF_GRAYED);

		/* 次のエラー箇所へメニューを有効にする */
		EnableMenuItem(hMenu, ID_ERROR, MF_ENABLED);

		/* 再読み込みメニューを有効にする */
		EnableMenuItem(hMenu, ID_RELOAD, MF_ENABLED);
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
	SendMessage(hWndStatus, SB_SETTEXT, (WPARAM)(0 | 1),
				(LPARAM)get_script_file_name());

	/* 行番号を設定する */
	snprintf(line, sizeof(line), "%d", get_line_num() + 1);
	SetWindowText(hWndTextboxLine, line);
	SendMessage(hWndStatus, SB_SETTEXT, (WPARAM)(0 | 2), (LPARAM)line);

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
	top = (line_num - 9 < 0) ? 0 : (line_num - 9);
	SendMessage(hWndListbox, LB_SETCURSEL, (WPARAM)line_num, 0);
	SendMessage(hWndListbox, LB_SETTOPINDEX, (WPARAM)top, 0);

	/* 変数の情報を更新する */
	if(check_variable_updated() || script_changed)
		UpdateVariableTextBox();
}

/* 変数の情報を更新する */
static VOID UpdateVariableTextBox(void)
{
	char line[1024];
	int index;
	int val;

	szTextboxVar[0] = '\0';

	for(index = 0; index < LOCAL_VAR_SIZE + GLOBAL_VAR_SIZE; index++)
	{
		/* 変数が初期値の場合 */
		val = get_variable(index);
		if(val == 0 && !is_variable_changed(index))
			continue;

		/* 行を追加する */
		snprintf(line, sizeof(line), "$%d=%d\r\n", index, val);
		strcat(szTextboxVar, line);
	}

	/* テキストボックスにセットする */
	SetWindowText(hWndTextboxVar, szTextboxVar);
}
#endif
