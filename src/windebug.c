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
#include <Richedit.h>
#include "resource.h"

/* Standard C (msvcrt.dll) */
#include <stdlib.h>	/* exit() */

/*
 * Constants
 */

/* UTF-8からSJISへの変換バッファサイズ */
#define NATIVE_MESSAGE_SIZE	(65536)

/* 変数テキストボックスのテキストの最大長(形: "$00001=12345678901\r\n") */
#define VAR_TEXTBOX_MAX		(11000 * (1 + 5 + 1 + 11 + 2))

/* ウィンドウクラス名 */
const wchar_t wszWindowClass[] = L"SuikaDebugPanel";

/*
 * Variables
 */

/* メインウィンドウのハンドル */
static HWND hWndMain;

/* ゲーム領域のハンドル */
static HWND hWndGame;

/* 英語モードか */
static BOOL bEnglish;

/* 実行状態 */
static BOOL bRunning;

/* メニュー */
static HMENU hMenu;

/* デバッガウィンドウ */
static HWND hWndDebug;

/* デバッガウィンドウのコントロール */
static HWND hWndBtnResume;
static HWND hWndBtnNext;
static HWND hWndBtnPause;
//static HWND hWndLabelScript;
static HWND hWndTextboxScript;
//static HWND hWndBtnChangeScript;
static HWND hWndBtnSelectScript;
//static HWND hWndLabelLine;
//static HWND hWndTextboxLine;
//static HWND hWndBtnChangeLine;
//static HWND hWndLabelCommand;
//static HWND hWndTextboxCommand;
//static HWND hWndBtnUpdate;
//static HWND hWndBtnReset;
//static HWND hWndLabelContent;
//static HWND hWndListbox;
//static HWND hWndBtnError;
//static HWND hWndBtnSave;
//static HWND hWndBtnReload;
//static HWND hWndLabelVar;
static HWND hWndRichEdit;
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
static wchar_t szTextboxVar[VAR_TEXTBOX_MAX + 1];

/*
 * Forward Declaration
 */

static VOID InitDebuggerMenu(HWND hWnd);
static HWND CreateTooltip(HWND hWndBtn, const wchar_t *pszTextEnglish, const wchar_t *pszTextJapanese);
//static VOID OnClickListBox(void);
static VOID OnSelectScript(void);
//static VOID OnPressReset(void);
static VOID OnPressSave(void);
static VOID OnPressError(void);
static VOID OnPressWriteVars(void);
static VOID OnExportPackage(void);
static VOID OnExportWin(void);
static VOID OnExportWinInst(void);
static VOID OnExportWinMac(void);
static VOID OnExportWeb(void);
static VOID OnExportAndroid(void);
static VOID OnExportIOS(void);
static VOID RecreateDirectory(const wchar_t *path);
static BOOL CopySourceFiles(const wchar_t *lpszSrcDir, const wchar_t *lpszDestDir);
static BOOL CopyMovFiles(const wchar_t *lpszSrcDir, const wchar_t *lpszDestDir);
static BOOL MovePackageFile(const wchar_t *lpszPkgFile, wchar_t *lpszDestDir);
static VOID UpdateVariableTextBox(void);
static VOID UpdateLineHighlight(void);
static VOID OnInsertLine(void);
static VOID OnReturn(void);

/*
 * 引数が指定された場合はパッケージャとして機能する
 *  - 単体機能のpackage.exeはセキュリティソフトに誤認識されるため
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
 * デバッガパネルを作成する
 */
BOOL InitDebuggerPanel(HWND hMainWnd, HWND hGameWnd, void *pWndProc)
{
	WNDCLASSEX wcex;
	RECT rcClient;
	HFONT hFont;
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
								  rcClient.right - DEBUGGER_WIDTH,
								  0,
								  DEBUGGER_WIDTH,
								  rcClient.bottom,
								  hWndMain,
								  NULL,
								  GetModuleHandle(NULL),
								  NULL);
	if(!hWndDebug)
		return FALSE;

	/* DPIを取得する */
	nDpi = GetDpiForWindow(hWndMain);

	/* フォントを作成する */
	hFont = CreateFont(MulDiv(18, nDpi, 96),
						 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE,
						 ANSI_CHARSET, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS,
						 DEFAULT_QUALITY,
						 DEFAULT_PITCH | FF_DONTCARE, L"Yu Gothic UI");
#if 0
	hFontFixed = CreateFont(16, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE,
							   DEFAULT_CHARSET, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS,
							   PROOF_QUALITY,
							   DEFAULT_PITCH | FF_DONTCARE, L"BIZ UDGothic");
#endif

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
		bEnglish ? L"(Paused)" : L"(停止中)",
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

#if 0
	/* スクリプトラベルを作成する */
	hWndLabelScript = CreateWindow(
		L"STATIC",
		bEnglish ? L"Script file name:" : L"スクリプトファイル名:",
		WS_VISIBLE | WS_CHILD,
		MulDiv(10, nDpi, 96),
		MulDiv(60, nDpi, 96),
		MulDiv(100, nDpi, 96),
		MulDiv(16, nDpi, 96),
		hWndDebug,
		0,
		(HINSTANCE)GetWindowLongPtr(hWndDebug, GWLP_HINSTANCE),
		NULL);
	SendMessage(hWndLabelScript, WM_SETFONT, (WPARAM)hFont, (LPARAM)TRUE);
