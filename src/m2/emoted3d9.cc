/* -*- coding: utf-8; tab-width: 4; indent-tabs-mode: t; -*- */

/*
 * E-mote support for Suika2 (Direct3D 9)
 * Copyrigth (c) 2023, Keiichi Tabata. All rights reserved.
 *
 * [Note]
 *  - emotedriver.dll and d3dx9_43.dll are needed
 *  - A trial version of emotedriver.dll is provided by M2 Co., Ltd. for free charge
 *  - A real version of emotedriver.dll will be provided by M2. Co., Ltd. after you signed an NDA
 *
 * [Disclamer]
 *  - E-mote is a registered trademark of M2 Co., Ltd.
 *  - E-mote is a proprietary technology of M2 Co., Ltd.
 *  - A Suika2 user who utilizes this E-mote support must acquire a license from M2 Co., Ltd.
 */

// Guard
#if defined(__GNUC__) || defined(__llvm__)
#error "We need MSVC to compile emoted3d9.cc for a C++ name mangling issue"
#endif

// Suika2 Base
extern "C" {
#include "../suika.h"
};

// E-mote
#include "iemote.h"
#pragma comment(lib, "emotedriver.lib")

// Windows
#include <windows.h>

// Direct3D 9
#include <d3d9.h>
extern "C" {
#include "../microsoft/d3drender.h"
};

//
// Variables
//

// E-mote Device
static IEmoteDevice *pEmoteDevice;

// E-mote Player
static IEmotePlayer *pEmotePlayer;

// Direct3D 9 Device
static LPDIRECT3DDEVICE9 pD3DDevice;

// Matrices for E-mote
static D3DMATRIX matWorld;
static D3DMATRIX matView;
static D3DMATRIX matProj;

// Start time
static float fStartTime;

//
// Forard Delcaration
//
static void device_lost_callback(void);

//
// Helpers to avoid using D3DX
//

static __inline D3DVECTOR normal(D3DVECTOR v)
{
	float len = sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);

	D3DVECTOR ret;
	ret.x = v.x / len;
	ret.y = v.y / len;
	ret.z = v.z / len;
	return ret;
}

static __inline D3DVECTOR cross(D3DVECTOR v1, D3DVECTOR v2)
{
	D3DVECTOR v;
	v.x = v1.y * v2.z - v1.z * v2.y;
	v.y = v1.z * v2.x - v1.x * v2.z;
	v.z = v1.x * v2.y - v1.y * v2.x;
	return v;
}

static __inline D3DVECTOR subtract(D3DVECTOR v1, D3DVECTOR v2)
{
	D3DVECTOR v;
	v.x = v1.x - v2.x;
	v.y = v1.y - v2.y;
	v.z = v1.z - v2.z;
	return v;
}

static __inline float dot(D3DVECTOR v1, D3DVECTOR v2)
{
	return v1.x * v2.x + v1.x * v2.y + v1.z * v2.z;
}

static __inline D3DMATRIX lookat(D3DVECTOR eye, D3DVECTOR at, D3DVECTOR up)
{
	D3DVECTOR zaxis = normal(subtract(at, eye));
	D3DVECTOR xaxis = normal(cross(up, zaxis));
	D3DVECTOR yaxis = cross(zaxis, xaxis);

	D3DMATRIX m;
	m._11 = xaxis.x;
	m._12 = yaxis.x;
	m._13 = zaxis.x;
	m._14 = 0;
	m._21 = xaxis.y;
	m._22 = -yaxis.y;
	m._23 = zaxis.y;
	m._24 = 0;
	m._31 = xaxis.z;
	m._32 = yaxis.z;
	m._33 = zaxis.z;
	m._34 = 0;
	m._41 = -dot(xaxis, eye);
	m._42 = -dot(yaxis, eye);
	m._43 = -dot(zaxis, eye);
	m._44 = 1;

	return m;
}

//
// A motion subsystem implementation
//

bool init_motion(void)
{
	// Check if we have d3dx9_43.dll
	HMODULE hModD3D = LoadLibrary(L"d3dx9_43.dll");
	if (hModD3D == NULL) {
		log_error("d3dx9_43.dll not found.");
		return false;
	}

	// Load emotedriver.dll
	HMODULE hModule = LoadLibrary(L"emotedriver.dll");
	if (hModule == NULL) {
		log_error("emotedriver.dll is missing.");
		return false;
	}

	// Load EmoteCreate().
	// We don't link emotedriver.dll.
	// emotedriver.dll is the sole plugin DLL for our E-mote support.
	IEmoteDevice *(*func_EmoteCreate)(const IEmoteDevice::InitParam &);
	func_EmoteCreate = (IEmoteDevice *(*)(const IEmoteDevice::InitParam &))GetProcAddress(hModule, "?EmoteCreate@@YAPAVIEmoteDevice@@ABUInitParam@1@@Z");
	if (func_EmoteCreate == NULL) {
		log_error("emotedriver.dll is invalid.");
		return false;
	}

	// Save a D3D device.
	pD3DDevice = (LPDIRECT3DDEVICE9)D3DGetDevice();

	// Create a world matrix for E-mote.
	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 4; j++)
			matWorld.m[i][j] = i == j ? 1.0f : 0.0f;

	// Create a projection matrix for E-mote.
	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 4; j++)
			matProj.m[i][j] = i == j ? 1.0f : 0.0f;
	matProj._11 = 1.0f / ((float)conf_window_width / (float)conf_window_height);
	matProj._34 = 1.0f;
	matProj._44 = 0.0f;
	matProj._41 = 0.0f;
	matProj._42 = 0.0f;

	// Create a view matrix.
	D3DVECTOR eye = {-0.5f, 0.5f, -(float)conf_window_height/2};
	D3DVECTOR at = {-0.5f, 0.5f, 0};
	D3DVECTOR up = {0, 1, 0};
	matView = lookat(eye, at, up);

	// Creaet an E-mote device.
    IEmoteDevice::InitParam param;
    memset(&param, 0, sizeof(param));
    param.d3dDevice = pD3DDevice;
    pEmoteDevice = func_EmoteCreate(param);
    pEmoteDevice->SetShaderModel(IEmoteDevice::SHADER_MODEL_2);

	// Set a device lost callback for Suika2 d3d subsystem.
	D3DSetDeviceLostCallback(device_lost_callback);

    return true;
}

