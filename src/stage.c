/* -*- coding: utf-8-with-signature; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Suika 2
 * Copyright (C) 2001-2021, TABATA Keiichi. All rights reserved.
 */

/*
 * [Changes]
 *  - 2016-06-14 作成
 *  - 2017-08-13 スイッチに対応
 *  - 2017-09-25 セリフの色付けに対応
 *  - 2018-08-28 不要なエラーログの削除
 *  - 2019-09-17 NEWSに対応
 *  - 2021-06-05 背景フェードの追加
 *  - 2021-06-10 マスクつき描画の対応
 *  - 2021-06-10 キャラのアニメ対応
 *  - 2021-06-12 画面揺らしモードの対応
 */

#include "suika.h"

/* false assertion */
#define BAD_POSITION		(0)
#define INVALID_BG_FADE_METHOD	(0)
#define INVALID_CH_FADE_METHOD	(0)

/* カーテンフェードのカーテンの幅 */
#define CURTAIN_WIDTH	(256)

/* レイヤ */
enum {
	/* 背景レイヤ */
	LAYER_BG,

	/* キャラクタレイヤ */
	LAYER_CHB,
	LAYER_CHL,
	LAYER_CHR,
	LAYER_CHC,

	/* メッセージレイヤ */
	LAYER_MSG,	/* 特殊: 実体イメージあり */

	/* 名前レイヤ */
	LAYER_NAME,	/* 特殊: 実体イメージあり */

	/* クリックアニメーション */
	LAYER_CLICK,

	/* 選択肢レイヤ */
	LAYER_SEL,	/* 特殊: 実体イメージあり */

	/*
	 * 下記のレイヤは次の場合に有効
	 *  - 背景フェード
	 *  - キャラのフェード
	 *  - イメージボタン
	 *  - セーブ・ロード
	 *  - スイッチ
	 */
	LAYER_FO,	/* 特殊: 実体イメージあり */

	/*
	 * 下記のレイヤは次の場合に有効
	 *  - キャラフェード
	 *  - イメージボタン
	 *  - セーブ・ロード
	 *  - スイッチ
	 */
	LAYER_FI,	/* 特殊: 実体イメージあり */

	/* 下記は背景のフェードモードでのみ有効 */
	LAYER_BG_FI,

	/* 総レイヤ数 */
	STAGE_LAYERS
};

/* 描画先イメージ */
static struct image *back_image;

/* レイヤのイメージ */
static struct image *layer_image[STAGE_LAYERS];

/* メッセージボックスのイメージ */
static struct image *msgbox_image;

/* メッセージボックスを表示するか */
static bool is_msgbox_visible;

/* 名前ボックスのイメージ */
static struct image *namebox_image;

/* 名前ボックスを表示するか */
static bool is_namebox_visible;

/* クリックアニメーションを表示するか */
static bool is_click_visible;

/* 選択肢ボックス(非選択)のイメージ */
static struct image *selbox_bg_image;

/* 選択肢ボックス(選択)のイメージ */
static struct image *selbox_fg_image;

/* 選択肢ボックスを表示するか */
static bool is_selbox_visible;

/* スイッチ(非選択)のイメージ */
static struct image *switch_bg_image;

/* スイッチ(選択)のイメージ */
static struct image *switch_fg_image;

/* NEWS(非選択)のイメージ */
static struct image *news_bg_image;

/* NEWS(選択)のイメージ */
static struct image *news_fg_image;

/* セーブ画面(非選択)のイメージ */
static struct image *save_bg_image;

/* セーブ画面(選択)のイメージ */
static struct image *save_fg_image;

/* レイヤのx座標 */
static int layer_x[STAGE_LAYERS];

/* レイヤのy座標 */
static int layer_y[STAGE_LAYERS];

/* レイヤのアルファ値 */
static int layer_alpha[STAGE_LAYERS];

/* レイヤのブレンドタイプ */
static int layer_blend[STAGE_LAYERS];

/* 背景フェードモードであるか */
static bool is_bg_fade_enabled;

/* キャラフェードモードであるか */
static bool is_ch_fade_enabled;

/* 背景イメージ名 */
static char *bg_file_name;

/* キャライメージ名 */
static char *ch_file_name[CH_LAYERS];

/*
 * アニメ中の情報
 *  - 現状キャラを1つずつ(1レイヤずつ)しか動かすことができない
 *  - 将来複数レイヤを動かせるような設計として、下記の情報を保持する
 */

/* キャラアニメモードであるか */
static bool is_ch_anime_enabled;

/* アニメ中のレイヤ */
static bool layer_anime_run[STAGE_LAYERS];

/* アニメ中のレイヤのx座標 */
static int layer_anime_x_from[STAGE_LAYERS];
static int layer_anime_x_to[STAGE_LAYERS];

/* アニメ中のレイヤのy座標 */
static int layer_anime_y_from[STAGE_LAYERS];
static int layer_anime_y_to[STAGE_LAYERS];

/* アニメ中のレイヤのアルファ値 */
static int layer_anime_alpha_from[STAGE_LAYERS];
static int layer_anime_alpha_to[STAGE_LAYERS];

/*
 * 画面揺らしモード中の情報
 */

/* 画面揺らしモード中であるか */
static bool is_shake_enabled;

/* 画面表示オフセット */
static int shake_offset_x;
static int shake_offset_y;

/*
 * 前方参照
 */
static bool setup_namebox(void);
static bool setup_msgbox(void);
static bool setup_click(void);
static bool setup_selbox(void);
static bool setup_switch(void);
static bool setup_news(void);
static bool setup_save(void);
static bool create_fade_layer_images(void);
static void destroy_layer_image(int layer);
static void draw_stage_bg_fade_normal(void);
static void draw_stage_bg_fade_mask(void);
static void draw_stage_bg_fade_curtain_right(void);
static void draw_stage_bg_fade_curtain_left(void);
static void draw_stage_bg_fade_curtain_up(void);
static void draw_stage_bg_fade_curtain_down(void);
static void draw_stage_bg_fade_slide_left(void);
static void draw_stage_bg_fade_slide_right(void);
static void draw_stage_bg_fade_slide_up(void);
static void draw_stage_bg_fade_slide_down(void);
static void draw_stage_ch_fade_normal(void);
static void draw_stage_ch_fade_mask(void);
static int pos_to_layer(int pos);
static float get_anime_interpolation(float progress, float from, float to);
static void draw_layer_image(struct image *target, int layer);
static void draw_layer_image_rect(struct image *target, int layer, int x,
				  int y, int w, int h);
static bool draw_char_on_layer(int layer, int x, int y, uint32_t wc,
			       pixel_t color, int *w, int *h);

/*
 * 初期化
 */

/*
 * ステージの初期化処理をする
 */
