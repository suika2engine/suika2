/* -*- coding: utf-8; indent-tabs-mode: t; tab-width: 4; c-basic-offset: 4; -*- */

/*
 * Suika 2
 * Copyright (C) 2001-2016, TABATA Keiichi. All rights reserved.
 */

/*
 * DirectSound ミキサ (DirectSound 5.0)
 *
 * [Changes]
 *  - 2004/01/10 作成 (class LXPcmInputStreamを入力)
 *  - 2016/06/05 作成 (struct waveを入力, C++からCに書き換え)
 *
 * dxguid.lib, dsound.libが必要
 */

#ifndef SUIKA_DSOUND_H
#define SUIKA_DSOUND_H

#include <windows.h>

BOOL DSInitialize(HWND hWnd);
VOID DSCleanup();

#endif