#endif

	/* スクリプト名のテキストボックスを作成する */
	hWndTextboxScript = CreateWindow(
		L"EDIT",
		NULL,
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | WS_BORDER | ES_READONLY| ES_AUTOHSCROLL,
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

#if 0
	/* スクリプトの変更ボタンを作成する */
	hWndBtnChangeScript = CreateWindow(
		L"BUTTON",
		bEnglish ? L"Change" : L"変更",
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
		320, 80, 80, 30,
		hWndDebug, (HMENU)ID_CHANGE_SCRIPT,
		(HINSTANCE)GetWindowLongPtr(hWndDebug, GWLP_HINSTANCE), NULL);
	SendMessage(hWndBtnChangeScript, WM_SETFONT, (WPARAM)hFont, (LPARAM)TRUE);
	CreateTooltip(hWndBtnChangeScript,
				  L"Jump to the script written in the left text box.",
				  L"左のテキストボックスに書いたスクリプトにジャンプします。");
#endif

	/* スクリプトの選択ボタンを作成する */
	hWndBtnSelectScript = CreateWindow(
		L"BUTTON", L"...",
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
		MulDiv(370, nDpi, 96),
		MulDiv(60, nDpi, 96),
		MulDiv(60, nDpi, 96),
		MulDiv(30, nDpi, 96),
		hWndDebug,
		(HMENU)ID_SELECT_SCRIPT,
		(HINSTANCE)GetWindowLongPtr(hWndDebug, GWLP_HINSTANCE),
		NULL);
	SendMessage(hWndBtnSelectScript, WM_SETFONT, (WPARAM)hFont, (LPARAM)TRUE);
	CreateTooltip(hWndBtnSelectScript,
				  L"Select a script file and jump to it.",
				  L"スクリプトファイルを選択してジャンプします。");

#if 0
	/* 行番号ラベルを作成する */
	hWndLabelLine = CreateWindow(
		L"STATIC",
		bEnglish ? L"Next line to be executed:" : L"次に実行される行番号:",
		WS_VISIBLE | WS_CHILD,
		10, 120, 300, 16,
		hWndDebug, 0,
		(HINSTANCE)GetWindowLongPtr(hWndDebug, GWLP_HINSTANCE), NULL);
	SendMessage(hWndLabelLine, WM_SETFONT, (WPARAM)hFont, (LPARAM)TRUE);

	/* 行番号のテキストボックスを作成する */
	hWndTextboxLine = CreateWindow(
		L"EDIT",
		NULL,
		WS_TABSTOP | ES_NUMBER | WS_VISIBLE | WS_CHILD | WS_BORDER,
		10, 140, 80, 30,
		hWndDebug, 0,
		(HINSTANCE)GetWindowLongPtr(hWndDebug, GWLP_HINSTANCE), NULL);
	SendMessage(hWndTextboxLine, WM_SETFONT, (WPARAM)hFont, (LPARAM)TRUE);
	CreateTooltip(hWndTextboxLine,
				  L"Write line number to be jumped to.",
				  L"ジャンプしたい行番号を書きます。");

	/* 行番号の変更ボタンを作成する */
	hWndBtnChangeLine = CreateWindow(
		L"BUTTON",
		bEnglish ? L"Change" : L"変更",
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
		100, 140, 80, 30,
		hWndDebug, (HMENU)ID_CHANGE_LINE,
		(HINSTANCE)GetWindowLongPtr(hWndDebug, GWLP_HINSTANCE), NULL);
	SendMessage(hWndBtnChangeLine, WM_SETFONT, (WPARAM)hFont, (LPARAM)TRUE);
	CreateTooltip(hWndBtnChangeLine,
				  L"Jump to the line written in the left text box.",
				  L"左のテキストボックスに書いた行にジャンプします。");

	/* コマンドのラベルを作成する */
	hWndLabelCommand = CreateWindow(
		L"STATIC",
		bEnglish ? L"Next command to be executed:" :
				   L"次に実行されるコマンド:",
		WS_VISIBLE | WS_CHILD,
		10, 180, 300, 16,
		hWndDebug, 0,
		(HINSTANCE)GetWindowLongPtr(hWndDebug, GWLP_HINSTANCE), NULL);
	SendMessage(hWndLabelCommand, WM_SETFONT, (WPARAM)hFont, (LPARAM)TRUE);

	/* コマンドのテキストボックスを作成する */
	hWndTextboxCommand = CreateWindow(
		L"EDIT",
		NULL,
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOVSCROLL |
		ES_MULTILINE | ES_WANTRETURN, // ES_READONLY
		10, 200, 330, 60,
		hWndDebug, 0,
		(HINSTANCE)GetWindowLongPtr(hWndDebug, GWLP_HINSTANCE), NULL);
	SendMessage(hWndTextboxCommand, WM_SETFONT, (WPARAM)hFontFixed,
				(LPARAM)TRUE);
	CreateTooltip(hWndTextboxCommand,
				  L"This is a command text to be executed next.",
				  L"次に実行されるコマンドのテキストです。"
				  L"編集することで書き換えることができます。");

	/* コマンドアップデートのボタンを作成する */
	hWndBtnUpdate = CreateWindow(
		L"BUTTON",
		bEnglish ? L"Update" : L"更新",
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
		DEBUGGER_WIDTH - 10 - 80, 200, 80, 30,
		hWndDebug, (HMENU)ID_UPDATE_COMMAND,
		(HINSTANCE)GetWindowLongPtr(hWndDebug, GWLP_HINSTANCE), NULL);
	SendMessage(hWndBtnUpdate, WM_SETFONT, (WPARAM)hFont, (LPARAM)TRUE);
	CreateTooltip(hWndBtnUpdate,
				  L"Reflect edited command for execution. ",
				  L"編集したコマンドを実行に反映します。");

	/* コマンドリセットのボタンを作成する */
	hWndBtnReset = CreateWindow(
		L"BUTTON",
		bEnglish ? L"Reset" : L"リセット",
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
		DEBUGGER_WIDTH - 10 - 80, 230, 80, 30,
		hWndDebug, (HMENU)ID_RESET_COMMAND,
		(HINSTANCE)GetWindowLongPtr(hWndDebug, GWLP_HINSTANCE), NULL);
	SendMessage(hWndBtnReset, WM_SETFONT, (WPARAM)hFont, (LPARAM)TRUE);
	CreateTooltip(hWndBtnReset,
				  L"Reset the command text.",
				  L"編集したコマンドを元に戻します。");

	/* スクリプト内容のラベルを作成する */
	hWndLabelContent = CreateWindow(
		L"STATIC",
		bEnglish ? L"Script content:" : L"スクリプトの内容:",
		WS_VISIBLE | WS_CHILD,
		10, 270, 100, 16,
		hWndDebug, 0,
		(HINSTANCE)GetWindowLongPtr(hWndDebug, GWLP_HINSTANCE), NULL);
	SendMessage(hWndLabelContent, WM_SETFONT, (WPARAM)hFont, (LPARAM)TRUE);

	/* スクリプトのリストボックスを作成する */
	hWndListbox = CreateWindow(
		L"LISTBOX",
		NULL,
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | WS_BORDER | WS_VSCROLL |
		LBS_NOTIFY | LBS_WANTKEYBOARDINPUT,
		10, 290, 420, 220,
		hWndDebug, 0,
		(HINSTANCE)GetWindowLongPtr(hWndDebug, GWLP_HINSTANCE), NULL);
	SendMessage(hWndListbox, WM_SETFONT, (WPARAM)hFontFixed, (LPARAM)TRUE);
	CreateTooltip(hWndListbox,
				  L"Current script content.",
				  L"実行中のスクリプトの内容です。");
#endif

	/* スクリプトのリッチエディットを作成する */
	LoadLibrary(L"Msftedit.dll");
	hWndRichEdit = CreateWindowEx(
		0,
		MSFTEDIT_CLASS,
		L"Text",
		ES_MULTILINE | WS_VISIBLE | WS_CHILD | WS_BORDER | WS_TABSTOP | ES_AUTOVSCROLL,
		MulDiv(10, nDpi, 96),
		MulDiv(100, nDpi, 96),
		MulDiv(420, nDpi, 96),
		MulDiv(400, nDpi, 96),
		hWndDebug, 0,
		(HINSTANCE)GetWindowLongPtr(hWndDebug, GWLP_HINSTANCE),
		NULL);
	SendMessage(hWndRichEdit, EM_SHOWSCROLLBAR, (WPARAM)SB_VERT, (LPARAM)TRUE);
	SendMessage(hWndRichEdit, EM_FMTLINES, (WPARAM)TRUE, 0);

#if 0
	/* エラーを探すを有効にする */
	hWndBtnError = CreateWindow(
		L"BUTTON",
		bEnglish ? L"Search for error" : L"エラーを探す",
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
		MulDiv(10, nDpi, 96),
		MulDiv(510, nDpi, 96),
		MulDiv(120, nDpi, 96),
		MulDiv(30, nDpi, 96),
		hWndDebug,
		(HMENU)ID_ERROR,
		(HINSTANCE)GetWindowLongPtr(hWndDebug, GWLP_HINSTANCE),
		NULL);
	SendMessage(hWndBtnError, WM_SETFONT, (WPARAM)hFont, (LPARAM)TRUE);
	CreateTooltip(hWndBtnError,
				  L"Search for a next error.",
				  L"次のエラー箇所を探します。");

	/* 上書き保存ボタンを作成する */
	hWndBtnSave = CreateWindow(
		L"BUTTON",
		bEnglish ? L"Overwrite" : L"上書き保存",
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
		MulDiv(DEBUGGER_WIDTH - 10 - 80 - 10 - 80, nDpi, 96),
		MulDiv(510, nDpi, 96),
		MulDiv(80, nDpi, 96),
		MulDiv(30, nDpi, 96),
		hWndDebug,
		(HMENU)ID_SAVE,
		(HINSTANCE)GetWindowLongPtr(hWndDebug, GWLP_HINSTANCE),
		NULL);
	SendMessage(hWndBtnSave, WM_SETFONT, (WPARAM)hFont, (LPARAM)TRUE);
	CreateTooltip(hWndBtnSave,
				  L"Overwrite the contents of the modified script.",
				  L"スクリプトの内容をファイルに上書き保存します。");

	/* 再読み込みボタンを作成する */
	hWndBtnReload = CreateWindow(
		L"BUTTON",
		bEnglish ? L"Reload" : L"再読み込み",
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
		MulDiv(DEBUGGER_WIDTH - 10 - 80, nDpi, 96),
		MulDiv(510, nDpi, 96),
		MulDiv(80, nDpi, 96),
		MulDiv(30, nDpi, 96),
		hWndDebug,
		(HMENU)ID_RELOAD,
		(HINSTANCE)GetWindowLongPtr(hWndDebug, GWLP_HINSTANCE),
		NULL);
	SendMessage(hWndBtnReload, WM_SETFONT, (WPARAM)hFont, (LPARAM)TRUE);
	CreateTooltip(hWndBtnReload,
				  L"Reload the script from file.",
				  L"スクリプトファイルの内容を再読み込みします。");

	/* 変数のラベルを作成する */
	hWndLabelVar = CreateWindow(
		L"STATIC",
		bEnglish ? L"Variables (non-initial values):" :
				   L"変数 (初期値でない):",
		WS_VISIBLE | WS_CHILD,
		MulDiv(10, nDpi, 96),
		MulDiv(550, nDpi, 96),
		MulDiv(200, nDpi, 96),
		MulDiv(16, nDpi, 96),
		hWndDebug,
		0,
		(HINSTANCE)GetWindowLongPtr(hWndDebug, GWLP_HINSTANCE),
		NULL);
	SendMessage(hWndLabelVar, WM_SETFONT, (WPARAM)hFont, (LPARAM)TRUE);
#endif

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
		(HMENU)ID_WRITE,
		(HINSTANCE)GetWindowLongPtr(hWndDebug, GWLP_HINSTANCE),
		NULL);
	SendMessage(hWndBtnVar, WM_SETFONT, (WPARAM)hFont, (LPARAM)TRUE);
	CreateTooltip(hWndBtnVar,
				  L"Write to the variables.",
				  L"変数の内容を書き込みます。");

	/* メニューを作成する */
	InitDebuggerMenu(hWndMain);

	return TRUE;
}

