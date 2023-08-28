/* -*- coding: utf-8; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Suika 2
 * Copyright (C) 2001-2023, TABATA Keiichi. All rights reserved.
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
 *  - 2021-06-16 時計フェードの対応
 *  - 2021-07-19 複数キャラ・背景同時変更の対応
 *  - 2021-07-19 リファクタ
 *  - 2021-07-20 @chsにエフェクト追加
 *  - 2022-06-29 ルール付き描画に対応, マスクつき描画の削除
 *  - 2022-07-16 システムメニューを追加
 *  - 2022-10-20 キャラ顔絵を追加
 *  - 2023-01-06 日本語の指定に対応
 *  - 2023-08-20 アニメサブシステムの導入, キャラクタアニメの移行
 *
 * [検討]
 *  - ソフトレンダの廃止(GPU描画のみへの移行)
 *  - @bg/@chもアニメーションサブシステムに移行
 *  - 既存の@bg/@chのエフェクトを残す上で、FI/FOレイヤが必要か検討
 *  - FI/FOはソフトレンダ向けの最適化なので、必要なければ廃止
 */

#include "suika.h"

/* false assertion */
#define BAD_POSITION		(0)
#define INVALID_FADE_METHOD	(0)

/* カーテンフェードのカーテンの幅 */
#define CURTAIN_WIDTH	(256)

/* レイヤ */
enum {
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
	 *  - セーブ
	 *  - ロード
	 *  - スイッチ
	 */
	LAYER_FI,	/* 特殊: 実体イメージあり */

	/* 背景レイヤ */
	LAYER_BG,

	/* キャラクタレイヤ(顔以外) */
	LAYER_CHB,
	LAYER_CHL,
	LAYER_CHR,
	LAYER_CHC,

	/* メッセージレイヤ */
	LAYER_MSG,	/* 特殊: 実体イメージあり */

	/* 名前レイヤ */
	LAYER_NAME,	/* 特殊: 実体イメージあり */

	/* キャラクタレイヤ(顔) */
	LAYER_CHF,

	/* クリックアニメーション */
	LAYER_CLICK,	/* 特殊: click_image[i]への参照 */

	/* オートモードバナー */
	LAYER_AUTO,

	/* スキップモードバナー */
	LAYER_SKIP,

	/* 総レイヤ数 */
	STAGE_LAYERS
};

enum stage_mode {
	STAGE_MODE_IDLE,
	STAGE_MODE_BG_FADE,
	STAGE_MODE_CH_FADE,
	STAGE_MODE_CH_ANIME,
	STAGE_MODE_SHAKE,
};

/* ステージの動作モード */
static int stage_mode;

/* レイヤのイメージ */
static struct image *layer_image[STAGE_LAYERS];

/* メッセージボックスの背景イメージ */
static struct image *msgbox_bg_image;

/* メッセージボックスの背景イメージ */
static struct image *msgbox_fg_image;

/* メッセージボックスを表示するか */
static bool is_msgbox_visible;

/* 名前ボックスのイメージ */
static struct image *namebox_image;

/* 名前ボックスを表示するか */
static bool is_namebox_visible;

/* クリックアニメーションのイメージ */
static struct image *click_image[CLICK_FRAMES];

/* クリックアニメーションを表示するか */
static bool is_click_visible;

/* オートモードバナーを表示するか */
static bool is_auto_visible;

/* スキップモードバナーを表示するか */
static bool is_skip_visible;

/* 選択肢(非選択時)のイメージ */
static struct image *switch_bg_image;

/* 選択肢(選択時)のイメージ */
static struct image *switch_fg_image;

/* NEWS(非選択)のイメージ */
static struct image *news_bg_image;

/* NEWS(選択)のイメージ */
static struct image *news_fg_image;

/* システムメニュー(非選択)のイメージ */
static struct image *sysmenu_idle_image;

/* システムメニュー(選択)のイメージ */
static struct image *sysmenu_hover_image;

/* システムメニュー(使用できない時)のイメージ */
static struct image *sysmenu_disable_image;

/* 折りたたみシステムメニュー(非選択)のイメージ */
static struct image *sysmenu_collapsed_idle_image;

/* 折りたたみシステムメニュー(選択)のイメージ */
static struct image *sysmenu_collapsed_hover_image;

/* セーブデータ用のサムネイルイメージ */
static struct image *thumb_image;

/* レイヤのx座標 */
static int layer_x[STAGE_LAYERS];

/* レイヤのy座標 */
static int layer_y[STAGE_LAYERS];

/* レイヤのアルファ値 */
static int layer_alpha[STAGE_LAYERS];

/* レイヤのブレンドタイプ */
static int layer_blend[STAGE_LAYERS];

/* 背景イメージ名 */
static char *bg_file_name;

/* キャライメージ名 */
static char *ch_file_name[CH_ALL_LAYERS];

/* STAGE_MODE_BG_FADEのときの新しい背景 */
static struct image *new_bg_img;

/* ルール画像 */
static struct image *rule_img;

/* FI/FOフェードの進捗 */
static float fi_fo_fade_progress;

/* キャラを暗くするか */
static bool ch_dim[CH_BASIC_LAYERS];

/*
 * 画面揺らしモード中の情報
 */

/* 画面表示オフセット */
static int shake_offset_x;
static int shake_offset_y;

/*
 * GUIモード
 */

static struct image *gui_idle_image;
static struct image *gui_hover_image;
static struct image *gui_active_image;

/*
 * 前方参照
 */
static bool setup_namebox(void);
static bool setup_msgbox(void);
static bool setup_click(void);
static bool setup_switch(void);
static bool setup_news(void);
static bool setup_sysmenu(void);
static bool setup_banner(void);
static bool setup_thumb(void);
static bool create_fade_layer_images(void);
static void destroy_layer_image(int layer);
static void draw_stage_fi_fo_fade(int fade_method);
static void draw_stage_fi_fo_fade_normal(void);
static void draw_stage_fi_fo_fade_rule(void);
static void draw_stage_fi_fo_fade_melt(void);
static void draw_stage_fi_fo_fade_curtain_right(void);
static void draw_stage_fi_fo_fade_curtain_left(void);
static void draw_stage_fi_fo_fade_curtain_up(void);
static void draw_stage_fi_fo_fade_curtain_down(void);
static void draw_stage_fi_fo_fade_slide_right(void);
static void draw_stage_fi_fo_fade_slide_left(void);
static void draw_stage_fi_fo_fade_slide_up(void);
static void draw_stage_fi_fo_fade_slide_down(void);
static void draw_stage_fi_fo_fade_shutter_right(void);
static void draw_stage_fi_fo_fade_shutter_left(void);
static void draw_stage_fi_fo_fade_shutter_up(void);
static void draw_stage_fi_fo_fade_shutter_down(void);
static void draw_stage_fi_fo_fade_clockwise(int method);
static void draw_stage_fi_fo_fade_counterclockwise(int method);
static float cw_step(int method, float progress);
static void draw_stage_fi_fo_fade_eye_open(void);
static void draw_stage_fi_fo_fade_eye_close(void);
static void draw_stage_fi_fo_fade_eye_open_v(void);
static void draw_stage_fi_fo_fade_eye_close_v(void);
static void draw_stage_fi_fo_fade_slit_open(void);
static void draw_stage_fi_fo_fade_slit_close(void);
static void draw_stage_fi_fo_fade_slit_open_v(void);
static void draw_stage_fi_fo_fade_slit_close_v(void);
static int pos_to_layer(int pos);
static int layer_to_pos(int layer);
static void render_layer_image(int layer);
static void draw_layer_image(struct image *target, int layer);
static void render_layer_image_rect(int layer, int x, int y, int w, int h);
static bool draw_char_on_layer(int layer, int x, int y, uint32_t wc,
			       pixel_t color, pixel_t outline_color,int *w,
			       int *h, int base_font_size, bool is_dim);

/*
 * 初期化
 */

/*
 * ステージの初期化処理をする
 */
bool init_stage(void)
{
	int i;

#ifdef ANDROID
	/* 再初期化のための処理 */
	cleanup_stage();
#endif

	/* "cg/"からファイルを読み込む */
	if (!reload_stage())
		return false;

	/* セーブデータのサムネイル画像をセットアップする */
	if (!setup_thumb())
		return false;

	/* 起動直後の仮の背景イメージを作成する */
	layer_image[LAYER_BG] = create_initial_bg();
	if (layer_image[LAYER_BG] == NULL)
		return false;

	/* フェードイン・アウトレイヤのイメージを作成する */
	if (!create_fade_layer_images())
		return false;

	/* ブレンドタイプを設定する */
	layer_blend[LAYER_FO] = BLEND_NONE;
	layer_blend[LAYER_FI] = BLEND_FAST;
	layer_blend[LAYER_BG] = BLEND_NONE;
	layer_blend[LAYER_CHB] = BLEND_FAST;
	layer_blend[LAYER_CHL] = BLEND_FAST;
	layer_blend[LAYER_CHR] = BLEND_FAST;
	layer_blend[LAYER_CHC] = BLEND_FAST;
	layer_blend[LAYER_MSG] = BLEND_FAST;
	layer_blend[LAYER_NAME] = BLEND_FAST;
	layer_blend[LAYER_CHF] = BLEND_FAST;
	layer_blend[LAYER_CLICK] = BLEND_FAST;
	layer_blend[LAYER_AUTO] = BLEND_FAST;
	layer_blend[LAYER_SKIP] = BLEND_FAST;

	/* アルファ値を設定する */
	for (i = 0; i < STAGE_LAYERS; i++)
		layer_alpha[i] = 255;

	return true;
}

/*
 * ステージのリロードを行う
 */
bool reload_stage(void)
{
	/* 名前ボックスをセットアップする */
	if (!setup_namebox())
		return false;

	/* メッセージボックスをセットアップする */
	if (!setup_msgbox())
		return false;

	/* クリックアニメーションをセットアップする */
	if (!setup_click())
		return false;

	/* スイッチをセットアップする */
	if (!setup_switch())
		return false;

	/* NEWSをセットアップする */
	if (!setup_news())
		return false;

	/* システムメニューをセットアップする */
	if (!setup_sysmenu())
		return false;

	/* バナーをセットアップする */
	if (!setup_banner())
		return false;

	return true;
}

/* 名前ボックスをセットアップする */
static bool setup_namebox(void)
{
	is_namebox_visible = false;

	/* 再初期化時に破棄する */
	if (namebox_image != NULL) {
		destroy_image(namebox_image);
		namebox_image = NULL;
	}
	if (layer_image[LAYER_NAME] != NULL) {
		destroy_image(layer_image[LAYER_NAME]);
		layer_image[LAYER_NAME] = NULL;
	}

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

	/* 再初期化時に破棄する */
	if (msgbox_bg_image != NULL) {
		destroy_image(msgbox_bg_image);
		msgbox_bg_image = NULL;
	}
	if (msgbox_fg_image != NULL) {
		destroy_image(msgbox_fg_image);
		msgbox_fg_image = NULL;
	}
	if (layer_image[LAYER_MSG] != NULL) {
		destroy_image(layer_image[LAYER_MSG]);
		layer_image[LAYER_MSG] = NULL;
	}

	/* メッセージボックスの背景画像を読み込む */
	msgbox_bg_image = create_image_from_file(CG_DIR, conf_msgbox_bg_file);
	if (msgbox_bg_image == NULL)
		return false;

	/* メッセージボックスの前景画像を読み込む */
	msgbox_fg_image = create_image_from_file(CG_DIR, conf_msgbox_fg_file);
	if (msgbox_fg_image == NULL)
		return false;

	/* メッセージボックスの前景と背景が同じサイズであることを確認する */
	if (get_image_width(msgbox_bg_image) !=
	    get_image_width(msgbox_fg_image) ||
	    get_image_height(msgbox_bg_image) !=
	    get_image_height(msgbox_fg_image)) {
		log_invalid_msgbox_size();
		return false;
	}

	/* メッセージボックスのレイヤのイメージを作成する */
	layer_image[LAYER_MSG] = create_image(
		get_image_width(msgbox_bg_image),
		get_image_height(msgbox_bg_image));
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
	int i;

	is_click_visible = false;

	/* 再初期化時に破棄する */
	for (i = 0; i < CLICK_FRAMES; i++) {
		if (click_image[i] != NULL) {
			destroy_image(click_image[i]);
			click_image[i] = NULL;
		}
	}

	/* クリックアニメーションの画像を読み込む */
	for (i = 0; i < click_frames; i++) {
		if (conf_click_file[i] != NULL) {
			/* ファイル名が指定されていれば読み込む */
			click_image[i] =
				create_image_from_file(CG_DIR,
						       conf_click_file[i]);
			if (click_image[i] == NULL)
				return false;
		} else {
			/* そうでなければ透明画像を作成する */
			click_image[i] = create_image(1, 1);
			if (click_image[i] == NULL)
				return false;
			lock_image(click_image[i]);
			clear_image_color(click_image[i],
					  make_pixel_slow(0, 0, 0, 0));
			unlock_image(click_image[i]);
		}
	}

	/* クリックアニメーションレイヤの配置を行う */
	layer_x[LAYER_CLICK] = conf_click_x;
	layer_y[LAYER_CLICK] = conf_click_y;

	/* クリックレイヤをいったんNULLにしておく */
	layer_image[LAYER_CLICK] = NULL;

	return true;
}

