/* -*- coding: utf-8-with-signature; indent-tabs-mode: t; tab-width: 8; c-basic-offset: 8; -*- */

/*
 * Suika 2
 * Copyright (C) 2001-2021, TABATA Keiichi. All rights reserved.
 */

/*
 * freetype2 レンダリング
 *
 * [Changes]
 *  - 2016/06/18 作成
 *  - 2021/07/28 フォントのアウトラインを描画するように変更
 */

#include "suika.h"

#ifdef SSE_VERSIONING
#include "x86.h"
#endif

#include <ft2build.h>
#include FT_FREETYPE_H
#ifdef EM
#include <ftstroke.h>
#else
#include <freetype/ftstroke.h>
#endif

#define SCALE	(64)

static FT_Library library;
static FT_Face face;
static FT_Byte *font_file_content;
static FT_Long font_file_size;

/*
 * 前方参照
 */
static bool read_font_file_content(void);
static void draw_glyph_func(unsigned char * RESTRICT font, int font_width,
			    int font_height, int margin_left, int margin_top,
			    pixel_t * RESTRICT image, int image_width,
			    int image_height, int image_x, int image_y,
			    pixel_t color);

/*
 * フォントレンダラの初期化処理を行う
 */
bool init_glyph(void)
{
	FT_Error err;

	/* FreeType2ライブラリを初期化する */
	err = FT_Init_FreeType(&library);
	if (err != 0) {
		log_api_error("FT_Init_FreeType");
		return false;
	}

	/* フォントファイルの内容を読み込む */
	if (!read_font_file_content())
		return false;
	
	/* フォントファイルを読み込む */
	err = FT_New_Memory_Face(library, font_file_content, font_file_size,
				 0, &face);
	if (err != 0) {
		log_font_file_error(conf_font_file);
		return false;
	}

	/* 文字サイズをセットする */
	err = FT_Set_Pixel_Sizes(face, 0, (FT_UInt)conf_font_size);
	if (err != 0) {
		log_api_error("FT_Set_Pixel_Sizes");
		return false;
	}

	/* 成功 */
	return true;
}

/* フォントファイルの内容を読み込む */
static bool read_font_file_content(void)
{
	struct rfile *rf;
	FT_Long remain, block;

	/* フォントファイルを開く */
	rf = open_rfile(FONT_DIR, conf_font_file, false);
	if (rf == NULL)
		return false;

	/* フォントファイルのサイズを取得する */
	font_file_size = (FT_Long)get_rfile_size(rf);
	if (font_file_size == 0) {
		log_font_file_error(conf_font_file);
		close_rfile(rf);
		return false;
	}

	/* メモリを確保する */
	font_file_content = malloc((size_t)font_file_size);
	if (font_file_content == NULL) {
		log_memory();
		close_rfile(rf);
		return false;
	}

	/* ファイルの内容を読み込む */
	remain = font_file_size;
	while (remain > 0) {
		block = (FT_Long)read_rfile(rf, font_file_content,
					    (size_t)remain);
		if (block == 0)
			break;
		assert(block <= remain);
		remain -= block;
	}
	if (remain > 0) {
		log_font_file_error(conf_font_file);
		close_rfile(rf);
		return false;
	}
	close_rfile(rf);

	return true;
}

/*
 * フォントレンダラの終了処理を行う
 */
void cleanup_glyph(void)
{
	FT_Done_Face(face);
	face = NULL;

	FT_Done_FreeType(library);
	library = NULL;

	if (font_file_content != NULL) {
		free(font_file_content);
		font_file_content = NULL;
	}
}

