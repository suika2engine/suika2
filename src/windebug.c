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
 *  2023-09-20 Android/iOSエクスポート対応
 */

#include "suika.h"
#include "windebug.h"
#include "package.h"
#include "resource.h"

#include <commctrl.h>

/* UTF-8からSJISへの変換バッファサイズ */
#define NATIVE_MESSAGE_SIZE	(65536)

/* 変数テキストボックスのテキストの最大長(形: "$00001=12345678901\r\n") */
#define VAR_TEXTBOX_MAX		(11000 * (1 + 5 + 1 + 11 + 2))

/* ウィンドウクラス名 */
const wchar_t wszWindowClass[] = L"SuikaDebugPanel";

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

//static HWND hWndRichEdit;

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

/* 前方参照 */
static VOID InitDebuggerMenu(HWND hWnd);
static HWND CreateTooltip(HWND hWndBtn, const wchar_t *pszTextEnglish,
						  const wchar_t *pszTextJapanese);
static VOID OnClickListBox(void);
static VOID OnSelectScript(void);
static VOID OnPressReset(void);
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

/*
 * デバッガパネルを作成する
 */
BOOL InitDebuggerPanel(HWND hMainWnd, HWND hGameWnd, void *pWndProc)
{
	WNDCLASSEX wcex;
	RECT rcClient;
	HFONT hFont, hFontFixed;

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

	/* フォントを作成する */
	hFont = CreateFont(16, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE,
					   ANSI_CHARSET, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS,
					   DEFAULT_QUALITY,
					   DEFAULT_PITCH | FF_DONTCARE, L"Yu Gothic UI");
	hFontFixed = CreateFont(16, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE,
							ANSI_CHARSET, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS,
							PROOF_QUALITY,
							DEFAULT_PITCH | FF_DONTCARE, L"Yu Gothic");

	/* 続けるボタンを作成する */
	hWndBtnResume = CreateWindow(
		L"BUTTON",
		bEnglish ? L"Resume" : L"続ける",
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
		10, 10, 100, 40,
		hWndDebug, (HMENU)ID_RESUME,
		(HINSTANCE)GetWindowLongPtr(hWndDebug, GWLP_HINSTANCE), NULL);
	SendMessage(hWndBtnResume, WM_SETFONT, (WPARAM)hFont, (LPARAM)TRUE);
	CreateTooltip(hWndBtnResume,
				  L"Start executing script and run continuosly.",
				  L"スクリプトの実行を開始し、継続して実行します。");

	/* 次へボタンを作成する */
	hWndBtnNext = CreateWindow(
		L"BUTTON",
		bEnglish ? L"Next" : L"次へ",
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
		120, 10, 100, 40,
		hWndDebug, (HMENU)ID_NEXT,
		(HINSTANCE)GetWindowLongPtr(hWndDebug, GWLP_HINSTANCE), NULL);
	SendMessage(hWndBtnNext, WM_SETFONT, (WPARAM)hFont, (LPARAM)TRUE);
	CreateTooltip(hWndBtnNext,
				  L"Run only one command and stop after it.",
				  L"コマンドを1個だけ実行し、停止します。");

	/* 停止ボタンを作成する */
	hWndBtnPause = CreateWindow(
		L"BUTTON",
		bEnglish ? L"(Paused)" : L"(停止中)",
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
		330, 10, 100, 40,
		hWndDebug, (HMENU)ID_PAUSE,
		(HINSTANCE)GetWindowLongPtr(hWndDebug, GWLP_HINSTANCE), NULL);
	EnableWindow(hWndBtnPause, FALSE);
	SendMessage(hWndBtnPause, WM_SETFONT, (WPARAM)hFont, (LPARAM)TRUE);
	CreateTooltip(hWndBtnPause,
				  L"Stop script execution.",
				  L"コマンドの実行を停止します。");

	/* スクリプトラベルを作成する */
	hWndLabelScript = CreateWindow(
		L"STATIC",
		bEnglish ? L"Script file name:" : L"スクリプトファイル名:",
		WS_VISIBLE | WS_CHILD,
		10, 60, 100, 16,
		hWndDebug, 0,
		(HINSTANCE)GetWindowLongPtr(hWndDebug, GWLP_HINSTANCE), NULL);
	SendMessage(hWndLabelScript, WM_SETFONT, (WPARAM)hFont, (LPARAM)TRUE);

	/* スクリプト名のテキストボックスを作成する */
	hWndTextboxScript = CreateWindow(
		L"EDIT",
		NULL,
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL,
		10, 80, 300, 30,
		hWndDebug, 0,
		(HINSTANCE)GetWindowLongPtr(hWndDebug, GWLP_HINSTANCE), NULL);
	SendMessage(hWndTextboxScript, WM_SETFONT, (WPARAM)hFont, (LPARAM)TRUE);
	CreateTooltip(hWndTextboxScript,
				  L"Write script file name to be jumped to.",
				  L"ジャンプしたいスクリプトファイル名を書きます。");

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

	/* スクリプトの選択ボタンを作成する */
	hWndBtnSelectScript = CreateWindow(
		L"BUTTON", L"...",
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
		405, 80, 25, 30,
		hWndDebug, (HMENU)ID_SELECT_SCRIPT,
		(HINSTANCE)GetWindowLongPtr(hWndDebug, GWLP_HINSTANCE), NULL);
	SendMessage(hWndBtnSelectScript, WM_SETFONT, (WPARAM)hFont, (LPARAM)TRUE);
	CreateTooltip(hWndBtnSelectScript,
				  L"Select a script file and jump to it.",
				  L"スクリプトファイルを選択してジャンプします。");

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

	/* エラーを探すを有効にする */
	hWndBtnError = CreateWindow(
		L"BUTTON",
		bEnglish ? L"Search for error" : L"エラーを探す",
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
		10, 510, 120, 30,
		hWndDebug, (HMENU)ID_ERROR,
		(HINSTANCE)GetWindowLongPtr(hWndDebug, GWLP_HINSTANCE), NULL);
	SendMessage(hWndBtnError, WM_SETFONT, (WPARAM)hFont, (LPARAM)TRUE);
	CreateTooltip(hWndBtnError,
				  L"Search for a next error.",
				  L"次のエラー箇所を探します。");

	/* 上書き保存ボタンを作成する */
	hWndBtnSave = CreateWindow(
		L"BUTTON",
		bEnglish ? L"Overwrite" : L"上書き保存",
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
		DEBUGGER_WIDTH - 10 - 80 - 10 - 80, 510, 80, 30,
		hWndDebug, (HMENU)ID_SAVE,
		(HINSTANCE)GetWindowLongPtr(hWndDebug, GWLP_HINSTANCE), NULL);
	SendMessage(hWndBtnSave, WM_SETFONT, (WPARAM)hFont, (LPARAM)TRUE);
	CreateTooltip(hWndBtnSave,
				  L"Overwrite the contents of the modified script.",
				  L"スクリプトの内容をファイルに上書き保存します。");

	/* 再読み込みボタンを作成する */
	hWndBtnReload = CreateWindow(
		L"BUTTON",
		bEnglish ? L"Reload" : L"再読み込み",
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
		DEBUGGER_WIDTH - 10 - 80, 510, 80, 30,
		hWndDebug, (HMENU)ID_RELOAD,
		(HINSTANCE)GetWindowLongPtr(hWndDebug, GWLP_HINSTANCE), NULL);
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
		10, 550, 200, 16,
		hWndDebug, 0,
		(HINSTANCE)GetWindowLongPtr(hWndDebug, GWLP_HINSTANCE), NULL);
	SendMessage(hWndLabelVar, WM_SETFONT, (WPARAM)hFont, (LPARAM)TRUE);

	/* 変数のテキストボックスを作成する */
	hWndTextboxVar = CreateWindow(
		L"EDIT",
		NULL,
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | WS_BORDER | WS_VSCROLL |
		ES_AUTOVSCROLL | ES_MULTILINE | ES_WANTRETURN,
		10, 570, 280, 60,
		hWndDebug, 0,
		(HINSTANCE)GetWindowLongPtr(hWndDebug, GWLP_HINSTANCE), NULL);
	SendMessage(hWndTextboxVar, WM_SETFONT, (WPARAM)hFontFixed, (LPARAM)TRUE);
	CreateTooltip(hWndTextboxVar,
				  L"List of variables which have non-initial values.",
				  L"初期値から変更された変数の一覧です。");

	/* 値を書き込むボタンを作成する */
	hWndBtnVar = CreateWindow(
		L"BUTTON",
		bEnglish ? L"Write values" : L"値を書き込む",
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
		300, 570, 130, 30,
		hWndDebug, (HMENU)ID_WRITE,
		(HINSTANCE)GetWindowLongPtr(hWndDebug, GWLP_HINSTANCE), NULL);
	SendMessage(hWndBtnVar, WM_SETFONT, (WPARAM)hFont, (LPARAM)TRUE);
	CreateTooltip(hWndBtnVar,
				  L"Write to the variables.",
				  L"変数の内容を書き込みます。");

	/* スクリプトのリストボックスを作成する */