bool init_stage(void)
{
	int i;

	/* 描画先を取得する */
	back_image = get_back_image();

	/* 名前ボックスをセットアップする */
	if (!setup_namebox())
		return false;

	/* メッセージボックスをセットアップする */
	if (!setup_msgbox())
		return false;

	/* クリックアニメーションをセットアップする */
	if (!setup_click())
		return false;

	/* 選択肢ボックスをセットアップする */
	if (!setup_selbox())
		return false;

	/* スイッチをセットアップする */
	if (!setup_switch())
		return false;

	/* NEWSをセットアップする */
	if (!setup_news())
		return false;

	/* セーブ画面をセットアップする */
	if (!setup_save())
		return false;

	/* フェードイン・アウトレイヤのイメージを作成する */
	if (!create_fade_layer_images())
		return false;

	/* ブレンドタイプを設定する */
	layer_blend[LAYER_BG] = BLEND_NONE;
	layer_blend[LAYER_CHB] = BLEND_FAST;
	layer_blend[LAYER_CHL] = BLEND_FAST;
	layer_blend[LAYER_CHR] = BLEND_FAST;
	layer_blend[LAYER_CHC] = BLEND_FAST;
	layer_blend[LAYER_MSG] = BLEND_FAST;
	layer_blend[LAYER_NAME] = BLEND_FAST;
	layer_blend[LAYER_CLICK] = BLEND_FAST;
	layer_blend[LAYER_SEL] = BLEND_FAST;
	layer_blend[LAYER_FO] = BLEND_NONE;
	layer_blend[LAYER_FI] = BLEND_FAST;
	layer_blend[LAYER_BG_FI] = BLEND_FAST;

	/* アルファ値を設定する */
	for (i = LAYER_BG; i < STAGE_LAYERS; i++)
		layer_alpha[i] = 255;

	return true;
}

/* 名前ボックスをセットアップする */
static bool setup_namebox(void)
{
	is_namebox_visible = false;

	/* 名前ボックスの画像を読み込む */
	namebox_image = create_image_from_file(CG_DIR, conf_namebox_file);
	if (namebox_image == NULL)
		return false;

	/* 名前ボックスのレイヤのイメージを作成する */
	layer_image[LAYER_NAME] = create_image(get_image_width(namebox_image),
					       get_image_height(namebox_image));
	if (layer_image[LAYER_NAME] == NULL)
		return false;

	/* 名前ボックスレイヤの配置を行う */
	layer_x[LAYER_NAME] = conf_namebox_x;
	layer_y[LAYER_NAME] = conf_namebox_y;

	/* 内容を転送する */
	clear_namebox();

	return true;
}

/* メッセージボックスをセットアップする */
static bool setup_msgbox(void)
{
	is_msgbox_visible = false;

	/* メッセージボックスの画像を読み込む */
	msgbox_image = create_image_from_file(CG_DIR, conf_msgbox_file);
	if (msgbox_image == NULL)
		return false;

	/* メッセージボックスのレイヤのイメージを作成する */
	layer_image[LAYER_MSG] = create_image(get_image_width(msgbox_image),
					      get_image_height(msgbox_image));
	if (layer_image[LAYER_MSG] == NULL)
		return false;

	/* メッセージボックスレイヤの配置を行う */
	layer_x[LAYER_MSG] = conf_msgbox_x;
	layer_y[LAYER_MSG] = conf_msgbox_y;

	/* 内容を転送する */
	clear_msgbox();

	return true;
}

/* クリックアニメーションをセットアップする */
static bool setup_click(void)
{
	is_click_visible = false;

	/* クリックアニメーションの画像を読み込む */
	layer_image[LAYER_CLICK] = create_image_from_file(CG_DIR,
							  conf_click_file);
	if (layer_image[LAYER_CLICK] == NULL)
		return false;

	/* クリックアニメーションレイヤの配置を行う */
	layer_x[LAYER_CLICK] = conf_click_x;
	layer_y[LAYER_CLICK] = conf_click_y;

	return true;
}

/* 選択肢ボックスをセットアップする */
static bool setup_selbox(void)
{
	is_selbox_visible = false;

	/* 選択肢ボックスの画像を読み込む */
	selbox_bg_image = create_image_from_file(CG_DIR, conf_selbox_bg_file);
	if (selbox_bg_image == NULL)
		return false;
	selbox_fg_image = create_image_from_file(CG_DIR, conf_selbox_fg_file);
	if (selbox_fg_image == NULL)
		return false;

	/* 選択肢ボックスのレイヤのイメージを作成する */
	layer_image[LAYER_SEL] = create_image(get_image_width(selbox_bg_image),
					      get_image_height(
						      selbox_bg_image));
	if (layer_image[LAYER_SEL] == NULL)
		return false;

	/* 選択肢ボックスのレイヤの配置を行う */
	layer_x[LAYER_SEL] = conf_selbox_x;
	layer_y[LAYER_SEL] = conf_selbox_y;

	return true;
}

/* スイッチをセットアップする */
static bool setup_switch(void)
{
	/* スイッチの非選択イメージを読み込む */
	switch_bg_image = create_image_from_file(CG_DIR, conf_switch_bg_file);
	if (switch_bg_image == NULL)
		return false;

	/* スイッチの選択イメージを読み込む */
	switch_fg_image = create_image_from_file(CG_DIR, conf_switch_fg_file);
	if (switch_fg_image == NULL)
		return false;

	return true;
}

/* NEWSをセットアップする */
static bool setup_news(void)
{
	/* NEWSの非選択イメージを読み込む */
	news_bg_image = create_image_from_file(CG_DIR, conf_news_bg_file);
	if (news_bg_image == NULL)
		return false;

	/* NEWSの選択イメージを読み込む */
	news_fg_image = create_image_from_file(CG_DIR, conf_news_fg_file);
	if (news_fg_image == NULL)
		return false;

	return true;
}

/* セーブ画面をセットアップする */
static bool setup_save(void)
{
	/* セーブ画面(非選択)の画像を読み込む */
	save_bg_image = create_image_from_file(CG_DIR, conf_save_bg_file);
	if (save_bg_image == NULL)
		return false;

	/* セーブ画面(非選択)の画像を読み込む */
	save_fg_image = create_image_from_file(CG_DIR, conf_save_fg_file);
	if (save_fg_image == NULL)
		return false;

	return true;
}

/* レイヤのイメージを作成する */
static bool create_fade_layer_images(void)
{
	/* フェードアウトのレイヤのイメージを作成する */
	layer_image[LAYER_FO] = create_image(conf_window_width,
					     conf_window_height);
	if (layer_image[LAYER_FO] == NULL)
		return false;

	/* フェードインのレイヤのイメージを作成する */
	layer_image[LAYER_FI] = create_image(conf_window_width,
					     conf_window_height);
	if (layer_image[LAYER_FI] == NULL)
		return false;

	return true;
}

/*
 * ステージの終了処理を行う
 */
void cleanup_stage(void)
{
	int i;

	for (i = LAYER_BG; i < STAGE_LAYERS; i++)
		destroy_layer_image(i);

	if (namebox_image != NULL) {
		destroy_image(namebox_image);
		namebox_image = NULL;
	}

	if (msgbox_image != NULL) {
		destroy_image(msgbox_image);
		msgbox_image = NULL;
	}
}

/*
 * レイヤのイメージを破棄する
 */
static void destroy_layer_image(int layer)
{
	assert(layer >= LAYER_BG && layer < STAGE_LAYERS);

	if (layer_image[layer] != NULL) {
		destroy_image(layer_image[layer]);
		layer_image[layer] = NULL;
	}
}

/*
 * ステージの描画
 */

/* ステージを描画する */
void draw_stage(void)
{
	assert(!is_save_mode());
	assert(!is_bg_fade_enabled);
	assert(!is_ch_fade_enabled);

	draw_stage_rect(0, 0, conf_window_width, conf_window_height);
}

/*
 * ステージを描画する
 */
