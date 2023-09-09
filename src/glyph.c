/* -*- coding: utf-8; indent-tabs-mode: t; tab-width: 8; c-basic-offset: 8; -*- */

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

/* グローバルセーブデータのフォントファイル名 */
static char *global_font_file;

/* ローカルセーブデータのメインフォントファイル名 */
static char *main_font_file;

/* ローカルセーブデータのalt1フォントファイル名 */
static char *alt1_font_file;

/* ローカルセーブデータのalt2フォントファイル名 */
static char *alt2_font_file;

/* フォントタイプからフォントファイル名への変換テーブル */
static char **font_file_name_tbl[FONT_COUNT] = {
	&global_font_file,
	&main_font_file,
	&alt1_font_file,
	&alt2_font_file,
};

/* FreeType2のオブジェクト */
static FT_Library library;
static FT_Face face[FONT_COUNT];
static FT_Byte *font_file_content[FONT_COUNT];
static FT_Long font_file_size[FONT_COUNT];

/* 選択されたフォント */
static int selected_font;

/* フォントサイズ */
static int font_size;

/* フォントのふちどり */
static bool is_outline_enabled;

/*
 * 前方参照
 */
static bool read_font_file_content(const char *file_name, FT_Byte **content,
				   FT_Long *size);
static bool draw_glyph_without_outline(struct image *img, int x, int y,
				       pixel_t color, uint32_t codepoint,
				       int *w, int *h, int base_font_size,
				       bool is_dim);
static void draw_glyph_func(unsigned char * RESTRICT font, int font_width,
			    int font_height, int margin_left, int margin_top,
			    pixel_t * RESTRICT image, int image_width,
			    int image_height, int image_x, int image_y,
			    pixel_t color);
static void draw_glyph_dim_func(unsigned char * RESTRICT font,
				int font_width, int font_height,
				int margin_left, int margin_top,
				pixel_t * RESTRICT image, int image_width,
				int image_height, int image_x, int image_y,
				pixel_t color);

/*
 * フォントレンダラの初期化処理を行う
 */
bool init_glyph(void)
{
	FT_Error err;
	int i;

#ifdef ANDROID
	/* Android用 */
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
#endif

	/* FreeType2ライブラリを初期化する */
	err = FT_Init_FreeType(&library);
	if (err != 0) {
		log_api_error("FT_Init_FreeType");
		return false;
	}

	/* フォントを読み込む */
	for (i = 0; i < FONT_COUNT; i++) {
		if (*font_file_name_tbl[i] == NULL)
			continue;

		/* フォントファイルの内容を読み込む */
		if (!read_font_file_content(
			    *font_file_name_tbl[i],
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
			log_font_file_error(*font_file_name_tbl[i]);
			return false;
		}
	}

	return true;
}

/*
 * フォントレンダラの終了処理を行う
 */
void cleanup_glyph(bool no_free_file_names)
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

	if (!no_free_file_names) {
		for (i = 0; i < FONT_COUNT; i++) {
			if (*font_file_name_tbl[i] != NULL) {
				free(*font_file_name_tbl[i]);
				*font_file_name_tbl[i] = NULL;
			}
		}
	}
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
	int w, h;

	w = h = 0;

	/* 幅を求める */
	draw_glyph(NULL, 0, 0, 0, 0, codepoint, &w, &h, font_size, false);

	return w;
}

/*
 * 文字を描画した際の高さを取得する
 */
