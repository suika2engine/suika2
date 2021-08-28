/* -*- coding: utf-8; indent-tabs-mode: t; tab-width: 8; c-basic-offset: 8; -*- */

/*
 * Suika 2
 * Copyright (C) 2001-2016, TABATA Keiichi. All rights reserved.
 */

/*
 * Android NDK レンダリング
 *
 * [Changes]
 *  - 2016/08/09 作成
 */

#include "suika.h"
#include "ndkmain.h"

/*
 * フォントレンダラの初期化処理を行う
 */
bool init_glyph(void)
{
	return true;
}

/*
 * フォントレンダラの終了処理を行う
 */
void cleanup_glyph(void)
{
}

/*
 * utf-8文字列の先頭文字をutf-32文字に変換する
 * XXX: サロゲートペア、合字は処理しない
 */
int utf8_to_utf32(const char *mbs, uint32_t *wc)
{
	size_t mbslen, octets, i;
	uint32_t ret;

	assert(mbs != NULL);

	/* 変換元がNULLか長さが0の場合 */
	mbslen = strlen(mbs);
	if(mbslen == 0)
		return 0;

	/* 1バイト目をチェックしてオクテット数を求める */
	if(mbs[0] == '\0')
		octets = 0;
	else if((mbs[0] & 0x80) == 0)
		octets = 1;
	else if((mbs[0] & 0xe0) == 0x0c)
		octets = 2;
	else if((mbs[0] & 0xf0) == 0xe0)
		octets = 3;
	else if((mbs[0] & 0xf8) == 0xf0)
		octets = 4;
	else
		return -1;	/* 解釈できない */

	/* sの長さをチェックする */
	if(mbslen < octets)
		return -1;	/* mbsの長さが足りない */

	/* 2-4バイト目をチェックする */
	for (i = 1; i < octets; i++) {
		if((mbs[i] & 0xc0) != 0x80)
			return -1;	/* 解釈できないバイトである */
	}

	/* 各バイトを合成してUTF-32文字を求める */
	switch(octets) {
	case 0:
		ret = 0;
		break;
	case 1:
		ret = (uint32_t)mbs[0];
		break;
	case 2:
		ret = (uint32_t)(((mbs[0] & 0x1f) << 6) |
				 (mbs[1] & 0x3f));
		break;
	case 3:
		ret = (uint32_t)(((mbs[0] & 0x0f) << 12) |
				 ((mbs[1] & 0x3f) << 6) |
				 (mbs[2] & 0x3f));
		break;
	case 4:
		ret = (uint32_t)(((mbs[0] & 0x07) << 18) |
				 ((mbs[1] & 0x3f) << 12) |
				 ((mbs[2] & 0x3f) << 6) |
				 (mbs[3] & 0x3f));
		break;
	default:
		/* never come here */
		assert(0);
		return -1;
	}

	/* 結果を格納する */
	if(wc != NULL)
		*wc = ret;

	/* 消費したオクテット数を返す */
	return (int)octets;
}

/*
 * utf-8文字列の文字数を返す
 */
int utf8_chars(const char *mbs)
{
	int count;
	int mblen;

	count = 0;
	while (*mbs != '\0') {
		mblen = utf8_to_utf32(mbs, NULL);
		if (mblen == -1)
			return -1;
		count++;
		mbs += mblen;
	}
	return count;
}

/*
 * 文字を描画した際の幅を取得する
 */
int get_glyph_width(uint32_t codepoint)
{
	jclass cls;
	jmethodID mid;
	jint ret;

	cls = (*jni_env)->FindClass(jni_env, "jp/luxion/suika/MainActivity");
	mid = (*jni_env)->GetMethodID(jni_env, cls, "getGlyphWidth", "(II)I");
	ret = (*jni_env)->CallIntMethod(jni_env, main_activity, mid, conf_font_size, codepoint);

	/* 解放しないとlocal reference tableが溢れる可能性がある */
	(*jni_env)->DeleteLocalRef(jni_env, cls);

	return (int)ret;
}

/*
 * utf-8文字列を描画した際の幅を取得する
 */
int get_utf8_width(const char *mbs)
{
	uint32_t c;
	int mblen, w;

	/* 1文字ずつ描画する */
	w = 0;
	c = 0; /* warning avoidance on gcc 5.3.1 */
	while (*mbs != '\0') {
		/* 文字を取得する */
		mblen = utf8_to_utf32(mbs, &c);
		if (mblen == -1)
			return -1;

		/* 幅を取得する */
		w += get_glyph_width(c);

		/* 次の文字へ移動する */
		mbs += mblen;
	}
	return w;
}

/*
 * 文字の描画を行う
 */
bool draw_glyph(struct image *img, int x, int y, pixel_t color,
		pixel_t outline_color, uint32_t codepoint, int *w, int *h)
{
	jclass cls;
	jmethodID mid;
	jint ret;

	cls = (*jni_env)->FindClass(jni_env, "jp/luxion/suika/MainActivity");

	mid = (*jni_env)->GetMethodID(jni_env, cls, "drawGlyph", "(Landroid/graphics/Bitmap;IIIII)I");
	ret = (*jni_env)->CallIntMethod(jni_env, main_activity, mid, get_image_object(img), x, y, conf_font_size, color, codepoint);
	*w = (int)ret;

	mid = (*jni_env)->GetMethodID(jni_env, cls, "getGlyphHeight", "(II)I");
	ret = (*jni_env)->CallIntMethod(jni_env, main_activity, mid, conf_font_size, codepoint);
	*h = (int)ret;

	/* 解放しないとlocal reference tableが溢れる可能性がある */
	(*jni_env)->DeleteLocalRef(jni_env, cls);

	return true;
}