void draw_stage_rect(int x, int y, int w, int h)
{
	assert(!is_save_mode());
	assert(!is_bg_fade_enabled);
	assert(!is_ch_fade_enabled);
	assert(x >= 0 && y >= 0 && w >= 0 && h >= 0);

	if (w == 0 || h == 0)
		return;
	if (x >= conf_window_width || y >= conf_window_height)
		return;
	if (x + w >= conf_window_width)
		w = conf_window_width - x;
	if (y + h >= conf_window_height)
		h = conf_window_height - y;

	draw_layer_image_rect(back_image, LAYER_BG, x, y, w, h);
	draw_layer_image_rect(back_image, LAYER_CHB, x, y, w, h);
	draw_layer_image_rect(back_image, LAYER_CHL, x, y, w, h);
	draw_layer_image_rect(back_image, LAYER_CHR, x, y, w, h);
	draw_layer_image_rect(back_image, LAYER_CHC, x, y, w, h);
	if (is_msgbox_visible)
		draw_layer_image_rect(back_image, LAYER_MSG, x, y, w, h);
	if (is_namebox_visible)
		draw_layer_image_rect(back_image, LAYER_NAME, x, y, w,h);
	if (is_click_visible)
		draw_layer_image_rect(back_image, LAYER_CLICK, x, y, w, h);
	if (is_selbox_visible)
		draw_layer_image_rect(back_image, LAYER_SEL, x, y, w, h);
}

/*
 * 背景フェードモードが有効な際のステージ描画を行う
 */
void draw_stage_bg_fade(int fade_method)
{
	assert(!is_save_mode());
	assert(is_bg_fade_enabled);
	assert(!is_ch_fade_enabled);

	switch (fade_method) {
	case BG_FADE_METHOD_NORMAL:
		draw_stage_bg_fade_normal();
		break;
	case BG_FADE_METHOD_MASK:
		draw_stage_bg_fade_mask();
		break;
	case BG_FADE_METHOD_CURTAIN_RIGHT:
		draw_stage_bg_fade_curtain_right();
		break;
	case BG_FADE_METHOD_CURTAIN_LEFT:
		draw_stage_bg_fade_curtain_left();
		break;
	case BG_FADE_METHOD_CURTAIN_UP:
		draw_stage_bg_fade_curtain_up();
		break;
	case BG_FADE_METHOD_CURTAIN_DOWN:
		draw_stage_bg_fade_curtain_down();
		break;
	case BG_FADE_METHOD_SLIDE_RIGHT:
		draw_stage_bg_fade_slide_right();
		break;
	case BG_FADE_METHOD_SLIDE_LEFT:
		draw_stage_bg_fade_slide_left();
		break;
	case BG_FADE_METHOD_SLIDE_UP:
		draw_stage_bg_fade_slide_up();
		break;
	case BG_FADE_METHOD_SLIDE_DOWN:
		draw_stage_bg_fade_slide_down();
		break;
	default:
		assert(INVALID_BG_FADE_METHOD);
		break;
	}
}

/* デフォルトの背景フェードの描画を行う  */
static void draw_stage_bg_fade_normal()
{
	draw_layer_image(back_image, LAYER_FO);
	draw_layer_image(back_image, LAYER_BG_FI);
}


/* マスクの背景フェードの描画を行う  */
static void draw_stage_bg_fade_mask()
{
	int mask_index;

	/* アルファ値からマスクインデックスを求める */
	mask_index = (int)((float)DRAW_IMAGE_MASK_LEVELS *
			   (float)layer_alpha[LAYER_BG_FI] / 255.0f);

	/* 古い背景を描画する */
	draw_layer_image(back_image, LAYER_FO);

	/* 新しい背景をマスクつきで描画する */
	draw_image_mask(back_image, 0, 0, layer_image[LAYER_BG_FI],
			conf_window_width, conf_window_height, 0, 0,
			mask_index);
}

/* 右方向カーテンフェードの描画を行う */
static void draw_stage_bg_fade_curtain_right(void)
{
	int right, alpha, i;

	/*
	 * カーテンの右端を求める
	 *  - カーテンの右端は0からconf_window_width+CURTAIN_WIDTHになる
	 */
	right = (int)((float)(conf_window_width + CURTAIN_WIDTH) *
		      (float)layer_alpha[LAYER_BG_FI] / 255.0f);

	/* カーテンが通り過ぎる前の背景をコピーする */
	if (right < conf_window_width) {
		draw_image(back_image, right, 0, layer_image[LAYER_FO],
			   conf_window_width - right, conf_window_height,
			   right, 0, 255, BLEND_NONE);
	}

	/* カーテンが通り過ぎた後の背景を描画する */
	if (right >= CURTAIN_WIDTH) {
		draw_image(back_image, 0, 0, layer_image[LAYER_BG_FI],
			   right - CURTAIN_WIDTH, conf_window_height, 0, 0,
			   255, BLEND_NONE);
	}

	/* カーテンを描画する */
	for (alpha = 0, i = right; i >= right - CURTAIN_WIDTH; i--, alpha++) {
		if (i < 0 || i >= conf_window_width)
			continue;
		draw_image(back_image, i, 0, layer_image[LAYER_BG_FI], 1,
			   conf_window_height, i, 0, alpha, BLEND_FAST);
	}
}

/* 左方向カーテンフェードの描画を行う */
static void draw_stage_bg_fade_curtain_left(void)
{
	int left, alpha, i;

	/*
	 * カーテンの左端を求める
	 *  - カーテンの左端はconf_window_widthから-CURTAIN_WIDTになる
	 */
	left = conf_window_width -
		(int)((float)(conf_window_width + CURTAIN_WIDTH) *
		      (float)layer_alpha[LAYER_BG_FI] / 255.0f);

	/* カーテンが通り過ぎる前の背景をコピーする */
	draw_image(back_image, 0, 0, layer_image[LAYER_FO],
		   left + CURTAIN_WIDTH, conf_window_height, 0, 0, 255,
		   BLEND_NONE);

	/* カーテンが通り過ぎた後の背景を描画する */
	if (left <= conf_window_width - CURTAIN_WIDTH) {
		draw_image(back_image, left + CURTAIN_WIDTH, 0,
			   layer_image[LAYER_BG_FI],
			   conf_window_width - left - CURTAIN_WIDTH,
			   conf_window_height, left + CURTAIN_WIDTH, 0, 255,
			   BLEND_NONE);
	}

	/* カーテンを描画する */
	for (alpha = 0, i = left; i <= left + CURTAIN_WIDTH; i++, alpha++) {
		if (i < 0 || i >= conf_window_width)
			continue;
		draw_image(back_image, i, 0, layer_image[LAYER_BG_FI], 1,
			   conf_window_height, i, 0, alpha, BLEND_FAST);
	}
}

/* 上方向カーテンフェードの描画を行う */
static void draw_stage_bg_fade_curtain_up(void)
{
	int top, alpha, i;

	/*
	 * カーテンの左端を求める
	 *  - カーテンの上端はconf_window_heightから-CURTAIN_WIDTHになる
	 */
	top = conf_window_height -
		(int)((float)(conf_window_height + CURTAIN_WIDTH) *
		      (float)layer_alpha[LAYER_BG_FI] / 255.0f);

	/* カーテンが通り過ぎる前の背景をコピーする */
	draw_image(back_image, 0, 0, layer_image[LAYER_FO],
		   conf_window_width, top + CURTAIN_WIDTH, 0, 0, 255,
		   BLEND_NONE);

	/* カーテンが通り過ぎた後の背景を描画する */
	if (top <= conf_window_height - CURTAIN_WIDTH) {
		draw_image(back_image, 0, top + CURTAIN_WIDTH,
			   layer_image[LAYER_BG_FI],
			   conf_window_width,
			   conf_window_height - top - CURTAIN_WIDTH,
			   0, top + CURTAIN_WIDTH, 255,
			   BLEND_NONE);
	}

	/* カーテンを描画する */
	for (alpha = 0, i = top; i <= top + CURTAIN_WIDTH; i++, alpha++) {
		if (i < 0 || i >= conf_window_height)
			continue;
		draw_image(back_image, 0, i, layer_image[LAYER_BG_FI],
			   conf_window_width, 1, 0, i, alpha, BLEND_FAST);
	}
}

