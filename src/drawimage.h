/* -*- coding: utf-8; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Suika 2
 * Copyright (c) 2001-2016, TABATA Keiichi. All rights reserved.
 */

/*
 * [Changes]
 *  2016-06-11 作成
 */

/*
 * 下記のマクロを定義してインクルードする
 *  - DRAW_BLEND_NONE
 *  - DRAW_BLEND_FAST
 *  - DRAW_BLEND_NORMAL
 *  - DRAW_BLEND_ADD
 *  - DRAW_BLEND_SUB
 */

/*
 * そのままコピーする描画関数
 */
void DRAW_BLEND_NONE(
	struct image * RESTRICT dst_image,
	int dst_left,
	int dst_top,
	struct image * RESTRICT src_image,
	int width,
	int height,
	int src_left,
	int src_top)
#ifdef PROTOTYPE_ONLY
;
#else
{
	pixel_t * RESTRICT src_ptr, * RESTRICT dst_ptr;
	int x, y, sw, dw;

	sw = get_image_width(src_image);
	dw = get_image_width(dst_image);
	src_ptr = get_image_pixels(src_image) + sw * src_top + src_left;
	dst_ptr = get_image_pixels(dst_image) + dw * dst_top + dst_left;

	for(y = 0; y < height; y++) {
		for(x = 0; x < width; x++)
			*(dst_ptr + x) = *(src_ptr + x);
		src_ptr += sw;
		dst_ptr += dw;
	}
}
#endif

/*
 * 高速なアルファ合成による描画関数
 *  - 描画先のアルファ値は計算されずに255で一定となる
 */
void DRAW_BLEND_FAST(
	struct image * RESTRICT dst_image,
	int dst_left,
	int dst_top,
	struct image * RESTRICT src_image,
	int width,
	int height,
	int src_left,
	int src_top,
	int alpha)
#ifdef PROTOTYPE_ONLY
;
#else
{
	pixel_t * RESTRICT src_ptr, * RESTRICT dst_ptr;
	float a, src_r, src_g, src_b, src_a, dst_r, dst_g, dst_b, dst_a;
	uint32_t src_pix, dst_pix;
	int src_line_inc, dst_line_inc, x, y, sw, dw;

	sw = get_image_width(src_image);
	dw = get_image_width(dst_image);
	src_ptr = get_image_pixels(src_image) + sw * src_top + src_left;
	dst_ptr = get_image_pixels(dst_image) + dw * dst_top + dst_left;
	src_line_inc = sw - width;
	dst_line_inc = dw - width;
	a = (float)alpha / 255.0f;

	for(y = 0; y < height; y++) {
		for(x = 0; x < width; x++) {
			/* 転送元と転送先のピクセルを取得する */
			src_pix	= *src_ptr++;
			dst_pix	= *dst_ptr;

			/* アルファ値を計算する */
			src_a = a * ((float)get_pixel_a(src_pix) / 255.0f);
			dst_a = 1.0f - src_a;

			/* 転送元ピクセルにアルファ値を乗算する */
			src_r = src_a * (float)get_pixel_c1(src_pix);
			src_g = src_a * (float)get_pixel_c2(src_pix);
			src_b = src_a * (float)get_pixel_c3(src_pix);

			/* 転送先ピクセルにアルファ値を乗算する */
			dst_r = dst_a * (float)get_pixel_c1(dst_pix);
			dst_g = dst_a * (float)get_pixel_c2(dst_pix);
			dst_b = dst_a * (float)get_pixel_c3(dst_pix);

			/* 転送先に格納する */
			*dst_ptr++ = make_pixel_fast(
				0xff,
				(uint32_t)(src_r + dst_r),
				(uint32_t)(src_g + dst_g),
				(uint32_t)(src_b + dst_b));
		}
		src_ptr += src_line_inc;
		dst_ptr += dst_line_inc;
	}
}
#endif

/*
 * 標準的なアルファ合成による描画関数
 */
void DRAW_BLEND_NORMAL(
	struct image * RESTRICT dst_image,
	int dst_left,
	int dst_top,
	struct image * RESTRICT src_image,
	int width,
	int height,
	int src_left,
	int src_top,
	int alpha)
