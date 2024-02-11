/* -*- coding: utf-8; indent-tabs-mode: t; tab-width: 4; c-basic-offset: 4; -*- */

/*
 * Suika2
 * Copyright (C) 2001-2023, Keiichi Tabata. All rights reserved.
 */

/*
 * [Changes]
 *  2014-05-24 Created (conskit)
 *  2016-05-29 Created (suika)
 *  2017-11-07 Added a support for screen resolution change
 *  2022-06-08 Added Suika2 Pro
 *  2023-07-17 Added a support for capture/replay (later dropped)
 *  2023-09-17 Added a support the full screen mode without resolution change
 *  2023-12-09 Refactored
 *  2023-12-11 Separated winmain.c to winmain.c and winpro.c
 *  2024-01-06 Changed the language check logic.
 */

/* Suika2 Base */
#include "../suika.h"

/* Suika2 HAL Implementaions */
#include "dx9render.h"		/* Graphics HAL */
#include "dsound.h"			/* Sound HAL */
#include "dsvideo.h"		/* Video HAL */
#include "tts_sapi.h"		/* TTS HAL */

/* Windows */
#include <windows.h>
#include <shlobj.h>			/* SHGetFolderPath() */
#include "resource.h"

/* Standard C */
#include <signal.h>

/* msvcrt  */
#include <io.h> /* _access() */
#include <locale.h> /* setlocale() */

/* A macro to check whether a file exists. */
#define FILE_EXISTS(fname)	(_access(fname, 0) != -1)

/* A manifest for Windows XP control style */
#ifdef _MSC_VER
#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
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
#define FRAME_MILLI		(16)

/* 1回にスリープする時間 */
#define SLEEP_MILLI		(5)

/* UTF-8/UTF-16の変換バッファサイズ */
#define CONV_MESSAGE_SIZE	(65536)

/* ウィンドウクラス名 */
static const wchar_t wszWindowClassMain[] = L"SuikaMain";

/*
 * Variables
 */

/* ウィンドウタイトル(UTF-16) */
static wchar_t wszTitle[TITLE_BUF_SIZE];

/* メッセージ変換バッファ */
static wchar_t wszMessage[CONV_MESSAGE_SIZE];
static char szMessage[CONV_MESSAGE_SIZE];

/* Windowsオブジェクト */
static HWND hWndMain;
static HMENU hMenu;

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

/* RunFrame()が描画してよいか */
static BOOL bRunFrameAllow;

/* ストップウォッチの停止した時間 */
DWORD dwStopWatchOffset;

/* ビューポート */
static int nViewportOffsetX;
static int nViewportOffsetY;
static int nViewportWidth;
static int nViewportHeight;

/* マウス座標計算用の拡大率 */
static float fMouseScale;

/* DirectShowでビデオを再生中か */
static BOOL bDShowMode;

/* DirectShow再生中にクリックでスキップするか */
static BOOL bDShowSkippable;

/*
 * Forward Declaration
 */

/* static */
static void SIGSEGV_Handler(int);
static BOOL InitApp(HINSTANCE hInstance, int nCmdShow);
static void CleanupApp(void);
static BOOL InitWindow(HINSTANCE hInstance, int nCmdShow);
static VOID InitMenu(void);
static void GameLoop(void);
static BOOL RunFrame(void);
static BOOL SyncEvents(void);
static BOOL WaitForNextFrame(void);
static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
static int ConvertKeyCode(int nVK);
static void OnPaint(HWND hWnd);
static void OnCommand(WPARAM wParam, LPARAM lParam);
static void OnSizing(int edge, LPRECT lpRect);
static void OnSize(void);
static void UpdateScreenOffsetAndScale(int nClientWidth, int nClientHeight);
static BOOL OpenLogFile(void);
const wchar_t *conv_utf8_to_utf16(const char *utf8_message);
const char *conv_utf16_to_utf8(const wchar_t *utf16_message);

/*
 * WinMain
 */
int WINAPI wWinMain(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPWSTR lpszCmd,
	int nCmdShow)
{
	int nRet = 1;

	UNUSED_PARAMETER(hPrevInstance);
	UNUSED_PARAMETER(lpszCmd);

	signal(SIGABRT, SIGSEGV_Handler);

	do {
		/* Do the lower layer initialization. */
		if (!InitApp(hInstance, nCmdShow))
			break;

		/* Do the upper layer initialization. */
		if (!on_event_init())
			break;

		/* Run the main loop. */
		GameLoop();

		/* Do the upper layer cleanup. */
		on_event_cleanup();

		nRet = 0;
	} while (0);

	/* Do the lower layer cleanup. */
	CleanupApp();

	return nRet;
}

