/* -*- coding: utf-8; indent-tabs-mode: t; tab-width: 4; c-basic-offset: 4; -*- */

/*
 * Suika2
 * Copyright (C) 2001-2024, Keiichi Tabata. All rights reserved.
 */

/*
 * Suika2 Pro
 *
 * [Changes]
 *  2014-05-24 Created (conskit)
 *  2016-05-29 Created (suika)
 *  2017-11-07 Added a support for screen resolution change
 *  2022-06-08 Added Suika2 Pro
 *  2023-07-17 Added a support for capture/replay (later dropped)
 *  2023-09-17 Added a support the full screen mode without resolution change
 *  2023-09-20 Added the iOS and Android export functions
 *  2023-10-25 Added the editor feature
 *  2023-12-05 Added a support for project files
 *  2023-12-09 Refactored
 *  2023-12-11 Separated winmain.c to winmain.c and winpro.c
 *  2024-01-04 Added the dark mode and the preference save/load.
 *  2024-01-06 Changed the language check logic.
 */

/* Suika2 Base */
#include "../suika.h"

/* Suika2 Pro */
#include "../pro.h"
#include "../package.h"

/* Suika2 HAL Implementaions */
#include "dx9render.h"		/* Graphics HAL */
#include "dsound.h"			/* Sound HAL */
#include "dsvideo.h"		/* Video HAL */

/* Windows */
#include <windows.h>
#include <shlobj.h>			/* SHGetFolderPath() */
#include <commctrl.h>		/* TOOLINFO */
#include <richedit.h>		/* RichEdit */
#include "resource.h"

/* msvcrt  */
#include <io.h>				/* _access() */
#define wcsdup(s)	_wcsdup(s)

/* A macro to check whether a file exists. */
#define FILE_EXISTS(fname)	(_access(fname, 0) != -1)

/* A manifest for Windows XP control style */
#ifdef _MSC_VER
#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif

/*
 * Constants
 */

/* The window title of message boxes. */
#define TITLE			L"Suika2 Pro"

/* The font name for the controls. */
#define CONTROL_FONT	L"Yu Gothic UI"

/* The font name for the script view. */
#define SCRIPT_FONT_JP		L"BIZ UDゴシック"
#define SCRIPT_FONT_EN		L"Courier New"

/* The version string. */
#define VERSION			\
	L"Suika2 Pro Desktop\n" \
	L"Copyright (c) 2001-2024, Keiichi Tabata. All rights reserved."

/* The minimum window size. */
#define WINDOW_WIDTH_MIN	(800)
#define WINDOW_HEIGHT_MIN	(600)

/* The width of the editor panel. */
#define EDITOR_WIDTH		(440)

/* フレームレート */
#define FPS				(30)

/* 1フレームの時間 */
#define FRAME_MILLI		(33)

/* 1回にスリープする時間 */
#define SLEEP_MILLI		(5)

/* UTF-8/UTF-16の変換バッファサイズ */
#define CONV_MESSAGE_SIZE	(65536)

/* タイマー */
#define ID_TIMER_BEGINNER	(1)
#define ID_TIMER_FORMAT		(2)

/* Colors */
#define LIGHT_BG_DEFAULT	0x00ffffff
#define LIGHT_FG_DEFAULT	0x00000000
#define LIGHT_COMMENT		0x00808080
#define LIGHT_LABEL			0x00ff0000
#define LIGHT_ERROR			0x000000ff
#define LIGHT_COMMAND_NAME	0x00ff0000
#define LIGHT_PARAM_NAME	0x00c0f0c0
#define LIGHT_NEXT_EXEC		0x00ffc0c0
#define LIGHT_CURRENT_EXEC	0x00c0c0ff
#define LIGHT_SELECTED		0x0033ccff
#define DARK_BG_DEFAULT		0x00282828
#define DARK_FG_DEFAULT		0x00ffffff
#define DARK_COMMENT		0x00808080
#define DARK_LABEL			0x006200ee
#define DARK_ERROR			0x000000ff
#define DARK_COMMAND_NAME	0x0060a0a0
#define DARK_PARAM_NAME		0x00e0acac
#define DARK_NEXT_EXEC		0x00a0e070
#define DARK_CURRENT_EXEC	0x00b0b0e0
#define DARK_SELECTED		0x00282828

/* 変数テキストボックスのテキストの最大長(形: "$00001=12345678901\r\n") */
#define VAR_TEXTBOX_MAX		(11000 * (1 + 5 + 1 + 11 + 2))

/* Window class names */
static const wchar_t wszWindowClassMainWindow[] = L"SuikaMainWindow";
static const wchar_t wszWindowClassRenderingPanel[] = L"SuikaRenderingPanel";
static const wchar_t wszWindowClassEditorPanel[] = L"SuikaEditorPanel";

/*
 * Variables
 */

/* Windows objects */
static HWND hWndMain;				/* メインウィンドウ */
static HWND hWndRender;				/* レンダリング領域のパネル */
static HWND hWndEditor;				/* エディタ部分のパネル */
static HWND hWndBtnResume;			/* 「続ける」ボタン */
static HWND hWndBtnNext;			/* 「次へ」ボタン */
static HWND hWndBtnPause;			/* 「停止」ボタン */
static HWND hWndBtnMove;			/* 「移動」ボタン */
static HWND hWndTextboxScript;		/* ファイル名のテキストボックス */
static HWND hWndBtnSelectScript;	/* ファイル選択のボタン */
static HWND hWndRichEdit;			/* スクリプトのリッチエディット */
static HWND hWndTextboxVar;			/* 変数一覧のテキストボックス */
static HWND hWndBtnVar;				/* 変数を反映するボタン */
static HMENU hMenu;					/* ウィンドウのメニュー */
static HMENU hMenuPopup;			/* ポップアップメニュー */

/* メッセージ変換バッファ */
static wchar_t wszMessage[CONV_MESSAGE_SIZE];
static char szMessage[CONV_MESSAGE_SIZE];

/* WaitForNextFrame()の時間管理用 */
static DWORD dwStartTime;

/* プロジェクトが読み込まれた */
static BOOL bProjectOpened;

/* フルスクリーンモードか */
static BOOL bFullScreen;

/* フルスクリーンモードに移行する必要があるか */
static BOOL bNeedFullScreen;

/* ウィンドウモードに移行する必要があるか */
static BOOL bNeedWindowed;

/* ウィンドウモードでのスタイル */
static DWORD dwStyle;
static DWORD dwExStyle;

/* ウィンドウモードでの位置 */
static RECT rcWindow;

/* 最後に設定されたウィンドウサイズとDPI */
static int nLastClientWidth, nLastClientHeight, nLastDpi;

/* RunFrame()が描画してよいか */
static BOOL bRunFrameAllow;

/* フルスクリーンモード時の描画オフセット */
static int nOffsetX;
static int nOffsetY;
static float fMouseScale;

/* DirectShowでビデオを再生中か */
static BOOL bDShowMode;

/* DirectShow再生中にクリックでスキップするか */
static BOOL bDShowSkippable;

/* 英語モードか */
static BOOL bEnglish;

/* 実行中であるか */
static BOOL bRunning;

/* ハイライトモードか */
static BOOL bHighlightMode;

/* ダークモードか */
static BOOL bDarkMode;

/* 発生したイベントの状態 */
static BOOL bContinuePressed;		/* 「続ける」ボタンが押下された */
static BOOL bNextPressed;			/* 「次へ」ボタンが押下された */
static BOOL bStopPressed;			/* 「停止」ボタンが押下された */
static BOOL bScriptOpened;			/* スクリプトファイルが選択された */
static BOOL bExecLineChanged;		/* 実行行が変更された */
static int nLineChanged;			/* 実行行が変更された場合の行番号 */
static BOOL bIgnoreChange;			/* リッチエディットへの変更を無視する */
static BOOL bNeedUpdateVars;		/* 変数が */

/* Colors */
static DWORD dwColorBgDefault = LIGHT_BG_DEFAULT;
static DWORD dwColorFgDefault = LIGHT_FG_DEFAULT;
static DWORD dwColorComment = LIGHT_COMMENT;
static DWORD dwColorLabel = LIGHT_LABEL;
static DWORD dwColorError = LIGHT_ERROR;
static DWORD dwColorCommandName = LIGHT_COMMAND_NAME;
static DWORD dwColorParamName = LIGHT_PARAM_NAME;
static DWORD dwColorNextExec = LIGHT_NEXT_EXEC;
static DWORD dwColorCurrentExec = LIGHT_CURRENT_EXEC;

/* The font name for the script view. */
static wchar_t wszFontName[128];

/* The font size for the script view. */
static int nFontSize = 10;

/*
 * Forward Declaration
 */

/*
 * Forward Declaration
 */

/* static */
static BOOL InitApp(HINSTANCE hInstance, int nCmdShow);
static void CleanupApp(void);
static BOOL InitWindow(HINSTANCE hInstance, int nCmdShow);
static BOOL InitRenderingPanel(HINSTANCE hInstance, int nWidth, int nHeight);
static BOOL InitMainWindow(HINSTANCE hInstance, int *pnRenderWidth, int *pnRenderHeight);
static BOOL InitEditorPanel(HINSTANCE hInstance);
static VOID InitMenu(HWND hWnd);
static HWND CreateTooltip(HWND hWndBtn, const wchar_t *pszTextEnglish, const wchar_t *pszTextJapanese);
static VOID StartGame(void);
static VOID GameLoop(void);
static BOOL RunFrame(void);
static BOOL SyncEvents(void);
static BOOL PretranslateMessage(MSG* pMsg);
static BOOL WaitForNextFrame(void);
static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
static int ConvertKeyCode(int nVK);
static void OnPaint(HWND hWnd);
static void OnCommand(WPARAM wParam, LPARAM lParam);
static void OnSizing(int edge, LPRECT lpRect);
static void OnSize(void);
static void OnDpiChanged(HWND hWnd, UINT nDpi, LPRECT lpRect);
static void Layout(int nClientWidth, int nClientHeight);
const wchar_t *conv_utf8_to_utf16(const char *utf8_message);
const char *conv_utf16_to_utf8(const wchar_t *utf16_message);

/* TextEdit (for Variables) */
static VOID Variable_UpdateText(void);

/* RichEdit */
static VOID RichEdit_OnChange(void);
static VOID RichEdit_SetFont(void);
static int RichEdit_GetCursorPosition(void);
static int RichEdit_GetSelectedLen(void);
static VOID RichEdit_SetCursorPosition(int nCursor);
static VOID RichEdit_SetSelectedRange(int nLineStart, int nLineLen);
static int RichEdit_GetCursorLine(void);
static wchar_t *RichEdit_GetText(void);
static VOID RichEdit_SetTextColorForAllLines(void);
static VOID RichEdit_SetTextColorForLine(const wchar_t *pText, int nLineStartCR, int nLineStartCRLF, int nLineLen);
static VOID RichEdit_ClearFormatAll(void);
static VOID RichEdit_ClearBackgroundColorAll(void);
static VOID RichEdit_SetBackgroundColorForNextExecuteLine(void);
static VOID RichEdit_SetBackgroundColorForCurrentExecuteLine(void);
static VOID RichEdit_GetLineStartAndLength(int nLine, int *nLineStart, int *nLineLen);
static VOID RichEdit_SetTextColorForSelectedRange(COLORREF cl);
static VOID RichEdit_SetBackgroundColorForSelectedRange(COLORREF cl);
static VOID RichEdit_AutoScroll(void);
static VOID RichEdit_GetLineStartAndLength(int nLine, int *nLineStart, int *nLineLen);
static BOOL RichEdit_SearchNextError(int nStart, int nEnd);
static VOID RichEdit_SetTextByScriptModel(void);
static VOID RichEdit_UpdateScriptModelFromText(void);
static VOID RichEdit_InsertText(const wchar_t *pLine, ...);
static VOID RichEdit_UpdateTheme(void);
static VOID RichEdit_DelayedHighligth(void);
static VOID __stdcall OnTimerFormat(HWND hWnd, UINT nID, UINT_PTR uTime, DWORD dwParam);

/* Project */
static VOID __stdcall OnTimerBeginner(HWND hWnd, UINT nID, UINT_PTR uTime, DWORD dwParam);
static BOOL CreateProjectFromTemplate(void);
static BOOL ChooseProject(void);
static BOOL OpenProjectAtPath(const wchar_t *pszPath);
static VOID ReadProjectFile(void);
static VOID WriteProjectFile(void);

/* Command Handlers */
static VOID OnNewProject(void);
static VOID OnOpenProject(void);
static VOID OnOpenGameFolder(void);
static VOID OnOpenScript(void);
static VOID OnReloadScript(void);
static const wchar_t *SelectFile(const char *pszDir);
static VOID OnContinue(void);
static VOID OnNext(void);
static VOID OnStop(void);
static VOID OnMove(void);
static VOID OnReturn(void);
static VOID OnSave(void);
static VOID OnNextError(void);
static VOID OnPopup(void);
static VOID OnWriteVars(void);
static VOID OnExportPackage(void);
static VOID OnExportWin(void);
static VOID OnExportWinInst(void);
static VOID OnExportWinMac(void);
static VOID OnExportWeb(void);
static VOID OnExportAndroid(void);
static VOID OnExportIOS(void);
static VOID OnFont(void);
static VOID OnHighlightMode(void);
static VOID OnDarkMode(void);

/* Export Helpers */
static VOID RecreateDirectory(const wchar_t *path);
static BOOL CopyLibraryFiles(const wchar_t* lpszSrcDir, const wchar_t* lpszDestDir);
static BOOL CopyGameFiles(const wchar_t* lpszSrcDir, const wchar_t* lpszDestDir);
static BOOL CopyMovFiles(const wchar_t *lpszSrcDir, const wchar_t *lpszDestDir);
static BOOL MovePackageFile(const wchar_t *lpszPkgFile, wchar_t *lpszDestDir);

/* Command Insertion */
static VOID OnInsertMessage(void);
static VOID OnInsertSerif(void);
static VOID OnInsertBg(void);
static VOID OnInsertBgOnly(void);
static VOID OnInsertCh(void);
static VOID OnInsertChsx(void);
static VOID OnInsertBgm(void);
static VOID OnInsertBgmStop(void);
static VOID OnInsertVolBgm(void);
static VOID OnInsertSe(void);
static VOID OnInsertSeStop(void);
static VOID OnInsertVolSe(void);
static VOID OnInsertVideo(void);
static VOID OnInsertShakeH(void);
static VOID OnInsertShakeV(void);
static VOID OnInsertChoose3(void);
static VOID OnInsertChoose2(void);
static VOID OnInsertChoose1(void);
static VOID OnInsertGui(void);
static VOID OnInsertClick(void);
static VOID OnInsertWait(void);
static VOID OnInsertLoad(void);

/*
 * 初期化
 */

/*
 * WinMain
 */
int WINAPI wWinMain(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPWSTR lpszCmd,
	int nCmdShow)
{
	int nRet;

	nRet = 1;
	do {
		/* Decide Japanese or English. */
		bEnglish = (GetUserDefaultLCID() & 0x3ff) == LANG_JAPANESE ? FALSE : TRUE;

		/* Initialize the app. */
		if(!InitApp(hInstance, nCmdShow))
			break;

		/* Run the main loop. */
		GameLoop();

		/* Cleanup the app. */
		CleanupApp();

		nRet = 0;
	} while (0);

	UNUSED_PARAMETER(hPrevInstance);
	UNUSED_PARAMETER(lpszCmd);

	return nRet;
}

/* Do the lower layer initialization. */
static BOOL InitApp(HINSTANCE hInstance, int nCmdShow)
{
	conf_window_width = 1280;
	conf_window_height = 720;
	conf_window_title = strdup("Suika2 Pro Desktop");

	/* Initialize the window. */
	if (!InitWindow(hInstance, nCmdShow))
		return FALSE;

	/* Initialize the graphics HAL. */
	if (!D3DInitialize(hWndRender))
	{
		log_error(get_ui_message(UIMSG_WIN32_NO_DIRECT3D));
		return FALSE;
	}

	/* Initialize the sound HAL. */
	if (!DSInitialize(hWndMain))
	{
		log_error(get_ui_message(UIMSG_NO_SOUND_DEVICE));
		return FALSE;
	}

	/* Open a project file if specified in argv[1]. */
	if (__argc >= 2)
		if (OpenProjectAtPath(__wargv[1]))
			StartGame();

	/* Set a timer for a beginner. */
	if (!bProjectOpened)
		SetTimer(hWndMain, ID_TIMER_BEGINNER, 5000, OnTimerBeginner);

	return TRUE;
}

/* 基盤レイヤの終了処理を行う */
static void CleanupApp(void)
{
	if (bProjectOpened)
	{
		on_event_cleanup();
		cleanup_conf();
		cleanup_file();
	}

	/* Direct3Dの終了処理を行う */
	D3DCleanup();

	/* DirectSoundの終了処理を行う */
	DSCleanup();
}

/*
 * A wrapper for GetDpiForWindow().
 */
int Win11_GetDpiForWindow(HWND hWnd)
{
	static UINT (__stdcall *pGetDpiForWindow)(HWND) = NULL;
	UINT nDpi;

	if (pGetDpiForWindow == NULL)
	{
		HMODULE hModule = LoadLibrary(L"user32.dll");
		if (hModule == NULL)
			return 96;

		pGetDpiForWindow = (void *)GetProcAddress(hModule, "GetDpiForWindow");
		if (pGetDpiForWindow == NULL)
			return 96;
	}

	nDpi = pGetDpiForWindow(hWnd);
	if (nDpi == 0)
		return 96;

	return (int)nDpi;
}

/* Initialize the window. */
static BOOL InitWindow(HINSTANCE hInstance, int nCmdShow)
{
	HRESULT hResult;
	int i, nRenderWidth, nRenderHeight;

	/* Initialize COM. */
	hResult = CoInitialize(0);
	if (FAILED(hResult))
	{
		log_api_error("CoInitialize");
		return FALSE;
	}

	/* Initialize the Common Controls. */
	InitCommonControls();

	/* Initialize the main window. */
	if (!InitMainWindow(hInstance, &nRenderWidth, &nRenderHeight))
		return FALSE;

	/* Initialize the rendering panel. */
	if (!InitRenderingPanel(hInstance, nRenderWidth, nRenderWidth))
		return FALSE;

	/* Initialize the editor panel. */
	if (!InitEditorPanel(hInstance))
		return FALSE;

	/* Initialize the menu. */
	InitMenu(hWndMain);

	/* Show the window. */
	ShowWindow(hWndMain, nCmdShow);
	UpdateWindow(hWndMain);

	/* Process events during a 0.1 second. */
	dwStartTime = GetTickCount();
	for(i = 0; i < FPS / 10; i++)
		WaitForNextFrame();

	return TRUE;
}

