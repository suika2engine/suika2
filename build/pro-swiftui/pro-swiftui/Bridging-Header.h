/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*- */

/*
 * Suika2
 * Copyright (C) 2001-2024, Keiichi Tabata. All rights reserved.
 */

/*
 * This is the bridging header of Swift and C.
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
    bool (* _Nonnull logInfo)(const char * _Nonnull msg);
    bool (* _Nonnull logWarn)(const char * _Nonnull msg);
    bool (* _Nonnull logError)(const char * _Nonnull msg);
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
    bool (* _Nonnull playVideo)(const char * _Nonnull fname, bool is_skippable);
    void (* _Nonnull stopVideo)(void);
    bool (* _Nonnull isVideoPlaying)(void);
    void (* _Nonnull updateWindowTitle)(void);
    bool (* _Nonnull isFullScreenSupported)(void);
    bool (* _Nonnull isFullScreenMode)(void);
    void (* _Nonnull enterFullScreenMode)(void);
    void (* _Nonnull leaveFullScreenMode)(void);
    const char * _Nonnull (* _Nonnull getSystemLocale)(void);
    void (* _Nonnull speakText)(const char * _Nullable text);
};
typedef struct HalSwiftExports HalSwiftExports;

/*
 * Initializes the HalSwiftExports table.
 */
void HalCSetup(const HalSwiftExports * _Nonnull hal);
