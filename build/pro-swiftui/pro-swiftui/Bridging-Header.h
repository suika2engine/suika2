/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*- */

/*
 * Suika2
 * Copyright (C) 2001-2024, Keiichi Tabata. All rights reserved.
 */

/*
 * This is the bridging header file.
 */

/* Suika2 Engine */
#import "suika.h"

/* Metal Shader */
#import "ShaderTypes.h"

/*
 * "struct image" acccessors
 */
int get_image_width(const void * _Nonnull img);
int get_image_height(const void * _Nonnull img);
const char * _Nonnull get_image_pixels(const void * _Nonnull img);

/*
 * A table to call Swift functions from C code.
 */
struct HalSwiftExports {
    void (* _Nonnull logInfo)(const char * _Nonnull msg);
    void (* _Nonnull logWarn)(const char * _Nonnull msg);
    void (* _Nonnull logError)(const char * _Nonnull msg);
    bool (* _Nonnull makeSavDir)(void);
    void (*_Nonnull makeValidPath)(const char * _Nullable dir, const char * _Nullable fname, void * _Nonnull dst, size_t len);
    void (* _Nonnull notifyImageUpdate)(struct image * _Nonnull img);
    void (* _Nonnull notifyImageFree)(struct image * _Nonnull img);
    void (* _Nonnull renderImageNormal)(int dst_left,
                                        int dst_top,
                                        int dst_width,
                                        int dst_height,
                                        struct image * _Nonnull src_image,
                                        int src_left,
                                        int src_top,
                                        int src_width,
                                        int src_height,
                                        int alpha);
    void (* _Nonnull renderImageAdd)(int dst_left,
                                     int dst_top,
                                     int dst_width,
                                     int dst_height,
                                     struct image * _Nonnull src_image,
                                     int src_left,
                                     int src_top,
                                     int src_width,
                                     int src_height,
                                     int alpha);
    void (* _Nonnull renderImageDim)(int dst_left,
                                     int dst_top,
                                     int dst_width,
                                     int dst_height,
                                     struct image * _Nonnull src_image,
                                     int src_left,
                                     int src_top,
                                     int src_width,
                                     int src_height,
                                     int alpha);
    void (* _Nonnull renderImageRule)(struct image * _Nonnull src_img,
                                      struct image * _Nonnull rule_img,
                                      int threshold);
    void (* _Nonnull renderImageMelt)(struct image * _Nonnull src_img,
                                      struct image * _Nonnull rule_img,
                                      int progress);
    void (* _Nonnull renderImage3DNormal)(float x1,
                                          float y1,
                                          float x2,
                                          float y2,
                                          float x3,
                                          float y3,
                                          float x4,
                                          float y4,
                                          struct image * _Nonnull image,
                                          int src_left,
                                          int src_top,
                                          int src_width,
                                          int src_height,
                                          int alpha);
    void (* _Nonnull renderImage3DAdd)(float x1,
                                       float y1,
                                       float x2,
                                       float y2,
                                       float x3,
                                       float y3,
                                       float x4,
                                       float y4,
                                       struct image * _Nonnull image,
                                       int src_left,
                                       int src_top,
                                       int src_width,
                                       int src_height,
                                       int alpha);
    bool (* _Nonnull playVideo)(const char * _Nonnull fname, bool is_skippable);
    void (* _Nonnull stopVideo)(void);
    bool (* _Nonnull isVideoPlaying)(void);
    void (* _Nonnull updateWindowTitle)(void);
    bool (* _Nonnull isFullScreenSupported)(void);
    bool (* _Nonnull isFullScreenMode)(void);
    void (* _Nonnull enterFullScreenMode)(void);
    void (* _Nonnull leaveFullScreenMode)(void);
    void (* _Nonnull getSystemLocale)(void * _Nonnull dst, size_t len);
    void (* _Nonnull speakText)(const char * _Nullable text);
    bool (* _Nonnull isContinuePushed)(void);
    bool (* _Nonnull isNextPushed)(void);
    bool (* _Nonnull isStopPushed)(void);
    bool (* _Nonnull isScriptOpened)(void);
    void (* _Nonnull getOpenedScript)(void * _Nonnull dst, size_t len);
    bool (* _Nonnull isExecLineChanged)(void);
    int (* _Nonnull getChangedExecLine)(void);
    void (* _Nonnull onChangeRunningState)(bool running, bool request_stop);
    void (* _Nonnull onLoadScript)(void);
    void (* _Nonnull onChangePosition)(void);
    void (* _Nonnull onUpdateVariable)(void);
};
typedef struct HalSwiftExports HalSwiftExports;

/*
 * Initializes the HalSwiftExports table.
 */
void HalCSetup(const HalSwiftExports * _Nonnull hal);
