/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*- */

/*
 * Suika2
 * Copyright (C) 2001-2024, Keiichi Tabata. All rights reserved.
 */

/*
 * This is the C language wrapper of the Suika2 HAL for SwiftUI.
 */

#include "Bridging-Header.h"

static const HalSwiftExports *p;

/*
 * Initialize.
 */
void HalCSetup(const HalSwiftExports * _Nonnull hal)
{
    p = hal;
}

/*
 * Logging
 */

bool log_info(const char *s, ...)
{
    char msg[4096];
    va_list ap;
    
    va_start(ap, s);
    vsnprintf(msg, sizeof(msg), s, ap);
    va_end(ap);
    
    p->logInfo(msg);

    return true;
}

bool log_warn(const char *s, ...)
{
    char msg[4096];
    va_list ap;
    
    va_start(ap, s);
    vsnprintf(msg, sizeof(msg), s, ap);
    va_end(ap);
    
    p->logWarn(msg);

    return true;
}

bool log_error(const char *s, ...)
{
    char msg[4096];
    va_list ap;
    
    va_start(ap, s);
    vsnprintf(msg, sizeof(msg), s, ap);
    va_end(ap);
    
    p->logError(msg);

    return true;
}

/*
 * Path Manipulation
 */

bool make_sav_dir(void)
{
    p->makeSavDir();
    
    return true;
}

char *make_valid_path(const char *dir, const char *fname)
{
    char path[1024];

    p->makeValidPath(dir, fname, path, sizeof(path));

    return strdup(path);
}

/*
 * Image helpers
 */

int get_image_width(const void * _Nonnull img)
{
    assert(img != NULL);

    const struct image *image = (const struct image *)img;
    return image->width;
}

int get_image_height(const void * _Nonnull img)
{
    assert(img != NULL);

    const struct image *image = (const struct image *)img;
    return image->height;
}

const char *get_image_pixels(const void * _Nonnull img)
{
    assert(img != NULL);

    const struct image *image = (const struct image *)img;
    return (const char *)image->pixels;
}

/*
 * Rendering
 */

void notify_image_update(struct image *img)
{
    p->notifyImageUpdate(img);
}

void notify_image_free(struct image *img)
{
    p->notifyImageFree(img);
}

void
render_image_normal(
    int dst_left,
    int dst_top,
    int dst_width,
    int dst_height,
    struct image *src_image,
    int src_left,
    int src_top,
    int src_width,
    int src_height,
    int alpha)
{
    p->renderImageNormal(dst_left,
                         dst_top,
                         dst_width,
                         dst_height,
                         src_image,
                         src_left,
                         src_top,
                         src_width,
                         src_height,
                         alpha);
}

void
render_image_add(
    int dst_left,
    int dst_top,
    int dst_width,
    int dst_height,
    struct image *src_image,
    int src_left,
    int src_top,
    int src_width,
    int src_height,
    int alpha)
{
    p->renderImageAdd(dst_left,
                      dst_top,
                      dst_width,
                      dst_height,
                      src_image,
                      src_left,
                      src_top,
                      src_width,
                      src_height,
                      alpha);
}

void
render_image_dim(
    int dst_left,
    int dst_top,
    int dst_width,
    int dst_height,
    struct image *src_image,
    int src_left,
    int src_top,
    int src_width,
    int src_height,
    int alpha)
{
    p->renderImageDim(dst_left,
                      dst_top,
                      dst_width,
                      dst_height,
                      src_image,
                      src_left,
                      src_top,
                      src_width,
                      src_height,
                      alpha);
}

void
render_image_rule(
    struct image *src_img,
    struct image *rule_img,
    int threshold)
{
    p->renderImageRule(src_img, rule_img, threshold);
}

void
render_image_melt(
    struct image *src_img,
    struct image *rule_img,
    int progress)
{
    p->renderImageMelt(src_img, rule_img, progress);
}

void
render_image_3d_normal(
    float x1,
    float y1,
    float x2,
    float y2,
    float x3,
    float y3,
    float x4,
    float y4,
    struct image *src_image,
    int src_left,
    int src_top,
    int src_width,
    int src_height,
    int alpha)
{
    p->renderImage3DNormal(x1, y1, x2, y2, x3, y3, x4, y4, src_image, src_left, src_top, src_width, src_height, alpha);
}

void
render_image_3d_add(
    float x1,
    float y1,
    float x2,
    float y2,
    float x3,
    float y3,
    float x4,
    float y4,
    struct image *src_image,
    int src_left,
    int src_top,
    int src_width,
    int src_height,
    int alpha)
{
    p->renderImage3DAdd(x1, y1, x2, y2, x3, y3, x4, y4, src_image, src_left, src_top, src_width, src_height, alpha);
}