static BOOL InitMainWindow(HINSTANCE hInstance, int *pnRenderWidth, int *pnRenderHeight)
{
	wchar_t wszTitle[1024];
	WNDCLASSEX wcex;
	RECT rc;
	int nVirtualScreenWidth, nVirtualScreenHeight;
	int nFrameAddWidth, nFrameAddHeight;
	int nMonitors;
	int nRenderWidth, nRenderHeight;
	int nWinWidth, nWinHeight;
	int nPosX, nPosY;

	/* ウィンドウクラスを登録する */
	ZeroMemory(&wcex, sizeof(wcex));
	wcex.cbSize			= sizeof(WNDCLASSEX);
	wcex.lpfnWndProc    = WndProc;
	wcex.hInstance      = hInstance;
	wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON));
	wcex.hCursor        = LoadCursor(NULL, IDC_ARROW);
	wcex.lpszClassName  = wszWindowClassMainWindow;
	wcex.hIconSm		= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON));
	if (!RegisterClassEx(&wcex))
		return FALSE;

	/* ウィンドウのスタイルを決める */
	dwStyle = WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_OVERLAPPED | WS_THICKFRAME;

	/* フレームのサイズを取得する */
	nFrameAddWidth = GetSystemMetrics(SM_CXFIXEDFRAME) * 2;
	nFrameAddHeight = GetSystemMetrics(SM_CYCAPTION) +
					  GetSystemMetrics(SM_CYMENU) +
					  GetSystemMetrics(SM_CYFIXEDFRAME) * 2;

	/* ウィンドウのタイトルをUTF-8からUTF-16に変換する */
	MultiByteToWideChar(CP_UTF8, 0, conf_window_title, -1, wszTitle, sizeof(wszTitle) / sizeof(wchar_t) - 1);

	/* モニタの数を取得する */
	nMonitors = GetSystemMetrics(SM_CMONITORS);

	/* ウィンドウのサイズをコンフィグから取得する */
	if (conf_window_resize &&
		conf_window_default_width > 0 &&
		conf_window_default_height > 0)
	{
		nRenderWidth = conf_window_default_width;
		nRenderHeight = conf_window_default_height;
	}
	else
	{
		nRenderWidth = conf_window_width;
		nRenderHeight = conf_window_height;
	}

	/* Get the display size. */
	nVirtualScreenWidth = GetSystemMetrics(SM_CXVIRTUALSCREEN);
	nVirtualScreenHeight = GetSystemMetrics(SM_CYVIRTUALSCREEN);

	/* Calc the window size. */
	nWinWidth = nRenderWidth + nFrameAddWidth + EDITOR_WIDTH;
	nWinHeight = nRenderHeight + nFrameAddHeight;

	/* If the display size is smaller than the window size in the config: */
	if (nVirtualScreenWidth < conf_window_width ||
		nVirtualScreenHeight < conf_window_height)
	{
		nWinWidth = nVirtualScreenWidth;
		nWinHeight = nVirtualScreenHeight;
		nRenderWidth = nWinWidth - EDITOR_WIDTH;
		nRenderHeight = nWinHeight;
	}

	/* Center the window if not multi-display environment. */
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
	hWndMain = CreateWindowEx(0, wszWindowClassMainWindow, wszTitle,
							  dwStyle, nPosX, nPosY, nWinWidth, nWinHeight,
							  NULL, NULL, hInstance, NULL);
	if (hWndMain == NULL)
	{
		log_api_error("CreateWindowEx");
		return FALSE;
	}

	/* ウィンドウのサイズを調整する */
	SetRectEmpty(&rc);
	rc.right = nRenderWidth;
	rc.bottom = nRenderHeight;
	AdjustWindowRectEx(&rc, dwStyle, TRUE, (DWORD)GetWindowLong(hWndMain, GWL_EXSTYLE));
	SetWindowPos(hWndMain, NULL, 0, 0, rc.right - rc.left, rc.bottom - rc.top, SWP_NOZORDER | SWP_NOMOVE);
	GetWindowRect(hWndMain, &rcWindow);

	*pnRenderWidth = nRenderWidth;
	*pnRenderHeight = nRenderHeight;

	return TRUE;
}

/* Initialize the rendering panel. */
static BOOL InitRenderingPanel(HINSTANCE hInstance, int nWidth, int nHeight)
{
	WNDCLASSEX wcex;

	ZeroMemory(&wcex, sizeof(wcex));
	wcex.cbSize			= sizeof(WNDCLASSEX);
	wcex.lpfnWndProc    = WndProc;
	wcex.hInstance      = hInstance;
	wcex.hbrBackground  = (HBRUSH)GetStockObject(conf_window_white ? WHITE_BRUSH : BLACK_BRUSH);
	wcex.lpszClassName  = wszWindowClassRenderingPanel;
	if (!RegisterClassEx(&wcex))
		return FALSE;

	hWndRender = CreateWindowEx(0, wszWindowClassRenderingPanel, NULL,
							  WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS,
							  0, 0, nWidth, nHeight,
							  hWndMain, NULL, hInstance, NULL);
	if (hWndRender == NULL)
	{
		log_api_error("CreateWindowEx");
		return FALSE;
	}

	return TRUE;
}

/* Initialize the editor panel. */
static BOOL InitEditorPanel(HINSTANCE hInstance)
{
	wchar_t wszCls[128];
	WNDCLASSEX wcex;
	RECT rcClient;
	HFONT hFont, hFontFixed;
	int nDpi;

	/* 領域の矩形を取得する */
	GetClientRect(hWndMain, &rcClient);

	/* ウィンドウクラスを登録する */
	ZeroMemory(&wcex, sizeof(wcex));
	wcex.cbSize			= sizeof(WNDCLASSEX);
	wcex.hInstance      = hInstance;
	wcex.lpfnWndProc    = WndProc;
	wcex.hbrBackground  = (HBRUSH)(COLOR_BTNFACE + 1);
	wcex.lpszClassName  = wszWindowClassEditorPanel;
	if (!RegisterClassEx(&wcex))
		return FALSE;

	/* ウィンドウを作成する */
	hWndEditor = CreateWindowEx(0, wszWindowClassEditorPanel,
								NULL,
								WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS,
								rcClient.right - EDITOR_WIDTH, 0,
								EDITOR_WIDTH, rcClient.bottom,
								hWndMain, NULL, GetModuleHandle(NULL), NULL);
	if(!hWndEditor)
		return FALSE;

	/* DPIを取得する */
	nDpi = Win11_GetDpiForWindow(hWndMain);

	/* フォントを作成する */
	wcscpy(wszFontName, bEnglish ? SCRIPT_FONT_EN : SCRIPT_FONT_JP);
	hFont = CreateFont(MulDiv(18, nDpi, 96), 0, 0, 0, FW_DONTCARE, FALSE,
					   FALSE, FALSE, ANSI_CHARSET, OUT_TT_PRECIS,
					   CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
					   DEFAULT_PITCH | FF_DONTCARE,
					   CONTROL_FONT);
	hFontFixed = CreateFont(MulDiv(nFontSize, nDpi, 96), 0, 0, 0,
							FW_DONTCARE, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
							OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS,
							DEFAULT_QUALITY, FIXED_PITCH | FF_DONTCARE,
							wszFontName);

	/* 続けるボタンを作成する */
	hWndBtnResume = CreateWindow(
		L"BUTTON",
		bEnglish ? L"Resume" : L"続ける",
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
		MulDiv(10, nDpi, 96),
		MulDiv(10, nDpi, 96),
		MulDiv(100, nDpi, 96),
		MulDiv(40, nDpi, 96),
		hWndEditor,
		(HMENU)ID_RESUME,
		hInstance,
		NULL);
	SendMessage(hWndBtnResume, WM_SETFONT, (WPARAM)hFont, (LPARAM)TRUE);
	EnableWindow(hWndBtnResume, FALSE);
	CreateTooltip(hWndBtnResume,
				  L"Start executing script and run continuosly.",
				  L"スクリプトの実行を開始し、継続して実行します。");

	/* 次へボタンを作成する */
	hWndBtnNext = CreateWindow(
		L"BUTTON",
		bEnglish ? L"Next" : L"次へ",
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
		MulDiv(120, nDpi, 96),
		MulDiv(10, nDpi, 96),
		MulDiv(100, nDpi, 96),
		MulDiv(40, nDpi, 96),
		hWndEditor,
		(HMENU)ID_NEXT,
		hInstance,
		NULL);
	SendMessage(hWndBtnNext, WM_SETFONT, (WPARAM)hFont, (LPARAM)TRUE);
	EnableWindow(hWndBtnNext, FALSE);
	CreateTooltip(hWndBtnNext,
				  L"Run only one command and stop after it.",
				  L"コマンドを1つだけ実行し、停止します。");

	/* 停止ボタンを作成する */
	hWndBtnPause = CreateWindow(
		L"BUTTON",
		bEnglish ? L"Paused" : L"停止",
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
		MulDiv(330, nDpi, 96),
		MulDiv(10, nDpi, 96),
		MulDiv(100, nDpi, 96),
		MulDiv(40, nDpi, 96),
		hWndEditor,
		(HMENU)ID_PAUSE,
		hInstance,
		NULL);
	EnableWindow(hWndBtnPause, FALSE);
	SendMessage(hWndBtnPause, WM_SETFONT, (WPARAM)hFont, (LPARAM)TRUE);
	EnableWindow(hWndBtnPause, FALSE);
	CreateTooltip(hWndBtnPause,
				  L"Stop script execution.",
				  L"コマンドの実行を停止します。");

	/* 移動ボタンを作成する */
	hWndBtnMove = CreateWindow(
		L"BUTTON",
		bEnglish ? L"Move" : L"移動",
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
		MulDiv(220, nDpi, 96),
		MulDiv(10, nDpi, 96),
		MulDiv(100, nDpi, 96),
		MulDiv(40, nDpi, 96),
		hWndEditor,
		(HMENU)ID_MOVE,
		hInstance,
		NULL);
	EnableWindow(hWndBtnMove, TRUE);
	SendMessage(hWndBtnMove, WM_SETFONT, (WPARAM)hFont, (LPARAM)TRUE);
	EnableWindow(hWndBtnMove, FALSE);
	CreateTooltip(hWndBtnMove,
				  L"Move to the cursor line and run only one command.",
				  L"カーソル行に移動してコマンドを1つだけ実行します。");

	/* スクリプト名のテキストボックスを作成する */
	hWndTextboxScript = CreateWindow(
		L"EDIT",
		NULL,
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | WS_BORDER | ES_READONLY | ES_AUTOHSCROLL,
		MulDiv(10, nDpi, 96),
		MulDiv(60, nDpi, 96),
		MulDiv(350, nDpi, 96),
		MulDiv(30, nDpi, 96),
		hWndEditor,
		0,
		hInstance,
		NULL);
	SendMessage(hWndTextboxScript, WM_SETFONT, (WPARAM)hFont, (LPARAM)TRUE);
	CreateTooltip(hWndTextboxScript,
				  L"Write script file name to be jumped to.",
				  L"ジャンプしたいスクリプトファイル名を書きます。");

	/* スクリプトの選択ボタンを作成する */
	hWndBtnSelectScript = CreateWindow(
		L"BUTTON", L"...",
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
		MulDiv(370, nDpi, 96),
		MulDiv(60, nDpi, 96),
		MulDiv(60, nDpi, 96),
		MulDiv(30, nDpi, 96),
		hWndEditor,
		(HMENU)ID_OPEN,
		hInstance,
		NULL);
	SendMessage(hWndBtnSelectScript, WM_SETFONT, (WPARAM)hFont, (LPARAM)TRUE);
	CreateTooltip(hWndBtnSelectScript,
				  L"Select a script file and jump to it.",
				  L"スクリプトファイルを選択してジャンプします。");
	EnableWindow(hWndBtnSelectScript, FALSE);

	/* スクリプトのリッチエディットを作成する */
	LoadLibrary(L"Msftedit.dll");
	hWndRichEdit = CreateWindowEx(
		0,
		MSFTEDIT_CLASS,
		L"Script",
		ES_MULTILINE | WS_VISIBLE | WS_CHILD | WS_BORDER | WS_TABSTOP | ES_AUTOVSCROLL,
		MulDiv(10, nDpi, 96),
		MulDiv(100, nDpi, 96),
		MulDiv(420, nDpi, 96),
		MulDiv(400, nDpi, 96),
		hWndEditor,
		(HMENU)ID_RICHEDIT,
		hInstance,
		NULL);
	GetClassName(hWndRichEdit, wszCls, sizeof(wszCls) / sizeof(wchar_t));
	if (wcscmp(wszCls, L"RICHEDIT50W") == 0)
	{
		/* Microsoft Office付属のリッチエディットでない場合(Windows付属の場合)、オートスクロールを使用しない */
		LONG style = GetWindowLong(hWndRichEdit, GWL_STYLE);
		style &= ~ES_AUTOVSCROLL;
		SetWindowLong(hWndRichEdit, GWL_STYLE, style);
	}
	SendMessage(hWndRichEdit, EM_SHOWSCROLLBAR, (WPARAM)SB_VERT, (LPARAM)TRUE);
	SendMessage(hWndRichEdit, EM_SETEVENTMASK, 0, (LPARAM)ENM_CHANGE);
	SendMessage(hWndRichEdit, EM_SETBKGNDCOLOR, (WPARAM)0, (LPARAM)dwColorBgDefault);
	SendMessage(hWndRichEdit, WM_SETFONT, (WPARAM)hFontFixed, (LPARAM)TRUE);
	RichEdit_SetFont();
	EnableWindow(hWndRichEdit, FALSE);

	/* 変数のテキストボックスを作成する */
	hWndTextboxVar = CreateWindow(
		L"EDIT",
		NULL,
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | WS_BORDER | WS_VSCROLL |
		ES_AUTOVSCROLL | ES_MULTILINE | ES_WANTRETURN,
		MulDiv(10, nDpi, 96),
		MulDiv(570, nDpi, 96),
		MulDiv(280, nDpi, 96),
		MulDiv(60, nDpi, 96),
		hWndEditor,
		0,
		hInstance,
		NULL);
	SendMessage(hWndTextboxVar, WM_SETFONT, (WPARAM)hFont, (LPARAM)TRUE);
	EnableWindow(hWndTextboxVar, FALSE);
	CreateTooltip(hWndTextboxVar,
				  L"List of variables which have non-initial values.",
				  L"初期値から変更された変数の一覧です。");

	/* 値を書き込むボタンを作成する */
	hWndBtnVar = CreateWindow(
		L"BUTTON",
		bEnglish ? L"Write values" : L"値を書き込む",
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
		MulDiv(300, nDpi, 96),
		MulDiv(570, nDpi, 96),
		MulDiv(130, nDpi, 96),
		MulDiv(30, nDpi, 96),
		hWndEditor,
		(HMENU)ID_VARS,
		hInstance,
		NULL);
	SendMessage(hWndBtnVar, WM_SETFONT, (WPARAM)hFont, (LPARAM)TRUE);
	EnableWindow(hWndBtnVar, FALSE);
	CreateTooltip(hWndBtnVar,
				  L"Write to the variables.",
				  L"変数の内容を書き込みます。");

	return TRUE;
}

