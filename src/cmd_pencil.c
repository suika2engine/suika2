/* -*- coding: utf-8; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Suika2
 * Copyright (C) 2001-2023, Keiichi Tabata. All rights reserved.
 */

/*
 * [Changes]
 *  - 2023/09/14 作成
 */

#include "suika.h"

/*
 * pencilコマンド
 */
bool pencil_command(void)
{
	struct draw_msg_context context;
	struct image *img;
	const char *text;
	pixel_t color, outline_color;
	int layer, layer_w, layer_h,total_chars;

	/* パラメータを取得する */
	layer = get_int_param(PENCIL_PARAM_LAYER);
	text = get_string_param(PENCIL_PARAM_TEXT);

	/* レイヤを制限する */
	if (layer < 0)
		layer = 0;
	if (layer > 7)
		layer = 7;
	layer = LAYER_TEXT1 + layer;

	/* 変数を展開する */
	text = expand_variable(text);

	/* テキストを保存する */
	if (!set_layer_text(layer, text))
		return false;

	/* レイヤのサイズを取得する */
	img = get_layer_image(layer);
	layer_w = img->width;
	layer_h = img->height;

	/* デフォルトの色を取得する */
	color = make_pixel(0xff,
			   (pixel_t)conf_font_color_r,
			   (pixel_t)conf_font_color_g,
			   (pixel_t)conf_font_color_b);
	outline_color = make_pixel(0xff,
				   (pixel_t)conf_font_outline_color_r,
				   (pixel_t)conf_font_outline_color_g,
				   (pixel_t)conf_font_outline_color_b);

	/* 描画する */
	construct_draw_msg_context(
		&context,
		layer,
		text,
		conf_font_select,
		conf_font_size,
		conf_font_size,
		conf_font_ruby_size,
		!conf_font_outline_remove,
		0,	/* pen_x */
		0,	/* pen_y */
		layer_w,
		layer_h,
		0,	/* left_margin */
		0,	/* right_margin */
		0,	/* top_margin */
		0,	/* bottom_margin */
		0,	/* line_margin */
		conf_msgbox_margin_char,
		color,
		outline_color,
		false,	/* is_dimming */
		false,	/* ignore_linefeed */
		false,	/* ignore_font */
		false,	/* ignore_outline */
		false,	/* ignore_color */
		false,	/* ignore_size */
		false,	/* ignore_position */
		false,	/* ignore_ruby */
		true,	/* ignore_wait */
		NULL,	/* inline_wait_hook */
		false);	/* use_tategaki */
	total_chars = count_chars_common(&context);
	draw_msg_common(&context, total_chars);

	/* 描画する */
	render_stage();

	return move_to_next_command();
}