/* 選択肢をセットアップする */
static bool setup_switch(void)
{
	/* 再初期化時に破棄する */
	if (switch_bg_image != NULL) {
		destroy_image(switch_bg_image);
		switch_bg_image = NULL;
	}
	if (switch_fg_image != NULL) {
		destroy_image(switch_fg_image);
		switch_fg_image = NULL;
	}

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

/* 選択肢をセットアップする */
static bool setup_news(void)
{
	/* 再初期化時に破棄する */
	if (news_bg_image != NULL) {
		destroy_image(news_bg_image);
		news_bg_image = NULL;
	}
	if (news_fg_image != NULL) {
		destroy_image(news_fg_image);
		news_fg_image = NULL;
	}

	/* NEWSの非選択イメージを読み込む */
	if (conf_news_bg_file != NULL) {
		news_bg_image = create_image_from_file(CG_DIR,
						       conf_news_bg_file);
		if (news_bg_image == NULL)
			return false;
	}

	/* NEWSの非選択イメージを読み込む */
	if (conf_news_fg_file != NULL) {
		news_fg_image = create_image_from_file(CG_DIR,
						       conf_news_fg_file);
		if (news_fg_image == NULL)
			return false;
	}

	return true;
}

/* システムメニューをセットアップする */
static bool setup_sysmenu(void)
{
	/* 再初期化時に破棄する */
	if (sysmenu_idle_image != NULL) {
		destroy_image(sysmenu_idle_image);
		sysmenu_idle_image = NULL;
	}
	if (sysmenu_hover_image != NULL) {
		destroy_image(sysmenu_hover_image);
		sysmenu_hover_image = NULL;
	}
	if (sysmenu_disable_image != NULL) {
		destroy_image(sysmenu_disable_image);
		sysmenu_disable_image = NULL;
	}
	if (sysmenu_collapsed_idle_image != NULL) {
		destroy_image(sysmenu_collapsed_idle_image);
		sysmenu_collapsed_idle_image = NULL;
	}
	if (sysmenu_collapsed_hover_image != NULL) {
		destroy_image(sysmenu_collapsed_hover_image);
		sysmenu_collapsed_hover_image = NULL;
	}

	/* システムメニュー(非選択)の画像を読み込む */
	sysmenu_idle_image = create_image_from_file(CG_DIR,
						    conf_sysmenu_idle_file);
	if (sysmenu_idle_image == NULL)
		return false;

	/* システムメニュー(選択)の画像を読み込む */
	sysmenu_hover_image = create_image_from_file(CG_DIR,
						     conf_sysmenu_hover_file);
	if (sysmenu_hover_image == NULL)
		return false;

	/* システムメニュー(使用できない時)の画像を読み込む */
	sysmenu_disable_image = create_image_from_file(
		CG_DIR, conf_sysmenu_disable_file);
	if (sysmenu_disable_image == NULL)
		return false;

	/* 折りたたみシステムメニュー(非選択)の画像を読み込む */
	sysmenu_collapsed_idle_image = create_image_from_file(
		CG_DIR, conf_sysmenu_collapsed_idle_file);
	if (sysmenu_collapsed_idle_image == NULL)
		return false;

	/* システムメニュー(選択)の画像を読み込む */
	sysmenu_collapsed_hover_image = create_image_from_file(
		CG_DIR, conf_sysmenu_collapsed_hover_file);
	if (sysmenu_collapsed_hover_image == NULL)
		return false;

	return true;
}

/* バナーをセットアップする */
static bool setup_banner(void)
{
	is_auto_visible = false;
	is_skip_visible = false;

	/* 再初期化時に破棄する */
	if (layer_image[LAYER_AUTO] != NULL) {
		destroy_image(layer_image[LAYER_AUTO]);
		layer_image[LAYER_AUTO] = NULL;
	}
	if (layer_image[LAYER_SKIP] != NULL) {
		destroy_image(layer_image[LAYER_SKIP]);
		layer_image[LAYER_SKIP] = NULL;
	}

	/* オートモードバナーの画像を読み込む */
	layer_image[LAYER_AUTO] = create_image_from_file(
		CG_DIR, conf_automode_banner_file);
	if (layer_image[LAYER_AUTO] == NULL)
		return false;

	layer_x[LAYER_AUTO] = conf_automode_banner_x;
	layer_y[LAYER_AUTO] = conf_automode_banner_y;

	/* スキップモードバナーの画像を読み込む */
	layer_image[LAYER_SKIP] = create_image_from_file(
		CG_DIR, conf_skipmode_banner_file);
	if (layer_image[LAYER_SKIP] == NULL)
		return false;

	layer_x[LAYER_SKIP] = conf_skipmode_banner_x;
	layer_y[LAYER_SKIP] = conf_skipmode_banner_y;

	return true;
}

/* セーブデータのサムネイル画像をセットアップする */
static bool setup_thumb(void)
{
	/* 再初期化時に破棄する */
	if (thumb_image != NULL) {
		destroy_image(thumb_image);
		thumb_image = NULL;
	}

	/* コンフィグの値がおかしければ補正する */
	if (conf_save_data_thumb_width <= 0)
		conf_save_data_thumb_width = 1;
	if (conf_save_data_thumb_height <= 0)
		conf_save_data_thumb_height = 1;

	/* イメージを作成する */
	thumb_image = create_image(conf_save_data_thumb_width,
				   conf_save_data_thumb_height);
	if (thumb_image == NULL)
		return false;

	return true;
}

/*
 * 起動直後の仮の背景イメージを作成する
 */
struct image *create_initial_bg(void)
{
	struct image *img;

	/* 背景レイヤのイメージを作成する */
	img = create_image(conf_window_width, conf_window_height);
	if (img == NULL)
		return NULL;

	/* 塗り潰す */
	lock_image(img);
	if (conf_window_white)
		clear_image_white(img);
	else
		clear_image_black(img);
	unlock_image(img);

	return img;
}

/* レイヤのイメージを作成する */
static bool create_fade_layer_images(void)
{
	/* 再初期化時に破棄する */
	if (layer_image[LAYER_FO] != NULL)
		destroy_image(layer_image[LAYER_FO]);
	if (layer_image[LAYER_FI] != NULL)
		destroy_image(layer_image[LAYER_FI]);

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

	if (is_gpu_accelerated()) {
		/* 時間のかかるGPUテクスチャ生成を先に行っておく */
		lock_image(layer_image[LAYER_FO]);
		unlock_image(layer_image[LAYER_FO]);
		lock_image(layer_image[LAYER_FI]);
		unlock_image(layer_image[LAYER_FI]);
	}

	return true;
}

/*
 * ステージの終了処理を行う
 */
void cleanup_stage(void)
{
	int i;

	for (i = 0; i < STAGE_LAYERS; i++) {
		if (i == LAYER_CLICK)
			layer_image[i] = NULL;
		else
			destroy_layer_image(i);
	}
	for (i = 0; i <  CLICK_FRAMES; i++) {
		if (click_image[i] != NULL) {
			destroy_image(click_image[i]);
			click_image[i] = NULL;
		}
	}
	if (msgbox_fg_image != NULL) {
		destroy_image(msgbox_fg_image);
		msgbox_fg_image = NULL;
	}
	if (msgbox_bg_image != NULL) {
		destroy_image(msgbox_bg_image);
		msgbox_bg_image = NULL;
	}
	if (namebox_image != NULL) {
		destroy_image(namebox_image);
		namebox_image = NULL;
	}
	if (switch_bg_image != NULL) {
		destroy_image(switch_bg_image);
		switch_bg_image = NULL;
	}
	if (switch_fg_image != NULL) {
		destroy_image(switch_fg_image);
		switch_fg_image = NULL;
	}
	if (news_bg_image != NULL) {
		destroy_image(news_bg_image);
		news_bg_image = NULL;
	}
	if (news_fg_image != NULL) {
		destroy_image(news_fg_image);
		news_fg_image = NULL;
	}
	if (sysmenu_idle_image != NULL) {
		destroy_image(sysmenu_idle_image);
		sysmenu_idle_image = NULL;
	}
	if (sysmenu_hover_image != NULL) {
		destroy_image(sysmenu_hover_image);
		sysmenu_hover_image = NULL;
	}
	if (sysmenu_disable_image != NULL) {
		destroy_image(sysmenu_disable_image);
		sysmenu_disable_image = NULL;
	}
	if (sysmenu_collapsed_idle_image != NULL) {
		destroy_image(sysmenu_collapsed_idle_image);
		sysmenu_collapsed_idle_image = NULL;
	}
	if (sysmenu_collapsed_hover_image != NULL) {
		destroy_image(sysmenu_collapsed_hover_image);
		sysmenu_collapsed_hover_image = NULL;
	}
	if (thumb_image != NULL) {
		destroy_image(thumb_image);
		thumb_image = NULL;
	}
	if (bg_file_name != NULL) {
		free(bg_file_name);
		bg_file_name = NULL;
	}
	for (i = 0; i < CH_ALL_LAYERS; i++) {
		if (ch_file_name[i] != NULL) {
			free(ch_file_name[i]);
			ch_file_name[i] = NULL;
		}
	}
	if (new_bg_img != NULL) {
		destroy_image(new_bg_img);
		new_bg_img = NULL;
	}
	if (rule_img != NULL) {
		destroy_image(rule_img);
		rule_img = NULL;
	}
	if (gui_idle_image != NULL) {
		destroy_image(gui_idle_image);
		gui_idle_image = NULL;
	}
	if (gui_hover_image != NULL) {
		destroy_image(gui_hover_image);
		gui_hover_image = NULL;
	}
	if (gui_active_image != NULL) {
		destroy_image(gui_active_image);
		gui_active_image = NULL;
	}
}

/*
 * レイヤのイメージを破棄する
 */
static void destroy_layer_image(int layer)
{
	assert(layer >= 0 && layer < STAGE_LAYERS);

	if (layer_image[layer] != NULL) {
		destroy_image(layer_image[layer]);
		layer_image[layer] = NULL;
	}
}

/*
 * ステージの描画
 */

/*
 * ステージを描画する
 */
void draw_stage(void)
{
	/* アニメーションサブシステムに更新させるデータの一覧 */
	struct params {
		int anime_layer;
		struct image **image;
		char **fname;
		int *x;
		int *y;
		int *a;
	} params[] = {
		{ANIME_LAYER_BG,
		 &layer_image[LAYER_BG],
		 &bg_file_name,
		 &layer_x[LAYER_BG],
		 &layer_y[LAYER_BG],
		 &layer_alpha[LAYER_BG]},
		{ANIME_LAYER_CHB,
		 &layer_image[LAYER_CHB],
		 &ch_file_name[CH_BACK],
		 &layer_x[LAYER_CHB],
		 &layer_y[LAYER_CHB],
		 &layer_alpha[LAYER_CHB]},
		{ANIME_LAYER_CHL,
		 &layer_image[LAYER_CHL],
		 &ch_file_name[CH_LEFT],
		 &layer_x[LAYER_CHL],
		 &layer_y[LAYER_CHL],
		 &layer_alpha[LAYER_CHL]},
		{ANIME_LAYER_CHR,
		 &layer_image[LAYER_CHR],
		 &ch_file_name[CH_RIGHT],
		 &layer_x[LAYER_CHR],
		 &layer_y[LAYER_CHR],
		 &layer_alpha[LAYER_CHR]},
		{ANIME_LAYER_CHC,
		 &layer_image[LAYER_CHC],
		 &ch_file_name[CH_CENTER],
		 &layer_x[LAYER_CHC],
		 &layer_y[LAYER_CHC],
		 &layer_alpha[LAYER_CHC]},
		{ANIME_LAYER_MSG,
		 NULL,
		 NULL,
		 &layer_x[LAYER_MSG],
		 &layer_y[LAYER_MSG],
		 &layer_alpha[LAYER_MSG]},
		{ANIME_LAYER_NAME,
		 NULL,
		 NULL,
		 &layer_x[LAYER_NAME],
		 &layer_y[LAYER_NAME],
		 &layer_alpha[LAYER_NAME]},
		{ANIME_LAYER_CHF,
		 NULL,
		 NULL,
		 &layer_x[LAYER_CHF],
		 &layer_y[LAYER_CHF],
		 &layer_alpha[LAYER_CHF]}
	};
	int i;

	/*
	 * @bgや@chのフェードはFI/FOレイヤで行うので、それらのフェード中には
	 * draw_stage()は使えない
	 */
	assert(stage_mode != STAGE_MODE_BG_FADE);
	assert(stage_mode != STAGE_MODE_CH_FADE);

	/*
	 * アニメーションのフレーム時刻を更新し、
	 * 完了していればフラグをセットする
	 */
	update_anime_frame();

	/*
	 * 各レイヤの描画パラメータを更新する
	 *  - 画像やファイル名も変わることがある
	 */
	for (i = 0; i < (int)(sizeof(params) / sizeof(struct params)); i++) {
		if (is_anime_running_for_layer(params[i].anime_layer) ||
		    is_anime_finished_for_layer(params[i].anime_layer)) {
			get_anime_layer_params(params[i].anime_layer,
					       params[i].image,
					       params[i].fname,
					       params[i].x,
					       params[i].y,
					       params[i].a);
		}
	}

	/* 描画を行う */
	draw_stage_rect(0, 0, conf_window_width, conf_window_height);
}

/*
 * ステージ全体を描画する(GPU用)
 */
void draw_stage_keep(void)
{
	if (is_gpu_accelerated())
		draw_stage();
}

/*
 * ステージを描画する
 */
void draw_stage_rect(int x, int y, int w, int h)
{
	assert(stage_mode != STAGE_MODE_BG_FADE);
	assert(stage_mode != STAGE_MODE_CH_FADE);
	assert(x >= 0 && y >= 0 && w >= 0 && h >= 0);

	if (is_gpu_accelerated()) {
		x = 0;
		y = 0;
		w = conf_window_width;
		h = conf_window_height;
	}

	if (w == 0 || h == 0)
		return;
	if (x >= conf_window_width || y >= conf_window_height)
		return;
	if (x + w >= conf_window_width)
		w = conf_window_width - x;
	if (y + h >= conf_window_height)
		h = conf_window_height - y;

	/* レイヤを描画する */
	render_layer_image_rect(LAYER_BG, x, y, w, h);
	render_layer_image_rect(LAYER_CHB, x, y, w, h);
	render_layer_image_rect(LAYER_CHL, x, y, w, h);
	render_layer_image_rect(LAYER_CHR, x, y, w, h);
	render_layer_image_rect(LAYER_CHC, x, y, w, h);
	if (is_msgbox_visible)
		render_layer_image_rect(LAYER_MSG, x, y, w, h);
	if (is_namebox_visible && !conf_namebox_hidden)
		render_layer_image_rect(LAYER_NAME, x, y, w,h);
	if (is_msgbox_visible)
		render_layer_image_rect(LAYER_CHF, x, y, w, h);
	if (is_click_visible && !conf_click_disable)
		render_layer_image_rect(LAYER_CLICK, x, y, w, h);
	if (is_auto_visible)
		render_layer_image_rect(LAYER_AUTO, x, y, w, h);
	if (is_skip_visible)
		render_layer_image_rect(LAYER_SKIP, x, y, w, h);
}

/*
 * ルール画像を設定する
 */
void set_rule_image(struct image *img)
{
	if (img == NULL)
		if (rule_img != NULL)
			destroy_image(rule_img);

	rule_img = img;
}

/*
 * 背景フェードモードが有効な際のステージ描画を行う
 */
void draw_stage_bg_fade(int fade_method)
{
	assert(stage_mode == STAGE_MODE_BG_FADE);

	draw_stage_fi_fo_fade(fade_method);
}

/*
 * キャラフェードモードが有効な際のステージ描画を行う (ルール不使用)
 */
void draw_stage_ch_fade(int fade_method)
{
	assert(stage_mode == STAGE_MODE_CH_FADE);

	draw_stage_fi_fo_fade(fade_method);
}

/* FI/FOフェードを行う */
static void draw_stage_fi_fo_fade(int fade_method)
{
	switch (fade_method) {
	case FADE_METHOD_NORMAL:
		draw_stage_fi_fo_fade_normal();
		break;
	case FADE_METHOD_RULE:
		draw_stage_fi_fo_fade_rule();
		break;
	case FADE_METHOD_MELT:
		draw_stage_fi_fo_fade_melt();
		break;
	case FADE_METHOD_CURTAIN_RIGHT:
		draw_stage_fi_fo_fade_curtain_right();
		break;
	case FADE_METHOD_CURTAIN_LEFT:
		draw_stage_fi_fo_fade_curtain_left();
		break;
	case FADE_METHOD_CURTAIN_UP:
		draw_stage_fi_fo_fade_curtain_up();
		break;
	case FADE_METHOD_CURTAIN_DOWN:
		draw_stage_fi_fo_fade_curtain_down();
		break;
	case FADE_METHOD_SLIDE_RIGHT:
		draw_stage_fi_fo_fade_slide_right();
		break;
	case FADE_METHOD_SLIDE_LEFT:
		draw_stage_fi_fo_fade_slide_left();
		break;
	case FADE_METHOD_SLIDE_UP:
		draw_stage_fi_fo_fade_slide_up();
		break;
	case FADE_METHOD_SLIDE_DOWN:
		draw_stage_fi_fo_fade_slide_down();
		break;
	case FADE_METHOD_SHUTTER_RIGHT:
		draw_stage_fi_fo_fade_shutter_right();
		break;
	case FADE_METHOD_SHUTTER_LEFT:
		draw_stage_fi_fo_fade_shutter_left();
		break;
	case FADE_METHOD_SHUTTER_UP:
		draw_stage_fi_fo_fade_shutter_up();
		break;
	case FADE_METHOD_SHUTTER_DOWN:
		draw_stage_fi_fo_fade_shutter_down();
		break;
	case FADE_METHOD_CLOCKWISE:
		draw_stage_fi_fo_fade_clockwise(FADE_METHOD_CLOCKWISE);
		break;
	case FADE_METHOD_COUNTERCLOCKWISE:
		draw_stage_fi_fo_fade_counterclockwise(
			FADE_METHOD_COUNTERCLOCKWISE);
		break;
	case FADE_METHOD_CLOCKWISE20:
		draw_stage_fi_fo_fade_clockwise(FADE_METHOD_CLOCKWISE20);
		break;
	case FADE_METHOD_COUNTERCLOCKWISE20:
		draw_stage_fi_fo_fade_counterclockwise(
			FADE_METHOD_COUNTERCLOCKWISE20);
		break;
	case FADE_METHOD_CLOCKWISE30:
		draw_stage_fi_fo_fade_clockwise(FADE_METHOD_CLOCKWISE30);
		break;
	case FADE_METHOD_COUNTERCLOCKWISE30:
		draw_stage_fi_fo_fade_counterclockwise(
			FADE_METHOD_COUNTERCLOCKWISE30);
		break;
	case FADE_METHOD_EYE_OPEN:
		draw_stage_fi_fo_fade_eye_open();
		break;
	case FADE_METHOD_EYE_CLOSE:
		draw_stage_fi_fo_fade_eye_close();
		break;
	case FADE_METHOD_EYE_OPEN_V:
		draw_stage_fi_fo_fade_eye_open_v();
		break;
	case FADE_METHOD_EYE_CLOSE_V:
		draw_stage_fi_fo_fade_eye_close_v();
		break;
	case FADE_METHOD_SLIT_OPEN:
		draw_stage_fi_fo_fade_slit_open();
		break;
	case FADE_METHOD_SLIT_CLOSE:
		draw_stage_fi_fo_fade_slit_close();
		break;
	case FADE_METHOD_SLIT_OPEN_V:
		draw_stage_fi_fo_fade_slit_open_v();
		break;
	case FADE_METHOD_SLIT_CLOSE_V:
		draw_stage_fi_fo_fade_slit_close_v();
		break;
	default:
		assert(INVALID_FADE_METHOD);
		break;
	}
}

/* デフォルトの背景フェードの描画を行う  */
static void draw_stage_fi_fo_fade_normal(void)
{
	render_layer_image(LAYER_FO);
	render_layer_image(LAYER_FI);
}

/* ルール描画を行う */
static void draw_stage_fi_fo_fade_rule(void)
{
	int threshold;

	assert(stage_mode == STAGE_MODE_BG_FADE ||
	       stage_mode == STAGE_MODE_CH_FADE);
	assert(rule_img != NULL);

	/* テンプレートの閾値を求める */
	threshold = (int)(255.0f * fi_fo_fade_progress);

	/* フェードアウトする画像をコピーする */
	render_image(0, 0, layer_image[LAYER_FO],
		     conf_window_width, conf_window_height,
		     0, 0, 255, BLEND_NONE);

	/* フェードインする画像をレンダリングする */
	render_image_rule(layer_image[LAYER_FI], rule_img, threshold);
}

/* ルール描画(メルト)を行う */
static void draw_stage_fi_fo_fade_melt(void)
{
	int threshold;

	assert(stage_mode == STAGE_MODE_BG_FADE ||
	       stage_mode == STAGE_MODE_CH_FADE);
	assert(rule_img != NULL);

	/* テンプレートの閾値を求める */
	threshold = (int)(255.0f * fi_fo_fade_progress);

	/* フェードアウトする画像をコピーする */
	render_image(0, 0, layer_image[LAYER_FO],
		     conf_window_width, conf_window_height,
		     0, 0, 255, BLEND_NONE);

	/* フェードインする画像をレンダリングする */
	render_image_melt(layer_image[LAYER_FI], rule_img, threshold);
}

/* 右方向カーテンフェードの描画を行う */
static void draw_stage_fi_fo_fade_curtain_right(void)
{
	int right, alpha, i;

	/*
	 * カーテンの右端を求める
	 *  - カーテンの右端は0からconf_window_width+CURTAIN_WIDTHになる
	 */
	right = (int)((float)(conf_window_width + CURTAIN_WIDTH) *
		      fi_fo_fade_progress);

	/* カーテンが通り過ぎる前の背景をコピーする */
	if (right < conf_window_width) {
		render_image(right, 0, layer_image[LAYER_FO],
			     conf_window_width - right, conf_window_height,
			     right, 0, 255, BLEND_NONE);
	}

	/* カーテンの部分の背景をコピーする */
	render_image(right - CURTAIN_WIDTH, 0, layer_image[LAYER_FO],
		     CURTAIN_WIDTH, conf_window_height,
		     right - CURTAIN_WIDTH, 0, 255, BLEND_NONE);

	/* カーテンが通り過ぎた後の背景を描画する */
	if (right >= CURTAIN_WIDTH) {
		render_image(0, 0, layer_image[LAYER_FI],
			     right - CURTAIN_WIDTH, conf_window_height, 0, 0,
			     255, BLEND_NONE);
	}

	/* カーテンを描画する */
	for (alpha = 0, i = right; i >= right - CURTAIN_WIDTH; i--, alpha++) {
		if (i < 0 || i >= conf_window_width)
			continue;
		if (alpha > 255)
			alpha = 255;
		render_image(i, 0, layer_image[LAYER_FI], 2,
			     conf_window_height, i, 0, alpha, BLEND_FAST);
	}
}

/* 左方向カーテンフェードの描画を行う */
static void draw_stage_fi_fo_fade_curtain_left(void)
{
	int left, alpha, i;

	/*
	 * カーテンの左端を求める
	 *  - カーテンの左端はconf_window_widthから-CURTAIN_WIDTになる
	 */
	left = conf_window_width -
		(int)((float)(conf_window_width + CURTAIN_WIDTH) *
		      fi_fo_fade_progress);

	/* カーテンが通り過ぎる前の背景をコピーする */
	render_image(0, 0, layer_image[LAYER_FO], left + CURTAIN_WIDTH,
		     conf_window_height, 0, 0, 255, BLEND_NONE);

	/* カーテンが通り過ぎた後の背景を描画する */
	if (left <= conf_window_width - CURTAIN_WIDTH) {
		render_image(left + CURTAIN_WIDTH, 0, layer_image[LAYER_FI],
			   conf_window_width - left - CURTAIN_WIDTH,
			   conf_window_height, left + CURTAIN_WIDTH, 0, 255,
			   BLEND_NONE);
	}

	/* カーテンを描画する */
	for (alpha = 0, i = left; i <= left + CURTAIN_WIDTH; i++, alpha++) {
		if (i < 0 || i >= conf_window_width)
			continue;
		if (alpha > 255)
			alpha = 255;
		render_image(i, 0, layer_image[LAYER_FI], 1,
			     conf_window_height, i, 0, alpha, BLEND_FAST);
	}
}

/* 上方向カーテンフェードの描画を行う */
static void draw_stage_fi_fo_fade_curtain_up(void)
{
	int top, alpha, i;

	/*
	 * カーテンの左端を求める
	 *  - カーテンの上端はconf_window_heightから-CURTAIN_WIDTHになる
	 */
	top = conf_window_height -
		(int)((float)(conf_window_height + CURTAIN_WIDTH) *
		      fi_fo_fade_progress);

	/* カーテンが通り過ぎる前の背景をコピーする */
	render_image(0, 0, layer_image[LAYER_FO], conf_window_width,
		     top + CURTAIN_WIDTH, 0, 0, 255, BLEND_NONE);

	/* カーテンが通り過ぎた後の背景を描画する */
	if (top <= conf_window_height - CURTAIN_WIDTH) {
		render_image(0, top + CURTAIN_WIDTH, layer_image[LAYER_FI],
			   conf_window_width,
			   conf_window_height - top - CURTAIN_WIDTH,
			   0, top + CURTAIN_WIDTH, 255, BLEND_NONE);
	}

	/* カーテンを描画する */
	for (alpha = 0, i = top; i <= top + CURTAIN_WIDTH; i++, alpha++) {
		if (i < 0 || i >= conf_window_height)
			continue;
		if (alpha > 255)
			alpha = 255;
		render_image(0, i, layer_image[LAYER_FI],
			     conf_window_width, 1, 0, i, alpha, BLEND_FAST);
	}
}

/* 下方向カーテンフェードの描画を行う */
static void draw_stage_fi_fo_fade_curtain_down(void)
{
	int bottom, alpha, i;

	/*
	 * カーテンの下端を求める
	 *  - カーテンの下端は0からconf_window_height+CURTAIN_WIDTHになる
	 */
	bottom = (int)((float)(conf_window_height + CURTAIN_WIDTH) *
		       fi_fo_fade_progress);

	/* カーテンが通り過ぎる前の背景をコピーする */
	if (bottom < conf_window_height) {
		render_image(0, bottom, layer_image[LAYER_FO],
			     conf_window_width, conf_window_height - bottom,
			     0, bottom, 255, BLEND_NONE);
	}

	/* カーテンの部分の背景をコピーする */
	render_image(0, bottom - CURTAIN_WIDTH, layer_image[LAYER_FO],
		     conf_window_width, CURTAIN_WIDTH,
		     0, bottom - CURTAIN_WIDTH, 255, BLEND_NONE);

	/* カーテンが通り過ぎた後の背景を描画する */
	if (bottom >= CURTAIN_WIDTH) {
		render_image(0, 0, layer_image[LAYER_FI], conf_window_width,
			     bottom - CURTAIN_WIDTH, 0, 0, 255, BLEND_NONE);
	}

	/* カーテンを描画する */
	for (alpha = 0, i = bottom; i >= bottom - CURTAIN_WIDTH;
	     i--, alpha++) {
		if (i < 0 || i >= conf_window_height)
			continue;
		if (alpha > 255)
			alpha = 255;
		render_image(0, i, layer_image[LAYER_FI],
			     conf_window_width, 1, 0, i, alpha, BLEND_FAST);
	}
}

/* 右方向スライドフェードの描画を行う */
static void draw_stage_fi_fo_fade_slide_right(void)
{
	int right;

	/*
	 * スライドの右端を求める
	 *  - スライドの右端は0からconf_window_widthになる
	 */
	right = (int)((float)conf_window_width * fi_fo_fade_progress);

	/* 左側の背景を表示する */
	render_image(0, 0, layer_image[LAYER_FI], right, conf_window_height,
		     conf_window_width - right, 0, 255, BLEND_NONE);

	/* 右側の背景を表示する */
	render_image(right, 0, layer_image[LAYER_FO],
		     conf_window_width - right, conf_window_height, 0, 0, 255,
		     BLEND_NONE);
}

/* 左方向スライドフェードの描画を行う */
static void draw_stage_fi_fo_fade_slide_left(void)
{
	int left;

	/*
	 * スライドの左端を求める
	 *  - スライドの左端はconf_window_widthから0になる
	 */
	left = conf_window_width -
		(int)((float)conf_window_width * fi_fo_fade_progress);

	/* 右側の背景を表示する */
	render_image(left, 0, layer_image[LAYER_FI], conf_window_width - left,
		     conf_window_height, 0, 0, 255, BLEND_NONE);

	/* 左側の背景を表示する */
	render_image(0, 0, layer_image[LAYER_FO], left, conf_window_height,
		     conf_window_width - left, 0, 255, BLEND_NONE);
}

/* 上方向スライドフェードの描画を行う */
static void draw_stage_fi_fo_fade_slide_up(void)
{
	int top;

	/*
	 * スライドの上端を求める
	 *  - スライドの上端はconf_window_heightから0になる
	 */
	top = conf_window_height -
		(int)((float)conf_window_height * fi_fo_fade_progress);

	/* 上側の背景を表示する */
	render_image(0, 0, layer_image[LAYER_FO], conf_window_width, top, 0,
		     conf_window_height - top, 255, BLEND_NONE);

	/* 下側の背景を表示する */
	render_image(0, top, layer_image[LAYER_FI], conf_window_width,
		     conf_window_height - top, 0, 0, 255, BLEND_NONE);
}

/* 下方向スライドフェードの描画を行う */
static void draw_stage_fi_fo_fade_slide_down(void)
{
	int bottom;

	/*
	 * スライドの下端を求める
	 *  - スライドの下端は0からconf_window_heightになる
	 */
	bottom = (int)((float)conf_window_height * fi_fo_fade_progress);

	/* 上側の背景を表示する */
	render_image(0, 0, layer_image[LAYER_FI], conf_window_width, bottom, 0,
		     conf_window_height - bottom, 255, BLEND_NONE);

	/* 下側の背景を表示する */
	render_image(0, bottom, layer_image[LAYER_FO], conf_window_width,
		     conf_window_height - bottom, 0, 0, 255, BLEND_NONE);
}

/* 右方向シャッターフェードの描画を行う Right direction shutter fade */
static void draw_stage_fi_fo_fade_shutter_right(void)
{
	int right;

	/*
	 * スライドの右端を求める
	 *  - スライドの右端は0からconf_window_widthになる
	 */
	right = (int)((float)conf_window_width * fi_fo_fade_progress);

	/* 左側の背景を表示する */
	render_image(0, 0, layer_image[LAYER_FI], right, conf_window_height,
		     conf_window_width - right, 0, 255, BLEND_NONE);

	/* 右側の背景を表示する */
	render_image(right, 0, layer_image[LAYER_FO],
		     conf_window_width - right, conf_window_height, right, 0,
		     255, BLEND_NONE);
}

/* 左方向シャッターフェードの描画を行う Left direction shutter fade */
static void draw_stage_fi_fo_fade_shutter_left(void)
{
	int left;

	/*
	 * スライドの左端を求める
	 *  - スライドの左端はconf_window_widthから0になる
	 */
	left = conf_window_width -
		(int)((float)conf_window_width * fi_fo_fade_progress);

	/* 右側の背景を表示する */
	render_image(left, 0, layer_image[LAYER_FI], conf_window_width - left,
		     conf_window_height, 0, 0, 255, BLEND_NONE);

	/* 左側の背景を表示する */
	render_image(0, 0, layer_image[LAYER_FO], left, conf_window_height, 0,
		     0, 255, BLEND_NONE);
}

/* 上方向シャッターフェードの描画を行う Up direction shutter fade */
static void draw_stage_fi_fo_fade_shutter_up(void)
{
	int top;

	/*
	 * スライドの上端を求める
	 *  - スライドの上端はconf_window_heightから0になる
	 */
	top = conf_window_height -
		(int)((float)conf_window_height * fi_fo_fade_progress);

	/* 上側の背景を表示する */
	render_image(0, 0, layer_image[LAYER_FO], conf_window_width, top, 0, 0,
		     255, BLEND_NONE);

	/* 下側の背景を表示する */
	render_image(0, top, layer_image[LAYER_FI], conf_window_width,
		     conf_window_height - top, 0, 0, 255, BLEND_NONE);
}

/* 下方向シャッターフェードの描画を行う Down direction shutter fade */
static void draw_stage_fi_fo_fade_shutter_down(void)
{
	int bottom;

	/*
	 * スライドの下端を求める
	 *  - スライドの下端は0からconf_window_heightになる
	 */
	bottom = (int)((float)conf_window_height * fi_fo_fade_progress);

	/* 上側の背景を表示する */
	render_image(0, 0, layer_image[LAYER_FI], conf_window_width, bottom, 0,
		     conf_window_height - bottom, 255, BLEND_NONE);

	/* 下側の背景を表示する */
	render_image(0, bottom, layer_image[LAYER_FO], conf_window_width,
		     conf_window_height - bottom, 0, bottom, 255, BLEND_NONE);
}

/* 時計回りフェードの描画を行う */
static void draw_stage_fi_fo_fade_clockwise(int method)
{
	const float PI = 3.14159265f;
	float progress, hand_len;
	int hand_x, hand_y, center_x, center_y, i, min, max, half_w, half_h;

	assert(fi_fo_fade_progress >= 0 && fi_fo_fade_progress <= 1.0f);
	progress = cw_step(method, fi_fo_fade_progress);
	assert(progress >= 0 && progress <= 1.0f);

	half_w = conf_window_width / 2;
	half_h = conf_window_height / 2;

	/* フェードアウトする背景の描画を行う */
	render_image(0, 0, layer_image[LAYER_FO], conf_window_width,
		     conf_window_height, 0, 0, 255, BLEND_NONE);

	/* 時計の針の位置を計算する */
	center_x = conf_window_width / 2;
	center_y = conf_window_height / 2;
	hand_len = (conf_window_width > conf_window_height) ?
		(float)conf_window_width : (float)conf_window_height;
	hand_x = center_x + (int)(hand_len *
				  sinf(PI - 2.0f * PI * progress));
	hand_y = center_y + (int)(hand_len *
				  cosf(PI - 2.0f * PI * progress));

	/* 第一象限を埋める */
	if (progress >= 0.25f) {
		render_image(center_x, 0, layer_image[LAYER_FI], half_w,
			     half_h + 1, center_x, 0, 255, BLEND_NONE);
	}

	/* 第四象限を埋める */
	if (progress >= 0.5f) {
		render_image(center_x, center_y, layer_image[LAYER_FI],
			     half_w, half_h, center_x, center_y, 255,
			     BLEND_NONE);
	}

	/* 第三象限を埋める */
	if (progress >= 0.75f) {
		render_image(0, center_y, layer_image[LAYER_FI], half_w + 1,
			     half_h, 0, center_y, 255, BLEND_NONE);
	}

	/* エッジをスキャンする */
	clear_scbuf();
	if (progress < 0.25f) {
		/* 第一象限を処理する */
		scan_edge_min(center_x, 0, center_x, conf_window_height);
		scan_edge_max(conf_window_width, 0, conf_window_width, hand_y);
		scan_edge_max(center_x, center_y, hand_x, hand_y);
	} else if (progress < 0.5f) {
		/* 第四象限を処理する */
		scan_edge_min(center_x, center_y, hand_x, hand_y);
		scan_edge_max(conf_window_width, center_y, conf_window_width,
			      conf_window_height);
	} else if (progress < 0.75f) {
		/* 第三象限を処理する */
		scan_edge_min(0, center_y, 0, conf_window_height);
		scan_edge_min(center_x, center_y, hand_x, hand_y);
		scan_edge_max(center_x, center_y, center_x,
			      conf_window_height);
	} else {
		/* 第二象限を処理する */
		scan_edge_min(0, 0, 0, center_y);
		scan_edge_max(center_x, center_y, hand_x, hand_y);
	}
	
	/* フェードインする背景の描画を行う */
	for (i = 0; i < conf_window_height; i++) {
		/* 走査線の範囲を取得する */
		get_scan_line(i, &min, &max);
		if (max < 0 || min >= conf_window_width)
			continue;

		/* 走査線を描画する */
		render_image(min, i, layer_image[LAYER_FI], max - min + 1,
			     1, min, i, 255, BLEND_NONE);
	}
}

/* 反時計回りフェードの描画を行う */
static void draw_stage_fi_fo_fade_counterclockwise(int method)
{
	const float PI = 3.14159265f;
	float progress, hand_len;
	int hand_x, hand_y, center_x, center_y, i, min, max, half_w, half_h;

	assert(fi_fo_fade_progress >= 0 && fi_fo_fade_progress <= 1.0f);
	progress = cw_step(method, fi_fo_fade_progress);
	assert(progress >= 0 && progress <= 1.0f);

	half_w = conf_window_width / 2;
	half_h = conf_window_height / 2;

	/* フェードアウトする背景の描画を行う */
	render_image(0, 0, layer_image[LAYER_FO], conf_window_width,
		     conf_window_height, 0, 0, 255, BLEND_NONE);

	/* 時計の針の位置を計算する */
	center_x = conf_window_width / 2;
	center_y = conf_window_height / 2;
	hand_len = (conf_window_width > conf_window_height) ?
		(float)conf_window_width : (float)conf_window_height;
	hand_x = center_x + (int)(hand_len *
				  sinf(2.0f * PI * progress - PI));
	hand_y = center_y + (int)(hand_len *
				  cosf(2.0f * PI * progress - PI));

	render_image(hand_x, hand_y, layer_image[LAYER_FI], 100, 100, 0, 0,
		     255, BLEND_NONE);

	/* 第二象限を埋める */
	if (progress >= 0.25f) {
		render_image(0, 0, layer_image[LAYER_FI], half_w, half_h + 1,
			     0, 0, 255, BLEND_NONE);
	}

	/* 第三象限を埋める */
	if (progress >= 0.5f) {
		render_image(0, center_y + 1, layer_image[LAYER_FI], half_w + 1,
			     half_h, 0, center_y + 1, 255, BLEND_NONE);
	}

	/* 第四象限を埋める */
	if (progress >= 0.75f) {
		render_image(center_x, center_y, layer_image[LAYER_FI],
			     half_w, half_h, center_x, center_y, 255,
			     BLEND_NONE);
	}

	/* エッジをスキャンする */
	clear_scbuf();
	if (progress < 0.25f) {
		/* 第二象限を処理する */
		scan_edge_min(0, 0, 0, hand_y);
		scan_edge_min(center_x, center_y, hand_x, hand_y);
		scan_edge_max(center_x - 1, 0, center_x - 1, center_y);
	} else if (progress < 0.5f) {
		/* 第三象限を処理する */
		scan_edge_min(0, center_y, 0, conf_window_height);
		scan_edge_max(center_x, center_y, hand_x, hand_y);
	} else if (progress < 0.75f) {
		/* 第四象限を処理する */
		scan_edge_min(center_x, center_y, center_x,
			      conf_window_height);
		scan_edge_max(conf_window_width, center_y, conf_window_width,
			      conf_window_height);
		scan_edge_max(center_x, center_y, hand_x, hand_y);
	} else {
		/* 第一象限を処理する */
		scan_edge_min(center_x, center_y, hand_x, hand_y);
		scan_edge_max(conf_window_width, 0, conf_window_width,
			      center_y);
	}
	
	/* フェードインする背景の描画を行う */
	for (i = 0; i < conf_window_height; i++) {
		/* 走査線の範囲を取得する */
		get_scan_line(i, &min, &max);
		if (max < 0 || min >= conf_window_width)
			continue;
	
		/* 走査線を描画する */
		render_image(min, i, layer_image[LAYER_FI], max - min + 1, 1,
			     min, i, 255, BLEND_NONE);
	}
}

/* 時計回りの進捗をステップ化する Calc stepped progress of clockwise */
static float cw_step(int method, float progress)
{
	float step, f;

	assert(method == FADE_METHOD_CLOCKWISE ||
	       method == FADE_METHOD_COUNTERCLOCKWISE ||
	       method == FADE_METHOD_CLOCKWISE20 ||
	       method == FADE_METHOD_COUNTERCLOCKWISE20 ||
	       method == FADE_METHOD_CLOCKWISE30 ||
	       method == FADE_METHOD_COUNTERCLOCKWISE30);
	assert(progress >= 0.0f && progress <= 1.0f);

	if (method == FADE_METHOD_CLOCKWISE ||
	    method == FADE_METHOD_COUNTERCLOCKWISE)
		return progress;
	else if (method == FADE_METHOD_CLOCKWISE20 ||
		 method == FADE_METHOD_COUNTERCLOCKWISE20)
		step = 20.0f;
	else
		step = 30.0f;

	progress *= 360.0f;
	for (f = 360.0f; f >= step; f -= step) {
		if (progress >= f - step) {
			progress = f;
			break;
		}
	}
	progress /= 360.0f;

	return progress;
}

/* 目開きフェードの描画を行う */
static void draw_stage_fi_fo_fade_eye_open(void)
{
	int up, down, i, a;
	const int ALPHA_STEP = 4;

	/* 上幕の下端を求める */
	up = (int)((float)(conf_window_height / 2 - 1) -
		   (float)(conf_window_height / 2 - 1) * fi_fo_fade_progress);

	/* 下幕の上端を求める */
	down = (int)((float)(conf_window_height / 2) +
		     (float)(conf_window_height / 2 - 1) * fi_fo_fade_progress);

	/* 幕が通り過ぎた後の背景をコピーする */
	render_image(0, 0, layer_image[LAYER_FI],
		     conf_window_width, conf_window_height,
		     0, 0, 255, BLEND_NONE);

	/* 上幕の描画を行う */
	for (i = up, a = 0; i >= 0; i--) {
		render_image(0, i, layer_image[LAYER_FO],
			     conf_window_width, 1,
			     0, i, a, BLEND_FAST);
		a += ALPHA_STEP;
		if (a > 255)
			a = 255;
	}

	/* 下幕の描画を行う */
	for (i = down, a = 0; i <= conf_window_height - 1; i++) {
		render_image(0, i, layer_image[LAYER_FO],
			     conf_window_width, 1,
			     0, i, a, BLEND_FAST);
		a += ALPHA_STEP;
		if (a > 255)
			a = 255;
	}
}

/* 目閉じフェードの描画を行う */
static void draw_stage_fi_fo_fade_eye_close(void)
{
	int up, down, i, a;
	const int ALPHA_STEP = 4;

	/* 上幕の下端を求める */
	up = (int)((float)(conf_window_height / 2 - 1) * fi_fo_fade_progress);

	/* 下幕の上端を求める */
	down = (int)((float)(conf_window_height - 1) -
		     (float)(conf_window_height / 2 - 1) * fi_fo_fade_progress);

	/* 幕が通り過ぎる前の背景をコピーする */
	render_image(0, 0, layer_image[LAYER_FO],
		     conf_window_width, conf_window_height,
		     0, 0, 255, BLEND_NONE);

	/* 上幕の描画を行う */
	for (i = up, a = 0; i >= 0; i--) {
		render_image(0, i, layer_image[LAYER_FI],
			     conf_window_width, 1,
			     0, i, a, BLEND_FAST);
		a += ALPHA_STEP;
		if (a > 255)
			a = 255;
	}

	/* 下幕の描画を行う */
	for (i = down, a = 0; i <= conf_window_height - 1; i++) {
		render_image(0, i, layer_image[LAYER_FI],
			     conf_window_width, 1,
			     0, i, a, BLEND_FAST);
		a += ALPHA_STEP;
		if (a > 255)
			a = 255;
	}
}

/* 目開きフェード(垂直)の描画を行う */
static void draw_stage_fi_fo_fade_eye_open_v(void)
{
	int left, right, i, a;
	const int ALPHA_STEP = 4;

	/* 左幕の右端を求める */
	left = (int)((float)(conf_window_width / 2 - 1) -
		     (float)(conf_window_width / 2 - 1) * fi_fo_fade_progress);

	/* 右幕の左端を求める */
	right = (int)((float)(conf_window_width / 2) +
		     (float)(conf_window_width / 2 - 1) * fi_fo_fade_progress);

	/* 幕が通り過ぎた後の背景をコピーする */
	render_image(0, 0, layer_image[LAYER_FI],
		     conf_window_width, conf_window_height,
		     0, 0, 255, BLEND_NONE);

	/* 左幕の描画を行う */
	for (i = left, a = 0; i >= 0; i--) {
		render_image(i, 0, layer_image[LAYER_FO],
			     1, conf_window_height,
			     i, 0, a, BLEND_FAST);
		a += ALPHA_STEP;
		if (a > 255)
			a = 255;
	}

	/* 下幕の描画を行う */
	for (i = right, a = 0; i <= conf_window_width - 1; i++) {
		render_image(i, 0, layer_image[LAYER_FO],
			     1, conf_window_height,
			     i, 0, a, BLEND_FAST);
		a += ALPHA_STEP;
		if (a > 255)
			a = 255;
	}
}

/* 目閉じフェード(垂直)の描画を行う */
static void draw_stage_fi_fo_fade_eye_close_v(void)
{
	int left, right, i, a;
	const int ALPHA_STEP = 4;

	/* 左幕の右端を求める */
	left = (int)((float)(conf_window_width / 2 - 1) * fi_fo_fade_progress);

	/* 右幕の左端を求める */
	right = (int)((float)(conf_window_width - 1) -
		      (float)(conf_window_width / 2 - 1) * fi_fo_fade_progress);

	/* 幕が通り過ぎる前の背景をコピーする */
	render_image(0, 0, layer_image[LAYER_FO],
		     conf_window_width, conf_window_height,
		     0, 0, 255, BLEND_NONE);

	/* 左幕の描画を行う */
	for (i = left, a = 0; i >= 0; i--) {
		render_image(i, 0, layer_image[LAYER_FI],
			     1, conf_window_height,
			     i, 0, a, BLEND_FAST);
		a += ALPHA_STEP;
		if (a > 255)
			a = 255;
	}

	/* 右幕の描画を行う */
	for (i = right, a = 0; i <= conf_window_width - 1; i++) {
		render_image(i, 0, layer_image[LAYER_FI],
			     1, conf_window_height,
			     i, 0, a, BLEND_FAST);
		a += ALPHA_STEP;
		if (a > 255)
			a = 255;
	}
}

/* スリット開きフェードの描画を行う */
static void draw_stage_fi_fo_fade_slit_open(void)
{
	int up, down;

	/* 上幕の下端を求める */
	up = (int)((float)(conf_window_height / 2 - 1) -
		   (float)(conf_window_height / 2 - 1) * fi_fo_fade_progress);

	/* 下幕の上端を求める */
	down = (int)((float)(conf_window_height / 2) +
		     (float)(conf_window_height / 2 - 1) * fi_fo_fade_progress);

	/* 幕が通り過ぎた後の背景をコピーする */
	render_image(0, 0, layer_image[LAYER_FI],
		     conf_window_width, conf_window_height,
		     0, 0, 255, BLEND_NONE);

	/* 上幕の描画を行う */
	render_image(0, 0, layer_image[LAYER_FO], conf_window_width, up + 1,
		     0, 0, 255, BLEND_NONE);

	/* 下幕の描画を行う */
	render_image(0, down, layer_image[LAYER_FO],
		     conf_window_width, conf_window_height - down + 1,
		     0, down, 255, BLEND_NONE);
}

/* スリット閉じフェードの描画を行う */
static void draw_stage_fi_fo_fade_slit_close(void)
{
	int up, down;

	/* 上幕の下端を求める */
	up = (int)((float)(conf_window_height / 2 - 1) * fi_fo_fade_progress);

	/* 下幕の上端を求める */
	down = (int)((float)(conf_window_height - 1) -
		     (float)(conf_window_height / 2 - 1) * fi_fo_fade_progress);

	/* 幕が通り過ぎる前の背景をコピーする */
	render_image(0, 0, layer_image[LAYER_FO],
		     conf_window_width, conf_window_height,
		     0, 0, 255, BLEND_NONE);

	/* 上幕の描画を行う */
	render_image(0, 0, layer_image[LAYER_FI], conf_window_width, up + 1,
		     0, 0, 255, BLEND_NONE);

	/* 下幕の描画を行う */
	render_image(0, down, layer_image[LAYER_FI],
		     conf_window_width, conf_window_height - down + 1,
		     0, down, 255, BLEND_NONE);
}

/* スリット開きフェード(垂直)の描画を行う */
static void draw_stage_fi_fo_fade_slit_open_v(void)
{
	int left, right;

	/* 左幕の右端を求める */
	left = (int)((float)(conf_window_width / 2 - 1) -
		     (float)(conf_window_width / 2 - 1) * fi_fo_fade_progress);

	/* 右幕の左端を求める */
	right = (int)((float)(conf_window_width / 2) +
		     (float)(conf_window_width / 2 - 1) * fi_fo_fade_progress);

	/* 幕が通り過ぎた後の背景をコピーする */
	render_image(0, 0, layer_image[LAYER_FI],
		     conf_window_width, conf_window_height,
		     0, 0, 255, BLEND_NONE);

	/* 左幕の描画を行う */
	render_image(0, 0, layer_image[LAYER_FO],
		     left + 1, conf_window_height,
		     0, 0, 255, BLEND_NONE);

	/* 下幕の描画を行う */
	render_image(right, 0, layer_image[LAYER_FO],
		     conf_window_width - right + 1, conf_window_height,
		     right, 0, 255, BLEND_NONE);
}

/* スリット開きフェード(垂直)の描画を行う */
static void draw_stage_fi_fo_fade_slit_close_v(void)
{
	int left, right;

	/* 左幕の右端を求める */
	left = (int)((float)(conf_window_width / 2 - 1) * fi_fo_fade_progress);

	/* 右幕の左端を求める */
	right = (int)((float)(conf_window_width - 1) -
		      (float)(conf_window_width / 2 - 1) * fi_fo_fade_progress);

	/* 幕が通り過ぎる前の背景をコピーする */
	render_image(0, 0, layer_image[LAYER_FO],
		     conf_window_width, conf_window_height,
		     0, 0, 255, BLEND_NONE);

	/* 左幕の描画を行う */
	render_image(0, 0, layer_image[LAYER_FI], left + 1, conf_window_height,
		     0, 0, 255, BLEND_NONE);

	/* 下幕の描画を行う */
	render_image(right, 0, layer_image[LAYER_FI],
		     conf_window_width - right + 1, conf_window_height,
		     right, 0, 255, BLEND_NONE);
}

/*
 * 画面揺らしモードが有効な際のステージ描画を行う
 */
void draw_stage_shake(void)
{
	/* FOレイヤを描画する */
	render_image(0, 0, layer_image[LAYER_FO], conf_window_width,
		     conf_window_height, 0, 0, 255, BLEND_NONE);

	/* FIレイヤを描画する */
	render_image(shake_offset_x, shake_offset_y,
		     layer_image[LAYER_FI], conf_window_width,
		     conf_window_height, 0, 0, 255, BLEND_NONE);

	if (is_auto_visible)
		render_layer_image(LAYER_AUTO);
	if (is_skip_visible)
		render_layer_image(LAYER_SKIP);
}

/*
 * ステージの背景(FO)全体と、前景(FI)の矩形を描画する
 */
void draw_stage_with_button(int x, int y, int w, int h)
{
	assert(stage_mode == STAGE_MODE_IDLE);

	/* 背景を描画する */
	render_image(0, 0, layer_image[LAYER_FO],
		     get_image_width(layer_image[LAYER_FO]),
		     get_image_height(layer_image[LAYER_FO]),
		     0, 0, 255, BLEND_NONE);

	/* ボタンを描画する */
	render_image(x, y, layer_image[LAYER_FI], w, h, x, y, 255, BLEND_NONE);
}

/*
 * ステージの背景(FO)全体と、前景(FI)の矩形を描画する(GPU用)
 */
void draw_stage_with_button_keep(int x, int y, int w, int h)
{
	if (is_gpu_accelerated())
		draw_stage_with_button(x, y, w, h);
}

/*
 * ステージの背景(FO)のうち1矩形と、前景(FI)のうち1矩形を描画する
 */
void draw_stage_rect_with_buttons(int old_x, int old_y, int old_w, int old_h,
				  int new_x, int new_y, int new_w, int new_h)
{
	assert(stage_mode != STAGE_MODE_BG_FADE);
	assert(stage_mode != STAGE_MODE_CH_FADE);

	if (is_gpu_accelerated()) {
		/* 背景を描画する */
		render_image(0, 0, layer_image[LAYER_FO],
			     get_image_width(layer_image[LAYER_FO]),
			     get_image_height(layer_image[LAYER_FO]),
			     0, 0, 255, BLEND_NONE);
	} else {
		/* 古いボタンを消す */
		render_image(old_x, old_y, layer_image[LAYER_FO], old_w, old_h, old_x,
			     old_y, 255, BLEND_NONE);
	}

	/* 新しいボタンを描画する */
	render_image(new_x, new_y, layer_image[LAYER_FI], new_w, new_h, new_x,
		     new_y, 255, BLEND_NONE);
}

/*
 * ステージの背景(FO)と前景(FI)にステージ全体を描画する
 */
void draw_stage_fo_fi(void)
{
	/* FOレイヤを描画する */
	draw_layer_image(layer_image[LAYER_FO], LAYER_BG);
	draw_layer_image(layer_image[LAYER_FO], LAYER_CHB);
	draw_layer_image(layer_image[LAYER_FO], LAYER_CHL);
	draw_layer_image(layer_image[LAYER_FO], LAYER_CHR);
	draw_layer_image(layer_image[LAYER_FO], LAYER_CHC);
	if (is_msgbox_visible)
		draw_layer_image(layer_image[LAYER_FO], LAYER_MSG);
	if (is_namebox_visible)
		draw_layer_image(layer_image[LAYER_FO], LAYER_NAME);
	if (is_msgbox_visible)
		draw_layer_image(layer_image[LAYER_FO], LAYER_CHF);
	draw_layer_image(layer_image[LAYER_FO], LAYER_CHF);
	
	/* FIレイヤを描画する */
	draw_layer_image(layer_image[LAYER_FI], LAYER_BG);
	draw_layer_image(layer_image[LAYER_FI], LAYER_CHB);
	draw_layer_image(layer_image[LAYER_FI], LAYER_CHL);
	draw_layer_image(layer_image[LAYER_FI], LAYER_CHR);
	draw_layer_image(layer_image[LAYER_FI], LAYER_CHC);
	if (is_msgbox_visible)
		draw_layer_image(layer_image[LAYER_FI], LAYER_MSG);
	if (is_namebox_visible)
		draw_layer_image(layer_image[LAYER_FI], LAYER_NAME);
	if (is_msgbox_visible)
		draw_layer_image(layer_image[LAYER_FO], LAYER_CHF);
}

/*
 * システムメニューの描画
 */

/*
 * システムメニューを描画する
 */
void draw_stage_sysmenu(bool is_auto_enabled,
			bool is_skip_enabled,
			bool is_save_load_enabled,
			bool is_qload_enabled,
			bool is_qsave_selected,
			bool is_qload_selected,
			bool is_save_selected,
			bool is_load_selected,
			bool is_auto_selected,
			bool is_skip_selected,
			bool is_history_selected,
			bool is_config_selected,
			bool is_custom1_selected,
			bool is_custom2_selected,
			int *x, int *y, int *w, int *h)
{
	/* 描画範囲を更新する */
	union_rect(x, y, w, h,
		   *x, *y, *w, *h,
		   conf_sysmenu_x,
		   conf_sysmenu_y,
		   get_image_width(sysmenu_idle_image),
		   get_image_height(sysmenu_idle_image));

	/* システムメニューの背景を描画する */
	render_image(conf_sysmenu_x, conf_sysmenu_y,
		     sysmenu_idle_image,
		     get_image_width(sysmenu_idle_image),
		     get_image_height(sysmenu_idle_image),
		     0, 0, 255, BLEND_FAST);

	/* 禁止になっている項目を描画する */
	if (!is_auto_enabled) {
		/* オートの項目(禁止)を描画する */
		render_image(conf_sysmenu_x + conf_sysmenu_auto_x,
			     conf_sysmenu_y + conf_sysmenu_auto_y,
			     sysmenu_disable_image,
			     conf_sysmenu_auto_width,
			     conf_sysmenu_auto_height,
			     conf_sysmenu_auto_x,
			     conf_sysmenu_auto_y, 255, BLEND_FAST);
	}
	if (!is_skip_enabled) {
		/* スキップの項目(禁止)を描画する */
		render_image(conf_sysmenu_x + conf_sysmenu_skip_x,
			     conf_sysmenu_y + conf_sysmenu_skip_y,
			     sysmenu_disable_image,
			     conf_sysmenu_skip_width,
			     conf_sysmenu_skip_height,
			     conf_sysmenu_skip_x,
			     conf_sysmenu_skip_y, 255, BLEND_FAST);
	}
	if (!is_save_load_enabled) {
		/* クイックセーブの項目(禁止)を描画する */
		render_image(conf_sysmenu_x + conf_sysmenu_qsave_x,
			     conf_sysmenu_y + conf_sysmenu_qsave_y,
			     sysmenu_disable_image,
			     conf_sysmenu_qsave_width,
			     conf_sysmenu_qsave_height,
			     conf_sysmenu_qsave_x,
			     conf_sysmenu_qsave_y, 255, BLEND_FAST);

		/* クイックロードの項目(禁止)を描画する */
		render_image(conf_sysmenu_x + conf_sysmenu_qload_x,
			     conf_sysmenu_y + conf_sysmenu_qload_y,
			     sysmenu_disable_image,
			     conf_sysmenu_qload_width,
			     conf_sysmenu_qload_height,
			     conf_sysmenu_qload_x,
			     conf_sysmenu_qload_y, 255, BLEND_FAST);

		/* セーブの項目(禁止)を描画する */
		render_image(conf_sysmenu_x + conf_sysmenu_save_x,
			     conf_sysmenu_y + conf_sysmenu_save_y,
			     sysmenu_disable_image,
			     conf_sysmenu_save_width,
			     conf_sysmenu_save_height,
			     conf_sysmenu_save_x,
			     conf_sysmenu_save_y, 255, BLEND_FAST);

		/* ロードの項目(禁止)を描画する */
		render_image(conf_sysmenu_x + conf_sysmenu_load_x,
			     conf_sysmenu_y + conf_sysmenu_load_y,
			     sysmenu_disable_image,
			     conf_sysmenu_load_width,
			     conf_sysmenu_load_height,
			     conf_sysmenu_load_x,
			     conf_sysmenu_load_y, 255, BLEND_FAST);
	}
	if (is_save_load_enabled && !is_qload_enabled) {
		/* クイックロードの項目(禁止)を描画する */
		render_image(conf_sysmenu_x + conf_sysmenu_qload_x,
			     conf_sysmenu_y + conf_sysmenu_qload_y,
			     sysmenu_disable_image,
			     conf_sysmenu_qload_width,
			     conf_sysmenu_qload_height,
			     conf_sysmenu_qload_x,
			     conf_sysmenu_qload_y, 255, BLEND_FAST);
	}

	/* 選択されている項目を描画する */
	if (is_qsave_selected) {
		/* クイックセーブの項目(選択)を描画する */
		render_image(conf_sysmenu_x + conf_sysmenu_qsave_x,
			     conf_sysmenu_y + conf_sysmenu_qsave_y,
			     sysmenu_hover_image,
			     conf_sysmenu_qsave_width,
			     conf_sysmenu_qsave_height,
			     conf_sysmenu_qsave_x,
			     conf_sysmenu_qsave_y, 255, BLEND_FAST);
	}
	if (is_qload_selected) {
		/* クイックロードの項目(選択)を描画する */
		render_image(conf_sysmenu_x + conf_sysmenu_qload_x,
			     conf_sysmenu_y + conf_sysmenu_qload_y,
			     sysmenu_hover_image,
			     conf_sysmenu_qload_width,
			     conf_sysmenu_qload_height,
			     conf_sysmenu_qload_x,
			     conf_sysmenu_qload_y, 255, BLEND_FAST);
	}
	if (is_save_selected) {
		/* セーブの項目(選択)を描画する */
		render_image(conf_sysmenu_x + conf_sysmenu_save_x,
			     conf_sysmenu_y + conf_sysmenu_save_y,
			     sysmenu_hover_image,
			     conf_sysmenu_save_width,
			     conf_sysmenu_save_height,
			     conf_sysmenu_save_x,
			     conf_sysmenu_save_y, 255, BLEND_FAST);
	}
	if (is_load_selected) {
		/* ロードの項目(選択)を描画する */
		render_image(conf_sysmenu_x + conf_sysmenu_load_x,
			     conf_sysmenu_y + conf_sysmenu_load_y,
			     sysmenu_hover_image,
			     conf_sysmenu_load_width,
			     conf_sysmenu_load_height,
			     conf_sysmenu_load_x,
			     conf_sysmenu_load_y, 255, BLEND_FAST);
	}
	if (is_auto_selected) {
		/* オートの項目(選択)を描画する */
		render_image(conf_sysmenu_x + conf_sysmenu_auto_x,
			     conf_sysmenu_y + conf_sysmenu_auto_y,
			     sysmenu_hover_image,
			     conf_sysmenu_auto_width,
			     conf_sysmenu_auto_height,
			     conf_sysmenu_auto_x,
			     conf_sysmenu_auto_y, 255, BLEND_FAST);
	}
	if (is_skip_selected) {
		/* スキップの項目(選択)を描画する */
		render_image(conf_sysmenu_x + conf_sysmenu_skip_x,
			     conf_sysmenu_y + conf_sysmenu_skip_y,
			     sysmenu_hover_image,
			     conf_sysmenu_skip_width,
			     conf_sysmenu_skip_height,
			     conf_sysmenu_skip_x,
			     conf_sysmenu_skip_y, 255, BLEND_FAST);
	}
	if (is_history_selected) {
		/* ヒストリの項目(選択)を描画する */
		render_image(conf_sysmenu_x + conf_sysmenu_history_x,
			     conf_sysmenu_y + conf_sysmenu_history_y,
			     sysmenu_hover_image,
			     conf_sysmenu_history_width,
			     conf_sysmenu_history_height,
			     conf_sysmenu_history_x,
			     conf_sysmenu_history_y, 255, BLEND_FAST);
	}
	if (is_config_selected) {
		/* コンフィグの項目(選択)を描画する */
		render_image(conf_sysmenu_x + conf_sysmenu_config_x,
			     conf_sysmenu_y + conf_sysmenu_config_y,
			     sysmenu_hover_image,
			     conf_sysmenu_config_width,
			     conf_sysmenu_config_height,
			     conf_sysmenu_config_x,
			     conf_sysmenu_config_y, 255, BLEND_FAST);
	}
	if (is_custom1_selected) {
		/* コンフィグの項目(選択)を描画する */
		render_image(conf_sysmenu_x + conf_sysmenu_custom1_x,
			     conf_sysmenu_y + conf_sysmenu_custom1_y,
			     sysmenu_hover_image,
			     conf_sysmenu_custom1_width,
			     conf_sysmenu_custom1_height,
			     conf_sysmenu_custom1_x,
			     conf_sysmenu_custom1_y, 255, BLEND_FAST);
	}
	if (is_custom2_selected) {
		/* コンフィグの項目(選択)を描画する */
		render_image(conf_sysmenu_x + conf_sysmenu_custom2_x,
			     conf_sysmenu_y + conf_sysmenu_custom2_y,
			     sysmenu_hover_image,
			     conf_sysmenu_custom2_width,
			     conf_sysmenu_custom2_height,
			     conf_sysmenu_custom2_x,
			     conf_sysmenu_custom2_y, 255, BLEND_FAST);
	}
}

/*
 * システムメニューの座標を取得する
 */
void get_sysmenu_rect(int *x, int *y, int *w, int *h)
{
	*x = conf_sysmenu_x;
	*y = conf_sysmenu_y;
	*w = get_image_width(sysmenu_idle_image);
	*h = get_image_height(sysmenu_idle_image);
}

/*
 * 折りたたみシステムメニューを描画する
 */
void draw_stage_collapsed_sysmenu(bool is_pointed,
				  int *x, int *y, int *w, int *h)
{
	/* 描画範囲を更新する */
	union_rect(x, y, w, h,
		   *x, *y, *w, *h,
		   conf_sysmenu_collapsed_x,
		   conf_sysmenu_collapsed_y,
		   get_image_width(sysmenu_collapsed_idle_image),
		   get_image_height(sysmenu_collapsed_idle_image));

	/* 折りたたみシステムメニューの背景を描画する */
	if (!is_pointed) {
		render_image(conf_sysmenu_collapsed_x,
			     conf_sysmenu_collapsed_y,
			     sysmenu_collapsed_idle_image,
			     get_image_width(sysmenu_collapsed_idle_image),
			     get_image_height(sysmenu_collapsed_idle_image),
			     0, 0, 255, BLEND_FAST);
	} else {
		render_image(conf_sysmenu_collapsed_x,
			     conf_sysmenu_collapsed_y,
			     sysmenu_collapsed_hover_image,
			     get_image_width(sysmenu_collapsed_hover_image),
			     get_image_height(sysmenu_collapsed_hover_image),
			     0, 0, 255, BLEND_FAST);
	}
}

/*
 * 折りたたみシステムメニューの座標を取得する
 */
void get_collapsed_sysmenu_rect(int *x, int *y, int *w, int *h)
{
	*x = conf_sysmenu_collapsed_x;
	*y = conf_sysmenu_collapsed_y;
	*w = get_image_width(sysmenu_collapsed_idle_image);
	*h = get_image_height(sysmenu_collapsed_idle_image);
}

/*
 * セーブデータ用サムネイルの描画
 */

/*
 * セーブデータ用サムネイル画像にステージ全体を描画する
 */
void draw_stage_to_thumb(void)
{
	int layer_index[] = {
		LAYER_BG,
		LAYER_CHB,
		LAYER_CHL,
		LAYER_CHR,
		LAYER_CHC,
		LAYER_MSG,
		LAYER_NAME,
		LAYER_CHF
	};
	int i;

	assert(stage_mode == STAGE_MODE_IDLE);

	lock_image(thumb_image);

	for (i = 0; i < (int)(sizeof(layer_index) / sizeof(int)); i++) {
		if (layer_image[layer_index[i]] == NULL)
			continue;
		if (layer_index[i] == LAYER_MSG)
			if (!is_msgbox_visible)
				continue;
		if (layer_index[i] == LAYER_NAME)
			if (!is_namebox_visible || conf_namebox_hidden)
				continue;

		draw_image_scale(thumb_image,
				 conf_window_width,
				 conf_window_height,
				 layer_x[layer_index[i]],
				 layer_y[layer_index[i]],
				 layer_image[layer_index[i]]);
	}

	unlock_image(thumb_image);
}

/*
 * セーブデータ用サムネイル画像にFO全体を描画する
 */
void draw_stage_fo_thumb(void)
{
	assert(stage_mode == STAGE_MODE_IDLE);

	lock_image(thumb_image);
	draw_image_scale(thumb_image,
			 conf_window_width,
			 conf_window_height,
			 0,
			 0,
			 layer_image[LAYER_FO]);
	unlock_image(thumb_image);
}

/*
 * セーブデータ用サムネイル画像を取得する
 */
struct image *get_thumb_image(void)
{
	return thumb_image;
}

/*
 * 背景の変更
 */

/*
 * 文字列からフェードメソッドを取得する (@bg, @ch)
 */
int get_fade_method(const char *method)
{
	/*
	 * ノーマルフェード
	 */

	if (strcmp(method, "normal") == 0 ||
	    strcmp(method, "n") == 0 ||
	    strcmp(method, "") == 0 ||
	    strcmp(method, "mask") == 0 ||
	    strcmp(method, "m") == 0 ||
	    strcmp(method, U8("標準")) == 0)
		return FADE_METHOD_NORMAL;

	/*
	 * カーテンフェード
	 */

	/* カーテンが右方向だけだった頃との互換性のため、省略形が複数ある */
	if (strcmp(method, "curtain-right") == 0 ||
	    strcmp(method, "curtain") == 0 ||
	    strcmp(method, "cr") == 0 ||
	    strcmp(method, "c") == 0 ||
	    strcmp(method, U8("右カーテン")) == 0)
		return FADE_METHOD_CURTAIN_RIGHT;

	if (strcmp(method, "curtain-left") == 0 ||
	    strcmp(method, "cl") == 0 ||
	    strcmp(method, U8("左カーテン")) == 0)
		return FADE_METHOD_CURTAIN_LEFT;

	if (strcmp(method, "curtain-up") == 0 ||
	    strcmp(method, "cu") == 0 ||
	    strcmp(method, U8("上カーテン")) == 0)
		return FADE_METHOD_CURTAIN_UP;

	if (strcmp(method, "curtain-down") == 0 ||
	    strcmp(method, "cd") == 0 ||
	    strcmp(method, U8("下カーテン")) == 0)
		return FADE_METHOD_CURTAIN_DOWN;

	/*
	 * スライドフェード
	 */

	if (strcmp(method, "slide-right") == 0 ||
	    strcmp(method, "sr") == 0 ||
	    strcmp(method, U8("右スライド")) == 0)
		return FADE_METHOD_SLIDE_RIGHT;

	if (strcmp(method, "slide-left") == 0 ||
	    strcmp(method, "sl") == 0 ||
    	    strcmp(method, U8("左スライド")) == 0)

		return FADE_METHOD_SLIDE_LEFT;

	if (strcmp(method, "slide-up") == 0 ||
	    strcmp(method, "su") == 0 ||
	    strcmp(method, U8("上スライド")) == 0)
		return FADE_METHOD_SLIDE_UP;

	if (strcmp(method, "slide-down") == 0 ||
	    strcmp(method, "sd") == 0 ||
	    strcmp(method, U8("下スライド")) == 0)
		return FADE_METHOD_SLIDE_DOWN;

	/*
	 * シャッターフェード Shutter fade
	 */

	if (strcmp(method, "shutter-right") == 0 ||
	    strcmp(method, "shr") == 0 ||
	    strcmp(method, U8("右シャッター")) == 0)
		return FADE_METHOD_SHUTTER_RIGHT;

	if (strcmp(method, "shutter-left") == 0 ||
	    strcmp(method, "shl") == 0 ||
	    strcmp(method, U8("左シャッター")) == 0)
		return FADE_METHOD_SHUTTER_LEFT;

	if (strcmp(method, "shutter-up") == 0 ||
	    strcmp(method, "shu") == 0 ||
	    strcmp(method, U8("上シャッター")) == 0)
		return FADE_METHOD_SHUTTER_UP;

	if (strcmp(method, "shutter-down") == 0 ||
	    strcmp(method, "shd") == 0 ||
	    strcmp(method, U8("下シャッター")) == 0)
		return FADE_METHOD_SHUTTER_DOWN;

	/*
	 * 時計フェード
	 */

	if (strcmp(method, "clockwise") == 0 ||
	    strcmp(method, "cw") == 0 ||
    	    strcmp(method, U8("時計回り")) == 0)
		return FADE_METHOD_CLOCKWISE;

	if (strcmp(method, "counterclockwise") == 0 ||
	    strcmp(method, "ccw") == 0 ||
    	    strcmp(method, U8("反時計回り")) == 0)
		return FADE_METHOD_COUNTERCLOCKWISE;

	/*
	 * 時計フェード(ステップ20°) Clockwise(20 degrees stepped)
	 */

	if (strcmp(method, "clockwise20") == 0 ||
	    strcmp(method, "cw20") == 0)
		return FADE_METHOD_CLOCKWISE20;

	if (strcmp(method, "counterclockwise20") == 0 ||
	    strcmp(method, "ccw20") == 0)
		return FADE_METHOD_COUNTERCLOCKWISE20;

	/*
	 * 時計フェード(ステップ30°) Clockwise(30 degrees stepped)
	 */

	if (strcmp(method, "clockwise30") == 0 ||
	    strcmp(method, "cw30") == 0)
		return FADE_METHOD_CLOCKWISE30;

	if (strcmp(method, "counterclockwise30") == 0 ||
	    strcmp(method, "ccw30") == 0)
		return FADE_METHOD_COUNTERCLOCKWISE30;

	/*
	 * 目開き/目閉じフェード eye-open/eye-close
	 */

	if (strcmp(method, "eye-open") == 0)
		return FADE_METHOD_EYE_OPEN;

	if (strcmp(method, "eye-close") == 0)
		return FADE_METHOD_EYE_CLOSE;

	if (strcmp(method, "eye-open-v") == 0)
		return FADE_METHOD_EYE_OPEN_V;

	if (strcmp(method, "eye-close-v") == 0)
		return FADE_METHOD_EYE_CLOSE_V;

	/*
	 * スリット開き/スリット閉じフェード slit-open/slit-close
	 */

	if (strcmp(method, "slit-open") == 0)
		return FADE_METHOD_SLIT_OPEN;

	if (strcmp(method, "slit-close") == 0)
		return FADE_METHOD_SLIT_CLOSE;

	if (strcmp(method, "slit-open-v") == 0)
		return FADE_METHOD_SLIT_OPEN_V;

	if (strcmp(method, "slit-close-v") == 0)
		return FADE_METHOD_SLIT_CLOSE_V;

	/*
	 * ルール
	 */

	if (strncmp(method, "rule:", 5) == 0)
		return FADE_METHOD_RULE;

	if (strncmp(method, "melt:", 5) == 0)
		return FADE_METHOD_MELT;

	/* 不正なフェード指定 */
	return FADE_METHOD_INVALID;
}

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
	assert(img != NULL);

	destroy_layer_image(LAYER_BG);
	layer_image[LAYER_BG] = img;
}

