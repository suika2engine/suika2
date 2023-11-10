/* -*- coding: utf-8; indent-tabs-mode: t; tab-width: 4; c-basic-offset: 4; -*- */

/*
 * Suika2
 * Copyright (C) 2001-2023, Keiichi Tabata. All rights reserved.
 */

/*
 * [Changes]
 *  2014-05-24 作成 (conskit)
 *  2016-05-29 作成 (suika)
 *  2017-11-07 フルスクリーンで解像度変更するように修正
 *  2023-09-20 Android/iOSエクスポート対応
 *  2023-10-25 エディタ対応
 */

/* Suika2 Base */
#include "suika.h"

/* Suika2 Pro */
#include "windebug.h"
#include "package.h"

/* Windows */
#include <windows.h>
#include <commctrl.h>	/* TOOLINFO */
#include <richedit.h>
#include "resource.h"

/* Standard C (msvcrt.dll) */
#include <stdlib.h>	/* exit() */

/* MSVC */
#define wcsdup(s)	_wcsdup(s)

/*
 * Constants
 */

#define COLOR_BG_DEFAULT	0x00ffffff
#define COLOR_FG_DEFAULT	0x00000000
#define COLOR_COMMENT		0x00808080
#define COLOR_LABEL			0x00ff0000
#define COLOR_ERROR			0x000000ff
#define COLOR_COMMAND_NAME	0x00ff0000
#define COLOR_PARAM_NAME	0x00c0f0c0
#define COLOR_NEXT_EXEC		0x00ffc0c0
#define COLOR_CURRENT_EXEC	0x00c0c0ff

/* 変数テキストボックスのテキストの最大長(形: "$00001=12345678901\r\n") */
#define VAR_TEXTBOX_MAX		(11000 * (1 + 5 + 1 + 11 + 2))

/* ウィンドウクラス名 */
static const wchar_t wszWindowClass[] = L"SuikaDebugPanel";

/*
 * Variables
 */

/* ウィンドウのハンドル (winmain.cで作成) */
static HWND hWndMain;		/* メインウィンドウ */
static HWND hWndGame;		/* ゲーム部分の描画領域のパネル */

/* デバッガのコントロール */
static HWND hWndDebug;				/* デバッガのパネル */
static HWND hWndBtnResume;			/* 「続ける」ボタン */
static HWND hWndBtnNext;				/* 「次へ」ボタン */
static HWND hWndBtnPause;			/* 「停止」ボタン */
static HWND hWndTextboxScript;		/* ファイル名のテキストボックス */
static HWND hWndBtnSelectScript;		/* ファイル選択のボタン */
static HWND hWndRichEdit;			/* スクリプトのリッチエディット */
static HWND hWndTextboxVar;			/* 変数一覧のテキストボックス */
static HWND hWndBtnVar;				/* 変数を反映するボタン */

/* ウィンドウのメニュー */
static HMENU hMenu;

/* ポップアップメニュー */
static HMENU hMenuPopup;

/* 英語モードか */
static BOOL bEnglish;

/* 実行中であるか */
static BOOL bRunning;

/* 発生したイベントの状態 */	
static BOOL bContinuePressed;		/* 「続ける」ボタンが押下された */
static BOOL bNextPressed;			/* 「次へ」ボタンが押下された */
static BOOL bStopPressed;			/* 「停止」ボタンが押下された */
static BOOL bScriptOpened;			/* スクリプトファイルが選択された */
static BOOL bExecLineChanged;		/* 実行行が変更された */
static int nLineChanged;			/* 実行行が変更された場合の行番号 */
static BOOL bRangedChanged;			/* 複数行の変更が加えられるか */
static BOOL bFirstChange;			/* スクリプトモデル変更後、最初の通知 */
static BOOL bIgnoreChange;			/* リッチエディットへの変更を無視する */

/* GetDpiForWindow() APIへのポインタ */
UINT (__stdcall *pGetDpiForWindow)(HWND);

/*
 * Forward Declaration
 */

/* Initialization */
static VOID InitMenu(HWND hWnd);
static HWND CreateTooltip(HWND hWndBtn, const wchar_t *pszTextEnglish, const wchar_t *pszTextJapanese);

/* Command Handlers */
static VOID OnOpenScript(void);
static const wchar_t *SelectFile(const char *pszDir);
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

/* Export Helpers */
static VOID RecreateDirectory(const wchar_t *path);
static BOOL CopySourceFiles(const wchar_t *lpszSrcDir, const wchar_t *lpszDestDir);
static BOOL CopyMovFiles(const wchar_t *lpszSrcDir, const wchar_t *lpszDestDir);
static BOOL MovePackageFile(const wchar_t *lpszPkgFile, wchar_t *lpszDestDir);

/* TextEdit for Variables */
static VOID Variable_UpdateText(void);

/* RichEdit Handlers */
static VOID RichEdit_OnChange(void);
static VOID RichEdit_OnReturn(void);

/* RichEdit helpers */
static VOID RichEdit_SetFont(void);
static int RichEdit_GetCursorPosition(void);
static VOID RichEdit_SetCursorPosition(int nCursor);
static VOID RichEdit_GetSelectedRange(int *nStart, int *nEnd);
static VOID RichEdit_SetSelectedRange(int nLineStart, int nLineLen);
static int RichEdit_GetCursorLine(void);
static wchar_t *RichEdit_GetText(void);
static VOID RichEdit_SetTextColorForAllLines(void);
static VOID RichEdit_SetTextColorForCursorLine(void);
static VOID RichEdit_SetTextColorForLine(const wchar_t *pText, int nLineStartCR, int nLineStartCRLF, int nLineLen);
static VOID RichEdit_ClearBackgroundColorAll(void);
static VOID RichEdit_SetBackgroundColorForNextExecuteLine(void);
static VOID RichEdit_SetBackgroundColorForCurrentExecuteLine(void);
static VOID RichEdit_GetLineStartAndLength(int nLine, int *nLineStart, int *nLineLen);
static VOID RichEdit_SetTextColorForSelectedRange(COLORREF cl);
static VOID RichEdit_SetBackgroundColorForSelectedRange(COLORREF cl);
static VOID RichEdit_AutoScroll(void);
static BOOL RichEdit_IsLineTop(void);
static BOOL RichEdit_IsLineEnd(void);
static VOID RichEdit_GetLineStartAndLength(int nLine, int *nLineStart, int *nLineLen);
static BOOL RichEdit_SearchNextError(int nStart, int nEnd);

/* Script Model */
static VOID RichEdit_UpdateTextFromScriptModel(void);
static VOID RichEdit_UpdateScriptModelFromText(void);
static VOID RichEdit_UpdateScriptModelFromCurrentLineText(void);
static VOID RichEdit_InsertText(const wchar_t *pLine, ...);

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
 * コマンドライン引数の処理
 */

/*
 * 引数が指定された場合はパッケージャとして機能する
 */
VOID DoPackagingIfArgExists(VOID)
{
	if (__argc == 2 && wcscmp(__wargv[1], L"--package") == 0)
	{
		if (!create_package(""))
		{
			log_error("Packaging error!");
			exit(1);
		}
		exit(0);
	}
}

/*
 * スタートアップファイル/ラインを取得する
 */
BOOL GetStartupPosition(void)
{
	int line;
	if (__argc >= 3) {
		/* 先に行番号を取得する(conv_*()がstatic変数が指すため) */
		line = atoi(conv_utf16_to_utf8(__wargv[2]));

		/* スタートアップファイル/ラインを指定する */
		if (!set_startup_file_and_line(conv_utf16_to_utf8(__wargv[1]), line))
			return FALSE;

		/* 実行開始ボタンが押されたことにする */
		bContinuePressed = TRUE;
	}

	return TRUE;
}

