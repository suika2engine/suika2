/* -*- coding: utf-8; indent-tabs-mode: t; tab-width: 4; c-basic-offset: 4; -*- */

/*
 * Suika2
 * Copyright (C) 2001-2022, Keiichi Tabata. All rights reserved.
 */

/*
 * [Changes]
 *  - 2022/05/11 作成
 */

#ifndef SUIKA_DSVIDEO_H
#define SUIKA_DSVIDEO_H

#include "../suika.h"

#include <windows.h>

#define WM_GRAPHNOTIFY	(WM_APP + 13)

BOOL DShowPlayVideo(HWND hWnd, const char *pszFileName, int nOfsX, int nOfsY, int nWidth, int nHeight);
VOID DShowStopVideo(void);
BOOL DShowProcessEvent(void);

#endif