/*
 * 背景フェードを開始する
 */
void start_bg_fade(struct image *img)
{
	assert(stage_mode == STAGE_MODE_IDLE);

	/* 背景フェードを有効にする */
	stage_mode = STAGE_MODE_BG_FADE;

	/* フェードアウト用のレイヤにステージを描画する */
	lock_image(layer_image[LAYER_FO]);
	draw_layer_image(layer_image[LAYER_FO], LAYER_BG);
	draw_layer_image(layer_image[LAYER_FO], LAYER_CHB);
	draw_layer_image(layer_image[LAYER_FO], LAYER_CHL);
	draw_layer_image(layer_image[LAYER_FO], LAYER_CHR);
	draw_layer_image(layer_image[LAYER_FO], LAYER_CHC);
	if (is_msgbox_visible)
		draw_layer_image(layer_image[LAYER_FO], LAYER_MSG);
	if (is_namebox_visible && !conf_namebox_hidden)
		draw_layer_image(layer_image[LAYER_FO], LAYER_NAME);
	if (is_msgbox_visible)
		draw_layer_image(layer_image[LAYER_FO], LAYER_CHF);
	if (is_auto_visible)
		draw_layer_image(layer_image[LAYER_FO], LAYER_AUTO);
	if (is_skip_visible)
		draw_layer_image(layer_image[LAYER_FO], LAYER_SKIP);
	unlock_image(layer_image[LAYER_FO]);

	/* フェードイン用のレイヤにイメージをセットする */
	new_bg_img = img;

	/* フェードイン用のレイヤに背景を描画する */
	lock_image(layer_image[LAYER_FI]);
	draw_image(layer_image[LAYER_FI], 0, 0, img, conf_window_width,
		   conf_window_height, 0, 0, 255, BLEND_NONE);
	if (conf_msgbox_show_on_bg) {
		if (is_msgbox_visible)
			draw_layer_image(layer_image[LAYER_FI], LAYER_MSG);
		if (is_namebox_visible && !conf_namebox_hidden)
			draw_layer_image(layer_image[LAYER_FI], LAYER_NAME);
		if (is_msgbox_visible)
			draw_layer_image(layer_image[LAYER_FI], LAYER_CHF);
	}
	if (is_auto_visible)
		draw_layer_image(layer_image[LAYER_FI], LAYER_AUTO);
	if (is_skip_visible)
		draw_layer_image(layer_image[LAYER_FI], LAYER_SKIP);
	unlock_image(layer_image[LAYER_FI]);

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
	assert(stage_mode == STAGE_MODE_BG_FADE);

	/* 進捗率を保存する */
	fi_fo_fade_progress = progress;

	/* アルファ値に変換する */
	layer_alpha[LAYER_FI] = (uint8_t)(progress * 255.0f);
}