/*
 * 初期化
 */

/*
 * DPI拡張の初期化を行う
 */
VOID InitDpiExtension(void)
{
	HMODULE hModule;

	hModule = LoadLibrary(L"user32.dll");
	if (hModule == NULL)
		return;

	pGetDpiForWindow = (void *)GetProcAddress(hModule, "GetDpiForWindow");
}

/*
 * DPIを取得する
 */
int Win11_GetDpiForWindow(HWND hWnd)
{
	int nDpi;

	if (pGetDpiForWindow == NULL)
		return 96;

	nDpi = (int)pGetDpiForWindow(hWnd);
	if (nDpi == 0)
		return 96;

	return nDpi;
}

/*
 * デバッガパネルを作成する
 */
BOOL InitDebuggerPanel(HWND hMainWnd, HWND hGameWnd, void *pWndProc)
{
	WNDCLASSEX wcex;
	RECT rcClient;
	HFONT hFont, hFontFixed;
	int nDpi;

	hWndMain = hMainWnd;
	hWndGame= hGameWnd;

	/* 領域の矩形を取得する */
	GetClientRect(hWndMain, &rcClient);

	/* ウィンドウクラスを登録する */
	ZeroMemory(&wcex, sizeof(wcex));
	wcex.cbSize			= sizeof(WNDCLASSEX);
	wcex.lpfnWndProc    = pWndProc;
	wcex.hbrBackground  = (HBRUSH)(COLOR_BTNFACE + 1);
	wcex.lpszClassName  = wszWindowClass;
	if (!RegisterClassEx(&wcex))
		return FALSE;

	/* ウィンドウを作成する */
	hWndDebug = CreateWindowEx(0,
							   wszWindowClass,
							   NULL,
							   WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS,
							   rcClient.right - DEBUGGER_PANEL_WIDTH,
							   0,
							   DEBUGGER_PANEL_WIDTH,
							   rcClient.bottom,
							   hWndMain,
							   NULL,
							   GetModuleHandle(NULL),
							   NULL);
	if(!hWndDebug)
		return FALSE;

	/* DPIを取得する */
	nDpi = Win11_GetDpiForWindow(hWndMain);

	/* フォントを作成する */
	hFont = CreateFont(MulDiv(18, nDpi, 96),
					   0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE,
					   ANSI_CHARSET, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS,
					   DEFAULT_QUALITY,
					   DEFAULT_PITCH | FF_DONTCARE, L"Yu Gothic UI");
	hFontFixed = CreateFont(MulDiv(14, nDpi, 96),
							0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE,
							DEFAULT_CHARSET, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS,
							DEFAULT_QUALITY,
							FIXED_PITCH | FF_DONTCARE, L"BIZ UDゴシック");

	/* 続けるボタンを作成する */
	hWndBtnResume = CreateWindow(
		L"BUTTON",
		bEnglish ? L"Resume" : L"続ける",
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
		MulDiv(10, nDpi, 96),
		MulDiv(10, nDpi, 96),
		MulDiv(100, nDpi, 96),
		MulDiv(40, nDpi, 96),
		hWndDebug,
		(HMENU)ID_RESUME,
		(HINSTANCE)GetWindowLongPtr(hWndDebug, GWLP_HINSTANCE),
		NULL);
	SendMessage(hWndBtnResume, WM_SETFONT, (WPARAM)hFont, (LPARAM)TRUE);
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
		hWndDebug,
		(HMENU)ID_NEXT,
		(HINSTANCE)GetWindowLongPtr(hWndDebug, GWLP_HINSTANCE),
		NULL);
	SendMessage(hWndBtnNext, WM_SETFONT, (WPARAM)hFont, (LPARAM)TRUE);
	CreateTooltip(hWndBtnNext,
				  L"Run only one command and stop after it.",
				  L"コマンドを1個だけ実行し、停止します。");

	/* 停止ボタンを作成する */
	hWndBtnPause = CreateWindow(
		L"BUTTON",
		bEnglish ? L"Paused" : L"停止",
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
		MulDiv(330, nDpi, 96),
		MulDiv(10, nDpi, 96),
		MulDiv(100, nDpi, 96),
		MulDiv(40, nDpi, 96),
		hWndDebug,
		(HMENU)ID_PAUSE,
		(HINSTANCE)GetWindowLongPtr(hWndDebug, GWLP_HINSTANCE),
		NULL);
	EnableWindow(hWndBtnPause, FALSE);
	SendMessage(hWndBtnPause, WM_SETFONT, (WPARAM)hFont, (LPARAM)TRUE);
	CreateTooltip(hWndBtnPause,
				  L"Stop script execution.",
				  L"コマンドの実行を停止します。");

	/* スクリプト名のテキストボックスを作成する */
	hWndTextboxScript = CreateWindow(
		L"EDIT",
		NULL,
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | WS_BORDER | ES_READONLY | ES_AUTOHSCROLL,
		MulDiv(10, nDpi, 96),
		MulDiv(60, nDpi, 96),
		MulDiv(350, nDpi, 96),
		MulDiv(30, nDpi, 96),
		hWndDebug,
		0,
		(HINSTANCE)GetWindowLongPtr(hWndDebug, GWLP_HINSTANCE),
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
		hWndDebug,
		(HMENU)ID_OPEN,
		(HINSTANCE)GetWindowLongPtr(hWndDebug, GWLP_HINSTANCE),
		NULL);
	SendMessage(hWndBtnSelectScript, WM_SETFONT, (WPARAM)hFont, (LPARAM)TRUE);
	CreateTooltip(hWndBtnSelectScript,
				  L"Select a script file and jump to it.",
				  L"スクリプトファイルを選択してジャンプします。");

	/* スクリプトのリッチエディットを作成する */
	LoadLibrary(L"Msftedit.dll");
	bFirstChange = TRUE;
	hWndRichEdit = CreateWindowEx(
		0,
		MSFTEDIT_CLASS,
		L"Text",
		ES_MULTILINE | WS_VISIBLE | WS_CHILD | WS_BORDER | WS_TABSTOP | ES_AUTOVSCROLL,
		MulDiv(10, nDpi, 96),
		MulDiv(100, nDpi, 96),
		MulDiv(420, nDpi, 96),
		MulDiv(400, nDpi, 96),
		hWndDebug,
		(HMENU)ID_RICHEDIT,
		(HINSTANCE)GetWindowLongPtr(hWndDebug, GWLP_HINSTANCE),
		NULL);
	SendMessage(hWndRichEdit, EM_SHOWSCROLLBAR, (WPARAM)SB_VERT, (LPARAM)TRUE);
	SendMessage(hWndRichEdit, EM_SETEVENTMASK, 0, (LPARAM)ENM_CHANGE);
	SendMessage(hWndRichEdit, WM_SETFONT, (WPARAM)hFontFixed, (LPARAM)TRUE);

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
		hWndDebug,
		0,
		(HINSTANCE)GetWindowLongPtr(hWndDebug, GWLP_HINSTANCE),
		NULL);
	SendMessage(hWndTextboxVar, WM_SETFONT, (WPARAM)hFont, (LPARAM)TRUE);
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
		hWndDebug,
		(HMENU)ID_VARS,
		(HINSTANCE)GetWindowLongPtr(hWndDebug, GWLP_HINSTANCE),
		NULL);
	SendMessage(hWndBtnVar, WM_SETFONT, (WPARAM)hFont, (LPARAM)TRUE);
	CreateTooltip(hWndBtnVar,
				  L"Write to the variables.",
				  L"変数の内容を書き込みます。");

	/* メニューを作成する */
	InitMenu(hWndMain);

	return TRUE;
}