/* 下方向カーテンフェードの描画を行う */
static void draw_stage_bg_fade_curtain_down(void)
{
	int bottom, alpha, i;

	/*
	 * カーテンの下端を求める
	 *  - カーテンの下端は0からconf_window_height+CURTAIN_WIDTHになる
	 */
	bottom = (int)((float)(conf_window_height + CURTAIN_WIDTH) *
		      (float)layer_alpha[LAYER_BG_FI] / 255.0f);

	/* カーテンが通り過ぎる前の背景をコピーする */
	if (bottom < conf_window_height) {
		draw_image(back_image, 0, bottom, layer_image[LAYER_FO],
			   conf_window_width, conf_window_height - bottom,
			   0, bottom, 255, BLEND_NONE);
	}

	/* カーテンが通り過ぎた後の背景を描画する */
	if (bottom >= CURTAIN_WIDTH) {
		draw_image(back_image, 0, 0, layer_image[LAYER_BG_FI],
			   conf_window_width, bottom - CURTAIN_WIDTH, 0, 0,
			   255, BLEND_NONE);
	}

	/* カーテンを描画する */
	for (alpha = 0, i = bottom; i >= bottom - CURTAIN_WIDTH;
	     i--, alpha++) {
		if (i < 0 || i >= conf_window_height)
			continue;
		draw_image(back_image, 0, i, layer_image[LAYER_BG_FI],
			   conf_window_width, 1, 0, i, alpha, BLEND_FAST);
	}
}

/* 右方向スライドフェードの描画を行う */
static void draw_stage_bg_fade_slide_right(void)
{
	int right;

	/*
	 * スライドの右端を求める
	 *  - スライドの右端は0からconf_window_widthになる
	 */
	right = (int)((float)conf_window_width *
		      (float)layer_alpha[LAYER_BG_FI] / 255.0f);

	/* 左側の背景を表示する */
	draw_image(back_image, 0, 0, layer_image[LAYER_BG_FI],
		   right, conf_window_height, conf_window_width - right, 0,
		   255, BLEND_NONE);

	/* 右側の背景を表示する */
	draw_image(back_image, right, 0, layer_image[LAYER_FO],
		   conf_window_width - right, conf_window_height, 0, 0, 255,
		   BLEND_NONE);
}

/* 左方向スライドフェードの描画を行う */
static void draw_stage_bg_fade_slide_left(void)
{
	int left;

	/*
	 * スライドの左端を求める
	 *  - スライドの左端はconf_window_widthから0になる
	 */
	left = conf_window_width -
		(int)((float)conf_window_width *
		      (float)layer_alpha[LAYER_BG_FI] / 255.0f);

	/* 右側の背景を表示する */
	draw_image(back_image, left, 0, layer_image[LAYER_BG_FI],
		   conf_window_width - left, conf_window_height, 0, 0, 255,
		   BLEND_NONE);

	/* 左側の背景を表示する */
	draw_image(back_image, 0, 0, layer_image[LAYER_FO],
		   left, conf_window_height, conf_window_width - left, 0,
		   255, BLEND_NONE);
}

/* 上方向スライドフェードの描画を行う */
static void draw_stage_bg_fade_slide_up(void)
{
	int top;

	/*
	 * スライドの上端を求める
	 *  - スライドの上端はconf_window_heightから0になる
	 */
	top = conf_window_height -
		(int)((float)conf_window_height *
		      (float)layer_alpha[LAYER_BG_FI] / 255.0f);

	/* 上側の背景を表示する */
	draw_image(back_image, 0, 0, layer_image[LAYER_FO],
		   conf_window_width, top, 0, conf_window_height - top,
		   255, BLEND_NONE);

	/* 下側の背景を表示する */
	draw_image(back_image, 0, top, layer_image[LAYER_BG_FI],
		   conf_window_width, conf_window_height - top, 0, 0, 255,
		   BLEND_NONE);
}

/* 下方向スライドフェードの描画を行う */
static void draw_stage_bg_fade_slide_down(void)
{
	int bottom;

	/*
	 * スライドの下端を求める
	 *  - スライドの下端は0からconf_window_heightになる
	 */
	bottom = (int)((float)conf_window_height *
		       (float)layer_alpha[LAYER_BG_FI] / 255.0f);

	/* 上側の背景を表示する */
	draw_image(back_image, 0, 0, layer_image[LAYER_BG_FI],
		   conf_window_width, bottom, 0, conf_window_height - bottom,
		   255, BLEND_NONE);

	/* 下側の背景を表示する */
	draw_image(back_image, 0, bottom, layer_image[LAYER_FO],
		   conf_window_width, conf_window_height - bottom, 0, 0, 255,
		   BLEND_NONE);
}

/*
 * キャラフェードモードが有効な際のステージ描画を行う
 */
void draw_stage_ch_fade(int fade_method)
{
	assert(!is_save_mode());
	assert(!is_bg_fade_enabled);
	assert(is_ch_fade_enabled);

	switch (fade_method) {
	case CH_FADE_METHOD_NORMAL:
		draw_stage_ch_fade_normal();
		break;
	case CH_FADE_METHOD_MASK:
		draw_stage_ch_fade_mask();
		break;
	default:
		assert(INVALID_CH_FADE_METHOD);
		break;
	}
}

static void draw_stage_ch_fade_normal(void)
{
	draw_layer_image(back_image, LAYER_FO);
	draw_layer_image(back_image, LAYER_FI);
}

static void draw_stage_ch_fade_mask(void)
{
	int mask_index;

	/* アルファ値からマスクインデックスを求める */
	mask_index = (int)((float)DRAW_IMAGE_MASK_LEVELS *
			   (float)layer_alpha[LAYER_FI] / 255.0f);

	/* 古い背景を描画する */
	draw_layer_image(back_image, LAYER_FO);

	/* 新しい背景をマスクつきで描画する */
	draw_image_mask(back_image, 0, 0, layer_image[LAYER_FI],
			conf_window_width, conf_window_height, 0, 0,
			mask_index);
}

/*
 * 画面揺らしモードが有効な際のステージ描画を行う
 */
void draw_stage_shake(void)
{
	/* 背景を塗り潰す */
	if (conf_window_white)
		clear_image_white(back_image);
	else
		clear_image_black(back_image);

	/* FOレイヤを描画する */
	draw_image(back_image, shake_offset_x, shake_offset_y,
		   layer_image[LAYER_FO], conf_window_width,
		   conf_window_height, 0, 0, 255, BLEND_NONE);
}

/*
 * ステージの背景(FO)全体と、前景(FI)のうち2矩形を描画する
 */
