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

// テクスチャ管理用構造体
struct Texture
{
	// Direct3Dのテクスチャオブジェクト
	IDirect3DTexture9 *pTex;	

	// リンクリスト
	Texture *pNext;
};

// テクスチャなし座標変換済み頂点
struct VertexRHW
{
	float x, y, z, rhw;
	DWORD color;
};

// テクスチャあり座標変換済み頂点
struct VertexRHWTex
{
	float x, y, z, rhw;
	DWORD color;
	float u, v;
};

// Direct3Dオブジェクト
static LPDIRECT3D9 pD3D;
static LPDIRECT3DDEVICE9 pD3DDevice;

// テクスチャリストの先頭
static Texture *pTexList;

// 全画面用の表示オフセット
static int nDisplayOffsetX;
static int nDisplayOffsetY;

// 前方参照
static VOID DestroyDirect3DTextureObjects();

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

//
// Direct3Dの再初期化を行う
//
BOOL D3DReinitialize(int nOffsetX, int nOffsetY)
{
	HRESULT hResult;

	// すべてのDirect3Dテクスチャオブジェクトを破棄する
//	DestroyDirect3DTextureObjects();

	// Direct3Dデバイスをリセットする
	D3DPRESENT_PARAMETERS d3dpp;
	ZeroMemory(&d3dpp, sizeof(d3dpp));
	d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
	d3dpp.BackBufferCount = 1;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.Windowed = TRUE;
	hResult = pD3DDevice->Reset(&d3dpp);
    if(FAILED(hResult))
		return FALSE;

	nDisplayOffsetX = nOffsetX;
	nDisplayOffsetY = nOffsetY;

	return TRUE;
}

//
// Direct3Dの終了処理を行う
//
VOID D3DCleanup(void)
{
	// すべてのDirect3Dテクスチャオブジェクトを破棄する
	DestroyDirect3DTextureObjects();

	// Direct3Dデバイスを破棄する
	if(pD3DDevice != NULL)
	{
		pD3DDevice->Release();
		pD3DDevice = NULL;
	}

	// Direct3Dオブジェクトを破棄する
	if(pD3D != NULL)
	{
		pD3D->Release();
		pD3D = NULL;
	}
}

//
// テクスチャをロックする
// (lock_texture()のDirect3D版実装)
//
BOOL D3DLockTexture(int width, int height, pixel_t *pixels,
					pixel_t **locked_pixels, void **texture)
{
	assert(*locked_pixels == NULL);

	UNUSED_PARAMETER(width);
	UNUSED_PARAMETER(height);
	UNUSED_PARAMETER(pixels);

	if(*texture == NULL)
	{
		// テクスチャ管理用オブジェクトを作成する
		Texture *t = (Texture *)malloc(sizeof(Texture));
		if(t == NULL)
			return FALSE;

		// 初期化してリストの先頭に加える
		t->pTex = NULL;
		t->pNext = pTexList;
		pTexList = t;

		// ポインタを設定する
		*texture = t;
	}

	// ロック中の描画先ポインタを設定する
	*locked_pixels = pixels;

	return TRUE;
}

//
// テクスチャをアンロックする
// (unlock_texture()のDirect3D版実装)
//
BOOL D3DUnlockTexture(int width, int height, pixel_t *pixels,
					  pixel_t **locked_pixels, void **texture)
{
	assert(*locked_pixels != NULL);
	assert(*texture != NULL);

	UNUSED_PARAMETER(width);
	UNUSED_PARAMETER(height);
	UNUSED_PARAMETER(pixels);

	Texture *t = (Texture *)*texture;
	if(t->pTex == NULL)
	{
		// Direct3Dテクスチャオブジェクトを作成する
		HRESULT hResult = pD3DDevice->CreateTexture(width, height, 1, 0,
													D3DFMT_A8R8G8B8,
													D3DPOOL_MANAGED,
													&t->pTex,
													NULL);
		if(FAILED(hResult))
		{
			log_api_error("Direct3DDevice9::CreateTexture");
			return FALSE;
		}
	}

	// Direct3Dテクスチャオブジェクトの矩形をロックする
	D3DLOCKED_RECT lockedRect;
	HRESULT hResult = t->pTex->LockRect(0, &lockedRect, NULL, 0);
	if(FAILED(hResult))
		return FALSE;

	// ピクセルデータをコピーする
	memcpy(lockedRect.pBits, *locked_pixels, width * height * sizeof(pixel_t));

	// Direct3Dテクスチャオブジェクトの矩形をアンロックする
	t->pTex->UnlockRect(0);

	// ロック中の描画先ポインタをクリアする
	*locked_pixels = NULL;

	return TRUE;
}