/*
 * 背景フェードモードを終了する
 */
void stop_bg_fade(void)
{
	assert(stage_mode == STAGE_MODE_BG_FADE);

	/* ルールイメージを破棄する */
	if (rule_img != NULL) {
		destroy_image(rule_img);
		rule_img = NULL;
	}

	stage_mode = STAGE_MODE_IDLE;
	destroy_layer_image(LAYER_BG);
	layer_image[LAYER_BG] = new_bg_img;
	new_bg_img = NULL;
}

/*
 * キャラの変更
 */

/*
 * キャラのファイル名を設定する
 */
bool set_ch_file_name(int pos, const char *file)
{
	assert(pos >= 0 && pos < CH_ALL_LAYERS);
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
	assert(pos >= 0 && pos < CH_ALL_LAYERS);

	return ch_file_name[pos];
}

/*
 * キャラの座標を取得する
 */
void get_ch_position(int pos, int *x, int *y)
{
	int layer;

	assert(pos >= 0 && pos < CH_ALL_LAYERS);

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

	assert(pos >= 0 && pos < CH_ALL_LAYERS);

	layer = pos_to_layer(pos);
	return layer_alpha[layer];
}

/*
 * キャラを暗くするかを設定する
 */
void set_ch_dim(int pos, bool dim)
{
	assert(pos >= 0 && pos < CH_BASIC_LAYERS);

	ch_dim[pos] = dim;
}