void draw_stage_with_buttons(int x1, int y1, int w1, int h1, int x2, int y2,
			     int w2, int h2)
{
	assert(!is_bg_fade_enabled);
	assert(!is_ch_fade_enabled);

	/* 背景を描画する */
	draw_image(back_image, 0, 0, layer_image[LAYER_FO],
		   get_image_width(layer_image[LAYER_FO]),
		   get_image_height(layer_image[LAYER_FO]),
		   0, 0, 255, BLEND_NONE);

	/* 1つめのボタンを描画する */
	draw_image(back_image, x1, y1, layer_image[LAYER_FI], w1, h1, x1, y1,
		   255, BLEND_NONE);

	/* 2つめのボタンを描画する */
	draw_image(back_image, x2, y2, layer_image[LAYER_FI], w2, h2, x2, y2,
		   255, BLEND_NONE);
}

/*
 * ステージの背景(FO)のうち1矩形と、前景(FI)のうち1矩形を描画する
 */
void draw_stage_rect_with_buttons(int old_x, int old_y, int old_w, int old_h,
				  int new_x, int new_y, int new_w, int new_h)
{
	assert(!is_save_mode());
	assert(!is_bg_fade_enabled);
	assert(!is_ch_fade_enabled);

	/* 古いボタンを消す */
	draw_image(back_image, old_x, old_y, layer_image[LAYER_FO], old_w,
		   old_h, old_x, old_y, 255, BLEND_NONE);

	/* 新しいボタンを描画する */
	draw_image(back_image, new_x, new_y, layer_image[LAYER_FI], new_w,
		   new_h, new_x, new_y, 255, BLEND_NONE);
}

/*
 * ステージの指定された矩形とスイッチを描画する
 */
void draw_stage_rect_with_switch(int x, int y, int w, int h)
{
	draw_stage_rect(x, y, w, h);
	draw_image(back_image, x, y, layer_image[LAYER_FI], w, h, x, y, 255,
		   BLEND_FAST);
}

/*
 * ステージの背景(FO)と前景(FI)を描画する
 */
void draw_stage_history(void)
{
	assert(is_history_mode());
	assert(!is_save_mode());
	assert(!is_bg_fade_enabled);
	assert(!is_ch_fade_enabled);

	/* 古いボタンを消す */
	draw_image(back_image, 0, 0, layer_image[LAYER_FO], conf_window_width,
		   conf_window_height, 0, 0, 255, BLEND_NONE);

	/* 新しいボタンを描画する */
	draw_image(back_image, 0, 0, layer_image[LAYER_FI], conf_window_width,
		   conf_window_height, 0, 0, 255, BLEND_FAST);
}

/*
 * 背景の変更
 */

/*
 * 背景のファイル名を設定する
 */
bool set_bg_file_name(const char *file)
{
	if (bg_file_name != NULL)
		free(bg_file_name);

	if (file == NULL) {
		bg_file_name = NULL;
	} else {
		bg_file_name = strdup(file);
		if (bg_file_name == NULL) {
			log_memory();
			return false;
		}
	}

	return true;
}

/*
 * 背景のファイル名を取得する
 */
const char *get_bg_file_name(void)
{
	return bg_file_name;
}

/*
 * 背景をフェードせずにただちに切り替える
 */
void change_bg_immediately(struct image *img)
{
	destroy_layer_image(LAYER_BG);
	layer_image[LAYER_BG] = img;
}

/*
 * 背景フェードを開始する
 */
void start_bg_fade(struct image *img)
{
	assert(!is_ch_fade_enabled);
	assert(!is_ch_anime_enabled);
	assert(!is_ch_fade_enabled);
	assert(!is_shake_enabled);

	/* 背景フェードを有効にする */
	is_bg_fade_enabled = true;

	/* フェードアウト用のレイヤにステージを描画する */
	draw_layer_image(layer_image[LAYER_FO], LAYER_BG);
	draw_layer_image(layer_image[LAYER_FO], LAYER_CHB);
	draw_layer_image(layer_image[LAYER_FO], LAYER_CHL);
	draw_layer_image(layer_image[LAYER_FO], LAYER_CHR);
	draw_layer_image(layer_image[LAYER_FO], LAYER_CHC);

	/* フェードイン用のレイヤにイメージをセットする */
	layer_image[LAYER_BG_FI] = img;

	/* 無効になるレイヤのイメージを破棄する */
	destroy_layer_image(LAYER_BG);
	destroy_layer_image(LAYER_CHB);
	destroy_layer_image(LAYER_CHL);
	destroy_layer_image(LAYER_CHR);
	destroy_layer_image(LAYER_CHC);
}

/*
 * 背景フェードモードの進捗率を設定する
 */
void set_bg_fade_progress(float progress)
{
	assert(is_bg_fade_enabled);
	assert(!is_ch_fade_enabled);
	assert(!is_ch_anime_enabled);
	assert(!is_shake_enabled);

	layer_alpha[LAYER_BG_FI] = (uint8_t)(progress * 255.0f);
}

/*
 * 背景フェードモードを終了する
 */
void stop_bg_fade(void)
{
	assert(is_bg_fade_enabled);
	assert(!is_ch_fade_enabled);
	assert(!is_ch_anime_enabled);
	assert(!is_shake_enabled);

	is_bg_fade_enabled = false;
	destroy_layer_image(LAYER_BG);
	layer_image[LAYER_BG] = layer_image[LAYER_BG_FI];
	layer_image[LAYER_BG_FI] = NULL;
}

/*
 * キャラの変更
 */

/*
 * キャラのファイル名を設定する
 */
bool set_ch_file_name(int pos, const char *file)
{
	assert(pos == CH_BACK || pos == CH_LEFT || pos == CH_RIGHT ||
	       pos == CH_CENTER);
	assert(file == NULL || strcmp(file, "") != 0);
	       
	if (ch_file_name[pos] != NULL)
		free(ch_file_name[pos]);

	if (file == NULL) {
		ch_file_name[pos] = NULL;
	} else {
		ch_file_name[pos] = strdup(file);
		if (ch_file_name[pos] == NULL) {
			log_memory();
			return false;
		}
	}

	return true;
}

/*
 * キャラのファイル名を取得する
 */
const char *get_ch_file_name(int pos)
{
	assert(pos == CH_BACK || pos == CH_LEFT || pos == CH_RIGHT ||
	       pos == CH_CENTER);

	return ch_file_name[pos];
}

/*
 * キャラの座標を取得する
 */
void get_ch_position(int pos, int *x, int *y)
{
	int layer;

	assert(pos == CH_BACK || pos == CH_LEFT || pos == CH_RIGHT ||
	       pos == CH_CENTER);

	layer = pos_to_layer(pos);
	*x = layer_x[layer];
	*y = layer_y[layer];
}

/*
 * キャラのアルファ値を取得する
 */
int get_ch_alpha(int pos)
{
	int layer;

	assert(pos == CH_BACK || pos == CH_LEFT || pos == CH_RIGHT ||
	       pos == CH_CENTER);

	layer = pos_to_layer(pos);
	return layer_alpha[layer];
}

/*
 * キャラをフェードせずにただちに切り替える
 */
void change_ch_immediately(int pos, struct image *img, int x, int y, int alpha)
{
	int layer;

	assert(pos == CH_BACK || pos == CH_LEFT || pos == CH_RIGHT ||
	       pos == CH_CENTER);

	layer = pos_to_layer(pos);
	destroy_layer_image(layer);
	layer_image[layer] = img;
	layer_x[layer] = x;
	layer_y[layer] = y;
	layer_alpha[layer] = alpha;
}

/*
 * キャラの位置とアルファを設定する
 */
