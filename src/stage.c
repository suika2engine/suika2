/* -*- coding: utf-8-with-signature; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Suika 2
 * Copyright (C) 2001-2017, TABATA Keiichi. All rights reserved.
 */

/*
 * [Changes]
 *  - 2016-06-14 作成
 *  - 2017-08-13 スイッチに対応
 */

#include "suika.h"

/* false assertion */
#define BAD_POSITION	(0)

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
 * 前方参照
 */
static bool setup_namebox(void);
static bool setup_msgbox(void);
static bool setup_click(void);
static bool setup_selbox(void);
static bool setup_switch(void);
static bool setup_save(void);
static bool create_fade_layer_images(void);
static void destroy_layer_image(int layer);
static void draw_stage_bg_fade_curtain(void);
static int pos_to_layer(int pos);
static void draw_layer_image(struct image *target, int layer);
static void draw_layer_image_rect(struct image *target, int layer, int x,
				  int y, int w, int h);
static bool draw_char_on_layer(int layer, int x, int y, uint32_t wc, int *w,
			       int *h);

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
	if (namebox_image == NULL) {
		log_dir_file_open(CG_DIR, conf_namebox_file);
		return false;
	}

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
	if (msgbox_image == NULL) {
		log_dir_file_open(CG_DIR, conf_msgbox_file);
		return false;
	}

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
	if (layer_image[LAYER_CLICK] == NULL) {
		log_dir_file_open(CG_DIR, conf_click_file);
		return false;
	}

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
	if (selbox_bg_image == NULL) {
		log_dir_file_open(CG_DIR, conf_selbox_bg_file);
		return false;
	}
	selbox_fg_image = create_image_from_file(CG_DIR, conf_selbox_fg_file);
	if (selbox_fg_image == NULL) {
		log_dir_file_open(CG_DIR, conf_selbox_fg_file);
		return false;
	}

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
	if (switch_bg_image == NULL) {
		log_dir_file_open(CG_DIR, conf_switch_bg_file);
		return false;
	}

	/* スイッチの選択イメージを読み込む */
	switch_fg_image = create_image_from_file(CG_DIR, conf_switch_fg_file);
	if (selbox_fg_image == NULL) {
		log_dir_file_open(CG_DIR, conf_switch_fg_file);
		return false;
	}

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
	if (save_fg_image == NULL) {
		log_dir_file_open(CG_DIR, conf_save_fg_file);
		log_script_exec_footer();
		return false;
	}

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

	if (w == 0 || h == 0)
		return;

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
void draw_stage_bg_fade(bool is_curtain)
{
	assert(!is_save_mode());
	assert(is_bg_fade_enabled);
	assert(!is_ch_fade_enabled);

	if (!is_curtain) {
		draw_layer_image(back_image, LAYER_FO);
		draw_layer_image(back_image, LAYER_BG_FI);
	} else {
		draw_stage_bg_fade_curtain();
	}
}

/* カーテンフェードの描画を行う */
static void draw_stage_bg_fade_curtain(void)
{
	int right, left, curtain, i;

	/* フェードイン画像の右端を求める */
	right = (int)((float)(conf_window_width + CURTAIN_WIDTH) *
		      (float)layer_alpha[LAYER_BG_FI] / 255.0f);
	if (right >= conf_window_width) {
		curtain = CURTAIN_WIDTH - (right - conf_window_width);
		right = conf_window_width;
	} else {
		curtain = CURTAIN_WIDTH;
	}

	/* フェードイン画像の右端を求める */
	left = right - conf_window_width;
	(void)left;

	/* 背景の非透過部分を描画する */
	draw_image(back_image, right, 0, layer_image[LAYER_FO],
		   conf_window_width - right, conf_window_height,
		   right, 0, 255, BLEND_NONE);

	/* 前景の非透過部分を描画する */
	if (right - curtain > 0) {
		draw_image(back_image, 0, 0, layer_image[LAYER_BG_FI],
			   right - curtain, conf_window_height,
			   0, 0, 255, BLEND_NONE);
	}

	/* 前景の透過部分を描画する */
	for (i = curtain - 1; i >= 0; i--) {
		if (right - i < 0 || right - i >= conf_window_width)
			continue;
		draw_image(back_image, right - i, 0, layer_image[LAYER_BG_FI],
			   1, conf_window_height, right - i, 0, i, BLEND_FAST);
	}
}

/*
 * キャラフェードモードが有効な際のステージ描画を行う
 */
void draw_stage_ch_fade(void)
{
	assert(!is_save_mode());
	assert(!is_bg_fade_enabled);
	assert(is_ch_fade_enabled);

	draw_layer_image(back_image, LAYER_FO);
	draw_layer_image(back_image, LAYER_FI);
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

	layer_alpha[LAYER_BG_FI] = (uint8_t)(progress * 255.0f);
}

/*
 * 背景フェードモードを終了する
 */
void stop_bg_fade(void)
{
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
 * キャラをフェードせずにただちに切り替える
 */
void change_ch_immediately(int pos, struct image *img, int x, int y)
{
	int layer;

	assert(pos == CH_BACK || pos == CH_LEFT || pos == CH_RIGHT ||
	       pos == CH_CENTER);

	layer = pos_to_layer(pos);
	destroy_layer_image(layer);
	layer_image[layer] = img;
	layer_x[layer] = x;
	layer_y[layer] = y;

	assert(layer_alpha[layer] == 255);
}

/*
 * キャラフェードモードを開始する
 */
void start_ch_fade(int pos, struct image *img, int x, int y)
{
	int layer;

	assert(!is_bg_fade_enabled);
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
	layer_alpha[layer] = 255;
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

	layer_alpha[LAYER_FI] = (uint8_t)(progress * 255.0f);
}

/*
 * キャラフェードモードを終了する
 */
void stop_ch_fade(void)
{
	assert(is_ch_fade_enabled);
	assert(!is_bg_fade_enabled);

	is_ch_fade_enabled = false;
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
int draw_char_on_namebox(int x, int y, uint32_t wc)
{
	int w, h;

	draw_char_on_layer(LAYER_NAME, x, y, wc, &w, &h);

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
int draw_char_on_msgbox(int x, int y, uint32_t wc)
{
	int w, h;

	draw_char_on_layer(LAYER_MSG, x, y, wc, &w, &h);

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
	int w, h;

	draw_char_on_layer(LAYER_SEL, x, y, wc, &w, &h);

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
	int w, h;

	draw_char_on_layer(LAYER_FO, x, y, wc, &w, &h);
	draw_char_on_layer(LAYER_FI, x, y, wc, &w, &h);

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
	draw_char_on_layer(LAYER_FI, x, y, wc, w, h);
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
static bool draw_char_on_layer(int layer, int x, int y, uint32_t wc, int *w,
			       int *h)
{
	/* 文字を描画する */
	if (!draw_glyph(layer_image[layer], x, y,
			make_pixel(0xff,
				   (pixel_t)conf_font_color_r,
				   (pixel_t)conf_font_color_g,
				   (pixel_t)conf_font_color_b),
			wc, w, h)) {
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