/*
 デバッガウィンドウの位置を修正する
 */
VOID UpdateDebuggerWindowPosition(int nGameWidth, int nGameHeight)
{
	int y, nDpi, nDebugWidth, nDebugMinHeight;

	nDpi = GetDpiForWindow(hWndMain);
	if (nDpi == 0)
		nDpi = 96;

	nDebugWidth = MulDiv(DEBUGGER_WIDTH, nDpi, 96);
	nDebugMinHeight = MulDiv(DEBUGGER_MIN_HEIGHT, nDpi, 96);

	/* エディタのコントロールをサイズ変更する */
	MoveWindow(hWndRichEdit,
				MulDiv(10, nDpi, 96),
				MulDiv(100, nDpi, 96),
				MulDiv(420, nDpi, 96),
				nGameHeight - MulDiv(180, nDpi, 96),
				TRUE);

	/* エディタより下のコントロールを移動する */
	y = nGameHeight - MulDiv(130, nDpi, 96);
#if 0
	MoveWindow(hWndBtnError,
				MulDiv(10, nDpi, 96),
				y,
				MulDiv(120, nDpi, 96),
				MulDiv(30, nDpi, 96),
				TRUE);
	MoveWindow(hWndBtnSave,
				MulDiv(DEBUGGER_WIDTH - 10 - 80 - 10 - 80, nDpi, 96),
				y,
				MulDiv(80, nDpi, 96),
				MulDiv(30, nDpi, 96),
				TRUE);
	MoveWindow(hWndBtnReload,
				MulDiv(DEBUGGER_WIDTH - 10 - 80, nDpi, 96),
				y,
				MulDiv(80, nDpi, 96),
				MulDiv(30, nDpi, 96),
				TRUE);
	MoveWindow(hWndLabelVar,
				MulDiv(10, nDpi, 96),
				y + MulDiv(40, nDpi, 96),
				MulDiv(200, nDpi, 96),
				MulDiv(16, nDpi, 96),
				TRUE);
#endif
	MoveWindow(hWndTextboxVar,
				MulDiv(10, nDpi, 96),
				y + MulDiv(60, nDpi, 96),
				MulDiv(280, nDpi, 96),
				MulDiv(60, nDpi, 96),
				TRUE);
	MoveWindow(hWndBtnVar,
				MulDiv(300, nDpi, 96),
				y + MulDiv(70, nDpi, 96),
				MulDiv(130, nDpi, 96),
				MulDiv(30, nDpi, 96),
				TRUE);

	/* パネルの位置を変更する */
	MoveWindow(hWndDebug,
				nGameWidth,
				0,
				nDebugWidth,
				nGameHeight,
				TRUE);
}

