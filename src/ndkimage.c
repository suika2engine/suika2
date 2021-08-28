/* -*- coding: utf-8-with-signature; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Suika 2
 * Copyright (C) 2001-2016, TABATA Keiichi. All rights reserved.
 */

/*
 * [Changes]
 *  2001-10-03 作成 [KImage@VNStudio]
 *  2002-03-10 アルファブレンド対応, 乗算テーブル [VNImage@西瓜Studio]
 *  2004-01-05 MMX対応 [LXImage@Texture Alpha Maker]
 *  2006-09-05 Cに書き換え [VImage@V]
 *  2016-05-27 gcc5.3.1のベクトル化に対応, 浮動小数点 [image@Suika]
 *  2016-06-11 SSEバージョニングを実装
 *  2016-06-16 OSX対応
 *  2016-08-05 Android NDK対応
 */

#include "suika.h"
#include "ndkmain.h"

#include <stdio.h>
#include <stdlib.h>

#include <android/bitmap.h>

/*
 * image構造体
 */
struct image {
	jobject bm;
	int width;
	int height;
};

/*
 * イメージを作成する
 */
struct image *create_image(int w, int h)
{
	struct image *img;
	jclass cls;
	jmethodID mid;

	/* image構造体のメモリを確保する */
	img = malloc(sizeof(struct image));
	if (img == NULL)
		return NULL;

	/* Bitmapを作成する */
	cls = (*jni_env)->FindClass(jni_env, "jp/luxion/suika/MainActivity");
	mid = (*jni_env)->GetMethodID(jni_env, cls, "createBitmap", "(II)Landroid/graphics/Bitmap;");
	img->bm = (*jni_env)->CallObjectMethod(jni_env, main_activity, mid, w, h);
	img->bm = (*jni_env)->NewGlobalRef(jni_env, img->bm);
	img->width = w;
	img->height = h;

	return img;
}

/*
 * ファイル名を指定してイメージを作成する
 */
struct image *create_image_from_file(const char *dir, const char *file)
{
	char buf[1024];
	AndroidBitmapInfo info;
	struct image *img;
	jclass cls;
	jmethodID mid;

	/* パスを作成する */
	snprintf(buf, sizeof(buf), "%s/%s", dir, file);

	/* image構造体のメモリを確保する */
	img = malloc(sizeof(struct image));
	if (img == NULL)
		return NULL;

	/* Bitmapを作成する */
	cls = (*jni_env)->FindClass(jni_env, "jp/luxion/suika/MainActivity");
	mid = (*jni_env)->GetMethodID(jni_env, cls, "loadBitmap", "(Ljava/lang/String;)Landroid/graphics/Bitmap;");
	img->bm = (*jni_env)->CallObjectMethod(jni_env, main_activity, mid, (*jni_env)->NewStringUTF(jni_env, buf));
	if (img->bm == NULL) {
		free(img);
		return NULL;
	}
	img->bm = (*jni_env)->NewGlobalRef(jni_env, img->bm);

	/* Bitmapの幅と高さを取得する */
	AndroidBitmap_getInfo(jni_env, img->bm, &info);
	img->width = (int)info.width;
	img->height = (int)info.height;

	return img;
}

/*
 * 文字列で色を指定してイメージを作成する
 */
struct image *create_image_from_color_string(int w, int h, const char *color)
{
	struct image *img;
	uint32_t r, g, b;
	pixel_t cl;
	int rgb;

	/* イメージを作成する */
	img = create_image(w, h);
	if (img == NULL)
		return NULL;

	/* 色指定文字列を読み込む */
	sscanf(color, "%x", &rgb);
	r = (rgb >> 16) & 0xff;
	g = (rgb >> 8) & 0xff;
	b = rgb & 0xff;
	cl = make_pixel(0xff, r, g, b);

	/* イメージを塗り潰す */
	lock_image(img);
	clear_image_color(img, cl);
	unlock_image(img);

	return img;
}

/*
 * イメージを削除する
 */
void destroy_image(struct image *img)
{
	/* Bitmapを破棄する */
	(*jni_env)->DeleteGlobalRef(jni_env, img->bm);
	img->bm = NULL;
	free(img);
}

/*
 * イメージに関連付けられたシステムのオブジェクトを取得する
 */
void *get_image_object(struct image *img)
{
	return img->bm;
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
 * イメージをロックする
 */
bool lock_image(struct image *img)
{
	return true;
}

/*
 * イメージをアンロックする
 */
void unlock_image(struct image *img)
{
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
	jclass cls;
	jmethodID mid;

	/* 矩形を描画する */
	cls = (*jni_env)->FindClass(jni_env, "jp/luxion/suika/MainActivity");
	mid = (*jni_env)->GetMethodID(jni_env, cls, "drawRect", "(Landroid/graphics/Bitmap;IIIII)V");
	(*jni_env)->CallVoidMethod(jni_env, main_activity, mid, img->bm, x, y, w, h, color);
}

/*
 * イメージを描画する
 */
void draw_image(struct image * RESTRICT dst_image, int dst_left, int dst_top,
		struct image * RESTRICT src_image, int width, int height,
		int src_left, int src_top, int alpha, int bt)
{
	jclass cls;
	jmethodID mid;

	if (bt == BLEND_FAST) {
		cls = (*jni_env)->FindClass(jni_env, "jp/luxion/suika/MainActivity");
		mid = (*jni_env)->GetMethodID(jni_env, cls, "drawBitmapAlpha", "(Landroid/graphics/Bitmap;IILandroid/graphics/Bitmap;IIIII)V");
		(*jni_env)->CallVoidMethod(jni_env, main_activity, mid, dst_image->bm, dst_left, dst_top, src_image->bm, width, height, src_left, src_top, alpha);
	} else if (bt == BLEND_NONE) {
		cls = (*jni_env)->FindClass(jni_env, "jp/luxion/suika/MainActivity");
		mid = (*jni_env)->GetMethodID(jni_env, cls, "drawBitmapCopy", "(Landroid/graphics/Bitmap;IILandroid/graphics/Bitmap;IIII)V");
		(*jni_env)->CallVoidMethod(jni_env, main_activity, mid, dst_image->bm, dst_left, dst_top, src_image->bm, width, height, src_left, src_top);
	} else {
		assert(0);
	}
}

/* イメージをマスクつきで描画する */
void draw_image_mask(struct image * RESTRICT dst_image, int dst_left,
		     int dst_top, struct image * RESTRICT src_image, int width,
		     int height, int src_left, int src_top, int mask_level)
{
	int alpha = (int)(((float)mask_level / 27.0f) * 255.0f);

	draw_image(dst_image, dst_left, dst_top, src_image, width, height,
		   src_left, src_top, alpha, BLEND_NORMAL);
}