bool load_motion(int index, const char *fname)
{
	// Open a file.
    struct rfile *rf = open_rfile(NULL, fname, false);
    if (rf == NULL)
        return false;

	// Get a file size.
	size_t size = get_rfile_size(rf);

	// Read a file.
    emote_uint32_t file_size[1] = {size};
    emote_uint8_t *file_data[1];
	file_data[0] = (emote_uint8_t *)malloc(size);
    if (file_data[0] == NULL) {
        log_memory();
		close_rfile(rf);
        return false;
    }
    size_t read_size = read_rfile(rf, file_data[0], size);
	if (read_size != size) {
        log_error("Read error.");
		close_rfile(rf);
        return false;
    }

	// Create an E-mote player.
    pEmoteDevice->CreatePlayer(1, (const emote_uint8_t **)&file_data, (const emote_uint32_t *)&file_size, &pEmotePlayer);

	// Free data.
    free(file_data[0]);

	// Show.
    pEmotePlayer->Show();

	// Get our start time.
    fStartTime = (float)GetTickCount();

    return true;
}

void update_motion(void)
{
    float ms = (float)GetTickCount() - fStartTime;

    pEmotePlayer->Progress(ms * 60.0f / 1000.0f);
}

void render_motion(void)
{
	// Save matrices of Suika2.
	D3DMATRIX saveWorld, saveView, saveProj;
	pD3DDevice->GetTransform(D3DTS_WORLD, &saveWorld);
	pD3DDevice->GetTransform(D3DTS_VIEW, &saveView);
	pD3DDevice->GetTransform(D3DTS_PROJECTION, &saveProj);

	// Set matrices for E-mote.
	pD3DDevice->SetTransform(D3DTS_WORLD, &matWorld);
	pD3DDevice->SetTransform(D3DTS_VIEW, &matView);
	pD3DDevice->SetTransform(D3DTS_PROJECTION, &matProj);

	// Set rendering states for E-mote.
	pD3DDevice->SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1);
	pD3DDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
	pD3DDevice->SetTextureStageState(0,D3DTSS_ALPHAARG1,D3DTA_TEXTURE);
	pD3DDevice->SetTextureStageState(0,D3DTSS_ALPHAARG2,D3DTA_DIFFUSE);
	pD3DDevice->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
	pD3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	pD3DDevice->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
	pD3DDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	pD3DDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	pD3DDevice->SetRenderState(D3DRS_BLENDOPALPHA, D3DBLENDOP_ADD);
	pD3DDevice->SetRenderState(D3DRS_DESTBLENDALPHA, D3DBLEND_ONE);
	pD3DDevice->SetRenderState(D3DRS_SRCBLENDALPHA, D3DBLEND_ONE);
	pD3DDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
	pD3DDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
	pD3DDevice->SetRenderState(D3DRS_ALPHAREF, 0x00);
	pD3DDevice->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE); 
	pD3DDevice->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATER);
	pD3DDevice->SetRenderState(D3DRS_SEPARATEALPHABLENDENABLE, TRUE);
	pD3DDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	pD3DDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
	pD3DDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	pD3DDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
	pD3DDevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
	pD3DDevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
	pD3DDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
	pD3DDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT);

	// Render.
    pEmotePlayer->Render();

	// Restore matrices of Suika2.
	pD3DDevice->SetTransform(D3DTS_WORLD, &saveWorld);
	pD3DDevice->SetTransform(D3DTS_VIEW, &saveView);
	pD3DDevice->SetTransform(D3DTS_PROJECTION, &saveProj);
}

void unload_motion(int index)
{
    if (pEmotePlayer != NULL) {
        pEmotePlayer->Release();
        pEmotePlayer = NULL;
    }
}

void set_motion_offset(int index, int offset_x, int offset_y)
{
	float x, y;
	pEmotePlayer->GetCoord(x, y);
	pEmotePlayer->SetCoord(x + offset_x, y + offset_y);
}

void set_motion_scale(int index, float scale)
{
	pEmotePlayer->SetScale(scale);
}

void set_motion_rotate(int index, float rot)
{
	pEmotePlayer->SetRot(rot);
}

static void device_lost_callback(void)
{
	if (pEmoteDevice != NULL)
		pEmoteDevice->OnDeviceLost();
}