/*
  デバッガウィンドウの位置を修正する
*/
VOID RearrangeDebuggerPanel(int nGameWidth, int nGameHeight)
{
	int y, nDpi, nDebugWidth;

	/* DPIを求める */
	nDpi = Win11_GetDpiForWindow(hWndMain);

	/* デバッグパネルのサイズを求める*/
	nDebugWidth = MulDiv(DEBUGGER_PANEL_WIDTH, nDpi, 96);

	/* エディタのコントロールをサイズ変更する */
	MoveWindow(hWndRichEdit,
			   MulDiv(10, nDpi, 96),
			   MulDiv(100, nDpi, 96),
			   MulDiv(420, nDpi, 96),
			   nGameHeight - MulDiv(180, nDpi, 96),
			   TRUE);

	/* エディタより下のコントロールのY座標を計算する */
	y = nGameHeight - MulDiv(130, nDpi, 96);

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

	/* デバッグパネルの位置を変更する */
	MoveWindow(hWndDebug,
			   nGameWidth,
			   0,
			   nDebugWidth,
			   nGameHeight,
			   TRUE);
}

/* メニューを作成する */
static VOID InitMenu(HWND hWnd)
{
	HMENU hMenuFile = CreatePopupMenu();
	HMENU hMenuRun = CreatePopupMenu();
	HMENU hMenuDirection = CreatePopupMenu();
	HMENU hMenuExport = CreatePopupMenu();
	HMENU hMenuHelp = CreatePopupMenu();
    MENUITEMINFO mi;
	UINT nOrder;

	bEnglish = conf_locale == LOCALE_JA ? FALSE : TRUE;

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

	/* ヘルプ(H)を作成する */
	mi.hSubMenu = hMenuHelp;
	mi.dwTypeData = bEnglish ? L"Help(&H)": L"ヘルプ(&H)";
	InsertMenuItem(hMenu, nOrder++, TRUE, &mi);

	/* 2階層目を作成する準備を行う */
	mi.fMask = MIIM_TYPE | MIIM_ID;

	/* スクリプトを開く(Q)を作成する */
	nOrder = 0;
	mi.wID = ID_OPEN;
	mi.dwTypeData = bEnglish ?
		L"Open script(&Q)\tCtrl+O" :
		L"スクリプトを開く(&O)\tCtrl+O";
	InsertMenuItem(hMenuFile, nOrder++, TRUE, &mi);

	/* スクリプトを上書き保存する(S)を作成する */
	mi.wID = ID_SAVE;
	mi.dwTypeData = bEnglish ?
		L"Overwrite script(&S)\tCtrl+S" :
		L"スクリプトを上書き保存する(&S)\tCtrl+S";
	InsertMenuItem(hMenuFile, nOrder++, TRUE, &mi);

	/* 終了(Q)を作成する */
	mi.wID = ID_QUIT;
	mi.dwTypeData = bEnglish ? L"Quit(&Q)\tCtrl+Q" : L"終了(&Q)\tCtrl+Q";
	InsertMenuItem(hMenuFile, nOrder++, TRUE, &mi);

	/* 続ける(C)を作成する */
	nOrder = 0;
	mi.wID = ID_RESUME;
	mi.dwTypeData = bEnglish ? L"Continue(&R)\tCtrl+R" : L"続ける(&R)\tCtrl+R";
	InsertMenuItem(hMenuRun, nOrder++, TRUE, &mi);

	/* 次へ(N)を作成する */
	mi.wID = ID_NEXT;
	mi.dwTypeData = bEnglish ? L"Next(&N)\tCtrl+N" : L"次へ(&N)\tCtrl+N";
	InsertMenuItem(hMenuRun, nOrder++, TRUE, &mi);

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

	/* パッケージをエクスポートするを作成する */
	nOrder = 0;
	mi.wID = ID_EXPORT;
	mi.dwTypeData = bEnglish ?
		L"Export package(&X)" :
		L"パッケージをエクスポートする";
	InsertMenuItem(hMenuExport, nOrder++, TRUE, &mi);

	/* Windows向けにエクスポートするを作成する */
	mi.wID = ID_EXPORT_WIN;
	mi.dwTypeData = bEnglish ?
		L"Export for Windows" :
		L"Windows向けにエクスポートする";
	InsertMenuItem(hMenuExport, nOrder++, TRUE, &mi);

	/* Windows EXEインストーラを作成するを作成する */
	mi.wID = ID_EXPORT_WIN_INST;
	mi.dwTypeData = bEnglish ?
		L"Create EXE Installer for Windows" :
		L"Windows EXEインストーラを作成する";
	InsertMenuItem(hMenuExport, nOrder++, TRUE, &mi);

	/* Windows/Mac向けにエクスポートするを作成する */
	mi.wID = ID_EXPORT_WIN_MAC;
	mi.dwTypeData = bEnglish ?
		L"Export for Windows/Mac" :
		L"Windows/Mac向けにエクスポートする";
	InsertMenuItem(hMenuExport, nOrder++, TRUE, &mi);

	/* Web向けにエクスポートするを作成する */
	mi.wID = ID_EXPORT_WEB;
	mi.dwTypeData = bEnglish ?
		L"Export for Web" :
		L"Web向けにエクスポートする";
	InsertMenuItem(hMenuExport, nOrder++, TRUE, &mi);

	/* Androidプロジェクトをエクスポートするを作成する */
	mi.wID = ID_EXPORT_ANDROID;
	mi.dwTypeData = bEnglish ?
		L"Export Android project" :
		L"Androidプロジェクトをエクスポートする";
	InsertMenuItem(hMenuExport, nOrder++, TRUE, &mi);

	/* iOSプロジェクトをエクスポートするを作成する */
	mi.wID = ID_EXPORT_IOS;
	mi.dwTypeData = bEnglish ?
		L"Export iOS project" :
		L"iOSプロジェクトをエクスポートする";
	InsertMenuItem(hMenuExport, nOrder++, TRUE, &mi);

	/* 地の文を入力を作成する */
	nOrder = 0;
	mi.wID = ID_CMD_MESSAGE;
	mi.dwTypeData = bEnglish ? L"Message" : L"地の文を入力";
	InsertMenuItem(hMenuDirection, nOrder++, TRUE, &mi);

	/* セリフを入力を作成する */
	mi.wID = ID_CMD_SERIF;
	mi.dwTypeData = bEnglish ? L"Line" : L"セリフを入力";
	InsertMenuItem(hMenuDirection, nOrder++, TRUE, &mi);

	/* 背景を作成する */
	mi.wID = ID_CMD_BG;
	mi.dwTypeData = bEnglish ? L"Background" : L"背景";
	InsertMenuItem(hMenuDirection, nOrder++, TRUE, &mi);

	/* 背景だけ変更を作成する */
	mi.wID = ID_CMD_BG_ONLY;
	mi.dwTypeData = bEnglish ? L"Change Background Only" : L"背景だけ変更";
	InsertMenuItem(hMenuDirection, nOrder++, TRUE, &mi);

	/* キャラクタを作成する */
	mi.wID = ID_CMD_CH;
	mi.dwTypeData = bEnglish ? L"Character" : L"キャラクタ";
	InsertMenuItem(hMenuDirection, nOrder++, TRUE, &mi);

	/* キャラクタを同時に変更を作成する */
	mi.wID = ID_CMD_CHSX;
	mi.dwTypeData = bEnglish ? L"Change Multiple Characters" : L"キャラクタを同時に変更";
	InsertMenuItem(hMenuDirection, nOrder++, TRUE, &mi);

	/* 音楽を再生を作成する */
	mi.wID = ID_CMD_BGM;
	mi.dwTypeData = bEnglish ? L"Play BGM" : L"音楽を再生";
	InsertMenuItem(hMenuDirection, nOrder++, TRUE, &mi);

	/* 音楽を停止を作成する */
	mi.wID = ID_CMD_BGM_STOP;
	mi.dwTypeData = bEnglish ? L"Stop BGM" : L"音楽を停止";
	InsertMenuItem(hMenuDirection, nOrder++, TRUE, &mi);

	/* 音楽の音量を作成する */
	mi.wID = ID_CMD_VOL_BGM;
	mi.dwTypeData = bEnglish ? L"BGM Volume" : L"音楽の音量";
	InsertMenuItem(hMenuDirection, nOrder++, TRUE, &mi);

	/* 効果音を再生を作成する */
	mi.wID = ID_CMD_SE;
	mi.dwTypeData = bEnglish ? L"Play Sound Effect" : L"効果音を再生";
	InsertMenuItem(hMenuDirection, nOrder++, TRUE, &mi);

	/* 効果音を停止を作成する */
	mi.wID = ID_CMD_SE_STOP;
	mi.dwTypeData = bEnglish ? L"Stop Sound Effect" : L"効果音を停止";
	InsertMenuItem(hMenuDirection, nOrder++, TRUE, &mi);

	/* 効果音の音量を作成する */
	mi.wID = ID_CMD_VOL_SE;
	mi.dwTypeData = bEnglish ? L"Sound Effect Volume" : L"効果音の音量";
	InsertMenuItem(hMenuDirection, nOrder++, TRUE, &mi);

	/* 動画を再生する */
	mi.wID = ID_CMD_VIDEO;
	mi.dwTypeData = bEnglish ? L"Play Video" : L"動画を再生する";
	InsertMenuItem(hMenuDirection, nOrder++, TRUE, &mi);

	/* 画面を横に揺らすを作成する */
	mi.wID = ID_CMD_SHAKE_H;
	mi.dwTypeData = bEnglish ? L"Shake Screen Horizontally" : L"画面を横に揺らす";
	InsertMenuItem(hMenuDirection, nOrder++, TRUE, &mi);

	/* 画面を縦に揺らすを作成する */
	mi.wID = ID_CMD_SHAKE_V;
	mi.dwTypeData = bEnglish ? L"Shake Screen Vertically" : L"画面を縦に揺らす";
	InsertMenuItem(hMenuDirection, nOrder++, TRUE, &mi);

	/* 選択肢(3)を作成する */
	mi.wID = ID_CMD_CHOOSE_3;
	mi.dwTypeData = bEnglish ? L"Options (3)" : L"選択肢(3)";
	InsertMenuItem(hMenuDirection, nOrder++, TRUE, &mi);

	/* 選択肢(2)を作成する */
	mi.wID = ID_CMD_CHOOSE_2;
	mi.dwTypeData = bEnglish ? L"Options (2)" : L"選択肢(1)";
	InsertMenuItem(hMenuDirection, nOrder++, TRUE, &mi);

	/* 選択肢(1)を作成する */
	mi.wID = ID_CMD_CHOOSE_1;
	mi.dwTypeData = bEnglish ? L"Option (1)" : L"選択肢(1)";
	InsertMenuItem(hMenuDirection, nOrder++, TRUE, &mi);

	/* GUI呼び出しを作成する */
	mi.wID = ID_CMD_GUI;
	mi.dwTypeData = bEnglish ? L"Menu (GUI)" : L"メニュー (GUI)";
	InsertMenuItem(hMenuDirection, nOrder++, TRUE, &mi);

	/* クリック待ちを作成する */
	mi.wID = ID_CMD_CLICK;
	mi.dwTypeData = bEnglish ? L"Click Wait" : L"クリック待ち";
	InsertMenuItem(hMenuDirection, nOrder++, TRUE, &mi);

	/* 時間指定待ちを作成する */
	mi.wID = ID_CMD_WAIT;
	mi.dwTypeData = bEnglish ? L"Timed Wait" : L"時間指定待ち";
	InsertMenuItem(hMenuDirection, nOrder++, TRUE, &mi);

	/* 他のスクリプトへ移動を作成する */
	mi.wID = ID_CMD_LOAD;
	mi.dwTypeData = bEnglish ? L"Load Other Script" : L"他のスクリプトへ移動";
	InsertMenuItem(hMenuDirection, nOrder++, TRUE, &mi);

	/* バージョン(V)を作成する */
	nOrder = 0;
	mi.wID = ID_VERSION;
	mi.dwTypeData = bEnglish ? L"Version(&V)" : L"バージョン(&V)\tCtrl+V";
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
								  hWndDebug, NULL, GetModuleHandle(NULL),
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

/*
 * メッセージ処理
 */

/*
 * メッセージのトランスレート前処理を行う
 *  - return: メッセージが消費されたか
 *    - TRUEなら呼出元はメッセージをウィンドウプロシージャに送ってはならない
 *    - FALSEなら呼出元はメッセージをウィンドウプロシージャに送らなくてはいけない
 */
BOOL PretranslateForDebugger(MSG* pMsg)
{
	static BOOL bShiftDown;
	static BOOL bControlDown;
	int nStart, nEnd;

	bRangedChanged = FALSE;

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

	if (pMsg->message == WM_KEYUP)
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

	/* フォーカスを失うときにシフトとコントロールの */
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
			if (bShiftDown)
			{
				/* Shift+Returnは通常の改行としてモデルに反映する */
				bRangedChanged = TRUE;
			}
			else
			{
				/* 実行位置/実行状態の制御を行う */
				RichEdit_OnReturn();

				/* このメッセージをリッチエディットにディスパッチしない */
				return TRUE;
			}
			break;
		case VK_DELETE:
			if (RichEdit_IsLineEnd())
			{
				bRangedChanged = TRUE;
			}
			else
			{
				/* 範囲選択に対する削除の場合 */
				RichEdit_GetSelectedRange(&nStart, &nEnd);
				if (nStart != nEnd)
					bRangedChanged = TRUE;
			}
			break;
		case VK_BACK:
			if (RichEdit_IsLineTop())
			{
				bRangedChanged = TRUE;
			}
			else
			{
				/* 範囲選択に対する削除の場合 */
				RichEdit_GetSelectedRange(&nStart, &nEnd);
				if (nStart != nEnd)
					bRangedChanged = TRUE;
			}
			break;
		case 'X':
			/* Ctrl+Xを処理する */
			if (bControlDown)
				bRangedChanged = TRUE;
			break;
		case 'V':
			/* Ctrl+Vを処理する */
			if (bControlDown)
				bRangedChanged = TRUE;
			break;
		/*
		 * メニュー
		 */
		case 'O':
			/* Ctrl+Nを処理する */
			if (bControlDown)
			{
				pMsg->hwnd = hWndMain;
				pMsg->message = WM_COMMAND;
				pMsg->wParam = ID_OPEN;
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
			/* Ctrl+Sを処理する */
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
			/* 範囲選択に対する置き換えの場合 */
			RichEdit_GetSelectedRange(&nStart, &nEnd);
			if (nStart != nEnd)
				bRangedChanged = TRUE;
			break;
		}
		return FALSE;
	}

	/* このメッセージは引き続きリッチエディットで処理する */
	return FALSE;
}

