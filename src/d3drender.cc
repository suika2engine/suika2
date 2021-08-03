/* -*- coding: utf-8-with-signature; indent-tabs-mode: t; tab-width: 4; c-basic-offset: 4; -*- */

/*
 * Suika 2
 * Copyright (C) 2001-2021, TABATA Keiichi. All rights reserved.
 */

/*
 * [Changes]
 *  2021-08-03 作成
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "suika.h"
#include "d3drender.h"

#ifdef __cplusplus
};
#endif

/* Direct3Dオブジェクト */
static LPDIRECT3D9 pD3d;
static LPDIRECT3DDEVICE9 pDevice;

/* Direct3Dの初期化を行う */
BOOL D3DInitialize(HWND hWnd)
{
	HRESULT hResult;

	// Direct3Dの作成を行う
	pD3d = Direct3DCreate9(D3D_SDK_VERSION);
	if(pD3d == NULL)
    {
		log_api_error("Direct3DCreate9");
        return FALSE;
    }

    // Direct3Dデバイスを作成する
    D3DPRESENT_PARAMETERS d3dpp;
    ZeroMemory(&d3dpp, sizeof(d3dpp));
    d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
    d3dpp.BackBufferCount = 1;
    d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    d3dpp.Windowed = TRUE;
	hResult = pD3d->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd,
								 D3DCREATE_MIXED_VERTEXPROCESSING, &d3dpp,
								 &pDevice);
    if(hResult != S_OK)
    {
        hResult = pD3d->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_REF,
									 hWnd, D3DCREATE_MIXED_VERTEXPROCESSING,
									 &d3dpp, &pDevice);
		if(hResult != S_OK)
        {
			log_api_error("Direct3D::CreateDevice");
            return FALSE;
        }
    }

    return TRUE;
}

VOID D3DCleanup(void)
{
}