int get_glyph_height(uint32_t codepoint)
{
	int w, h;

	w = h = 0;

	/* 幅を求める */
	draw_glyph(NULL, 0, 0, 0, 0, codepoint, &w, &h, font_size, false);

	return h;
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
		pixel_t outline_color, uint32_t codepoint, int *w, int *h,
		int base_font_size, bool is_dim)
{
	FT_Stroker stroker;
	FT_UInt glyphIndex;
	FT_Glyph glyph;
	FT_BitmapGlyph bitmapGlyph;
	int descent;

	if (conf_font_outline_remove) {
		return draw_glyph_without_outline(img, x, y, color, codepoint,
						  w, h, base_font_size,
						  is_dim);
	}

	/* アウトライン(内側)を描画する */
	FT_Stroker_New(library, &stroker);
	FT_Stroker_Set(stroker, 2*64, FT_STROKER_LINECAP_ROUND, FT_STROKER_LINEJOIN_ROUND, 0);
	glyphIndex = FT_Get_Char_Index(face[selected_font], codepoint);
	FT_Load_Glyph(face[selected_font], glyphIndex, FT_LOAD_DEFAULT);
	FT_Get_Glyph(face[selected_font]->glyph, &glyph);
	FT_Glyph_StrokeBorder(&glyph, stroker, true, true);
	FT_Glyph_To_Bitmap(&glyph, FT_RENDER_MODE_NORMAL, NULL, true);
	bitmapGlyph = (FT_BitmapGlyph)glyph;
	if (img != NULL) {
		draw_glyph_func(bitmapGlyph->bitmap.buffer,
				(int)bitmapGlyph->bitmap.width,
				(int)bitmapGlyph->bitmap.rows,
				bitmapGlyph->left,
				font_size - bitmapGlyph->top,
				get_image_pixels(img),
				get_image_width(img),
				get_image_height(img),
				x,
				y - (font_size - base_font_size),
				outline_color);
	}
	FT_Done_Glyph(glyph);
	FT_Stroker_Done(stroker);

	/* アウトライン(外側)を描画する */
	FT_Stroker_New(library, &stroker);
	FT_Stroker_Set(stroker, 2*64, FT_STROKER_LINECAP_ROUND, FT_STROKER_LINEJOIN_ROUND, 0);
	glyphIndex = FT_Get_Char_Index(face[selected_font], codepoint);
	FT_Load_Glyph(face[selected_font], glyphIndex, FT_LOAD_DEFAULT);
	FT_Get_Glyph(face[selected_font]->glyph, &glyph);
	FT_Glyph_StrokeBorder(&glyph, stroker, false, true);
	FT_Glyph_To_Bitmap(&glyph, FT_RENDER_MODE_NORMAL, NULL, true);
	bitmapGlyph = (FT_BitmapGlyph)glyph;
	if (img != NULL) {
		draw_glyph_func(bitmapGlyph->bitmap.buffer,
				(int)bitmapGlyph->bitmap.width,
				(int)bitmapGlyph->bitmap.rows,
				bitmapGlyph->left,
				font_size - bitmapGlyph->top,
				get_image_pixels(img),
				get_image_width(img),
				get_image_height(img),
				x,
				y - (font_size - base_font_size),
				outline_color);
	}
	descent = (int)(face[selected_font]->glyph->metrics.height / SCALE) -
		  (int)(face[selected_font]->glyph->metrics.horiBearingY / SCALE);
	*w = (int)face[selected_font]->glyph->advance.x / SCALE;
	*h = font_size + descent + 2;
	FT_Done_Glyph(glyph);
	FT_Stroker_Done(stroker);
	if (img == NULL)
		return true;

	/* 中身を描画する */
	glyphIndex = FT_Get_Char_Index(face[selected_font], codepoint);
	FT_Load_Glyph(face[selected_font], glyphIndex, FT_LOAD_DEFAULT);
	FT_Get_Glyph(face[selected_font]->glyph, &glyph);
	FT_Glyph_To_Bitmap(&glyph, FT_RENDER_MODE_NORMAL, NULL, true);
	bitmapGlyph = (FT_BitmapGlyph)glyph;
	draw_glyph_func(bitmapGlyph->bitmap.buffer,
			(int)bitmapGlyph->bitmap.width,
			(int)bitmapGlyph->bitmap.rows,
			bitmapGlyph->left,
			font_size - bitmapGlyph->top,
			get_image_pixels(img),
			get_image_width(img),
			get_image_height(img),
			x,
			y - (font_size - base_font_size),
			color);
	FT_Done_Glyph(glyph);

	/* 成功 */
	return true;
}

static bool draw_glyph_without_outline(struct image *img, int x, int y,
				       pixel_t color, uint32_t codepoint,
				       int *w, int *h, int base_font_size,
				       bool is_dim)
{
	FT_Error err;
	int descent;

	/* 文字をグレースケールビットマップとして取得する */
	err = FT_Load_Char(face[selected_font], codepoint, FT_LOAD_RENDER);
	if (err != 0) {
		log_api_error("FT_Load_Char");
		return false;
	}

	/* 文字のビットマップを対象イメージに描画する */
	if (img != NULL && !is_dim) {
		draw_glyph_func(face[selected_font]->glyph->bitmap.buffer,
				(int)face[selected_font]->glyph->bitmap.width,
				(int)face[selected_font]->glyph->bitmap.rows,
				face[selected_font]->glyph->bitmap_left,
				font_size - face[selected_font]->glyph->bitmap_top,
				get_image_pixels(img),
				get_image_width(img),
				get_image_height(img),
				x,
				y - (font_size - base_font_size),
				color);
	} else if (img != NULL && is_dim) {
		draw_glyph_dim_func(face[selected_font]->glyph->bitmap.buffer,
				    (int)face[selected_font]->glyph->bitmap.width,
				    (int)face[selected_font]->glyph->bitmap.rows,
				    face[selected_font]->glyph->bitmap_left,
				    font_size - face[selected_font]->glyph->bitmap_top,
				    get_image_pixels(img),
				    get_image_width(img),
				    get_image_height(img),
				    x,
				    y - (font_size - base_font_size),
				    color);
	}

	/* descentを求める */
	descent = (int)(face[selected_font]->glyph->metrics.height / SCALE) -
		  (int)(face[selected_font]->glyph->metrics.horiBearingY / SCALE);

	/* 描画した幅と高さを求める */
	*w = (int)face[selected_font]->glyph->advance.x / SCALE;
	*h = font_size + descent;

	return true;
}

