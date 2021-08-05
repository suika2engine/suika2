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

#include <d3d9.h>

// テクスチャなし座標変換済み頂点
struct VertexRHWColor {
	float x, y, z, rhw;
	DWORD color;
};

// テクスチャあり座標変換済み頂点
struct VertexRHWTex {
	float x, y, z, rhw, u, v;
};

// Direct3Dオブジェクト
static LPDIRECT3D9 pD3D;
static LPDIRECT3DDEVICE9 pD3DDevice;

//
// Direct3Dの初期化を行う
//
BOOL D3DInitialize(HWND hWnd)
{
	HRESULT hResult;

	// Direct3Dの作成を行う
	pD3D = Direct3DCreate9(D3D_SDK_VERSION);
	if(pD3D == NULL)
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
	hResult = pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd,
								 D3DCREATE_MIXED_VERTEXPROCESSING, &d3dpp,
								 &pD3DDevice);
    if(FAILED(hResult))
    {
        hResult = pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_REF,
									 hWnd, D3DCREATE_MIXED_VERTEXPROCESSING,
									 &d3dpp, &pD3DDevice);
		if(FAILED(hResult))
        {
			log_api_error("Direct3D::CreateDevice");
            return FALSE;
        }
    }

    return TRUE;
}

VOID D3DCleanup(void)
{
	if(pD3DDevice != NULL)
		pD3DDevice->Release();

	if(pD3D != NULL)
		pD3D->Release();
}

BOOL D3DLockTexture(int width, int height, pixel_t *pixels,
					pixel_t **locked_pixels, void **texture)
{
	assert(*locked_pixels == NULL);

	UNUSED_PARAMETER(pixels);

	IDirect3DTexture9 *pTex = NULL;
	if(*texture == NULL)
	{
		// テクスチャを作成する
		HRESULT hResult = pD3DDevice->CreateTexture(width, height, 1, 0,
													D3DFMT_A8R8G8B8,
													D3DPOOL_MANAGED, &pTex,
													NULL);
		if(FAILED(hResult))
		{
			log_api_error("Direct3DDevice9::CreateTexture");
			return FALSE;
		}
		*texture = pTex;
	}
	else
	{
		// 既存テクスチャを取得する
		pTex = (IDirect3DTexture9 *)*texture;
	}

	// テクスチャをロックする
	D3DLOCKED_RECT lockedRect;
	HRESULT hResult = pTex->LockRect(0, &lockedRect, NULL, 0);
	if(FAILED(hResult))
		return FALSE;
	*locked_pixels = (pixel_t *)lockedRect.pBits;

	// コピーする
	memcpy(*locked_pixels, pixels, width * height * sizeof(pixel_t));

	return TRUE;
}

VOID D3DUnlockTexture(int width, int height, pixel_t *pixels,
					  pixel_t **locked_pixels, void **texture)
{
	assert(*locked_pixels != NULL);
	assert(*texture != NULL);

	UNUSED_PARAMETER(width);
	UNUSED_PARAMETER(height);
	UNUSED_PARAMETER(pixels);

	// コピーする
	memcpy(pixels, *locked_pixels, width * height * sizeof(pixel_t));

	// アンロックする
	IDirect3DTexture9 *pTex = (IDirect3DTexture9 *)*texture;
	pTex->UnlockRect(0);

	*locked_pixels = NULL;
}

VOID D3DDestroyTexture(void *texture)
{
	if(texture != NULL) {
		IDirect3DTexture9 *pTex = (IDirect3DTexture9 *)texture;
		pTex->Release();
	}
}

VOID D3DStartFrame(void)
{
	pD3DDevice->Clear(0, NULL,
					  D3DCLEAR_TARGET | D3DCLEAR_STENCIL | D3DCLEAR_ZBUFFER,
					  D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0 );
	pD3DDevice->BeginScene();
}

VOID D3DEndFrame(void)
{
	pD3DDevice->EndScene();
	pD3DDevice->Present(NULL, NULL, NULL, NULL);
}

