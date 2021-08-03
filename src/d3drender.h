/* -*- coding: utf-8-with-signature; indent-tabs-mode: t; tab-width: 4; c-basic-offset: 4; -*- */

/*
 * Suika 2
 * Copyright (C) 2001-2016, TABATA Keiichi. All rights reserved.
 */

/*
 * [Changes]
 *  - 2021/08/03 作成
 */

#ifndef SUIKA_D3D_H
#define SUIKA_D3D_H

#include <windows.h>
#include <d3d9.h>

BOOL D3DInitialize(HWND hWnd);
VOID D3DCleanup(void);

#endif
