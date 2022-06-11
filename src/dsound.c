/* -*- coding: utf-8-with-signature; indent-tabs-mode: t; tab-width: 4; c-basic-offset: 4; -*- */

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

#include <windows.h>
#include <process.h>

#include <math.h>

#include "suika.h"

#define DIRECTSOUND_VERSION 0x0500 /* Windows 98 default */
#include <dsound.h>

/*
 * バッファのフォーマット
 */
#define BIT_DEPTH			(16)
#define SAMPLES_PER_SEC		(44100)
#define CHANNELS			(2)
#define BYTES_PER_SAMPLE	(4)

/*
 * リングバッファ(1秒分)のバッファサイズ
 */
#define BUF_SAMPLES			(44100)
#define BUF_BYTES			(SAMPLES_PER_SEC * BIT_DEPTH / 8 * CHANNELS)

/*
 * バッファを更新する単位となる分割数とサイズ
 */
#define BUF_AREAS			(4)
#define AREA_SAMPLES		(BUF_SAMPLES / BUF_AREAS)
#define AREA_BYTES			(BUF_BYTES / BUF_AREAS)

/*
 * DirectSoundのオブジェクト
 */
static LPDIRECTSOUND pDS;
static LPDIRECTSOUNDBUFFER pDSBuffer[MIXER_STREAMS];
static LPDIRECTSOUNDNOTIFY pDSNotify[MIXER_STREAMS];
static WAVEFORMATEX wfPrimary;

/*
 * 再生用のイベント通知スレッド
 */
static HANDLE hEventThread;

/*
 * スレッド間通信用のイベントハンドル
 */
static HANDLE hNotifyEvent[MIXER_STREAMS];
static HANDLE hQuitEvent;

/*
 * 各チャネルの入力ストリームと
 * pStreamへアクセスする際に取得するクリティカルセクション
 */
static struct wave *pStream[MIXER_STREAMS];
static CRITICAL_SECTION	StreamCritical;

/*
 * 各チャネルの再生終了フラグ
 */
static BOOL bFinish[MIXER_STREAMS];

/*
 * 各ストリームの現在の更新エリア
 *  - 初回の更新の際は-1となる
 */
static int nPosCurArea[MIXER_STREAMS];

/*
 * 各ストリームが再生を終了するエリア
 *  - ストリーム終端に達するまでは-1となる
 */
static int nPosEndArea[MIXER_STREAMS];

/*
 * Internal functions
 */
static BOOL CreatePrimaryBuffer();
static BOOL CreateSecondaryBuffers();
static BOOL RestoreBuffers(int nBuffer);
static BOOL PlaySoundBuffer(int nBuffer, struct wave *pStr);
static VOID StopSoundBuffer(int nBuffer);
static BOOL SetBufferVolume(int nBuffer, float Vol);
static BOOL WriteNext(int nBuffer);
static VOID EventThread(VOID *);
static VOID OnNotifyPlayPos(int nBuffer);

/*
 * dsound.hの実装
 */

/*
 * DirectSoundを初期化する
 */
BOOL DSInitialize(HWND hWnd)
{
	HRESULT hRet;
	uintptr_t t;

	/* IDirectSoundのインスタンスを取得して初期化する */
	hRet = CoCreateInstance(&CLSID_DirectSound,
							NULL,
							CLSCTX_INPROC_SERVER,
							&IID_IDirectSound,
							(void **)&pDS);
	if(hRet != S_OK || pDS == NULL)
		return FALSE;

	/* COMオブジェクトを初期化する */
	IDirectSound_Initialize(pDS, NULL);

	/* 協調レベルを設定する */
	hRet = IDirectSound_SetCooperativeLevel(pDS, hWnd, DSSCL_PRIORITY);
	if(hRet != DS_OK)
		return FALSE;

	/* プライマリバッファのフォーマットを設定する */
	if(!CreatePrimaryBuffer())
		return FALSE;

	/* セカンダリバッファを作成する */
	if(!CreateSecondaryBuffers())
		return FALSE;

	/* イベントスレッドへの終了通知用のイベントを作成する */
	hQuitEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	if(hQuitEvent == NULL)
		return FALSE;

	/* DirectSoundの再生位置通知を受け取るスレッドを開始する */
	t = _beginthread(EventThread, 0, NULL);
	if(t == (unsigned long)-1)
		return FALSE;

	hEventThread = (HANDLE)t;
	return TRUE;
}

/*
 * DirectSoundの使用を終了する
 */