/*
 * デバッガへのWM_COMMANDを処理する
 */
VOID OnCommandForDebugger(WPARAM wParam, UNUSED(LPARAM lParam))
{
	UINT nID;
	UINT nNotify;

	nID = LOWORD(wParam);
	nNotify = (WORD)(wParam >> 16) & 0xFFFF;

	/* EN_CHANGEを確認する */
	if (nID == ID_RICHEDIT && nNotify == EN_CHANGE)
	{
		RichEdit_OnChange();
		return;
	}

	/* その他の通知を処理する */
	switch(nID)
	{
	/* ファイル */
	case ID_OPEN:
		OnOpenScript();
		break;
	case ID_SAVE:
		OnSave();
		break;
	case ID_QUIT:
		DestroyWindow(hWndMain);
		break;
	/* スクリプト実行 */
	case ID_RESUME:
		bContinuePressed = TRUE;
		break;
	case ID_NEXT:
		bNextPressed = TRUE;
		break;
	case ID_PAUSE:
		bStopPressed = TRUE;
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
	case ID_EXPORT:
		OnExportPackage();
		break;
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
	/* ヘルプ */
	case ID_VERSION:
		MessageBox(hWndMain, bEnglish ? VERSION_EN : VERSION_JP,
				   MSGBOX_TITLE, MB_OK | MB_ICONINFORMATION);
		break;
	/* ボタン */
	case ID_VARS:
		OnWriteVars();
		break;
	default:
		break;
	}
}