static void SIGSEGV_Handler(int)
{
	BOOL bEnglish;

	bEnglish = strcmp(get_system_locale(), "ja") != 0;

	log_error(bEnglish ?
			  "Sorry, the app was crashed.\n"
			  "Please send a bug report to the author." :
			  "ご迷惑をかけ申し訳ございません。\n"
			  "アプリがクラッシュしました。\n"
			  "バグ報告をいただけますと幸いです。");
	log_error(bEnglish ?
			  "Sorry, Suika2 was crashed.\n"
			  "Please send a bug report to the author.\n"
			  "You can get 1000JPY gift code if the author solved this problem." :
			  "ご迷惑をかけ申し訳ございません。\n"
			  "Suika2がクラッシュしました。\n"
			  "バグ報告をいただけますと幸いです。\n"
			  "このコード(JVXP401)をお伝えいただければギフトコード1000円分を進呈します。");
	exit(1);
}

/* 基盤レイヤの初期化処理を行う */
static BOOL InitApp(HINSTANCE hInstance, int nCmdShow)
{
	RECT rcClient;
	HRESULT hResult;

	setlocale(LC_NUMERIC, "C");

	/* COMの初期化を行う */
	hResult = CoInitialize(0);
	if (FAILED(hResult))
		return FALSE;

	if (!FILE_EXISTS("conf\\config.txt") &&
		!FILE_EXISTS("data01.arc") )
	{
		log_error(get_ui_message(UIMSG_NO_GAME_FILES));
		return FALSE;
	}

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

	/* 描画エンジンを初期化する */
	if (!D3DInitialize(hWndMain))
	{
		log_info(get_ui_message(UIMSG_WIN32_NO_DIRECT3D));
		return FALSE;
	}

	/* アスペクト比を補正する */
	GetClientRect(hWndMain, &rcClient);
	if (rcClient.right != conf_window_width || rcClient.bottom != conf_window_height)
		UpdateScreenOffsetAndScale(rcClient.right, rcClient.bottom);

	/* DirectSoundを初期化する */
	if (!DSInitialize(hWndMain))
	{
		log_error(get_ui_message(UIMSG_NO_SOUND_DEVICE));
		return FALSE;
	}

	if (conf_tts_enable)
		InitSAPI();

	if (!init_motion())
		return FALSE;

	load_motion(0, "emote_test.psb");

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
	D3DCleanup();

	/* DirectSoundの終了処理を行う */
	DSCleanup();

	/* ログファイルをクローズする */
	if(pLogFile != NULL)
		fclose(pLogFile);
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
	wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON));
	wcex.hCursor        = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground  = (HBRUSH)GetStockObject(conf_window_white ? WHITE_BRUSH : BLACK_BRUSH);
	wcex.lpszClassName  = wszWindowClassMain;
	wcex.hIconSm		= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON));
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
	MultiByteToWideChar(CP_UTF8, 0, conf_window_title, -1, wszTitle, TITLE_BUF_SIZE - 1);

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

	nWinWidth = nGameWidth + nFrameAddWidth;
	nWinHeight = nGameHeight + nFrameAddHeight;

	/* ディスプレイのサイズが足りない場合 */
	if (nVirtualScreenWidth < nWinWidth ||
		nVirtualScreenHeight < nWinHeight)
	{
		log_error(get_ui_message(UIMSG_WIN32_SMALL_DISPLAY),
				  nVirtualScreenWidth, nVirtualScreenHeight);
		return FALSE;
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
	hWndMain = CreateWindowEx(0, wszWindowClassMain, wszTitle, dwStyle,
							  nPosX, nPosY, nWinWidth, nWinHeight,
							  NULL, NULL, hInstance, NULL);
	if (hWndMain == NULL)
	{
		log_api_error("CreateWindowEx");
		return FALSE;
	}

	/* ウィンドウのサイズを調整する */
	SetRectEmpty(&rc);
	rc.right = nGameWidth;
	rc.bottom = nGameHeight;
	AdjustWindowRectEx(&rc, dwStyle,conf_window_menubar, (DWORD)GetWindowLong(hWndMain, GWL_EXSTYLE));
	SetWindowPos(hWndMain, NULL, 0, 0, rc.right - rc.left, rc.bottom - rc.top, SWP_NOZORDER | SWP_NOMOVE);
	GetWindowRect(hWndMain, &rcWindow);

	/* ゲーム用メニューを作成する */
	if(conf_window_menubar)
		InitMenu();

	/* ウィンドウを表示する */
	ShowWindow(hWndMain, nCmdShow);
	UpdateWindow(hWndMain);

	/* 0.1秒間(3フレーム)でウィンドウに関連するイベントを処理してしまう */
	dwStartTime = GetTickCount();
	for(i = 0; i < FPS / 10; i++)
		WaitForNextFrame();

	return TRUE;
}