#ifdef PROTOTYPE_ONLY
;
#else
{
	pixel_t * RESTRICT src_ptr, * RESTRICT dst_ptr;
	float a, pix_a, src_r, src_g, src_b, dst_r, dst_g, dst_b;
	uint32_t src_pix, dst_pix, src_a, dst_a, add_a;
	int src_line_inc, dst_line_inc, x, y, sw, dw;

	sw = get_image_width(src_image);
	dw = get_image_width(dst_image);
	src_ptr = get_image_pixels(src_image) + sw * src_top + src_left;
	dst_ptr = get_image_pixels(dst_image) + dw * dst_top + dst_left;
	src_line_inc = sw - width;
	dst_line_inc = dw - width;
	a = (float)alpha / 255.0f;

	for(y = 0; y < height; y++) {
		for(x = 0; x < width; x++) {
			/* 転送元と転送先のピクセルを取得する */
			src_pix	= *src_ptr++;
			dst_pix	= *dst_ptr;

			/* アルファ値を求める */
			src_a = get_pixel_a(src_pix);
			dst_a = get_pixel_a(dst_pix);
			pix_a = (float)src_a / 255.0f * a;

			/* 転送元ピクセルにアルファ値を乗算する */
			src_r = pix_a * (float)get_pixel_c1(src_pix);
			src_g = pix_a * (float)get_pixel_c2(src_pix);
			src_b = pix_a * (float)get_pixel_c3(src_pix);

			/* 転送先ピクセルにアルファ値を乗算する */
			dst_r = (1.0f - pix_a) * (float)get_pixel_c1(dst_pix);
			dst_g = (1.0f - pix_a) * (float)get_pixel_c2(dst_pix);
			dst_b = (1.0f - pix_a) * (float)get_pixel_c3(dst_pix);

			/* A値の飽和加算を行う */
			add_a = src_a + dst_a > 255 ? 255 : src_a + dst_a;

			/* 転送先に格納する */
			*dst_ptr++ = make_pixel_fast(
				(pixel_t)add_a,
				(pixel_t)(src_r + dst_r),
				(pixel_t)(src_g + dst_g),
				(pixel_t)(src_b + dst_b));
		}
		src_ptr += src_line_inc;
		dst_ptr += dst_line_inc;
	}
}
#endif

/*
 * 加算ブレンドによる描画関数
 */
void DRAW_BLEND_ADD(
	struct image * RESTRICT dst_image,
	int dst_left,
	int dst_top,
	struct image * RESTRICT src_image,
	int width,
	int height,
	int src_left,
	int src_top,
	int alpha)
#ifdef PROTOTYPE_ONLY
;
#else
{
	pixel_t * RESTRICT src_ptr, * RESTRICT dst_ptr;
	float a, pix_a;
	uint32_t src_pix, dst_pix;
	uint32_t src_r, src_g, src_b, src_a;
	uint32_t dst_r, dst_g, dst_b, dst_a;
	uint32_t sadd_r, sadd_g, sadd_b, sadd_a;
	int src_line_inc, dst_line_inc, x, y, sw, dw;

	sw = get_image_width(src_image);
	dw = get_image_width(dst_image);
	src_ptr = get_image_pixels(src_image) + sw * src_top + src_left;
	dst_ptr = get_image_pixels(dst_image) + dw * dst_top + dst_left;
	src_line_inc = sw - width;
	dst_line_inc = dw - width;
	a = (float)alpha / 255.0f;

	for(y = 0; y < height; y++) {
		for(x = 0; x < width; x++, dst_ptr++) {
			/* 転送元ピクセルを取得する */
			src_pix	= *src_ptr++;
			src_a = get_pixel_a(src_pix);
			pix_a = (float)src_a / 255.0f * a;

			/* 転送元ピクセルにアルファ値を乗算する */
			src_r = (uint32_t)(pix_a *
			    (float)get_pixel_c1(src_pix));
			src_g = (uint32_t)(pix_a *
			    (float)get_pixel_c2(src_pix));
			src_b = (uint32_t)(pix_a *
			    (float)get_pixel_c3(src_pix));

			/* 転送先ピクセルを取得する */
			dst_pix	= *dst_ptr;
			dst_r = get_pixel_c1(dst_pix);
			dst_g = get_pixel_c2(dst_pix);
			dst_b = get_pixel_c3(dst_pix);
			dst_a = get_pixel_a(dst_pix);

			/* RGBA各値の飽和加算を行う */
			sadd_r = src_r + dst_r;
			sadd_r |= (-(int)(sadd_r >> 8)) & 0xff;
			sadd_g = src_g + dst_g;
			sadd_g |= (-(int)(sadd_g >> 8)) & 0xff;
			sadd_b = src_b + dst_b;
			sadd_b |= (-(int)(sadd_b >> 8)) & 0xff;
			sadd_a = src_a + dst_a;
			sadd_a |= (-(int)(sadd_a >> 8)) & 0xff;

			/* 転送先に格納する */
			*dst_ptr = make_pixel_fast(sadd_a, sadd_r, sadd_g,
						   sadd_b);
		}
		src_ptr += src_line_inc;
		dst_ptr += dst_line_inc;
	}
}
#endif