/*
 * コマンド処理
 */

/* スクリプトオープン */
static VOID OnOpenScript(void)
{
	const wchar_t *pFile;

	pFile = SelectFile(SCRIPT_DIR);
	if (pFile == NULL)
		return;

	SetWindowText(hWndTextboxScript, pFile);
	bScriptOpened = TRUE;
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
				   MSGBOX_TITLE, MB_OK | MB_ICONERROR);
	}
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
			   MSGBOX_TITLE, MB_ICONINFORMATION | MB_OK);
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
				   MSGBOX_TITLE, MB_ICONWARNING | MB_OKCANCEL) != IDOK)
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
				   MSGBOX_TITLE, MB_ICONWARNING | MB_OKCANCEL) != IDOK)
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
	if (!CopySourceFiles(L".\\suika.exe", L".\\windows-export\\suika.exe"))
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
			   MSGBOX_TITLE, MB_ICONINFORMATION | MB_OK);

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
				   MSGBOX_TITLE, MB_ICONWARNING | MB_OKCANCEL) != IDOK)
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
	if (!CopySourceFiles(L".\\suika.exe", L".\\windows-installer-export\\asset\\suika.exe"))
	{
		log_info(bEnglish ?
				 "Failed to copy exe file." :
				 "実行ファイルのコピーに失敗しました。");
		return;
	}

	/* ファイルをコピーする */
	if (!CopySourceFiles(L".\\tools\\installer\\create-installer.bat", L".\\windows-installer-export\\asset\\create-installer.bat"))
	{
		log_info(bEnglish ?
				 "Failed to copy exe file." :
				 "実行ファイルのコピーに失敗しました。");
		return;
	}

	/* ファイルをコピーする */
	if (!CopySourceFiles(L".\\tools\\installer\\icon.ico", L".\\windows-installer-export\\asset\\icon.ico"))
	{
		log_info(bEnglish ?
				 "Failed to copy exe file." :
				 "実行ファイルのコピーに失敗しました。");
		return;
	}

	/* ファイルをコピーする */
	if (!CopySourceFiles(L".\\tools\\installer\\install-script.nsi", L".\\windows-installer-export\\asset\\install-script.nsi"))
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
				   MSGBOX_TITLE, MB_ICONWARNING | MB_OKCANCEL) != IDOK)
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
	if (!CopySourceFiles(L".\\suika.exe", L".\\windows-mac-export\\suika.exe"))
	{
		log_info(bEnglish ?
				 "Failed to copy exe file." :
				 "実行ファイルのコピーに失敗しました。");
		return;
	}

	/* ファイルをコピーする */
	if (!CopySourceFiles(L".\\mac.dmg", L".\\windows-mac-export\\mac.dmg"))
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
			   MSGBOX_TITLE, MB_ICONINFORMATION | MB_OK);

	/* Explorerを開く */
	ShellExecuteW(NULL, L"explore", L".\\windows-mac-export", NULL, NULL, SW_SHOW);
}

/* Web向けにエクスポートのメニューが押下されたときの処理を行う */
VOID OnExportWeb(void)
{
	if (MessageBox(hWndMain, bEnglish ?
				   L"Takes a while. Are you sure?\n" :
				   L"エクスポートには時間がかかります。よろしいですか？",
				   MSGBOX_TITLE, MB_ICONWARNING | MB_OKCANCEL) != IDOK)
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
	if (!CopySourceFiles(L".\\tools\\web\\*", L".\\web-export"))
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
			   MSGBOX_TITLE, MB_ICONINFORMATION | MB_OK);

	/* Explorerを開く */
	ShellExecuteW(NULL, L"explore", L".\\web-export", NULL, NULL, SW_SHOW);
}

/* Androidプロジェクトをエクスポートのメニューが押下されたときの処理を行う */
VOID OnExportAndroid(void)
{
	if (MessageBox(hWndMain, bEnglish ?
				   L"Takes a while. Are you sure?\n" :
				   L"エクスポートには時間がかかります。よろしいですか？",
				   MSGBOX_TITLE, MB_ICONWARNING | MB_OKCANCEL) != IDOK)
		return;

	/* フォルダを再作成する */
	RecreateDirectory(L".\\android-export");

	/* ソースをコピーする */
	if (!CopySourceFiles(L".\\tools\\android-src", L".\\android-export"))
	{
		log_info(bEnglish ?
				 "Failed to copy source files for Android." :
				 "ソースコードのコピーに失敗しました。"
				 "最新のtools/android-srcフォルダが存在するか確認してください。");
		return;
	}

	/* アセットをコピーする */
	CopySourceFiles(L".\\anime", L".\\android-export\\app\\src\\main\\assets\\anime");
	CopySourceFiles(L".\\bg", L".\\android-export\\app\\src\\main\\assets\\bg");
	CopySourceFiles(L".\\bgm", L".\\android-export\\app\\src\\main\\assets\\bgm");
	CopySourceFiles(L".\\cg", L".\\android-export\\app\\src\\main\\assets\\cg");
	CopySourceFiles(L".\\ch", L".\\android-export\\app\\src\\main\\assets\\ch");
	CopySourceFiles(L".\\conf", L".\\android-export\\app\\src\\main\\assets\\conf");
	CopySourceFiles(L".\\cv", L".\\android-export\\app\\src\\main\\assets\\cv");
	CopySourceFiles(L".\\font", L".\\android-export\\app\\src\\main\\assets\\font");
	CopySourceFiles(L".\\gui", L".\\android-export\\app\\src\\main\\assets\\gui");
	CopySourceFiles(L".\\mov", L".\\android-export\\app\\src\\main\\assets\\mov");
	CopySourceFiles(L".\\rule", L".\\android-export\\app\\src\\main\\assets\\rule");
	CopySourceFiles(L".\\se", L".\\android-export\\app\\src\\main\\assets\\se");
	CopySourceFiles(L".\\txt", L".\\android-export\\app\\src\\main\\assets\\txt");
	CopySourceFiles(L".\\wms", L".\\android-export\\app\\src\\main\\assets\\wms");

	MessageBox(hWndMain, bEnglish ?
			   L"Will open the exported source code folder.\n"
			   L"Build with Android Studio." :
			   L"エクスポートしたソースコードフォルダを開きます。\n"
			   L"Android Studioでそのままビルドできます。",
			   MSGBOX_TITLE, MB_ICONINFORMATION | MB_OK);

	/* Explorerを開く */
	ShellExecuteW(NULL, L"explore", L".\\android-export", NULL, NULL, SW_SHOW);
}