/*
 * Lap Timer
 */

/* POSIX */
#import <sys/time.h>

void reset_lap_timer(uint64_t *origin)
{
    struct timeval tv;

    gettimeofday(&tv, NULL);
    *origin = (uint64_t)(tv.tv_sec * 1000 + tv.tv_usec / 1000);
}

uint64_t get_lap_timer_millisec(uint64_t *origin)
{
    struct timeval tv;
    uint64_t now;

    gettimeofday(&tv, NULL);
    now = (uint64_t)(tv.tv_sec * 1000 + tv.tv_usec / 1000);
    if (now < *origin) {
        reset_lap_timer(origin);
        return 0;
    }
    return (int)(now - *origin);
}

/*
 * Dialog
 */

bool exit_dialog(void)
{
    /* stub */
    return false;
}

bool title_dialog(void)
{
    /* stub */
    return false;
}

bool delete_dialog(void)
{
    /* stub */
    return false;
}

bool overwrite_dialog(void)
{
    /* stub */
    return false;
}

bool default_dialog(void)
{
    /* stub */
    return false;
}

/*
 * Video Playback
 */

bool play_video(const char *fname, bool is_skippable)
{
    bool ret;

    ret = p->playVideo(fname, is_skippable);

    return ret;
}

void stop_video(void)
{
    p->stopVideo();
}

bool is_video_playing(void)
{
    if (!p->isVideoPlaying())
        return false;
    return true;
}

/*
 * Window
 */

void update_window_title(void)
{
    p->updateWindowTitle();
}

bool is_full_screen_supported(void)
{
    bool ret;

    ret = p->isFullScreenSupported();

    return ret;
}

bool is_full_screen_mode(void)
{
    bool ret;
    
    ret = p->isFullScreenMode();

    return ret;
}

void enter_full_screen_mode(void)
{
    p->enterFullScreenMode();
}

void leave_full_screen_mode(void)
{
    p->leaveFullScreenMode();
}

/*
 * Locale
 */

const char *get_system_locale(void)
{
    static char locale[16];

    p->getSystemLocale((void *)&locale[0], 16);

    return locale;
}

/*
 * Text-to-speech
 */

void speak_text(const char *text)
{
    /* stub */
}

/*
 * Suika2 Pro
 */

bool is_continue_pushed(void)
{
    return p->isContinuePushed();
}

bool is_next_pushed(void)
{
    return p->isNextPushed();
}

bool is_stop_pushed(void)
{
    return p->isStopPushed();
}

bool is_script_opened(void)
{
    return p->isScriptOpened();
}

const char *get_opened_script(void)
{
    static char name[128];

    p->getOpenedScript(name, sizeof(name));

    return strdup(name);
}

bool is_exec_line_changed(void)
{
    return p->isExecLineChanged();
}

int get_changed_exec_line(void)
{
    return p->getChangedExecLine();
}

void on_change_running_state(bool running, bool request_stop)
{
    p->onChangeRunningState(running, request_stop);
}

void on_load_script(void)
{
    p->onLoadScript();
}

void on_change_position(void)
{
    p->onChangePosition();
}

void on_update_variable(void)
{
    p->onUpdateVariable();
}

/*
 * Sound (AudioUnit)
 *
 * [Changes]
 *  - 2016/06/17 Created.
 *  - 2016/07/03 Implemented the mixing.
 *  - 2024/02/17 Moved behind the Swift HAL.
 */

#include <AudioUnit/AudioUnit.h>
#include <pthread.h>

/* PCM format. */
#define SAMPLING_RATE   (44100)
#define CHANNELS        (2)
#define DEPTH           (16)
#define FRAME_SIZE      (4)

/* Temporary samples. */
#define TMP_SAMPLES     (512)

/* AudioUnit */
static AudioUnit au;

/* Mutual exclusion between the main thread and the callback thread. */
static pthread_mutex_t mutex;

/* Input streams. */
static struct wave *wave[MIXER_STREAMS];

/* Volumes. */
static float volume[MIXER_STREAMS];

/* Flags to indicate playbacks are finished. */
static bool finish[MIXER_STREAMS];

/* Temporary samples. */
static uint32_t tmpBuf[TMP_SAMPLES];

/* Is this module initialized? */
static bool isInitialized;

/* Is the playback running? */
static bool isPlaying;

/* Forward declarations. */
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
 * Initialize AudioUnit.
 */