/*
 * キャラをフェードせずにただちに切り替える
 */
void change_ch_immediately(int pos, struct image *img, int x, int y, int alpha)
{
	int layer;

	assert(pos >= 0 && pos < CH_ALL_LAYERS);

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

	assert(pos >= 0 && pos < CH_ALL_LAYERS);

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

	assert(stage_mode == STAGE_MODE_IDLE);
	assert(pos >= 0 && pos < CH_ALL_LAYERS);

	stage_mode = STAGE_MODE_CH_FADE;

	/* キャラフェードアウトレイヤにステージを描画する */
	lock_image(layer_image[LAYER_FO]);
	draw_layer_image(layer_image[LAYER_FO], LAYER_BG);
	draw_layer_image(layer_image[LAYER_FO], LAYER_CHB);
	draw_layer_image(layer_image[LAYER_FO], LAYER_CHL);
	draw_layer_image(layer_image[LAYER_FO], LAYER_CHR);
	draw_layer_image(layer_image[LAYER_FO], LAYER_CHC);
	if (conf_msgbox_show_on_ch) {
		if (is_msgbox_visible)
			draw_layer_image(layer_image[LAYER_FO], LAYER_MSG);
		if (is_namebox_visible && !conf_namebox_hidden)
			draw_layer_image(layer_image[LAYER_FO], LAYER_NAME);
		if (is_msgbox_visible)
			draw_layer_image(layer_image[LAYER_FO], LAYER_CHF);
	}
	if (is_auto_visible)
		draw_layer_image(layer_image[LAYER_FO], LAYER_AUTO);
	if (is_skip_visible)
		draw_layer_image(layer_image[LAYER_FO], LAYER_SKIP);
	unlock_image(layer_image[LAYER_FO]);

	/* キャラを入れ替える */
	layer = pos_to_layer(pos);
	destroy_layer_image(layer);
	layer_image[layer] = img;
	layer_alpha[layer] = alpha;
	layer_x[layer] = x;
	layer_y[layer] = y;

	/* キャラフェードインレイヤにステージを描画する */
	lock_image(layer_image[LAYER_FI]);
	draw_layer_image(layer_image[LAYER_FI], LAYER_BG);
	draw_layer_image(layer_image[LAYER_FI], LAYER_CHB);
	draw_layer_image(layer_image[LAYER_FI], LAYER_CHL);
	draw_layer_image(layer_image[LAYER_FI], LAYER_CHR);
	draw_layer_image(layer_image[LAYER_FI], LAYER_CHC);
	if (conf_msgbox_show_on_ch) {
		if (is_msgbox_visible)
			draw_layer_image(layer_image[LAYER_FI], LAYER_MSG);
		if (is_namebox_visible && !conf_namebox_hidden)
			draw_layer_image(layer_image[LAYER_FI], LAYER_NAME);
		if (is_msgbox_visible)
			draw_layer_image(layer_image[LAYER_FI], LAYER_CHF);
	}
	if (is_auto_visible)
		draw_layer_image(layer_image[LAYER_FI], LAYER_AUTO);
	if (is_skip_visible)
		draw_layer_image(layer_image[LAYER_FI], LAYER_SKIP);
	unlock_image(layer_image[LAYER_FI]);
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
	case CH_FACE:
		return LAYER_CHF;
	}
	assert(BAD_POSITION);
	return -1;	/* never come here */
}

