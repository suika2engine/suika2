/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*- */

/*
 * Suika 2
 * Copyright (C) 2001-2016, TABATA Keiichi. All rights reserved.
 */

/*
 * Audio Unit ミキサ
 *
 * [Changes]
 *  - 2016/06/17 作成
 *  - 2016/07/03 ミキシング実装
 */

#include <AudioUnit/AudioUnit.h>
#include <pthread.h>

#include "suika.h"
#include "aunit.h"

/* フォーマット */
#define SAMPLING_RATE   (44100)
#define CHANNELS        (2)
#define DEPTH           (16)
#define FRAME_SIZE      (4)

/* 一時保管するサンプル数 */
#define TMP_SAMPLES     (512)

/* オーディオユニット */
static AudioUnit au;

/* コールバックスレッドとの同期用 */
static pthread_mutex_t mutex;

/* 入力ストリーム */
static struct wave *wave[MIXER_STREAMS];

/* ボリューム */
static float volume[MIXER_STREAMS];

/* 再生終了フラグ */
static bool finish[MIXER_STREAMS];

/* サンプルの一時保管場所 */
static uint32_t tmpBuf[TMP_SAMPLES];

/* 初期化済みか */
static bool isInitialized;

/* 再生を開始済みか */
static bool isPlaying;

/* 前方参照 */
static bool create_audio_unit(void);
static void destroy_audio_unit(void);
static OSStatus callback(void *inRef,
                         AudioUnitRenderActionFlags *ioActionFlags,
                         const AudioTimeStamp *inTimeStamp,
                         UInt32 inBusNumber,
                         UInt32 inNumberFrames,
                         AudioBufferList *ioData);
static void mul_add_pcm(uint32_t *dst, uint32_t *src, float vol, int samples);

/*
 * ミキサの初期化処理を行う
 */
bool init_aunit(void)
{
    int n;
    bool ret;

    /* オーディオユニットを作成する */
    if(!create_audio_unit())
        return false;

    /* ミューテックスを初期化する */
    pthread_mutex_init(&mutex, NULL);

    /* ボリュームを設定する */
    for (n = 0; n < MIXER_STREAMS; n++)
        volume[n] = 1.0f;

    /* 初期化済みとする */
    isInitialized = true;

    /* オーディオ再生を開始する */
    ret = AudioOutputUnitStart(au) == noErr;
    if (!ret)
        log_api_error("AudioOutputUnitStart");
    else
        isPlaying = true;

    return ret;
}

/* オーディオユニットを作成する */
static bool create_audio_unit(void)
{
    AudioComponentDescription cd;
    AudioComponent comp;
    AURenderCallbackStruct cb;
    AudioStreamBasicDescription streamFormat;

    /* オーディオコンポーネントを取得する */
    cd.componentType = kAudioUnitType_Output;
#ifdef IOS
    cd.componentSubType = kAudioUnitSubType_RemoteIO;
#else
    cd.componentSubType = kAudioUnitSubType_DefaultOutput;
#endif
    cd.componentManufacturer = kAudioUnitManufacturer_Apple;
    cd.componentFlags = 0;
    cd.componentFlagsMask = 0;
    comp = AudioComponentFindNext(NULL, &cd);
    if(comp == NULL) {
        log_api_error("AudioComponentFindNext");
        return false;
    }
    if(AudioComponentInstanceNew(comp, &au) != noErr) {
        log_api_error("AudioComponentInstanceNew");
        return false;
    }
    if(AudioUnitInitialize(au) != noErr) {
        log_api_error("AudioUnitInitialize");
        return false;
    }

    /* コールバックをセットする */
    cb.inputProc = callback;
    cb.inputProcRefCon = NULL;
    if(AudioUnitSetProperty(au, kAudioUnitProperty_SetRenderCallback,
                            kAudioUnitScope_Input, 0, &cb,
                            sizeof(AURenderCallbackStruct)) != noErr) {
        log_api_error("AudioUnitSetProperty");
        return false;
    }

    /* フォーマットを設定する */
    streamFormat.mSampleRate = 44100.0;
    streamFormat.mFormatID = kAudioFormatLinearPCM;
    streamFormat.mFormatFlags = kAudioFormatFlagIsSignedInteger |
                                kAudioFormatFlagIsPacked;
    streamFormat.mBitsPerChannel = 16;
    streamFormat.mChannelsPerFrame = 2;
    streamFormat.mFramesPerPacket = 1;
    streamFormat.mBytesPerFrame = 4;
    streamFormat.mBytesPerPacket = 4;
    streamFormat.mReserved = 0;
    if(AudioUnitSetProperty(au, kAudioUnitProperty_StreamFormat,
                            kAudioUnitScope_Input, 0, &streamFormat,
                            sizeof(streamFormat)) != noErr) {
        log_api_error("AudioUnitSetProperty");
        return false;
    }

    return true;
}

/*
 * ミキサの終了処理を行う
 */
void cleanup_aunit(void)
{
    if(isInitialized) {
        /* 再生を終了する */
        destroy_audio_unit();

        /* ミューテックスを破棄する */
        pthread_mutex_destroy(&mutex);
    }
}

/* オーディオユニットを破棄する */
static void destroy_audio_unit(void)
{
    AudioOutputUnitStop(au);
    AudioUnitUninitialize(au);
    AudioComponentInstanceDispose(au);
}

/*
 * サウンドを再生を開始する
 */