/*
 * utf-8文字列の先頭文字をutf-32文字に変換する
 * FIXME: サロゲートペア、合字
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
	else if((mbs[0] & 0xe0) == 0xc0)
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
	FT_Error err;

	/* 文字をロードする */
	/* FIXME: FT_LOAD_RENDER以外のオプションにできるか？ */
	err = FT_Load_Char(face, codepoint, FT_LOAD_RENDER);
	if (err != 0) {
		log_api_error("FT_Load_Char");
		return -1;
	}

	/* 幅を求める */
	return (int)face->glyph->advance.x / SCALE;
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
	FT_Stroker stroker;
	FT_UInt glyphIndex;
	FT_Glyph glyph;
	FT_BitmapGlyph bitmapGlyph;
	int descent;

	/* アウトラインを描画する Draw outline. */
	FT_Stroker_New(library, &stroker);
	FT_Stroker_Set(stroker, 2*64, FT_STROKER_LINECAP_ROUND, FT_STROKER_LINEJOIN_ROUND, 0);
	glyphIndex = FT_Get_Char_Index(face, codepoint);
	FT_Load_Glyph(face, glyphIndex, FT_LOAD_DEFAULT);
	FT_Get_Glyph(face->glyph, &glyph);
	FT_Glyph_StrokeBorder(&glyph, stroker, false, true);
	FT_Glyph_To_Bitmap(&glyph, FT_RENDER_MODE_NORMAL, NULL, true);
	bitmapGlyph = (FT_BitmapGlyph)glyph;
	draw_glyph_func(bitmapGlyph->bitmap.buffer,
			(int)bitmapGlyph->bitmap.width,
			(int)bitmapGlyph->bitmap.rows,
			bitmapGlyph->left,
			conf_font_size - bitmapGlyph->top,
			get_image_pixels(img),
			get_image_width(img),
			get_image_height(img),
			x,
			y,
			outline_color);

	/* descentを求める */
	descent = (int)(face->glyph->metrics.height / SCALE) -
		  (int)(face->glyph->metrics.horiBearingY / SCALE);

	/* 描画した幅と高さを求める */
	*w = (int)face->glyph->advance.x / SCALE;
	*h = conf_font_size + descent + 2;

	/* 中身を描画する Draw body. */
	FT_Stroker_New(library, &stroker);
	FT_Stroker_Set(stroker, 2*64, FT_STROKER_LINECAP_ROUND, FT_STROKER_LINEJOIN_ROUND, 0);
	glyphIndex = FT_Get_Char_Index(face, codepoint);
	FT_Load_Glyph(face, glyphIndex, FT_LOAD_DEFAULT);
	FT_Get_Glyph(face->glyph, &glyph);
	FT_Glyph_To_Bitmap(&glyph, FT_RENDER_MODE_NORMAL, NULL, true);
	bitmapGlyph = (FT_BitmapGlyph)glyph;
	draw_glyph_func(bitmapGlyph->bitmap.buffer,
			(int)bitmapGlyph->bitmap.width,
			(int)bitmapGlyph->bitmap.rows,
			bitmapGlyph->left,
			conf_font_size - bitmapGlyph->top,
			get_image_pixels(img),
			get_image_width(img),
			get_image_height(img),
			x,
			y,
			color);

	/* 成功 */
	return true;
}

#if 0
static bool draw_glyph_old(struct image *img, int x, int y, pixel_t color,
			   uint32_t codepoint, int *w, int *h)
{
	FT_Error err;
	int descent;

	/* 文字をグレースケールビットマップとして取得する */
	err = FT_Load_Char(face, codepoint, FT_LOAD_RENDER);
	if (err != 0) {
		log_api_error("FT_Load_Char");
		return false;
	}

	/* 文字のビットマップを対象イメージに描画する */
	if (img != NULL) {
		draw_glyph_func(face->glyph->bitmap.buffer,
				(int)face->glyph->bitmap.width,
				(int)face->glyph->bitmap.rows,
				face->glyph->bitmap_left,
				conf_font_size - face->glyph->bitmap_top,
				get_image_pixels(img),
				get_image_width(img),
				get_image_height(img),
				x,
				y,
				color);
	}

	/* descentを求める */
	descent = (int)(face->glyph->metrics.height / SCALE) -
		  (int)(face->glyph->metrics.horiBearingY / SCALE);

	/* 描画した幅と高さを求める */
	*w = (int)face->glyph->advance.x / SCALE;
	*h = conf_font_size + descent;
}
#endif

/*
 * SSEバージョニングを行わない場合
 */
#ifndef SSE_VERSIONING