void change_ch_attributes(int pos, int x, int y, int alpha)
{
	int layer;

	assert(pos == CH_BACK || pos == CH_LEFT || pos == CH_RIGHT ||
	       pos == CH_CENTER);

	layer = pos_to_layer(pos);
	layer_x[layer] = x;
	layer_y[layer] = y;
	layer_alpha[layer] = alpha;
}

/*
 * キャラフェードモードを開始する
 */
void start_ch_fade(int pos, struct image *img, int x, int y, int alpha)
{
	int layer;

	assert(!is_bg_fade_enabled);
	assert(!is_ch_fade_enabled);
	assert(!is_ch_anime_enabled);
	assert(!is_shake_enabled);
	assert(pos == CH_BACK || pos == CH_LEFT || pos == CH_RIGHT ||
	       pos == CH_CENTER);

	is_ch_fade_enabled = true;

	/* キャラフェードアウトレイヤにステージを描画する */
	draw_layer_image(layer_image[LAYER_FO], LAYER_BG);
	draw_layer_image(layer_image[LAYER_FO], LAYER_CHB);
	draw_layer_image(layer_image[LAYER_FO], LAYER_CHL);
	draw_layer_image(layer_image[LAYER_FO], LAYER_CHR);
	draw_layer_image(layer_image[LAYER_FO], LAYER_CHC);

	/* キャラを入れ替える */
	layer = pos_to_layer(pos);
	destroy_layer_image(layer);
	layer_image[layer] = img;
	layer_alpha[layer] = alpha;
	layer_x[layer] = x;
	layer_y[layer] = y;

	/* キャラフェードインレイヤにステージを描画する */
	draw_layer_image(layer_image[LAYER_FI], LAYER_BG);
	draw_layer_image(layer_image[LAYER_FI], LAYER_CHB);
	draw_layer_image(layer_image[LAYER_FI], LAYER_CHL);
	draw_layer_image(layer_image[LAYER_FI], LAYER_CHR);
	draw_layer_image(layer_image[LAYER_FI], LAYER_CHC);
}

/* キャラの位置をレイヤインデックスに変換する */
static int pos_to_layer(int pos)
{
	switch (pos) {
	case CH_BACK:
		return LAYER_CHB;
	case CH_LEFT:
		return LAYER_CHL;
	case CH_RIGHT:
		return LAYER_CHR;
	case CH_CENTER:
		return LAYER_CHC;
	}
	assert(BAD_POSITION);
	return -1;	/* never come here */
}

/*
 * キャラフェードモードの進捗率を設定する
 */
void set_ch_fade_progress(float progress)
{
	assert(is_ch_fade_enabled);
	assert(!is_bg_fade_enabled);
	assert(!is_ch_anime_enabled);
	assert(!is_shake_enabled);

	layer_alpha[LAYER_FI] = (uint8_t)(progress * 255.0f);
}

/*
 * キャラフェードモードを終了する
 */
void stop_ch_fade(void)
{
	assert(is_ch_fade_enabled);
	assert(!is_bg_fade_enabled);
	assert(!is_ch_anime_enabled);
	assert(!is_shake_enabled);

	is_ch_fade_enabled = false;
}

/*
 * キャラアニメ
 */

/*
 * キャラアニメを開始する
 */
void start_ch_anime(int pos, int to_x, int to_y, int to_alpha)
{
	int layer, i;

	assert(!is_bg_fade_enabled);
	assert(!is_ch_fade_enabled);
	assert(!is_ch_anime_enabled);
	assert(!is_shake_enabled);
	assert(pos == CH_BACK || pos == CH_LEFT || pos == CH_RIGHT ||
	       pos == CH_CENTER);

	is_ch_anime_enabled = true;

	/* 座標とアルファ値を保存する */
	layer = pos_to_layer(pos);
	layer_anime_alpha_from[layer] = layer_alpha[layer];
	layer_anime_alpha_to[layer] = to_alpha;
	layer_anime_x_from[layer] = layer_x[layer];
	layer_anime_x_to[layer] = to_x;
	layer_anime_y_from[layer] = layer_y[layer];
	layer_anime_y_to[layer] = to_y;

	/* アニメ中のレイヤを設定する */
	for (i = 0; i < STAGE_LAYERS; i++)
		layer_anime_run[i] = (i == layer) ? true : false;
}

/*
 * キャラアニメモードの進捗率を設定する
 */
void set_ch_anime_progress(float progress)
{
	int i;

	assert(is_ch_anime_enabled);
	assert(!is_bg_fade_enabled);
	assert(!is_ch_fade_enabled);
	assert(!is_shake_enabled);

	/* すべてのレイヤについて座標とアルファ値を更新する */
	for (i = 0; i < STAGE_LAYERS; i++) {
		/* アニメ中でない場合は更新しない */
		if (!layer_anime_run[i])
			continue;

		layer_alpha[i] = (uint8_t)get_anime_interpolation(progress,
					(float)layer_anime_alpha_from[i],
					(float)layer_anime_alpha_to[i]);
		layer_x[i] = (int)get_anime_interpolation(progress,
					(float)layer_anime_x_from[i],
					(float)layer_anime_x_to[i]);
		layer_y[i] = (int)get_anime_interpolation(progress,
					(float)layer_anime_y_from[i],
					(float)layer_anime_y_to[i]);
	}
}

/* アニメの補間を行う */
static float get_anime_interpolation(float progress, float from, float to)
{
	return from + (to - from) * progress;
}

/*
 * キャラアニメモードを終了する
 */
void stop_ch_anime(void)
{
	int i;

	assert(is_ch_anime_enabled);
	assert(!is_bg_fade_enabled);
	assert(!is_ch_fade_enabled);
	assert(!is_shake_enabled);

	is_ch_anime_enabled = false;

	/* すべてのレイヤについて座標とアルファ値を最終値に更新する */
	for (i = 0; i < STAGE_LAYERS; i++) {
		/* アニメ中でない場合は更新しない */
		if (!layer_anime_run[i])
			continue;

		layer_alpha[i] = layer_anime_alpha_to[i];
		layer_x[i] = layer_anime_x_to[i];
		layer_y[i] = layer_anime_y_to[i];
	}
}

/*
 * 画面揺らしモード
 */

/* 画面揺らしモードを開始する */
void start_shake(void)
{
	assert(!is_shake_enabled);
	assert(!is_bg_fade_enabled);
	assert(!is_ch_fade_enabled);
	assert(!is_ch_anime_enabled);

	is_shake_enabled = true;

	/* フェードアウト用のレイヤにステージを描画する */
	draw_layer_image(layer_image[LAYER_FO], LAYER_BG);
	draw_layer_image(layer_image[LAYER_FO], LAYER_CHB);
	draw_layer_image(layer_image[LAYER_FO], LAYER_CHL);
	draw_layer_image(layer_image[LAYER_FO], LAYER_CHR);
	draw_layer_image(layer_image[LAYER_FO], LAYER_CHC);
}

/* 画面揺らしモードの表示オフセットを設定する */
void set_shake_offset(int x, int y)
{
	assert(is_shake_enabled);
	assert(!is_bg_fade_enabled);
	assert(!is_ch_fade_enabled);
	assert(!is_ch_anime_enabled);

	shake_offset_x = x;
	shake_offset_y = y;
}

/* 画面揺らしモードを終了する */
void stop_shake(void)
{
	assert(is_shake_enabled);
	assert(!is_bg_fade_enabled);
	assert(!is_ch_fade_enabled);
	assert(!is_ch_anime_enabled);

	is_shake_enabled = false;
}