/*
 * グローバルのフォントファイル名を設定する
 *  - init_glyph()よりも前に呼ばれる
 *  - 最初のinit_glyph()のあとで反映するには、cleanup_glyph(true), init_glyph() の
 *    順で呼び出す
 */
bool set_global_font_file_name(const char *file)
{
	if (global_font_file != NULL) {
		free(global_font_file);
		global_font_file = NULL;
	}

	if (file != NULL) {
		global_font_file = strdup(file);
		if (global_font_file == NULL) {
			log_memory();
			return false;
		}
	}

	return true;
}

/*
 * グローバルのフォントファイル名を取得する
 */
const char *get_global_font_file_name(void)
{
	assert(global_font_file != NULL);
	return global_font_file;
}

/*
 * ローカルのフォントファイル名を設定する
 *  - init_glyph()よりも前に呼ばれることはない
 *  - 反映するには、cleanup_glyph(true), init_glyph() の順で呼び出す
 */
bool set_local_font_file_name(int type, const char *file)
{
	assert(type != FONT_GLOBAL);
	assert(type == FONT_MAIN || type == FONT_ALT1 || type == FONT_ALT2);

	if (*font_file_name_tbl[type] != NULL) {
		free(*font_file_name_tbl[type]);
		*font_file_name_tbl[type] = NULL;
	}

	if (file != NULL) {
		*font_file_name_tbl[type] = strdup(file);
		if (*font_file_name_tbl[type] == NULL) {
			log_memory();
			return false;
		}
	}

	return true;
}

/*
 * ローカルのフォントファイル名を取得する
 */
const char *get_local_font_file_name(int type)
{
	assert(type != FONT_GLOBAL);
	assert(type == FONT_MAIN || type == FONT_ALT1 || type == FONT_ALT2);

	return *font_file_name_tbl[type];
}

/*
 * サポートされているアルファベットか調べる
 */
bool isgraph_extended(const char **mbs, uint32_t *wc)
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

/*
 * フォントサイズを指定する
 */
bool set_font_size(int size)
{
	FT_Error err;

	font_size = size;

	/* 文字サイズをセットする */
	err = FT_Set_Pixel_Sizes(face[selected_font], 0, (FT_UInt)size);
	if (err != 0) {
		log_api_error("FT_Set_Pixel_Sizes");
		return false;
	}
	return true;
}

/*
 * フォントサイズを取得する
 */
int get_font_size(void)
{
	return font_size;
}

/*
 * フォントを選択する
 */
void select_font(int type)
{
	assert(type == FONT_GLOBAL || type == FONT_MAIN ||
	       type == FONT_ALT1 || type == FONT_ALT2);

	if (type == FONT_GLOBAL) {
		selected_font = FONT_GLOBAL;
		return;
	}
	if (type == FONT_MAIN) {
		if (main_font_file == NULL)
			selected_font = FONT_GLOBAL;
		else
			selected_font = FONT_MAIN;
		return;
	}
	if (type == FONT_ALT1) {
		if (alt1_font_file == NULL)
			selected_font = FONT_GLOBAL;
		else
			selected_font = FONT_ALT1;
		return;
	}
	if (type == FONT_ALT2) {
		if (alt2_font_file == NULL)
			selected_font = FONT_GLOBAL;
		else
			selected_font = FONT_ALT2;
		return;
	}
	assert(0);
}

/*
 * フォントのふちどりの有無を設定する
 */
void set_font_outline(bool is_enabled)
{
	is_outline_enabled = is_enabled;
}

/*
 * SSEバージョニングを行わない場合
 */
#ifndef SSE_VERSIONING

#define DRAW_GLYPH_FUNC draw_glyph_func
#define DRAW_GLYPH_DIM_FUNC draw_glyph_dim_func
#include "drawglyph.h"

/*
 * SSEバージョニングを行う場合
 */
#else

