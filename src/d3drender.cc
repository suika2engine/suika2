/* -*- coding: utf-8-with-signature; indent-tabs-mode: t; tab-width: 4; c-basic-offset: 4; -*- */

/*
 * Suika 2
 * Copyright (C) 2001-2021, TABATA Keiichi. All rights reserved.
 */

/*
 * [Changes]
 *  2021-08-03 作成
 */

extern "C" {

#include "suika.h"
#include "d3drender.h"

};

#include <d3d9.h>

// d3dx9_43.dllを不要にするため、シェーダを予めコンパイルするときに使う
// -ld3dx9をつけて使う
//#define COMPILE_SHADER

// テクスチャ管理用構造体
struct TextureListNode
{
	// Direct3Dのテクスチャオブジェクト
	IDirect3DTexture9 *pTex;	

	// リンクリスト
	TextureListNode *pNext;
};

// 座標変換済み頂点 (テクスチャ2枚)
struct VertexRHWTex
{
	float x, y, z, rhw;
	DWORD color;
	float u1, v1;
	float u2, v2;
};

// Direct3Dオブジェクト
static LPDIRECT3D9 pD3D;
static LPDIRECT3DDEVICE9 pD3DDevice;
static IDirect3DPixelShader9 *pRuleShader;

// テクスチャリストの先頭
static TextureListNode *pTexList;

// 全画面用の表示オフセット
static int nDisplayOffsetX;
static int nDisplayOffsetY;

#ifdef COMPILE_SHADER
// ルール付き描画のピクセルシェーダ
const char szRulePixelShader[] =
	"ps_1_4               \n"
	"def c0, 0, 0, 0, 0   \n"
	"def c1, 1, 1, 1, 1   \n"
    "texld r0, t0         \n"
    "texld r1, t1         \n"
	"sub r1, r1, c2       \n"
	"cmp r2, r1, c0, c1   \n"
    "mov r0.a, r2.b       \n";

unsigned char ruleShaderBin[1024];
#else
// コンパイル済みのシェーダバイナリ
const unsigned char ruleShaderBin[] = {
	0x04, 0x01, 0xff, 0xff, 0x51, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x0f, 0xa0, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x51, 0x00, 0x00, 0x00,
	0x01, 0x00, 0x0f, 0xa0, 0x00, 0x00, 0x80, 0x3f,
	0x00, 0x00, 0x80, 0x3f, 0x00, 0x00, 0x80, 0x3f,
	0x00, 0x00, 0x80, 0x3f, 0x42, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x0f, 0x80, 0x00, 0x00, 0xe4, 0xb0,
	0x42, 0x00, 0x00, 0x00, 0x01, 0x00, 0x0f, 0x80,
	0x01, 0x00, 0xe4, 0xb0, 0x03, 0x00, 0x00, 0x00,
	0x01, 0x00, 0x0f, 0x80, 0x01, 0x00, 0xe4, 0x80,
	0x02, 0x00, 0xe4, 0xa0, 0x58, 0x00, 0x00, 0x00,
	0x02, 0x00, 0x0f, 0x80, 0x01, 0x00, 0xe4, 0x80,
	0x00, 0x00, 0xe4, 0xa0, 0x01, 0x00, 0xe4, 0xa0,
	0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x80,
	0x02, 0x00, 0xaa, 0x80, 0xff, 0xff, 0x00, 0x00,
};
#endif

// 前方参照
static VOID DestroyDirect3DTextureObjects();
static VOID DrawPrimitives(int dst_left, int dst_top,
						   struct image * RESTRICT src_image,
						   struct image * RESTRICT rule_image,
						   int width, int height,
						   int src_left, int src_top,
						   int alpha, int bt);
#ifdef COMPILE_SHADER
static void CompileShader();
#endif

//
// Direct3Dの初期化を行う
//
BOOL D3DInitialize(HWND hWnd)
{
	HRESULT hResult;

	// Direct3Dの作成を行う
	pD3D = Direct3DCreate9(D3D_SDK_VERSION);
	if (pD3D == NULL)
    {
		log_info("Direct3DCreate9() failed.");
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
    if (FAILED(hResult))
    {
		hResult = pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_REF,
									 hWnd, D3DCREATE_MIXED_VERTEXPROCESSING,
									 &d3dpp, &pD3DDevice);
		if (FAILED(hResult))
        {
			log_info("Direct3D::CreateDevice() failed.");
			pD3D->Release();
			pD3D = NULL;
            return FALSE;
        }
    }

	// ピクセルシェーダのコンパイルを行う
#ifdef COMPILE_SHADER
	// d3dx9_43.dllのある環境でコンパイルして、ruleShaderBinに転写する
	CompileShader();
#endif

	// シェーダを作成する
	if (FAILED(pD3DDevice->CreatePixelShader((DWORD *)ruleShaderBin,
											 &pRuleShader)))
	{
		log_info("Direct3DDevice9::CreatePixelShader() failed.");
		pD3DDevice->Release();
		pD3DDevice = NULL;
		pD3D->Release();
		pD3D = NULL;
		return FALSE;
	}

    return TRUE;
}