bool play_sound(int stream, struct wave *w)
{
    bool ret;

    ret = true;
    pthread_mutex_lock(&mutex);
    {
        /* 再生中のストリームをセットする */
        wave[stream] = w;

        /* 再生終了フラグをクリアする */
        finish[stream] = false;

        /* まだ再生中でなければ、再生を開始する */
        if(!isPlaying) {
            ret = AudioOutputUnitStart(au) == noErr;
            if (!ret)
                log_api_error("AudioOutputUnitStart");
            isPlaying = true;
        }
    }
    pthread_mutex_unlock(&mutex);

    return ret;
}

/*
 * サウンドの再生を停止する
 */
bool stop_sound(int stream)
{
    bool ret;

    ret = true;
    pthread_mutex_lock(&mutex);
    {
        /* 再生中のストリームをなしとする */
        wave[stream] = NULL;

        /* 再生終了フラグをセットする */
        finish[stream] = true;
    }
    pthread_mutex_unlock(&mutex);

    return ret;
}

/*
 * サウンドのボリュームを設定する
 */
bool set_sound_volume(int stream, float vol)
{
    /*
     * pthread_mutex_lock(&mutex);
     * {
     */

    volume[stream] = vol;

    /* 
     * }
     * pthread_mutex_unlock(&mutex);
     */

    return true;
}

/*
 * サウンドが再生終了したか調べる
 */
bool is_sound_finished(int stream)
{
    if (finish[stream])
        return true;

    return false;
}

/*
 * コールバックスレッド
 */

/* コールバック */
static OSStatus callback(void *inRef,
                         AudioUnitRenderActionFlags *ioActionFlags,
                         const AudioTimeStamp *inTimeStamp,
                         UInt32 inBusNumber,
                         UInt32 inNumberFrames,
                         AudioBufferList *ioData)
{
    uint32_t *samplePtr;
    int stream, ret, remain, readSamples;

    UNUSED_PARAMETER(inRef);
    UNUSED_PARAMETER(ioActionFlags);
    UNUSED_PARAMETER(inTimeStamp);
    UNUSED_PARAMETER(inBusNumber);

    /* 再生バッファをひとまずゼロクリアする */
    samplePtr = (uint32_t *)ioData->mBuffers[0].mData;
    memset(samplePtr, 0, sizeof(uint32_t) * inNumberFrames);

    pthread_mutex_lock(&mutex);
    {
        remain = (int)inNumberFrames;
        while (remain > 0) {
            /* 1回に読み込むサンプル数を求める */
            readSamples = remain > TMP_SAMPLES ? TMP_SAMPLES : remain;

            /* 各ストリームについて */
            for (stream = 0; stream < MIXER_STREAMS; stream++) {
                /* 再生中でなければサンプルを取得しない */
                if (wave[stream] == NULL)
                    continue;

                /* 入力ストリームからサンプルを取得する */
                ret = get_wave_samples(wave[stream], tmpBuf, readSamples);

                /* ストリームの終端に達した場合 */
                if(ret < readSamples) {
                    /* 足りない分を0クリアする */
                    memset(tmpBuf + ret, 0,
                           (size_t)(readSamples - ret) * sizeof(uint32_t));

                    /* 再生中のストリームをなしとする */
                    wave[stream] = NULL;

                    /* 再生終了フラグをセットする */
                    finish[stream] = true;
                }

                /* ミキシングを行う */
                mul_add_pcm(samplePtr, tmpBuf, volume[stream], readSamples);
            }

            /* 書き込み位置を進める */
            samplePtr += readSamples;
            remain -= readSamples;
        }
    }
    pthread_mutex_unlock(&mutex);

    return noErr;
}

/*
 * サウンドを一時停止する
 */
void pause_sound(void)
{
    pthread_mutex_lock(&mutex);
    {
        if(isPlaying) {
            AudioOutputUnitStop(au);
            isPlaying = false;
        }
    }
    pthread_mutex_unlock(&mutex);
}

/*
 * サウンドを再開する
 */
void resume_sound(void)
{
    pthread_mutex_lock(&mutex);
    {
        if(!isPlaying) {
            AudioOutputUnitStart(au);
            isPlaying = true;
        }
    }
    pthread_mutex_unlock(&mutex);
}

static void mul_add_pcm(uint32_t *dst, uint32_t *src, float vol, int samples)
{
    float scale;
    int i;
    int32_t il, ir; /* intermediate L/R */
    int16_t sl, sr; /* source L/R*/
    int16_t dl, dr; /* destination L/R */

    /* スケールファクタを指数関数にする */
    scale = (powf(10.0f, vol) - 1.0f) / (10.0f - 1.0f);

    /* 各サンプルを合成する */
    for (i = 0; i < samples; i++) {
        dl = (int16_t)(uint16_t)dst[i];
        dr = (int16_t)(uint16_t)(dst[i] >> 16);

        sl = (int16_t)(uint16_t)src[i];
        sr = (int16_t)(uint16_t)(src[i] >> 16);

        il = (int32_t)dl + (int32_t)(sl * scale);
        ir = (int32_t)dr + (int32_t)(sr * scale);

        il = il > 32767 ? 32767 : il;
        il = il < -32768 ? -32768 : il;
        ir = ir > 32767 ? 32767 : ir;
        ir = ir < -32768 ? -32768 : ir;

        dst[i] = ((uint32_t)(uint16_t)(int16_t)il) |
                 (((uint32_t)(uint16_t)(int16_t)ir) << 16);
    }
}