/* メニューを初期化する */
static VOID InitMenu(void)
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
	mi.dwTypeData = _wcsdup(conv_utf8_to_utf16(get_ui_message(UIMSG_WIN32_MENU_FILE)));
	InsertMenuItem(hMenu, 0, TRUE, &mi);

	/* 表示(V)を作成する */
	mi.hSubMenu = hMenuView;
	mi.dwTypeData = _wcsdup(conv_utf8_to_utf16(get_ui_message(UIMSG_WIN32_MENU_VIEW)));
	InsertMenuItem(hMenu, 1, TRUE, &mi);

	/* 2階層目を作成する準備を行う */
	mi.fMask = MIIM_TYPE | MIIM_ID;

	/* 終了(Q)を作成する */
	mi.wID = ID_QUIT;
	mi.dwTypeData = _wcsdup(conv_utf8_to_utf16(get_ui_message(UIMSG_WIN32_MENU_QUIT)));
	InsertMenuItem(hMenuFile, 0, TRUE, &mi);

	/* フルスクリーン(S)を作成する */
	mi.wID = ID_FULLSCREEN;
	mi.dwTypeData = _wcsdup(conv_utf8_to_utf16(get_ui_message(UIMSG_WIN32_MENU_FULLSCREEN)));
	InsertMenuItem(hMenuView, 0, TRUE, &mi);

	/* メニューをセットする */
	SetMenu(hWndMain, hMenu);
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

		/* フレームを実行する */
		if (!RunFrame())
			bBreak = TRUE;
	}
}

/* フレームを実行する */
static BOOL RunFrame(void)
{
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
		if(!on_event_frame())
			return FALSE;

		return TRUE;
	}

	/* フレームの描画を開始する */
	D3DStartFrame();

	/* フレームの実行と描画を行う */
	bRet = TRUE;
	if(!on_event_frame())
	{
		/* スクリプトの終端に達した */
		bRet = FALSE;
		bRunFrameAllow = FALSE;
	}

	set_motion_scale(0, 0.5f);
	update_motion();
	render_motion();

	/* フレームの描画を終了する */
	D3DEndFrame();

	return bRet;
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

		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	/* イベント処理にかかった時間をストップウォッチから除外するようにする */
	/* dwStopWatchOffset += GetTickCount() - dwStopWatchPauseStart; */

	return TRUE;
}