/*
 * 名前ボックスの描画
 */

/*
 * 名前ボックスの矩形を取得する
 */
void get_namebox_rect(int *x, int *y, int *w, int *h)
{
	*x = layer_x[LAYER_NAME];
	*y = layer_y[LAYER_NAME];
	*w = get_image_width(layer_image[LAYER_NAME]);
	*h = get_image_height(layer_image[LAYER_NAME]);
}

/*
 * 名前ボックスをクリアする
 */
void clear_namebox(void)
{
	if (namebox_image == NULL)
		return;

	draw_image(layer_image[LAYER_NAME], 0, 0, namebox_image,
		   get_image_width(layer_image[LAYER_NAME]),
		   get_image_height(layer_image[LAYER_NAME]),
		   0, 0, 255, BLEND_NONE);
}

/*
 * 名前ボックスの表示・非表示を設定する
 */
void show_namebox(bool show)
{
	is_namebox_visible = show;
}

/*
 * メッセージボックスに文字を描画する
 *  - 描画した幅を返す
 */
int draw_char_on_namebox(int x, int y, uint32_t wc, pixel_t color)
{
	int w, h;

	draw_char_on_layer(LAYER_NAME, x, y, wc, color, &w, &h);

	return w;
}

/*
 * メッセージボックスの描画
 */

/*
 * メッセージボックスの矩形を取得する
 */
void get_msgbox_rect(int *x, int *y, int *w, int *h)
{
	*x = layer_x[LAYER_MSG];
	*y = layer_y[LAYER_MSG];
	*w = get_image_width(layer_image[LAYER_MSG]);
	*h = get_image_height(layer_image[LAYER_MSG]);
}

/*
 * メッセージボックスをクリアする
 */
void clear_msgbox(void)
{
	if (msgbox_image == NULL)
		return;

	draw_image(layer_image[LAYER_MSG], 0, 0, msgbox_image,
		   get_image_width(layer_image[LAYER_MSG]),
		   get_image_height(layer_image[LAYER_MSG]),
		   0, 0, 255, BLEND_NONE);
}

/*
 * メッセージボックスの表示・非表示を設定する
 */
void show_msgbox(bool show)
{
	is_msgbox_visible = show;
}

/*
 * メッセージボックスに文字を描画する
 *  - 描画した高さを返す
 */
int draw_char_on_msgbox(int x, int y, uint32_t wc, pixel_t color)
{
	int w, h;

	draw_char_on_layer(LAYER_MSG, x, y, wc, color, &w, &h);

	return h;
}

/*
 * クリックアニメーションの描画
 */

/*
 * クリックアニメーションの矩形を取得する
 */
void get_click_rect(int *x, int *y, int *w, int *h)
{
	*x = layer_x[LAYER_CLICK];
	*y = layer_y[LAYER_CLICK];
	*w = get_image_width(layer_image[LAYER_CLICK]);
	*h = get_image_height(layer_image[LAYER_CLICK]);
}

/*
 * クリックアニメーションの表示・非表示を設定する
 */
void show_click(bool show)
{
	is_click_visible = show;
}

/*
 * 選択肢ボックスの描画
 */

/*
 * 選択肢ボックスの矩形を取得する
 */
void get_selbox_rect(int *x, int *y, int *w, int *h)
{
	*x = layer_x[LAYER_SEL];
	*y = layer_y[LAYER_SEL];
	*w = get_image_width(layer_image[LAYER_SEL]);
	*h = get_image_height(layer_image[LAYER_SEL]);
}

/*
 * 選択肢ボックスをクリアする
 */
void clear_selbox(int fg_x, int fg_y, int fg_w, int fg_h)
{
	draw_image(layer_image[LAYER_SEL], 0, 0, selbox_bg_image,
		   get_image_width(layer_image[LAYER_SEL]),
		   get_image_height(layer_image[LAYER_SEL]),
		   0, 0, 255, BLEND_NONE);
	draw_image(layer_image[LAYER_SEL], fg_x, fg_y, selbox_fg_image, fg_w,
		   fg_h, fg_x, fg_y, 255, BLEND_NONE);
}

/*
 * 選択肢ボックスの表示・非表示を設定する
 */
void show_selbox(bool show)
{
	is_selbox_visible = show;
}

/*
 * 選択肢ボックスに文字を描画する
 *  - 描画した幅を返す
 */
int draw_char_on_selbox(int x, int y, uint32_t wc)
{
	pixel_t color;
	int w, h;

	color = make_pixel(0xff, (pixel_t)conf_font_color_r,
			   (pixel_t)conf_font_color_g,
			   (pixel_t)conf_font_color_b);

	draw_char_on_layer(LAYER_SEL, x, y, wc, color, &w, &h);

	return w;
}

/*
 * スイッチの描画
 */

/*
 * スイッチの親選択肢の矩形を取得する
 */
void get_switch_rect(int index, int *x, int *y, int *w, int *h)
{
	int width, height;

	width = get_image_width(switch_bg_image);
	height = get_image_height(switch_bg_image);

	*x = conf_switch_x;
	*y = conf_switch_y + (height + conf_switch_margin_y) * index;
	*w = width;
	*h = height;
}

/*
 * FIレイヤにスイッチの非選択イメージを描画する
 */
void draw_switch_bg_image(int x, int y)
{
	draw_image(layer_image[LAYER_FI], x, y, switch_bg_image,
		   get_image_width(switch_bg_image),
		   get_image_height(switch_bg_image),
		   0, 0, 255, BLEND_NONE);
}

/*
 * FIレイヤにスイッチの選択イメージを描画する
 */
void draw_switch_fg_image(int x, int y)
{
	draw_image(layer_image[LAYER_FI], x, y, switch_fg_image,
		   get_image_width(switch_fg_image),
		   get_image_height(switch_fg_image),
		   0, 0, 255, BLEND_NONE);
}

/*
 * NEWSの描画
 */

/*
 * NEWSの親選択肢の矩形を取得する
 */
void get_news_rect(int index, int *x, int *y, int *w, int *h)
{
	const int NORTH = 0;
	const int EAST = 1;
	const int WEST = 2;
	const int SOUTH = 3;
	const int SWITCH_BASE = 4;

	if (index == NORTH) {
		*w = get_image_width(news_bg_image);
		*h = get_image_height(news_bg_image);
		*x = (conf_window_width - *w) / 2;
		*y = conf_news_margin;
	} else if (index == EAST) {
		*w = get_image_width(news_bg_image);
		*h = get_image_height(news_bg_image);
		*x = conf_window_width - *w - conf_news_margin;
		*y = *h + conf_news_margin * 2;
	} else if (index == WEST) {
		*w = get_image_width(news_bg_image);
		*h = get_image_height(news_bg_image);
		*x = conf_news_margin;
		*y = *h + conf_news_margin * 2;
	} else if (index == SOUTH) {
		*w = get_image_width(news_bg_image);
		*h = get_image_height(news_bg_image);
		*x = (conf_window_width - *w) / 2;
		*y = *h * 2 + conf_news_margin * 3;
	} else {
		*w = get_image_width(switch_bg_image);
		*h = get_image_height(switch_bg_image);
		*x = conf_switch_x;
		*y = (get_image_height(news_bg_image) + conf_news_margin) * 3 +
			conf_news_margin +
			(*h + conf_switch_margin_y) * (index - SWITCH_BASE);
	}
}