/* レイヤインデックスをキャラの位置に変換する */
static int layer_to_pos(int layer)
{
	assert(layer == LAYER_CHB || layer == LAYER_CHL ||
	       layer == LAYER_CHR || layer == LAYER_CHC);
	switch (layer) {
	case LAYER_CHB:
		return CH_BACK;
	case LAYER_CHL:
		return CH_LEFT;
	case LAYER_CHR:
		return CH_RIGHT;
	case LAYER_CHC:
		return CH_CENTER;
	default:
		assert(0);
		break;
	}
	return -1;
}

/*
 * キャラフェードモード(複数,背景も)を開始する
 */
void start_ch_fade_multi(const bool *stay, struct image **img, const int *x,
			 const int *y)
{
	int i, layer;

	assert(stage_mode == STAGE_MODE_IDLE);

	/* このフェードでもSTAGE_MODE_CH_FADEを利用する */
	stage_mode = STAGE_MODE_CH_FADE;

	/* キャラフェードアウトレイヤにステージを描画する */
	lock_image(layer_image[LAYER_FO]);
	draw_layer_image(layer_image[LAYER_FO], LAYER_BG);
	draw_layer_image(layer_image[LAYER_FO], LAYER_CHB);
	draw_layer_image(layer_image[LAYER_FO], LAYER_CHL);
	draw_layer_image(layer_image[LAYER_FO], LAYER_CHR);
	draw_layer_image(layer_image[LAYER_FO], LAYER_CHC);
	if (conf_msgbox_show_on_ch) {
		if (is_msgbox_visible)
			draw_layer_image(layer_image[LAYER_FO], LAYER_MSG);
		if (is_namebox_visible && !conf_namebox_hidden)
			draw_layer_image(layer_image[LAYER_FO], LAYER_NAME);
		if (is_msgbox_visible)
			draw_layer_image(layer_image[LAYER_FO], LAYER_CHF);
	}
	if (is_auto_visible)
		draw_layer_image(layer_image[LAYER_FO], LAYER_AUTO);
	if (is_skip_visible)
		draw_layer_image(layer_image[LAYER_FO], LAYER_SKIP);
	unlock_image(layer_image[LAYER_FO]);

	/* キャラを入れ替える */
	for (i = 0; i < CH_BASIC_LAYERS; i++) {
		if (!stay[i]) {
			layer = pos_to_layer(i);
			destroy_layer_image(layer);
			layer_image[layer] = img[i];
			layer_alpha[layer] = 255;
			layer_x[layer] = x[i];
			layer_y[layer] = y[i];
		}
	}

	/* 背景を入れ替える */
	if (!stay[CH_BASIC_LAYERS]) {
		destroy_layer_image(LAYER_BG);
		layer_image[LAYER_BG] = img[CH_BASIC_LAYERS];
	}

	/* キャラフェードインレイヤにステージを描画する */
	lock_image(layer_image[LAYER_FI]);
	draw_layer_image(layer_image[LAYER_FI], LAYER_BG);
	draw_layer_image(layer_image[LAYER_FI], LAYER_CHB);
	draw_layer_image(layer_image[LAYER_FI], LAYER_CHL);
	draw_layer_image(layer_image[LAYER_FI], LAYER_CHR);
	draw_layer_image(layer_image[LAYER_FI], LAYER_CHC);
	if (conf_msgbox_show_on_ch) {
		if (is_msgbox_visible)
			draw_layer_image(layer_image[LAYER_FI], LAYER_MSG);
		if (is_namebox_visible && !conf_namebox_hidden)
			draw_layer_image(layer_image[LAYER_FI], LAYER_NAME);
		if (is_msgbox_visible)
			draw_layer_image(layer_image[LAYER_FI], LAYER_CHF);
	}
	if (is_auto_visible)
		draw_layer_image(layer_image[LAYER_FI], LAYER_AUTO);
	if (is_skip_visible)
		draw_layer_image(layer_image[LAYER_FI], LAYER_SKIP);
	unlock_image(layer_image[LAYER_FI]);
}