/* iOSプロジェクトをエクスポートのメニューが押下されたときの処理を行う */
VOID OnExportIOS(void)
{
	if (MessageBox(hWndMain, bEnglish ?
				   L"Takes a while. Are you sure?\n" :
				   L"エクスポートには時間がかかります。よろしいですか？",
				   MSGBOX_TITLE, MB_ICONWARNING | MB_OKCANCEL) != IDOK)
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
	if (!CopySourceFiles(L".\\tools\\ios-src", L".\\ios-export"))
	{
		log_info(bEnglish ?
				 "Failed to copy source files for Android." :
				 "ソースコードのコピーに失敗しました。"
				 "最新のtools/ios-srcフォルダが存在するか確認してください。");
		return;
	}

	/* movをコピーする */
	CopyMovFiles(L".\\mov", L".\\ios-export\\suika\\mov");

	/* パッケージを移動する */
	if (!MovePackageFile(L".\\data01.arc", L".\\ios-export\\suika\\data01.arc"))
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
			   MSGBOX_TITLE, MB_ICONINFORMATION | MB_OK);

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

/* ソースツリーをコピーする */
static BOOL CopySourceFiles(const wchar_t *lpszSrcDir, const wchar_t *lpszDestDir)
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
		log_info("error code = %d", ret);
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
	SHFileOperationW(&fos);

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
 * platform.h
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
 * 旧Suika2 Pro用: VLSでは使用しない TODO: 削除
 */
bool is_command_updated(void) { return false; }
const char* get_updated_command(void) { assert(0); return NULL; }
bool is_script_reloaded(void) { 	return false; }

/*
 * コマンドの実行中状態を設定する
 */