VOID DSCleanup()
{
	int i;

	/* イベントスレッドの終了を待つ */
	if(hQuitEvent != NULL)
	{
		/* 終了を通知する*/
		SetEvent(hQuitEvent);

		// スレッドの終了を待つ
		if(hEventThread != NULL)
			WaitForSingleObject(hEventThread, 1000*30);

		CloseHandle(hQuitEvent);

		/*
		 * _beginthread()で作成したスレッドのハンドルhEventThreadは、
		 * CloseHandle()で閉じては*いけない*。
		 */
	}

	/* クリティカルセクションを削除する */
	DeleteCriticalSection(&StreamCritical);

	/* セカンダリバッファと通知イベントを解放する */
	for(i=0; i<MIXER_STREAMS; i++)
	{
		if(pDSNotify[i] != NULL)
		{
			IDirectSoundNotify_Release(pDSNotify[i]);
			pDSNotify[i] = NULL;
		}
		if(pDSBuffer[i] != NULL)
		{
			IDirectSoundBuffer_Release(pDSBuffer[i]);
			pDSBuffer[i] = NULL;
		}
		if(hNotifyEvent[i] != NULL)
		{
			CloseHandle(hNotifyEvent[i]);
			hNotifyEvent[i] = NULL;
		}
	}

	/* DirectSoundオブジェクトを解放する */
	if(pDS != NULL)
	{
		IDirectSound_Release(pDS); 
		pDS = NULL;
	}
}

/*
 * platform.hの実装
 */

/*
 * 指定したストリーム上で再生する
 */
bool play_sound(int stream, struct wave *w)
{
	assert(pDS != NULL);
	assert(stream >= 0 && stream < MIXER_STREAMS);
	assert(w != NULL);

	/* ストリームが再生中の場合は停止する */
	StopSoundBuffer(stream);

	/* バッファを再生する */
	PlaySoundBuffer(stream, w);
	return true;
}

/*
 * 指定チャネルの再生を停止する
 */
bool stop_sound(int stream)
{
	assert(pDS != NULL);
	assert(stream >= 0 && stream < MIXER_STREAMS);

	/* バッファが再生中の場合は停止する */
	StopSoundBuffer(stream);

	return true;
}

/*
 * チャネルのボリュームをセットする
 */
bool set_sound_volume(int stream, float vol)
{
	assert(pDS != NULL);
	assert(stream >= 0 && stream < MIXER_STREAMS);

	return SetBufferVolume(stream, vol);
}

/*
 * サウンドが再生中か調べる
 */
bool is_sound_finished(int stream)
{
    if (bFinish[stream])
        return true;

    return false;
}

/*
 * 内部関数
 */

/*
 * プライマリバッファを作成してフォーマットを設定する
 */
static BOOL CreatePrimaryBuffer()
{
	DSBUFFERDESC dsbd;
	LPDIRECTSOUNDBUFFER pDSPrimary;
	HRESULT hRet;

	ZeroMemory(&dsbd, sizeof(DSBUFFERDESC));
	dsbd.dwSize = sizeof(DSBUFFERDESC);
	dsbd.dwFlags = DSBCAPS_PRIMARYBUFFER;
	dsbd.dwBufferBytes = 0;
	dsbd.lpwfxFormat = NULL;

	/* プライマリバッファを作成する */
	hRet = IDirectSound_CreateSoundBuffer(pDS, &dsbd, &pDSPrimary, NULL);
	if(hRet != DS_OK)
		return FALSE;

	/* プライマリバッファのフォーマットを設定する */
	ZeroMemory(&wfPrimary, sizeof(WAVEFORMATEX));
	wfPrimary.wFormatTag = WAVE_FORMAT_PCM;
	wfPrimary.nChannels = CHANNELS;
	wfPrimary.nSamplesPerSec = SAMPLES_PER_SEC;
	wfPrimary.wBitsPerSample = BIT_DEPTH;
	wfPrimary.nBlockAlign = (WORD)(wfPrimary.wBitsPerSample / 8 *
							wfPrimary.nChannels);
	wfPrimary.nAvgBytesPerSec = wfPrimary.nSamplesPerSec *
								wfPrimary.nBlockAlign;
	hRet = IDirectSoundBuffer_SetFormat(pDSPrimary, &wfPrimary);

	/* プライマリバッファにはアクセスしないので解放してよい */
	IDirectSoundBuffer_Release(pDSPrimary);

	return hRet == DS_OK;
}

/*
 * セカンダリバッファを作成し、再生位置通知を有効にする
 */
