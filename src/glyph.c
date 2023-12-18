/* -*- coding: utf-8; indent-tabs-mode: t; tab-width: 8; c-basic-offset: 8; -*- */

/*
 * Suika2
 * Copyright (C) 2001-2023, Keiichi Tabata. All rights reserved.
 */

/*
 * Glyph rendering and text layout subsystem
 *
 * [Changes]
 *  - 2016-06-18 Created.
 *  - 2021-07-28 Add font outline.
 *  - 2023-12-08 Refactored.
 */

#include "suika.h"

/*
 * FreeType2 headers
 */
#include <ft2build.h>
#include FT_FREETYPE_H
#ifdef SUIKA_TARGET_WASM
#include <ftstroke.h>
#else
#include <freetype/ftstroke.h>
#endif

/*
 * The scale constant
 */
#define SCALE	(64)

/*
 * FreeType2 objects
 */
static FT_Library library;
static FT_Face face[FONT_COUNT];
static FT_Byte *font_file_content[FONT_COUNT];
static FT_Long font_file_size[FONT_COUNT];

/*
 * Forward declarations
 */
static bool read_font_file_content(
	const char *file_name,
	FT_Byte **content,
	FT_Long *size);
static bool draw_glyph_wrapper(
	struct image *img,
	int font,
	int font_size,
	int base_font_size,
	bool use_outline,
	int x,
	int y,
	pixel_t color,
	pixel_t outline_color,
	uint32_t wc,
	int *ret_width,
	int *ret_height,
	bool is_dimming);
static bool draw_glyph_without_outline(
	struct image *img,
	int font_type,
	int font_size,
	int base_font_size,
	int x,
	int y,
	pixel_t color,
	uint32_t codepoint,
	int *ret_w,
	int *ret_h,
	bool is_dim);
static void draw_glyph_func(
	unsigned char * RESTRICT font,
	int font_width,
	int font_height,
	int margin_left,
	int margin_top,
	pixel_t * RESTRICT image,
	int image_width,
	int image_height,
	int image_x,
	int image_y,
	pixel_t color);
static void draw_glyph_dim_func(
	unsigned char * RESTRICT font,
	int font_width,
	int font_height,
	int margin_left,
	int margin_top,
	pixel_t * RESTRICT image,
	int image_width,
	int image_height,
	int image_x,
	int image_y,
	pixel_t color);
static bool isgraph_extended(const char **mbs, uint32_t *wc);
static int translate_font_type(int font_ype);
static bool apply_font_size(int font_type, int size);

/*
 * フォントレンダラの初期化処理を行う
 */
bool init_glyph(void)
{
	const char *fname[FONT_COUNT];
	FT_Error err;
	int i;

#ifdef SUIKA_TARGET_ANDROID
	cleanup_glyph();
#endif

	/* FreeType2ライブラリを初期化する */
	err = FT_Init_FreeType(&library);
	if (err != 0) {
		log_api_error("FT_Init_FreeType");
		return false;
	}

	/* コンフィグを読み込む */
	fname[FONT_GLOBAL] = conf_font_global_file;
	fname[FONT_MAIN] = conf_font_main_file;
	fname[FONT_ALT1] = conf_font_alt1_file;
	fname[FONT_ALT2] = conf_font_alt2_file;

	/* フォントを読み込む */
	for (i = 0; i < FONT_COUNT; i++) {
		if (fname[i] == NULL)
			continue;

		/* フォントファイルの内容を読み込む */
		if (!read_font_file_content(fname[i],
					    &font_file_content[i],
					    &font_file_size[i]))
			return false;

		/* フォントファイルを読み込む */
		err = FT_New_Memory_Face(library,
					 font_file_content[i],
					 font_file_size[i],
					 0,
					 &face[i]);
		if (err != 0) {
			log_font_file_error(conf_font_global_file);
			return false;
		}
	}

	/* フォントのプリロードを行う */
	for (i = 0; i < FONT_COUNT; i++) 
		get_glyph_width(i, conf_font_size, 'A');

	return true;
}

/*
 * フォントレンダラの終了処理を行う
 */
void cleanup_glyph(void)
{
	int i;

	for (i = 0; i < FONT_COUNT; i++) {
		if (face[i] != NULL) {
			FT_Done_Face(face[i]);
			face[i] = NULL;
		}
		if (font_file_content[i] != NULL) {
			free(font_file_content[i]);
			font_file_content[i] = NULL;
		}
	}

	if (library != NULL) {
		FT_Done_FreeType(library);
		library = NULL;
	}
}

/*
 * グローバルフォントを変更する
 */
bool update_global_font(void)
{
	FT_Error err;

	assert(conf_font_global_file != NULL);

	/* Return if before init. */
	if (face[FONT_GLOBAL] == NULL)
		return true;

	/* Cleanup the current global font. */
	assert(face[FONT_GLOBAL] != NULL);
	FT_Done_Face(face[FONT_GLOBAL]);
	face[FONT_GLOBAL] = NULL;
	assert(font_file_content[FONT_GLOBAL] != NULL);
	free(font_file_content[FONT_GLOBAL]);
	font_file_content[FONT_GLOBAL] = NULL;

	/* フォントファイルの内容を読み込む */
	if (!read_font_file_content(conf_font_global_file,
				    &font_file_content[FONT_GLOBAL],
				    &font_file_size[FONT_GLOBAL]))
		return false;

	/* フォントファイルを読み込む */
	err = FT_New_Memory_Face(library,
				 font_file_content[FONT_GLOBAL],
				 font_file_size[FONT_GLOBAL],
				 0,
				 &face[FONT_GLOBAL]);
	if (err != 0) {
		log_font_file_error(conf_font_global_file);
		return false;
	}

	return true;
}