/* 次のフレームの開始時刻までイベント処理とスリープを行う */
static BOOL WaitForNextFrame(void)
{
	DWORD end, lap, wait, span;

	/* 60FPSを目指す */
	span = FRAME_MILLI;

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
static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int kc;

	switch(message)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	case WM_SYSKEYDOWN:	/* Alt + Key */
		/* Alt + Enter */
		if(wParam == VK_RETURN && (HIWORD(lParam) & KF_ALTDOWN))
		{
			if (!conf_window_fullscreen_disable)
			{
				SendMessage(hWndMain, WM_SYSCOMMAND,
							(WPARAM)SC_MAXIMIZE, (LPARAM)0);
			}
			return 0;
		}

		/* Alt + F4 */
		if(wParam == VK_F4)
		{
			if (MessageBox(hWnd,
						   conv_utf8_to_utf16(get_ui_message(UIMSG_EXIT)),
						   wszTitle, MB_OKCANCEL) == IDOK)
			{
				DestroyWindow(hWnd);
				return 0;
			}
		}
		break;
	case WM_CLOSE:
		if (MessageBox(hWnd,
					   conv_utf8_to_utf16(get_ui_message(UIMSG_EXIT)),
					   wszTitle,
					   MB_OKCANCEL) == IDOK)
			DestroyWindow(hWnd);
		return 0;
	case WM_LBUTTONDOWN:
		on_event_mouse_press(MOUSE_LEFT,
							 (int)((float)(LOWORD(lParam) - nViewportOffsetX) / fMouseScale),
							 (int)((float)(HIWORD(lParam) - nViewportOffsetY) / fMouseScale));
		return 0;
	case WM_LBUTTONUP:
		on_event_mouse_release(MOUSE_LEFT,
							   (int)((float)(LOWORD(lParam) - nViewportOffsetX) / fMouseScale),
							   (int)((float)(HIWORD(lParam) - nViewportOffsetY) / fMouseScale));
		return 0;
	case WM_RBUTTONDOWN:
		on_event_mouse_press(MOUSE_RIGHT,
							 (int)((float)(LOWORD(lParam) - nViewportOffsetX) / fMouseScale),
							 (int)((float)(HIWORD(lParam) - nViewportOffsetY) / fMouseScale));
		return 0;
	case WM_RBUTTONUP:
		on_event_mouse_release(MOUSE_RIGHT,
							   (int)((float)(LOWORD(lParam) - nViewportOffsetX) / fMouseScale),
							   (int)((float)(HIWORD(lParam) - nViewportOffsetY) / fMouseScale));
		return 0;
	case WM_KEYDOWN:
		/* オートリピートの場合を除外する */
		if((HIWORD(lParam) & 0x4000) != 0)
			return 0;

		/* フルスクリーン時にESCが押された場合 */
		if((int)wParam == VK_ESCAPE && bFullScreen)
		{
			bNeedWindowed = TRUE;
			SendMessage(hWndMain, WM_SIZE, 0, 0);
			return 0;
		}

		/* Vキーが押された場合 */
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

		/* その他のキーの場合 */
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
		on_event_mouse_move((int)((float)(LOWORD(lParam) - nViewportOffsetX) / fMouseScale),
							(int)((float)(HIWORD(lParam) - nViewportOffsetY) / fMouseScale));
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
	case WM_PAINT:
		OnPaint(hWnd);
		return 0;
	case WM_COMMAND:
		OnCommand(wParam, lParam);
		return 0;
	case WM_GRAPHNOTIFY:
		if(!DShowProcessEvent())
			bDShowMode = FALSE;
		break;
	case WM_SIZING:
		if (conf_window_resize)
		{
			OnSizing((int)wParam, (LPRECT)lParam);
			return TRUE;
		}
		return FALSE;
	case WM_SIZE:
		OnSize();
		return 0;
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
	RunFrame();
	EndPaint(hWnd, &ps);

	UNUSED_PARAMETER(hDC);
}

/* WM_COMMANDを処理する */
static void OnCommand(WPARAM wParam, LPARAM lParam)
{
	UINT nID;

	UNUSED_PARAMETER(lParam);

	nID = LOWORD(wParam);
	switch(nID)
	{
	case ID_QUIT:
		PostMessage(hWndMain, WM_CLOSE, 0, 0);
		break;
	case ID_FULLSCREEN:
		if (!conf_window_fullscreen_disable)
		{
			SendMessage(hWndMain, WM_SYSCOMMAND,
						(WPARAM)SC_MAXIMIZE, (LPARAM)0);
		}
		break;
	default:
		break;
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

	/* Apply adjustments.*/
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

	/* If there's a size change, update the viewport size. */
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

	if (D3DIsSoftRendering())
	{
		nViewportWidth = conf_window_width;
		nViewportHeight = conf_window_height;
		nViewportOffsetX = 0;
		nViewportOffsetY = 0;
		fMouseScale = 1.0f;
		return;
	}

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
	nViewportOffsetX = (int)((((float)nClientWidth - fUseWidth) / 2.0f) + 0.5);
	nViewportOffsetY = (int)((((float)nClientHeight - fUseHeight) / 2.0f) + 0.5);

	/* Save the viewport size. */
	nViewportWidth = (int)fUseWidth;
	nViewportHeight = (int)fUseHeight;

	/* Update the screen offset and scale for drawing subsystem. */
	D3DResizeWindow(nViewportOffsetX, nViewportOffsetY, fMouseScale);
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

	/* 必要ならログファイルをオープンする */
	if(!OpenLogFile())
		return false;

	/* メッセージボックスを表示する */
	va_start(ap, s);
	vsnprintf(buf, sizeof(buf), s, ap);
	va_end(ap);

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
			MessageBox(NULL, conv_utf8_to_utf16(get_ui_message(UIMSG_CANNOT_OPEN_LOG)),
					   wszTitle, MB_OK | MB_ICONWARNING);
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
			MessageBox(NULL, conv_utf8_to_utf16(get_ui_message(UIMSG_CANNOT_OPEN_LOG)),
					   wszTitle, MB_OK | MB_ICONERROR);
			return FALSE;
		}
	}

	/* 成功 */
	return TRUE;
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
 * タイマをリセットする
 */