//
// Direct3Dの終了処理を行う
//
VOID D3DCleanup(void)
{
	// すべてのDirect3Dテクスチャオブジェクトを破棄する
	DestroyDirect3DTextureObjects();

	// ピクセルシェーダを破棄する
	if (pRuleShader != NULL)
	{
		pD3DDevice->SetPixelShader(NULL);
		pRuleShader->Release();
		pRuleShader = NULL;
	}

	// Direct3Dデバイスを破棄する
	if (pD3DDevice != NULL)
	{
		pD3DDevice->Release();
		pD3DDevice = NULL;
	}

	// Direct3Dオブジェクトを破棄する
	if (pD3D != NULL)
	{
		pD3D->Release();
		pD3D = NULL;
	}
}

//
// フルスクリーン表示用のオフセットを設定する
//
BOOL D3DSetDisplayOffset(int nOffsetX, int nOffsetY)
{
	nDisplayOffsetX = nOffsetX;
	nDisplayOffsetY = nOffsetY;

	return TRUE;
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
		TextureListNode *t = (TextureListNode *)
			malloc(sizeof(TextureListNode));
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

	TextureListNode *t = (TextureListNode *)*texture;
	if(t->pTex == NULL)
	{
		// Direct3Dテクスチャオブジェクトを作成する
		// TODO: managedでなくする
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
	TextureListNode *t = (TextureListNode *)texture;
	if(t->pTex != NULL)
		t->pTex->Release();

	// テクスチャ管理用オブジェクトのリストから外す
	TextureListNode *p = pTexList;
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
	TextureListNode *t = pTexList;
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
	// クリアする
	pD3DDevice->Clear(0, NULL,
					  D3DCLEAR_TARGET | D3DCLEAR_STENCIL | D3DCLEAR_ZBUFFER,
					  D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);

	// 描画を開始する
	pD3DDevice->BeginScene();
}

//
// フレームの描画を終了する
//
VOID D3DEndFrame(void)
{
	// 描画を完了する
	pD3DDevice->EndScene();

	// 表示する
	if(pD3DDevice->Present(NULL, NULL, NULL, NULL) == D3DERR_DEVICELOST)
	{
		// Direct3Dデバイスがロストしている
		// リセット可能な状態になるまで、メッセージループを回す必要がある
		if(pD3DDevice->TestCooperativeLevel() != D3DERR_DEVICENOTRESET)
			return;

		// Direct3Dデバイスをリセットする
		D3DPRESENT_PARAMETERS d3dpp;
		ZeroMemory(&d3dpp, sizeof(d3dpp));
		d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
		d3dpp.BackBufferCount = 1;
		d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
		d3dpp.Windowed = TRUE;
		pD3DDevice->Reset(&d3dpp);
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
	DrawPrimitives(dst_left, dst_top, src_image, NULL, width, height,
				   src_left, src_top, alpha, bt);
}

//
// イメージをルール付きでレンダリングする
// (render_image_rule()のDirect3D版実装)
//
VOID D3DRenderImageRule(struct image * RESTRICT src_image,
						struct image * RESTRICT rule_image,
						int threshold)
{
	DrawPrimitives(0, 0, src_image, rule_image,
				   get_image_width(src_image), get_image_height(src_image),
				   0, 0, threshold, BLEND_NONE);
}

// プリミティブを描画する
static VOID DrawPrimitives(int dst_left, int dst_top,
						   struct image * RESTRICT src_image,
						   struct image * RESTRICT rule_image,
						   int width, int height,
						   int src_left, int src_top,
						   int alpha, int bt)
{
	// ソース画像のテクスチャを取得する
	TextureListNode *src_tex = (TextureListNode *)get_texture_object(src_image);
	assert(src_tex != NULL);

	// ルール画像のテクスチャを取得する
	TextureListNode *rule_tex = NULL;
	if (rule_image != NULL)
	{
		rule_tex = (TextureListNode *)get_texture_object(rule_image);
		assert(rule_tex != NULL);
	}

	// 描画の必要があるか判定する
	if(width == 0 || height == 0)
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
	v[0].x = (float)(dst_left + nDisplayOffsetX) - 0.5f;
	v[0].y = (float)(dst_top + nDisplayOffsetY) - 0.5f;
	v[0].z = 0.0f;
	v[0].rhw = 1.0f;
	v[0].u1 = (float)src_left / img_w;
	v[0].v1 = (float)src_top / img_h;
	v[0].u2 = v[0].u1;
	v[0].v2 = v[0].v1;
	v[0].color = D3DCOLOR_ARGB(alpha, 0xff, 0xff, 0xff);

	// 右上
	v[1].x = (float)(dst_left + width - 1 - nDisplayOffsetX) + 0.5f;
	v[1].y = (float)(dst_top + nDisplayOffsetY) - 0.5f;
	v[1].z = 0.0f;
	v[1].rhw = 1.0f;
	v[1].u1 = (float)(src_left + width) / img_w;
	v[1].v1 = (float)src_top / img_h;
	v[1].u2 = v[1].u1;
	v[1].v2 = v[1].v1;
	v[1].color = D3DCOLOR_ARGB(alpha, 0xff, 0xff, 0xff);

	// 左下
	v[2].x = (float)(dst_left + nDisplayOffsetX) - 0.5f;
	v[2].y = (float)(dst_top + height - 1 - nDisplayOffsetY) + 0.5f;
	v[2].z = 0.0f;
	v[2].rhw = 1.0f;
	v[2].u1 = (float)src_left / img_w;
	v[2].v1 = (float)(src_top + height) / img_h;
	v[2].u2 = v[2].u1;
	v[2].v2 = v[2].v1;
	v[2].color = D3DCOLOR_ARGB(alpha, 0xff, 0xff, 0xff);

	// 右下
	v[3].x = (float)(dst_left + width - 1 - nDisplayOffsetX) + 0.5f;
	v[3].y = (float)(dst_top + height - 1 - nDisplayOffsetY) + 0.5f;
	v[3].z = 0.0f;
	v[3].rhw = 1.0f;
	v[3].u1 = (float)(src_left + width) / img_w;
	v[3].v1 = (float)(src_top + height) / img_h;
	v[3].u2 = v[3].u1;
	v[3].v2 = v[3].v1;
	v[3].color = D3DCOLOR_ARGB(alpha, 0xff, 0xff, 0xff);

	if (rule_image == NULL && bt != BLEND_NONE)
	{
		// ブレンドしない場合
		pD3DDevice->SetPixelShader(NULL);
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
	else if (rule_image == NULL)
	{
		// ブレンドする場合
		pD3DDevice->SetPixelShader(NULL);
		pD3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	}
	else
	{
		// ルールシェーダを使用する場合
		FLOAT th = (float)alpha / 255.0f;
		FLOAT th4[4] = {th, th, th, th};
		pD3DDevice->SetPixelShader(pRuleShader);
		pD3DDevice->SetPixelShaderConstantF(2, th4, 1);
		pD3DDevice->SetTexture(1, rule_tex->pTex);
		pD3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
		pD3DDevice->SetRenderState(D3DRS_SRCBLEND,  D3DBLEND_SRCALPHA);
		pD3DDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	}

	pD3DDevice->SetTexture(0, src_tex->pTex);
	pD3DDevice->SetFVF(D3DFVF_XYZRHW | D3DFVF_TEX2 | D3DFVF_DIFFUSE);

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
		v[1].y += 1.0f;
		pD3DDevice->DrawPrimitiveUP(D3DPT_LINELIST, 1, v,
									sizeof(VertexRHWTex));
	}
	else
	{
		pD3DDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, v,
									sizeof(VertexRHWTex));
	}
}

#ifdef COMPILE_SHADER
#include <d3dx9.h>
#include "log.h"

void CompileShader()
{
	ID3DXBuffer *pShader;
	ID3DXBuffer *pError;

	if (FAILED(D3DXAssembleShader(szRulePixelShader,
								  sizeof(szRulePixelShader) - 1,
								  0, NULL, 0, &pShader, &pError)))
	{
		log_api_error("D3DXAssembleShader");

		LPSTR pszError = (LPSTR)pError->GetBufferPointer();
		if (pszError != NULL)
			log_error("%s", pszError);

		exit(1);
	}

	FILE *fp;
	fp = fopen("shader.txt", "w");
	if (fp == NULL)
		exit(1);

	int size = pShader->GetBufferSize();
	unsigned char *p = (unsigned char *)pShader->GetBufferPointer();
	for (int i=0; i<size; i++) {
		ruleShaderBin[i] = p[i];
		fprintf(fp, "0x%02x, ", p[i]);
		if (i % 8 == 7)
			fprintf(fp, "\n");
	}

	fclose(fp);
}
#endif