/*
 * FIレイヤにNEWSの非選択イメージを描画する
 */
void draw_news_bg_image(int x, int y)
{
	draw_image(layer_image[LAYER_FI], x, y, news_bg_image,
		   get_image_width(news_bg_image),
		   get_image_height(news_bg_image),
		   0, 0, 255, BLEND_NONE);
}

/*
 * FIレイヤにスイッチの選択イメージを描画する
 */
void draw_news_fg_image(int x, int y)
{
	draw_image(layer_image[LAYER_FI], x, y, news_fg_image,
		   get_image_width(news_fg_image),
		   get_image_height(news_fg_image),
		   0, 0, 255, BLEND_NONE);
}

/*
 * セーブ画面の描画
 */

/*
 * セーブ画面用にFI/FOレイヤをクリアする
 */
void clear_save_stage(void)
{
	draw_image(layer_image[LAYER_FO], 0, 0, save_bg_image,
		   get_image_width(layer_image[LAYER_FO]),
		   get_image_height(layer_image[LAYER_FO]),
		   0, 0, 255, BLEND_NONE);
	draw_image(layer_image[LAYER_FI], 0, 0, save_fg_image,
		   get_image_width(layer_image[LAYER_FI]),
		   get_image_height(layer_image[LAYER_FI]),
		   0, 0, 255, BLEND_NONE);
}

/* FO/FIの2レイヤに文字を描画する */
int draw_char_on_fo_fi(int x, int y, uint32_t wc)
{
	pixel_t color;
	int w, h;

	color = make_pixel(0xff, (pixel_t)conf_font_color_r,
			   (pixel_t)conf_font_color_g,
			   (pixel_t)conf_font_color_b);

	draw_char_on_layer(LAYER_FO, x, y, wc, color, &w, &h);
	draw_char_on_layer(LAYER_FI, x, y, wc, color, &w, &h);

	return w;
}

/*
 * メニュー画面・CG回想画面の描画
 */

/*
 * FOレイヤにイメージを描画する
 */
void draw_image_to_fo(struct image *img)
{
	draw_image(layer_image[LAYER_FO], 0, 0, img, get_image_width(img),
		   get_image_height(img), 0, 0, 255, BLEND_NONE);
}

/*
 * FIレイヤにイメージを描画する
 */
void draw_image_to_fi(struct image *img)
{
	draw_image(layer_image[LAYER_FI], 0, 0, img, get_image_width(img),
		   get_image_height(img), 0, 0, 255, BLEND_NONE);
}

/*
 * FOレイヤに矩形を描画する
 */
void draw_rect_to_fo(int x, int y, int w, int h, pixel_t color)
{
	clear_image_color_rect(layer_image[LAYER_FO], x, y, w, h, color);
}

/*
 * ヒストリ画面の表示
 */

/*
 * FOレイヤにステージを描画する
 */
void draw_history_fo(void)
{
	draw_layer_image_rect(layer_image[LAYER_FO], LAYER_BG, 0, 0,
			      conf_window_width, conf_window_height);
	draw_layer_image_rect(layer_image[LAYER_FO], LAYER_CHB, 0, 0,
      			      conf_window_width, conf_window_height);
	draw_layer_image_rect(layer_image[LAYER_FO], LAYER_CHL, 0, 0,
			      conf_window_width, conf_window_height);
	draw_layer_image_rect(layer_image[LAYER_FO], LAYER_CHR, 0, 0,
			      conf_window_width, conf_window_height);
	draw_layer_image_rect(layer_image[LAYER_FO], LAYER_CHC, 0, 0,
			      conf_window_width, conf_window_height);
}

/*
 * FIレイヤを色で塗り潰す
 */
void draw_history_fi(pixel_t color)
{
	clear_image_color(layer_image[LAYER_FI], color);
}

/*
 * FIレイヤに文字を描画する
 */
void draw_char_on_fi(int x, int y, uint32_t wc, int *w, int *h)
{
	pixel_t color;

	color = make_pixel(0xff, (pixel_t)conf_font_color_r,
			   (pixel_t)conf_font_color_g,
			   (pixel_t)conf_font_color_b);

	draw_char_on_layer(LAYER_FI, x, y, wc, color, w, h);
}

/*
 * 共通ルーチン
 */

/* レイヤを描画する */
static void draw_layer_image(struct image *target, int layer)
{
	assert(layer >= LAYER_BG && layer < STAGE_LAYERS);

	/* 背景イメージがセットされていなければクリアする */
	if (layer == LAYER_BG && layer_image[LAYER_BG] == NULL) {
		if (conf_window_white)
			clear_image_white(target);
		else
			clear_image_black(target);
		return;
	}

	/* その他のレイヤはイメージがセットされていれば描画する */
	if (layer_image[layer] != NULL) {
		draw_image(target,
			   layer_x[layer],
			   layer_y[layer],
			   layer_image[layer],
			   get_image_width(layer_image[layer]),
			   get_image_height(layer_image[layer]),
			   0,
			   0,
			   layer_alpha[layer],
			   layer_blend[layer]);
	}
}

/* レイヤの矩形を描画する */
static void draw_layer_image_rect(struct image *target, int layer, int x,
				  int y, int w, int h)
{
	assert(layer >= LAYER_BG && layer < STAGE_LAYERS);

	/* FIXME: 背景イメージがセットされていなければクリアする */
	if (layer == LAYER_BG && layer_image[LAYER_BG] == NULL) {
		if (conf_window_white)
			clear_image_white_rect(target, x, y, w, h);
		else
			clear_image_black_rect(target, x, y, w, h);
		return;
	}

	/* その他のレイヤはイメージがセットされていれば描画する */
	if (layer_image[layer] != NULL) {
		draw_image(target, x, y, layer_image[layer], w, h,
			   x - layer_x[layer], y - layer_y[layer],
			   layer_alpha[layer], layer_blend[layer]);
	}
}

/* レイヤに文字を描画する */
static bool draw_char_on_layer(int layer, int x, int y, uint32_t wc,
			       pixel_t color, int *w, int *h)
{
	/* 文字を描画する */
	if (!draw_glyph(layer_image[layer], x, y, color, wc, w, h)) {
		/* グリフがない、コードポイントがおかしい、など */
		return false;
	}

	return true;
}

/*
 * 2つの矩形を囲う矩形を求める
 */
void union_rect(int *x, int *y, int *w, int *h, int x1, int y1, int w1, int h1,
		int x2, int y2, int w2, int h2)
{
	if ((w1 == 0 || h1 == 0) && (w2 == 0 || h2 == 0)) {
		*x = 0;
		*y = 0;
		*w = 0;
		*h = 0;
		return;
	}
	if (w1 ==0 || h1 == 0 ) {
		*x = x2;
		*y = y2;
		*w = w2;
		*h = h2;
		return;
	}
	if (w2 ==0 || h2 == 0 ) {
		*x = x1;
		*y = y1;
		*w = w1;
		*h = h1;
		return;
	}

	w1 = x1 + w1 - 1;
	h1 = y1 + h1 - 1;
	w2 = x2 + w2 - 1;
	h2 = y2 + h2 - 1;

	*x = x1 < x2 ? x1 : x2;
	*y = y1 < y2 ? y1 : y2;
	*w = w1 > w2 ? w1 - *x + 1 : w2 - *x + 1;
	*h = h1 > h2 ? h1 - *y + 1 : h2 - *y + 1;
}
