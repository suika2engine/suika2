/* -*- coding: utf-8; indent-tabs-mode: t; tab-width: 4; c-basic-offset: 4; -*- */

#include <windows.h>
#include <sapi.h>

extern "C" {
#include "../suika.h"
#include "tts_sapi.h"
};

ISpVoice *pVoice;

void InitSAPI(void)
{
	HRESULT hRes;

	/* SAPI5 Text-To-Speechを初期化する */
	if (pVoice == NULL)
	{
		hRes = CoCreateInstance(CLSID_SpVoice, NULL, CLSCTX_ALL, IID_ISpVoice, (void **)&pVoice);
		if (!SUCCEEDED(hRes))
			log_info("TTS initialization failed");
	}
}

void SpeakSAPI(const wchar_t *text)
{
	ULONG nSkipped;

	if (pVoice != NULL)
	{
		if (text != NULL)
			pVoice->Speak(text, SVSFlagsAsync, NULL);
		else
			pVoice->Skip(L"SENTENCE", 100, &nSkipped);
	}
}