static BOOL CreateSecondaryBuffers()
{
	DSBPOSITIONNOTIFY pn[4];
	DSBUFFERDESC dsbd;
	HRESULT hRet;
	int i;

	memset(&dsbd, 0, sizeof(DSBUFFERDESC));
	dsbd.dwSize = sizeof(DSBUFFERDESC);
	dsbd.dwFlags = DSBCAPS_CTRLPOSITIONNOTIFY |	 /* 再生位置通知を利用可能にする */
				   DSBCAPS_GETCURRENTPOSITION2 | /* GetCurrentPositon()で正確な再生位置を取得可能にする */
				   DSBCAPS_GLOBALFOCUS |         /* 非アクティブでも再生する */
				   DSBCAPS_CTRLVOLUME;           /* ボリュームを設定可能にする */
	dsbd.dwBufferBytes = BUF_BYTES;
	dsbd.lpwfxFormat = &wfPrimary;

	for(i=0; i<MIXER_STREAMS; i++)
	{
		// セカンダリバッファを作成する
		hRet = IDirectSound_CreateSoundBuffer(pDS, &dsbd, &pDSBuffer[i], NULL);
		if(hRet != DS_OK)
			return FALSE;

		// 再生位置通知を利用する準備を行う
		hRet = IDirectSoundBuffer_QueryInterface(pDSBuffer[i],
												 &IID_IDirectSoundNotify,
												 (VOID**)&pDSNotify[i]);
		if(hRet != S_OK)
			return FALSE;

		hNotifyEvent[i] = CreateEvent(NULL, FALSE, FALSE, NULL);
		if(hNotifyEvent[i] == NULL)
			return FALSE;

		// 通知位置を設定する
		pn[0].dwOffset = 0;
		pn[0].hEventNotify = hNotifyEvent[i];
		pn[1].dwOffset = AREA_BYTES;
		pn[1].hEventNotify = hNotifyEvent[i];
		pn[2].dwOffset = AREA_BYTES * 2;
		pn[2].hEventNotify = hNotifyEvent[i];
		pn[3].dwOffset = AREA_BYTES * 3;
		pn[3].hEventNotify = hNotifyEvent[i];
		hRet = IDirectSoundNotify_SetNotificationPositions(pDSNotify[i],
														   4,
														   pn);
		if(hRet != DS_OK)
			return FALSE;
    }

	InitializeCriticalSection(&StreamCritical);
	return TRUE;
}

/*
 * バッファをリストアする
 */
static BOOL RestoreBuffers(int nBuffer)
{
	DWORD dwStatus;
	HRESULT hRet;

	assert(pDSBuffer[nBuffer] != NULL);
	assert(nBuffer >= 0 && nBuffer < MIXER_STREAMS);

	hRet = IDirectSoundBuffer_GetStatus(pDSBuffer[nBuffer], &dwStatus);
	if(hRet != DS_OK)
		return FALSE;
	if(dwStatus & DSBSTATUS_BUFFERLOST)
    {
		/*
		 * アプリケーションがアクティブになったばかりなので
		 * DirectSoundのコントロールを取得できない可能性がある。
		 * よって、コントロールを取得できるまでスリープする。
		 */
		while(1) {
			Sleep(10);
            hRet = IDirectSoundBuffer_Restore(pDSBuffer[nBuffer]);
            if(hRet != DSERR_BUFFERLOST)
				break;
		}
    }
    return TRUE;
}

/*
 * バッファを再生する
 */
static BOOL PlaySoundBuffer(int nBuffer, struct wave *pStr)
{
	HRESULT hRet;
	int i;

	assert(pDSBuffer[nBuffer] != NULL);
	assert(pStream[nBuffer] == NULL);
	assert(nBuffer >= 0 && nBuffer < MIXER_STREAMS);

	/* バッファがロストしていれば修復する */
	if(!RestoreBuffers(nBuffer))
		return FALSE;

	/* 再生終了フラグをクリアする */
	bFinish[nBuffer] = FALSE;

	/*
	 * イベントスレッドと排他制御する
	 *  - 停止->再生が即座に行われた場合、停止前の通知で再生後のバッファリング
	 *    が行われる恐れがあるので、きちんと排他制御する
	 */
	EnterCriticalSection(&StreamCritical);
	{
		/* チャネルのストリームをセットする */
		pStream[nBuffer] = pStr;

		/* 終了領域を未定とする */
		nPosEndArea[nBuffer] = -1;

		/* バッファいっぱいに読み込む */
		nPosCurArea[nBuffer] = 0;
		for(i=0; i<BUF_AREAS; i++)
			WriteNext(nBuffer);
	}
	LeaveCriticalSection(&StreamCritical);

	/* バッファを再生する */
	hRet = IDirectSoundBuffer_Play(pDSBuffer[nBuffer],
								   0,
								   0,
								   DSBPLAY_LOOPING);
	if(hRet != DS_OK)
		return FALSE;

	return TRUE;
}

