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
#endif

#include "suika.h"
#include "windebug.h"
#include "resource.h"

#include <commctrl.h>

/* UTF-8からSJISへの変換バッファサイズ */
#define NATIVE_MESSAGE_SIZE	(65536)

/* 変数テキストボックスのテキストの最大長(形: "$00001=12345678901\r\n") */
#define VAR_TEXTBOX_MAX		(11000 * (1 + 5 + 1 + 11 + 2))

/* メインウィンドウのハンドル */
static HWND hWndMain;

/* 英語モードか */
static BOOL bEnglish;

/* 実行状態 */
static BOOL bRunning;

/* メニュー */
static HMENU hMenu;

/* ステータスバー */
static HWND hWndStatus;

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

/* SJISからUTF-8への変換バッファ */
static char szUtf8Message[NATIVE_MESSAGE_SIZE];

/* 前方参照 */
static HWND CreateTooltip(HWND hWndBtn, const char *pszTextEnglish,
						  const char *pszTextJapanese);
static VOID OnClickListBox(void);
static VOID OnSelectScript(void);
static VOID OnPressReset(void);
static VOID OnPressSave(void);
static VOID OnPressError(void);
static VOID OnPressWriteVars(void);
static VOID UpdateVariableTextBox(void);
static const char *ConvNativeToUtf8(const char *lpszNativeMessage);

/*
 * メニューを作成する
 */
VOID InitMenu(HWND hWnd)
{
	HMENU hMenuFile = CreatePopupMenu();
	HMENU hMenuScript = CreatePopupMenu();
	HMENU hMenuHelp = CreatePopupMenu();
    MENUITEMINFO mi;

	hWndMain = hWnd;
	bEnglish = conf_language == NULL ? FALSE : TRUE;

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

/*
 * ステータスバーを作成する
 */
HWND InitStatusBar(void)
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
	SendMessage(hWndStatus, SB_SETPARTS, (WPARAM)3, (LPARAM)(LPINT)sizes);
	SendMessage(hWndStatus, SB_SETTEXT, (WPARAM)(0 | 0),
				(LPARAM)(bEnglish ? "Stopped" : "停止中"));
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

	return hWndStatus;
}

/*
 * デバッガウィンドウを作成する
 */
BOOL InitDebuggerWindow(HINSTANCE hInstance, int nCmdShow)
{
	const char szWndClass[] = "suikadebug";
	WNDCLASSEX wcex;
	RECT rc;
	HFONT hFont, hFontFixed;
	DWORD style;
	int dw, dh;
	const int WIN_WIDTH = 440;
	const int WIN_HEIGHT = 905;

	/* ウィンドウクラスを登録する */
	wcex.cbSize			= sizeof(WNDCLASSEX);
	wcex.style          = 0;
	wcex.lpfnWndProc    = WndProc;
	wcex.cbClsExtra     = 0;
	wcex.cbWndExtra     = 0;
	wcex.hInstance      = hInstance;
	wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SUIKA));
	wcex.hCursor        = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground  = (HBRUSH)(COLOR_BTNFACE + 1);
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

	/* ウィンドウのサイズを調整する */
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
				  "Reflect edited command for execution. ",
				  "編集したコマンドを実行に反映します。");

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
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | WS_BORDER | WS_VSCROLL |
		ES_AUTOVSCROLL | ES_MULTILINE | ES_WANTRETURN,
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
	ti.lpszText = (char *)(bEnglish ? pszTextEnglish : pszTextJapanese);
	GetClientRect(hWndBtn, &ti.rect);
	SendMessage(hWndTip, TTM_ADDTOOL, 0, (LPARAM)&ti);

	return hWndTip;
}

/*
 * デバッガ関連のウィンドウプロシージャの処理を行う
 */
LRESULT CALLBACK WndProcDebug(HWND hWnd, UINT message, WPARAM wParam,
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
			MessageBox(hWndMain, VERSION, MSGBOX_TITLE,
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
	ofn.lpstrFilter = bEnglish ?
		"Text Files\0*.txt;\0All Files(*.*)\0*.*\0\0" : 
		"テキストファイル\0*.txt;\0すべてのファイル(*.*)\0*.*\0\0";
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

	if (MessageBox(hWndMain, bEnglish  ?
				   "Are you sure you want to overwrite the script file?" :
				   "スクリプトファイルを上書き保存します。\n"
				   "よろしいですか？",
				   MSGBOX_TITLE, MB_ICONWARNING | MB_OKCANCEL) != IDOK)
		return;

	path = make_valid_path(SCRIPT_DIR, scr);

	fp = fopen(path, "w");
	if (fp == NULL)
	{
		free(path);
		MessageBox(hWndMain, bEnglish ?
				   "Cannot write to file." :
				   "ファイルに書き込めません。",
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
					   "Cannot write to file." :
					   "ファイルに書き込めません。",
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
			   "No error." : "エラーはありません。",
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

/*
 * ネイティブ文字コードのメッセージをUtf-8文字コードに変換する
 */
static const char *ConvNativeToUtf8(const char *lpszNativeMessage)
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
	bRunning = running;

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
		SendMessage(hWndTextboxVar, EM_SETREADONLY, TRUE, 0);

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
		SendMessage(hWndTextboxVar, EM_SETREADONLY, TRUE, 0);

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
		SendMessage(hWndTextboxVar, EM_SETREADONLY, FALSE, 0);

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