//
// テクスチャを破棄する
// (destroy_texture()のDirect3D版実装)
//
VOID D3DDestroyTexture(void *texture)
{
	// テクスチャ管理用オブジェクトが作成されていなければ何もしない
	if(texture == NULL)
		return;

	// Direct3Dテクスチャオブジェクトを破棄する
	Texture *t = (Texture *)texture;
	if(t->pTex != NULL)
		t->pTex->Release();

	// テクスチャ管理用オブジェクトのリストから外す
	Texture *p = pTexList;
	if(p == t)
	{
		pTexList = t->pNext;
	}
	else
	{
		while(p->pNext != NULL)
		{
			if(p->pNext == t)
			{
				p->pNext = t->pNext;
				break;
			}
			p = p->pNext;
		}
	}

	// テクスチャ管理用オブジェクトを破棄する
	free(t);
}

// すべてのDirect3Dテクスチャオブジェクトを破棄する
static VOID DestroyDirect3DTextureObjects()
{
	Texture *t = pTexList;
	while(t != NULL)
	{
		if(t->pTex != NULL)
		{
			t->pTex->Release();
			t->pTex = NULL;
		}
		t = t->pNext;
	}
}

//
// フレームの描画を開始する
//
VOID D3DStartFrame(void)
{
	pD3DDevice->Clear(0, NULL,
					  D3DCLEAR_TARGET | D3DCLEAR_STENCIL | D3DCLEAR_ZBUFFER,
					  D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);
	pD3DDevice->BeginScene();
}

//
// フレームの描画を終了する
//
VOID D3DEndFrame(void)
{
	pD3DDevice->EndScene();
	if(pD3DDevice->Present(NULL, NULL, NULL, NULL) == D3DERR_DEVICELOST)
	{
		if(pD3DDevice->TestCooperativeLevel() != D3DERR_DEVICENOTRESET)
			return;

		D3DReinitialize();		
	}
}

