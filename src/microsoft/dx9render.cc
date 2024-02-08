/* -*- coding: utf-8; indent-tabs-mode: t; tab-width: 4; c-basic-offset: 4; -*- */

/*
 * Suika2
 * Copyright (C) 2001-2024, Keiichi Tabata. All rights reserved.
 */

/*
 * [Changes]
 *  2021-08-03 Created.
 */

extern "C" {
#include "../suika.h"
#include "dx9render.h"
};

#include <d3d9.h>

//
// パイプラインの種類
//
enum  {
	PIPELINE_NORMAL,
	PIPELINE_ADD,
	PIPELINE_DIM,
	PIPELINE_RULE,
	PIPELINE_MELT,
};

//
// 座標変換済み頂点の構造体
//  - 頂点シェーダを使わないため、変換済み座標を直接指定している
//
struct Vertex
{
	float x, y, z;	// (x, y, 0)
	float rhw;		// (1.0)
	DWORD color;	// (alpha, 1.0, 1.0, 1.0)
	float u1, v1;	// (u, v) of samplerColor
	float u2, v2;	// (u, v) of samplerRule
};

//
// Direct3Dオブジェクト
//
static LPDIRECT3D9 pD3D;
static LPDIRECT3DDEVICE9 pD3DDevice;
static IDirect3DPixelShader9 *pDimShader;
static IDirect3DPixelShader9 *pRuleShader;
static IDirect3DPixelShader9 *pMeltShader;

//
// レンダリング対象のウィンドウ
//
static HWND hMainWnd;

//
// オフセットとスケール
//
static float fDisplayOffsetX;
static float fDisplayOffsetY;
static float fScale;

//
// デバイスロスト時のコールバック
//
extern "C" {
void (*pDeviceLostCallback)(void);
};

//
// シェーダ
//

// Note: 頂点シェーダはなく、固定シェーダを使用している

// Note: "copy"パイプラインは固定シェーダで実行する

// Note: "normal"パイプラインは固定シェーダで実行する

// Note: "add"パイプラインは固定シェーダで実行する

//
// "dim"パイプラインのピクセルシェーダ
//
// dimShaderSrc[] =
//	"ps_1_4                                                    \n"
//	"                     // c0: the slot for dim factor       \n"
//	"texld r0, t0         // r0 = samplerColor;                \n"
//	"mul r0, r0, c0       // r0 *= c0;                         \n";
//                        // return r0;
//
static  unsigned char dimShaderBin[] {
	0x04, 0x01, 0xff, 0xff, 0x42, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x0f, 0x80, 0x00, 0x00, 0xe4, 0xb0,
	0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0x80,
	0x00, 0x00, 0xe4, 0x80, 0x00, 0x00, 0xe4, 0xa0,
	0xff, 0xff, 0x00, 0x00, 
};

