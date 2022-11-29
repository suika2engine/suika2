/* -*- coding: utf-8; indent-tabs-mode: t; tab-width: 4; c-basic-offset: 4; -*- */

/*
 * Suika 2
 * Copyright (C) 2001-2021, TABATA Keiichi. All rights reserved.
 */

/*
 * Debugger (Suika2 Pro for Crerators)
 *
 * [Changes]
 *  2022-06-11 作成
 */

#ifndef SUIKA_WINDEBUG_H
#define SUIKA_WINDEBUG_H

#include <windows.h>

/* メッセージボックスのタイトル */
#define MSGBOX_TITLE		L"Suika2 Pro for Creators"

/* バージョン文字列 */
#define VERSION				\
	L"Suika2 Pro for Creators\n" \
	L"Copyright (c) 2022, the Suika2 Development Team. All rights reserved.\n"

/* デバッガ用メニューを作成する */
VOID InitDebuggerMenu(HWND hWnd);

/* デバッガウィンドウを作成する */
BOOL InitDebuggerWindow(HINSTANCE hInstance, int nCmdShow);

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