//
// イメージをレンダリングする
// (render_image()のDirect3D版実装)
//
VOID D3DRenderImage(int dst_left, int dst_top,
					struct image * RESTRICT src_image, int width, int height,
					int src_left, int src_top, int alpha, int bt)
{
	struct Texture *tex = (struct Texture *)get_texture_object(src_image);
	assert(tex != NULL);

	// 描画の必要があるか判定する
	if(alpha == 0 || width == 0 || height == 0)
		return;	// 描画の必要がない
	if(!clip_by_source(get_image_width(src_image), get_image_height(src_image),
					   &width, &height, &dst_left, &dst_top, &src_left,
					   &src_top))
		return;	// 描画範囲外
	if(!clip_by_dest(conf_window_width, conf_window_height, &width, &height,
					 &dst_left, &dst_top, &src_left, &src_top))
		return;	// 描画範囲外

	float img_w = (float)get_image_width(src_image);
	float img_h = (float)get_image_height(src_image);

	VertexRHWTex v[4];

	// 左上
	v[0].x = (float)(dst_left + nDisplayOffsetX);
	v[0].y = (float)(dst_top + nDisplayOffsetY);
	v[0].z = 0.0f;
	v[0].rhw = 1.0f;
	v[0].u = (float)src_left / img_w;
	v[0].v = (float)src_top / img_h;
	v[0].color = D3DCOLOR_ARGB(alpha, 0xff, 0xff, 0xff);

	// 右上
	v[1].x = (float)(dst_left + width - 1 - nDisplayOffsetX);
	v[1].y = (float)(dst_top + nDisplayOffsetY);
	v[1].z = 0.0f;
	v[1].rhw = 1.0f;
	v[1].u = (float)(src_left + width) / img_w;
	v[1].v = (float)src_top / img_h;
	v[1].color = D3DCOLOR_ARGB(alpha, 0xff, 0xff, 0xff);

	// 左下
	v[2].x = (float)(dst_left + nDisplayOffsetX);
	v[2].y = (float)(dst_top + height - 1 - nDisplayOffsetY);
	v[2].z = 0.0f;
	v[2].rhw = 1.0f;
	v[2].u = (float)src_left / img_w;
	v[2].v = (float)(src_top + height) / img_h;
	v[2].color = D3DCOLOR_ARGB(alpha, 0xff, 0xff, 0xff);

	// 右下
	v[3].x = (float)(dst_left + width - 1 - nDisplayOffsetX);
	v[3].y = (float)(dst_top + height - 1 - nDisplayOffsetY);
	v[3].z = 0.0f;
	v[3].rhw = 1.0f;
	v[3].u = (float)(src_left + width) / img_w;
	v[3].v = (float)(src_top + height) / img_h;
	v[3].color = D3DCOLOR_ARGB(alpha, 0xff, 0xff, 0xff);

	if(bt != BLEND_NONE)
	{
		pD3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
		pD3DDevice->SetRenderState(D3DRS_SRCBLEND,  D3DBLEND_SRCALPHA);
		pD3DDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
		pD3DDevice->SetTextureStageState(0,	D3DTSS_COLORARG1, D3DTA_TEXTURE);
		pD3DDevice->SetTextureStageState(0,	D3DTSS_COLOROP, D3DTOP_MODULATE);
		pD3DDevice->SetTextureStageState(0,	D3DTSS_COLORARG2, D3DTA_DIFFUSE);
		pD3DDevice->SetTextureStageState(0,	D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
		pD3DDevice->SetTextureStageState(0,	D3DTSS_ALPHAOP, D3DTOP_MODULATE);
		pD3DDevice->SetTextureStageState(0,	D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
	}
	else
	{
		pD3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	}

	pD3DDevice->SetTexture(0, tex->pTex);
	pD3DDevice->SetFVF(D3DFVF_XYZRHW | D3DFVF_TEX1 | D3DFVF_DIFFUSE);

	if(width == 1 && height == 1)
	{
		pD3DDevice->DrawPrimitiveUP(D3DPT_POINTLIST, 1, v,
									sizeof(VertexRHWTex));
	}
	else if(width == 1)
	{
		pD3DDevice->DrawPrimitiveUP(D3DPT_LINELIST, 1, v + 1,
									sizeof(VertexRHWTex));
	}
	else if(height == 1)
	{
		pD3DDevice->DrawPrimitiveUP(D3DPT_LINELIST, 1, v,
									sizeof(VertexRHWTex));
	}
	else
	{
		pD3DDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, v,
									sizeof(VertexRHWTex));
	}
}

//
// イメージをマスク描画でレンダリングする
// (render_image_mask()のDirect3D版実装)
//
VOID D3DRenderImageMask(int dst_left, int dst_top,
						struct image * RESTRICT src_image, int width,
						int height, int src_left, int src_top, int mask)
{
	int a = (int)(255.0f * ((float)mask / 27.0f));
	D3DRenderImage(dst_left, dst_top, src_image, width, height, src_left,
				   src_top, a, BLEND_NONE);
}

//
// 矩形をクリアする
// (render_clear()のDirect3D版実装)
//
VOID D3DRenderClear(int left, int top, int width, int height, pixel_t color)
{
	UNUSED_PARAMETER(left);
	UNUSED_PARAMETER(top);
	UNUSED_PARAMETER(width);
	UNUSED_PARAMETER(height);
	UNUSED_PARAMETER(color);

/*
	D3DCOLOR d3dcolor = D3DCOLOR_ARGB(get_pixel_a(color),
									  get_pixel_r(color),
									  get_pixel_g(color),
									  get_pixel_b(color));

	VertexRHW v[4];

	// 左上
	v[0].x = (float)left;
	v[0].y = (float)top;
	v[0].z = 0.0f;
	v[0].rhw = 1.0f;
	v[0].color = d3dcolor;
	v[1].x = (float)(left + width);
	v[1].y = (float)top;
	v[1].z = 0.0f;
	v[1].rhw = 1.0f;
	v[1].color = d3dcolor;
	v[2].x = (float)left;
	v[2].y = (float)(top + height);
	v[2].z = 0.0f;
	v[2].rhw = 1.0f;
	v[2].color = d3dcolor;
	v[3].x = (float)(left + width);
	v[3].y = (float)(top + height);
	v[3].z = 0.0f;
	v[3].rhw = 1.0f;
	v[3].color = d3dcolor;

	pD3DDevice->SetFVF(D3DFVF_XYZRHW | D3DFVF_DIFFUSE);
	pD3DDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, v, sizeof(VertexRHW));
*/
}
