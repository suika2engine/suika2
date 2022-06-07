/* -*- coding: utf-8-with-signature; indent-tabs-mode: t; tab-width: 4; c-basic-offset: 4; -*- */

/*
 * Suika 2
 * Copyright (C) 2001-2022, TABATA Keiichi. All rights reserved.
 */

/*
 * [Changes]
 *  - 2022/05/11 作成
 */

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

extern "C" {

#include "suika.h"
#include "dsvideo.h"

};

#include <dshow.h>

#define PATH_SIZE	(1024)

// メインウィンドウのハンドル
static HWND hWndMain;

// DirectShowのインタフェース
static IGraphBuilder *pBuilder;
static IMediaControl *pControl;
static IVideoWindow *pWindow;
static IMediaEventEx *pEvent;

static BOOL DisplayRenderFileErrorMessage(HRESULT hr);

//
// 動画を再生する
//
BOOL DShowPlayVideo(HWND hWnd, const char *pszFileName)
{
	HRESULT hRes;

	hWndMain = hWnd;

	// IGraphBuilderのインスタンスを取得する
	hRes = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER,
							IID_IGraphBuilder, (void**)&pBuilder);
	if(!pBuilder)
	{
		log_api_error("CoCreateInstance");
		return FALSE;
	}

	// フィルタグラフを生成する
	WCHAR wFileName[PATH_SIZE];
	mbstowcs(wFileName, pszFileName, PATH_SIZE);
	HRESULT hr = pBuilder->RenderFile(wFileName, NULL);
	if(!DisplayRenderFileErrorMessage(hr))
		return FALSE;

	// オーナーウィンドウを指定する
	pBuilder->QueryInterface(IID_IVideoWindow, (void **)&pWindow);
	if(pBuilder == NULL)
	{
		log_api_error("IGraphBuilder::QueryInterface");
		return FALSE;
	}
	pWindow->put_Owner((OAHWND)hWndMain);
	pWindow->put_WindowStyle(WS_CHILD | WS_CLIPSIBLINGS);
	pWindow->SetWindowPosition(0, 0, conf_window_width, conf_window_height);

	// hWndMainでイベントを取得するようにする
	pBuilder->QueryInterface(IID_IMediaEventEx, (void**)&pEvent);
	if(pEvent == NULL)
	{
		log_api_error("IGraphBuilder::QueryInterface");
		return FALSE;
	}
	pEvent->SetNotifyWindow((OAHWND)hWndMain, WM_GRAPHNOTIFY, 0);

	// ファイルを再生する
	pBuilder->QueryInterface(IID_IMediaControl, (void **)&pControl);
	if(pControl == NULL)
	{
		log_api_error("IGraphBuilder::QueryInterface");
		return FALSE;
	}
	pControl->Run();

	return TRUE;
}

static BOOL DisplayRenderFileErrorMessage(HRESULT hr)
{
	switch(hr)
	{
	case S_OK:
	case VFW_S_DUPLICATE_NAME:
		break;
	case VFW_S_AUDIO_NOT_RENDERED:
	case VFW_S_PARTIAL_RENDER:
	case VFW_S_VIDEO_NOT_RENDERED:
		log_video_error("Unsupported codec.");
		return FALSE;
	case E_ABORT:
	case E_FAIL:
	case E_INVALIDARG:
	case E_OUTOFMEMORY:
	case E_POINTER:
	case VFW_E_CANNOT_CONNECT:
	case VFW_E_CANNOT_LOAD_SOURCE_FILTER:
	case VFW_E_CANNOT_RENDER:
		log_video_error("Runtime error.");
		return FALSE;
	case VFW_E_INVALID_FILE_FORMAT:
		log_video_error("Invalid video file format.");
		return FALSE;
	case VFW_E_NOT_FOUND:
		log_video_error("File not found.");
		return FALSE;
	case VFW_E_UNKNOWN_FILE_TYPE:
		log_video_error("Unknown video file type.");
		return FALSE;
	case VFW_E_UNSUPPORTED_STREAM:
		log_video_error("Unsupported video stream.");
		return FALSE;
	default:
		log_video_error("Unknown video error.");
		return FALSE;
	}

	return TRUE;
}

//
// イベントを処理する(WndProcから呼ばれる)
//
BOOL DShowProcessEvent(void)
{
	assert(pEvent != NULL);

	// 再生完了イベントを取得する(それ以外のイベントは処理しない)
	LONG code, p, q;
	BOOL bContinue = TRUE;
	while(SUCCEEDED(pEvent->GetEvent(&code, &p, &q, 0)))
	{
		switch(code)
		{
		case EC_COMPLETE:
			// 再生完了イベント
			bContinue = FALSE;
			pEvent->SetNotifyWindow((OAHWND)NULL, 0, 0);
			break;
		}

		pEvent->FreeEventParams(code, p, q);
	}

	// 再生完了した場合、リソースを解放する
	if(!bContinue)
	{
		hWndMain = NULL;
		if(pBuilder)
		{
			pBuilder->Release();
			pBuilder = NULL;
		}
		if(pWindow)
		{
			pWindow->put_Visible(OAFALSE);
			pWindow->put_Owner((OAHWND)NULL);
			pWindow->Release();
			pWindow = NULL;
		}
		if(pControl)
		{
			pControl->Release();
			pControl = NULL;
		}
		if(pEvent)
		{
			pEvent->Release();
			pEvent = NULL;
		}
	}

	// 再生を続ける場合TRUE、終了した場合FALSEを返す
	return bContinue;
}
