/* -*- Coding: utf-8-with-signature; indent-tabs-mode: t; tab-width: 4; c-basic-offset: 4; -*- */

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
#define MSGBOX_TITLE		"Suika2 Pro for Creators"

/* バージョン文字列 */
#define VERSION				\
	"Suika2 Pro for Creators\n" \
	"Copyright (c) 2022, LUXION SOFT. All rights reserved.\n" \
	"\n" \
	"Developers:\n" \
	"TABATA Keiichi (Programmer)\n" \
	"MATSUNO Seiji (Supervisor)\n"

/* メニューを作成する */
VOID InitMenu(HWND hWnd);

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

#endif