/*
 * バッファの再生を停止する
 */
static VOID StopSoundBuffer(int nBuffer)
{
	assert(pDSBuffer[nBuffer] != NULL);
	assert(nBuffer >= 0 && nBuffer < MIXER_STREAMS);

	/* イベントスレッドと排他制御する */
	EnterCriticalSection(&StreamCritical);
	{
		/* バッファが再生中であれば */
		if(pStream[nBuffer] != NULL)
		{
			/* 再生を停止する */
			IDirectSoundBuffer_Stop(pDSBuffer[nBuffer]);
			IDirectSoundBuffer_SetCurrentPosition(pDSBuffer[nBuffer], 0);

			/* ストリームを停止状態にする */
			pStream[nBuffer] = NULL;
		}
	}
	LeaveCriticalSection(&StreamCritical);
}

/*
 * バッファのボリュームを設定する
 */
BOOL SetBufferVolume(int nBuffer, float Vol)
{
	LONG dB;

	Vol = (Vol > 1.0f) ? 1.0f : Vol;
	Vol = (Vol < 0.0f) ? 0.0f : Vol;

	/*
	 * 減衰率の対数(dB)を求める
	 *  - 0dB(DSBVOLIME_MAX=0)が原音、-100dB(DSBVOLUME_MIN=-10000)が無音
	 *  - 100倍固定少数なので100倍する
     */
	if(Vol <= 0.00001f)
		dB = DSBVOLUME_MIN;
	else
		dB = (LONG)(20.0f * (float)log10(Vol) * 100.0f);

	assert(dB >= DSBVOLUME_MIN && dB <= DSBVOLUME_MAX);

	/* バッファのボリュームをセットする */
	IDirectSoundBuffer_SetVolume(pDSBuffer[nBuffer], (LONG)dB);
	return TRUE;
}

/*
 * PCMストリームからバッファにデータを読み込む
 *  - イベントスレッドからクリティカルセクション内で呼び出されるので注意
 */
static BOOL WriteNext(int nBuffer)
{
	VOID *pBuf[2];
	DWORD dwLockedBytes[2];
	DWORD dwOffset;
	HRESULT hRet;
	int nArea, nSamples;

	assert(nBuffer >= 0 && nBuffer < MIXER_STREAMS);
	assert(nPosCurArea[nBuffer] >= 0 && nPosCurArea[nBuffer] < BUF_AREAS);

	/* 再生が終了した領域(=書き込みする領域)を取得してインクリメントする */
	nArea = nPosCurArea[nBuffer];
	nPosCurArea[nBuffer] = (nPosCurArea[nBuffer] + 1) % BUF_AREAS;

	/* バッファをロックする */
	dwOffset = (DWORD)nArea * AREA_BYTES;
	hRet = IDirectSoundBuffer_Lock(pDSBuffer[nBuffer],
								   dwOffset,
								   AREA_BYTES,
								   &pBuf[0],
								   &dwLockedBytes[0],
								   &pBuf[1],
								   &dwLockedBytes[1],
								   0);
	switch(hRet)
	{
	case DS_OK:
		assert(pBuf[1] == NULL && dwLockedBytes[1] == 0);
		break;
	case DSERR_BUFFERLOST:
		/* バッファをリストアして再度ロックする */
		if(!RestoreBuffers(nBuffer))
			return FALSE;
		hRet = IDirectSoundBuffer_Lock(pDSBuffer[nBuffer],
									   dwOffset,
									   AREA_BYTES,
									   &pBuf[0],
									   &dwLockedBytes[0],
									   &pBuf[1],
									   &dwLockedBytes[1],
									   0);
		if (hRet != DS_OK)
			return FALSE;
		break;
	default:
		return FALSE;
	}

	/* 入力データがEOSに達していない場合 */
	if(nPosEndArea[nBuffer] == -1)
	{
		/* PCMストリームからバッファにコピーする */
		nSamples = get_wave_samples(pStream[nBuffer], (uint32_t *)pBuf[0],
									AREA_SAMPLES);

		/* 入力が終端に達した場合 */
		if(nSamples == 0)
		{
			/* 再生終了位置を記憶する */
			nPosEndArea[nBuffer] = nArea == 0 ? BUF_AREAS - 1 : nArea - 1;
		}
		else if(nSamples != AREA_SAMPLES)
		{
			/* バッファの残りをゼロクリアする */
			ZeroMemory((char*)pBuf[0] + nSamples * BYTES_PER_SAMPLE,
					   (size_t)(AREA_SAMPLES - nSamples) * BYTES_PER_SAMPLE);

			/* 再生終了位置を記憶する */
			nPosEndArea[nBuffer] = nArea;
		}
	}
	else 
	{
		/* 入力データがEOSに達している場合、領域をゼロクリアする */
		ZeroMemory(pBuf[0], AREA_BYTES);
	}

	/* バッファをアンロックする */
	hRet = IDirectSoundBuffer_Unlock(pDSBuffer[nBuffer],
									 pBuf[0],
									 dwLockedBytes[0],
									 pBuf[1],
									 dwLockedBytes[1]);
	if(hRet != DS_OK)
		return FALSE;

	return TRUE;
}

