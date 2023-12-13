#ifndef SUIKA_TTS_SAPI_H
#define SUIKA_TTS_SAPI_H

#ifdef __cplusplus
extern "C" {
#endif

void InitSAPI(void);
void SpeakSAPI(const wchar_t *text);

#ifdef __cplusplus
};
#endif

#endif