//
// "rule"パイプラインは下記のピクセルシェーダ
//
// ruleShaderSrc[] =
//	"ps_1_4                                                             \n"
//	"def c0, 0, 0, 0, 0  // c0: zeros                                   \n"
//	"def c1, 1, 1, 1, 1  // c1: ones                                    \n"
//	"                    // c2: the slot for the threshould argument    \n"
//	"texld r0, t0        // r0 = samplerColor                           \n"
//	"texld r1, t1        // r1 = samplerRule                            \n"
//	"sub r1, r1, c2      // tmp = 1.0 - step(threshold, samplerRule);   \n"
//	"cmp r2, r1, c0, c1  // ...                                         \n"
//	"mov r0.a, r2.b      // samplerColor.a = tmp.b;                     \n";
//                       // return samplerColor;
//
static const unsigned char ruleShaderBin[] = {
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

//
// "melt"パイプラインのピクセルシェーダ
//
// meltShaderSrc[] =
//	"ps_1_4                                                                  \n"
//	"def c0, 0, 0, 0, 0	  // c0: zeros                                         \n"
//	"def c1, 1, 1, 1, 1   // c1: ones                                          \n"
//	                      // c2: the slot for the threshould argument          \n"
//	"texld r0, t0         // r0 = samplerColor                                 \n"
//	"texld r1, t1         // r1 = samplerRule                                  \n"
//	                      // tmp = (1.0 - rule) + (threshold * 2.0 - 1.0);     \n"
//	"add r2, c2, c2       //   ... <<r2 = progress * 2.0>>                     \n"
//	"sub r2, r2, r1       //   ... <<r2 = r2 - rule>>                          \n"
//	                      // tmp = clamp(tmp);                                 \n"
//	"cmp r2, r2, r2, c0   //   ... <<r2 = r2 > 0 ? r2 : 0>>                    \n"
//	"sub r3, c1, r2       //   ... <<r3 = 1.0 - r3>>                           \n"
//	"cmp r2, r3, r2, c1   //   ... <<r2 = r3 > 0 ? r2 : c1>>                   \n"
//	"mov r0.a, r2.b       // samplerRule.a = tmp.b;                            \n";
//                        // return samplerRule.a;
//
static const unsigned char meltShaderBin[] = {
	0x04, 0x01, 0xff, 0xff, 0x51, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x0f, 0xa0, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x51, 0x00, 0x00, 0x00, 
	0x01, 0x00, 0x0f, 0xa0, 0x00, 0x00, 0x80, 0x3f, 
	0x00, 0x00, 0x80, 0x3f, 0x00, 0x00, 0x80, 0x3f, 
	0x00, 0x00, 0x80, 0x3f, 0x42, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x0f, 0x80, 0x00, 0x00, 0xe4, 0xb0, 
	0x42, 0x00, 0x00, 0x00, 0x01, 0x00, 0x0f, 0x80, 
	0x01, 0x00, 0xe4, 0xb0, 0x02, 0x00, 0x00, 0x00, 
	0x02, 0x00, 0x0f, 0x80, 0x02, 0x00, 0xe4, 0xa0, 
	0x02, 0x00, 0xe4, 0xa0, 0x03, 0x00, 0x00, 0x00, 
	0x02, 0x00, 0x0f, 0x80, 0x02, 0x00, 0xe4, 0x80, 
	0x01, 0x00, 0xe4, 0x80, 0x58, 0x00, 0x00, 0x00, 
	0x02, 0x00, 0x0f, 0x80, 0x02, 0x00, 0xe4, 0x80, 
	0x02, 0x00, 0xe4, 0x80, 0x00, 0x00, 0xe4, 0xa0, 
	0x03, 0x00, 0x00, 0x00, 0x03, 0x00, 0x0f, 0x80, 
	0x01, 0x00, 0xe4, 0xa0, 0x02, 0x00, 0xe4, 0x80, 
	0x58, 0x00, 0x00, 0x00, 0x02, 0x00, 0x0f, 0x80, 
	0x03, 0x00, 0xe4, 0x80, 0x02, 0x00, 0xe4, 0x80, 
	0x01, 0x00, 0xe4, 0xa0, 0x01, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x08, 0x80, 0x02, 0x00, 0xaa, 0x80, 
	0xff, 0xff, 0x00, 0x00, 
};

//
// HLSLのサンプル(未使用, 今後の参考)
//
#if 0
// ブラーのピクセルシェーダ
static const char szBlurPixelShader[] =
	"texture tex0 : register(s0);                                          \n"
	"sampler2D s_2D;                                                       \n"
	"                                                                      \n"
	"float4 blur(float2 tex : TEXCOORD0, float4 dif : COLOR0) : COLOR      \n"
	"{                                                                     \n"
	"    float2 scale = dif.a / 200.0;                                     \n"
	"    float4 color = 0;                                                 \n"
	"    color += tex2D(s_2D, tex.xy + float2(-1.0, -1.0) * scale);        \n"
	"    color += tex2D(s_2D, tex.xy + float2(-1.0, 1.0) * scale);         \n"
	"    color += tex2D(s_2D, tex.xy + float2(1.0, -1.0) * scale);         \n"
	"    color += tex2D(s_2D, tex.xy + float2(1.0, 1.0) * scale);          \n"
	"    color += tex2D(s_2D, tex.xy + float2(-0.70711, 0.0) * scale);     \n"
	"    color += tex2D(s_2D, tex.xy + float2(0.0, 0.70711) * scale);      \n"
	"    color += tex2D(s_2D, tex.xy + float2(0.70711, 0) * scale);        \n"
	"    color += tex2D(s_2D, tex.xy + float2(0.0, -0.70711) * scale);     \n"
	"    color /= 8.0;                                                     \n"
	"    color.a = 1.0;                                                    \n"
	"    return color;                                                     \n"
	"}                                                                     \n";
#endif

//
// 前方参照
//
static VOID DrawPrimitives(int dst_left,
						   int dst_top,
						   int dst_width,
						   int dst_height,
						   struct image *src_image,
						   struct image *rule_image,
						   int src_left,
						   int src_top,
						   int src_width,
						   int src_height,
						   int alpha,
						   int pipeline);
static BOOL UploadTextureIfNeeded(struct image *img);

//
// Direct3Dの初期化を行う
//
BOOL D3DInitialize(HWND hWnd)
{
	HRESULT hResult;

	hMainWnd = hWnd;
	fDisplayOffsetX = 0.0f;
	fDisplayOffsetY = 0.0f;
	fScale = 1.0f;

	// Direct3Dの作成を行う
	pD3D = Direct3DCreate9(D3D_SDK_VERSION);
	if (pD3D == NULL)
    {
		log_api_error("Direct3DCreate9()");
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
		hResult = pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_SW,
									 hWnd, D3DCREATE_MIXED_VERTEXPROCESSING,
									 &d3dpp, &pD3DDevice);
		if (FAILED(hResult))
        {
			log_api_error("Direct3D::CreateDevice()");
			pD3D->Release();
			pD3D = NULL;
            return FALSE;
        }
    }

	// シェーダを作成する
	do {
		hResult = pD3DDevice->CreatePixelShader((DWORD *)dimShaderBin, &pDimShader);
		if (FAILED(hResult))
			break;

		hResult = pD3DDevice->CreatePixelShader((DWORD *)ruleShaderBin, &pRuleShader);
		if (FAILED(hResult))
			break;

		hResult = pD3DDevice->CreatePixelShader((DWORD *)meltShaderBin, &pMeltShader);
		if (FAILED(hResult))
			break;
	} while (0);
	if (FAILED(hResult))
	{
		log_api_error("Direct3DDevice9::CreatePixelShader()");
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
	if (pMeltShader != NULL)
	{
		pD3DDevice->SetPixelShader(NULL);
		pMeltShader->Release();
		pMeltShader = NULL;
	}
	if (pRuleShader != NULL)
	{
		pD3DDevice->SetPixelShader(NULL);
		pRuleShader->Release();
		pRuleShader = NULL;
	}
	if (pD3DDevice != NULL)
	{
		pD3DDevice->Release();
		pD3DDevice = NULL;
	}
	if (pD3D != NULL)
	{
		pD3D->Release();
		pD3D = NULL;
	}
}

//
// ウィンドウをリサイズする
//
BOOL D3DResizeWindow(int nOffsetX, int nOffsetY, float scale)
{
	fDisplayOffsetX = (float)nOffsetX;
	fDisplayOffsetY = (float)nOffsetY;
	fScale = scale;

	if (pD3DDevice != NULL)
	{
		// Direct3Dデバイスをリセットする
		D3DPRESENT_PARAMETERS d3dpp;
		ZeroMemory(&d3dpp, sizeof(d3dpp));
		d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
		d3dpp.BackBufferCount = 1;
		d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
		d3dpp.Windowed = TRUE;
		pD3DDevice->Reset(&d3dpp);
	}

	return TRUE;
}

//
// フレームの描画を開始する
//
VOID D3DStartFrame(void)
{
	// クリアする
	pD3DDevice->Clear(0,
					  NULL,
					  D3DCLEAR_TARGET,
					  D3DCOLOR_RGBA(0, 0, 0, 255),
					  0,
					  0);

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

	// 帯をクリアする
	RECT rcClient;
	GetClientRect(hMainWnd, &rcClient);
	if (fDisplayOffsetY > 0)
	{
		D3DRECT rc[2] = {
			{0, 0, rcClient.right, (LONG)fDisplayOffsetY},
			{0, (LONG)(rcClient.bottom - (int)fDisplayOffsetY), rcClient.right, rcClient.bottom}
		};			
		pD3DDevice->Clear(2, &rc[0], D3DCLEAR_TARGET, D3DCOLOR_RGBA(0, 0, 0, 255), 0, 0);
	}
	if (fDisplayOffsetX > 0)
	{
		D3DRECT rc[2] = {
			{0, 0, (LONG)fDisplayOffsetX, rcClient.bottom},
			{(LONG)(rcClient.right - (int)fDisplayOffsetX), 0, rcClient.right, rcClient.bottom}
		};			
		pD3DDevice->Clear(2, &rc[0], D3DCLEAR_TARGET, D3DCOLOR_RGBA(0, 0, 0, 255), 0, 0);
	}

	// 表示する
	if(pD3DDevice->Present(NULL, NULL, NULL, NULL) == D3DERR_DEVICELOST)
	{
		// Direct3Dデバイスがロストしている場合
		if(pD3DDevice->TestCooperativeLevel() == D3DERR_DEVICENOTRESET)
		{
			// Direct3Dデバイスをリセットする
			D3DPRESENT_PARAMETERS d3dpp;
			ZeroMemory(&d3dpp, sizeof(d3dpp));
			d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
			d3dpp.BackBufferCount = 1;
			d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
			d3dpp.Windowed = TRUE;
			pD3DDevice->Reset(&d3dpp);

			if (pDeviceLostCallback != NULL)
				pDeviceLostCallback();
		}
	}
}

//
// 前のフレームの内容で再描画を行う
//
BOOL D3DRedraw(void)
{
	if(pD3DDevice->Present(NULL, NULL, NULL, NULL) == D3DERR_DEVICELOST)
	{
		// Direct3Dデバイスがロストしている
		// リセット可能な状態になるまで、メッセージループを回す必要がある
		if(pD3DDevice->TestCooperativeLevel() == D3DERR_DEVICENOTRESET)
		{
			// Direct3Dデバイスをリセットする
			D3DPRESENT_PARAMETERS d3dpp;
			ZeroMemory(&d3dpp, sizeof(d3dpp));
			d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
			d3dpp.BackBufferCount = 1;
			d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
			d3dpp.Windowed = TRUE;
			pD3DDevice->Reset(&d3dpp);
			return FALSE;
		}
	}
	return TRUE;
}

//
// [参考]
//  - シェーダ言語(アセンブリ or HLSL)のコンパイル
//    - 実行時にコンパイルするにはd3dx9_43.dllのインストールが必要になる
//    - これがインストールされていなくても実行可能なようにしたい
//    - そこで、シェーダは開発者がコンパイルしてバイトコードをベタ書きする
//    - 下記コードでコンパイルを行って、shader.txtの内容を利用すること
//    - 開発中のみリンカオプションで-ld3dx9とする
//
#if 0
#include <d3dx9.h>

void CompileShader(const char *pSrc, unsigned char *pDst, BOOL bHLSL)
{
	ID3DXBuffer *pShader;
	ID3DXBuffer *pError;

	if (!bHLSL)
	{
		// For pixel shader assembly
		if (FAILED(D3DXAssembleShader(pSrc, strlen(pSrc), 0, NULL, 0,
									  &pShader, &pError)))
		{
			log_api_error("D3DXAssembleShader");

			LPSTR pszError = (LPSTR)pError->GetBufferPointer();
			if (pszError != NULL)
				log_error("%s", pszError);

			exit(1);
		}
	}
	else
	{
		// For pixel shader HLSL
		if (FAILED(D3DXCompileShader(pSrc, strlen(pSrc) - 1,
									 NULL, NULL, "blur", "ps_2_0", 0,
									 &pShader, &pError, NULL)))
		{
			log_api_error("D3DXCompileShader");

			LPSTR pszError = (LPSTR)pError->GetBufferPointer();
			if (pszError != NULL)
				log_error("%s", pszError);

			exit(1);
		}
	}

	FILE *fp;
	fp = fopen("shader.txt", "w");
	if (fp == NULL)
		exit(1);

	int size = pShader->GetBufferSize();
	unsigned char *p = (unsigned char *)pShader->GetBufferPointer();
	for (int i=0; i<size; i++) {
		pDst[i] = p[i];
		fprintf(fp, "0x%02x, ", p[i]);
		if (i % 8 == 7)
			fprintf(fp, "\n");
	}

	fclose(fp);
}
#endif

//
// HAL: Notifies an image update.
//  - TODO: Support lazy upload. (Probably I'll write for the Direct3D 12 support.)
//
void notify_image_update(struct image *img)
{
	img->need_upload = true;
}

//
// HAL: Notifies an image free.
//
void notify_image_free(struct image *img)
{
	IDirect3DTexture9 *pTex = (IDirect3DTexture9 *)img->texture;
	if(pTex == NULL)
		return;
	pTex->Release();
	img->texture = NULL;
}

//
// イメージをレンダリングする(normal)
//
void
render_image_normal(
	int dst_left,				/* The X coordinate of the screen */
	int dst_top,				/* The Y coordinate of the screen */
	int dst_width,				/* The width of the destination rectangle */
	int dst_height,				/* The width of the destination rectangle */
	struct image *src_image,	/* [IN] an image to be rendered */
	int src_left,				/* The X coordinate of a source image */
	int src_top,				/* The Y coordinate of a source image */
	int src_width,				/* The width of the source rectangle */
	int src_height,				/* The height of the source rectangle */
	int alpha)					/* The alpha value (0 to 255) */
{
	DrawPrimitives(dst_left,
				   dst_top,
				   dst_width,
				   dst_height,
				   src_image,
				   NULL,
				   src_left,
				   src_top,
				   src_width,
				   src_height,
				   alpha,
				   PIPELINE_NORMAL);
}

//
// イメージをレンダリングする(add)
//
void
render_image_add(
	int dst_left,				/* The X coordinate of the screen */
	int dst_top,				/* The Y coordinate of the screen */
	int dst_width,				/* The width of the destination rectangle */
	int dst_height,				/* The width of the destination rectangle */
	struct image *src_image,	/* [IN] an image to be rendered */
	int src_left,				/* The X coordinate of a source image */
	int src_top,				/* The Y coordinate of a source image */
	int src_width,				/* The width of the source rectangle */
	int src_height,				/* The height of the source rectangle */
	int alpha)					/* The alpha value (0 to 255) */
{
	DrawPrimitives(dst_left,
				   dst_top,
				   dst_width,
				   dst_height,
				   src_image,
				   NULL,
				   src_left,
				   src_top,
				   src_width,
				   src_height,
				   alpha,
				   PIPELINE_ADD);
}

//
// イメージをレンダリングする(dim)
//
void
render_image_dim(
	int dst_left,				/* The X coordinate of the screen */
	int dst_top,				/* The Y coordinate of the screen */
	int dst_width,				/* The width of the destination rectangle */
	int dst_height,				/* The height of the destination rectangle */
	struct image *src_image,	/* [IN] an image to be rendered */
	int src_left,				/* The X coordinate of a source image */
	int src_top,				/* The Y coordinate of a source image */
	int src_width,				/* The width of the source rectangle */
	int src_height,				/* The height of the source rectangle */
	int alpha)					/* The alpha value (0 to 255) */
{
	DrawPrimitives(dst_left,
				   dst_top,
				   dst_width,
				   dst_height,
				   src_image,
				   NULL,
				   src_left,
				   src_top,
				   src_width,
				   src_height,
				   alpha,
				   PIPELINE_DIM);
}

//
// 画面にイメージをルール付きでレンダリングする
//
void render_image_rule(struct image *src_image, struct image *rule_image, int threshold)
{
	DrawPrimitives(0, 0, -1, -1, src_image, rule_image, 0, 0, -1, -1, threshold, PIPELINE_RULE);
}

//
// 画面にイメージをルール付き(メルト)でレンダリングする
//
void render_image_melt(struct image *src_image, struct image *rule_image, int progress)
{
	DrawPrimitives(0, 0, -1, -1, src_image, rule_image, 0, 0, -1, -1, progress, PIPELINE_MELT);
}

// プリミティブを描画する
static VOID
DrawPrimitives(
	int dst_left,
	int dst_top,
	int dst_width,
	int dst_height,
	struct image *src_image,
	struct image *rule_image,
	int src_left,
	int src_top,
	int src_width,
	int src_height,
	int alpha,
	int pipeline)
{
	IDirect3DTexture9 *pTexColor = NULL;
	IDirect3DTexture9 *pTexRule = NULL;

	// テクスチャをアップロードする
	if (!UploadTextureIfNeeded(src_image))
		return;
	pTexColor = (IDirect3DTexture9 *)src_image->texture;
	if (rule_image != NULL) {
		if (!UploadTextureIfNeeded(rule_image))
			return;
		pTexRule = (IDirect3DTexture9 *)rule_image->texture;
	}

	// 描画の必要があるか判定する
	if (dst_width == 0 || dst_height == 0)
		return;	// 描画の必要がない

	if (dst_width == -1)
		dst_width = src_image->width;
	if (dst_height == -1)
		dst_height = src_image->height;
	if (src_width == -1)
		src_width = src_image->width;
	if (src_height == -1)
		src_height = src_image->height;

	float img_w = (float)src_image->width;
	float img_h = (float)src_image->height;

	Vertex v[4];

	// 左上
	v[0].x = (float)dst_left * fScale + fDisplayOffsetX - 0.5f;
	v[0].y = (float)dst_top * fScale + fDisplayOffsetY - 0.5f;
	v[0].z = 0.0f;
	v[0].rhw = 1.0f;
	v[0].u1 = (float)src_left / img_w;
	v[0].v1 = (float)src_top / img_h;
	v[0].u2 = v[0].u1;
	v[0].v2 = v[0].v1;
	v[0].color = D3DCOLOR_ARGB(alpha, 0xff, 0xff, 0xff);

	// 右上
	v[1].x = (float)dst_left * fScale + (float)dst_width * fScale - 1.0f + fDisplayOffsetX + 0.5f;
	v[1].y = (float)dst_top * fScale + fDisplayOffsetY - 0.5f;
	v[1].z = 0.0f;
	v[1].rhw = 1.0f;
	v[1].u1 = (float)(src_left + src_width) / img_w;
	v[1].v1 = (float)src_top / img_h;
	v[1].u2 = v[1].u1;
	v[1].v2 = v[1].v1;
	v[1].color = D3DCOLOR_ARGB(alpha, 0xff, 0xff, 0xff);

	// 左下
	v[2].x = (float)dst_left * fScale + fDisplayOffsetX - 0.5f;
	v[2].y = (float)dst_top * fScale + (float)dst_height * fScale - 1.0f + fDisplayOffsetY + 0.5f;
	v[2].z = 0.0f;
	v[2].rhw = 1.0f;
	v[2].u1 = (float)src_left / img_w;
	v[2].v1 = (float)(src_top + src_height) / img_h;
	v[2].u2 = v[2].u1;
	v[2].v2 = v[2].v1;
	v[2].color = D3DCOLOR_ARGB(alpha, 0xff, 0xff, 0xff);

	// 右下
	v[3].x = (float)dst_left * fScale + (float)dst_width * fScale - 1.0f + fDisplayOffsetX + 0.5f;
	v[3].y = (float)dst_top * fScale + (float)dst_height * fScale - 1.0f + fDisplayOffsetY + 0.5f;
	v[3].z = 0.0f;
	v[3].rhw = 1.0f;
	v[3].u1 = (float)(src_left + src_width) / img_w;
	v[3].v1 = (float)(src_top + src_height) / img_h;
	v[3].u2 = v[3].u1;
	v[3].v2 = v[3].v1;
	v[3].color = D3DCOLOR_ARGB(alpha, 0xff, 0xff, 0xff);

	FLOAT th = (float)alpha / 255.0f;
	FLOAT th4[4] = {th, th, th, th};

	switch (pipeline)
	{
	case PIPELINE_NORMAL:
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
		break;
	case PIPELINE_ADD:
		pD3DDevice->SetPixelShader(NULL);
		pD3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
		pD3DDevice->SetRenderState(D3DRS_SRCBLEND,  D3DBLEND_ONE);
		pD3DDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
		pD3DDevice->SetTextureStageState(0,	D3DTSS_COLORARG1, D3DTA_TEXTURE);
		pD3DDevice->SetTextureStageState(0,	D3DTSS_COLOROP, D3DTOP_MODULATE);
		pD3DDevice->SetTextureStageState(0,	D3DTSS_COLORARG2, D3DTA_DIFFUSE);
		pD3DDevice->SetTextureStageState(0,	D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
		pD3DDevice->SetTextureStageState(0,	D3DTSS_ALPHAOP, D3DTOP_MODULATE);
		pD3DDevice->SetTextureStageState(0,	D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
		break;
	case PIPELINE_DIM:
		pD3DDevice->SetPixelShader(pDimShader);
		pD3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
		pD3DDevice->SetRenderState(D3DRS_SRCBLEND,  D3DBLEND_SRCALPHA);
		pD3DDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
		th4[0] = th4[1] = th4[2] = 0.5f;
		pD3DDevice->SetPixelShaderConstantF(0, th4, 1);
		pD3DDevice->SetTextureStageState(0,	D3DTSS_COLORARG1, D3DTA_TEXTURE);
		pD3DDevice->SetTextureStageState(0,	D3DTSS_COLOROP, D3DTOP_MODULATE);
		pD3DDevice->SetTextureStageState(0,	D3DTSS_COLORARG2, D3DTA_DIFFUSE);
		pD3DDevice->SetTextureStageState(0,	D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
		pD3DDevice->SetTextureStageState(0,	D3DTSS_ALPHAOP, D3DTOP_MODULATE);
		pD3DDevice->SetTextureStageState(0,	D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
		break;
	case PIPELINE_RULE:
		pD3DDevice->SetPixelShader(pRuleShader);
		pD3DDevice->SetPixelShaderConstantF(2, th4, 1);
		pD3DDevice->SetTexture(1, pTexRule);
		pD3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
		pD3DDevice->SetRenderState(D3DRS_SRCBLEND,  D3DBLEND_SRCALPHA);
		pD3DDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
		break;
	case PIPELINE_MELT:
		pD3DDevice->SetPixelShader(pMeltShader);
		pD3DDevice->SetPixelShaderConstantF(2, th4, 1);
		pD3DDevice->SetTexture(1, pTexRule);
		pD3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
		pD3DDevice->SetRenderState(D3DRS_SRCBLEND,  D3DBLEND_SRCALPHA);
		pD3DDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
		break;
	default:
		assert(0);
		break;
	}

	// テクスチャ0を設定する
	pD3DDevice->SetTexture(0, pTexColor);
	pD3DDevice->SetFVF(D3DFVF_XYZRHW | D3DFVF_TEX2 | D3DFVF_DIFFUSE);

	// リニアフィルタを設定する
	pD3DDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	pD3DDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	pD3DDevice->SetSamplerState(1, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	pD3DDevice->SetSamplerState(1, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);

	// UVラッピングを設定する
	pD3DDevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
	pD3DDevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
	pD3DDevice->SetSamplerState(1, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
	pD3DDevice->SetSamplerState(1, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);

	// 描画する
	if(dst_width == 1 && dst_height == 1)
	{
		pD3DDevice->DrawPrimitiveUP(D3DPT_POINTLIST, 1, v, sizeof(Vertex));
	}
	else if(dst_width == 1)
	{
		pD3DDevice->DrawPrimitiveUP(D3DPT_LINELIST, 1, v + 1, sizeof(Vertex));
	}
	else if(dst_height == 1)
	{
		v[1].y += 1.0f;
		pD3DDevice->DrawPrimitiveUP(D3DPT_LINELIST, 1, v, sizeof(Vertex));
	}
	else
	{
		pD3DDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, v, sizeof(Vertex));
	}
}

// テクスチャのアップロードを行う
static BOOL UploadTextureIfNeeded(struct image *img)
{
	HRESULT hResult;

	if (!img->need_upload)
		return TRUE;

	IDirect3DTexture9 *pTex = (IDirect3DTexture9 *)img->texture;
	if (pTex == NULL)
	{
		// Direct3Dテクスチャオブジェクトを作成する
		hResult = pD3DDevice->CreateTexture((UINT)img->width,
											(UINT)img->height,
											1, // mip map levels
											0, // usage
											D3DFMT_A8R8G8B8,
											D3DPOOL_MANAGED,
											&pTex,
											NULL);
		if (FAILED(hResult))
			return FALSE;

		img->texture = pTex;
	}

	// Direct3Dテクスチャオブジェクトの矩形をロックする
	D3DLOCKED_RECT lockedRect;
	hResult = pTex->LockRect(0, &lockedRect, NULL, 0);
	if (FAILED(hResult))
	{
		pTex->Release();
		img->texture = NULL;
		return FALSE;
	}

	// ピクセルデータをコピーする
	memcpy(lockedRect.pBits, img->pixels, (UINT)img->width * (UINT)img->height * sizeof(pixel_t));

	// Direct3Dテクスチャオブジェクトの矩形をアンロックする
	hResult = pTex->UnlockRect(0);
	if (FAILED(hResult))
	{
		pTex->Release();
		img->texture = NULL;
		return FALSE;
	}

	// アップロード完了した
	img->need_upload = false;
	return TRUE;
}

VOID *D3DGetDevice(void)
{
	return pD3DDevice;
}

VOID D3DSetDeviceLostCallback(void (*pFunc)(void))
{
	pDeviceLostCallback = pFunc;
}