/*
 * 減算ブレンドによる描画関数
 */
void DRAW_BLEND_SUB(
	struct image * RESTRICT dst_image,
	int dst_left,
	int dst_top,
	struct image * RESTRICT src_image,
	int width,
	int height,
	int src_left,
	int src_top,
	int alpha)
#ifdef PROTOTYPE_ONLY
;
#else
{
	pixel_t * RESTRICT src_ptr, * RESTRICT dst_ptr;
	float a, pix_a;
	uint32_t src_pix, dst_pix;
	uint32_t src_r, src_g, src_b, src_a;
	uint32_t dst_r, dst_g, dst_b, dst_a;
	uint32_t sadd_r, sadd_g, sadd_b, sadd_a;
	int src_line_inc, dst_line_inc, x, y, sw, dw;

	sw = get_image_width(src_image);
	dw = get_image_width(dst_image);
	src_ptr = get_image_pixels(src_image) + sw * src_top + src_left;
	dst_ptr = get_image_pixels(dst_image) + dw * dst_top + dst_left;
	src_line_inc = sw - width;
	dst_line_inc = dw - width;
	a = (float)alpha / 255.0f;

	for(y = 0; y < height; y++) {
		for(x = 0; x < width; x++, dst_ptr++) {
			/* 転送元ピクセルとそのアルファ値を取得する */
			src_pix	= *src_ptr++;
			src_a = get_pixel_a(src_pix);
			pix_a = (float)src_a / 255.0f * a;

			/* 転送元ピクセルにアルファ値を乗算する */
			src_r = (uint32_t)(pix_a *
			    (float)get_pixel_c1(src_pix));
			src_g = (uint32_t)(pix_a *
			    (float)get_pixel_c2(src_pix));
			src_b = (uint32_t)(pix_a *
			    (float)get_pixel_c3(src_pix));

			/* 転送先ピクセルのRGB各値を取得する */
			dst_pix	= *dst_ptr;
			dst_r = get_pixel_c1(dst_pix);
			dst_g = get_pixel_c2(dst_pix);
			dst_b = get_pixel_c3(dst_pix);
			dst_a = get_pixel_a(dst_pix);

			/* RGB各値の飽和減算と、A値の飽和加算を行う */
			sadd_r = dst_r - src_r;
			sadd_r &= (sadd_r >> 8) ^ 0xff;
			sadd_g = dst_g - src_g;
			sadd_g &= (sadd_g >> 8) ^ 0xff;
			sadd_b = dst_b - src_b;
			sadd_b &= (sadd_b >> 8) ^ 0xff;
			sadd_a = dst_a + src_a;
			sadd_a |= (-(int)(sadd_r >> 8)) & 0xff;

			/* 転送先に格納する */
			*dst_ptr = make_pixel_fast(sadd_a, sadd_r, sadd_g,
						   sadd_b);
		}
		src_ptr += src_line_inc;
		dst_ptr += dst_line_inc;
	}
}
#endif

#undef DRAW_BLEND_NONE
#undef DRAW_BLEND_FAST
#undef DRAW_BLEND_NORMAL
#undef DRAW_BLEND_ADD
#undef DRAW_BLEND_SUB
#undef PROTOTYPE_ONLY