VOID D3DRenderImage(int dst_left, int dst_top,
					struct image * RESTRICT src_image, int width, int height,
					int src_left, int src_top, int alpha, int bt)
{
	IDirect3DTexture9 *pTex = (IDirect3DTexture9 *)get_texture_object(src_image);
	assert(pTex != NULL);

	UNUSED_PARAMETER(dst_left);
	UNUSED_PARAMETER(dst_top);
	UNUSED_PARAMETER(width);
	UNUSED_PARAMETER(height);
	UNUSED_PARAMETER(src_left);
	UNUSED_PARAMETER(src_top);
	UNUSED_PARAMETER(alpha);
	UNUSED_PARAMETER(bt);

	/* 描画の必要があるか判定する */
	if(alpha == 0 || width == 0 || height == 0)
		return;	/* 描画の必要がない*/
	if(!clip_by_source(get_image_width(src_image), get_image_height(src_image),
					   &width, &height, &dst_left, &dst_top, &src_left,
					   &src_top))
		return;	/* 描画範囲外 */
	if(!clip_by_dest(conf_window_width, conf_window_height, &width, &height,
					 &dst_left, &dst_top, &src_left, &src_top))
		return;	/* 描画範囲外 */

	float img_w = (float)get_image_width(src_image);
	float img_h = (float)get_image_height(src_image);

	VertexRHWTex vtx[4] = {
		{(float)dst_left, (float)dst_top, 0.0f, 1.0f, (float)src_left/img_w, (float)src_top/img_h},
		{(float)(dst_left + width), (float)dst_top, 0.0f, 1.0f, (float)(src_left + width)/img_w, (float)src_top/img_h},
		{(float)dst_left, (float)(dst_top + height), 0.0f, 1.0f, (float)src_left/img_w, (float)(src_top+height)/img_h},
		{(float)(dst_left + width), (float)(dst_top + height), 0.0f, 1.0f, (float)(src_left + width)/img_w, (float)(src_top + height)/img_h},
	};

	// VertexRHWTex vtx[4] = {
	// 	{(float)dst_left, (float)dst_top, 0.0f, 1.0f, 0.0f, 0.0f},
	// 	{(float)(dst_left + width), (float)dst_top, 0.0f, 1.0f, 1.0f, 0.0f},
	// 	{(float)dst_left, (float)(dst_top + height), 0.0f, 1.0f, 0.0f, 1.0f},
	// 	{(float)(dst_left + width), (float)(dst_top + height), 0.0f, 1.0f, 1.0f, 1.0f},
	// };

	// int posx = conf_window_width / 2;
	// int posy = conf_window_height / 2;
	// int w = 256;
	// int h = 256;
	// VertexRHWTex vtx[4]={
	// 	{ (float)(posx + w/2), (float)(posy - h/2), 0.0f, 1.0f, 1.0f, 0.0f},
	// 	{ (float)(posx + w/2), (float)(posy + h/2), 0.0f, 1.0f, 1.0f, 1.0f},
	// 	{ (float)(posx - w/2), (float)(posy - h/2), 0.0f, 1.0f, 0.0f, 0.0f},
	// 	{ (float)(posx - w/2), (float)(posy + h/2), 0.0f, 1.0f, 0.0f, 1.0f}
	// };

	if(bt != BLEND_NONE)
	{
		pD3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
		pD3DDevice->SetRenderState(D3DRS_SRCBLEND,  D3DBLEND_SRCALPHA);
		pD3DDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	}
	else
	{
		pD3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	}

	pD3DDevice->SetTexture(0, pTex);
	pD3DDevice->SetFVF(D3DFVF_XYZRHW | D3DFVF_TEX1);
	pD3DDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, vtx,
								sizeof(VertexRHWTex));
}

VOID D3DRenderImageMask(int dst_left, int dst_top,
						struct image * RESTRICT src_image, int width,
						int height, int src_left, int src_top, int mask)
{
	IDirect3DTexture9 *pTex =
		(IDirect3DTexture9 *)get_texture_object(src_image);
	assert(pTex != NULL);

	UNUSED_PARAMETER(dst_left);
	UNUSED_PARAMETER(dst_top);
	UNUSED_PARAMETER(width);
	UNUSED_PARAMETER(height);
	UNUSED_PARAMETER(src_left);
	UNUSED_PARAMETER(src_top);
	UNUSED_PARAMETER(mask);
}

VOID D3DRenderClear(int left, int top, int width, int height, pixel_t color)
{
	UNUSED_PARAMETER(left);
	UNUSED_PARAMETER(top);
	UNUSED_PARAMETER(width);
	UNUSED_PARAMETER(height);
	UNUSED_PARAMETER(color);

	VertexRHWColor vtx[4] = {
		{(float)left, (float)top, 0.0f, 1.0f, color},
		{(float)(left + width), (float)top, 0.0f, 1.0f, color},
		{(float)left, (float)(top + height), 0.0f, 1.0f, color},
		{(float)(left + width), (float)(top + height), 0.0f, 1.0f, color},
	};

	pD3DDevice->SetFVF(D3DFVF_XYZRHW | D3DFVF_DIFFUSE);
	pD3DDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, vtx, sizeof(VertexRHWColor));
}