void on_change_running_state(bool running, bool request_stop)
{
	bRunning = running;

	if(request_stop)
	{
		/*
		 * 停止によりコマンドの完了を待機中のとき
		 *  - コントロールとメニューアイテムを無効にする
		 */
		EnableWindow(hWndBtnResume, FALSE);
		EnableWindow(hWndBtnNext, FALSE);
		EnableWindow(hWndBtnPause, FALSE);
		EnableWindow(hWndTextboxScript, FALSE);
		EnableWindow(hWndBtnSelectScript, FALSE);
		SendMessage(hWndRichEdit, EM_SETREADONLY, TRUE, 0);
		SendMessage(hWndTextboxVar, EM_SETREADONLY, TRUE, 0);
		EnableWindow(hWndBtnVar, FALSE);
		EnableMenuItem(hMenu, ID_OPEN, MF_GRAYED);
		EnableMenuItem(hMenu, ID_SAVE, MF_GRAYED);
		EnableMenuItem(hMenu, ID_EXPORT, MF_GRAYED);
		EnableMenuItem(hMenu, ID_RESUME, MF_GRAYED);
		EnableMenuItem(hMenu, ID_NEXT, MF_GRAYED);
		EnableMenuItem(hMenu, ID_PAUSE, MF_GRAYED);
		EnableMenuItem(hMenu, ID_ERROR, MF_GRAYED);

		/* 実行中の背景色を設定する */
		RichEdit_SetBackgroundColorForCurrentExecuteLine();
	}
	else if(running)
	{
		/*
		 * 実行中のとき
		 *  - 「停止」だけ有効、他は無効にする
		 */
		EnableWindow(hWndBtnResume, FALSE);
		EnableWindow(hWndBtnNext, FALSE);
		EnableWindow(hWndBtnPause, TRUE);					/* 有効 */
		EnableWindow(hWndTextboxScript, FALSE);
		EnableWindow(hWndBtnSelectScript, FALSE);
		SendMessage(hWndRichEdit, EM_SETREADONLY, TRUE, 0);
		SendMessage(hWndTextboxVar, EM_SETREADONLY, TRUE, 0);
		EnableWindow(hWndBtnVar, FALSE);
		EnableMenuItem(hMenu, ID_OPEN, MF_GRAYED);
		EnableMenuItem(hMenu, ID_SAVE, MF_GRAYED);
		EnableMenuItem(hMenu, ID_EXPORT, MF_GRAYED);
		EnableMenuItem(hMenu, ID_RESUME, MF_GRAYED);
		EnableMenuItem(hMenu, ID_NEXT, MF_GRAYED);
		EnableMenuItem(hMenu, ID_PAUSE, MF_ENABLED);		/* 有効 */
		EnableMenuItem(hMenu, ID_ERROR, MF_GRAYED);

		/* 実行中の背景色を設定する */
		RichEdit_SetBackgroundColorForCurrentExecuteLine();
	}
	else
	{
		/*
		 * 完全に停止中のとき
		 *  - 「停止」だけ無効、他は有効にする
		 */
		EnableWindow(hWndBtnResume, TRUE);
		EnableWindow(hWndBtnNext, TRUE);
		EnableWindow(hWndBtnPause, FALSE);					/* 無効 */
		EnableWindow(hWndTextboxScript, TRUE);
		EnableWindow(hWndBtnSelectScript, TRUE);
		SendMessage(hWndRichEdit, EM_SETREADONLY, FALSE, 0);
		SendMessage(hWndTextboxVar, EM_SETREADONLY, FALSE, 0);
		EnableWindow(hWndBtnVar, TRUE);
		EnableMenuItem(hMenu, ID_OPEN, MF_ENABLED);
		EnableMenuItem(hMenu, ID_SAVE, MF_ENABLED);
		EnableMenuItem(hMenu, ID_EXPORT, MF_ENABLED);
		EnableMenuItem(hMenu, ID_RESUME, MF_ENABLED);
		EnableMenuItem(hMenu, ID_NEXT, MF_ENABLED);
		EnableMenuItem(hMenu, ID_PAUSE, MF_GRAYED);		/* 無効 */
		EnableMenuItem(hMenu, ID_ERROR, MF_ENABLED);

		/* 次の実行される行の背景色を設定する */
		RichEdit_SetBackgroundColorForNextExecuteLine();
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
	EnableWindow(hWndRichEdit, FALSE);
	RichEdit_UpdateTextFromScriptModel();
	RichEdit_SetFont();
	RichEdit_SetTextColorForAllLines();
	EnableWindow(hWndRichEdit, TRUE);
}

/*
 * 実行位置が変更されたときのコールバック
 */
void on_change_position(void)
{
	/* 実行行のハイライトを行う */
	if (!bRunning)
		RichEdit_SetBackgroundColorForNextExecuteLine();
	else
		RichEdit_SetBackgroundColorForCurrentExecuteLine();

	/* スクロールする */
	RichEdit_AutoScroll();
}

/*
 * 変数が変更されたときのコールバック
 */
void on_update_variable(void)
{
	/* 変数の情報を更新する */
	Variable_UpdateText();
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
 * リッチエディットのハンドラ
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

	/* スクリプトモデルの変更後、最初の通知のとき */
	if (bFirstChange)
	{
		bFirstChange = FALSE;

		/* フォントを設定する */
		RichEdit_SetFont();

		/* ハイライトを更新する */
		RichEdit_SetTextColorForAllLines();
	}
	/* 複数行にまたがる可能性のある変更の通知のとき */
	else if (bRangedChanged)
	{
		bRangedChanged = FALSE;

		/* スクリプトモデルを作り直す */
		RichEdit_UpdateScriptModelFromText();

		/* フォントを設定する */
		RichEdit_SetFont();

		/* ハイライトを更新する */
		//RichEdit_SetTextColorForAllLines();
	}
	/* 単一行内での変更の通知のとき */
	else
	{
		/* 現在の行のテキスト色を変更する */
		RichEdit_SetTextColorForCursorLine();
	}

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
}

/* リッチエディットでのReturnキー押下を処理する */
static VOID RichEdit_OnReturn(void)
{
	int nCursorLine;

	/* リッチエディットのカーソル行番号を取得する */
	nCursorLine = RichEdit_GetCursorLine();
	if (nCursorLine == -1)
		return;

	/* 次フレームでの実行行の移動の問い合わせに真を返すようにしておく */
	if (nCursorLine != get_expanded_line_num())
	{
		nLineChanged = nCursorLine;
		bExecLineChanged = TRUE;
	}

	/* スクリプトモデルを更新する */
	RichEdit_UpdateScriptModelFromCurrentLineText();

	/* 次フレームでの一行実行の問い合わせに真を返すようにしておく */
	if (dbg_get_parse_error_count() == 0)
		bNextPressed = TRUE;
}

/*
 * リッチエディットを操作するヘルパー
 */

/* リッチエディットのフォントを設定する */
static VOID RichEdit_SetFont(void)
{
	CHARFORMAT2W cf;

	memset(&cf, 0, sizeof(cf));
	cf.cbSize = sizeof(cf);
	cf.dwMask = CFM_FACE;
	wcscpy(&cf.szFaceName[0], L"BIZ UDゴシック");
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

/* リッチエディットのカーソル位置を設定する */
static VOID RichEdit_SetCursorPosition(int nCursor)
{
	CHARRANGE cr;

	/* カーソル位置を取得する */
	cr.cpMin = nCursor;
	cr.cpMax = nCursor;
	SendMessage(hWndRichEdit, EM_EXSETSEL, 0, (LPARAM)&cr);
}

/* リッチエディットの範囲選択範囲を求める */
static VOID RichEdit_GetSelectedRange(int *nStart, int *nEnd)
{
	CHARRANGE cr;
	memset(&cr, 0, sizeof(cr));
	SendMessage(hWndRichEdit, EM_EXGETSEL, 0, (LPARAM)&cr);
	*nStart = cr.cpMin;
	*nEnd = cr.cpMax;
}

/* リッチエディットの範囲を選択する */
static VOID RichEdit_SetSelectedRange(int nLineStart, int nLineLen)
{
	CHARRANGE cr;

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

/* 現在のカーソル行のテキスト色を設定する */
static VOID RichEdit_SetTextColorForCursorLine(void)
{
	wchar_t *pText, *pLineStop;
	int i, nCursor, nLineStartCRLF, nLineStartCR, nLineLen;

	pText = RichEdit_GetText();
	nCursor = RichEdit_GetCursorPosition();
	nLineStartCRLF = 0;		/* WM_GETTEXTは改行をCRLFで返す */
	nLineStartCR = 0;		/* EM_EXSETSELでは改行はCRの1文字 */
	for (i = 0; i < get_line_count(); i++)
	{
		/* 行の終了位置を求める */
		pLineStop = wcswcs(pText + nLineStartCRLF, L"\r\n");
		nLineLen = pLineStop != NULL ?
			(int)(pLineStop - (pText + nLineStartCRLF)) :
			(int)wcslen(pText + nLineStartCRLF);

		if (nCursor >= nLineStartCR && nCursor <= nLineStartCR + nLineLen)
		{
			/* 行の色付けを行う */
			RichEdit_SetTextColorForLine(pText, nLineStartCR, nLineStartCRLF, nLineLen);
			break;
		}

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
	const wchar_t *pCommandStop, *pParamStart, *pParamStop, *pParamSpace;
	int nParamLen, nCommandType;

	/* 行を選択して選択範囲のテキスト色をデフォルトに変更する */
	RichEdit_SetSelectedRange(nLineStartCR, nLineLen);
	RichEdit_SetTextColorForSelectedRange(COLOR_FG_DEFAULT);

	/* コメントを処理する */
	if (pText[nLineStartCRLF] == L'#')
	{
		/* 行全体を選択して、選択範囲のテキスト色を変更する */
		RichEdit_SetSelectedRange(nLineStartCR, nLineLen);
		RichEdit_SetTextColorForSelectedRange(COLOR_COMMENT);
	}
	/* ラベルを処理する */
	else if (pText[nLineStartCRLF] == L':')
	{
		/* 行全体を選択して、選択範囲のテキスト色を変更する */
		RichEdit_SetSelectedRange(nLineStartCR, nLineLen);
		RichEdit_SetTextColorForSelectedRange(COLOR_LABEL);
	}
	/* エラー行を処理する */
	if (pText[nLineStartCRLF] == L'!')
	{
		/* 行全体を選択して、選択範囲のテキスト色を変更する */
		RichEdit_SetSelectedRange(nLineStartCR, nLineLen);
		RichEdit_SetTextColorForSelectedRange(COLOR_ERROR);
	}
	/* コマンド行を処理する */
	else if (pText[nLineStartCRLF] == L'@')
	{
		/* コマンド名部分を抽出する */
		pCommandStop = wcswcs(pText + nLineStartCRLF, L" ");
		nParamLen = pCommandStop != NULL ?
			(int)(pCommandStop - (pText + nLineStartCRLF)) :
			(int)wcslen(pText + nLineStartCRLF);
		wcsncpy(wszCommandName, &pText[nLineStartCRLF],
				(size_t)nParamLen < sizeof(wszCommandName) / sizeof(wchar_t) ?
				(size_t)nParamLen :
				sizeof(wszCommandName) / sizeof(wchar_t));
		nCommandType = get_command_type_from_name(conv_utf16_to_utf8(wszCommandName));
		if (nCommandType != COMMAND_SET &&
			nCommandType != COMMAND_IF &&
			nCommandType != COMMAND_UNLESS &&
			nCommandType != COMMAND_PENCIL)
		{
			/* コマンド名のテキストに色を付ける */
			RichEdit_SetSelectedRange(nLineStartCR, nParamLen);
			RichEdit_SetTextColorForSelectedRange(COLOR_COMMAND_NAME);

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
				if (pParamSpace < pParamStop)
					continue;

				/* 引数名部分を選択してテキスト色を変更する */
				nNameStart = nLineStartCR + (pParamStart - (pText + nLineStartCRLF));
				nNameLen = pParamStop - pParamStart + 1;
				RichEdit_SetSelectedRange(nNameStart, nNameLen);
				RichEdit_SetTextColorForSelectedRange(COLOR_PARAM_NAME);
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
	RichEdit_SetBackgroundColorForSelectedRange(COLOR_NEXT_EXEC);

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
	RichEdit_SetBackgroundColorForSelectedRange(COLOR_CURRENT_EXEC);

	/* カーソル位置を実行行の先頭に設定する */
	RichEdit_SetCursorPosition(nLineStart);
}

/* リッチエディットのテキスト全体の背景色をクリアする */
static VOID RichEdit_ClearBackgroundColorAll(void)
{
	CHARFORMAT2W cf;

	memset(&cf, 0, sizeof(cf));
	cf.cbSize = sizeof(cf);
	cf.dwMask = CFM_BACKCOLOR;
	cf.crBackColor = COLOR_BG_DEFAULT;
	SendMessage(hWndRichEdit, EM_SETCHARFORMAT, (WPARAM)SCF_ALL, (LPARAM)&cf);
}

/* リッチエディットの選択範囲のテキスト色を変更する */
static VOID RichEdit_SetTextColorForSelectedRange(COLORREF cl)
{
	CHARFORMAT2W cf;

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
	SendMessage(hWndRichEdit, EM_SCROLLCARET, 0, 0);

	/* リッチエディットを再描画する */
	InvalidateRect(hWndRichEdit, NULL, TRUE);
}

/* カーソルが行頭にあるか調べる */
static BOOL RichEdit_IsLineTop(void)
{
	wchar_t *pWcs, *pCRLF;
	int i, nCursor, nLineStartCharCR, nLineStartCharCRLF;

	pWcs = RichEdit_GetText();
	nCursor = RichEdit_GetCursorPosition();
	nLineStartCharCR = 0;
	nLineStartCharCRLF = 0;
	for (i = 0; i < get_line_count(); i++)
	{
		pCRLF = wcswcs(pWcs + nLineStartCharCRLF, L"\r\n");
		int nLen = (pCRLF != NULL) ?
			(int)(pCRLF - (pWcs + nLineStartCharCRLF)) :
			(int)wcslen(pWcs + nLineStartCharCRLF);
		if (nCursor >= nLineStartCharCR && nCursor <= nLineStartCharCR + nLen)
		{
			free(pWcs);
			if (nCursor == nLineStartCharCR)
				return TRUE;
			return FALSE;
		}
		nLineStartCharCRLF += nLen + 2; /* +2 for CRLF */
		nLineStartCharCR += nLen + 1; /* +1 for CR */
	}
	free(pWcs);

	return FALSE;
}

/* カーソルが行末にあるか調べる */
static BOOL RichEdit_IsLineEnd(void)
{
	wchar_t *pWcs, *pCRLF;
	int i, nCursor, nLineStartCharCR, nLineStartCharCRLF;

	pWcs = RichEdit_GetText();
	nCursor = RichEdit_GetCursorPosition();
	nLineStartCharCR = 0;
	nLineStartCharCRLF = 0;
	for (i = 0; i < get_line_count(); i++)
	{
		pCRLF = wcswcs(pWcs + nLineStartCharCRLF, L"\r\n");
		int nLen = (pCRLF != NULL) ?
			(int)(pCRLF - (pWcs + nLineStartCharCRLF)) :
			(int)wcslen(pWcs + nLineStartCharCRLF);
		if (nCursor >= nLineStartCharCR && nCursor <= nLineStartCharCR + nLen)
		{
			free(pWcs);
			if (nCursor == nLineStartCharCR + nLen)
				return TRUE;
			return FALSE;
		}
		nLineStartCharCRLF += nLen + 2; /* +2 for CRLF */
		nLineStartCharCR += nLen + 1; /* +1 for CR */
	}
	free(pWcs);

	return FALSE;
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

/*
 * リッチエディットとスクリプトモデルの対応付け
 */

/* リッチエディットのテキストをスクリプトモデルを元に設定する */
static VOID RichEdit_UpdateTextFromScriptModel(void)
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
	bFirstChange = TRUE;
	SetWindowText(hWndRichEdit, pWcs);

	/* メモリを解放する */
	free(pWcs);

	/* 複数行の変更があったことを記録する */
	bRangedChanged = TRUE;
}

/* リッチエディットの内容を元にスクリプトモデルを更新する */
static VOID RichEdit_UpdateScriptModelFromText(void)
{
	wchar_t *pWcs, *pCRLF;
	int i, nTotal, nLine, nLineStartCharCR, nLineStartCharCRLF;
	BOOL bExecLineChanged;

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
		update_script_line(nLine, conv_utf16_to_utf8(pLine));

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
	if (bExecLineChanged)
		RichEdit_SetBackgroundColorForNextExecuteLine();

	/* コマンドのパースに失敗した場合 */
	if (dbg_get_parse_error_count() > 0)
	{
		/* 行頭の'!'を反映するためにテキストを再設定する */
		RichEdit_UpdateTextFromScriptModel();
		RichEdit_SetTextColorForAllLines();
	}
}

/* リッチエディットの現在の内容を元にスクリプトモデルを更新する */
static VOID RichEdit_UpdateScriptModelFromCurrentLineText(void)
{
	wchar_t *pWcs, *pCRLF, *pLine;
	int nCursorLine, nTotal, nLine, nLineStartCharCR, nLineStartCharCRLF, nLen;

	/* パースエラーをリセットして、最初のパースエラーで通知を行う */
	dbg_reset_parse_error_count();

	/* カーソル行を取得する */
	nCursorLine = RichEdit_GetCursorLine();

	/* リッチエディットのテキストの内容でスクリプトの各行をアップデートする */
	pWcs = RichEdit_GetText();
	nTotal = (int)wcslen(pWcs);
	nLine = 0;
	nLineStartCharCR = 0;
	nLineStartCharCRLF = 0;
	while (nLineStartCharCRLF < nTotal)
	{
		/* 行を切り出す */
		pLine = pWcs + nLineStartCharCRLF;
		pCRLF = wcswcs(pLine, L"\r\n");
		nLen = (pCRLF != NULL) ?
			(int)(pCRLF - (pWcs + nLineStartCharCRLF)) :
			(int)wcslen(pWcs + nLineStartCharCRLF);
		if (pCRLF != NULL)
			*pCRLF = L'\0';

		/* 行を更新する */
		if (nLine == nCursorLine)
		{
			update_script_line(nLine, conv_utf16_to_utf8(pLine));
			break;
		}

		nLineStartCharCRLF += nLen + 2; /* +2 for CRLF */
		nLineStartCharCR += nLen + 1; /* +1 for CR */
		nLine++;
	}
	free(pWcs);

	/* コマンドのパースに失敗した場合 */
	if (dbg_get_parse_error_count() > 0)
	{
		/* 行頭の'!'を反映するためにテキストを再設定する */
		RichEdit_UpdateTextFromScriptModel();
		RichEdit_SetTextColorForAllLines();
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
	RichEdit_SetTextColorForSelectedRange(COLOR_FG_DEFAULT);
	SendMessage(hWndRichEdit, EM_REPLACESEL, (WPARAM)TRUE, (LPARAM)buf);

	/* 行を選択する */
	RichEdit_SetCursorPosition(nLineStart);

	/* 次のフレームで実行位置を変更する */
	nLineChanged = nLine;
	bExecLineChanged = TRUE;
}

/*
 * Suika2コマンドの挿入
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
		RichEdit_InsertText(L"@chsx bg=%ls duration=1.0 left=stay center=stay right=stay back=stay", pFile);
	else
		RichEdit_InsertText(L"@場面転換X 背景=%ls 秒=1.0 左=stay 中央=stay 右=stay 背面=stay", pFile);
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
