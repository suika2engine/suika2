/* -*- coding: utf-8; indent-tabs-mode: t; tab-width: 4; c-basic-offset: 4; -*- */

/*
 * Suika 2
 * Copyright (C) 2001-2023, TABATA Keiichi. All rights reserved.
 */

/*
 * Debugger (Suika2 Pro)
 *
 * [Changes]
 *  2022-06-11 作成
 *  2023-09-20 エクスポート機能強化
 *  2023-09-21 縦幅縮小
 */

#ifndef SUIKA_WINDEBUG_H
#define SUIKA_WINDEBUG_H

#include <windows.h>

/* メッセージボックスのタイトル */
#define MSGBOX_TITLE		L"Suika2 Pro"

/* バージョン文字列 */
#define VERSION_EN			L"Suika2 Pro 13\n" \
							L"A part of the Suika Studio Professional Development Suite\n" \
							L"This product is guaranteed to be available as open source software in the future."
#define VERSION_JP			L"Suika2 Pro 13\n" \
							L"A part of the Suika Studio Professional Development Suite\n" \
							L"本製品は将来に渡ってオープンソースソフトウェアとして提供されることが保証されます。"

/* デバッガ用メニューを作成する */
VOID InitDebuggerMenu(HWND hWnd);

/* デバッガウィンドウを作成する */
BOOL InitDebuggerWindow(HINSTANCE hInstance, int nCmdShow);

/* スタートアップファイル/ラインを取得する */
BOOL GetStartupPosition(void);

/* デバッガのウィンドウハンドルであるかを返す */
BOOL IsDebuggerHWND(HWND hWnd);

/* デバッガのウィンドウプロシージャの処理 */
LRESULT CALLBACK WndProcDebugHook(HWND hWnd, UINT message, WPARAM wParam,
								  LPARAM lParam);

/* winmain.c */
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam,
						 LPARAM lParam);
const wchar_t *conv_utf8_to_utf16(const char *utf8_message);
const char *conv_utf16_to_utf8(const wchar_t *utf16_message);

#endif