/* メニューを作成する */
static VOID InitDebuggerMenu(HWND hWnd)
{
	HMENU hMenuFile = CreatePopupMenu();
	HMENU hMenuScript = CreatePopupMenu();
	HMENU hMenuExport = CreatePopupMenu();
	HMENU hMenuHelp = CreatePopupMenu();
    MENUITEMINFO mi;

	bEnglish = conf_locale == LOCALE_JA ? FALSE : TRUE;

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
	mi.dwTypeData = bEnglish ? L"File(&F)": L"ファイル(&F)";
	InsertMenuItem(hMenu, 0, TRUE, &mi);

	/* スクリプト(S)を作成する */
	mi.hSubMenu = hMenuScript;
	mi.dwTypeData = bEnglish ? L"Script(&S)": L"スクリプト(&S)";
	InsertMenuItem(hMenu, 1, TRUE, &mi);

	/* エクスポート(E)を作成する */
	mi.hSubMenu = hMenuExport;
	mi.dwTypeData = bEnglish ? L"Export(&E)": L"エクスポート(&E)";
	InsertMenuItem(hMenu, 2, TRUE, &mi);

	/* ヘルプ(H)を作成する */
	mi.hSubMenu = hMenuHelp;
	mi.dwTypeData = bEnglish ? L"Help(&H)": L"ヘルプ(&H)";
	InsertMenuItem(hMenu, 3, TRUE, &mi);

	/* 2階層目を作成する準備を行う */
	mi.fMask = MIIM_TYPE | MIIM_ID;

	/* スクリプトを開く(Q)を作成する */
	mi.wID = ID_SELECT_SCRIPT;
	mi.dwTypeData = bEnglish ?
		L"Open script(&Q)\tAlt+O" :
		L"スクリプトを開く(&O)\tAlt+O";
	InsertMenuItem(hMenuFile, 0, TRUE, &mi);

	/* スクリプトを上書き保存する(S)を作成する */
	mi.wID = ID_SAVE;
	mi.dwTypeData = bEnglish ?
		L"Overwrite script(&S)\tAlt+S" :
		L"スクリプトを上書き保存する(&S)\tAlt+S";
	InsertMenuItem(hMenuFile, 1, TRUE, &mi);

	/* 終了(Q)を作成する */
	mi.wID = ID_QUIT;
	mi.dwTypeData = bEnglish ? L"Quit(&Q)\tAlt+Q" : L"終了(&Q)\tAlt+Q";
	InsertMenuItem(hMenuFile, 2, TRUE, &mi);

	/* 続ける(C)を作成する */
	mi.wID = ID_RESUME;
	mi.dwTypeData = bEnglish ? L"Resume(&R)\tAlt+R" : L"続ける(&R)\tAlt+R";
	InsertMenuItem(hMenuScript, 0, TRUE, &mi);

	/* 次へ(N)を作成する */
	mi.wID = ID_NEXT;
	mi.dwTypeData = bEnglish ? L"Next(&N)\tAlt+N" : L"次へ(&N)\tAlt+N";
	InsertMenuItem(hMenuScript, 1, TRUE, &mi);

	/* 停止(P)を作成する */
	mi.wID = ID_PAUSE;
	mi.dwTypeData = bEnglish ? L"Pause(&P)\tAlt+P" : L"停止(&P)\tAlt+P";
	InsertMenuItem(hMenuScript, 2, TRUE, &mi);
	EnableMenuItem(hMenu, ID_PAUSE, MF_GRAYED);

	/* 次のエラー箇所へ移動(E)を作成する */
	mi.wID = ID_ERROR;
	mi.dwTypeData = bEnglish ?
		L"Go to next error(&E)\tAlt+E" :
		L"次のエラー箇所へ移動(&E)\tAlt+E";
	InsertMenuItem(hMenuScript, 3, TRUE, &mi);

	/* 再読み込み(R)を作成する */
	mi.wID = ID_RELOAD;
	mi.dwTypeData = bEnglish ? L"Reload(&R)\tF5" : L"再読み込み(&R)\tF5";
	InsertMenuItem(hMenuScript, 4, TRUE, &mi);

	/* パッケージをエクスポートするを作成する */
	mi.wID = ID_EXPORT;
	mi.dwTypeData = bEnglish ?
		L"Export package(&X)" :
		L"パッケージをエクスポートする";
	InsertMenuItem(hMenuExport, 0, TRUE, &mi);

	/* Windows向けにエクスポートするを作成する */
	mi.wID = ID_EXPORT_WIN;
	mi.dwTypeData = bEnglish ?
		L"Export for Windows" :
		L"Windows向けにエクスポートする";
	InsertMenuItem(hMenuExport, 1, TRUE, &mi);

	/* Windows EXEインストーラを作成するを作成する */
	mi.wID = ID_EXPORT_WIN_INST;
	mi.dwTypeData = bEnglish ?
		L"Create EXE Installer for Windows" :
		L"Windows EXEインストーラを作成する";
	InsertMenuItem(hMenuExport, 2, TRUE, &mi);

	/* Windows/Mac向けにエクスポートするを作成する */
	mi.wID = ID_EXPORT_WIN_MAC;
	mi.dwTypeData = bEnglish ?
		L"Export for Windows/Mac" :
		L"Windows/Mac向けにエクスポートする";
	InsertMenuItem(hMenuExport, 3, TRUE, &mi);

	/* Web向けにエクスポートするを作成する */
	mi.wID = ID_EXPORT_WEB;
	mi.dwTypeData = bEnglish ?
		L"Export for Web" :
		L"Web向けにエクスポートする";
	InsertMenuItem(hMenuExport, 4, TRUE, &mi);

	/* Androidプロジェクトをエクスポートするを作成する */
	mi.wID = ID_EXPORT_ANDROID;
	mi.dwTypeData = bEnglish ?
		L"Export Android project" :
		L"Androidプロジェクトをエクスポートする";
	InsertMenuItem(hMenuExport, 5, TRUE, &mi);

	/* iOSプロジェクトをエクスポートするを作成する */
	mi.wID = ID_EXPORT_IOS;
	mi.dwTypeData = bEnglish ?
		L"Export iOS project" :
		L"iOSプロジェクトをエクスポートする";
	InsertMenuItem(hMenuExport, 6, TRUE, &mi);

	/* バージョン(V)を作成する */
	mi.wID = ID_VERSION;
	mi.dwTypeData = bEnglish ? L"Version(&V)" : L"バージョン(&V)\tAlt+V";
	InsertMenuItem(hMenuHelp, 0, TRUE, &mi);

	/* メニューをセットする */
	SetMenu(hWnd, hMenu);
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
		bResumePressed = TRUE;
	}

	return TRUE;
}

