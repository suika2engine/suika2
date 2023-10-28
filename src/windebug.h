/* -*- coding: utf-8; indent-tabs-mode: t; tab-width: 4; c-basic-offset: 4; -*- */

/*
 * Suika2
 * Copyright (C) 2001-2023, Keiichi Tabata. All rights reserved.
 */

/*
 * Suika2 Pro
 *
 * [Changes]
 *  2022-06-11 作成
 *  2023-09-20 エクスポート機能強化
 *  2023-09-21 縦幅縮小
 *  2023-10-27 エディタ化
 */

#ifndef SUIKA_WINDEBUG_H
#define SUIKA_WINDEBUG_H

#include <windows.h>

/*
 * デバッガパネルのサイズの基本値
 */
#define DEBUGGER_PANEL_WIDTH		(440)
#define DEBUGGER_WIN_WIDTH_MIN		(500)
#define DEBUGGER_WIN_HEIGHT_MIN		(700)

/*
 * メッセージボックスのタイトル
 */
#define MSGBOX_TITLE	L"Suika2 Pro"

/*
 * バージョン文字列
 */
#define VERSION_EN \
	L"Suika2 Pro 14\n" \
	L"A part of the Suika Studio Professional Development Suite\n"		\
	L"This product is guaranteed to be available as an open source software in the future."
#define VERSION_JP \
	L"Suika2 Pro 14\n" \
	L"A part of the Suika Studio Professional Development Suite\n"		\
	L"本製品は将来に渡ってオープンソースソフトウェアとして提供されることが保証されます。"

/*
 * windebug.cからエクスポートされる関数
 */

/* コマンドライン引数がある場合にパッケージングを行う */
VOID DoPackagingIfArgExists(VOID);

/* スタートアップファイル/ラインを取得する */
BOOL GetStartupPosition(void);

/* DPI拡張の初期化を行う */
VOID InitDpiExtension(void);

/* DPIを取得する */
int Win11_GetDpiForWindow(HWND hWnd);

/* デバッガパネルを作成する */
BOOL InitDebuggerPanel(HWND hWndMain, HWND hWndGame, void *pWndProc);

/* デバッガパネルの位置を修正する */
VOID RearrangeDebuggerPanel(int nGameWidth, int nGameHeight);

/* メッセージのトランスレート前処理を行う */
BOOL PretranslateForDebugger(MSG *pMsg);

/* デバッガへのWM_COMMANDを処理する */
VOID OnCommandForDebugger(WPARAM wParam, LPARAM lParam);

/*
 * winmain.cからエクスポートされる関数
 */
const wchar_t *conv_utf8_to_utf16(const char *utf8_message);
const char *conv_utf16_to_utf8(const wchar_t *utf16_message);

#endif
