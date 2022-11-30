/* -*- coding: utf-8; indent-tabs-mode: t; tab-width: 8; c-basic-offset: 8; -*- */

/*
 * Suika 2
 * Copyright (C) 2001-2016, TABATA Keiichi. All rights reserved.
 */

/*
 * グレースケール文字レンダリング
 *
 * [Changes]
 *  - 2016/06/19 作成
 */

/*
 * フォントをイメージに描画する
 */
void DRAW_GLYPH_FUNC(unsigned char * RESTRICT font,
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
#ifdef PROTOTYPE_ONLY
;
#else
{
	unsigned char * RESTRICT src_ptr, src_pix;
	pixel_t * RESTRICT dst_ptr, dst_pix, dst_aa;
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
	color_r = (float)get_pixel_c1(color);
	color_g = (float)get_pixel_c2(color);
	color_b = (float)get_pixel_c3(color);
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
			dst_r = dst_a * (float)get_pixel_c1(dst_pix);
			dst_g = dst_a * (float)get_pixel_c2(dst_pix);
			dst_b = dst_a * (float)get_pixel_c3(dst_pix);

			/* 転送先ピクセルのアルファ値を求める */
			dst_aa = src_pix + get_pixel_a(dst_pix);
			dst_aa = dst_aa >= 255 ? 255 : dst_aa;

			/* 転送先に格納する */
			*dst_ptr++ = make_pixel_fast(
				dst_aa,
				(uint32_t)(src_r + dst_r),
				(uint32_t)(src_g + dst_g),
				(uint32_t)(src_b + dst_b));
		}
		dst_ptr += image_width - font_real_width;
		src_ptr += font_width - font_real_width;
	}
}
#endif

#undef DRAW_GLYPH_FUNC
#undef PROTOTYPE_ONLY