void reset_lap_timer(uint64_t *origin)
{
	*origin = GetTickCount();
	dwStopWatchOffset = 0;
}

/*
 * タイマのラップを秒単位で取得する
 */
uint64_t get_lap_timer_millisec(uint64_t *origin)
{
	DWORD dwCur = GetTickCount();
	return (uint64_t)(dwCur - *origin - dwStopWatchOffset);
}

/*
 * 終了ダイアログを表示する
 */
bool exit_dialog(void)
{
	if (MessageBox(hWndMain,
				   conv_utf8_to_utf16(get_ui_message(UIMSG_EXIT)),
				   wszTitle, MB_OKCANCEL) == IDOK)
		return true;
	return false;
}

/*
 * タイトルに戻るダイアログを表示する
 */
bool title_dialog(void)
{
	if (MessageBox(hWndMain,
				   conv_utf8_to_utf16(get_ui_message(UIMSG_TITLE)),
				   wszTitle, MB_OKCANCEL) == IDOK)
		return true;
	return false;
}

/*
 * 削除ダイアログを表示する
 */
bool delete_dialog(void)
{
	if (MessageBox(hWndMain,
				   conv_utf8_to_utf16(get_ui_message(UIMSG_DELETE)),
				   wszTitle, MB_OKCANCEL) == IDOK)
		return true;
	return false;
}

/*
 * 上書きダイアログを表示する
 */
bool overwrite_dialog(void)
{
	if (MessageBox(hWndMain,
				   conv_utf8_to_utf16(get_ui_message(UIMSG_OVERWRITE)),
				   wszTitle, MB_OKCANCEL) == IDOK)
		return true;
	return false;
}

/*
 * 初期設定ダイアログを表示する
 */
bool default_dialog(void)
{
	if (MessageBox(hWndMain,
				   conv_utf8_to_utf16(get_ui_message(UIMSG_DEFAULT)),
				   wszTitle, MB_OKCANCEL) == IDOK)
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
	BOOL ret = DShowPlayVideo(hWndMain, path, nViewportOffsetX, nViewportOffsetY, nViewportWidth, nViewportHeight);
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
	wchar_t wszNewTitle[1024];
	wchar_t wszTmp[1024];
	const char *separator;
	const char *chapter;

	ZeroMemory(&wszNewTitle[0], sizeof(wszTitle));
	ZeroMemory(&wszTmp[0], sizeof(wszTmp));

	/* コンフィグのウィンドウタイトルをUTF-8からUTF-16に変換する */
	wcscpy(wszNewTitle, conv_utf8_to_utf16(conf_window_title));

	/* セパレータを取得する */
	separator = conf_window_title_separator;
	if (separator == NULL)
		separator = " ";

	/* 章タイトルを取得する */
	chapter = get_chapter_name();

	if (!conf_window_title_chapter_disable && strcmp(chapter, "") != 0)
	{
		/* セパレータを連結する */
		wcscat(wszNewTitle, conv_utf8_to_utf16(separator));

		/* 章タイトルを連結する */
		wcscat(wszNewTitle, conv_utf8_to_utf16(chapter));
	}

	/* ウィンドウのタイトルを設定する */
	SetWindowText(hWndMain, wszNewTitle);
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
	DWORD dwLang = GetUserDefaultLCID() & 0x3ff;
	switch (dwLang) {
	case LANG_ENGLISH:
		return "en";
	case LANG_FRENCH:
		return "fr";
	case LANG_GERMAN:
		return "de";
	case LANG_SPANISH:
		return "es";
	case LANG_ITALIAN:
		return "it";
	case LANG_GREEK:
		return "el";
	case LANG_RUSSIAN:
		return "ru";
	case LANG_CHINESE_SIMPLIFIED:
		return "zh";
	case LANG_CHINESE_TRADITIONAL:
		return "tw";
	case LANG_JAPANESE:
		return "ja";
	default:
		break;
	}
	return "other";
}

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