/* メニューを作成する */
static VOID InitMenu(HWND hWnd)
{
	HMENU hMenuFile = CreatePopupMenu();
	HMENU hMenuRun = CreatePopupMenu();
	HMENU hMenuDirection = CreatePopupMenu();
	HMENU hMenuExport = CreatePopupMenu();
	HMENU hMenuPref = CreatePopupMenu();
	HMENU hMenuHelp = CreatePopupMenu();
    MENUITEMINFO mi;
	UINT nOrder;

	/* 演出メニューは右クリック時のポップアップとしても使う */
	hMenuPopup = hMenuDirection;

	/* メインメニューを作成する */
	hMenu = CreateMenu();

	/* 1階層目を作成する準備を行う */
	ZeroMemory(&mi, sizeof(MENUITEMINFO));
	mi.cbSize = sizeof(MENUITEMINFO);
	mi.fMask = MIIM_TYPE | MIIM_SUBMENU;
	mi.fType = MFT_STRING;
	mi.fState = MFS_ENABLED;

	/* ファイル(F)を作成する */
	nOrder = 0;
	mi.hSubMenu = hMenuFile;
	mi.dwTypeData = bEnglish ? L"File(&F)": L"ファイル(&F)";
	InsertMenuItem(hMenu, nOrder++, TRUE, &mi);

	/* 実行(R)を作成する */
	mi.hSubMenu = hMenuRun;
	mi.dwTypeData = bEnglish ? L"Run(&R)": L"実行(&R)";
	InsertMenuItem(hMenu, nOrder++, TRUE, &mi);

	/* 演出(D)を作成する */
	mi.hSubMenu = hMenuDirection;
	mi.dwTypeData = bEnglish ? L"Direction(&D)": L"演出(&D)";
	InsertMenuItem(hMenu, nOrder++, TRUE, &mi);

	/* エクスポート(E)を作成する */
	mi.hSubMenu = hMenuExport;
	mi.dwTypeData = bEnglish ? L"Export(&E)": L"エクスポート(&E)";
	InsertMenuItem(hMenu, nOrder++, TRUE, &mi);

	/* 設定(P)を作成する */
	mi.hSubMenu = hMenuPref;
	mi.dwTypeData = bEnglish ? L"Preference(&P)": L"設定(&P)";
	InsertMenuItem(hMenu, nOrder++, TRUE, &mi);

	/* ヘルプ(H)を作成する */
	mi.hSubMenu = hMenuHelp;
	mi.dwTypeData = bEnglish ? L"Help(&H)": L"ヘルプ(&H)";
	InsertMenuItem(hMenu, nOrder++, TRUE, &mi);

	/* 2階層目を作成する準備を行う */
	mi.fMask = MIIM_TYPE | MIIM_ID;

	/* 新規プロジェクトを作成する */
	nOrder = 0;
	mi.wID = ID_NEW_PROJECT;
	mi.dwTypeData = bEnglish ?
		L"New game" :
		L"新規ゲーム";
	InsertMenuItem(hMenuFile, nOrder++, TRUE, &mi);

	/* プロジェクトを開くを作成する */
	mi.wID = ID_OPEN_PROJECT;
	mi.dwTypeData = bEnglish ?
		L"Open game" :
		L"ゲームを開く";
	InsertMenuItem(hMenuFile, nOrder++, TRUE, &mi);

	/* ゲームフォルダを開くを作成する */
	mi.wID = ID_OPEN_GAME_FOLDER;
	mi.dwTypeData = bEnglish ?
		L"Open game folder" :
		L"ゲームフォルダを開く";
	InsertMenuItem(hMenuFile, nOrder++, TRUE, &mi);
	EnableMenuItem(hMenu, ID_OPEN_GAME_FOLDER, MF_GRAYED);

	/* スクリプトを開く(O)を作成する */
	mi.wID = ID_OPEN;
	mi.dwTypeData = bEnglish ?
		L"Open script(&O)\tCtrl+O" :
		L"スクリプトを開く(&O)\tCtrl+O";
	InsertMenuItem(hMenuFile, nOrder++, TRUE, &mi);
	EnableMenuItem(hMenu, ID_OPEN, MF_GRAYED);

	/* スクリプトをリロードを作成する */
	mi.wID = ID_RELOAD;
	mi.dwTypeData = bEnglish ?
		L"Reload script(&L)\tCtrl+L" :
		L"スクリプトをリロードする(&L)\tCtrl+L";
	InsertMenuItem(hMenuFile, nOrder++, TRUE, &mi);
	EnableMenuItem(hMenu, ID_RELOAD, MF_GRAYED);

	/* スクリプトを上書き保存する(S)を作成する */
	mi.wID = ID_SAVE;
	mi.dwTypeData = bEnglish ?
		L"Overwrite script(&S)\tCtrl+S" :
		L"スクリプトを上書き保存する(&S)\tCtrl+S";
	InsertMenuItem(hMenuFile, nOrder++, TRUE, &mi);
	EnableMenuItem(hMenu, ID_SAVE, MF_GRAYED);

	/* レイヤー画像のデバッグ出力を作成する */
	mi.wID = ID_DEBUG_LAYERS;
	mi.dwTypeData = bEnglish ?
		L"Write layer images for debugging" :
		L"レイヤー画像のデバッグ出力";
	InsertMenuItem(hMenuFile, nOrder++, TRUE, &mi);

	/* 終了(Q)を作成する */
	mi.wID = ID_QUIT;
	mi.dwTypeData = bEnglish ?
		L"Quit(&Q)\tCtrl+Q" :
		L"終了(&Q)\tCtrl+Q";
	InsertMenuItem(hMenuFile, nOrder++, TRUE, &mi);

	/* 続ける(C)を作成する */
	nOrder = 0;
	mi.wID = ID_RESUME;
	mi.dwTypeData = bEnglish ? L"Continue(&R)\tCtrl+R" : L"続ける(&R)\tCtrl+R";
	InsertMenuItem(hMenuRun, nOrder++, TRUE, &mi);
	EnableMenuItem(hMenu, ID_RESUME, MF_GRAYED);

	/* 次へ(N)を作成する */
	mi.wID = ID_NEXT;
	mi.dwTypeData = bEnglish ? L"Next(&N)\tCtrl+N" : L"次へ(&N)\tCtrl+N";
	InsertMenuItem(hMenuRun, nOrder++, TRUE, &mi);
	EnableMenuItem(hMenu, ID_NEXT, MF_GRAYED);

	/* 停止(P)を作成する */
	mi.wID = ID_PAUSE;
	mi.dwTypeData = bEnglish ? L"Pause(&P)\tCtrl+P" : L"停止(&P)\tCtrl+P";
	InsertMenuItem(hMenuRun, nOrder++, TRUE, &mi);
	EnableMenuItem(hMenu, ID_PAUSE, MF_GRAYED);

	/* 次のエラー箇所へ移動(E)を作成する */
	mi.wID = ID_ERROR;
	mi.dwTypeData = bEnglish ?
		L"Go to next error(&E)\tCtrl+E" :
		L"次のエラー箇所へ移動(&E)\tCtrl+E";
	InsertMenuItem(hMenuRun, nOrder++, TRUE, &mi);
	EnableMenuItem(hMenu, ID_ERROR, MF_GRAYED);

	/* Windowsゲームをエクスポートするを作成する */
	nOrder = 0;
	mi.wID = ID_EXPORT_WIN;
	mi.dwTypeData = bEnglish ?
		L"Export a Windows game" :
		L"Windowsゲームをエクスポートする";
	InsertMenuItem(hMenuExport, nOrder++, TRUE, &mi);
	EnableMenuItem(hMenu, ID_EXPORT_WIN, MF_GRAYED);

	/* Windows EXEインストーラを作成するを作成する */
	mi.wID = ID_EXPORT_WIN_INST;
	mi.dwTypeData = bEnglish ?
		L"Export a Windows game (installer)" :
		L"Windowsゲームをエクスポートする(インストーラ)";
	InsertMenuItem(hMenuExport, nOrder++, TRUE, &mi);
	EnableMenuItem(hMenu, ID_EXPORT_WIN_INST, MF_GRAYED);

	/* Windows/Macゲームをエクスポートするを作成する */
	mi.wID = ID_EXPORT_WIN_MAC;
	mi.dwTypeData = bEnglish ?
		L"Export a desktop game for Windows and others" :
		L"Windowsなどのデスクトップゲームをエクスポートする";
	InsertMenuItem(hMenuExport, nOrder++, TRUE, &mi);
	EnableMenuItem(hMenu, ID_EXPORT_WIN_MAC, MF_GRAYED);

	/* Webゲームをエクスポートするを作成する */
	mi.wID = ID_EXPORT_WEB;
	mi.dwTypeData = bEnglish ?
		L"Export for Web" :
		L"Webゲームをエクスポートする";
	InsertMenuItem(hMenuExport, nOrder++, TRUE, &mi);
	EnableMenuItem(hMenu, ID_EXPORT_WEB, MF_GRAYED);

	/* Androidプロジェクトをエクスポートするを作成する */
	mi.wID = ID_EXPORT_ANDROID;
	mi.dwTypeData = bEnglish ?
		L"Export Android project" :
		L"Androidプロジェクトをエクスポートする";
	InsertMenuItem(hMenuExport, nOrder++, TRUE, &mi);
	EnableMenuItem(hMenu, ID_EXPORT_ANDROID, MF_GRAYED);

	/* iOSプロジェクトをエクスポートするを作成する */
	mi.wID = ID_EXPORT_IOS;
	mi.dwTypeData = bEnglish ?
		L"Export iOS project" :
		L"iOSプロジェクトをエクスポートする";
	InsertMenuItem(hMenuExport, nOrder++, TRUE, &mi);
	EnableMenuItem(hMenu, ID_EXPORT_IOS, MF_GRAYED);

	/* パッケージをエクスポートするを作成する */
	mi.wID = ID_EXPORT_PACKAGE;
	mi.dwTypeData = bEnglish ?
		L"Export package only" :
		L"パッケージのみをエクスポートする";
	InsertMenuItem(hMenuExport, nOrder++, TRUE, &mi);
	EnableMenuItem(hMenu, ID_EXPORT_PACKAGE, MF_GRAYED);

	/* 地の文を入力を作成する */
	nOrder = 0;
	mi.wID = ID_CMD_MESSAGE;
	mi.dwTypeData = bEnglish ? L"Message" : L"地の文を入力";
	InsertMenuItem(hMenuDirection, nOrder++, TRUE, &mi);
	EnableMenuItem(hMenu, ID_CMD_MESSAGE, MF_GRAYED);

	/* セリフを入力を作成する */
	mi.wID = ID_CMD_SERIF;
	mi.dwTypeData = bEnglish ? L"Line" : L"セリフを入力";
	InsertMenuItem(hMenuDirection, nOrder++, TRUE, &mi);
	EnableMenuItem(hMenu, ID_CMD_SERIF, MF_GRAYED);

	/* 背景を作成する */
	mi.wID = ID_CMD_BG;
	mi.dwTypeData = bEnglish ? L"Background" : L"背景";
	InsertMenuItem(hMenuDirection, nOrder++, TRUE, &mi);
	EnableMenuItem(hMenu, ID_CMD_BG, MF_GRAYED);

	/* 背景だけ変更を作成する */
	mi.wID = ID_CMD_BG_ONLY;
	mi.dwTypeData = bEnglish ? L"Change Background Only" : L"背景だけ変更";
	InsertMenuItem(hMenuDirection, nOrder++, TRUE, &mi);
	EnableMenuItem(hMenu, ID_CMD_BG_ONLY, MF_GRAYED);

	/* キャラクタを作成する */
	mi.wID = ID_CMD_CH;
	mi.dwTypeData = bEnglish ? L"Character" : L"キャラクタ";
	InsertMenuItem(hMenuDirection, nOrder++, TRUE, &mi);
	EnableMenuItem(hMenu, ID_CMD_CH, MF_GRAYED);

	/* キャラクタを同時に変更を作成する */
	mi.wID = ID_CMD_CHSX;
	mi.dwTypeData = bEnglish ? L"Change Multiple Characters" : L"キャラクタを同時に変更";
	InsertMenuItem(hMenuDirection, nOrder++, TRUE, &mi);
	EnableMenuItem(hMenu, ID_CMD_CHSX, MF_GRAYED);

	/* 音楽を再生を作成する */
	mi.wID = ID_CMD_BGM;
	mi.dwTypeData = bEnglish ? L"Play BGM" : L"音楽を再生";
	InsertMenuItem(hMenuDirection, nOrder++, TRUE, &mi);
	EnableMenuItem(hMenu, ID_CMD_BGM, MF_GRAYED);

	/* 音楽を停止を作成する */
	mi.wID = ID_CMD_BGM_STOP;
	mi.dwTypeData = bEnglish ? L"Stop BGM" : L"音楽を停止";
	InsertMenuItem(hMenuDirection, nOrder++, TRUE, &mi);
	EnableMenuItem(hMenu, ID_CMD_BGM_STOP, MF_GRAYED);

	/* 音楽の音量を作成する */
	mi.wID = ID_CMD_VOL_BGM;
	mi.dwTypeData = bEnglish ? L"BGM Volume" : L"音楽の音量";
	InsertMenuItem(hMenuDirection, nOrder++, TRUE, &mi);
	EnableMenuItem(hMenu, ID_CMD_VOL_BGM, MF_GRAYED);

	/* 効果音を再生を作成する */
	mi.wID = ID_CMD_SE;
	mi.dwTypeData = bEnglish ? L"Play Sound Effect" : L"効果音を再生";
	InsertMenuItem(hMenuDirection, nOrder++, TRUE, &mi);
	EnableMenuItem(hMenu, ID_CMD_SE, MF_GRAYED);

	/* 効果音を停止を作成する */
	mi.wID = ID_CMD_SE_STOP;
	mi.dwTypeData = bEnglish ? L"Stop Sound Effect" : L"効果音を停止";
	InsertMenuItem(hMenuDirection, nOrder++, TRUE, &mi);
	EnableMenuItem(hMenu, ID_CMD_SE_STOP, MF_GRAYED);

	/* 効果音の音量を作成する */
	mi.wID = ID_CMD_VOL_SE;
	mi.dwTypeData = bEnglish ? L"Sound Effect Volume" : L"効果音の音量";
	InsertMenuItem(hMenuDirection, nOrder++, TRUE, &mi);
	EnableMenuItem(hMenu, ID_CMD_VOL_SE, MF_GRAYED);

	/* 動画を再生する */
	mi.wID = ID_CMD_VIDEO;
	mi.dwTypeData = bEnglish ? L"Play Video" : L"動画を再生する";
	InsertMenuItem(hMenuDirection, nOrder++, TRUE, &mi);
	EnableMenuItem(hMenu, ID_CMD_VIDEO, MF_GRAYED);

	/* 画面を横に揺らすを作成する */
	mi.wID = ID_CMD_SHAKE_H;
	mi.dwTypeData = bEnglish ? L"Shake Screen Horizontally" : L"画面を横に揺らす";
	InsertMenuItem(hMenuDirection, nOrder++, TRUE, &mi);
	EnableMenuItem(hMenu, ID_CMD_SHAKE_H, MF_GRAYED);

	/* 画面を縦に揺らすを作成する */
	mi.wID = ID_CMD_SHAKE_V;
	mi.dwTypeData = bEnglish ? L"Shake Screen Vertically" : L"画面を縦に揺らす";
	InsertMenuItem(hMenuDirection, nOrder++, TRUE, &mi);
	EnableMenuItem(hMenu, ID_CMD_SHAKE_V, MF_GRAYED);

	/* 選択肢(3)を作成する */
	mi.wID = ID_CMD_CHOOSE_3;
	mi.dwTypeData = bEnglish ? L"Options (3)" : L"選択肢(3)";
	InsertMenuItem(hMenuDirection, nOrder++, TRUE, &mi);
	EnableMenuItem(hMenu, ID_CMD_CHOOSE_3, MF_GRAYED);

	/* 選択肢(2)を作成する */
	mi.wID = ID_CMD_CHOOSE_2;
	mi.dwTypeData = bEnglish ? L"Options (2)" : L"選択肢(2)";
	InsertMenuItem(hMenuDirection, nOrder++, TRUE, &mi);
	EnableMenuItem(hMenu, ID_CMD_CHOOSE_2, MF_GRAYED);

	/* 選択肢(1)を作成する */
	mi.wID = ID_CMD_CHOOSE_1;
	mi.dwTypeData = bEnglish ? L"Option (1)" : L"選択肢(1)";
	InsertMenuItem(hMenuDirection, nOrder++, TRUE, &mi);
	EnableMenuItem(hMenu, ID_CMD_CHOOSE_1, MF_GRAYED);

	/* GUI呼び出しを作成する */
	mi.wID = ID_CMD_GUI;
	mi.dwTypeData = bEnglish ? L"Menu (GUI)" : L"メニュー (GUI)";
	InsertMenuItem(hMenuDirection, nOrder++, TRUE, &mi);
	EnableMenuItem(hMenu, ID_CMD_GUI, MF_GRAYED);

	/* クリック待ちを作成する */
	mi.wID = ID_CMD_CLICK;
	mi.dwTypeData = bEnglish ? L"Click Wait" : L"クリック待ち";
	InsertMenuItem(hMenuDirection, nOrder++, TRUE, &mi);
	EnableMenuItem(hMenu, ID_CMD_CLICK, MF_GRAYED);

	/* 時間指定待ちを作成する */
	mi.wID = ID_CMD_WAIT;
	mi.dwTypeData = bEnglish ? L"Timed Wait" : L"時間指定待ち";
	InsertMenuItem(hMenuDirection, nOrder++, TRUE, &mi);
	EnableMenuItem(hMenu, ID_CMD_WAIT, MF_GRAYED);

	/* 他のスクリプトへ移動を作成する */
	mi.wID = ID_CMD_LOAD;
	mi.dwTypeData = bEnglish ? L"Load Other Script" : L"他のスクリプトへ移動";
	InsertMenuItem(hMenuDirection, nOrder++, TRUE, &mi);
	EnableMenuItem(hMenu, ID_CMD_LOAD, MF_GRAYED);

	/* フォント選択を作成する */
	nOrder = 0;
	mi.wID = ID_FONT;
	mi.dwTypeData = bEnglish ? L"Font settings" : L"フォント設定";
	InsertMenuItem(hMenuPref, nOrder++, TRUE, &mi);

	/* ハイライトモードを作成する */
	mi.wID = ID_HIGHLIGHTMODE;
	mi.dwTypeData = bEnglish ? L"Highlight mode" : L"ハイライトモード";
	InsertMenuItem(hMenuPref, nOrder++, TRUE, &mi);

	/* ダークモードを作成する */
	mi.wID = ID_DARKMODE;
	mi.dwTypeData = bEnglish ? L"Dark mode" : L"ダークモード";
	InsertMenuItem(hMenuPref, nOrder++, TRUE, &mi);

	/* バージョンを作成する */
	nOrder = 0;
	mi.wID = ID_VERSION;
	mi.dwTypeData = bEnglish ? L"Version" : L"バージョン";
	InsertMenuItem(hMenuHelp, nOrder++, TRUE, &mi);

	/* メインメニューをセットする */
	SetMenu(hWnd, hMenu);
}

/* ツールチップを作成する */
static HWND CreateTooltip(HWND hWndBtn, const wchar_t *pszTextEnglish,
						  const wchar_t *pszTextJapanese)
{
	TOOLINFO ti;

	/* ツールチップを作成する */
	HWND hWndTip = CreateWindowEx(0, TOOLTIPS_CLASS, NULL, TTS_ALWAYSTIP,
								  CW_USEDEFAULT, CW_USEDEFAULT,
								  CW_USEDEFAULT, CW_USEDEFAULT,
								  hWndEditor, NULL, GetModuleHandle(NULL),
								  NULL);

	/* ツールチップをボタンに紐付ける */
	ZeroMemory(&ti, sizeof(ti));
	ti.cbSize = sizeof(ti);
	ti.uFlags = TTF_SUBCLASS;
	ti.hwnd = hWndBtn;
	ti.lpszText = (wchar_t *)(bEnglish ? pszTextEnglish : pszTextJapanese);
	GetClientRect(hWndBtn, &ti.rect);
	SendMessage(hWndTip, TTM_ADDTOOL, 0, (LPARAM)&ti);

	return hWndTip;
}

static VOID StartGame(void)
{
	RECT rcClient;

	do {
		assert(FILE_EXISTS("conf\\config.txt"));
		assert(FILE_EXISTS("txt\\init.txt"));

		/* Initialize the locale code. */
		init_locale_code();

		/* Initialize the file subsyetem. */
		if (!init_file())
			break;

		/* Initialize the config subsyetem. */
		if (!init_conf())
			break;

		/* Move the game panel and notify its position to the rendering subsyetem. */
		GetClientRect(hWndMain, &rcClient);
		nLastClientWidth = 0;
		nLastClientHeight = 0;
		Layout(rcClient.right, rcClient.bottom);

		/* Do the upper layer initialization. */
		if (!on_event_init())
			break;

		/* Mark as opened. */
		bProjectOpened = TRUE;

		/* Make controls enabled/disabled. */
		EnableWindow(hWndBtnResume, TRUE);
		EnableWindow(hWndBtnNext, TRUE);
		EnableWindow(hWndBtnMove, TRUE);
		EnableWindow(hWndBtnPause, FALSE);
		EnableWindow(hWndBtnSelectScript, TRUE);
		EnableWindow(hWndRichEdit, TRUE);
		EnableWindow(hWndTextboxVar, TRUE);
		EnableWindow(hWndBtnVar, TRUE);

		/* Make menu items enabled/disabled. */
		EnableMenuItem(hMenu, ID_NEW_PROJECT, MF_GRAYED);
		EnableMenuItem(hMenu, ID_OPEN_PROJECT, MF_GRAYED);
		EnableMenuItem(hMenu, ID_OPEN_GAME_FOLDER, MF_ENABLED);
		EnableMenuItem(hMenu, ID_OPEN, MF_ENABLED);
		EnableMenuItem(hMenu, ID_RELOAD, MF_ENABLED);
		EnableMenuItem(hMenu, ID_SAVE, MF_ENABLED);
		EnableMenuItem(hMenu, ID_RESUME, MF_ENABLED);
		EnableMenuItem(hMenu, ID_NEXT, MF_ENABLED);
		EnableMenuItem(hMenu, ID_PAUSE, MF_ENABLED);
		EnableMenuItem(hMenu, ID_ERROR, MF_ENABLED);
		EnableMenuItem(hMenu, ID_EXPORT_WIN, MF_ENABLED);
		EnableMenuItem(hMenu, ID_EXPORT_WIN_INST, MF_ENABLED);
		EnableMenuItem(hMenu, ID_EXPORT_WIN_MAC, MF_ENABLED);
		EnableMenuItem(hMenu, ID_EXPORT_WEB, MF_ENABLED);
		EnableMenuItem(hMenu, ID_EXPORT_ANDROID, MF_ENABLED);
		EnableMenuItem(hMenu, ID_EXPORT_IOS, MF_ENABLED);
		EnableMenuItem(hMenu, ID_EXPORT_PACKAGE, MF_ENABLED);
		EnableMenuItem(hMenu, ID_CMD_MESSAGE, MF_ENABLED);
		EnableMenuItem(hMenu, ID_CMD_SERIF, MF_ENABLED);
		EnableMenuItem(hMenu, ID_CMD_BG, MF_ENABLED);
		EnableMenuItem(hMenu, ID_CMD_BG_ONLY, MF_ENABLED);
		EnableMenuItem(hMenu, ID_CMD_CH, MF_ENABLED);
		EnableMenuItem(hMenu, ID_CMD_CHSX, MF_ENABLED);
		EnableMenuItem(hMenu, ID_CMD_BGM, MF_ENABLED);
		EnableMenuItem(hMenu, ID_CMD_BGM_STOP, MF_ENABLED);
		EnableMenuItem(hMenu, ID_CMD_VOL_BGM, MF_ENABLED);
		EnableMenuItem(hMenu, ID_CMD_SE, MF_ENABLED);
		EnableMenuItem(hMenu, ID_CMD_SE_STOP, MF_ENABLED);
		EnableMenuItem(hMenu, ID_CMD_VOL_SE, MF_ENABLED);
		EnableMenuItem(hMenu, ID_CMD_VIDEO, MF_ENABLED);
		EnableMenuItem(hMenu, ID_CMD_SHAKE_H, MF_ENABLED);
		EnableMenuItem(hMenu, ID_CMD_SHAKE_V, MF_ENABLED);
		EnableMenuItem(hMenu, ID_CMD_CHOOSE_3, MF_ENABLED);
		EnableMenuItem(hMenu, ID_CMD_CHOOSE_2, MF_ENABLED);
		EnableMenuItem(hMenu, ID_CMD_CHOOSE_1, MF_ENABLED);
		EnableMenuItem(hMenu, ID_CMD_CLICK, MF_ENABLED);
	} while (0);
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

	/* プロジェクトが開かれていない場合 */
	if (!bProjectOpened)
		return TRUE;

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
		if (PretranslateMessage(&msg))
			continue;
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return TRUE;
}

/*
 * メッセージのトランスレート前処理を行う
 *  - return: メッセージが消費されたか
 *    - TRUEなら呼出元はメッセージをウィンドウプロシージャに送ってはならない
 *    - FALSEなら呼出元はメッセージをウィンドウプロシージャに送らなくてはいけない
 */
static BOOL PretranslateMessage(MSG* pMsg)
{
	static BOOL bShiftDown;
	static BOOL bControlDown;

	/* Alt+Enterを処理する */
	if (pMsg->hwnd == hWndRichEdit &&
		pMsg->message == WM_SYSKEYDOWN &&
		pMsg->wParam == VK_RETURN &&
		(HIWORD(pMsg->lParam) & KF_ALTDOWN))
	{
		if (!bFullScreen)
			bNeedFullScreen = TRUE;
		else
			bNeedWindowed = TRUE;
		SendMessage(hWndMain, WM_SIZE, 0, 0);

		/* このメッセージをリッチエディットにディスパッチしない */
		return TRUE;
	}

	/* シフト押下状態を保存する */
	if (pMsg->hwnd == hWndRichEdit && pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_SHIFT)
	{
		bShiftDown = TRUE;
		return FALSE;
	}
	if (pMsg->hwnd == hWndRichEdit && pMsg->message == WM_KEYUP && pMsg->wParam == VK_SHIFT)
	{
		bShiftDown = FALSE;
		return FALSE;
	}

	/* コントロール押下状態を保存する */
	if (pMsg->hwnd == hWndRichEdit && pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_CONTROL)
	{
		bControlDown = TRUE;
		return FALSE;
	}
	if (pMsg->hwnd == hWndRichEdit && pMsg->message == WM_KEYUP && pMsg->wParam == VK_CONTROL)
	{
		bControlDown = FALSE;
		return FALSE;
	}

	/* フォーカスを失うときにシフトとコントロールの押下状態をクリアする */
	if (pMsg->hwnd == hWndRichEdit && pMsg->message == WM_KILLFOCUS)
	{
		bShiftDown = FALSE;
		bControlDown = FALSE;
		return FALSE;
	}

	/* 右クリック押下を処理する */
	if (pMsg->hwnd == hWndRichEdit &&
		pMsg->message == WM_RBUTTONDOWN)
	{
		/* ポップアップを開くためのWM_COMMANDをポストする */
		PostMessage(hWndMain, WM_COMMAND, (WPARAM)ID_POPUP, 0);
		return FALSE;
	}

	/* キー押下を処理する */
	if (pMsg->hwnd == hWndRichEdit && pMsg->message == WM_KEYDOWN)
	{
		switch (pMsg->wParam)
		{
		/*
		 * リッチエディットの編集
		 */
		case VK_RETURN:
			if (!bShiftDown)
			{
				OnReturn();

				/* このメッセージはリッチエディットに送らない(改行しない) */
				return TRUE;
			}
			break;
		/*
		 * メニュー
		 */
		case 'O':
			/* Ctrl+Oを処理する */
			if (bControlDown)
			{
				pMsg->hwnd = hWndMain;
				pMsg->message = WM_COMMAND;
				pMsg->wParam = ID_OPEN;
				pMsg->lParam = 0;
			}
			break;
		case 'L':
			/* Ctrl+Lを処理する */
			if (bControlDown)
			{
				pMsg->hwnd = hWndMain;
				pMsg->message = WM_COMMAND;
				pMsg->wParam = ID_RELOAD;
				pMsg->lParam = 0;
			}
			break;
		case 'S':
			/* Ctrl+Sを処理する */
			if (bControlDown)
			{
				pMsg->hwnd = hWndMain;
				pMsg->message = WM_COMMAND;
				pMsg->wParam = ID_SAVE;
				pMsg->lParam = 0;
			}
			break;
		case 'Q':
			/* Ctrl+Qを処理する */
			if (bControlDown)
			{
				pMsg->hwnd = hWndMain;
				pMsg->message = WM_COMMAND;
				pMsg->wParam = ID_QUIT;
				pMsg->lParam = 0;
			}
			break;
		case 'R':
			/* Ctrl+Rを処理する */
			if (bControlDown)
			{
				pMsg->hwnd = hWndMain;
				pMsg->message = WM_COMMAND;
				pMsg->wParam = ID_RESUME;
				pMsg->lParam = 0;
			}
			break;
		case 'N':
			/* Ctrl+Nを処理する */
			if (bControlDown)
			{
				pMsg->hwnd = hWndMain;
				pMsg->message = WM_COMMAND;
				pMsg->wParam = ID_NEXT;
				pMsg->lParam = 0;
			}
			break;
		case 'P':
			/* Ctrl+Pを処理する */
			if (bControlDown)
			{
				pMsg->hwnd = hWndMain;
				pMsg->message = WM_COMMAND;
				pMsg->wParam = ID_PAUSE;
				pMsg->lParam = 0;
			}
			break;
		case 'E':
			/* Ctrl+Eを処理する */
			if (bControlDown)
			{
				pMsg->hwnd = hWndMain;
				pMsg->message = WM_COMMAND;
				pMsg->wParam = ID_ERROR;
				pMsg->lParam = 0;
			}
			break;
		default:
			break;
		}
		return FALSE;
	}

	/* このメッセージは引き続きリッチエディットで処理する */
	return FALSE;
}