/* フォントファイルの内容を読み込む */
static bool read_font_file_content(const char *file_name,
				   FT_Byte **content,
				   FT_Long *size)
{
	struct rfile *rf;

	/* フォントファイルを開く */
	rf = open_rfile(FONT_DIR, file_name, false);
	if (rf == NULL)
		return false;

	/* フォントファイルのサイズを取得する */
	*size = (FT_Long)get_rfile_size(rf);
	if (*size == 0) {
		log_font_file_error(file_name);
		close_rfile(rf);
		return false;
	}

	/* メモリを確保する */
	*content = malloc((size_t)*size);
	if (*content == NULL) {
		log_memory();
		close_rfile(rf);
		return false;
	}

	/* ファイルの内容を読み込む */
	if (read_rfile(rf, *content, (size_t)*size) != (size_t)*size) {
		log_font_file_error(file_name);
		close_rfile(rf);
		return false;
	}
	close_rfile(rf);

	return true;
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
 * utf-8文字列のワイド文字数を返す
 */
int count_utf8_chars(const char *mbs)
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
int get_glyph_width(int font_type, int font_size, uint32_t codepoint)
{
	int w, h;

	w = h = 0;

	/* 幅を求める */
	draw_glyph(NULL, font_type, font_size, font_size, false, 0, 0, 0, 0,
		   codepoint, &w, &h, false);

	return w;
}

/*
 * 文字を描画した際の高さを取得する
 */
int get_glyph_height(int font_type, int font_size, uint32_t codepoint)
{
	int w, h;

	w = h = 0;

	/* 幅を求める */
	draw_glyph(NULL, font_type, font_size, font_size, false, 0, 0, 0, 0,
		   codepoint, &w, &h, false);

	return h;
}

/*
 * 文字列を描画した際の幅を取得する
 */
int get_string_width(int font_type, int font_size, const char *mbs)
{
	uint32_t c;
	int mblen, w;

	/* 1文字ずつ描画する */
	w = 0;
	c = 0; /* warning avoidance on gcc 5.3.1 */
	while (*mbs != '\0') {
		/* エスケープシーケンスをスキップする */
		while (*mbs == '\\') {
			if (*(mbs + 1) == 'n') {
				mbs += 2;
				continue;
			}
			while (*mbs != '\0' && *mbs != '}')
				mbs++;
			mbs++;
		}

		/* 文字を取得する */
		mblen = utf8_to_utf32(mbs, &c);
		if (mblen == -1)
			return -1;

		/* 幅を取得する */
		w += get_glyph_width(font_type, font_size, c);

		/* 次の文字へ移動する */
		mbs += mblen;
	}
	return w;
}

/*
 * 文字列を描画した際の高さを取得する
 */
int get_string_height(int font_type, int font_size, const char *mbs)
{
	uint32_t c;
	int mblen, w;

	/* 1文字ずつ描画する */
	w = 0;
	c = 0; /* warning avoidance on gcc 5.3.1 */
	while (*mbs != '\0') {
		/* エスケープシーケンスをスキップする */
		while (*mbs == '\\') {
			if (*(mbs + 1) == 'n') {
				mbs += 2;
				continue;
			}
			while (*mbs != '\0' && *mbs != '}')
				mbs++;
			mbs++;
		}

		/* 文字を取得する */
		mblen = utf8_to_utf32(mbs, &c);
		if (mblen == -1)
			return -1;

		/* 高さを取得する */
		w += get_glyph_height(font_type, font_size, c);

		/* 次の文字へ移動する */
		mbs += mblen;
	}
	return w;
}

/*
 * 文字の描画を行う
 */

/* 文字の描画を行う */
bool draw_glyph(struct image *img,
		int font_type,
		int font_size,
		int base_font_size,
		bool use_outline,
		int x,
		int y,
		pixel_t color,
		pixel_t outline_color,
		uint32_t codepoint,
		int *ret_w,
		int *ret_h,
		bool is_dim)
{
	FT_Stroker stroker;
	FT_UInt glyphIndex;
	FT_Glyph glyph;
	FT_BitmapGlyph bitmapGlyph;
	int descent;

	if (!use_outline) {
		return draw_glyph_without_outline(img, font_type, font_size,
						  base_font_size, x, y,
						  color, codepoint,
						  ret_w, ret_h, is_dim);
	}
	font_type = translate_font_type(font_type);
	apply_font_size(font_type, font_size);

	/* アウトライン(内側)を描画する */
	FT_Stroker_New(library, &stroker);
	FT_Stroker_Set(stroker, 2*64, FT_STROKER_LINECAP_ROUND, FT_STROKER_LINEJOIN_ROUND, 0);
	glyphIndex = FT_Get_Char_Index(face[font_type], codepoint);
	FT_Load_Glyph(face[font_type], glyphIndex, FT_LOAD_DEFAULT);
	FT_Get_Glyph(face[font_type]->glyph, &glyph);
	FT_Glyph_StrokeBorder(&glyph, stroker, true, true);
	FT_Glyph_To_Bitmap(&glyph, FT_RENDER_MODE_NORMAL, NULL, true);
	bitmapGlyph = (FT_BitmapGlyph)glyph;
	if (img != NULL) {
		draw_glyph_func(bitmapGlyph->bitmap.buffer,
				(int)bitmapGlyph->bitmap.width,
				(int)bitmapGlyph->bitmap.rows,
				bitmapGlyph->left,
				font_size - bitmapGlyph->top,
				img->pixels,
				img->width,
				img->height,
				x,
				y - (font_size - base_font_size),
				outline_color);
	}
	FT_Done_Glyph(glyph);
	FT_Stroker_Done(stroker);

	/* アウトライン(外側)を描画する */
	FT_Stroker_New(library, &stroker);
	FT_Stroker_Set(stroker, 2*64, FT_STROKER_LINECAP_ROUND, FT_STROKER_LINEJOIN_ROUND, 0);
	glyphIndex = FT_Get_Char_Index(face[font_type], codepoint);
	FT_Load_Glyph(face[font_type], glyphIndex, FT_LOAD_DEFAULT);
	FT_Get_Glyph(face[font_type]->glyph, &glyph);
	FT_Glyph_StrokeBorder(&glyph, stroker, false, true);
	FT_Glyph_To_Bitmap(&glyph, FT_RENDER_MODE_NORMAL, NULL, true);
	bitmapGlyph = (FT_BitmapGlyph)glyph;
	if (img != NULL) {
		draw_glyph_func(bitmapGlyph->bitmap.buffer,
				(int)bitmapGlyph->bitmap.width,
				(int)bitmapGlyph->bitmap.rows,
				bitmapGlyph->left,
				font_size - bitmapGlyph->top,
				img->pixels,
				img->width,
				img->height,
				x,
				y - (font_size - base_font_size),
				outline_color);
	}
	descent = (int)(face[font_type]->glyph->metrics.height / SCALE) -
		  (int)(face[font_type]->glyph->metrics.horiBearingY / SCALE);
	*ret_w = (int)face[font_type]->glyph->advance.x / SCALE;
	*ret_h = font_size + descent + 2;
	FT_Done_Glyph(glyph);
	FT_Stroker_Done(stroker);
	if (img == NULL)
		return true;

	/* 中身を描画する */
	glyphIndex = FT_Get_Char_Index(face[font_type], codepoint);
	FT_Load_Glyph(face[font_type], glyphIndex, FT_LOAD_DEFAULT);
	FT_Get_Glyph(face[font_type]->glyph, &glyph);
	FT_Glyph_To_Bitmap(&glyph, FT_RENDER_MODE_NORMAL, NULL, true);
	bitmapGlyph = (FT_BitmapGlyph)glyph;
	draw_glyph_func(bitmapGlyph->bitmap.buffer,
			(int)bitmapGlyph->bitmap.width,
			(int)bitmapGlyph->bitmap.rows,
			bitmapGlyph->left,
			font_size - bitmapGlyph->top,
			img->pixels,
			img->width,
			img->height,
			x,
			y - (font_size - base_font_size),
			color);
	FT_Done_Glyph(glyph);

	notify_image_update(img);

	/* 成功 */
	return true;
}

static bool draw_glyph_without_outline(struct image *img,
				       int font_type,
				       int font_size,
				       int base_font_size,
				       int x,
				       int y,
				       pixel_t color,
				       uint32_t codepoint,
				       int *ret_w,
				       int *ret_h,
				       bool is_dim)
{
	FT_Error err;
	int descent;

	font_type = translate_font_type(font_type);
	apply_font_size(font_type, font_size);

	/* 文字をグレースケールビットマップとして取得する */
	err = FT_Load_Char(face[font_type], codepoint, FT_LOAD_RENDER);
	if (err != 0) {
		log_api_error("FT_Load_Char");
		return false;
	}

	/* 文字のビットマップを対象イメージに描画する */
	if (img != NULL && !is_dim) {
		draw_glyph_func(face[font_type]->glyph->bitmap.buffer,
				(int)face[font_type]->glyph->bitmap.width,
				(int)face[font_type]->glyph->bitmap.rows,
				face[font_type]->glyph->bitmap_left,
				font_size - face[font_type]->glyph->bitmap_top,
				img->pixels,
				img->width,
				img->height,
				x,
				y - (font_size - base_font_size),
				color);
	} else if (img != NULL && is_dim) {
		draw_glyph_dim_func(face[font_type]->glyph->bitmap.buffer,
				    (int)face[font_type]->glyph->bitmap.width,
				    (int)face[font_type]->glyph->bitmap.rows,
				    face[font_type]->glyph->bitmap_left,
				    font_size - face[font_type]->glyph->bitmap_top,
				    img->pixels,
				    img->width,
				    img->height,
				    x,
				    y - (font_size - base_font_size),
				    color);
	}

	/* descentを求める */
	descent = (int)(face[font_type]->glyph->metrics.height / SCALE) -
		  (int)(face[font_type]->glyph->metrics.horiBearingY / SCALE);

	/* 描画した幅と高さを求める */
	*ret_w = (int)face[font_type]->glyph->advance.x / SCALE;
	*ret_h = font_size + descent;

	if (img != NULL)
		notify_image_update(img);

	return true;
}

/* サポートされているアルファベットか調べる */
static bool isgraph_extended(const char **mbs, uint32_t *wc)
{
	int len;

	/* 英語のアルファベットと記号の場合 */
	if (isgraph((unsigned char)**mbs)) {
		*wc = (unsigned char)**mbs;
		(*mbs)++;
		return true;
	}

	/* Unicode文字を取得する */
	len = utf8_to_utf32(*mbs, wc);
	if (len < 1)
		return false;
	*mbs += len;

	/* アクセント付きラテンアルファベットの場合 */
	if (*wc >= 0x00c0 && *wc <= 0x017f)
		return true;

	/* ギリシャ語の場合 */
	if (*wc >= 0x0370 && *wc <= 0x3ff)
		return true;

	/* ロシア語の場合 */
	if (*wc >= 0x410 && *wc <= 0x44f)
		return true;

	/* 他の言語 */
	return false;
}

/* フォントサイズを指定する */
static bool apply_font_size(int font_type, int size)
{
	FT_Error err;

	font_type = translate_font_type(font_type);

	/* 文字サイズをセットする */
	err = FT_Set_Pixel_Sizes(face[font_type], 0, (FT_UInt)size);
	if (err != 0) {
		log_api_error("FT_Set_Pixel_Sizes");
		return false;
	}
	return true;
}

/* フォントを選択する */
static int translate_font_type(int font_type)
{
	assert(font_type == FONT_GLOBAL || font_type == FONT_MAIN ||
	       font_type == FONT_ALT1 || font_type == FONT_ALT2);

	if (font_type == FONT_GLOBAL)
		return FONT_GLOBAL;
	if (font_type == FONT_MAIN) {
		if (conf_font_main_file == NULL)
			return FONT_GLOBAL;
		else
			return FONT_MAIN;
	}
	if (font_type == FONT_ALT1) {
		if (conf_font_alt1_file == NULL)
			return FONT_GLOBAL;
		else
			return FONT_ALT1;
	}
	if (font_type == FONT_ALT2) {
		if (conf_font_alt2_file == NULL)
			return FONT_GLOBAL;
		else
			return FONT_ALT2;
	}
	assert(0);
	return FONT_GLOBAL;
}

/*
 * フォントをイメージに描画する
 */
static void draw_glyph_func(unsigned char *font,
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
	unsigned char *src_ptr, src_pix;
	pixel_t *dst_ptr, dst_pix, dst_aa;
	float color_r, color_g, color_b;
	float src_a, src_r, src_g, src_b;
	float dst_a, dst_r, dst_g, dst_b;
	int image_real_x, image_real_y;
	int font_real_x, font_real_y;
	int font_real_width, font_real_height;
	int px, py;

	/* 完全に描画しない場合のクリッピングを行う */
	if (image_x + margin_left + font_width < 0)
		return;
	if (image_x + margin_left >= image_width)
		return;
	if (image_y + margin_top + font_height < 0)
		return;
	if (image_y + margin_top > image_height)
		return;

	/* 部分的に描画しない場合のクリッピングを行う */
	image_real_x = image_x + margin_left;
	image_real_y = image_y + margin_top;
	font_real_x = 0;
	font_real_y = 0;
	font_real_width = font_width;
	font_real_height = font_height;
	if (image_real_x < 0) {
		font_real_x -= image_real_x;
		font_real_width += image_real_x;
		image_real_x = 0;
	}
	if (image_real_x + font_real_width >= image_width) {
		font_real_width -= (image_real_x + font_real_width) -
				   image_width;
	}
	if (image_real_y < 0) {
		font_real_y -= image_real_y;
		font_real_height += image_real_y;
		image_real_y = 0;
	}
	if (image_real_y + font_real_height >= image_height) {
		font_real_height -= (image_real_y + font_real_height) -
				    image_height;
	}

	/* 描画する */
	color_r = (float)get_pixel_r(color);
	color_g = (float)get_pixel_g(color);
	color_b = (float)get_pixel_b(color);
	dst_ptr = image + image_real_y * image_width + image_real_x;
	src_ptr = font + font_real_y * font_width + font_real_x;
	for (py = font_real_y; py < font_real_y + font_real_height; py++) {
		for (px = font_real_x; px < font_real_x + font_real_width;
		     px++) {
			/* アルファ値を計算する */
			src_pix = *src_ptr++;
			src_a = src_pix / 255.0f;
			dst_a = 1.0f - src_a;

			/* 色にアルファ値を乗算する */
			src_r = src_a * color_r;
			src_g = src_a * color_g;
			src_b = src_a * color_b;

			/* 転送先ピクセルにアルファ値を乗算する */
			dst_pix	= *dst_ptr;
			dst_r = dst_a * (float)get_pixel_r(dst_pix);
			dst_g = dst_a * (float)get_pixel_g(dst_pix);
			dst_b = dst_a * (float)get_pixel_b(dst_pix);

			/* 転送先ピクセルのアルファ値を求める */
			dst_aa = src_pix + get_pixel_a(dst_pix);
			dst_aa = dst_aa >= 255 ? 255 : dst_aa;

			/* 転送先に格納する */
			*dst_ptr++ = make_pixel(dst_aa,
						(uint32_t)(src_r + dst_r),
						(uint32_t)(src_g + dst_g),
						(uint32_t)(src_b + dst_b));
		}
		dst_ptr += image_width - font_real_width;
		src_ptr += font_width - font_real_width;
	}
}

/*
 * フォントをイメージに描画する(dim用)
 */
static void draw_glyph_dim_func(unsigned char *font,
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
	unsigned char *src_ptr, src_pix;
	pixel_t *dst_ptr, dst_pix, dst_aa;
	float color_r, color_g, color_b;
	float src_a, src_r, src_g, src_b;
	float dst_a, dst_r, dst_g, dst_b;
	int image_real_x, image_real_y;
	int font_real_x, font_real_y;
	int font_real_width, font_real_height;
	int px, py;

	/* 完全に描画しない場合のクリッピングを行う */
	if (image_x + margin_left + font_width < 0)
		return;
	if (image_x + margin_left >= image_width)
		return;
	if (image_y + margin_top + font_height < 0)
		return;
	if (image_y + margin_top > image_height)
		return;

	/* 部分的に描画しない場合のクリッピングを行う */
	image_real_x = image_x + margin_left;
	image_real_y = image_y + margin_top;
	font_real_x = 0;
	font_real_y = 0;
	font_real_width = font_width;
	font_real_height = font_height;
	if (image_real_x < 0) {
		font_real_x -= image_real_x;
		font_real_width += image_real_x;
		image_real_x = 0;
	}
	if (image_real_x + font_real_width >= image_width) {
		font_real_width -= (image_real_x + font_real_width) -
				   image_width;
	}
	if (image_real_y < 0) {
		font_real_y -= image_real_y;
		font_real_height += image_real_y;
		image_real_y = 0;
	}
	if (image_real_y + font_real_height >= image_height) {
		font_real_height -= (image_real_y + font_real_height) -
				    image_height;
	}

	/* 描画する */
	color_r = (float)get_pixel_r(color);
	color_g = (float)get_pixel_g(color);
	color_b = (float)get_pixel_b(color);
	dst_ptr = image + image_real_y * image_width + image_real_x;
	src_ptr = font + font_real_y * font_width + font_real_x;
	for (py = font_real_y; py < font_real_y + font_real_height; py++) {
		for (px = font_real_x; px < font_real_x + font_real_width;
		     px++) {
			/* アルファ値を計算する */
			src_pix = *src_ptr++;
			src_a = src_pix > 0 ? 1.0f : 0.0f;
			dst_a = 1.0f - src_a;

			/* 色にアルファ値を乗算する */
			src_r = src_a * color_r;
			src_g = src_a * color_g;
			src_b = src_a * color_b;

			/* 転送先ピクセルにアルファ値を乗算する */
			dst_pix	= *dst_ptr;
			dst_r = dst_a * (float)get_pixel_r(dst_pix);
			dst_g = dst_a * (float)get_pixel_g(dst_pix);
			dst_b = dst_a * (float)get_pixel_b(dst_pix);

			/* 転送先ピクセルのアルファ値を求める */
			dst_aa = src_pix + get_pixel_a(dst_pix);
			dst_aa = dst_aa >= 255 ? 255 : dst_aa;

			/* 転送先に格納する */
			*dst_ptr++ = make_pixel(dst_aa,
						(uint32_t)(src_r + dst_r),
						(uint32_t)(src_g + dst_g),
						(uint32_t)(src_b + dst_b));
		}
		dst_ptr += image_width - font_real_width;
		src_ptr += font_width - font_real_width;
	}
}

/*
 * Text layout and drawing
 */

/* Forward declarations. */
static void process_escape_sequence(struct draw_msg_context *context);
static void process_escape_sequence_lf(struct draw_msg_context *context);
static bool process_escape_sequence_font(struct draw_msg_context *context);
static bool process_escape_sequence_outline(struct draw_msg_context *context);
static bool process_escape_sequence_color(struct draw_msg_context *context);
static bool process_escape_sequence_size(struct draw_msg_context *context);
static bool process_escape_sequence_wait(struct draw_msg_context *context);
static bool process_escape_sequence_pen(struct draw_msg_context *context);
static bool process_escape_sequence_ruby(struct draw_msg_context *context);
static bool search_for_end_of_escape_sequence(const char **msg);
static bool do_word_wrapping(struct draw_msg_context *context);
static int get_en_word_width(struct draw_msg_context *context);
static uint32_t convert_tategaki_char(uint32_t wc);
static bool is_tategaki_punctuation(uint32_t wc);
static bool process_lf(struct draw_msg_context *context, uint32_t c,
		       int glyph_width, int glyph_height);
static bool is_small_kana(uint32_t wc);

/*
 * Initialize a message drawing context.
 *  - Too many parameters, but for now, I think it's useful to detect bugs
 *    as compile errors when we add a change on the design.
 */
void construct_draw_msg_context(
	struct draw_msg_context *context,
	int stage_layer,
	const char *msg,
	int font,
	int font_size,
	int base_font_size,
	int ruby_size,
	bool use_outline,
	int pen_x,
	int pen_y,
	int area_width,
	int area_height,
	int left_margin,
	int right_margin,
	int top_margin,
	int bottom_margin,
	int line_margin,
	int char_margin,
	pixel_t color,
	pixel_t outline_color,
	bool is_dimming,
	bool ignore_linefeed,
	bool ignore_font,
	bool ignore_outline,
	bool ignore_color,
	bool ignore_size,
	bool ignore_position,
	bool ignore_ruby,
	bool ignore_wait,
	void (*inline_wait_hook)(float),
	bool use_tategaki)
{
	context->stage_layer = stage_layer;
	context->msg = msg;
	context->font = font;
	context->font_size = font_size;
	context->base_font_size = base_font_size;
	context->ruby_size = ruby_size;
	context->use_outline = use_outline;
	context->pen_x = pen_x;
	context->pen_y = pen_y;
	context->area_width = area_width;
	context->area_height = area_height;
	context->left_margin = left_margin;
	context->right_margin = right_margin;
	context->top_margin = top_margin;
	context->bottom_margin = bottom_margin;
	context->line_margin = line_margin;
	context->char_margin = char_margin;
	context->color = color;
	context->outline_color = outline_color;
	context->is_dimming = is_dimming;
	context->ignore_linefeed = ignore_linefeed;
	context->ignore_font = ignore_font;
	context->ignore_outline = ignore_outline;
	context->ignore_color = ignore_color;
	context->ignore_size = ignore_size;
	context->ignore_position = ignore_position;
	context->ignore_ruby = ignore_ruby;
	context->ignore_wait = ignore_wait;
	context->inline_wait_hook = inline_wait_hook;
	context->use_tategaki = use_tategaki;

	/* Get a layer image. */
	if (stage_layer != -1) {
		context->layer_image = get_layer_image(stage_layer);
		context->layer_x = get_layer_x(stage_layer);
		context->layer_y = get_layer_y(stage_layer);
	} else {
		context->layer_x = 0;
		context->layer_y = 0;
	}

	/* The first character is treated as after-space. */
	context->runtime_is_after_space = true;

	/* Set zeros. */
	context->runtime_is_inline_wait = false;
	context->runtime_ruby_x = 0;
	context->runtime_ruby_y = 0;
}

/*
 * Set an alternative target image.
 */
void set_alternative_target_image(struct draw_msg_context *context,
				  struct image *img)
{
	context->layer_image = img;
}

/*
 * エスケープシーケンスを除いた描画文字数を取得する
 *  - Unicodeの合成はサポートしていない
 *  - 基底文字+結合文字はそれぞれ1文字としてカウントする
 */
int count_chars_common(struct draw_msg_context *context)
{
	const char *msg;
	uint32_t wc;
	int count, mblen;

	count = 0;
	msg = context->msg;
	while (*msg) {
		/* 先頭のエスケープシーケンスを読み飛ばす */
		while (*msg == '\\') {
			switch (*(msg + 1)) {
			case 'n':	/* 改行 */
				msg += 2;
				break;
			case 'f':	/* フォント指定 */
			case 'o':	/* アウトライン指定 */
			case '#':	/* 色指定 */
			case '@':	/* サイズ指定 */
			case 'w':	/* インラインウェイト */
			case 'p':	/* ペン移動 */
			case '^':	/* ルビ */
				if (!search_for_end_of_escape_sequence(&msg))
					return count;
				break;
			default:
				/*
				 * 不正なエスケープシーケンス
				 *  - 読み飛ばさない
				 */
				return count;
			}
		}
		if (*msg == '\0')
			break;

		/* 次の1文字を取得する */
		mblen = utf8_to_utf32(msg, &wc);
		if (mblen == -1)
			break;

		msg += mblen;
		count++;
	}

	return count;
}

/* エスケープシーケンスの終了位置までポインタをインクリメントする */
static bool search_for_end_of_escape_sequence(const char **msg)
{
	const char *s;
	int len;

	s = *msg;
	len = 0;
	while (*s != '\0' && *s != '}') {
		s++;
		len++;
	}
	if (*s == '\0')
		return false;

	*msg += len + 1;
	return true;
}

/*
 * Draw characters in a message up to (max_chars) characters.
 */
int
draw_msg_common(
	struct draw_msg_context *context,	/* a drawing context. */
	int char_count)				/* characters to draw. */
{
	uint32_t wc = 0;
	int i, mblen;
	int glyph_width, glyph_height, ofs_x, ofs_y;
	int ret_width = 0, ret_height = 0;

	context->font = translate_font_type(context->font);
	apply_font_size(context->font, context->font_size);

	if (char_count == -1)
		char_count = count_chars_common(context);

	/* 1文字ずつ描画する */
	for (i = 0; i < char_count; i++) {
		if (*context->msg == '\0')
			break;

		/* 先頭のエスケープシーケンスをすべて処理する */
		process_escape_sequence(context);
		if (context->runtime_is_inline_wait) {
			context->runtime_is_inline_wait = false;
			return i;
		}

		/* ワードラッピングを処理する */
		if (!do_word_wrapping(context))
			return i;

		/* 描画する文字を取得する */
		mblen = utf8_to_utf32(context->msg, &wc);
		if (mblen == -1) {
			/* Invalid utf-8 sequence. */
			return -1;
		}

		/* 縦書きの句読点変換を行う */
		if (context->use_tategaki)
			wc = convert_tategaki_char(wc);

		/* 描画する文字の幅と高さを取得する */
		glyph_width = get_glyph_width(context->font, context->font_size, wc);
		glyph_height = get_glyph_height(context->font, context->font_size, wc);

		/* 右側の幅が足りなければ改行する */
		if (!process_lf(context, wc, glyph_width, glyph_height))
			return i;

		/* 小さいひらがな/カタカタのオフセットを計算する */
		if (context->use_tategaki && is_small_kana(wc)) {
			/* FIXME: 何らかの調整を加える */
			ofs_x = 0;
			ofs_y = 0;
		} else {
			ofs_x = 0;
			ofs_y = 0;
		}

		/* 描画する */
		draw_glyph_wrapper(context->layer_image,
				   context->font,
				   context->font_size,
				   context->base_font_size,
				   context->use_outline,
				   context->pen_x + ofs_x,
				   context->pen_y + ofs_y,
				   context->color,
				   context->outline_color,
				   wc,
				   &ret_width,
				   &ret_height,
				   context->is_dimming);

		/* ルビ用のペン位置を更新する */
		if (!context->use_tategaki) {
			context->runtime_ruby_x = context->pen_x;
			context->runtime_ruby_y = context->pen_y -
				context->ruby_size;
		} else {
			context->runtime_ruby_x = context->pen_x + ret_width;
			context->runtime_ruby_y = context->pen_y;
		}

		/* 次の文字へ移動する */
		context->msg += mblen;
		if (!context->use_tategaki) {
			context->pen_x += glyph_width + context->char_margin;
		} else {
			if (is_tategaki_punctuation(wc))
				context->pen_y += context->font_size;
			else
				context->pen_y += glyph_height;
			context->pen_y += context->char_margin;
		}
	}

	/* 末尾のエスケープシーケンスを処理する */
	process_escape_sequence(context);
	if (context->runtime_is_inline_wait)
		context->runtime_is_inline_wait = false;

	/* 描画した文字数を返す */
	return i;
}

/* ワードラッピングを処理する */
static bool do_word_wrapping(struct draw_msg_context *context)
{
	if (context->use_tategaki)
		return true;

	if (context->runtime_is_after_space) {
		if (context->pen_x + get_en_word_width(context) >=
		    context->area_width - context->right_margin) {
			if (context->ignore_linefeed)
				return false;

			context->pen_y += context->line_margin;
			context->pen_x = context->left_margin;
		}
	}

	context->runtime_is_after_space = *context->msg == ' ';

	return true;
}

/* msgが英単語の先頭であれば、その単語の描画幅、それ以外の場合0を返す */
static int get_en_word_width(struct draw_msg_context *context)
{
	const char *m;
	uint32_t wc;
	int width;

	m = context->msg;
	width = 0;
	while (isgraph_extended(&m, &wc))
		width += get_glyph_width(context->font, context->font_size, wc);

	return width;
}

/* 右側の幅が足りなければ改行する */
static bool process_lf(struct draw_msg_context *context, uint32_t c,
		       int glyph_width, int glyph_height)
{
	if (!context->use_tategaki) {
		/* 右側の幅が足りる場合、改行しない */
		if (context->pen_x + glyph_width + context->char_margin <
		    context->area_width - context->right_margin)
			return true;
	} else {
		/* 下側の幅が足りる場合、改行しない */
		if (context->pen_y + glyph_height + context->char_margin <
		    context->area_height - context->bottom_margin)
			return true;
	}

	/* 禁則文字の場合、改行しない */
	if (c == ' ' || c == ',' || c == '.' || c == ':' || c == ';' ||
	    c == CHAR_TOUTEN || c == CHAR_KUTEN)
		return true;

	if (context->ignore_linefeed)
		return false;

	/* 改行する */
	if (!context->use_tategaki) {
		context->pen_y += context->line_margin;
		context->pen_x = context->left_margin;
	} else {
		context->pen_x -= context->line_margin;
		context->pen_y = context->top_margin;
	}

	return true;
}

/* 縦書きの句読点変換を行う */
static uint32_t convert_tategaki_char(uint32_t wc)
{
	switch (wc) {
	case U32_C('、'): return U32_C('︑');
	case U32_C('，'): return U32_C('︐');
	case U32_C('。'): return U32_C('︒');
	case U32_C('（'): return U32_C('︵');
	case U32_C('）'): return U32_C('︶');
	case U32_C('｛'): return U32_C('︷');
	case U32_C('｝'): return U32_C('︸');
	case U32_C('「'): return U32_C('﹁');
	case U32_C('」'): return U32_C('﹂');
	case U32_C('『'): return U32_C('﹃');
	case U32_C('』'): return U32_C('﹄');
	case U32_C('【'): return U32_C('︻');
	case U32_C('】'): return U32_C('︼');
	case U32_C('［'): return U32_C('﹇');
	case U32_C('］'): return U32_C('﹈');
	case U32_C('〔'): return U32_C('︹');
	case U32_C('〕'): return U32_C('︺');
	case U32_C('…'): return U32_C('︙');
	case U32_C('‥'): return U32_C('︰');
	case U32_C('ー'): return U32_C('丨');
	default:
		break;
	}
	return wc;
}

/* 縦書きの句読点かどうか調べる */
static bool is_tategaki_punctuation(uint32_t wc)
{
	switch (wc) {
	case U32_C('︑'): return true;
	case U32_C('︐'): return true;
	case U32_C('︒'): return true;
	case U32_C('︵'): return true;
	case U32_C('︶'): return true;
	case U32_C('︷'): return true;
	case U32_C('︸'): return true;
	case U32_C('﹁'): return true;
	case U32_C('﹂'): return true;
	case U32_C('﹃'): return true;
	case U32_C('﹄'): return true;
	case U32_C('︻'): return true;
	case U32_C('︼'): return true;
	case U32_C('﹇'): return true;
	case U32_C('﹈'): return true;
	case U32_C('︹'): return true;
	case U32_C('︺'): return true;
	case U32_C('︙'): return true;
	case U32_C('︰'): return true;
	case U32_C('丨'): return true;
	default:
		break;
	}
	return false;
}

/* 小さい仮名文字であるか調べる */
static bool is_small_kana(uint32_t wc)
{
	switch (wc) {
	case U32_C('ぁ'): return true;
	case U32_C('ぃ'): return true;
	case U32_C('ぅ'): return true;
	case U32_C('ぇ'): return true;
	case U32_C('ぉ'): return true;
	case U32_C('っ'): return true;
	case U32_C('ゃ'): return true;
	case U32_C('ゅ'): return true;
	case U32_C('ょ'): return true;
	case U32_C('ゎ'): return true;
	case U32_C('ゕ'): return true;
	case U32_C('ゖ'): return true;
	case U32_C('ァ'): return true;
	case U32_C('ィ'): return true;
	case U32_C('ゥ'): return true;
	case U32_C('ェ'): return true;
	case U32_C('ォ'): return true;
	case U32_C('ッ'): return true;
	case U32_C('ャ'): return true;
	case U32_C('ュ'): return true;
	case U32_C('ョ'): return true;
	case U32_C('ヮ'): return true;
	case U32_C('ヵ'): return true;
	case U32_C('ヶ'): return true;
	default: break;
	}
	return false;
}

/* 先頭のエスケープシーケンスを処理する */
static void process_escape_sequence(struct draw_msg_context *context)
{
	/* エスケープシーケンスが続く限り処理する */
	while (*context->msg == '\\') {
		switch (*(context->msg + 1)) {
		case 'n':
			/* 改行 */
			process_escape_sequence_lf(context);
			break;
		case 'f':
			/* フォント指定 */
			if (!process_escape_sequence_font(context))
				return; /* 不正: 読み飛ばさない */
			break;
		case 'o':
			/* アウトライン指定 */
			if (!process_escape_sequence_outline(context))
				return; /* 不正: 読み飛ばさない */
			break;
		case '#':
			/* 色指定 */
			if (!process_escape_sequence_color(context))
				return; /* 不正: 読み飛ばさない */
			break;
		case '@':
			/* サイズ指定 */
			if (!process_escape_sequence_size(context))
				return; /* 不正: 読み飛ばさない */
			break;
		case 'w':
			/* インラインウェイト */
			if (!process_escape_sequence_wait(context))
				return; /* 不正: 読み飛ばさない */
			break;
		case 'p':
			/* ペン移動 */
			if (!process_escape_sequence_pen(context))
				return; /* 不正: 読み飛ばさない */
			break;
		case '^':
			/* ルビ */
			if (!process_escape_sequence_ruby(context))
				return; /* 不正: 読み飛ばさない */
			break;
		default:
			/* 不正なエスケープシーケンスなので読み飛ばさない */
			return;
		}
	}
}

/* 改行("\\n")を処理する */
static void process_escape_sequence_lf(struct draw_msg_context *context)
{
	if (context->ignore_linefeed) {
		context->msg += 2;
		return;
	}

	if (!context->use_tategaki) {
		context->pen_y += context->line_margin;
		context->pen_x = context->left_margin;
	} else {
		context->pen_x -= context->line_margin;
		context->pen_y = context->top_margin;
	}
	context->msg += 2;
}

/* フォント指定("\\f{X}")を処理する */
static bool process_escape_sequence_font(struct draw_msg_context *context)
{
	char font_type;
	const char *p;

	p = context->msg;
	assert(*p == '\\');
	assert(*(p + 1) == 'f');

	/* '{'をチェックする */
	if (*(p + 2) != '{')
		return false;

	/* 長さが足りない場合 */
	if (strlen(p + 3) < 6)
		return false;

	/* '}'をチェックする */
	if (*(p + 4) != '}')
		return false;

	if (!context->ignore_font) {
		/* フォントタイプを読む */
		font_type = *(p + 3);
		switch (font_type) {
		case 'g':
			context->font = FONT_GLOBAL;
			break;
		case 'm':
			context->font = translate_font_type(FONT_MAIN);
			break;
		case 'a':
			context->font = translate_font_type(FONT_ALT1);
			break;
		case 'b':
			context->font = translate_font_type(FONT_ALT2);
			break;
		default:
			break;
		}
	}

	/* "\\#{" + "X" + "}" */
	context->msg += 3 + 1 + 1;
	return true;
}

/* アウトライン指定("\\o{X}")を処理する */
static bool process_escape_sequence_outline(struct draw_msg_context *context)
{
	char outline_type;
	const char *p;

	p = context->msg;
	assert(*p == '\\');
	assert(*(p + 1) == 'o');

	/* '{'をチェックする */
	if (*(p + 2) != '{') {
		log_memory();
		return false;
	}

	/* 長さが足りない場合 */
	if (*(p + 3) == '\0') {
		log_memory();
		return false;
	}

	/* '}'をチェックする */
	if (*(p + 4) != '}') {
		log_memory();
		return false;
	}

	if (!context->ignore_outline) {
		/* アウトラインタイプを読む */
		outline_type = *(p + 3);
		switch (outline_type) {
		case '+':
			context->use_outline = true;
			break;
		case '-':
			context->use_outline = false;
			break;
		default:
			break;
		}
	}

	/* "\\o{" + "X" + "}" */
	context->msg += 3 + 1 + 1;
	return true;
}

/* 色指定("\\#{RRGGBB}")を処理する */
static bool process_escape_sequence_color(struct draw_msg_context *context)
{
	char color_code[7];
	const char *p;
	uint32_t r, g, b;
	int rgb;

	p = context->msg;
	assert(*p == '\\');
	assert(*(p + 1) == '#');

	/* '{'をチェックする */
	if (*(p + 2) != '{')
		return false;

	/* 長さが足りない場合 */
	if (strlen(p + 3) < 6)
		return false;

	/* '}'をチェックする */
	if (*(p + 9) != '}')
		return false;

	if (!context->ignore_color) {
		/* カラーコードを読む */
		memcpy(color_code, p + 3, 6);
		color_code[6] = '\0';
		rgb = 0;
		sscanf(color_code, "%x", &rgb);
		r = (rgb >> 16) & 0xff;
		g = (rgb >> 8) & 0xff;
		b = rgb & 0xff;
		context->color = make_pixel(0xff, r, g, b);
	}

	/* "\\#{" + "RRGGBB" + "}" */
	context->msg += 3 + 6 + 1;
	return true;
}

/* サイズ指定("\\@{xxx}")を処理する */
static bool process_escape_sequence_size(struct draw_msg_context *context)
{
	char size_spec[8];
	const char *p;
	int i, size;

	p = context->msg;
	assert(*p == '\\');
	assert(*(p + 1) == '@');

	/* '{'をチェックする */
	if (*(p + 2) != '{')
		return false;

	/* サイズ文字列を読む */
	for (i = 0; i < (int)sizeof(size_spec) - 1; i++) {
		if (*(p + 3 + i) == '\0')
			return false;
		if (*(p + 3 + i) == '}')
			break;
		size_spec[i] = *(p + 3 + i);
	}
	size_spec[i] = '\0';

	if (!context->ignore_size) {
		/* サイズ文字列を整数に変換する */
		size = 0;
		sscanf(size_spec, "%d", &size);

		/* フォントサイズを変更する */
		context->font_size = size;
	}

	/* "\\@{" + "xxx" + "}" */
	context->msg += 3 + i + 1;
	return true;
}

/* インラインウェイト("\\w{f.f}")を処理する */
static bool process_escape_sequence_wait(struct draw_msg_context *context)
{
	char time_spec[16];
	const char *p;
	float wait_time;
	int i;

	p = context->msg;
	assert(*p == '\\');
	assert(*(p + 1) == 'w');

	/* '{'をチェックする */
	if (*(p + 2) != '{')
		return false;

	/* 時間文字列を読む */
	for (i = 0; i < (int)sizeof(time_spec) - 1; i++) {
		if (*(p + 3 + i) == '\0')
			return false;
		if (*(p + 3 + i) == '}')
			break;
		time_spec[i] = *(p + 3 + i);
	}
	time_spec[i] = '\0';

	if (!context->ignore_wait) {
		/* 時間文字列を浮動小数点数に変換する */
		sscanf(time_spec, "%f", &wait_time);

		/* ウェイトを処理する */
		context->runtime_is_inline_wait = true;
		context->inline_wait_hook(wait_time);
	}

	/* "\\w{" + "f.f" + "}" */
	context->msg += 3 + i + 1;
	return true;
}

/* ペン移動("\\p{x,y}")を処理する */
static bool process_escape_sequence_pen(struct draw_msg_context *context)
{
	char pos_spec[32];
	const char *p;
	int i, pen_x, pen_y;
	bool separator_found;

	p = context->msg;
	assert(*p == '\\');
	assert(*(p + 1) == 'p');

	/* '{'をチェックする */
	if (*(p + 2) != '{')
		return false;

	/* 座標文字列を読む */
	separator_found = false;
	for (i = 0; i < (int)sizeof(pos_spec) - 1; i++) {
		if (*(p + 3 + i) == '\0')
			return false;
		if (*(p + 3 + i) == '}')
			break;
		if (*(p + 3 + i) == ',')
			separator_found = true;
		pos_spec[i] = *(p + 3 + i);
	}
	pos_spec[i] = '\0';
	if (!separator_found)
		return false;

	if (!context->ignore_position) {
		/* 座標文字列を浮動小数点数に変換する */
		sscanf(pos_spec, "%d,%d", &pen_x, &pen_y);

		/* 描画位置を更新する */
		context->pen_x = pen_x;
		context->pen_y = pen_y;
	}

	/* "\\w{" + "x,y" + "}" */
	context->msg += 3 + i + 1;
	return true;
}

/* ルビ("\\^{ルビ}")を処理する */
static bool process_escape_sequence_ruby(struct draw_msg_context *context)
{
	char ruby[64];
	const char *p;
	uint32_t wc = 0;
	int i, mblen, ret_w, ret_h;

	p = context->msg;
	assert(*p == '\\');
	assert(*(p + 1) == '^');

	/* '{'をチェックする */
	if (*(p + 2) != '{')
		return false;

	/* ルビを読む */
	for (i = 0; i < (int)sizeof(ruby) - 1; i++) {
		if (*(p + 3 + i) == '\0')
			return false;
		if (*(p + 3 + i) == '}')
			break;
		ruby[i] = *(p + 3 + i);
	}
	ruby[i] = '\0';

	/* \^{ + ruby[] + } */
	context->msg += 3 + i + 1;

	if (context->ignore_ruby)
		return true;

	/* 描画する */
	p = ruby;
	while (*p) {
		mblen = utf8_to_utf32(p, &wc);
		if (mblen == -1)
			return false;

		draw_glyph_wrapper(context->layer_image,
				   context->font,
				   context->ruby_size,
				   context->ruby_size,
				   context->use_outline,
				   context->runtime_ruby_x,
				   context->runtime_ruby_y,
				   context->color,
				   context->outline_color,
				   wc,
				   &ret_w,
				   &ret_h,
				   context->is_dimming);

		if (!context->use_tategaki)
			context->runtime_ruby_x += ret_w;
		else
			context->runtime_ruby_y += ret_h;

		p += mblen;
	}

	return true;
}

static bool draw_glyph_wrapper(
	struct image *img,
	int font, int font_size, int base_font_size, bool use_outline,
	int x, int y, pixel_t color, pixel_t outline_color,
	uint32_t wc, int *ret_width, int *ret_height,
	bool is_dimming)
{
	bool ret;

	/* 文字を描画する */
	ret = draw_glyph(
		img,
		font,
		font_size,
		base_font_size,
		use_outline,
		x,
		y,
		color,
		outline_color,
		wc,
		ret_width,
		ret_height,
		is_dimming);
	if (!ret) {
		/* グリフがない、コードポイントがおかしい、など */
		return false;
	}

	return true;
}

/*
 * Get a pen position.
 */
void get_pen_position_common(struct draw_msg_context *context, int *pen_x,
			     int *pen_y)
{
	*pen_x = context->pen_x;
	*pen_y = context->pen_y;
}