bool init_aunit(void)
{
    int n;
    bool ret;

    /* Create an AudioUnit. */
    if(!create_audio_unit())
        return false;

    /* Initialize the mutex. */
    pthread_mutex_init(&mutex, NULL);

    /* Set the volumes. */
    for (n = 0; n < MIXER_STREAMS; n++)
        volume[n] = 1.0f;

    /* Mark this module initialize. */
    isInitialized = true;

    /* Start audio playback. */
    ret = AudioOutputUnitStart(au) == noErr;
    if (!ret)
        log_api_error("AudioOutputUnitStart");
    else
        isPlaying = true;

    return ret;
}

/* Create an AudioUnit. */
static bool create_audio_unit(void)
{
    AudioComponentDescription cd;
    AudioComponent comp;
    AURenderCallbackStruct cb;
    AudioStreamBasicDescription streamFormat;

    /* Get an audio component. */
    cd.componentType = kAudioUnitType_Output;
#ifdef SUIKA_TARGET_IOS
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

    /* Set the callback. */
    cb.inputProc = callback;
    cb.inputProcRefCon = NULL;
    if(AudioUnitSetProperty(au,
                            kAudioUnitProperty_SetRenderCallback,
                            kAudioUnitScope_Input,
                            0,
                            &cb,
                            sizeof(AURenderCallbackStruct)) != noErr) {
        log_api_error("AudioUnitSetProperty");
        return false;
    }

    /* Set the PCM format. */
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
    if(AudioUnitSetProperty(au,
                            kAudioUnitProperty_StreamFormat,
                            kAudioUnitScope_Input,
                            0,
                            &streamFormat,
                            sizeof(streamFormat)) != noErr) {
        log_api_error("AudioUnitSetProperty");
        return false;
    }

    return true;
}

/*
 * Cleanup AudioUnit.
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

/* Destroy AudioUnit. */
static void destroy_audio_unit(void)
{
    AudioOutputUnitStop(au);
    AudioUnitUninitialize(au);
    AudioComponentInstanceDispose(au);
}

/*
 * HAL: Play sound.
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
 * HAL: Stop sound.
 */
bool stop_sound(int stream)
{
    bool ret;

    ret = true;
    pthread_mutex_lock(&mutex);
    {
        /* Set the playing stream to "none". */
        wave[stream] = NULL;

        /* Raise the finished flag. */
        finish[stream] = true;
    }
    pthread_mutex_unlock(&mutex);

    return ret;
}

/*
 * HAL: Set a volume of a sound stream.
 */
bool set_sound_volume(int stream, float vol)
{
    volume[stream] = vol;
    __sync_synchronize();
    return true;
}

/*
 * HAL: Check if a sound stream playbackサウンドが再生終了したか調べる
 */
bool is_sound_finished(int stream)
{
    if (finish[stream])
        return true;

    return false;
}

/*
 * Callback thread.
 */

/* Callback. */
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

    /* First, zero-clear the buffer. */
    samplePtr = (uint32_t *)ioData->mBuffers[0].mData;
    memset(samplePtr, 0, sizeof(uint32_t) * inNumberFrames);

    pthread_mutex_lock(&mutex);
    {
        remain = (int)inNumberFrames;
        while (remain > 0) {
            /* Calc the samples to read. */
            readSamples = remain > TMP_SAMPLES ? TMP_SAMPLES : remain;

            /* For each stream: */
            for (stream = 0; stream < MIXER_STREAMS; stream++) {
                /* If not playing, skip. */
                if (wave[stream] == NULL)
                    continue;

                /* Get PCM samples. */
                ret = get_wave_samples(wave[stream], tmpBuf, readSamples);

                /* When reached the end-of-stream: */
                if (ret < readSamples) {
                    /* Zero-clear the missing samples. */
                    memset(tmpBuf + ret, 0, (size_t)(readSamples - ret) * sizeof(uint32_t));

                    /* Set the playing stream to "none". */
                    wave[stream] = NULL;

                    /* Raise the finish flag. */
                    finish[stream] = true;
                }

                /* Apply the volume. */
                mul_add_pcm(samplePtr, tmpBuf, volume[stream], readSamples);
            }

            /* Increment the write position. */
            samplePtr += readSamples;
            remain -= readSamples;
        }
    }
    pthread_mutex_unlock(&mutex);

    return noErr;
}

/*
 * HAL: Stop sound.
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
 * HAL: Resume sound.
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

/* Apply sound volume to PCM samples. */
static void mul_add_pcm(uint32_t *dst, uint32_t *src, float vol, int samples)
{
    float scale;
    int i;
    int32_t il, ir; /* intermediate L/R */
    int16_t sl, sr; /* source L/R*/
    int16_t dl, dr; /* destination L/R */

    /* Convert a scale factor to exponential. */
    scale = (powf(10.0f, vol) - 1.0f) / (10.0f - 1.0f);

    /* Make PCM samples. */
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