#define DRAW_GLYPH_FUNC draw_glyph_func
#include "drawglyph.h"

/*
 * SSEバージョニングを行う場合
 */
#else

/* AVX-512版の描画関数を宣言する */
#define PROTOTYPE_ONLY
#define DRAW_GLYPH_FUNC draw_glyph_func_avx512
#include "drawglyph.h"

/* AVX2版の描画関数を宣言する */
#define PROTOTYPE_ONLY
#define DRAW_GLYPH_FUNC draw_glyph_func_avx2
#include "drawglyph.h"

/* AVX版の描画関数を宣言する */
#define PROTOTYPE_ONLY
#define DRAW_GLYPH_FUNC draw_glyph_func_avx
#include "drawglyph.h"

#if !defined(_MSC_VER)

/* SSE4.2版の描画関数を宣言する */
#define PROTOTYPE_ONLY
#define DRAW_GLYPH_FUNC draw_glyph_func_sse42
#include "drawglyph.h"

/* SSE4.1版の描画関数を宣言する */
#define PROTOTYPE_ONLY
#define DRAW_GLYPH_FUNC draw_glyph_func_sse41
#include "drawglyph.h"

/* SSE3版の描画関数を宣言する */
#define PROTOTYPE_ONLY
#define DRAW_GLYPH_FUNC draw_glyph_func_sse3
#include "drawglyph.h"

#endif /* !defined(_MSC_VER) */

/* SSE2版の描画関数を宣言する */
#define PROTOTYPE_ONLY
#define DRAW_GLYPH_FUNC draw_glyph_func_sse2
#include "drawglyph.h"

/* SSE版の描画関数を宣言する */
#define PROTOTYPE_ONLY
#define DRAW_GLYPH_FUNC draw_glyph_func_sse
#include "drawglyph.h"

/* 非ベクトル化版の描画関数を宣言する */
#define PROTOTYPE_ONLY
#define DRAW_GLYPH_FUNC draw_glyph_func_novec
#include "drawglyph.h"

/* draw_glyph_func()をディスパッチする */
void draw_glyph_func(unsigned char * RESTRICT font,
		     int font_width,
		     int font_height,
		     int margin_left,
		     int margin_top,
		     pixel_t * RESTRICT image,
		     int image_width,
		     int image_height,
		     int image_x,
		     int image_y,
		     pixel_t color)
{
	if (has_avx512) {
		draw_glyph_func_avx512(font, font_width, font_height,
				       margin_left, margin_top, image,
				       image_width, image_height, image_x,
				       image_y, color);
	} else if (has_avx2) {
		draw_glyph_func_avx2(font, font_width, font_height,
				     margin_left, margin_top, image,
				     image_width, image_height, image_x,
				     image_y, color);
	} else if (has_avx) {
		draw_glyph_func_avx(font, font_width, font_height,
				    margin_left, margin_top, image,
				    image_width, image_height, image_x,
				    image_y, color);
#if !defined(_MSC_VER)
	} else if (has_sse42) {
		draw_glyph_func_sse42(font, font_width, font_height,
				      margin_left, margin_top, image,
				      image_width, image_height, image_x,
				      image_y, color);
	} else if (has_sse41) {
		draw_glyph_func_sse41(font, font_width, font_height,
				      margin_left, margin_top, image,
				      image_width, image_height, image_x,
				      image_y, color);
	} else if (has_sse3) {
		draw_glyph_func_sse3(font, font_width, font_height,
				     margin_left, margin_top, image,
				     image_width, image_height, image_x,
				     image_y, color);
#endif
	} else if (has_sse2) {
		draw_glyph_func_sse2(font, font_width, font_height,
				     margin_left, margin_top, image,
				     image_width, image_height, image_x,
				     image_y, color);
	} else if (has_sse) {
		draw_glyph_func_sse(font, font_width, font_height,
				    margin_left, margin_top, image,
				    image_width, image_height, image_x,
				    image_y, color);
	} else {
		draw_glyph_func_novec(font, font_width, font_height,
				      margin_left, margin_top, image,
				      image_width, image_height, image_x,
				      image_y, color);
	}
}

#endif