/* 次のフレームの開始時刻までイベント処理とスリープを行う */
static BOOL WaitForNextFrame(void)
{
	DWORD end, lap, wait, span;

	/* 30FPSを目指す */
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
	case WM_SYSKEYDOWN:
		/* Alt + F4 */
		if(wParam == VK_F4)
		{
			DestroyWindow(hWnd);
			return 0;
		}
		break;
	case WM_CLOSE:
		if (hWnd != NULL && hWnd == hWndMain)
		{
			DestroyWindow(hWnd);
			return 0;
		}
		break;
	case WM_LBUTTONDOWN:
		if (hWnd != NULL && hWnd == hWndRender)
		{
			on_event_mouse_press(MOUSE_LEFT,
								 (int)((float)(LOWORD(lParam) - nOffsetX) / fMouseScale),
								 (int)((float)(HIWORD(lParam) - nOffsetY) / fMouseScale));
			return 0;
		}
		break;
	case WM_LBUTTONUP:
		if (hWnd != NULL && hWnd == hWndRender)
		{
			on_event_mouse_release(MOUSE_LEFT,
								   (int)((float)(LOWORD(lParam) - nOffsetX) / fMouseScale),
								   (int)((float)(HIWORD(lParam) - nOffsetY) / fMouseScale));
			return 0;
		}
		break;
	case WM_RBUTTONDOWN:
		if (hWnd != NULL && hWnd == hWndRender)
		{
			on_event_mouse_press(MOUSE_RIGHT,
								 (int)((float)(LOWORD(lParam) - nOffsetX) / fMouseScale),
								 (int)((float)(HIWORD(lParam) - nOffsetY) / fMouseScale));
			return 0;
		}
		break;
	case WM_RBUTTONUP:
		if (hWnd != NULL && hWnd == hWndRender)
		{
			on_event_mouse_release(MOUSE_RIGHT,
								   (int)((float)(LOWORD(lParam) - nOffsetX) / fMouseScale),
								   (int)((float)(HIWORD(lParam) - nOffsetY) / fMouseScale));
			return 0;
		}
		break;
	case WM_KEYDOWN:
		if (hWnd != NULL && hWnd == hWndRender)
		{
			/* オートリピートの場合を除外する */
			if((HIWORD(lParam) & 0x4000) != 0)
				return 0;

			/* フルスクリーン中のエスケープキーの場合 */
			if((int)wParam == VK_ESCAPE && bFullScreen)
			{
				bNeedWindowed = TRUE;
				SendMessage(hWndMain, WM_SIZE, 0, 0);
				return 0;
			}

			/* その他のキーの場合 */
			kc = ConvertKeyCode((int)wParam);
			if(kc != -1)
				on_event_key_press(kc);
			return 0;
		}
		break;
	case WM_KEYUP:
		if (hWnd != NULL && (hWnd == hWndRender || hWnd == hWndMain))
		{
			kc = ConvertKeyCode((int)wParam);
			if(kc != -1)
				on_event_key_release(kc);
			return 0;
		}
		break;
	case WM_MOUSEMOVE:
		if (hWnd != NULL && hWnd == hWndRender)
		{
			on_event_mouse_move((int)((float)(LOWORD(lParam) - nOffsetX) / fMouseScale),
								(int)((float)(HIWORD(lParam) - nOffsetY) / fMouseScale));
			return 0;
		}
		break;
	case WM_MOUSEWHEEL:
		if (hWnd != NULL && hWnd == hWndRender)
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
		if (hWnd != NULL && (hWnd == hWndRender || hWnd == hWndMain))
		{
			on_event_key_release(KEY_CONTROL);
			return 0;
		}
		break;
	case WM_SYSCHAR:
		if (hWnd != NULL && (hWnd == hWndRender || hWnd == hWndMain))
		{
			return 0;
		}
		break;
	case WM_PAINT:
		if (hWnd != NULL && (hWnd == hWndRender || hWnd == hWndMain))
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
			OnSizing((int)wParam, (LPRECT)lParam);
			return TRUE;
		}
		break;
	case WM_SIZE:
		if (hWnd != NULL && hWnd == hWndMain)
		{
			OnSize();
			return 0;
		}
		break;
	case WM_DPICHANGED:
		OnDpiChanged(hWnd, HIWORD(wParam), (LPRECT)lParam);
		return 0;
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
	if (hWnd == hWndRender)
		RunFrame();
	EndPaint(hWnd, &ps);

	UNUSED_PARAMETER(hDC);
}

/* WM_COMMANDを処理する */
static void OnCommand(WPARAM wParam, LPARAM lParam)
{
	UINT nID;
	UINT nNotify;

	UNUSED_PARAMETER(lParam);

	nID = LOWORD(wParam);
	nNotify = (WORD)(wParam >> 16) & 0xFFFF;

	/* リッチエディットのEN_CHANGEを確認する */
	if (nID == ID_RICHEDIT && nNotify == EN_CHANGE)
	{
		RichEdit_OnChange();
		return;
	}

	/* IDごとに処理する */
	switch(nID)
	{
	/* ファイル */
	case ID_NEW_PROJECT:
		OnNewProject();
		break;
	case ID_OPEN_PROJECT:
		OnOpenProject();
		break;
	case ID_OPEN_GAME_FOLDER:
		OnOpenGameFolder();
		break;
	case ID_OPEN:
		OnOpenScript();
		break;
	case ID_RELOAD:
		OnReloadScript();
		break;
	case ID_SAVE:
		OnSave();
		break;
	case ID_DEBUG_LAYERS:
		write_layers_to_files();
		break;
	case ID_QUIT:
		DestroyWindow(hWndMain);
		break;
	/* スクリプト実行 */
	case ID_RESUME:
		OnContinue();
		break;
	case ID_NEXT:
		OnNext();
		break;
	case ID_PAUSE:
		OnStop();
		break;
	case ID_MOVE:
		OnMove();
		break;
	case ID_ERROR:
		OnNextError();
		break;
	/* ポップアップ */
	case ID_POPUP:
		OnPopup();
		break;
	/* 演出 */
	case ID_CMD_MESSAGE:
		OnInsertMessage();
		break;
	case ID_CMD_SERIF:
		OnInsertSerif();
		break;
	case ID_CMD_BG:
		OnInsertBg();
		break;
	case ID_CMD_BG_ONLY:
		OnInsertBgOnly();
		break;
	case ID_CMD_CH:
		OnInsertCh();
		break;
	case ID_CMD_CHSX:
		OnInsertChsx();
		break;
	case ID_CMD_BGM:
		OnInsertBgm();
		break;
	case ID_CMD_BGM_STOP:
		OnInsertBgmStop();
		break;
	case ID_CMD_VOL_BGM:
		OnInsertVolBgm();
		break;
	case ID_CMD_SE:
		OnInsertSe();
		break;
	case ID_CMD_SE_STOP:
		OnInsertSeStop();
		break;
	case ID_CMD_VOL_SE:
		OnInsertVolSe();
		break;
	case ID_CMD_VIDEO:
		OnInsertVideo();
		break;
	case ID_CMD_SHAKE_H:
		OnInsertShakeH();
		break;
	case ID_CMD_SHAKE_V:
		OnInsertShakeV();
		break;
	case ID_CMD_CHOOSE_3:
		OnInsertChoose3();
		break;
	case ID_CMD_CHOOSE_2:
		OnInsertChoose2();
		break;
	case ID_CMD_CHOOSE_1:
		OnInsertChoose1();
		break;
	case ID_CMD_GUI:
		OnInsertGui();
		break;
	case ID_CMD_CLICK:
		OnInsertClick();
		break;
	case ID_CMD_WAIT:
		OnInsertWait();
		break;
	case ID_CMD_LOAD:
		OnInsertLoad();
		break;
	/* エクスポート */
	case ID_EXPORT_WIN:
		OnExportWin();
		break;
	case ID_EXPORT_WIN_INST:
		OnExportWinInst();
		break;
	case ID_EXPORT_WIN_MAC:
		OnExportWinMac();
		break;
	case ID_EXPORT_WEB:
		OnExportWeb();
		break;
	case ID_EXPORT_ANDROID:
		OnExportAndroid();
		break;
	case ID_EXPORT_IOS:
		OnExportIOS();
		break;
	case ID_EXPORT_PACKAGE:
		OnExportPackage();
		break;
	/* ビュー */
	case ID_FONT:
		OnFont();
		break;
	case ID_HIGHLIGHTMODE:
		OnHighlightMode();
		break;
	case ID_DARKMODE:
		OnDarkMode();
		break;
	/* ヘルプ */
	case ID_VERSION:
		MessageBox(hWndMain, VERSION, TITLE, MB_OK | MB_ICONINFORMATION);
		break;
	/* ボタン */
	case ID_VARS:
		OnWriteVars();
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
		/* Apply the minimum window size. */
		if (fWidth < WINDOW_WIDTH_MIN)
			fWidth = WINDOW_WIDTH_MIN;
		if (fHeight < WINDOW_HEIGHT_MIN)
			fHeight = WINDOW_HEIGHT_MIN;

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
	}

	/* If there's a size change, update the screen size with the debugger panel size. */
	if (nOrigWidth != lpRect->right - lpRect->left + 1 ||
		nOrigHeight != lpRect->bottom - lpRect->top + 1)
		Layout((int)(fWidth + 0.5f), (int)(fHeight + 0.5f));
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
		dwExStyle = (DWORD)GetWindowLong(hWndMain, GWL_EXSTYLE);
		GetWindowRect(hWndMain, &rcWindow);

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
		SetWindowLong(hWndMain, GWL_EXSTYLE, (LONG)dwExStyle);
		MoveWindow(hWndMain, rcWindow.left, rcWindow.top, rcWindow.right - rcWindow.left, rcWindow.bottom - rcWindow.top, TRUE);
		InvalidateRect(hWndMain, NULL, TRUE);
		GetClientRect(hWndMain, &rc);
	}
	else
	{
		GetClientRect(hWndMain, &rc);
	}

	/* Update the screen offset and scale. */
	Layout(rc.right, rc.bottom);
}

/* スクリーンのオフセットとスケールを計算する */
static void Layout(int nClientWidth, int nClientHeight)
{
	float fAspect, fRenderWidth, fRenderHeight;
	int nDpi, nRenderWidth, nEditorWidth, y;

	nDpi = Win11_GetDpiForWindow(hWndMain);

	/* If size and dpi are not changed, just return. */
	if (nClientWidth == nLastClientWidth && nClientHeight == nLastClientHeight && nLastDpi != nDpi)
		return;

	/* Save the last client size and the dpi. */
	nLastClientWidth = nClientWidth;
	nLastClientHeight = nClientHeight;
	nLastDpi = nDpi;

	/* Calc the editor width and render width. */
	nEditorWidth = MulDiv(EDITOR_WIDTH, nDpi, 96);
	nRenderWidth = nClientWidth - nEditorWidth;

	/* Calc the rendering area. */
	fAspect = (float)conf_window_height / (float)conf_window_width;
	if ((float)nRenderWidth * fAspect <= (float)nClientHeight)
	{
		/* Width-first way. */
		fRenderWidth = (float)nRenderWidth;
		fRenderHeight = fRenderWidth * fAspect;
		fMouseScale = (float)nRenderWidth / (float)conf_window_width;
	}
	else
	{
		/* Height-first way. */
        fRenderHeight = (float)nClientHeight;
        fRenderWidth = (float)nClientHeight / fAspect;
        fMouseScale = (float)nClientHeight / (float)conf_window_height;
    }

	/* Calc the viewport origin. */
	nOffsetX = (int)((((float)nRenderWidth - fRenderWidth) / 2.0f) + 0.5);
	nOffsetY = (int)((((float)nClientHeight - fRenderHeight) / 2.0f) + 0.5);

	/* Move the rendering panel. */
	MoveWindow(hWndRender, 0, 0, nRenderWidth, nClientHeight, TRUE);

	/* Update the screen offset and scale for drawing subsystem. */
	D3DResizeWindow(nOffsetX, nOffsetY, fMouseScale);

	/* エディタのコントロールをサイズ変更する */
	MoveWindow(hWndRichEdit,
			   MulDiv(10, nDpi, 96),
			   MulDiv(100, nDpi, 96),
			   MulDiv(420, nDpi, 96),
			   nClientHeight - MulDiv(180, nDpi, 96),
			   TRUE);

	/* エディタより下のコントロールのY座標を計算する */
	y = nClientHeight - MulDiv(130, nDpi, 96);

	/* 変数のテキストボックスを移動する */
	MoveWindow(hWndTextboxVar,
			   MulDiv(10, nDpi, 96),
			   y + MulDiv(60, nDpi, 96),
			   MulDiv(280, nDpi, 96),
			   MulDiv(60, nDpi, 96),
			   TRUE);

	/* 変数書き込みのボタンを移動する */
	MoveWindow(hWndBtnVar,
			   MulDiv(300, nDpi, 96),
			   y + MulDiv(70, nDpi, 96),
			   MulDiv(130, nDpi, 96),
			   MulDiv(30, nDpi, 96),
			   TRUE);

	/* Move the editor panel. */
	MoveWindow(hWndEditor, nRenderWidth, 0, nEditorWidth, nClientHeight, TRUE);
}

/* WM_DPICHANGED */
VOID OnDpiChanged(HWND hWnd, UINT nDpi, LPRECT lpRect)
{
	RECT rcClient;

	UNUSED_PARAMETER(nDpi);

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
		Layout(rcClient.right, rcClient.bottom);
		RichEdit_SetFont();
	}
}

/*
 * HAL (main)
 */

/*
 * INFOログを出力する
 */
bool log_info(const char *s, ...)
{
	char buf[4096];
	va_list ap;

	/* メッセージボックスを表示する */
	va_start(ap, s);
	vsnprintf(buf, sizeof(buf), s, ap);
	va_end(ap);
	MessageBox(hWndMain, conv_utf8_to_utf16(buf), TITLE, MB_OK | MB_ICONINFORMATION);

	return true;
}

/*
 * WARNログを出力する
 */
bool log_warn(const char *s, ...)
{
	char buf[4096];
	va_list ap;

	/* メッセージボックスを表示する */
	va_start(ap, s);
	vsnprintf(buf, sizeof(buf), s, ap);
	va_end(ap);
	MessageBox(hWndMain, conv_utf8_to_utf16(buf), TITLE, MB_OK | MB_ICONWARNING);

	return true;
}

/*
 * ERRORログを出力する
 */
bool log_error(const char *s, ...)
{
	char buf[4096];
	va_list ap;

	/* メッセージボックスを表示する */
	va_start(ap, s);
	vsnprintf(buf, sizeof(buf), s, ap);
	va_end(ap);
	MessageBox(hWndMain, conv_utf8_to_utf16(buf), TITLE, MB_OK | MB_ICONERROR);

	return true;
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
}

/*
 * タイマのラップを秒単位で取得する
 */
uint64_t get_lap_timer_millisec(uint64_t *origin)
{
	DWORD dwCur = GetTickCount();
	return (uint64_t)(dwCur - *origin);
}

/*
 * 終了ダイアログを表示する
 */
