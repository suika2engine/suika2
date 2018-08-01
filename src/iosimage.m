/* -*- coding: utf-8; tab-width: 4; indent-tabs-mode: nil; -*- */

/*
 * Suika 2
 * Copyright (C) 2001-2016, TABATA Keiichi. All rights reserved.
 */

/*
 * [Changes]
 *  2001-10-03 作成 [KImage@VNStudio]
 *  2002-03-10 アルファブレンド対応 [VNImage@西瓜Studio]
 *  2004-01-05 MMX対応 [LXImage@Texture Alpha Maker]
 *  2006-09-05 Cに書き換え [VImage@V]
 *  2016-05-27 gcc5.3.1のベクトル化に対応, 浮動小数点 [image@Suika]
 *  2016-06-11 SSEバージョニングを実装
 *  2016-06-16 OSX対応
 *  2016-08-05 Android NDK対応
 *  2016-08-23 iOS対応
 */

#include "suika.h"
#include <CoreGraphics/CoreGraphics.h>

/*
 * image構造体
 */
struct image {
    int width;
    int height;
    CGColorSpaceRef rgb;
    CGDataProviderRef provider;
    CGImageRef img;
};

/*
 * イメージを作成する
 */
struct image *create_image(int w, int h)
{
    struct image *img;
    void *buf;

    assert(w > 0 && h > 0);

    /* イメージ構造体のメモリを確保する */
    img = (struct image *)malloc(sizeof(struct image));
    if (img == NULL) {
        log_memory();
        return NULL;
    }

    /* イメージのメモリを確保する */
    buf = malloc(w * h * 4);
    if (buf == NULL) {
        log_memory();
        free(img);
        return NULL;
    }

    /* イメージを作成する */
    img->rgb = CGColorSpaceCreateDeviceRGB();
    img->provider = CGDataProviderCreateWithData(NULL, &buf, w * h * 4, NULL);
    img->img = CGImageCreate(w, h, 8, 8 * 4, w * 4, img->rgb,
                             kCGBitmapByteOrderDefault | kCGImageAlphaLast,
                             img->provider, NULL, false,
                             kCGRenderingIntentDefault);

    /* 成功 */
    return img;
}

/*
 * ファイル名を指定してイメージを作成する
 */
struct image *create_image_from_file(const char *dir, const char *file)
{
    UIImage *compressedImage = [UIImage imageNamed:[NSString stringWithFormat:@"%s/%s", dir, file];

    
    CFDataRef imgData = (CFDataRef)data;
    CGDataProviderRef imgDataProvider = CGDataProviderCreateWithCFData (imgData);
    CGImageRef image = CGImageCreateWithPNGDataProvider(imgDataProvider, NULL, true, kCGRenderingIntentDefault);

    size_t width = CGImageGetWidth(compressedImage);
    size_t height = CGImageGetHeight(compressedImage);
    size_t bitsPerComponent = 8;
    size_t bytesPerRow = 4 * width;
    size_t bytesSize = bytesPerRow * height;
    uint8_t *bytes = malloc(bytesSize);
    
    CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
    
    CGContextRef context = CGBitmapContextCreate(bytes, width, height, bitsPerComponent, bytesPerRow, colorSpace, kCGBitmapAlphaInfoMask &amp; kCGImageAlphaPremultipliedFirst);
    CGContextSetBlendMode(context, kCGBlendModeCopy);
    CGContextSetInterpolationQuality(context, kCGInterpolationNone);
    CGContextDrawImage(context, CGRectMake(0, 0, width, height), compressedImage);
    
    CGContextRelease(context);
    
    CGDataProviderRef dataProvider = CGDataProviderCreateWithData(NULL, bytes, bytesSize, bufferFree);
    size_t bitsPerPixel = 32;
    CGImageRef rasterImage = CGImageCreate(width,
                                           height,
                                           bitsPerComponent,
                                           bitsPerPixel,
                                           bytesPerRow,
                                           colorSpace,
                                           kCGBitmapAlphaInfoMask &amp; kCGImageAlphaPremultipliedFirst,
                                           dataProvider,
                                           NULL,
                                           NO,
                                           kCGRenderingIntentDefault);
    CGDataProviderRelease(dataProvider);
    CGColorSpaceRelease(colorSpace);
    return rasterImage;
}

/*
 * イメージを削除する
 */
void destroy_image(struct image *img)
{
    CGImageRelease(img->img);
    CGDataProviderRelease(img->provider);
    CGColorSpaceRelease(img->rgb);
    free(img);
}

/*
 * イメージに関連付けられたオブジェクトを取得する(for NDK, iOS)
 */
void *get_image_object(struct image *img)
{
    return img->img;
}

/*
 * イメージの幅を取得する
 */
int get_image_width(struct image *img)
{
    return img->width;
}

/*
 * イメージの高さを取得する
 */
int get_image_height(struct image *img)
{
    return img->height;
}

/*
 * イメージを黒色でクリアする
 */
void clear_image_black(struct image *img)
{
    clear_image_color_rect(img, 0, 0, img->width, img->height,
                           make_pixel(0xff, 0, 0, 0));
}

/*
 * イメージの矩形を黒色でクリアする
 */
void clear_image_black_rect(struct image *img, int x, int y, int w, int h)
{
    clear_image_color_rect(img, x, y, w, h, make_pixel(0xff, 0, 0, 0));
}

/*
 * イメージを白色でクリアする
 */
void clear_image_white(struct image *img)
{
    clear_image_color_rect(img, 0, 0, img->width, img->height,
                           make_pixel(0xff, 0xff, 0xff, 0xff));
}

/*
 * イメージの矩形を白色でクリアする
 */
void clear_image_white_rect(struct image *img, int x, int y, int w, int h)
{
    clear_image_color_rect(img, x, y, w, h,
                           make_pixel(0xff, 0xff, 0xff, 0xff));
}

/*
 * イメージを色でクリアする
 */
void clear_image_color(struct image *img, pixel_t color)
{
    clear_image_color_rect(img, 0, 0, img->width, img->height, color);
}

/*
 * イメージの矩形を色でクリアする
 */
void clear_image_color_rect(struct image *img, int x, int y, int w, int h,
                            pixel_t color)
{
    // TODO
}

/*
 * イメージを描画する
 */
void draw_image(struct image * RESTRICT dst_image, int dst_left, int dst_top,
                struct image * RESTRICT src_image, int width, int height,
                int src_left, int src_top, int alpha, int bt)
{
}