/*
 * イベントスレッド (Internal)
 */

/*
 * イベントスレッドのメインループ
 */
static void EventThread(UNUSED(void *p))
{
	HANDLE hEvents[MIXER_STREAMS+1];
	DWORD dwResult;
	int i, nBuf;

	/* イベントの配列を作成する */
	for(i=0; i<MIXER_STREAMS; i++)
		hEvents[i] = hNotifyEvent[i];	/* 再生位置通知 */
	hEvents[MIXER_STREAMS] = hQuitEvent;	/* 終了通知 */

	/* イベント待機ループ */
	while(1)
	{
		/* 通知を待つ */
		dwResult = WaitForMultipleObjects(MIXER_STREAMS + 1,
										  hEvents,
										  FALSE,
										  INFINITE);
		if(dwResult == WAIT_TIMEOUT || dwResult == WAIT_FAILED)
			continue;	/* TODO: breakでいいか */
		if(dwResult == WAIT_OBJECT_0 + MIXER_STREAMS)
			break;		/* hQuitEventがセットされた */

		/* 通知元のバッファの番号を取得する */
		nBuf = (int)(dwResult - WAIT_OBJECT_0);
		assert(nBuf >= 0 && nBuf < MIXER_STREAMS);

		/* イベントを非シグナル状態に戻す */
		ResetEvent(hNotifyEvent[nBuf]);

		/* StopSoundBuffer()と排他する */
		EnterCriticalSection(&StreamCritical);
		{
			if(pStream[nBuf] == NULL) {
				/* 排他制御の結果、バッファの再生が停止された */
			} else {
				/* バッファを更新する */
				OnNotifyPlayPos(nBuf);
			}
		}
		LeaveCriticalSection(&StreamCritical);
	}
}

/*
 * 再生位置通知のハンドラ
 */
static VOID OnNotifyPlayPos(int nBuffer)
{
	DWORD dwPlayPos;
	HRESULT hRet;

	// 再生終了領域を再生し終わった場合
	if(nPosCurArea[nBuffer] == nPosEndArea[nBuffer])
	{
		/* 再生を停止する */
		IDirectSoundBuffer_Stop(pDSBuffer[nBuffer]);
		IDirectSoundBuffer_SetCurrentPosition(pDSBuffer[nBuffer], 0);

		/* ストリームを停止状態にする */
		pStream[nBuffer] = NULL;

		/* 再生終了フラグをセットする */
		bFinish[nBuffer] = TRUE;

		return;	// 再生終了
	}

	// 再生位置を取得する
	hRet = IDirectSoundBuffer_GetCurrentPosition(pDSBuffer[nBuffer],
												 &dwPlayPos,
												 NULL);
	if(hRet != DS_OK)
		return;	// 再生位置の取得に失敗した

	/*
	 * 更新する領域上を再生中である場合
	 *  - 初回の通知のときに発生する
	 *  - または、遅延がバッファN周分のとき発生する
	 *  - バッファを更新しないで次の通知を待つ
	 *  - 遅延の場合は同じ音が繰り返し再生される(壊れたCDのように)
	 */
	if(dwPlayPos >= (DWORD)nPosCurArea[nBuffer] * AREA_BYTES &&
	   dwPlayPos < ((DWORD)nPosCurArea[nBuffer] + 1) * AREA_BYTES)
		return;	/* 更新しない */

	// バッファを更新する
	WriteNext(nBuffer);
}