bool exit_dialog(void)
{
	if (MessageBox(hWndMain,
				   conv_utf8_to_utf16(get_ui_message(UIMSG_EXIT)),
				   TITLE,
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
				   conv_utf8_to_utf16(get_ui_message(UIMSG_TITLE)),
				   TITLE,
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
				   conv_utf8_to_utf16(get_ui_message(UIMSG_DELETE)),
				   TITLE,
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
				   conv_utf8_to_utf16(get_ui_message(UIMSG_OVERWRITE)),
				   TITLE,
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
				   conv_utf8_to_utf16(get_ui_message(UIMSG_DEFAULT)),
				   TITLE,
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
	wchar_t wszTitle[1024];
	wchar_t wszTmp[1024];
	const char *separator;

	ZeroMemory(&wszTitle[0], sizeof(wszTitle));
	ZeroMemory(&wszTmp[0], sizeof(wszTmp));

	/* コンフィグのウィンドウタイトルをUTF-8からUTF-16に変換する */
	wcscpy(wszTitle, conv_utf8_to_utf16(conf_window_title));
	if (!conf_window_title_chapter_disable)
	{
		/* セパレータを取得する */
		separator = conf_window_title_separator;
		if (separator == NULL)
			separator = " ";

		/* セパレータを連結する */
		wcscat(wszTitle, conv_utf8_to_utf16(separator));

		/* 章タイトルを連結する */
		wcscat(wszTitle, conv_utf8_to_utf16(get_chapter_name()));
	}

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
 * TTSによる読み上げを行う
 */
void speak_text(const char *text)
{
	UNUSED_PARAMETER(text);
}

/*
 * HAL (pro)
 */

/*
 * 続けるボタンが押されたか調べる
 */
bool is_continue_pushed(void)
{
	bool ret = bContinuePressed;
	bContinuePressed = FALSE;
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
bool is_stop_pushed(void)
{
	bool ret = bStopPressed;
	bStopPressed = FALSE;
	return ret;
}

/*
 * 実行するスクリプトファイルが変更されたか調べる
 */
bool is_script_opened(void)
{
	bool ret = bScriptOpened;
	bScriptOpened = FALSE;
	return ret;
}

/*
 * 変更された実行するスクリプトファイル名を取得する
 */
const char *get_opened_script(void)
{
	static wchar_t script[256];

	GetWindowText(hWndTextboxScript,
				  script,
				  sizeof(script) /sizeof(wchar_t) - 1);
	script[255] = L'\0';
	return conv_utf16_to_utf8(script);
}

/*
 * 実行する行番号が変更されたか調べる
 */
bool is_exec_line_changed(void)
{
	bool ret = bExecLineChanged;
	bExecLineChanged = FALSE;
	return ret;
}

/*
 * 変更された実行する行番号を取得する
 */
int get_changed_exec_line(void)
{
	return nLineChanged;
}

/*
 * コマンドの実行中状態を設定する
 */
void on_change_running_state(bool running, bool request_stop)
{
	UINT i;

	bRunning = running;

	if(request_stop)
	{
		/*
		 * 実行中だが停止要求によりコマンドの完了を待機中のとき
		 *  - コントロールとメニューアイテムを無効にする
		 */
		EnableWindow(hWndBtnResume, FALSE);
		EnableWindow(hWndBtnNext, FALSE);
		EnableWindow(hWndBtnPause, FALSE);
		EnableWindow(hWndBtnMove, FALSE);
		EnableWindow(hWndTextboxScript, FALSE);
		EnableWindow(hWndBtnSelectScript, FALSE);
		SendMessage(hWndRichEdit, EM_SETREADONLY, TRUE, 0);
		SendMessage(hWndTextboxVar, EM_SETREADONLY, TRUE, 0);
		EnableWindow(hWndBtnVar, FALSE);
		EnableMenuItem(hMenu, ID_OPEN, MF_GRAYED);
		EnableMenuItem(hMenu, ID_SAVE, MF_GRAYED);
		EnableMenuItem(hMenu, ID_RELOAD, MF_GRAYED);
		EnableMenuItem(hMenu, ID_RESUME, MF_GRAYED);
		EnableMenuItem(hMenu, ID_NEXT, MF_GRAYED);
		EnableMenuItem(hMenu, ID_PAUSE, MF_GRAYED);
		EnableMenuItem(hMenu, ID_ERROR, MF_GRAYED);
		EnableMenuItem(hMenu, ID_EXPORT_WIN, MF_GRAYED);
		EnableMenuItem(hMenu, ID_EXPORT_WIN_INST, MF_GRAYED);
		EnableMenuItem(hMenu, ID_EXPORT_WIN_MAC, MF_GRAYED);
		EnableMenuItem(hMenu, ID_EXPORT_WEB, MF_GRAYED);
		EnableMenuItem(hMenu, ID_EXPORT_ANDROID, MF_GRAYED);
		EnableMenuItem(hMenu, ID_EXPORT_IOS, MF_GRAYED);
		EnableMenuItem(hMenu, ID_EXPORT_PACKAGE, MF_GRAYED);
		for (i = ID_CMD_MESSAGE; i <= ID_CMD_LOAD; i++)
			EnableMenuItem(hMenu, i, MF_GRAYED);

		/* 実行中の背景色を設定する */
		SetFocus(NULL);
		RichEdit_SetBackgroundColorForCurrentExecuteLine();
		SetFocus(hWndRichEdit);
	}
	else if(running)
	{
		/*
		 * 実行中のとき
		 *  - 「停止」だけ有効、他は無効にする
		 */
		EnableWindow(hWndBtnResume, FALSE);
		EnableWindow(hWndBtnNext, FALSE);
		EnableWindow(hWndBtnPause, TRUE);
		EnableWindow(hWndBtnMove, FALSE);
		EnableWindow(hWndTextboxScript, FALSE);
		EnableWindow(hWndBtnSelectScript, FALSE);
		SendMessage(hWndRichEdit, EM_SETREADONLY, TRUE, 0);
		SendMessage(hWndTextboxVar, EM_SETREADONLY, TRUE, 0);
		EnableWindow(hWndBtnVar, FALSE);
		EnableMenuItem(hMenu, ID_OPEN, MF_GRAYED);
		EnableMenuItem(hMenu, ID_SAVE, MF_GRAYED);
		EnableMenuItem(hMenu, ID_RELOAD, MF_GRAYED);
		EnableMenuItem(hMenu, ID_RESUME, MF_GRAYED);
		EnableMenuItem(hMenu, ID_NEXT, MF_GRAYED);
		EnableMenuItem(hMenu, ID_PAUSE, MF_ENABLED);
		EnableMenuItem(hMenu, ID_ERROR, MF_GRAYED);
		EnableMenuItem(hMenu, ID_EXPORT_WIN, MF_GRAYED);
		EnableMenuItem(hMenu, ID_EXPORT_WIN_INST, MF_GRAYED);
		EnableMenuItem(hMenu, ID_EXPORT_WIN_MAC, MF_GRAYED);
		EnableMenuItem(hMenu, ID_EXPORT_WEB, MF_GRAYED);
		EnableMenuItem(hMenu, ID_EXPORT_ANDROID, MF_GRAYED);
		EnableMenuItem(hMenu, ID_EXPORT_IOS, MF_GRAYED);
		EnableMenuItem(hMenu, ID_EXPORT_PACKAGE, MF_GRAYED);
		for (i = ID_CMD_MESSAGE; i <= ID_CMD_LOAD; i++)
			EnableMenuItem(hMenu, i, MF_GRAYED);

		/* 実行中の背景色を設定する */
		SetFocus(NULL);
		RichEdit_SetBackgroundColorForCurrentExecuteLine();
		SetFocus(hWndRichEdit);
	}
	else
	{
		/*
		 * 完全に停止中のとき
		 *  - 「停止」だけ無効、他は有効にする
		 */
		EnableWindow(hWndBtnResume, TRUE);
		EnableWindow(hWndBtnNext, TRUE);
		EnableWindow(hWndBtnPause, FALSE);
		EnableWindow(hWndBtnMove, TRUE);
		EnableWindow(hWndTextboxScript, TRUE);
		EnableWindow(hWndBtnSelectScript, TRUE);
		SendMessage(hWndRichEdit, EM_SETREADONLY, FALSE, 0);
		SendMessage(hWndTextboxVar, EM_SETREADONLY, FALSE, 0);
		EnableWindow(hWndBtnVar, TRUE);
		EnableMenuItem(hMenu, ID_OPEN, MF_ENABLED);
		EnableMenuItem(hMenu, ID_SAVE, MF_ENABLED);
		EnableMenuItem(hMenu, ID_RELOAD, MF_ENABLED);
		EnableMenuItem(hMenu, ID_RESUME, MF_ENABLED);
		EnableMenuItem(hMenu, ID_NEXT, MF_ENABLED);
		EnableMenuItem(hMenu, ID_PAUSE, MF_GRAYED);
		EnableMenuItem(hMenu, ID_ERROR, MF_ENABLED);
		EnableMenuItem(hMenu, ID_EXPORT_WIN, MF_ENABLED);
		EnableMenuItem(hMenu, ID_EXPORT_WIN_INST, MF_ENABLED);
		EnableMenuItem(hMenu, ID_EXPORT_WIN_MAC, MF_ENABLED);
		EnableMenuItem(hMenu, ID_EXPORT_WEB, MF_ENABLED);
		EnableMenuItem(hMenu, ID_EXPORT_ANDROID, MF_ENABLED);
		EnableMenuItem(hMenu, ID_EXPORT_IOS, MF_ENABLED);
		EnableMenuItem(hMenu, ID_EXPORT_PACKAGE, MF_ENABLED);
		for (i = ID_CMD_MESSAGE; i <= ID_CMD_LOAD; i++)
			EnableMenuItem(hMenu, i, MF_ENABLED);

		/* 次の実行される行の背景色を設定する */
		SetFocus(NULL);
		RichEdit_SetBackgroundColorForNextExecuteLine();
		SetFocus(hWndRichEdit);
	}
}

/*
 * スクリプトがロードされたときのコールバック
 */
void on_load_script(void)
{
	const char *script_file;

	/* スクリプトファイル名を設定する */
	script_file = get_script_file_name();
	SetWindowText(hWndTextboxScript, conv_utf8_to_utf16(script_file));

	/* 実行中のスクリプトファイルが変更されたとき、リッチエディットにテキストを設定する */
	SetFocus(NULL);
	RichEdit_SetTextByScriptModel();
	SetFocus(hWndRichEdit);

	/* 全体のテキスト色を変更する(遅延) */
	RichEdit_DelayedHighligth();
}

/*
 * 実行位置が変更されたときのコールバック
 */
void on_change_position(void)
{
	SetFocus(NULL);

	/* 実行行のハイライトを行う */
	if (!bRunning)
		RichEdit_SetBackgroundColorForNextExecuteLine();
	else
		RichEdit_SetBackgroundColorForCurrentExecuteLine();

	/* 変数の情報を更新する */
	if (bNeedUpdateVars)
	{
		Variable_UpdateText();
		bNeedUpdateVars = FALSE;
	}

	/* スクロールする */
	RichEdit_AutoScroll();

	SetFocus(hWndRichEdit);
}

/*
 * 変数が変更されたときのコールバック
 */
void on_update_variable(void)
{
	bNeedUpdateVars = TRUE;
}

/*
 * 変数テキストボックス
 */

/* 変数の情報を更新する */
static VOID Variable_UpdateText(void)
{
	static wchar_t szTextboxVar[VAR_TEXTBOX_MAX];
	wchar_t line[1024];
	int index;
	int val;

	szTextboxVar[0] = L'\0';

	for(index = 0; index < LOCAL_VAR_SIZE + GLOBAL_VAR_SIZE; index++)
	{
		/* 変数が初期値の場合 */
		val = get_variable(index);
		if(val == 0 && !is_variable_changed(index))
			continue;

		/* 行を追加する */
		_snwprintf(line,
				   sizeof(line) / sizeof(wchar_t),
				   L"$%d=%d\r\n",
				   index,
				   val);
		line[1023] = L'\0';
		wcscat(szTextboxVar, line);
	}

	/* テキストボックスにセットする */
	SetWindowText(hWndTextboxVar, szTextboxVar);
}

/*
 * リッチエディット
 */

/* リッチエディットの内容の更新通知を処理する */
static VOID RichEdit_OnChange(void)
{
	int nCursor;

	if (bIgnoreChange)
	{
		bIgnoreChange = FALSE;
		return;
	}

	/* カーソル位置を取得する */
	nCursor = RichEdit_GetCursorPosition();

	SetFocus(NULL);

	/* フォントを設定する */
	RichEdit_SetFont();

	/* 実行行の背景色を設定する */
	if (bRunning)
		RichEdit_SetBackgroundColorForCurrentExecuteLine();
	else
		RichEdit_SetBackgroundColorForNextExecuteLine();

	/*
	 * カーソル位置を設定する
	 *  - 色付けで選択が変更されたのを修正する
	 */
	RichEdit_SetCursorPosition(nCursor);

	SetFocus(hWndRichEdit);

	/* 全体のテキスト色を変更する(遅延) */
	RichEdit_DelayedHighligth();
}

/* リッチエディットのフォントを設定する */
static VOID RichEdit_SetFont(void)
{
	CHARFORMAT2W cf;

	bIgnoreChange = TRUE;
	memset(&cf, 0, sizeof(cf));
	cf.cbSize = sizeof(cf);
	cf.dwMask = CFM_FACE | CFM_SIZE;
	cf.yHeight = nFontSize * 20;
	wcscpy(&cf.szFaceName[0], wszFontName);
	SendMessage(hWndRichEdit, EM_SETCHARFORMAT, (WPARAM)SCF_ALL, (LPARAM)&cf);
}

/* リッチエディットのカーソル位置を取得する */
static int RichEdit_GetCursorPosition(void)
{
	CHARRANGE cr;

	/* カーソル位置を取得する */
	SendMessage(hWndRichEdit, EM_EXGETSEL, 0, (LPARAM)&cr);

	return cr.cpMin;
}

/* リッチエディットの選択範囲の長さを取得する */
static int RichEdit_GetSelectedLen(void)
{
	CHARRANGE cr;

	/* カーソル位置を取得する */
	SendMessage(hWndRichEdit, EM_EXGETSEL, 0, (LPARAM)&cr);

	return cr.cpMax - cr.cpMin;
}

/* リッチエディットのカーソル位置を設定する */
static VOID RichEdit_SetCursorPosition(int nCursor)
{
	CHARRANGE cr;

	bIgnoreChange = TRUE;
	cr.cpMin = nCursor;
	cr.cpMax = nCursor;
	SendMessage(hWndRichEdit, EM_EXSETSEL, 0, (LPARAM)&cr);
}

/* リッチエディットの範囲を選択する */
static VOID RichEdit_SetSelectedRange(int nLineStart, int nLineLen)
{
	CHARRANGE cr;

	bIgnoreChange = TRUE;
	memset(&cr, 0, sizeof(cr));
	cr.cpMin = nLineStart;
	cr.cpMax = nLineStart + nLineLen;
	SendMessage(hWndRichEdit, EM_EXSETSEL, 0, (LPARAM)&cr);
}

/* リッチエディットのカーソル行の行番号を取得する */
static int RichEdit_GetCursorLine(void)
{
	wchar_t *pWcs, *pCRLF;
	int nTotal, nCursor, nLineStartCharCR, nLineStartCharCRLF, nLine;

	pWcs = RichEdit_GetText();
	nTotal = (int)wcslen(pWcs);
	nCursor = RichEdit_GetCursorPosition();
	nLineStartCharCR = 0;
	nLineStartCharCRLF = 0;
	nLine = 0;
	while (nLineStartCharCRLF < nTotal)
	{
		pCRLF = wcswcs(pWcs + nLineStartCharCRLF, L"\r\n");
		int nLen = (pCRLF != NULL) ?
			(int)(pCRLF - (pWcs + nLineStartCharCRLF)) :
			(int)wcslen(pWcs + nLineStartCharCRLF);
		if (nCursor >= nLineStartCharCR && nCursor <= nLineStartCharCR + nLen)
			break;
		nLineStartCharCRLF += nLen + 2; /* +2 for CRLF */
		nLineStartCharCR += nLen + 1; /* +1 for CR */
		nLine++;
	}
	free(pWcs);

	return nLine;
}

/* リッチエディットのテキストを取得する */
static wchar_t *RichEdit_GetText(void)
{
	wchar_t *pText;
	int nTextLen;

	/* リッチエディットのテキストの長さを取得する */
	nTextLen = (int)SendMessage(hWndRichEdit, WM_GETTEXTLENGTH, 0, 0);
	if (nTextLen == 0)
	{
		pText = wcsdup(L"");
		if (pText == NULL)
		{
			log_memory();
			abort();
		}
	}

	/* テキスト全体を取得する */
	pText = malloc((size_t)(nTextLen + 1) * sizeof(wchar_t));
	if (pText == NULL)
	{
		log_memory();
		abort();
	}
	SendMessage(hWndRichEdit, WM_GETTEXT, (WPARAM)(nTextLen + 1), (LPARAM)pText);
	pText[nTextLen] = L'\0';

	return pText;
}

/* リッチエディットのテキストすべてについて、行の内容により色付けを行う */
static VOID RichEdit_SetTextColorForAllLines(void)
{
	wchar_t *pText, *pLineStop;
	int i, nLineStartCRLF, nLineStartCR, nLineLen;

	pText = RichEdit_GetText();
	nLineStartCRLF = 0;		/* WM_GETTEXTは改行をCRLFで返す */
	nLineStartCR = 0;		/* EM_EXSETSELでは改行はCRの1文字 */
	for (i = 0; i < get_line_count(); i++)
	{
		/* 行の終了位置を求める */
		pLineStop = wcswcs(pText + nLineStartCRLF, L"\r\n");
		nLineLen = pLineStop != NULL ?
			(int)(pLineStop - (pText + nLineStartCRLF)) :
			(int)wcslen(pText + nLineStartCRLF);

		/* 行の色付けを行う */
		RichEdit_SetTextColorForLine(pText, nLineStartCR, nLineStartCRLF, nLineLen);

		/* 次の行へ移動する */
		nLineStartCRLF += nLineLen + 2;	/* +2 for CRLF */
		nLineStartCR += nLineLen + 1;	/* +1 for CR */
	}
	free(pText);
}

/* 特定の行のテキスト色を設定する */
static VOID RichEdit_SetTextColorForLine(const wchar_t *pText, int nLineStartCR, int nLineStartCRLF, int nLineLen)
{
	wchar_t wszCommandName[1024];
	const wchar_t *pCommandSpaceStop, *pCommandCRStop, *pParamStart, *pParamStop, *pParamSpace;
	int nParamLen, nCommandType;

	/* 行を選択して選択範囲のテキスト色をデフォルトに変更する */
	RichEdit_SetSelectedRange(nLineStartCR, nLineLen);
	RichEdit_SetTextColorForSelectedRange(dwColorFgDefault);

	/* コメントを処理する */
	if (pText[nLineStartCRLF] == L'#')
	{
		/* 行全体を選択して、選択範囲のテキスト色を変更する */
		RichEdit_SetSelectedRange(nLineStartCR, nLineLen);
		RichEdit_SetTextColorForSelectedRange(dwColorComment);
	}
	/* ラベルを処理する */
	else if (pText[nLineStartCRLF] == L':')
	{
		/* 行全体を選択して、選択範囲のテキスト色を変更する */
		RichEdit_SetSelectedRange(nLineStartCR, nLineLen);
		RichEdit_SetTextColorForSelectedRange(dwColorLabel);
	}
	/* エラー行を処理する */
	if (pText[nLineStartCRLF] == L'!')
	{
		/* 行全体を選択して、選択範囲のテキスト色を変更する */
		RichEdit_SetSelectedRange(nLineStartCR, nLineLen);
		RichEdit_SetTextColorForSelectedRange(dwColorError);
	}
	/* コマンド行を処理する */
	else if (pText[nLineStartCRLF] == L'@')
	{
		/* コマンド名部分を抽出する */
		pCommandSpaceStop = wcswcs(pText + nLineStartCRLF, L" ");
		pCommandCRStop = wcswcs(pText + nLineStartCRLF, L"\r\n");
		if (pCommandSpaceStop == NULL && pCommandCRStop == NULL)
			nParamLen = nLineLen; /* EOF */
		else if (pCommandSpaceStop < pCommandCRStop)
			nParamLen = (int)(pCommandSpaceStop - (pText + nLineStartCRLF));
		else
			nParamLen = (int)(pCommandCRStop - (pText + nLineStartCRLF));
		wcsncpy(wszCommandName, &pText[nLineStartCRLF],
				(size_t)nParamLen < sizeof(wszCommandName) / sizeof(wchar_t) ?
				(size_t)nParamLen :
				sizeof(wszCommandName) / sizeof(wchar_t));
		wszCommandName[nParamLen] = L'\0';

		nCommandType = get_command_type_from_name(conv_utf16_to_utf8(wszCommandName));
		if (nCommandType != -1)
		{
			/* コマンド名のテキストに色を付ける */
			RichEdit_SetSelectedRange(nLineStartCR, nParamLen);
			RichEdit_SetTextColorForSelectedRange(dwColorCommandName);

			if (nCommandType != COMMAND_SET &&
				nCommandType != COMMAND_IF &&
				nCommandType != COMMAND_UNLESS &&
				nCommandType != COMMAND_PENCIL)
			{
				/* 引数名を灰色にする */
				pParamStart = pText + nLineStartCRLF + nParamLen;
				while ((pParamStart = wcswcs(pParamStart, L" ")) != NULL)
				{
					int nNameStart;
					int nNameLen;

					/* 次の行以降の' 'にヒットしている場合はループから抜ける */
					if (pParamStart >= pText + nLineStartCRLF + nLineLen)
						break;

					/* ' 'の次の文字を開始位置にする */
					pParamStart++;

					/* '='を探す。次の行以降にヒットした場合はループから抜ける */
					pParamStop = wcswcs(pParamStart, L"=");
					if (pParamStop == NULL || pParamStop >= pText + nLineStartCRLF + nLineLen)
						break;

					/* '='の手前に' 'があればスキップする */
					pParamSpace = wcswcs(pParamStart, L" ");
					if (pParamSpace != NULL && pParamSpace < pParamStop)
						continue;

					/* 引数名部分を選択してテキスト色を変更する */
					nNameStart = nLineStartCR + (pParamStart - (pText + nLineStartCRLF));
					nNameLen = pParamStop - pParamStart + 1;
					RichEdit_SetSelectedRange(nNameStart, nNameLen);
					RichEdit_SetTextColorForSelectedRange(dwColorParamName);
				}
			}
		}
	}
}

/* 次の実行行の背景色を設定する */
static VOID RichEdit_SetBackgroundColorForNextExecuteLine(void)
{
	int nLine, nLineStart, nLineLen;

	/* すべてのテキストの背景色を白にする */
	RichEdit_ClearBackgroundColorAll();

	/* 実行行を取得する */
	nLine = get_expanded_line_num();

	/* 実行行の開始文字と終了文字を求める */
	RichEdit_GetLineStartAndLength(nLine, &nLineStart, &nLineLen);

	/* 実行行を選択する */
	RichEdit_SetSelectedRange(nLineStart, nLineLen);

	/* 選択範囲の背景色を変更する */
	RichEdit_SetBackgroundColorForSelectedRange(dwColorNextExec);

	/* カーソル位置を実行行の先頭に設定する */
	RichEdit_SetCursorPosition(nLineStart);
}

/* 現在実行中の行の背景色を設定する */
static VOID RichEdit_SetBackgroundColorForCurrentExecuteLine(void)
{
	int nLine, nLineStart, nLineLen;

	/* すべてのテキストの背景色を白にする */
	RichEdit_ClearBackgroundColorAll();

	/* 実行行を取得する */
	nLine = get_expanded_line_num();

	/* 実行行の開始文字と終了文字を求める */
	RichEdit_GetLineStartAndLength(nLine, &nLineStart, &nLineLen);

	/* 実行行を選択する */
	RichEdit_SetSelectedRange(nLineStart, nLineLen);

	/* 選択範囲の背景色を変更する */
	RichEdit_SetBackgroundColorForSelectedRange(dwColorCurrentExec);

	/* カーソル位置を実行行の先頭に設定する */
	RichEdit_SetCursorPosition(nLineStart);
}

/* リッチエディットの書式をクリアする */
static VOID RichEdit_ClearFormatAll(void)
{
	CHARFORMAT2W cf;

	bIgnoreChange = TRUE;
	memset(&cf, 0, sizeof(cf));
	cf.cbSize = sizeof(cf);
	cf.dwMask = CFM_BACKCOLOR | CFM_COLOR;
	cf.crBackColor = dwColorBgDefault;
	cf.crTextColor = dwColorFgDefault;
	SendMessage(hWndRichEdit, EM_SETCHARFORMAT, (WPARAM)SCF_ALL, (LPARAM)&cf);
}

/* リッチエディットのテキスト全体の背景色をクリアする */
static VOID RichEdit_ClearBackgroundColorAll(void)
{
	CHARFORMAT2W cf;

	bIgnoreChange = TRUE;
	memset(&cf, 0, sizeof(cf));
	cf.cbSize = sizeof(cf);
	cf.dwMask = CFM_BACKCOLOR;
	cf.crBackColor = dwColorBgDefault;
	SendMessage(hWndRichEdit, EM_SETCHARFORMAT, (WPARAM)SCF_ALL, (LPARAM)&cf);
}

/* リッチエディットの選択範囲のテキスト色を変更する */
static VOID RichEdit_SetTextColorForSelectedRange(COLORREF cl)
{
	CHARFORMAT2W cf;

	bIgnoreChange = TRUE;
	memset(&cf, 0, sizeof(cf));
	cf.cbSize = sizeof(cf);
	cf.dwMask = CFM_COLOR;
	cf.crTextColor = cl;
	bIgnoreChange = TRUE;
	SendMessage(hWndRichEdit, EM_SETCHARFORMAT, (WPARAM)SCF_SELECTION, (LPARAM)&cf);
}

/* リッチエディットの選択範囲の背景色を変更する */
static VOID RichEdit_SetBackgroundColorForSelectedRange(COLORREF cl)
{
	CHARFORMAT2W cf;

	bIgnoreChange = TRUE;
	memset(&cf, 0, sizeof(cf));
	cf.cbSize = sizeof(cf);
	cf.dwMask = CFM_BACKCOLOR;
	cf.crBackColor = cl;
	SendMessage(hWndRichEdit, EM_SETCHARFORMAT, (WPARAM)SCF_SELECTION, (LPARAM)&cf);
}

/* リッチエディットを自動スクロールする */
static VOID RichEdit_AutoScroll(void)
{
	/* リッチエディットをフォーカスする */
	SetFocus(hWndRichEdit);

	/* リッチエディットをスクロールする */
	SendMessage(hWndRichEdit, EM_SETREADONLY, TRUE, 0);
	SendMessage(hWndRichEdit, EM_SCROLLCARET, 0, 0);
	SendMessage(hWndRichEdit, EM_SETREADONLY, FALSE, 0);

	/* リッチエディットを再描画する */
	InvalidateRect(hWndRichEdit, NULL, TRUE);
}

/* 実行行の開始文字と終了文字を求める */
static VOID RichEdit_GetLineStartAndLength(int nLine, int *nLineStart, int *nLineLen)
{
	wchar_t *pText, *pCRLF;
	int i, nLineStartCharCRLF, nLineStartCharCR;

	pText = RichEdit_GetText();
	nLineStartCharCRLF = 0;		/* WM_GETTEXTは改行をCRLFで返す */
	nLineStartCharCR = 0;		/* EM_EXSETSELでは改行はCRの1文字 */
	for (i = 0; i < nLine; i++)
	{
		int nLen;
		pCRLF = wcswcs(pText + nLineStartCharCRLF, L"\r\n");
		nLen = pCRLF != NULL ?
			(int)(pCRLF - (pText + nLineStartCharCRLF)) :
			(int)wcslen(pText + nLineStartCharCRLF);
		nLineStartCharCRLF += nLen + 2;		/* +2 for CRLF */
		nLineStartCharCR += nLen + 1;		/* +1 for CR */
	}
	pCRLF = wcswcs(pText + nLineStartCharCRLF, L"\r\n");
	*nLineStart = nLineStartCharCR;
	*nLineLen = pCRLF != NULL ?
		(int)(pCRLF - (pText + nLineStartCharCRLF)) :
		(int)wcslen(pText + nLineStartCharCRLF);
	free(pText);
}

/* リッチエディットで次のエラーを探す */
static BOOL RichEdit_SearchNextError(int nStart, int nEnd)
{
	wchar_t *pWcs, *pCRLF, *pLine;
	int nTotal, nLineStartCharCR, nLineStartCharCRLF, nLen;
	BOOL bFound;

	/* リッチエディットのテキストの内容でスクリプトの各行をアップデートする */
	pWcs = RichEdit_GetText();
	nTotal = (int)wcslen(pWcs);
	nLineStartCharCR = nStart;
	nLineStartCharCRLF = 0;
	bFound = FALSE;
	while (nLineStartCharCRLF < nTotal)
	{
		if (nEnd != -1 && nLineStartCharCRLF >= nEnd)
			break;

		/* 行を切り出す */
		pLine = pWcs + nLineStartCharCRLF;
		pCRLF = wcswcs(pLine, L"\r\n");
		nLen = (pCRLF != NULL) ?
			(int)(pCRLF - (pWcs + nLineStartCharCRLF)) :
			(int)wcslen(pWcs + nLineStartCharCRLF);
		if (pCRLF != NULL)
			*pCRLF = L'\0';

		/* エラーを発見したらカーソルを移動する */
		if (pLine[0] == L'!')
		{
			bFound = TRUE;
			RichEdit_SetCursorPosition(nLineStartCharCR);
			break;
		}

		nLineStartCharCRLF += nLen + 2; /* +2 for CRLF */
		nLineStartCharCR += nLen + 1; /* +1 for CR */
	}
	free(pWcs);

	return bFound;
}

/* リッチエディットのテキストをスクリプトモデルを元に設定する */
static VOID RichEdit_SetTextByScriptModel(void)
{
	wchar_t *pWcs;
	int nScriptSize;
	int i;

	/* スクリプトのサイズを計算する */
	nScriptSize = 0;
	for (i = 0; i < get_line_count(); i++)
	{
		const char *pUtf8Line = get_line_string_at_line_num(i);
		nScriptSize += (int)strlen(pUtf8Line) + 1; /* +1 for CR */
	}

	/* スクリプトを格納するメモリを確保する */
	pWcs = malloc((size_t)(nScriptSize + 1) * sizeof(wchar_t));
	if (pWcs == NULL)
	{
		log_memory();
		abort();
	}

	/* 行を連列してスクリプト文字列を作成する */
	pWcs[0] = L'\0';
	for (i = 0; i < get_line_count(); i++)
	{
		const char *pUtf8Line = get_line_string_at_line_num(i);
		wcscat(pWcs, conv_utf8_to_utf16(pUtf8Line));
		wcscat(pWcs, L"\r");
	}

	/* リッチエディットにテキストを設定する */
	bIgnoreChange = TRUE;
	SetWindowText(hWndRichEdit, pWcs);

	/* メモリを解放する */
	free(pWcs);
}

/* リッチエディットの内容を元にスクリプトモデルを更新する */
static VOID RichEdit_UpdateScriptModelFromText(void)
{
	char szLine[2048];
	wchar_t *pWcs, *pCRLF;
	int i, nTotal, nLine, nLineStartCharCR, nLineStartCharCRLF;

	/* パースエラーをリセットして、最初のパースエラーで通知を行う */
	dbg_reset_parse_error_count();

	/* リッチエディットのテキストの内容でスクリプトの各行をアップデートする */
	pWcs = RichEdit_GetText();
	nTotal = (int)wcslen(pWcs);
	nLine = 0;
	nLineStartCharCR = 0;
	nLineStartCharCRLF = 0;
	while (nLineStartCharCRLF < nTotal)
	{
		wchar_t *pLine;
		int nLen;

		/* 行を切り出す */
		pLine = pWcs + nLineStartCharCRLF;
		pCRLF = wcswcs(pWcs + nLineStartCharCRLF, L"\r\n");
		nLen = (pCRLF != NULL) ?
			(int)(pCRLF - (pWcs + nLineStartCharCRLF)) :
			(int)wcslen(pWcs + nLineStartCharCRLF);
		if (pCRLF != NULL)
			*pCRLF = L'\0';

		/* 行を更新する */
		strncpy(szLine, conv_utf16_to_utf8(pLine), sizeof(szLine) - 1);
		szLine[sizeof(szLine) - 1] = '\0';
		if (nLine < get_line_count())
			update_script_line(nLine, szLine);
		else
			insert_script_line(nLine, szLine);

		nLine++;
		nLineStartCharCRLF += nLen + 2; /* +2 for CRLF */
		nLineStartCharCR += nLen + 1; /* +1 for CR */
	}
	free(pWcs);

	/* 削除された末尾の行を処理する */
	bExecLineChanged = FALSE;
	for (i = get_line_count() - 1; i >= nLine; i--)
		if (delete_script_line(nLine))
			bExecLineChanged = TRUE;

	/* 拡張構文がある場合に対応する */
	reparse_script_for_structured_syntax();

	/* コマンドのパースに失敗した場合 */
	if (dbg_get_parse_error_count() > 0)
	{
		/* 行頭の'!'を反映するためにテキストを再設定する */
		RichEdit_SetTextByScriptModel();
	}
}

/* テキストを挿入する */
static VOID RichEdit_InsertText(const wchar_t *pFormat, ...)
{
	va_list ap;
	wchar_t buf[1024];
		
	int nLine, nLineStart, nLineLen;

	va_start(ap, pFormat);
	vswprintf(buf, sizeof(buf) / sizeof(wchar_t), pFormat, ap);
	va_end(ap);

	/* カーソル行を取得する */
	nLine = RichEdit_GetCursorLine();

	/* 行の先頭にカーソルを移す */
	RichEdit_GetLineStartAndLength(nLine, &nLineStart, &nLineLen);
	RichEdit_SetCursorPosition(nLineStart);

	/* スクリプトモデルに行を追加する */
	insert_script_line(nLine, conv_utf16_to_utf8(buf));

	/* リッチエディットに行を追加する */
	wcscat(buf, L"\r");
	RichEdit_SetTextColorForSelectedRange(dwColorFgDefault);
	SendMessage(hWndRichEdit, EM_REPLACESEL, (WPARAM)TRUE, (LPARAM)buf);

	/* 行を選択する */
	RichEdit_SetCursorPosition(nLineStart);

	/* 次のフレームで実行位置を変更する */
	nLineChanged = nLine;
	bExecLineChanged = TRUE;
}

static VOID RichEdit_UpdateTheme(void)
{
	int nCursor;

	if (hWndRichEdit == NULL)
		return;

	SetFocus(NULL);

	nCursor = RichEdit_GetCursorPosition();
	SendMessage(hWndRichEdit, EM_SETBKGNDCOLOR, (WPARAM)0, (LPARAM)dwColorBgDefault);
	RichEdit_ClearFormatAll();
	RichEdit_ClearBackgroundColorAll();
	if (bHighlightMode)
		RichEdit_SetTextColorForAllLines();
	if (!bRunning)
		RichEdit_SetBackgroundColorForNextExecuteLine();
	else
		RichEdit_SetBackgroundColorForCurrentExecuteLine();
	RichEdit_SetCursorPosition(nCursor);
	RichEdit_AutoScroll();

	SetFocus(hWndRichEdit);
}

static VOID RichEdit_DelayedHighligth(void)
{
	if (bHighlightMode)
	{
		KillTimer(hWndMain, ID_TIMER_FORMAT);
		SetTimer(hWndMain, ID_TIMER_FORMAT, 1000, OnTimerFormat);
	}
}

static VOID __stdcall OnTimerFormat(HWND hWnd, UINT nID, UINT_PTR uTime, DWORD dwParam)
{
	HIMC hImc;
	DWORD dwConversion, dwSentence;
	int nCursor;
	BOOL bRet;

	UNUSED_PARAMETER(hWnd);
	UNUSED_PARAMETER(nID);
	UNUSED_PARAMETER(uTime);
	UNUSED_PARAMETER(dwParam);

	/* 選択範囲がある場合は更新せず、1秒後に再び確認する */
	if (RichEdit_GetSelectedLen() > 0)
		return;

	/* IMEを使用中は更新せず、1秒後に再び確認する */
	hImc = ImmGetContext(hWndRichEdit);
	if (hImc != NULL)
	{
		if (ImmGetOpenStatus(hImc))
		{
			bRet = ImmGetConversionStatus(hImc, &dwConversion, &dwSentence);
			ImmReleaseContext(hWndRichEdit, hImc);
			if (bRet)
			{
				if ((dwConversion & IME_CMODE_CHARCODE) != 0 ||
					(dwConversion & IME_CMODE_EUDC) != 0 ||
					(dwConversion & IME_CMODE_FIXED) != 0 ||
					(dwConversion & IME_CMODE_HANJACONVERT) != 0 ||
					(dwConversion & IME_CMODE_KATAKANA) != 0 ||
					(dwConversion & IME_CMODE_NOCONVERSION) != 0 ||
					(dwConversion & IME_CMODE_ROMAN) != 0 ||
					(dwConversion & IME_CMODE_SOFTKBD) != 0 ||
					(dwConversion & IME_CMODE_SYMBOL) != 0)
				{
					/* 入力中 */
					return;
				}
			}
		}
	}

	/* タイマを止める */
	KillTimer(hWndMain, ID_TIMER_FORMAT);

	/* 現在のカーソル位置を取得する */
	nCursor = RichEdit_GetCursorPosition();

	/* スクロールを避けるためにフォーカスを外す */
	SetFocus(NULL);

	/* フォントを適用する */
	bIgnoreChange = TRUE;
	RichEdit_SetFont();

	/* ハイライトを適用する */
	if (bHighlightMode)
	{
		bIgnoreChange = TRUE;
		RichEdit_SetTextColorForAllLines();
	}

	/* カーソル位置を戻す */
	RichEdit_SetCursorPosition(nCursor);

	/* フォーカスを戻す */
	SetFocus(hWndRichEdit);
}

/*
 * Project
 */

/* Create a new project or choose an existing project. */
static VOID __stdcall OnTimerBeginner(HWND hWnd, UINT nID, UINT_PTR uTime, DWORD dwParam)
{
	UNUSED_PARAMETER(hWnd);
	UNUSED_PARAMETER(nID);
	UNUSED_PARAMETER(uTime);
	UNUSED_PARAMETER(dwParam);

	KillTimer(hWndMain, ID_TIMER_BEGINNER);
	if (bProjectOpened)
		return;

	MessageBox(hWndMain,
			   bEnglish ?
			   L"May I help you?\n"
			   L"\n"
			   L"Press File->New game to create a new game.\n"
			   L"Press File->Open game to open an existing game." :
			   L"お困りですか？\n"
			   L"\n"
			   L"メニューの「ファイル」→「新規ゲーム」を押すと新規ゲームを作成します。\n"
			   L"メニューの「ファイル」→「ゲームを開く」を押すと既存ゲームを開きます。\n",
			   TITLE,
			   MB_OK);
}

/* Create a new project from a template. */
static BOOL CreateProjectFromTemplate(void)
{
	static wchar_t wszPath[1024];
	OPENFILENAMEW ofn;
    WIN32_FIND_DATAW wfd;
    HANDLE hFind;
	HANDLE hFile;
	wchar_t *pFile;
	BOOL bNonEmpty;

	while (1)
	{
		/* Show a save dialog. */
		ZeroMemory(&ofn, sizeof(OPENFILENAMEW));
		wcscpy(&wszPath[0], L"game.suika2project");
		ofn.lStructSize = sizeof(OPENFILENAMEW);
		ofn.nFilterIndex  = 1;
		ofn.lpstrFile = wszPath;
		ofn.nMaxFile = sizeof(wszPath) / sizeof(wchar_t);
		ofn.Flags = OFN_OVERWRITEPROMPT;
		ofn.lpstrFilter = bEnglish ?
			L"Suika2 Project Files\0*.suika2project;*.suika2project\0\0" :
			L"Suika2 プロジェクトファイル\0*.suika2project\0\0";
		ofn.lpstrDefExt = L".suika2project";
		if (!GetSaveFileNameW(&ofn))
			return FALSE;
		if (ofn.lpstrFile[0] == L'\0')
			return FALSE;

		/* Get the base file name. */
		pFile = wcsrchr(ofn.lpstrFile, L'\\');
		if (pFile == NULL)
			return FALSE;
		pFile++;

		/* Check if the directory is empty. */
		hFind = FindFirstFileW(L".\\*.*", &wfd);
		if (hFind == INVALID_HANDLE_VALUE)
		{
			MessageBox(NULL, bEnglish ?
					   L"Invalid folder. Please choose one again." :
					   L"フォルダが存在しません。選択しなおしてください。",
					   TITLE,
					   MB_OK | MB_ICONERROR);
			continue;
		}
		bNonEmpty = FALSE;
		do
		{
			if ((wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && wfd.cFileName[0] == L'.')
				continue;
			bNonEmpty = TRUE;
			break;
		} while(FindNextFileW(hFind, &wfd));
		FindClose(hFind);
		if (bNonEmpty)
		{
			MessageBox(NULL, bEnglish ?
					   L"Folder is not empty. Please create an empty folder." :
					   L"フォルダが空ではありません。空のフォルダを作成してください。",
					   TITLE,
					   MB_OK | MB_ICONERROR);
			continue;
		}

		/* Finish choosing a directory. */
		break;
	}

	/* プロジェクトファイルを作成する */
	hFile = CreateFileW(pFile, GENERIC_WRITE, 0, NULL, CREATE_NEW, 0, NULL);
	CloseHandle(hFile);

	/* .vscodeを生成する */
	CreateDirectory(L".\\.vscode", 0);
	CopyLibraryFiles(bEnglish ?
					 L"plaintext.code-snippets.en" :
					 L"plaintext.code-snippets.jp",
					 L".\\.vscode\\plaintext.code-snippets");

	/* テンプレートを選択する */
	if (MessageBox(NULL,
				   bEnglish ?
				   L"Do you want to use the good old full screen style, that is, NVL mode?\n"
				   L"(No is recommended.)" :
				   L"全画面スタイルにしますか？\n"
				   L"「はい」を押すと全画面スタイルにします。\n"
				   L"「いいえ」を押すとアドベンチャースタイルにします。\n",
				   TITLE,
				   MB_YESNO) == IDYES)
	{
		/* 英語の場合 */
		if (bEnglish)
			return CopyLibraryFiles(L"games\\nvl-en\\*", L".\\");

		/* 日本語の場合 */
		if (MessageBox(NULL, L"縦書きにしますか？", TITLE, MB_YESNO) == IDYES)
			return CopyLibraryFiles(L"games\\nvl-tategaki\\*", L".\\");
		else
			return CopyLibraryFiles(L"games\\nvl\\*", L".\\");
	}
	if (bEnglish)
		return CopyLibraryFiles(L"games\\english\\*", L".\\");
	return CopyLibraryFiles(L"games\\japanese\\*", L".\\");
}

/* Choose a project and open it. */
static BOOL ChooseProject(void)
{
	static wchar_t wszPath[1024];
	OPENFILENAMEW ofn;

	/* Open a file dialog. */
	ZeroMemory(&ofn, sizeof(OPENFILENAMEW));
	wcscpy(&wszPath[0], L"game.suika2project");
	ofn.lStructSize = sizeof(OPENFILENAMEW);
	ofn.nFilterIndex = 1;
	ofn.lpstrFile = wszPath;
	ofn.nMaxFile = sizeof(wszPath) / sizeof(wchar_t);
	ofn.Flags = OFN_FILEMUSTEXIST;
	ofn.lpstrFilter = bEnglish ?
		L"Suika2 Project Files\0*.suika2project;*.suika2project\0\0" :
		L"Suika2 プロジェクトファイル\0*.suika2project\0\0";
	ofn.lpstrDefExt = L".suika2project";

	/* This will set the working directory to the game directory. */
	if (!GetOpenFileNameW(&ofn))
		return FALSE;

	/* If no file was selected. */
	if(ofn. lpstrFile[0] == L'\0')
		return FALSE;

	/* Read a project file. */
	ReadProjectFile();

	return TRUE;
}

/* Open a project by a path. */
static BOOL OpenProjectAtPath(const wchar_t *pszPath)
{
	wchar_t path[1024];
	wchar_t *pLastSeparator;

	/* Get the folder name. */
	wcsncpy(path, pszPath, sizeof(path) / sizeof(wchar_t) - 1);
	path[sizeof(path) / sizeof(wchar_t) - 1] = L'\0';
	pLastSeparator = wcsrchr(path, L'\\');
	if (pLastSeparator == NULL)
	{
		MessageBox(NULL, bEnglish ?
				   L"Invalid file name." :
				   L"ファイル名が正しくありません。",
				   TITLE,
				   MB_OK | MB_ICONERROR);
		return FALSE;
	}
	*pLastSeparator = L'\0';

	/* Set the working directory. */
	if (!SetCurrentDirectory(path))
	{
		MessageBox(NULL, bEnglish ?
				   L"Invalid game folder." :
				   L"ゲームフォルダが正しくありません。",
				   TITLE,
				   MB_OK | MB_ICONERROR);
		return FALSE;
	}

	/* Read a project file. */
	ReadProjectFile();

	return TRUE;
}

static void ReadProjectFile(void)
{
	char buf[1024];
	FILE *fp;

	/* Defaults */
	bDarkMode = FALSE;
	bHighlightMode = FALSE;
	wcscpy(wszFontName, bEnglish ? SCRIPT_FONT_EN : SCRIPT_FONT_JP);
	nFontSize = 10;
	dwColorBgDefault = LIGHT_BG_DEFAULT;
	dwColorFgDefault = LIGHT_FG_DEFAULT;
	dwColorComment = LIGHT_COMMENT;
	dwColorLabel = LIGHT_LABEL;
	dwColorError = LIGHT_ERROR;
	dwColorCommandName = LIGHT_COMMAND_NAME;
	dwColorParamName = LIGHT_PARAM_NAME;
	dwColorNextExec = LIGHT_NEXT_EXEC;
	dwColorCurrentExec = LIGHT_CURRENT_EXEC;
	CheckMenuItem(hMenu, ID_HIGHLIGHTMODE, MF_UNCHECKED);
	CheckMenuItem(hMenu, ID_DARKMODE, MF_UNCHECKED);

	/* Read the preference. */
	fp = fopen("game.suika2project", "r");
	if (fp == NULL)
		log_info("failed to read the project file.");
	while (1)
	{
		char *stop;

		if (fgets(buf, sizeof(buf) - 1, fp) == NULL)
			break;
		stop = strstr(buf, "\n");
		if (stop != NULL)
			*stop = '\0';

		if (strncmp(buf, "font-name:", 10) == 0)
			wcscpy(wszFontName, conv_utf8_to_utf16(buf + 10));
		else if (strncmp(buf, "font-size:", 10) == 0)
			nFontSize = abs(atoi(buf + 10));
		else if (strcmp(buf, "highlightmode") == 0)
		{
			bHighlightMode = TRUE;
			CheckMenuItem(hMenu, ID_HIGHLIGHTMODE, MF_CHECKED);
		}
		else if (strcmp(buf, "darkmode") == 0)
		{
			bDarkMode = TRUE;
			dwColorBgDefault = DARK_BG_DEFAULT;
			dwColorFgDefault = DARK_FG_DEFAULT;
			dwColorComment = DARK_COMMENT;
			dwColorLabel = DARK_LABEL;
			dwColorError = DARK_ERROR;
			dwColorCommandName = DARK_COMMAND_NAME;
			dwColorParamName = DARK_PARAM_NAME;
			dwColorNextExec = DARK_NEXT_EXEC;
			dwColorCurrentExec = DARK_CURRENT_EXEC;
			CheckMenuItem(hMenu, ID_DARKMODE, MF_CHECKED);
		}
		else
		{
			log_info("unknown project setting: %s", buf);
		}
	}
	fclose(fp);

	RichEdit_UpdateTheme();
}

static void WriteProjectFile(void)
{
	FILE *fp;

	fp = fopen("game.suika2project", "w");
	if (fp == NULL)
		return;

	fprintf(fp, "font-name:%s\n", conv_utf16_to_utf8(wszFontName));
	fprintf(fp, "font-size:%d\n", nFontSize);
	if (bHighlightMode)
		fprintf(fp, "highlightmode\n");
	if (bDarkMode)
		fprintf(fp, "darkmode\n");

	fclose(fp);
}

/*
 * コマンド処理
 */

/* 新規ゲームプロジェクト作成 */
static VOID OnNewProject(void)
{
	KillTimer(hWndMain, ID_TIMER_BEGINNER);

	if (!CreateProjectFromTemplate())
		return;

	StartGame();
}

/* ゲームプロジェクトを開く */
static VOID OnOpenProject(void)
{
	KillTimer(hWndMain, ID_TIMER_BEGINNER);

	if (!ChooseProject())
		return;

	StartGame();
}

/* ゲームフォルダオープン */
static VOID OnOpenGameFolder(void)
{
	/* Explorerを開く */
	ShellExecuteW(NULL, L"explore", L".\\", NULL, NULL, SW_SHOW);
}

/* スクリプトオープン */
static VOID OnOpenScript(void)
{
	const wchar_t *pFile;

	pFile = SelectFile(SCRIPT_DIR);
	if (pFile == NULL)
		return;

	SetWindowText(hWndTextboxScript, pFile);
	bScriptOpened = TRUE;
	bExecLineChanged = FALSE;
	nLineChanged = 0;
}

/* スクリプトリロード */
static VOID OnReloadScript(void)
{
	bScriptOpened = TRUE;
	bExecLineChanged = TRUE;
	nLineChanged = get_expanded_line_num();
}

/* ファイルを開くダイアログを表示して素材ファイルを選択する */
static const wchar_t *SelectFile(const char *pszDir)
{
	static wchar_t wszPath[1024];
	wchar_t wszBase[1024];
	OPENFILENAMEW ofn;

	ZeroMemory(&wszPath[0], sizeof(wszPath));
	ZeroMemory(&ofn, sizeof(OPENFILENAMEW));

	/* ゲームのベースディレクトリを取得する */
	GetCurrentDirectory(sizeof(wszBase) / sizeof(wchar_t), wszBase);

	/* ファイルダイアログの準備を行う */
	ofn.lStructSize = sizeof(OPENFILENAMEW);
	ofn.hwndOwner = hWndMain;
	ofn.nFilterIndex = 1;
	ofn.lpstrFile = wszPath;
	ofn.nMaxFile = sizeof(wszPath);
	ofn.lpstrInitialDir = conv_utf8_to_utf16(pszDir);
	ofn.Flags = OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
	if (strcmp(pszDir, BG_DIR) == 0 ||
		strcmp(pszDir, CH_DIR) == 0)
	{
		ofn.lpstrFilter = bEnglish ?
			L"Image Files\0*.png;*.jpg;*.webp;\0All Files(*.*)\0*.*\0\0" : 
			L"画像ファイル\0*.png;*.jpg;*.webp;\0すべてのファイル(*.*)\0*.*\0\0";
		ofn.lpstrDefExt = L"png";
	}
	else if (strcmp(pszDir, BGM_DIR) == 0 ||
			 strcmp(pszDir, SE_DIR) == 0)
	{
		ofn.lpstrFilter = bEnglish ?
			L"Sound Files\0*.ogg;\0All Files(*.*)\0*.*\0\0" : 
			L"音声ファイル\0*.ogg;\0すべてのファイル(*.*)\0*.*\0\0";
		ofn.lpstrDefExt = L"ogg";
	}
	else if (strcmp(pszDir, MOV_DIR) == 0)
	{
		ofn.lpstrFilter = bEnglish ?
			L"Video Files\0*.mp4;*.wmv;\0All Files(*.*)\0*.*\0\0" : 
			L"動画ファイル\0*.mp4;*.wmv;\0すべてのファイル(*.*)\0*.*\0\0";
		ofn.lpstrDefExt = L"ogg";
	}
	else if (strcmp(pszDir, SCRIPT_DIR) == 0 ||
			 strcmp(pszDir, GUI_DIR) == 0)
	{
		ofn.lpstrFilter = bEnglish ?
			L"Text Files\0*.txt;\0All Files(*.*)\0*.*\0\0" : 
			L"テキストファイル\0*.txt;\0すべてのファイル(*.*)\0*.*\0\0";
		ofn.lpstrDefExt = L"txt";
	}

	/* ファイルダイアログを開く */
	GetOpenFileNameW(&ofn);
	if(ofn.lpstrFile[0] == L'\0')
		return NULL;
	if (wcswcs(wszPath, wszBase) == NULL)
		return NULL;
	if (wcslen(wszPath) < strlen(pszDir) + 1)
		return NULL;

	/* 素材ディレクトリ内の相対パスを返す */
	return wszPath + wcslen(wszBase) + 1 + strlen(pszDir) + 1;
}

/* 上書き保存 */
static VOID OnSave(void)
{
	if (!save_script())
	{
		MessageBox(hWndMain, bEnglish ?
				   L"Cannot write to file." :
				   L"ファイルに書き込めません。",
				   TITLE,
				   MB_OK | MB_ICONERROR);
	}
}

/* 続ける */
static VOID OnContinue(void)
{
	bContinuePressed = TRUE;
}

/* 次へ */
static VOID OnNext(void)
{
	bNextPressed = TRUE;
}

/* 停止 */
static VOID OnStop(void)
{
	bStopPressed = TRUE;
}

/* 移動 */
static VOID OnMove(void)
{
	RichEdit_UpdateScriptModelFromText();

	nLineChanged = RichEdit_GetCursorLine();
	bExecLineChanged = TRUE;

	SetFocus(NULL);
	RichEdit_SetBackgroundColorForNextExecuteLine();
	SetFocus(hWndRichEdit);

	RichEdit_DelayedHighligth();
}

/* 次のエラー箇所へ移動ボタンが押下されたとき */
static VOID OnNextError(void)
{
	int nStart;

	nStart = RichEdit_GetCursorPosition();
	if (RichEdit_SearchNextError(nStart, -1))
		return;

	if (RichEdit_SearchNextError(0, nStart - 1))
		return;

	MessageBox(hWndMain, bEnglish ?
			   L"No error.\n" :
			   L"エラーはありません。\n",
			   TITLE,
			   MB_ICONINFORMATION | MB_OK);
}

/* リッチエディットでのReturnを処理する(シフトキーなしの場合) */
static VOID OnReturn(void)
{
	int nCursorLine;

	/* スクリプトモデルを更新する */
	RichEdit_UpdateScriptModelFromText();

	/* パースエラーがないとき */
	if (dbg_get_parse_error_count() == 0)
	{
		/* 次フレームでの一行実行する */
		nCursorLine = RichEdit_GetCursorLine();
		if (nCursorLine != -1)
		{
			bNextPressed = TRUE;
			nLineChanged = nCursorLine;
			bExecLineChanged = TRUE;
		}
	}

	/* 全体のテキスト色を変更する(遅延) */
	RichEdit_DelayedHighligth();
}

/* ポップアップを表示する */
static VOID OnPopup(void)
{
	POINT point;

	GetCursorPos(&point);
	TrackPopupMenu(hMenuPopup, 0, point.x, point.y, 0, hWndMain, NULL);
}

/* 変数の書き込みボタンが押下された場合を処理する */
static VOID OnWriteVars(void)
{
	static wchar_t szTextboxVar[VAR_TEXTBOX_MAX];
	wchar_t *p, *next_line;
	int index, val;

	/* テキストボックスの内容を取得する */
	GetWindowText(hWndTextboxVar, szTextboxVar, sizeof(szTextboxVar) / sizeof(wchar_t) - 1);

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
		if(swscanf(p, L"$%d=%d", &index, &val) != 2)
			index = -1, val = -1;
		if(index >= LOCAL_VAR_SIZE + GLOBAL_VAR_SIZE)
			index = -1;

		/* 変数を設定する */
		if(index != -1)
			set_variable(index, val);

		/* 次の行へポインタを進める */
		p = next_line;
	}

	Variable_UpdateText();
}

/* パッケージを作成メニューが押下されたときの処理を行う */
VOID OnExportPackage(void)
{
	if (MessageBox(hWndMain, bEnglish ?
				   L"Are you sure you want to export the package file?\n"
				   L"This may take a while." :
				   L"パッケージをエクスポートします。\n"
				   L"この処理には時間がかかります。\n"
				   L"よろしいですか？",
				   TITLE,
				   MB_ICONWARNING | MB_OKCANCEL) != IDOK)
		return;

	/* パッケージを作成する */
	if (create_package("")) {
		log_info(bEnglish ?
				 "Successfully exported data01.arc" :
				 "data01.arcのエクスポートに成功しました。");
	}
}

/* Windows向けにエクスポートのメニューが押下されたときの処理を行う */
VOID OnExportWin(void)
{
	if (MessageBox(hWndMain, bEnglish ?
				   L"Takes a while. Are you sure?\n" :
				   L"エクスポートには時間がかかります。よろしいですか？",
				   TITLE,
				   MB_ICONWARNING | MB_OKCANCEL) != IDOK)
		return;

	/* パッケージを作成する */
	if (!create_package(""))
	{
		log_info(bEnglish ?
				 "Failed to export data01.arc" :
				 "data01.arcのエクスポートに失敗しました。");
		return;
	}

	/* フォルダを再作成する */
	RecreateDirectory(L".\\windows-export");

	/* ファイルをコピーする */
	if (!CopyLibraryFiles(L"tools\\suika.exe", L".\\windows-export\\suika.exe"))
	{
		log_info(bEnglish ?
				 "Failed to copy exe file." :
				 "実行ファイルのコピーに失敗しました。");
		return;
	}

	/* movをコピーする */
	CopyMovFiles(L".\\mov", L".\\windows-export\\mov");

	/* パッケージを移動する */
	if (!MovePackageFile(L".\\data01.arc", L".\\windows-export\\data01.arc"))
	{
		log_info(bEnglish ?
				 "Failed to move data01.arc" :
				 "data01.arcの移動に失敗しました。");
		return;
	}

	MessageBox(hWndMain, bEnglish ?
			   L"Export succeeded. Will open the folder." :
			   L"エクスポートに成功しました。フォルダを開きます。",
			   TITLE,
			   MB_ICONINFORMATION | MB_OK);

	/* Explorerを開く */
	ShellExecuteW(NULL, L"explore", L".\\windows-export", NULL, NULL, SW_SHOW);
}

/* Windows向けインストーラをエクスポートするのメニューが押下されたときの処理を行う */
VOID OnExportWinInst(void)
{
	wchar_t cmdline[1024];
	STARTUPINFOW si;
	PROCESS_INFORMATION pi;

	if (MessageBox(hWndMain, bEnglish ?
				   L"Takes a while. Are you sure?\n" :
				   L"エクスポートには時間がかかります。よろしいですか？",
				   TITLE,
				   MB_ICONWARNING | MB_OKCANCEL) != IDOK)
		return;

	/* パッケージを作成する */
	if (!create_package(""))
	{
		log_info(bEnglish ?
				 "Failed to export data01.arc" :
				 "data01.arcのエクスポートに失敗しました。");
		return;
	}

	/* フォルダを再作成する */
	RecreateDirectory(L".\\windows-installer-export");
	CreateDirectory(L".\\windows-installer-export\\asset", 0);

	/* ファイルをコピーする */
	if (!CopyLibraryFiles(L"tools\\suika.exe", L".\\windows-installer-export\\asset\\suika.exe"))
	{
		log_info(bEnglish ?
				 "Failed to copy exe file." :
				 "実行ファイルのコピーに失敗しました。");
		return;
	}

	/* ファイルをコピーする */
	if (!CopyLibraryFiles(L"tools\\installer\\create-installer.bat", L".\\windows-installer-export\\asset\\create-installer.bat"))
	{
		log_info(bEnglish ?
				 "Failed to copy exe file." :
				 "実行ファイルのコピーに失敗しました。");
		return;
	}

	/* ファイルをコピーする */
	if (!CopyLibraryFiles(L"tools\\installer\\icon.ico", L".\\windows-installer-export\\asset\\icon.ico"))
	{
		log_info(bEnglish ?
				 "Failed to copy exe file." :
				 "実行ファイルのコピーに失敗しました。");
		return;
	}

	/* ファイルをコピーする */
	if (!CopyLibraryFiles(L"tools\\installer\\install-script.nsi", L".\\windows-installer-export\\asset\\install-script.nsi"))
	{
		log_info(bEnglish ?
				 "Failed to copy exe file." :
				 "実行ファイルのコピーに失敗しました。");
		return;
	}

	/* movをコピーする */
	CopyMovFiles(L".\\mov", L".\\windows-installer-export\\asset\\mov");

	/* パッケージを移動する */
	if (!MovePackageFile(L".\\data01.arc", L".\\windows-installer-export\\asset\\data01.arc"))
	{
		log_info(bEnglish ?
				 "Failed to move data01.arc" :
				 "data01.arcの移動に失敗しました。");
		return;
	}

	/* バッチファイルを呼び出す */
	wcscpy(cmdline, L"cmd.exe /k create-installer.bat");
	ZeroMemory(&si, sizeof(STARTUPINFOW));
	si.cb = sizeof(STARTUPINFOW);
	CreateProcessW(NULL,	/* lpApplication */
				   cmdline,
				   NULL,	/* lpProcessAttribute */
				   NULL,	/* lpThreadAttributes */
				   FALSE,	/* bInheritHandles */
				   NORMAL_PRIORITY_CLASS | CREATE_NEW_CONSOLE | CREATE_NEW_PROCESS_GROUP,
				   NULL,	/* lpEnvironment */
				   L".\\windows-installer-export\\asset",
				   &si,
				   &pi);
	if (pi.hProcess != NULL)
		CloseHandle(pi.hThread);
	if (pi.hProcess != NULL)
		CloseHandle(pi.hProcess);
}

/* Windows/Mac向けにエクスポートのメニューが押下されたときの処理を行う */
VOID OnExportWinMac(void)
{
	if (MessageBox(hWndMain, bEnglish ?
				   L"Takes a while. Are you sure?\n" :
				   L"エクスポートには時間がかかります。よろしいですか？",
				   TITLE,
				   MB_ICONWARNING | MB_OKCANCEL) != IDOK)
		return;

	/* パッケージを作成する */
	if (!create_package(""))
	{
		log_info(bEnglish ?
				 "Failed to export data01.arc" :
				 "data01.arcのエクスポートに失敗しました。");
		return;
	}

	/* フォルダを再作成する */
	RecreateDirectory(L".\\windows-mac-export");

	/* ファイルをコピーする */
	if (!CopyLibraryFiles(L"tools\\suika.exe", L".\\windows-mac-export\\suika.exe"))
	{
		log_info(bEnglish ?
				 "Failed to copy exe file." :
				 "実行ファイルのコピーに失敗しました。");
		return;
	}

	/* ファイルをコピーする */
	if (!CopyLibraryFiles(L"tools\\suika-mac.dmg", L".\\windows-mac-export\\suika-mac.dmg"))
	{
		log_info(bEnglish ?
				 "Failed to copy exe file." :
				 "Mac用の実行ファイルのコピーに失敗しました。");
		return;
	}

	/* movをコピーする */
	CopyMovFiles(L".\\mov", L".\\windows-mac-export\\mov");

	/* パッケージを移動する */
	if (!MovePackageFile(L".\\data01.arc", L".\\windows-mac-export\\data01.arc"))
	{
		log_info(bEnglish ?
				 "Failed to move data01.arc" :
				 "data01.arcの移動に失敗しました。");
		return;
	}

	MessageBox(hWndMain, bEnglish ?
			   L"Export succeeded. Will open the folder." :
			   L"エクスポートに成功しました。フォルダを開きます。",
			   TITLE,
			   MB_ICONINFORMATION | MB_OK);

	/* Explorerを開く */
	ShellExecuteW(NULL, L"explore", L".\\windows-mac-export", NULL, NULL, SW_SHOW);
}

/* Web向けにエクスポートのメニューが押下されたときの処理を行う */
VOID OnExportWeb(void)
{
	if (MessageBox(hWndMain, bEnglish ?
				   L"Takes a while. Are you sure?\n" :
				   L"エクスポートには時間がかかります。よろしいですか？",
				   TITLE,
				   MB_ICONWARNING | MB_OKCANCEL) != IDOK)
		return;

	/* パッケージを作成する */
	if (!create_package(""))
	{
		log_info(bEnglish ?
				 "Failed to export data01.arc" :
				 "data01.arcのエクスポートに失敗しました。");
		return;
	}

	/* フォルダを再作成する */
	RecreateDirectory(L".\\web-export");

	/* ソースをコピーする */
	if (!CopyLibraryFiles(L"tools\\web\\*", L".\\web-export"))
	{
		log_info(bEnglish ?
				 "Failed to copy source files for Web." :
				 "ソースコードのコピーに失敗しました。"
				 "最新のtools/webフォルダが存在するか確認してください。");
		return;
	}

	/* movをコピーする */
	CopyMovFiles(L".\\mov", L".\\web-export\\mov");

	/* パッケージを移動する */
	if (!MovePackageFile(L".\\data01.arc", L".\\web-export\\data01.arc"))
	{
		log_info(bEnglish ?
				 "Failed to move data01.arc" :
				 "data01.arcの移動に失敗しました。");
		return;
	}

	MessageBox(hWndMain, bEnglish ?
			   L"Export succeeded. Will open the folder." :
			   L"エクスポートに成功しました。フォルダを開きます。",
			   TITLE,
			   MB_ICONINFORMATION | MB_OK);

	/* Explorerを開く */
	ShellExecuteW(NULL, L"explore", L".\\web-export", NULL, NULL, SW_SHOW);
}

/* Androidプロジェクトをエクスポートのメニューが押下されたときの処理を行う */
VOID OnExportAndroid(void)
{
	if (MessageBox(hWndMain, bEnglish ?
				   L"Takes a while. Are you sure?\n" :
				   L"エクスポートには時間がかかります。よろしいですか？",
				   TITLE,
				   MB_ICONWARNING | MB_OKCANCEL) != IDOK)
		return;

	/* フォルダを再作成する */
	RecreateDirectory(L".\\android-export");

	/* ソースをコピーする */
	if (!CopyLibraryFiles(L"tools\\android-src", L".\\android-export"))
	{
		log_info(bEnglish ?
				 "Failed to copy source files for Android." :
				 "ソースコードのコピーに失敗しました。"
				 "最新のtools/android-srcフォルダが存在するか確認してください。");
		return;
	}

	/* アセットをコピーする */
	CopyGameFiles(L".\\anime", L".\\android-export\\app\\src\\main\\assets\\anime");
	CopyGameFiles(L".\\bg", L".\\android-export\\app\\src\\main\\assets\\bg");
	CopyGameFiles(L".\\bgm", L".\\android-export\\app\\src\\main\\assets\\bgm");
	CopyGameFiles(L".\\cg", L".\\android-export\\app\\src\\main\\assets\\cg");
	CopyGameFiles(L".\\ch", L".\\android-export\\app\\src\\main\\assets\\ch");
	CopyGameFiles(L".\\conf", L".\\android-export\\app\\src\\main\\assets\\conf");
	CopyGameFiles(L".\\cv", L".\\android-export\\app\\src\\main\\assets\\cv");
	CopyGameFiles(L".\\font", L".\\android-export\\app\\src\\main\\assets\\font");
	CopyGameFiles(L".\\gui", L".\\android-export\\app\\src\\main\\assets\\gui");
	CopyGameFiles(L".\\mov", L".\\android-export\\app\\src\\main\\assets\\mov");
	CopyGameFiles(L".\\rule", L".\\android-export\\app\\src\\main\\assets\\rule");
	CopyGameFiles(L".\\se", L".\\android-export\\app\\src\\main\\assets\\se");
	CopyGameFiles(L".\\txt", L".\\android-export\\app\\src\\main\\assets\\txt");
	CopyGameFiles(L".\\wms", L".\\android-export\\app\\src\\main\\assets\\wms");

	MessageBox(hWndMain, bEnglish ?
			   L"Will open the exported source code folder.\n"
			   L"Build with Android Studio." :
			   L"エクスポートしたソースコードフォルダを開きます。\n"
			   L"Android Studioでそのままビルドできます。",
			   TITLE,
			   MB_ICONINFORMATION | MB_OK);

	/* Explorerを開く */
	ShellExecuteW(NULL, L"explore", L".\\android-export", NULL, NULL, SW_SHOW);
}

/* iOSプロジェクトをエクスポートのメニューが押下されたときの処理を行う */
VOID OnExportIOS(void)
{
	if (MessageBox(hWndMain, bEnglish ?
				   L"Takes a while. Are you sure?\n" :
				   L"エクスポートには時間がかかります。よろしいですか？",
				   TITLE,
				   MB_ICONWARNING | MB_OKCANCEL) != IDOK)
		return;

	/* パッケージを作成する */
	if (!create_package(""))
	{
		log_info(bEnglish ?
				 "Failed to export data01.arc" :
				 "data01.arcのエクスポートに失敗しました。");
		return;
	}

	/* フォルダを再作成する */
	RecreateDirectory(L".\\ios-export");

	/* ソースをコピーする */
	if (!CopyLibraryFiles(L"tools\\ios-src", L".\\ios-export"))
	{
		log_info(bEnglish ?
				 "Failed to copy source files for Android." :
				 "ソースコードのコピーに失敗しました。"
				 "最新のtools/ios-srcフォルダが存在するか確認してください。");
		return;
	}

	/* movをコピーする */
	CopyMovFiles(L".\\mov", L".\\ios-export\\engine-ios\\mov");

	/* パッケージを移動する */
	if (!MovePackageFile(L".\\data01.arc", L".\\ios-export\\engine-ios\\data01.arc"))
	{
		log_info(bEnglish ?
				 "Failed to move data01.arc" :
				 "data01.arcの移動に失敗しました。");
		return;
	}

	MessageBox(hWndMain, bEnglish ?
			   L"Will open the exported source code folder.\n"
			   L"Build with Xcode." :
			   L"エクスポートしたソースコードフォルダを開きます。\n"
			   L"Xcodeでそのままビルドできます。\n",
			   TITLE,
			   MB_ICONINFORMATION | MB_OK);

	/* Explorerを開く */
	ShellExecuteW(NULL, L"explore", L".\\ios-export", NULL, NULL, SW_SHOW);
}

/* フォルダを再作成する */
static VOID RecreateDirectory(const wchar_t *path)
{
	wchar_t newpath[MAX_PATH];
	SHFILEOPSTRUCT fos;

	/* 二重のNUL終端を行う */
	wcscpy(newpath, path);
	newpath[wcslen(path) + 1] = L'\0';

	/* コピーする */
	ZeroMemory(&fos, sizeof(SHFILEOPSTRUCT));
	fos.wFunc = FO_DELETE;
	fos.pFrom = newpath;
	fos.fFlags = FOF_NOCONFIRMATION | FOF_NOERRORUI | FOF_SILENT;
	SHFileOperationW(&fos);
}

/* ライブラリファイルをコピーする (インストール先 to エクスポート先) */
static BOOL CopyLibraryFiles(const wchar_t *lpszSrcDir, const wchar_t *lpszDestDir)
{
	wchar_t from[MAX_PATH];
	wchar_t to[MAX_PATH];
	SHFILEOPSTRUCTW fos;
	wchar_t *pSep;
	int ret;

	/* コピー元を求める */
	GetModuleFileName(NULL, from, MAX_PATH);
	pSep = wcsrchr(from, L'\\');
	if (pSep != NULL)
		*(pSep + 1) = L'\0';
	wcscat(from, lpszSrcDir);
	from[wcslen(from) + 1] = L'\0';	/* 二重のNUL終端を行う */

	/* コピー先を求める */
	wcscpy(to, lpszDestDir);
	to[wcslen(lpszDestDir) + 1] = L'\0';	/* 二重のNUL終端を行う */

	/* コピーする */
	ZeroMemory(&fos, sizeof(SHFILEOPSTRUCT));
	fos.wFunc = FO_COPY;
	fos.pFrom = from;
	fos.pTo = to;
	fos.fFlags = FOF_NOCONFIRMATION | FOF_NOCONFIRMMKDIR;
	ret = SHFileOperationW(&fos);
	if (ret != 0)
	{
		log_info("%s: error code = %d", conv_utf16_to_utf8(lpszSrcDir), ret);
		return FALSE;
	}

	return TRUE;
}

/* ゲームファイルをコピーする (ゲーム内 to エクスポート先) */
static BOOL CopyGameFiles(const wchar_t* lpszSrcDir, const wchar_t* lpszDestDir)
{
	wchar_t from[MAX_PATH];
	wchar_t to[MAX_PATH];
	SHFILEOPSTRUCTW fos;
	int ret;

	/* 二重のNUL終端を行う */
	wcscpy(from, lpszSrcDir);
	from[wcslen(lpszSrcDir) + 1] = L'\0';
	wcscpy(to, lpszDestDir);
	to[wcslen(lpszDestDir) + 1] = L'\0';

	/* コピーする */
	ZeroMemory(&fos, sizeof(SHFILEOPSTRUCT));
	fos.wFunc = FO_COPY;
	fos.pFrom = from;
	fos.pTo = to;
	fos.fFlags = FOF_NOCONFIRMATION | FOF_NOCONFIRMMKDIR;
	ret = SHFileOperationW(&fos);
	if (ret != 0)
	{
		log_info("%s: error code = %d", conv_utf16_to_utf8(lpszSrcDir), ret);
		return FALSE;
	}

	return TRUE;
}

/* movをコピーする */
static BOOL CopyMovFiles(const wchar_t *lpszSrcDir, const wchar_t *lpszDestDir)
{
	wchar_t from[MAX_PATH];
	wchar_t to[MAX_PATH];
	SHFILEOPSTRUCTW fos;
	int ret;

	/* 二重のNUL終端を行う */
	wcscpy(from, lpszSrcDir);
	from[wcslen(lpszSrcDir) + 1] = L'\0';
	wcscpy(to, lpszDestDir);
	to[wcslen(lpszDestDir) + 1] = L'\0';

	/* コピーする */
	ZeroMemory(&fos, sizeof(SHFILEOPSTRUCT));
	fos.wFunc = FO_COPY;
	fos.pFrom = from;
	fos.pTo = to;
	fos.fFlags = FOF_NOCONFIRMATION | FOF_NOCONFIRMMKDIR | FOF_NOERRORUI |
		FOF_SILENT;
	ret = SHFileOperationW(&fos);
	if (ret != 0)
	{
		log_info("%s: error code = %d", conv_utf16_to_utf8(lpszSrcDir), ret);
		return FALSE;
	}

	return TRUE;
}

/* パッケージファイルを移動する */
static BOOL MovePackageFile(const wchar_t *lpszPkgFile, wchar_t *lpszDestDir)
{
	wchar_t from[MAX_PATH];
	wchar_t to[MAX_PATH];
	SHFILEOPSTRUCTW fos;
	int ret;

	/* 二重のNUL終端を行う */
	wcscpy(from, lpszPkgFile);
	from[wcslen(lpszPkgFile) + 1] = L'\0';
	wcscpy(to, lpszDestDir);
	to[wcslen(lpszDestDir) + 1] = L'\0';

	/* 移動する */
	ZeroMemory(&fos, sizeof(SHFILEOPSTRUCT));
	fos.hwnd = NULL;
	fos.wFunc = FO_MOVE;
	fos.pFrom = from;
	fos.pTo = to;
	fos.fFlags = FOF_NOCONFIRMATION;
	ret = SHFileOperationW(&fos);
	if (ret != 0)
	{
		log_info("error code = %d", ret);
		return FALSE;
	}

	return TRUE;
}

/*
 * View
 */

static VOID OnFont(void)
{
	CHOOSEFONT cf;
	LOGFONT lf;
	HDC hDC;

	ZeroMemory(&cf, sizeof(cf));
	cf.lStructSize = sizeof(cf);
	cf.hwndOwner = hWndMain;
	cf.lpLogFont = &lf;
	cf.Flags = CF_NOVERTFONTS;
	if (!ChooseFont(&cf))
		return;

	hDC = GetDC(NULL);
	nFontSize = -MulDiv (lf.lfHeight, 72, GetDeviceCaps(hDC, LOGPIXELSY));
	ReleaseDC(NULL, hDC);

	wcscpy(wszFontName, lf.lfFaceName);

	if (bProjectOpened)
		WriteProjectFile();

	RichEdit_SetFont();
}

static VOID OnHighlightMode(void)
{
	if (!bHighlightMode)
	{
		CheckMenuItem(hMenu, ID_HIGHLIGHTMODE, MF_CHECKED);
		bHighlightMode = TRUE;
	}
	else
	{
		CheckMenuItem(hMenu, ID_HIGHLIGHTMODE, MF_UNCHECKED);
		bHighlightMode = FALSE;
	}

	if (bProjectOpened)
		WriteProjectFile();

	RichEdit_UpdateTheme();
}

static VOID OnDarkMode(void)
{
	if (!bDarkMode)
	{
		CheckMenuItem(hMenu, ID_DARKMODE, MF_CHECKED);
		bDarkMode = TRUE;
		dwColorBgDefault = DARK_BG_DEFAULT;
		dwColorFgDefault = DARK_FG_DEFAULT;
		dwColorComment = DARK_COMMENT;
		dwColorLabel = DARK_LABEL;
		dwColorError = DARK_ERROR;
		dwColorCommandName = DARK_COMMAND_NAME;
		dwColorParamName = DARK_PARAM_NAME;
		dwColorNextExec = DARK_NEXT_EXEC;
		dwColorCurrentExec = DARK_CURRENT_EXEC;
	}
	else
	{
		CheckMenuItem(hMenu, ID_DARKMODE, MF_UNCHECKED);
		bDarkMode = FALSE;
		dwColorBgDefault = LIGHT_BG_DEFAULT;
		dwColorFgDefault = LIGHT_FG_DEFAULT;
		dwColorComment = LIGHT_COMMENT;
		dwColorLabel = LIGHT_LABEL;
		dwColorError = LIGHT_ERROR;
		dwColorCommandName = LIGHT_COMMAND_NAME;
		dwColorParamName = LIGHT_PARAM_NAME;
		dwColorNextExec = LIGHT_NEXT_EXEC;
		dwColorCurrentExec = LIGHT_CURRENT_EXEC;
	}

	if (bProjectOpened)
		WriteProjectFile();

	RichEdit_UpdateTheme();
}

/*
 * suika2コマンドの挿入
 */

static VOID OnInsertMessage(void)
{
	if (bEnglish)
		RichEdit_InsertText(L"Edit this message and press return.");
	else
		RichEdit_InsertText(L"この行のメッセージを編集して改行してください。");
}

static VOID OnInsertSerif(void)
{
	if (bEnglish)
		RichEdit_InsertText(L"*Name*Edit this line and press return.");
	else
		RichEdit_InsertText(L"名前「このセリフを編集して改行してください。」");
}

static VOID OnInsertBg(void)
{
	const wchar_t *pFile;

	pFile = SelectFile(BG_DIR);
	if (pFile == NULL)
		return;

	if (bEnglish)
		RichEdit_InsertText(L"@bg file=%ls duration=1.0", pFile);
	else
		RichEdit_InsertText(L"@背景 ファイル=%ls 秒=1.0", pFile);
}

static VOID OnInsertBgOnly(void)
{
	const wchar_t *pFile;

	pFile = SelectFile(BG_DIR);
	if (pFile == NULL)
		return;

	if (bEnglish)
		RichEdit_InsertText(L"@chsx bg=%ls duration=1.0", pFile);
	else
		RichEdit_InsertText(L"@場面転換X 背景=%ls 秒=1.0", pFile);
}

static VOID OnInsertCh(void)
{
	const wchar_t *pFile;

	pFile = SelectFile(CH_DIR);
	if (pFile == NULL)
		return;

	if (bEnglish)
		RichEdit_InsertText(L"@ch position=center file=%ls duration=1.0", pFile);
	else
		RichEdit_InsertText(L"@キャラ 位置=中央 ファイル=%ls 秒=1.0", pFile);
}

static VOID OnInsertChsx(void)
{
	if (bEnglish)
		RichEdit_InsertText(L"@chsx left=file-name.png center=file-name.png right=file-name.png back=file-name.png bg=file-name.png duration=1.0");
	else
		RichEdit_InsertText(L"@場面転換X 左=ファイル名.png 中央=ファイル名.png 右=ファイル名.png 背面=ファイル名.png 背景=ファイル名.png 秒=1.0");
}

static VOID OnInsertBgm(void)
{
	const wchar_t *pFile;

	pFile = SelectFile(BGM_DIR);
	if (pFile == NULL)
		return;

	if (bEnglish)
		RichEdit_InsertText(L"@bgm file=%ls", pFile);
	else
		RichEdit_InsertText(L"@音楽 ファイル=%ls", pFile);
}

static VOID OnInsertBgmStop(void)
{
	if (bEnglish)
		RichEdit_InsertText(L"@bgm stop");
	else
		RichEdit_InsertText(L"@音楽 停止");
}

static VOID OnInsertVolBgm(void)
{
	if (bEnglish)
		RichEdit_InsertText(L"@vol track=bgm volume=1.0 duration=1.0");
	else
		RichEdit_InsertText(L"@音量 トラック=bgm 音量=1.0 秒=1.0");
}

static VOID OnInsertSe(void)
{
	const wchar_t *pFile;

	pFile = SelectFile(BGM_DIR);
	if (pFile == NULL)
		return;

	if (bEnglish)
		RichEdit_InsertText(L"@se file=%ls", pFile);
	else
		RichEdit_InsertText(L"@効果音 ファイル=%ls", pFile);
}

static VOID OnInsertSeStop(void)
{
	if (bEnglish)
		RichEdit_InsertText(L"@se stop");
	else
		RichEdit_InsertText(L"@効果音 停止");
}

static VOID OnInsertVolSe(void)
{
	if (bEnglish)
		RichEdit_InsertText(L"@vol track=se volume=1.0 duration=1.0");
	else
		RichEdit_InsertText(L"@音量 トラック=se 音量=1.0 秒=1.0");
}

static VOID OnInsertVideo(void)
{
	wchar_t buf[1024], *pExt;
	const wchar_t *pFile;

	pFile = SelectFile(MOV_DIR);
	if (pFile == NULL)
		return;

	/* 拡張子を削除する */
	wcscpy(buf, pFile);
	pExt = wcswcs(buf, L".mp4");
	if (pExt == NULL)
		pExt = wcswcs(buf, L".wmv");
	if (pExt != NULL)
		*pExt = L'\0';

	if (bEnglish)
		RichEdit_InsertText(L"@video file=%ls", buf);
	else
		RichEdit_InsertText(L"@動画 ファイル=%ls", buf);
}

static VOID OnInsertShakeH(void)
{
	if (bEnglish)
		RichEdit_InsertText(L"@shake direction=horizontal duration=1.0 times=3 amplitude-100");
	else
		RichEdit_InsertText(L"@振動 方向=横 秒=1.0 回数=3 大きさ=100");
}

static VOID OnInsertShakeV(void)
{
	if (bEnglish)
		RichEdit_InsertText(L"@shake direction=vertical duration=1.0 times=3 amplitude=100");
	else
		RichEdit_InsertText(L"@振動 方向=縦 秒=1.0 回数=3 大きさ=100\r");
}

static VOID OnInsertChoose3(void)
{
	if (bEnglish)
		RichEdit_InsertText(L"@choose L1 \"Option1\" L2 \"Option2\" L3 \"Option3\"");
	else
		RichEdit_InsertText(L"@選択肢 L1 \"候補1\" L2 \"候補2\" L3 \"候補3\"");
}

static VOID OnInsertChoose2(void)
{
	if (bEnglish)
		RichEdit_InsertText(L"@choose L1 \"Option1\" L2 \"Option2\"");
	else
		RichEdit_InsertText(L"@選択肢 L1 \"候補1\" L2 \"候補2\"");
}

static VOID OnInsertChoose1(void)
{
	if (bEnglish)
		RichEdit_InsertText(L"@choose L1 \"Option1\"");
	else
		RichEdit_InsertText(L"@選択肢 L1 \"候補1\"");
}

static VOID OnInsertGui(void)
{
	const wchar_t *pFile;

	pFile = SelectFile(GUI_DIR);
	if (pFile == NULL)
		return;

	if (bEnglish)
		RichEdit_InsertText(L"@gui file=%ls", pFile);
	else
		RichEdit_InsertText(L"@メニュー ファイル=%ls", pFile);
}

static VOID OnInsertClick(void)
{
	if (bEnglish)
		RichEdit_InsertText(L"@click");
	else
		RichEdit_InsertText(L"@クリック");
}

static VOID OnInsertWait(void)
{
	if (bEnglish)
		RichEdit_InsertText(L"@wait duration=1.0");
	else
		RichEdit_InsertText(L"@時間待ち 秒=1.0");
}

static VOID OnInsertLoad(void)
{
	const wchar_t *pFile;

	pFile = SelectFile(SCRIPT_DIR);
	if (pFile == NULL)
		return;

	if (bEnglish)
		RichEdit_InsertText(L"@load file=%ls", pFile);
	else
		RichEdit_InsertText(L"@シナリオ ファイル=%ls", pFile);
}