/*
 * デバッガのウィンドウハンドルであるかを返す
 */
BOOL IsDebuggerHWND(HWND hWnd)
{
	if(hWnd == hWndDebug ||
	   hWnd == hWndBtnResume ||
	   hWnd == hWndBtnNext ||
	   hWnd == hWndBtnPause ||
	   hWnd == hWndTextboxScript ||
	   hWnd == hWndRichEdit
//	   hWnd == hWndLabelScript ||
//     hWnd == hWndBtnChangeScript ||
//	   hWnd == hWndLabelLine ||
//	   hWnd == hWndTextboxLine ||
//	   hWnd == hWndBtnChangeLine ||
//	   hWnd == hWndLabelCommand ||
//	   hWnd == hWndTextboxCommand ||
//	   hWnd == hWndBtnUpdate ||
//	   hWnd == hWndBtnReset ||
//	   hWnd == hWndListbox ||
//	   hWnd == hWndBtnError
		)
		return TRUE;

	return FALSE;
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
 * デバッガ関連のウィンドウプロシージャの処理を行う
 */
LRESULT CALLBACK WndProcDebugHook(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
#if 0
	case WM_VKEYTOITEM:
		if (hWnd == hWndDebug && LOWORD(wParam) == VK_RETURN)
		{
			OnClickListBox();
			return 0;
		}
		break;
#endif
	default:
		break;
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}

/*
 * エディタのキーボードメッセージ前処理
 *  - return: TRUEならメッセージは無視され、FALSEならメッセージは引き続き処理される
 */
BOOL PretranslateEditKeyDown(MSG* pMsg)
{
	static BOOL bShiftDown;

	/* シフト押下状態を保存する */
	if (pMsg->hwnd == hWndRichEdit && pMsg->wParam == VK_SHIFT)
	{
		if (pMsg->message == WM_KEYDOWN)
			bShiftDown = TRUE;
		if (pMsg->message == WM_KEYUP)
			bShiftDown = TRUE;
	}

	/* 改行を処理する */
	if (pMsg->hwnd == hWndRichEdit && pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN)
	{
		if (bShiftDown)
		{
			/* 改行による行分割をフックして、スクリプトモデルに反映する */
			OnInsertLine();

			/* このメッセージは引き続きリッチエディットで処理する */
			return FALSE;
		}
		else
		{
			/* 改行をフックして、実行位置を移動する */
			OnReturn();

			/* このメッセージはリッチエディットで処理しない */
			return TRUE;
		}
	}

	return FALSE;
}

/*
 * デバッガのWM_COMMANDハンドラ
 */
VOID OnCommandDebug(UINT nID, UINT nEvent)
{
	if(nEvent == LBN_DBLCLK)
	{
//		OnClickListBox();
		return;
	}
	switch(nID)
	{
	case ID_QUIT:
		DestroyWindow(hWndMain);
		break;
	case ID_VERSION:
		MessageBox(hWndMain, bEnglish ? VERSION_EN : VERSION_JP,
				   MSGBOX_TITLE, MB_OK | MB_ICONINFORMATION);
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
#if 0
	case ID_CHANGE_LINE:
		bChangeLinePressed = TRUE;
		break;
	case ID_UPDATE_COMMAND:
		bUpdatePressed = TRUE;
		break;
	case ID_RESET_COMMAND:
		OnPressReset();
		break;
#endif
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
	default:
		break;
	}
}

#if 0
/* リストボックスのクリックと改行キー入力を処理する */
static VOID OnClickListBox(void)
{
	wchar_t line[10];
	if(!bRunning)
	{
		_snwprintf(line, sizeof(line) / sizeof(wchar_t),
					L"%d",
					(int)SendMessage(hWndListbox, LB_GETCURSEL, 0, 0) + 1);
		line[9] = L'\0';
		SetWindowText(hWndTextboxLine, line);
		bChangeLinePressed = TRUE;
	}
}
#endif

/* スクリプト選択ボタンが押された場合の処理を行う */
static VOID OnSelectScript(void)
{
	OPENFILENAMEW ofn;
	wchar_t szPath[1024];
	size_t i;

	memset(szPath, 0, sizeof(szPath));

	ZeroMemory(&ofn, sizeof(OPENFILENAMEW));
	ofn.lStructSize = sizeof(OPENFILENAMEW);
	ofn.hwndOwner = hWndMain;
	ofn.lpstrFilter = bEnglish ?
		L"Text Files\0*.txt;\0All Files(*.*)\0*.*\0\0" : 
		L"テキストファイル\0*.txt;\0すべてのファイル(*.*)\0*.*\0\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFile = szPath;
	ofn.nMaxFile = sizeof(szPath);
	ofn.lpstrInitialDir = conv_utf8_to_utf16(SCRIPT_DIR);
	ofn.Flags = OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
	ofn.lpstrDefExt = L"txt";
	GetOpenFileNameW(&ofn);
	if(ofn.lpstrFile[0])
	{
		for (i = wcslen(szPath) - 1; i != 0; i--) {
			if (*(szPath + i) == L'\\')
			{
				SetWindowText(hWndTextboxScript, szPath + i + 1);
				bChangeScriptPressed = TRUE;
				break;
			}
		}
	}
}

#if 0
/* コマンドリセットボタンが押下された場合の処理を行う */
static VOID OnPressReset(void)
{
	/* コマンド文字列を設定する */
	SetWindowText(hWndTextboxCommand, conv_utf8_to_utf16(get_line_string()));
}
#endif

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

	if (MessageBox(hWndMain, bEnglish ?
				   L"Are you sure you want to overwrite the script file?" :
				   L"スクリプトファイルを上書き保存します。\n"
				   L"よろしいですか？",
				   MSGBOX_TITLE, MB_ICONWARNING | MB_OKCANCEL) != IDOK)
		return;

	path = make_valid_path(SCRIPT_DIR, scr);

	fp = fopen(path, "w");
	if (fp == NULL)
	{
		free(path);
		MessageBox(hWndMain, bEnglish ?
				   L"Cannot write to file." :
				   L"ファイルに書き込めません。",
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
			MessageBox(hWndMain, bEnglish ?
					   L"Cannot write to file." :
					   L"ファイルに書き込めません。",
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
#if 0
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
		for(i=0; i<=start; i++)
		{
			text = get_line_string_at_line_num(i);
			if(text[0] == '!')
			{
				SendMessage(hWndListbox, LB_SETCURSEL, (WPARAM)i, 0);
				return;
			}
			
		}
	}

	MessageBox(hWndDebug, bEnglish ?
			   L"No error." : L"エラーはありません。",
			   MSGBOX_TITLE, MB_OK | MB_ICONINFORMATION);
#endif
}

/* 変数の書き込みボタンが押下された場合を処理する */
static VOID OnPressWriteVars(void)
{
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

	UpdateVariableTextBox();
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

/* 変数の情報を更新する */
static VOID UpdateVariableTextBox(void)
{
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
 * platform.h
 */

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
bool is_line_changed(void)
{
	bool ret = bChangeLinePressed;
	bChangeLinePressed = FALSE;
	return ret;
}

/*
 * 変更された実行する行番号を取得する
 */
int get_changed_line(void)
{
#if 0
	static wchar_t text[256];
	int line;

	GetWindowText(hWndTextboxLine,
					text,
					sizeof(text) / sizeof(wchar_t) - 1);
	line = _wtoi(text) - 1;

	return line;
#endif
	CHARRANGE cr;
	int nCursor, nLine;

	/* カーソル位置を取得する */
	SendMessage(hWndRichEdit, EM_EXGETSEL, 0, (LPARAM)&cr);
	nCursor = cr.cpMin;

	/* カーソル位置の行番号を取得する*/
	nLine = SendMessage(hWndRichEdit, EM_EXLINEFROMCHAR, 0, (LPARAM)nCursor);
	assert(nLine < get_line_count());

	return nLine;
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
#if 0
	static wchar_t text[4096];
	wchar_t *p;

	GetWindowText(hWndTextboxCommand,
					text,
					sizeof(text) / sizeof(wchar_t) - 1);

	/* 改行をスペースに置き換える */
	p = text;
	while(*p)
	{
		if(*p == L'\r' || *p == L'\n')
			*p = L' ';
		p++;
	}

	return conv_utf16_to_utf8(text);
#endif
	return "";
}

/*
 * スクリプトがリロードされたかを調べる
 */
bool is_script_reloaded(void)
{
	bool ret = bReloadPressed;
	bReloadPressed = FALSE;
	return ret;
}

/*
 * コマンドの実行中状態を設定する
 */
void set_running_state(bool running, bool request_stop)
{
	bRunning = running;

	/* 停止によりコマンドの完了を待機中のとき */
	if(request_stop)
	{
		/* 続けるボタンを無効にする */
		EnableWindow(hWndBtnResume, FALSE);
		SetWindowText(hWndBtnResume, bEnglish ? L"(Resume)" : L"(続ける)");

		/* 次へボタンを無効にする */
		EnableWindow(hWndBtnNext, FALSE);
		SetWindowText(hWndBtnNext, bEnglish ? L"(Next)" : L"(次へ)");

		/* 停止ボタンを無効にする */
		EnableWindow(hWndBtnPause, FALSE);
		SetWindowText(hWndBtnPause, bEnglish ? L"(Waiting)" : L"(完了待ち)");

		/* スクリプトテキストボックスを無効にする */
		EnableWindow(hWndTextboxScript, FALSE);

		/* スクリプト選択ボタンを無効にする */
		EnableWindow(hWndBtnSelectScript, FALSE);

#if 0
		/* スクリプト変更ボタンを無効にする */
		EnableWindow(hWndBtnChangeScript, FALSE);

		/* 行番号ラベルを設定する */
		SetWindowText(hWndLabelLine, bEnglish ?
					  L"Current waiting line:" :
					  L"現在完了待ちの行番号:");

		/* 行番号テキストボックスを無効にする */
		EnableWindow(hWndTextboxLine, FALSE);

		/* 行番号変更ボタンを無効にする */
		EnableWindow(hWndBtnChangeLine, FALSE);

		/* コマンドラベルを設定する */
		SetWindowText(hWndLabelCommand, bEnglish ?
					  L"Current waiting command:" :
					  L"現在完了待ちのコマンド:");

		/* コマンドテキストボックスを無効にする */
		EnableWindow(hWndTextboxCommand, FALSE);

		/* コマンドアップデートボタンを無効にする */
		EnableWindow(hWndBtnUpdate, FALSE);

		/* コマンドリセットボタンを無効にする */
		EnableWindow(hWndBtnReset, FALSE);

		/* リストボックスを無効にする */
//		EnableWindow(hWndListbox, FALSE);
#endif

		/* リッチエディットを無効にする */
		EnableWindow(hWndRichEdit, FALSE);

#if 0
		/* エラーを探すを無効にする */
		EnableWindow(hWndBtnError, FALSE);

		/* 上書き保存ボタンを無効にする */
		EnableWindow(hWndBtnSave, FALSE);

		/* 再読み込みボタンを無効にする */
		EnableWindow(hWndBtnReload, FALSE);
#endif

		/* 変数のテキストボックスを無効にする */
		SendMessage(hWndTextboxVar, EM_SETREADONLY, TRUE, 0);

		/* 変数の書き込みボタンを無効にする */
		EnableWindow(hWndBtnVar, FALSE);

		/* スクリプトを開くメニューを無効にする */
		EnableMenuItem(hMenu, ID_SELECT_SCRIPT, MF_GRAYED);

		/* 上書き保存メニューを無効にする */
		EnableMenuItem(hMenu, ID_SAVE, MF_GRAYED);

		/* エクスポートメニューを無効にする */
		EnableMenuItem(hMenu, ID_EXPORT, MF_GRAYED);

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
		/* 続けるボタンを無効にする */
		EnableWindow(hWndBtnResume, FALSE);
		SetWindowText(hWndBtnResume, bEnglish ? L"(Resume)" : L"(続ける)");

		/* 次へボタンを無効にする */
		EnableWindow(hWndBtnNext, FALSE);
		SetWindowText(hWndBtnNext, bEnglish ? L"(Next)" : L"(次へ)");

		/* 停止ボタンを有効にする */
		EnableWindow(hWndBtnPause, TRUE);
		SetWindowText(hWndBtnPause, bEnglish ? L"Pause" : L"停止");

		/* スクリプトテキストボックスを無効にする */
		EnableWindow(hWndTextboxScript, FALSE);

		/* スクリプト選択ボタンを無効にする */
		EnableWindow(hWndBtnSelectScript, FALSE);

#if 0
		/* スクリプト変更ボタンを無効にする */
		EnableWindow(hWndBtnChangeScript, FALSE);

		/* 行番号ラベルを設定する */
		SetWindowText(hWndLabelLine, bEnglish ?
					  L"Current running line:" :
					  L"現在実行中の行番号:");

		/* 行番号テキストボックスを無効にする */
		EnableWindow(hWndTextboxLine, FALSE);

		/* 行番号変更ボタンを無効にする */
		EnableWindow(hWndBtnChangeLine, FALSE);

		/* コマンドラベルを設定する */
		SetWindowText(hWndLabelCommand, bEnglish ?
					  L"Current running command:" :
					  L"現在実行中のコマンド:");

		/* コマンドテキストボックスを無効にする */
		EnableWindow(hWndTextboxCommand, FALSE);

		/* コマンドアップデートボタンを無効にする */
		EnableWindow(hWndBtnUpdate, FALSE);

		/* コマンドリセットボタンを無効にする */
		EnableWindow(hWndBtnReset, FALSE);

		/* リストボックスを無効にする */
//		EnableWindow(hWndRichEdit, FALSE);
#endif

		/* リッチエディットを無効にする */
		EnableWindow(hWndRichEdit, FALSE);

#if 0
		/* エラーを探すを無効にする */
		EnableWindow(hWndBtnError, FALSE);

		/* 上書きボタンを無効にする */
		EnableWindow(hWndBtnSave, FALSE);

		/* 再読み込みボタンを無効にする */
		EnableWindow(hWndBtnReload, FALSE);
#endif

		/* 変数のテキストボックスを無効にする */
		SendMessage(hWndTextboxVar, EM_SETREADONLY, TRUE, 0);

		/* 変数の書き込みボタンを無効にする */
		EnableWindow(hWndBtnVar, FALSE);

		/* スクリプトを開くメニューを無効にする */
		EnableMenuItem(hMenu, ID_SELECT_SCRIPT, MF_GRAYED);

		/* 上書き保存メニューを無効にする */
		EnableMenuItem(hMenu, ID_SAVE, MF_GRAYED);

		/* エクスポートメニューを無効にする */
		EnableMenuItem(hMenu, ID_EXPORT, MF_GRAYED);

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
		/* 続けるボタンを有効にする */
		EnableWindow(hWndBtnResume, TRUE);
		SetWindowText(hWndBtnResume, bEnglish ? L"Resume" : L"続ける");

		/* 次へボタンを有効にする */
		EnableWindow(hWndBtnNext, TRUE);
		SetWindowText(hWndBtnNext, bEnglish ? L"Next" : L"次へ");

		/* 停止ボタンを無効にする */
		EnableWindow(hWndBtnPause, FALSE);
		SetWindowText(hWndBtnPause, bEnglish ? L"(Pausing)" : L"(停止中)");

		/* スクリプトテキストボックスを有効にする */
		EnableWindow(hWndTextboxScript, TRUE);

		/* スクリプト選択ボタンを有効にする */
		EnableWindow(hWndBtnSelectScript, TRUE);

#if 0
		/* スクリプト変更ボタンを有効にする */
		EnableWindow(hWndBtnChangeScript, TRUE);

		/* 行番号ラベルを設定する */
		SetWindowText(hWndLabelLine, bEnglish ?
					  L"Next line to be executed:" :
					  L"次に実行される行番号:");

		/* 行番号テキストボックスを有効にする */
		EnableWindow(hWndTextboxLine, TRUE);

		/* 行番号変更ボタンを有効にする */
		EnableWindow(hWndBtnChangeLine, TRUE);

		/* コマンドラベルを設定する */
		SetWindowText(hWndLabelCommand, bEnglish ?
					  L"Next command to be executed:" :
					  L"次に実行されるコマンド:");

		/* コマンドテキストボックスを有効にする */
		EnableWindow(hWndTextboxCommand, TRUE);

		/* コマンドアップデートボタンを有効にする */
		EnableWindow(hWndBtnUpdate, TRUE);

		/* コマンドリセットボタンを有効にする */
		EnableWindow(hWndBtnReset, TRUE);

		/* リストボックスを有効にする */
//		EnableWindow(hWndListbox, TRUE);
#endif

		/* リッチエディットを有効にする */
		EnableWindow(hWndRichEdit, TRUE);

#if 0
		/* エラーを探すを有効にする */
		EnableWindow(hWndBtnError, TRUE);

		/* 上書き保存ボタンを有効にする */
		EnableWindow(hWndBtnSave, TRUE);

		/* 再読み込みボタンを有効にする */
		EnableWindow(hWndBtnReload, TRUE);
#endif

		/* 変数のテキストボックスを有効にする */
		SendMessage(hWndTextboxVar, EM_SETREADONLY, FALSE, 0);

		/* 変数の書き込みボタンを有効にする */
		EnableWindow(hWndBtnVar, TRUE);

		/* スクリプトを開くメニューを有効にする */
		EnableMenuItem(hMenu, ID_SELECT_SCRIPT, MF_ENABLED);

		/* 上書き保存メニューを有効にする */
		EnableMenuItem(hMenu, ID_SAVE, MF_ENABLED);

		/* エクスポートメニューを有効にする */
		EnableMenuItem(hMenu, ID_EXPORT, MF_ENABLED);

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
	wchar_t *buf;
	const char *script_file;
	int line_num, script_size;

	/* スクリプトファイル名を設定する */
	script_file = get_script_file_name();
	SetWindowText(hWndTextboxScript, conv_utf8_to_utf16(script_file));

#if 0
	/* 行番号を設定する */
	_snwprintf(line_num_buf,
				sizeof(line_num_buf) / sizeof(wchar_t),
				L"%d",
				get_expanded_line_num() + 1);
	line_num_buf[9] = L'\0';
	SetWindowText(hWndTextboxLine, line_num_buf);

	/* コマンド文字列を設定する */
	line_string = get_line_string();
	SetWindowText(hWndTextboxCommand, conv_utf8_to_utf16(line_string));

	/* スクリプトのリストボックスを設定する */
	if(script_changed)
	{
		SendMessage(hWndListbox, LB_RESETCONTENT, 0 , 0);
		for(line_num = 0; line_num < get_line_count(); line_num++)
		{
			command_string = get_line_string_at_line_num(line_num);
			SendMessage(hWndListbox, LB_ADDSTRING, 0,
						(LPARAM)conv_utf8_to_utf16(command_string));
		}
	}
	line_num = get_expanded_line_num();
	top = (line_num - 9 < 0) ? 0 : (line_num - 9);
	SendMessage(hWndListbox, LB_SETCURSEL, (WPARAM)line_num, 0);
	SendMessage(hWndListbox, LB_SETTOPINDEX, (WPARAM)top, 0);
#endif

	/* スクリプトのリッチエディットを設定する */
	if (script_changed)
	{
		script_size = 0;
		for (line_num = 0; line_num < get_line_count(); line_num++)
			script_size += strlen(get_line_string_at_line_num(line_num)) + 1;

		buf = malloc((script_size + 1) * sizeof(wchar_t));
		buf[0] = L'\0';
		for (line_num = 0; line_num < get_line_count(); line_num++)
		{
			wcscat(buf, conv_utf8_to_utf16(get_line_string_at_line_num(line_num)));
			wcscat(buf, L"\r");
		}
		SetWindowText(hWndRichEdit, buf);
		free(buf);
	}
	UpdateLineHighlight();

	// TODO: スクロール
	// line_num = get_expanded_line_num();
	// top = (line_num - 9 < 0) ? 0 : (line_num - 9);
	// SendMessage(hWndListbox, LB_SETCURSEL, (WPARAM)line_num, 0);
	// SendMessage(hWndListbox, LB_SETTOPINDEX, (WPARAM)top, 0);

	/* 変数の情報を更新する */
	if(check_variable_updated() || script_changed)
		UpdateVariableTextBox();
}

/* 実行行のハイライトを行う */
static VOID UpdateLineHighlight(void)
{
	CHARFORMAT2W cf;
	CHARRANGE cr;
	wchar_t* pText, *pCrLf;
	int nLine, nTextLen, nLineStart, nLineStartWithoutLf, nLineLen, i;

	/* 全体の書式をクリアして、背景を白にする */
	memset(&cf, 0, sizeof(cf));
	cf.cbSize = sizeof(cf);
	cf.dwMask = CFM_BACKCOLOR;
	cf.crBackColor = 0x00ffffff;
	wcscpy(&cf.szFaceName[0], L"BIZ UDゴシック");
	SendMessage(hWndRichEdit, EM_SETCHARFORMAT, (WPARAM)SCF_ALL, (LPARAM)&cf);

	/* 実行行を取得する */
	nLine = get_expanded_line_num();

	/* リッチエディットのテキストの長さを取得する */
	nTextLen = SendMessage(hWndRichEdit, WM_GETTEXTLENGTH, 0, 0);
	if (nTextLen == 0)
		return;

	/* テキスト全体を取得する */
	pText = malloc((nTextLen + 1) * sizeof(wchar_t));
	if (pText == NULL)
	{
		log_memory();
		abort();
	}
	SendMessage(hWndRichEdit, WM_GETTEXT, (WPARAM)nTextLen, (LPARAM)pText);
	pText[nTextLen] = L'\0';

	/* 実行行の開始文字位置を求める */
	nLineStart = 0;			/* WM_GETTEXTは改行をCRLFで返す */
	nLineStartWithoutLf = 0;	/* EM_EXSETSELでは改行はCRの1文字 */
	nLineLen = 0;
	for (i = 0; i < nLine; i++)
	{
		pCrLf = wcswcs(pText + nLineStart, L"\r\n");
		if (i < get_line_count() - 1)
		{
			int len;
			assert(pCrLf != NULL);
			len = pCrLf - (pText + nLineStart);
			nLineStart += len + 2; /* +1 for CRLF */
			nLineStartWithoutLf += len + 1; /* +1 for CR */
		}
	}
	pCrLf = wcswcs(pText + nLineStart, L"\r\n");
	nLineLen = (pCrLf != NULL) ? (pCrLf - (pText + nLineStart)) : wcslen(pText + nLineStart);

	/* 実行行を選択する */
	memset(&cr, 0, sizeof(cr));
	cr.cpMin = nLineStartWithoutLf;
	cr.cpMax = nLineStartWithoutLf + nLineLen;
	SendMessage(hWndRichEdit, EM_EXSETSEL, 0, (LPARAM)&cr);

	/* 選択範囲(実行行)の背景色を変更する */
	memset(&cf, 0, sizeof(cf));
	cf.cbSize = sizeof(cf);
	cf.dwMask = CFM_BACKCOLOR;
	cf.crBackColor = 0x00ffc0c0;
	SendMessage(hWndRichEdit, EM_SETCHARFORMAT, (WPARAM)SCF_SELECTION, (LPARAM)&cf);

	/* カーソル位置を実行行の先頭に設定する */
	memset(&cr, 0, sizeof(cr));
	cr.cpMin = nLineStart;
	cr.cpMax = nLineStart;
	SendMessage(hWndRichEdit, EM_EXSETSEL, 0, (LPARAM)&cr);

	/* スクロールする */
	SendMessage(hWndRichEdit, EM_SCROLLCARET, 0, 0);
}

/* 改行を挿入する */
static VOID OnInsertLine(void)
{
	CHARRANGE cr;
	GETTEXTEX gt;
	wchar_t *pLineWcsBefore, *pStop;
	char *pLineUtf8After;
	int nCursor, nLine, nLineStart, nLineLen, nOfsInLine;

	/* カーソル位置を取得する */
	SendMessage(hWndRichEdit, EM_EXGETSEL, 0, (LPARAM)&cr);
	nCursor = cr.cpMin;

	/* カーソル位置の行番号を取得する*/
	nLine = SendMessage(hWndRichEdit, EM_EXLINEFROMCHAR, 0, (LPARAM)nCursor);
	assert(nLine < get_line_count());

	/* カーソル位置の行の先頭文字の位置を取得する*/
	nLineStart = SendMessage(hWndRichEdit, EM_LINEINDEX, (WPARAM)nLine, 0);

	/* カーソル位置の行の長さを取得する*/
	nLineLen = SendMessage(hWndRichEdit, EM_LINELENGTH, (WPARAM)nCursor, 0);

	/* 作業用の文字列バッファを確保する */
	pLineWcsBefore = malloc((nLineLen + 2) * sizeof(wchar_t));
	pLineUtf8After = malloc(nLineLen + 2);
	if (pLineWcsBefore == NULL || pLineUtf8After == NULL)
	{
		log_memory();
		abort();
	}

	/* カーソル位置の行の内容を取得する*/
	SendMessage(hWndRichEdit, EM_GETLINE, (WPARAM)nLine, (LPARAM)pLineWcsBefore);
	pLineWcsBefore[nLineLen - 1] = L'\0';	/* NUL termination */
	pStop = wcswcs(pLineWcsBefore, L"\r");
	if (pStop != NULL)
		*pStop = L'\0';	/* ignore CR */

	/* カーソル位置以降をUtf-8に変換して作業用バッファにコピーする */
	nOfsInLine = nCursor - nLineStart;
	strcpy(pLineUtf8After, conv_utf16_to_utf8(pLineWcsBefore + nOfsInLine));

	/* カーソル位置でワイド文字列を終端する */
	pLineWcsBefore[nOfsInLine] = L'\0';

	/* スクリプトモデルで行分割を行う */
	update_script_line(nLine, conv_utf16_to_utf8(pLineWcsBefore), pLineUtf8After);

	free(pLineWcsBefore);
	free(pLineUtf8After);

	/*
	 * このあとリッチテキストエディットに改行が送られ、
	 * コントロール内でも行分割される
	 */
}

/* 改行を処理する */
static VOID OnReturn(void)
{
	bChangeLinePressed = TRUE;
}