#if 0
	LoadLibrary(L"Msftedit.dll");
	hWndRichEdit = CreateWindowEx(
		0,
		L"RICHEDIT50W",
		L"Text",
		ES_MULTILINE | WS_VISIBLE | WS_CHILD | WS_BORDER | WS_TABSTOP,
		10, 290, 420, 220,
		hWndDebug, 0,
		(HINSTANCE)GetWindowLongPtr(hWndDebug, GWLP_HINSTANCE), NULL);
	SendMessage(hWndRichEdit, WM_SETFONT, (WPARAM)hFontFixed, (LPARAM)TRUE);
	CreateTooltip(hWndRichEdit,
				  L"Current script content.",
				  L"実行中のスクリプトの内容です。");
#endif

	/* メニューを作成する */
	InitDebuggerMenu(hWndMain);

	return TRUE;
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
 デバッガウィンドウの位置を修正する
 */
VOID UpdateDebuggerWindowPosition(int nGameWidth, int nGameHeight)
{
	MoveWindow(hWndDebug,
			   nGameWidth,
			   0,
			   DEBUGGER_WIDTH,
			   nGameHeight,
			   TRUE);
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
	   hWnd == hWndLabelScript ||
	   hWnd == hWndTextboxScript ||
	   hWnd == hWndBtnChangeScript ||
	   hWnd == hWndLabelLine ||
	   hWnd == hWndTextboxLine ||
	   hWnd == hWndBtnChangeLine ||
	   hWnd == hWndLabelCommand ||
	   hWnd == hWndTextboxCommand ||
	   hWnd == hWndBtnUpdate ||
	   hWnd == hWndBtnReset ||
	   hWnd == hWndListbox ||
	   hWnd == hWndBtnError)
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
LRESULT CALLBACK WndProcDebugHook(HWND hWnd, UINT message, WPARAM wParam,
								  LPARAM lParam)
{
	switch(message)
	{
	case WM_VKEYTOITEM:
		if(hWnd == hWndDebug && LOWORD(wParam) == VK_RETURN)
		{
			OnClickListBox();
			return 0;
		}
		break;
	default:
		break;
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}

/*
 * デバッガのWM_COMMANDハンドラ
 */
VOID OnCommandDebug(UINT nID, UINT nEvent)
{
	if(nEvent == LBN_DBLCLK)
	{
		OnClickListBox();
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

/* リストボックスのクリックと改行キー入力を処理する */
static VOID OnClickListBox(void)
{
	wchar_t line[10];
	if(!bRunning)
	{
		_snwprintf(line, sizeof(line), L"%d",
				 (int)SendMessage(hWndListbox, LB_GETCURSEL, 0, 0) + 1);
		SetWindowText(hWndTextboxLine, line);
		bChangeLinePressed = TRUE;
	}
}

/* スクリプト選択ボタンが押された場合の処理を行う */
static VOID OnSelectScript(void)
{
	OPENFILENAMEW ofn;
	wchar_t szPath[1024];
	size_t i;

	szPath[0] = '\0';

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
				SetWindowText(hWndTextboxScript, szPath + i);
				bChangeScriptPressed = TRUE;
				break;
			}
		}
	}
}