/*
 * キャラフェードモードの進捗率を設定する
 */
void set_ch_fade_progress(float progress)
{
	assert(stage_mode == STAGE_MODE_CH_FADE);

	/* 進捗率を保存する */
	fi_fo_fade_progress = progress;

	/* アルファ値に変換する */
	layer_alpha[LAYER_FI] = (uint8_t)(progress * 255.0f);
}

/*
 * キャラフェードモードを終了する
 */
void stop_ch_fade(void)
{
	assert(stage_mode == STAGE_MODE_CH_FADE);

	/* ルールイメージを破棄する */
	if (rule_img != NULL) {
		destroy_image(rule_img);
		rule_img = NULL;
	}

	stage_mode = STAGE_MODE_IDLE;
}

/*
 * 画面揺らしモード
 */

/* 画面揺らしモードを開始する */
void start_shake(void)
{
	assert(stage_mode == STAGE_MODE_IDLE);

	stage_mode = STAGE_MODE_SHAKE;

	/* フェードアウト用のレイヤをクリアする */
	lock_image(layer_image[LAYER_FO]);
	if (conf_window_white)
		clear_image_white(layer_image[LAYER_FO]);
	else
		clear_image_black(layer_image[LAYER_FO]);
	unlock_image(layer_image[LAYER_FO]);

	/* フェードイン用のレイヤにステージを描画する */
	lock_image(layer_image[LAYER_FI]);
	draw_layer_image(layer_image[LAYER_FI], LAYER_BG);
	draw_layer_image(layer_image[LAYER_FI], LAYER_CHB);
	draw_layer_image(layer_image[LAYER_FI], LAYER_CHL);
	draw_layer_image(layer_image[LAYER_FI], LAYER_CHR);
	draw_layer_image(layer_image[LAYER_FI], LAYER_CHC);
	if (conf_msgbox_show_on_bg) {
		if (is_msgbox_visible)
			draw_layer_image(layer_image[LAYER_FI], LAYER_MSG);
		if (is_namebox_visible && !conf_namebox_hidden)
			draw_layer_image(layer_image[LAYER_FI], LAYER_NAME);
		if (is_msgbox_visible)
			draw_layer_image(layer_image[LAYER_FI], LAYER_CHF);
	}
	unlock_image(layer_image[LAYER_FI]);
}

/* 画面揺らしモードの表示オフセットを設定する */
void set_shake_offset(int x, int y)
{
	assert(stage_mode == STAGE_MODE_SHAKE);

	shake_offset_x = x;
	shake_offset_y = y;
}