/* AVX-512版の描画関数を宣言する */
#define PROTOTYPE_ONLY
#define DRAW_GLYPH_FUNC draw_glyph_func_avx512
#define DRAW_GLYPH_DIM_FUNC draw_glyph_dim_func_avx512
#include "drawglyph.h"

/* AVX2版の描画関数を宣言する */
#define PROTOTYPE_ONLY
#define DRAW_GLYPH_FUNC draw_glyph_func_avx2
#define DRAW_GLYPH_DIM_FUNC draw_glyph_dim_func_avx2
#include "drawglyph.h"

/* AVX版の描画関数を宣言する */
#define PROTOTYPE_ONLY
#define DRAW_GLYPH_FUNC draw_glyph_func_avx
#define DRAW_GLYPH_DIM_FUNC draw_glyph_dim_func_avx
#include "drawglyph.h"

#if !defined(_MSC_VER)

/* SSE4.2版の描画関数を宣言する */
#define PROTOTYPE_ONLY
#define DRAW_GLYPH_FUNC draw_glyph_func_sse42
#define DRAW_GLYPH_DIM_FUNC draw_glyph_dim_func_sse42
#include "drawglyph.h"

/* SSE4.1版の描画関数を宣言する */
#define PROTOTYPE_ONLY
#define DRAW_GLYPH_FUNC draw_glyph_func_sse41
#define DRAW_GLYPH_DIM_FUNC draw_glyph_dim_func_sse41
#include "drawglyph.h"

/* SSE3版の描画関数を宣言する */
#define PROTOTYPE_ONLY
#define DRAW_GLYPH_FUNC draw_glyph_func_sse3
#define DRAW_GLYPH_DIM_FUNC draw_glyph_dim_func_sse3
#include "drawglyph.h"

#endif /* !defined(_MSC_VER) */

/* SSE2版の描画関数を宣言する */
#define PROTOTYPE_ONLY
#define DRAW_GLYPH_FUNC draw_glyph_func_sse2
#define DRAW_GLYPH_DIM_FUNC draw_glyph_dim_func_sse2
#include "drawglyph.h"

/* SSE版の描画関数を宣言する */
#define PROTOTYPE_ONLY
#define DRAW_GLYPH_FUNC draw_glyph_func_sse
#define DRAW_GLYPH_DIM_FUNC draw_glyph_dim_func_sse
#include "drawglyph.h"

/* 非ベクトル化版の描画関数を宣言する */
#define PROTOTYPE_ONLY
#define DRAW_GLYPH_FUNC draw_glyph_func_novec
#define DRAW_GLYPH_DIM_FUNC draw_glyph_dim_func_novec
#include "drawglyph.h"

/* draw_glyph_func()をディスパッチする */
static void draw_glyph_func(unsigned char * RESTRICT font,
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

/* draw_glyph_dim_func()をディスパッチする */
static void draw_glyph_dim_func(unsigned char * RESTRICT font,
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
		draw_glyph_dim_func_avx512(font, font_width, font_height,
					   margin_left, margin_top, image,
					   image_width, image_height, image_x,
					   image_y, color);
	} else if (has_avx2) {
		draw_glyph_dim_func_avx2(font, font_width, font_height,
					 margin_left, margin_top, image,
					 image_width, image_height, image_x,
					 image_y, color);
	} else if (has_avx) {
		draw_glyph_dim_func_avx(font, font_width, font_height,
					margin_left, margin_top, image,
					image_width, image_height, image_x,
					image_y, color);
#if !defined(_MSC_VER)
	} else if (has_sse42) {
		draw_glyph_dim_func_sse42(font, font_width, font_height,
					  margin_left, margin_top, image,
					  image_width, image_height, image_x,
					  image_y, color);
	} else if (has_sse41) {
		draw_glyph_dim_func_sse41(font, font_width, font_height,
					  margin_left, margin_top, image,
					  image_width, image_height, image_x,
					  image_y, color);
	} else if (has_sse3) {
		draw_glyph_dim_func_sse3(font, font_width, font_height,
					 margin_left, margin_top, image,
					 image_width, image_height, image_x,
					 image_y, color);
#endif
	} else if (has_sse2) {
		draw_glyph_dim_func_sse2(font, font_width, font_height,
					 margin_left, margin_top, image,
					 image_width, image_height, image_x,
					 image_y, color);
	} else if (has_sse) {
		draw_glyph_dim_func_sse(font, font_width, font_height,
					margin_left, margin_top, image,
					image_width, image_height, image_x,
					image_y, color);
	} else {
		draw_glyph_dim_func_novec(font, font_width, font_height,
					  margin_left, margin_top, image,
					  image_width, image_height, image_x,
					  image_y, color);
	}
}

#endif