/* コマンドリセットボタンが押下された場合の処理を行う */
static VOID OnPressReset(void)
{
	/* コマンド文字列を設定する */
	SetWindowText(hWndTextboxCommand, conv_utf8_to_utf16(get_line_string()));
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
}

/* 変数の書き込みボタンが押下された場合を処理する */
static VOID OnPressWriteVars(void)
{
	wchar_t *p, *next_line;
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
			   L"Android Studioでそのままビルドできます。\n"
			   L"build.ps1を実行すれば自動ビルドも可能です。",
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
		_snwprintf(line, sizeof(line), L"$%d=%d\r\n", index, val);
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

	GetWindowText(hWndTextboxScript, script, sizeof(script) - 1);

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
 * 変更された実行するスクリプトファイル名を取得する
 */
int get_changed_line(void)
{
	static wchar_t text[256];
	int line;

	GetWindowText(hWndTextboxLine, text, sizeof(text) - 1);

	line = _wtoi(text) - 1;

	return line;
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
	static wchar_t text[4096];
	wchar_t *p;

	GetWindowText(hWndTextboxCommand, text, sizeof(text) - 1);

	/* 改行をスペースに置き換える */
	p = text;
	while(*p)
	{
		if(*p == L'\r' || *p == L'\n')
			*p = L' ';
		p++;
	}

	return conv_utf16_to_utf8(text);
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

		/* スクリプト変更ボタンを無効にする */
		EnableWindow(hWndBtnChangeScript, FALSE);

		/* スクリプト選択ボタンを無効にする */
		EnableWindow(hWndBtnSelectScript, FALSE);

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

		/* リストボックスを有効にする */
		EnableWindow(hWndListbox, FALSE);

		/* エラーを探すを無効にする */
		EnableWindow(hWndBtnError, FALSE);

		/* 上書き保存ボタンを無効にする */
		EnableWindow(hWndBtnSave, FALSE);

		/* 再読み込みボタンを無効にする */
		EnableWindow(hWndBtnReload, FALSE);

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

		/* スクリプト変更ボタンを無効にする */
		EnableWindow(hWndBtnChangeScript, FALSE);

		/* スクリプト選択ボタンを無効にする */
		EnableWindow(hWndBtnSelectScript, FALSE);

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
		EnableWindow(hWndListbox, FALSE);

		/* エラーを探すを無効にする */
		EnableWindow(hWndBtnError, FALSE);

		/* 上書きボタンを無効にする */
		EnableWindow(hWndBtnSave, FALSE);

		/* 再読み込みボタンを無効にする */
		EnableWindow(hWndBtnReload, FALSE);

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

		/* スクリプト変更ボタンを有効にする */
		EnableWindow(hWndBtnChangeScript, TRUE);

		/* スクリプト選択ボタンを有効にする */
		EnableWindow(hWndBtnSelectScript, TRUE);

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
		EnableWindow(hWndListbox, TRUE);

		/* エラーを探すを有効にする */
		EnableWindow(hWndBtnError, TRUE);

		/* 上書き保存ボタンを有効にする */
		EnableWindow(hWndBtnSave, TRUE);

		/* 再読み込みボタンを有効にする */
		EnableWindow(hWndBtnReload, TRUE);

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
	wchar_t line[10];
	const char *command;
	int line_num;
	int top;

	/* スクリプトファイル名を設定する */
	SetWindowText(hWndTextboxScript,
				  conv_utf8_to_utf16(get_script_file_name()));

	/* 行番号を設定する */
	_snwprintf(line, sizeof(line), L"%d", get_expanded_line_num() + 1);
	SetWindowText(hWndTextboxLine, line);

	/* コマンド文字列を設定する */
	SetWindowText(hWndTextboxCommand, conv_utf8_to_utf16(get_line_string()));

	/* スクリプトのリストボックスを設定する */
	if(script_changed)
	{
		SendMessage(hWndListbox, LB_RESETCONTENT, 0 , 0);
		for(line_num = 0; line_num < get_line_count(); line_num++)
		{
			command = get_line_string_at_line_num(line_num);
			SendMessage(hWndListbox, LB_ADDSTRING, 0,
						(LPARAM)conv_utf8_to_utf16(command));
		}
	}
	line_num = get_expanded_line_num();
	top = (line_num - 9 < 0) ? 0 : (line_num - 9);
	SendMessage(hWndListbox, LB_SETCURSEL, (WPARAM)line_num, 0);
	SendMessage(hWndListbox, LB_SETTOPINDEX, (WPARAM)top, 0);

	/* 変数の情報を更新する */
	if(check_variable_updated() || script_changed)
		UpdateVariableTextBox();
}