/* 画面揺らしモードを終了する */
void stop_shake(void)
{
	assert(stage_mode == STAGE_MODE_SHAKE);

	stage_mode = STAGE_MODE_IDLE;
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

	lock_image(layer_image[LAYER_NAME]);
	draw_image(layer_image[LAYER_NAME], 0, 0, namebox_image,
		   get_image_width(layer_image[LAYER_NAME]),
		   get_image_height(layer_image[LAYER_NAME]),
		   0, 0, 255, BLEND_NONE);
	unlock_image(layer_image[LAYER_NAME]);
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
void draw_char_on_namebox(int x, int y, uint32_t wc, pixel_t color,
			  pixel_t outline_color, int *w, int *h,
			  int base_font_size, bool is_dim)
{
	lock_image(layer_image[LAYER_NAME]);
	draw_char_on_layer(LAYER_NAME, x, y, wc, color, outline_color, w, h,
			   base_font_size, is_dim);
	unlock_image(layer_image[LAYER_NAME]);
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
 * メッセージボックスを背景でクリアする
 */
void clear_msgbox(void)
{
	if (msgbox_bg_image == NULL)
		return;

	lock_image(layer_image[LAYER_MSG]);
	draw_image(layer_image[LAYER_MSG], 0, 0, msgbox_bg_image,
		   get_image_width(layer_image[LAYER_MSG]),
		   get_image_height(layer_image[LAYER_MSG]),
		   0, 0, 255, BLEND_NONE);
	unlock_image(layer_image[LAYER_MSG]);
}

/*
 * メッセージボックスの矩形を背景でクリアする
 */
void clear_msgbox_rect_with_bg(int x, int y, int w, int h)
{
	if (msgbox_bg_image == NULL)
		return;

	lock_image(layer_image[LAYER_MSG]);
	draw_image(layer_image[LAYER_MSG], x, y, msgbox_bg_image, w, h, x, y,
		   255, BLEND_NONE);
	unlock_image(layer_image[LAYER_MSG]);
}

/*
 * メッセージボックスの矩形を前景でクリアする
 */
void clear_msgbox_rect_with_fg(int x, int y, int w, int h)
{
	if (msgbox_fg_image == NULL)
		return;

	lock_image(layer_image[LAYER_MSG]);
	draw_image(layer_image[LAYER_MSG], x, y, msgbox_fg_image, w, h, x, y,
		   255, BLEND_NONE);
	unlock_image(layer_image[LAYER_MSG]);
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
void draw_char_on_msgbox(int x, int y, uint32_t wc, pixel_t color,
			 pixel_t outline_color, int *w, int *h,
			 int base_font_size, bool is_dim)
{
	lock_image(layer_image[LAYER_MSG]);
	draw_char_on_layer(LAYER_MSG, x, y, wc, color, outline_color, w, h,
			   base_font_size, is_dim);
	unlock_image(layer_image[LAYER_MSG]);
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
 * クリックアニメーションの位置を設定する
 */
void set_click_position(int x, int y)
{
	layer_x[LAYER_CLICK] = x;
	layer_y[LAYER_CLICK] = y;
}

/*
 * クリックアニメーションの表示・非表示を設定する
 */
void show_click(bool show)
{
	is_click_visible = show;
}

/*
 * クリックアニメーションのフレーム番号を指定する
 */
void set_click_index(int index)
{
	assert(index >= 0 && index < CLICK_FRAMES);
	assert(index < click_frames);

	layer_image[LAYER_CLICK] = click_image[index];
}

/*
 * スイッチ(@choose, @select, @switch)の描画
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
 * NEWSの親選択肢の矩形を取得する
 */
void get_news_rect(int index, int *x, int *y, int *w, int *h)
{
	const int NORTH = 0;
	const int EAST = 1;
	const int WEST = 2;
	const int SOUTH = 3;
	const int SWITCH_BASE = 4;

	struct image *bg;

	bg = news_bg_image != NULL ? news_bg_image : switch_bg_image;

	if (index == NORTH) {
		*w = get_image_width(bg);
		*h = get_image_height(bg);
		*x = (conf_window_width - *w) / 2;
		*y = conf_switch_y;
	} else if (index == EAST) {
		*w = get_image_width(bg);
		*h = get_image_height(bg);
		*x = conf_window_width - *w - conf_news_margin;
		*y = conf_switch_y + *h + conf_news_margin;
	} else if (index == WEST) {
		*w = get_image_width(bg);
		*h = get_image_height(bg);
		*x = conf_news_margin;
		*y = conf_switch_y + *h + conf_news_margin;
	} else if (index == SOUTH) {
		*w = get_image_width(bg);
		*h = get_image_height(bg);
		*x = (conf_window_width - *w) / 2;
		*y = conf_switch_y + (*h + conf_news_margin) * 2;
	} else {
		*w = get_image_width(switch_bg_image);
		*h = get_image_height(switch_bg_image);
		*x = conf_switch_x;
		*y = conf_switch_y + (*h + conf_news_margin) * 3 +
			(get_image_height(switch_bg_image) +
			 conf_switch_margin_y) *
			(index - SWITCH_BASE);
	}
}

/*
 * FOレイヤにスイッチの非選択イメージを描画する
 */
void draw_switch_bg_image(int x, int y)
{
	draw_image(layer_image[LAYER_FO], x, y, switch_bg_image,
		   get_image_width(switch_bg_image),
		   get_image_height(switch_bg_image),
		   0, 0, 255, BLEND_NORMAL);
}

/*
 * FIレイヤにスイッチの選択イメージを描画する
 */
void draw_switch_fg_image(int x, int y)
{
	draw_image(layer_image[LAYER_FI], x, y, switch_fg_image,
		   get_image_width(switch_fg_image),
		   get_image_height(switch_fg_image),
		   0, 0, 255, BLEND_NORMAL);
}

/*
 * FOレイヤにNEWSの非選択イメージを描画する
 */
void draw_news_bg_image(int x, int y)
{
	struct image *img;

	img = news_bg_image != NULL ? news_bg_image : switch_bg_image;

	draw_image(layer_image[LAYER_FO], x, y, img,
		   get_image_width(img),
		   get_image_height(img),
		   0, 0, 255, BLEND_NORMAL);
}

/*
 * FIレイヤにスイッチの選択イメージを描画する
 */
void draw_news_fg_image(int x, int y)
{
	struct image *img;

	img = news_fg_image != NULL ? news_fg_image : switch_fg_image;

	draw_image(layer_image[LAYER_FI], x, y, img,
		   get_image_width(img),
		   get_image_height(img),
		   0, 0, 255, BLEND_NORMAL);
}

/*
 * FO/FIの2レイヤに文字を描画する前にロックする
 */
void lock_draw_char_on_fo_fi(void)
{
	lock_image(layer_image[LAYER_FO]);
	lock_image(layer_image[LAYER_FI]);
}

/*
 * FO/FIの2レイヤに文字を描画した後にアンロックする
 */
void unlock_draw_char_on_fo_fi(void)
{
	unlock_image(layer_image[LAYER_FI]);
	unlock_image(layer_image[LAYER_FO]);
}

/*
 * FO/FIの2レイヤに文字を描画する
 */
void draw_char_on_fo_fi(int x, int y, uint32_t wc, pixel_t fo_body_color,
			pixel_t fo_outline_color, pixel_t fi_body_color,
			pixel_t fi_outline_color, int *ret_w, int *ret_h)
{
	int w, h;

	draw_char_on_layer(LAYER_FO, x, y, wc, fo_body_color, fo_outline_color,
			   &w, &h, conf_font_size, false);

	draw_char_on_layer(LAYER_FI, x, y, wc, fi_body_color, fi_outline_color,
			   &w, &h, conf_font_size, false);

	*ret_w = w;
	*ret_h = h;
}

/*
 * FO/FIの2レイヤに画像を描画する
 */
void draw_image_on_fo_fi(int x, int y, struct image *img)
{
	draw_image(layer_image[LAYER_FO], x, y, img, get_image_width(img),
		   get_image_height(img), 0, 0, 255, BLEND_NONE);
		   
	draw_image(layer_image[LAYER_FI], x, y, img, get_image_width(img),
		   get_image_height(img), 0, 0, 255, BLEND_NONE);
}

/*
 * メニュー画面・CG回想画面の描画
 */

/*
 * FO/FIレイヤをロックする
 */
void lock_fo_fi_for_menu(void)
{
	lock_image(layer_image[LAYER_FO]);
	lock_image(layer_image[LAYER_FI]);
}

/*
 * FO/FIレイヤをアンロックする
 */
void unlock_fo_fi_for_menu(void)
{
	unlock_image(layer_image[LAYER_FI]);
	unlock_image(layer_image[LAYER_FO]);
}

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
 * FOレイヤの内容を仮のBGレイヤに設定する
 */
bool create_temporary_bg(void)
{
	struct image *img;

	/* 既存のBGレイヤのイメージを破棄する */
	destroy_layer_image(LAYER_BG);

	/* 背景のイメージを作成する */
	img = create_image(conf_window_width, conf_window_height);
	if (img == NULL)
		return false;

	/* FOレイヤの中身をコピーする */
	lock_image(img);
	draw_image(img, 0, 0, layer_image[LAYER_FO], conf_window_width,
		   conf_window_height, 0, 0, 255, BLEND_NONE);
	unlock_image(img);

	/* BGレイヤにセットする */
	layer_image[LAYER_BG] = img;

	return true;
}

/*
 * バナーの描画
 */

/*
 * オートモードバナーの矩形を取得する
 */
void get_automode_banner_rect(int *x, int *y, int *w, int *h)
{
	*x = layer_x[LAYER_AUTO];
	*y = layer_y[LAYER_AUTO];
	*w = get_image_width(layer_image[LAYER_AUTO]);
	*h = get_image_height(layer_image[LAYER_AUTO]);
}

/*
 * スキップモードバナーの矩形を取得する
 */
void get_skipmode_banner_rect(int *x, int *y, int *w, int *h)
{
	*x = layer_x[LAYER_SKIP];
	*y = layer_y[LAYER_SKIP];
	*w = get_image_width(layer_image[LAYER_SKIP]);
	*h = get_image_height(layer_image[LAYER_SKIP]);
}

/*
 * オートモードバナーの表示・非表示を設定する
 */
void show_automode_banner(bool show)
{
	is_auto_visible = show;
}

/*
 * スキップモードバナーの表示・非表示を設定する
 */
void show_skipmode_banner(bool show)
{
	is_skip_visible = show;
}

/*
 * 共通ルーチン
 */

/* レイヤをレンダリングする */
static void render_layer_image(int layer)
{
	assert(layer >= 0 && layer < STAGE_LAYERS);

	/* 背景イメージは必ずセットされている必要がある */
	if (layer == LAYER_BG)
		assert(layer_image[LAYER_BG] != NULL);

	/* イメージがセットされていなければ描画しない */
	if (layer_image[layer] == NULL)
		return;

	if (layer >= LAYER_CHB && layer <= LAYER_CHC &&
	    ch_dim[layer_to_pos(layer)]) {
		/* 暗く描画する */
		render_image_dim(layer_x[layer],
				 layer_y[layer],
				 layer_image[layer],
				 get_image_width(layer_image[layer]),
				 get_image_height(layer_image[layer]),
				 0,
				 0,
				 layer_alpha[layer],
				 layer_blend[layer]);
	} else {
		/* 普通に描画する */
		render_image(layer_x[layer],
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

/* レイヤを描画する */
static void draw_layer_image(struct image *target, int layer)
{
	assert(layer >= 0 && layer < STAGE_LAYERS);

	/* 背景イメージは必ずセットされている必要がある */
	if (layer == LAYER_BG)
		assert(layer_image[LAYER_BG] != NULL);

	/* イメージがセットされていなければ描画しない */
	if (layer_image[layer] == NULL)
		return;

	if (layer >= LAYER_CHB && layer <= LAYER_CHC &&
	    ch_dim[layer_to_pos(layer)]) {
		/* 暗く描画する */
		draw_image_dim(target,
			       layer_x[layer],
			       layer_y[layer],
			       layer_image[layer],
			       get_image_width(layer_image[layer]),
			       get_image_height(layer_image[layer]),
			       0,
			       0,
			       layer_alpha[layer],
			       layer_blend[layer]);
	} else {
		/* 普通に描画する */
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

/* レイヤの矩形をレンダリングする */
static void render_layer_image_rect(int layer, int x, int y, int w, int h)
{
	assert(layer >= 0 && layer < STAGE_LAYERS);

	/* 背景イメージは必ずセットされている必要がある */
	if (layer == LAYER_BG)
		assert(layer_image[LAYER_BG] != NULL);

	/* イメージがセットされていなければ描画しない */
	if (layer_image[layer] == NULL)
		return;

	if (layer >= LAYER_CHB && layer <= LAYER_CHC &&
	    ch_dim[layer_to_pos(layer)]) {
		/* 暗く描画する */
		render_image_dim(x,
				 y,
				 layer_image[layer],
				 w,
				 h,
				 x - layer_x[layer],
				 y - layer_y[layer],
				 layer_alpha[layer],
				 layer_blend[layer]);
	} else {
		/* 暗く描画する */
		render_image(x,
			     y,
			     layer_image[layer],
			     w,
			     h,
			     x - layer_x[layer],
			     y - layer_y[layer],
			     layer_alpha[layer],
			     layer_blend[layer]);
	}
}

#if 0
/* レイヤの矩形を描画する */
static void draw_layer_image_rect(struct image *target, int layer, int x,
				  int y, int w, int h)
{
	assert(layer >= 0 && layer < STAGE_LAYERS);

	/* 背景イメージは必ずセットされている必要がある */
	if (layer == LAYER_BG)
		assert(layer_image[LAYER_BG] != NULL);

	/* レイヤはイメージがセットされていれば描画する */
	if (layer_image[layer] != NULL) {
		draw_image(target, x, y, layer_image[layer], w, h,
			   x - layer_x[layer], y - layer_y[layer],
			   layer_alpha[layer], layer_blend[layer]);
	}
}
#endif

/* レイヤに文字を描画する */
static bool draw_char_on_layer(int layer, int x, int y, uint32_t wc,
			       pixel_t color, pixel_t outline_color, int *w,
			       int *h, int base_font_size, bool is_dim)
{
	/* 文字を描画する */
	if (!draw_glyph(layer_image[layer], x, y, color, outline_color, wc, w,
			h, base_font_size, is_dim)) {
		/* グリフがない、コードポイントがおかしい、など */
		return false;
	}

	return true;
}

/*
 * GUI
 */

/*
 * GUIの画像を削除する
 */
void remove_gui_images(void)
{
	if (gui_idle_image != NULL) {
		destroy_image(gui_idle_image);
		gui_idle_image = NULL;
	}
	if (gui_hover_image != NULL) {
		destroy_image(gui_hover_image);
		gui_hover_image = NULL;
	}
	if (gui_active_image != NULL) {
		destroy_image(gui_active_image);
		gui_active_image = NULL;
	}
}

/*
 * GUIのidle画像を読み込む
 */
bool load_gui_idle_image(const char *file)
{
	if (gui_idle_image != NULL) {
		destroy_image(gui_idle_image);
		gui_idle_image = NULL;
	}

	gui_idle_image = create_image_from_file(CG_DIR, file);
	if (gui_idle_image == NULL)
		return false;

	return true;
}

/*
 * GUIのhover画像を読み込む
 */
bool load_gui_hover_image(const char *file)
{
	if (gui_hover_image != NULL) {
		destroy_image(gui_hover_image);
		gui_hover_image = NULL;
	}

	gui_hover_image = create_image_from_file(CG_DIR, file);
	if (gui_hover_image == NULL)
		return false;

	return true;
}

/*
 * GUIのactive画像を読み込む
 */
bool load_gui_active_image(const char *file)
{
	if (gui_active_image != NULL) {
		destroy_image(gui_active_image);
		gui_active_image = NULL;
	}

	gui_active_image = create_image_from_file(CG_DIR, file);
	if (gui_active_image == NULL)
		return false;

	return true;
}

/*
 * GUIのイメージがすべて揃っているか調べる
 */
bool check_stage_gui_images(void)
{
	if (gui_idle_image == NULL || gui_hover_image == NULL ||
	    gui_active_image == NULL)
		return false;
	return true;
}

/*
 * GUIのidle画像を描画する
 */
void draw_stage_gui_idle(void)
{
	render_image(0, 0, gui_idle_image, conf_window_width,
		     conf_window_height, 0, 0, 255, BLEND_NONE);
}

/*
 * GUIのhover画像を描画する
 */
void draw_stage_gui_hover(int x, int y, int w, int h)
{
	render_image(x, y, gui_hover_image, w, h, x, y, 255, BLEND_FAST);
}

/*
 * GUIのactive画像を描画する
 */
void draw_stage_gui_active(int x, int y, int w, int h, int sx, int sy)
{
	render_image(x, y, gui_active_image, w, h, sx, sy, 255, BLEND_FAST);
}

/*
 * idle画像の内容を仮のBGレイヤに設定する
 */
bool create_temporary_bg_for_gui(void)
{
	struct image *img;

	/* 既存のBGレイヤのイメージを破棄する */
	destroy_layer_image(LAYER_BG);

	/* 背景のイメージを作成する */
	img = create_image(conf_window_width, conf_window_height);
	if (img == NULL)
		return false;

	/* idleの中身をコピーする */
	if (gui_idle_image != NULL) {
		lock_image(img);
		draw_image(img, 0, 0, gui_idle_image, conf_window_width,
			   conf_window_height, 0, 0, 255, BLEND_NONE);
		unlock_image(img);
	}

	/* BGレイヤにセットする */
	layer_image[LAYER_BG] = img;

	return true;
}

/*
 * 更新領域の計算
 */

/*
 * 2つの矩形を囲う矩形を求める
 */
void union_rect(int *x, int *y, int *w, int *h, int x1, int y1, int w1, int h1,
		int x2, int y2, int w2, int h2)
{
	int right1, right2, bottom1, bottom2;

	/* 幅、高さがゼロの場合 */
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

	/* 左端を求める */
	if (x1 < x2)
		*x = x1;
	else
		*x = x2;

	/* 上端を求める */
	if (y1 < y2)
		*y = y1;
	else
		*y = y2;

	/* 右端を求める */
	right1 = x1 + w1 - 1;
	right2 = x2 + w2 - 1;
	if (right1 > right2)
		*w = right1 - *x + 1;
	else
		*w = right2 - *x + 1;

	/* 下端を求める */
	bottom1 = y1 + h1 - 1;
	bottom2 = y2 + h2 - 1;
	if (bottom1 > bottom2)
		*h = bottom1 - *y + 1;
	else
		*h = bottom2 - *y + 1;
}

/*
 * メッセージボックスと名前ボックスを更新する
 */
bool update_msgbox_and_namebox(void)
{
	/* 名前ボックスをセットアップする */
	if (!setup_namebox())
		return false;

	/* メッセージボックスをセットアップする */
	if (!setup_msgbox())
		return false;

	return true;
}
