/* -*- coding: utf-8; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Suika2
 * Copyright (C) 2001-2023, Keiichi Tabata. All rights reserved.
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
 *  - 2023-08-20 アニメサブシステムの導入, @chaをアニメサブシステムへ移行
 *  - 2023-08-29 @chsxを追加
 *  - 2023-09-14 テキストレイヤ、エフェクトレイヤを追加
 *  - 2023-09-18 リファクタリング
 *  - 2023-12-16 スケーリング対応
 */

#include "suika.h"

/* false assertion */
#define BAD_POSITION		(0)
#define INVALID_FADE_METHOD	(0)

/* カーテンフェードのカーテンの幅 */
#define CURTAIN_WIDTH	(256)

/*
 * ステージの動作モード
 */

static int stage_mode;

enum stage_mode {
	/* 通常: render_stage()を呼出可能 */
	STAGE_MODE_IDLE,

	/* フェード中: render_stage()でなくrender_fade()を呼出可能 */
	STAGE_MODE_BG_FADE,
	STAGE_MODE_CH_FADE,
	STAGE_MODE_CHS_FADE,
	STAGE_MODE_SHAKE_FADE,
};

/*
 * ステージのイメージ
 */

/* レイヤのイメージ */
static struct image *layer_image[STAGE_LAYERS];

/* FOイメージ */
static struct image *fo_image;

/* FIイメージ */
static struct image *fi_image;

/* メッセージボックスの背景イメージ */
static struct image *msgbox_bg_image;

/* メッセージボックスの前景イメージ */
static struct image *msgbox_fg_image;

/* 名前ボックスのイメージ */
static struct image *namebox_image;

/* クリックアニメーションのイメージ */
static struct image *click_image[CLICK_FRAMES];

/* 選択肢(非選択時)のイメージ */
static struct image *switch_bg_image[8];

/* 選択肢(選択時)のイメージ */
static struct image *switch_fg_image[8];

/* NEWS(非選択)のイメージ */
static struct image *news_bg_image;

/* NEWS(選択)のイメージ */
static struct image *news_fg_image;

/* 折りたたみシステムメニュー(非ポイント時)のイメージ */
static struct image *sysmenu_collapsed_idle_image;

/* 折りたたみシステムメニュー(ポイント時)のイメージ */
static struct image *sysmenu_collapsed_hover_image;

/* システムメニュー(ベース部分と非選択項目)のイメージ */
static struct image *sysmenu_idle_image;

/* システムメニュー(選択項目)のイメージ */
static struct image *sysmenu_hover_image;

/* システムメニュー(禁止項目)のイメージ */
static struct image *sysmenu_disable_image;

/* セーブストッロのNEW画像のイメージ */
static struct image *savenew_image;

/* セーブデータ用のサムネイルイメージ */
static struct image *thumb_image;

/* ルールイメージ */
static struct image *fade_rule_img;

/* キラキラエフェクト */
static struct image *kirakira_image[KIRAKIRA_FRAME_COUNT];

/*
 * レイヤの可視状態
 */

/* メッセージボックスを表示するか */
static bool is_msgbox_visible;

/* 名前ボックスを表示するか */
static bool is_namebox_visible;

/* クリックアニメーションを表示するか */
static bool is_click_visible;

/* オートモードバナーを表示するか */
static bool is_auto_visible;

/* スキップモードバナーを表示するか */
static bool is_skip_visible;

/*
 * レイヤの表示位置、アルファ値、ブレンドタイプ、ファイル名
 *  - 現状、ブレンドタイプは背景だとコピー、その他はFAST
 *  - 必要に応じて加算や減算のブレンドタイプに対応する
 *  - TODO: scale, rotateを追加する
 */

/* レイヤのX座標 */
static int layer_x[STAGE_LAYERS];

/* レイヤのY座標 */
static int layer_y[STAGE_LAYERS];

/* レイヤのアルファ値 */
static int layer_alpha[STAGE_LAYERS];

/* レイヤのブレンドモード */
static int layer_blend[STAGE_LAYERS];

/* レイヤのXスケール */
static float layer_scale_x[STAGE_LAYERS];

/* レイヤのYスケール */
static float layer_scale_y[STAGE_LAYERS];

/* レイヤの中心X座標 */
static float layer_center_x[STAGE_LAYERS];

/* レイヤの中心Y座標 */
static float layer_center_y[STAGE_LAYERS];

/* レイヤの回転(rad) */
static float layer_rotate[STAGE_LAYERS];

/* ファイル名(FI/FOを除く) */
static char *layer_file_name[STAGE_LAYERS];

/* 目/口のレイヤのフレーム番号 */
static int layer_frame[STAGE_LAYERS];

/*
 * 発話中のキャラ以外を暗くするためのフラグ
 */

/* キャラを暗くするか */
static bool ch_dim[CH_BASIC_LAYERS];

/* キャラの名前インデックス */
static int ch_name_mapping[CH_BASIC_LAYERS] = {-1, -1, -1, -1, -1, -1};

/* 発話中のキャラ */
static int ch_talking = -1;

/*
 * テキストレイヤ
 */

static char *layer_text[STAGE_LAYERS];

/*
 * フェードモード
 *  - bg, ch, chs, chsxはFOとFIの2レイヤの描画で行っている
 *  - フェード前(FO)とフェード後(FI)のレイヤを作成して、2レイヤだけ描画する
 *  - 描画最適化のため、全レイヤを毎フレーム描画することを避けている
 */

/* フェードメソッド */
static int fade_method;

/* FI/FOフェードの進捗 */
static float fi_fo_fade_progress;

/* shake用の画面表示オフセット */
static int shake_offset_x;
static int shake_offset_y;

/*
 * キラキラエフェクト
 */

/* 表示位置 */
static int kirakira_x;
static int kirakira_y;

/* 開始時刻 */
static uint64_t sw_kirakira;

/*
 * 前方参照
 */
static bool setup_namebox(void);
static bool setup_msgbox(bool no_bg, bool no_fg);
static bool setup_click(void);
static bool setup_switch(bool no_bg, bool no_fg, int index);
static bool setup_news(void);
static bool setup_sysmenu(void);
static bool setup_banners(void);
static bool setup_kirakira(void);
static bool setup_savenew(void);
static bool setup_thumb(void);
static void restore_text_layers(void);
static bool create_fade_layer_images(void);
static void destroy_layer_image(int layer);
static void draw_fo_common(void);
static void draw_fi_common(bool show_msgbox);
static void render_fade_normal(void);
static void render_fade_rule(void);
static void render_fade_melt(void);
static void render_fade_curtain_right(void);
static void render_fade_curtain_left(void);
static void render_fade_curtain_up(void);
static void render_fade_curtain_down(void);
static void render_fade_slide_right(void);
static void render_fade_slide_left(void);
static void render_fade_slide_up(void);
static void render_fade_slide_down(void);
static void render_fade_shutter_right(void);
static void render_fade_shutter_left(void);
static void render_fade_shutter_up(void);
static void render_fade_shutter_down(void);
static void render_fade_clockwise(int method);
static void render_fade_counterclockwise(int method);
static float cw_step(int method, float progress);
static void render_fade_eye_open(void);
static void render_fade_eye_close(void);
static void render_fade_eye_open_v(void);
static void render_fade_eye_close_v(void);
static void render_fade_slit_open(void);
static void render_fade_slit_close(void);
static void render_fade_slit_open_v(void);
static void render_fade_slit_close_v(void);
static void render_fade_shake(void);
static void render_layer_image(int layer);
static void draw_layer_image(struct image *target, int layer);

/*
 * 初期化
 */

/*
 * ステージの初期化処理をする
 */
bool init_stage(void)
{
	int i;

#ifdef SUIKA_TARGET_ANDROID
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

	/* 初期値を設定する */
	for (i = 0; i < STAGE_LAYERS; i++) {
		layer_scale_x[i] = 1.0f;
		layer_scale_y[i] = 1.0f;
		layer_alpha[i] = 255;
	}

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
	if (!setup_msgbox(false, false))
		return false;

	/* クリックアニメーションをセットアップする */
	if (!setup_click())
		return false;

	/* スイッチをセットアップする */
	if (!setup_switch(false, false, -1))
		return false;

	/* NEWSをセットアップする */
	if (!setup_news())
		return false;

	/* システムメニューをセットアップする */
	if (!setup_sysmenu())
		return false;

	/* バナーをセットアップする */
	if (!setup_banners())
		return false;

	/* キラキラ画像をセットアップする */
	if (!setup_kirakira())
		return false;

	/* セーブスロットのNEW画像をセットアップする */
	if (!setup_savenew())
		return false;

	/* テキストレイヤの文字を復元する */
	restore_text_layers();

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
	layer_image[LAYER_NAME] = create_image(namebox_image->width, namebox_image->height);
	if (layer_image[LAYER_NAME] == NULL)
		return false;

	/* 名前ボックスレイヤの配置を行う */
	layer_x[LAYER_NAME] = conf_namebox_x;
	layer_y[LAYER_NAME] = conf_namebox_y;

	/* 内容を転送する */
	fill_namebox();

	return true;
}

/* メッセージボックスをセットアップする */
static bool setup_msgbox(bool no_bg, bool no_fg)
{
	is_msgbox_visible = false;

	/* 再初期化時に破棄する */
	if (!no_bg) {
		if (msgbox_bg_image != NULL) {
			destroy_image(msgbox_bg_image);
			msgbox_bg_image = NULL;
		}
	}
	if (!no_fg) {
		if (msgbox_fg_image != NULL) {
			destroy_image(msgbox_fg_image);
			msgbox_fg_image = NULL;
		}
	}
	if (layer_image[LAYER_MSG] != NULL) {
		destroy_image(layer_image[LAYER_MSG]);
		layer_image[LAYER_MSG] = NULL;
	}

	/* メッセージボックスの背景画像を読み込む */
	if (!no_bg) {
		msgbox_bg_image = create_image_from_file(CG_DIR, conf_msgbox_bg_file);
		if (msgbox_bg_image == NULL)
			return false;
	}

	/* メッセージボックスの前景画像を読み込む */
	if (!no_fg) {
		msgbox_fg_image = create_image_from_file(CG_DIR, conf_msgbox_fg_file);
		if (msgbox_fg_image == NULL)
			return false;
	}

	/* メッセージボックスの前景と背景が同じサイズであることを確認する */
#if 0
	if (msgbox_bg_image->width != msgbox_fg_image->width ||
	    msgbox_bg_image->height != msgbox_fg_image->height) {
		log_invalid_msgbox_size();
		return false;
	}
#endif

	/* メッセージボックスのレイヤのイメージを作成する */
	layer_image[LAYER_MSG] = create_image(msgbox_bg_image->width, msgbox_bg_image->height);
	if (layer_image[LAYER_MSG] == NULL)
		return false;

	/* メッセージボックスレイヤの配置を行う */
	layer_x[LAYER_MSG] = conf_msgbox_x;
	layer_y[LAYER_MSG] = conf_msgbox_y;

	/* 内容を転送する */
	fill_msgbox();

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
			clear_image_color(click_image[i], make_pixel(0, 0, 0, 0));
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
static bool setup_switch(bool no_bg, bool no_fg, int index)
{
	int i;

	/* 再初期化時に破棄する */
	for (i = 0; i < 8; i ++) {
		if (index == -1 || index == 0) {
			if (!no_bg) {
				if (switch_bg_image[i] != NULL) {
					destroy_image(switch_bg_image[i]);
					switch_bg_image[i] = NULL;
				}
			}
			if (!no_fg) {
				if (switch_fg_image[i] != NULL) {
					destroy_image(switch_fg_image[i]);
					switch_fg_image[i] = NULL;
				}
			}
		}
	}

	for (i = 0; i < 8; i ++) {
		if (i != 0 &&
		    (conf_switch_bg_file[i] == NULL || 
		     conf_switch_fg_file[i] == NULL))
			continue;

		assert(conf_switch_bg_file[i] != NULL);
		assert(conf_switch_bg_file[i] != NULL);

		/* スイッチの非選択イメージを読み込む */
		if (!no_bg) {
			switch_bg_image[i] = create_image_from_file(CG_DIR, conf_switch_bg_file[i]);
			if (switch_bg_image[i] == NULL)
				return false;
		}

		/* スイッチの選択イメージを読み込む */
		if (!no_fg) {
			switch_fg_image[i] = create_image_from_file(CG_DIR, conf_switch_fg_file[i]);
			if (switch_fg_image[i] == NULL)
				return false;
		}
	}

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
static bool setup_banners(void)
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

/* キラキラ画像をセットアップする */
static bool setup_kirakira(void)
{
	int i;

	/* 再初期化時に破棄する */
	for (i = 0; i < KIRAKIRA_FRAME_COUNT; i++) {
		if (kirakira_image[i] != NULL) {
			destroy_image(kirakira_image[i]);
			kirakira_image[i] = NULL;
		}
	}

	/* キラキラ画像を読み込む */
	for (i = 0; i < KIRAKIRA_FRAME_COUNT; i++) {
		if (conf_kirakira_file[i] == NULL)
			continue;
		kirakira_image[i] =
			create_image_from_file(CG_DIR, conf_kirakira_file[i]);
		if (kirakira_image[i] == NULL)
			return false;
	}

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

/* セーブスロットのNEW画像をセットアップする */
static bool setup_savenew(void)
{
	/* 再初期化時に破棄する */
	if (savenew_image != NULL) {
		destroy_image(savenew_image);
		savenew_image = NULL;
	}

	/* コンフィグが指定されていない場合 */
	if (conf_save_data_new == NULL)
		return true;

	/* イメージを作成する */
	savenew_image = create_image_from_file(CG_DIR, conf_save_data_new);
	if (savenew_image == NULL)
		return false;

	return true;
}

/* テキストレイヤーの文字を復元する */
static void restore_text_layers(void)
{
	struct draw_msg_context context;
	pixel_t color, outline_color;
	int i, total_chars;

	/* デフォルト色をロードする */
	color = make_pixel(0xff,
			   (pixel_t)conf_font_color_r,
			   (pixel_t)conf_font_color_g,
			   (pixel_t)conf_font_color_b);
	outline_color = make_pixel(0xff,
				   (pixel_t)conf_font_outline_color_r,
				   (pixel_t)conf_font_outline_color_g,
				   (pixel_t)conf_font_outline_color_b);

	for (i = LAYER_TEXT1; i <= LAYER_TEXT8; i++) {
		if (layer_text[i] == NULL)
			continue;

		/* 描画する */
		construct_draw_msg_context(
			&context,
			i,
			layer_text[i],
			conf_font_select,
			conf_font_size,
			conf_font_size,
			conf_font_ruby_size,
			!conf_font_outline_remove,
			2 + conf_font_outline_add,
			0,	/* pen_x */
			0,	/* pen_y */
			layer_image[i]->width,
			layer_image[i]->height,
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
		notify_image_update(layer_image[i]);
	}
}

/* レイヤのイメージを作成する */
static bool create_fade_layer_images(void)
{
	/* 再初期化時に破棄する */
	if (fo_image != NULL)
		destroy_image(fo_image);
	if (fi_image != NULL)
		destroy_image(fi_image);

	/* フェードアウトのレイヤのイメージを作成する */
	fo_image = create_image(conf_window_width, conf_window_height);
	if (fo_image == NULL)
		return false;

	/* フェードインのレイヤのイメージを作成する */
	fi_image = create_image(conf_window_width, conf_window_height);
	if (fi_image == NULL)
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
	if (conf_window_white)
		clear_image_white(img);
	else
		clear_image_black(img);

	return img;
}

/*
 * メッセージボックスを更新する
 */
bool update_msgbox(bool is_fg)
{
	/* メッセージボックスをセットアップする */
	if (!is_fg) {
		if (!setup_msgbox(false, true))
			return false;
	} else {
		if (!setup_msgbox(true, false))
			return false;
	}

	return true;
}

/*
 * 名前ボックスを更新する
 */
bool update_namebox(void)
{
	/* 名前ボックスをセットアップする */
	if (!setup_namebox())
		return false;

	return true;
}

/*
 * 選択肢ボックスを更新する
 */
bool update_switchbox(bool is_fg, int index)
{
	if (!is_fg) {
		if (!setup_switch(false, true, index))
			return false;
	} else {
		if (!setup_switch(true, false, index))
			return false;
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
	for (i = 0; i < CLICK_FRAMES; i++) {
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
	for (i = 0; i < 8; i++) {
		if (switch_bg_image[i] != NULL) {
			destroy_image(switch_bg_image[i]);
			switch_bg_image[i] = NULL;
		}
		if (switch_fg_image[i] != NULL) {
			destroy_image(switch_fg_image[i]);
			switch_fg_image[i] = NULL;
		}
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
	for (i = 0; i < STAGE_LAYERS; i++) {
		if (layer_file_name[i] != NULL) {
			free(layer_file_name[i]);
			layer_file_name[i] = NULL;
		}
	}
	for (i = 0; i < STAGE_LAYERS; i++) {
		if (layer_file_name[i] != NULL) {
			free(layer_file_name[i]);
			layer_file_name[i] = NULL;
		}
	}
	if (fo_image != NULL) {
		destroy_image(fo_image);
		fo_image = NULL;
	}
	if (fi_image != NULL) {
		destroy_image(fi_image);
		fi_image = NULL;
	}
	if (fade_rule_img != NULL) {
		destroy_image(fade_rule_img);
		fade_rule_img = NULL;
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
 * Basic Functionality
 */

/*
 * Gets a layer x position.
 */
int get_layer_x(int layer)
{
	assert(layer >= 0 && layer < STAGE_LAYERS);
	return layer_x[layer];
}

/*
 * Gets a layer y position.
 */
int get_layer_y(int layer)
{
	assert(layer >= 0 && layer < STAGE_LAYERS);
	return layer_y[layer];
}

/*
 * Sets a layer position.
 */
void set_layer_position(int layer, int x, int y)
{
	assert(layer >= 0 && layer < STAGE_LAYERS);

	layer_x[layer] = x;
	layer_y[layer] = y;

	switch (layer) {
	case LAYER_CLICK: return;
	case LAYER_AUTO: return;
	case LAYER_SKIP: return;
	default: break;
	}
}

/*
 * Updates layer positions by config.
 */
void update_layer_position_by_config(void)
{
	layer_x[LAYER_MSG] = conf_msgbox_x;
	layer_y[LAYER_MSG] = conf_msgbox_y;

	layer_x[LAYER_NAME] = conf_namebox_x;
	layer_y[LAYER_NAME] = conf_namebox_y;

	layer_x[LAYER_NAME] = conf_namebox_x;
	layer_y[LAYER_NAME] = conf_namebox_y;

	layer_x[LAYER_CLICK] = conf_click_x;
	layer_y[LAYER_CLICK] = conf_click_y;

	layer_x[LAYER_AUTO] = conf_automode_banner_x;
	layer_y[LAYER_AUTO] = conf_automode_banner_y;

	layer_x[LAYER_SKIP] = conf_skipmode_banner_x;
	layer_y[LAYER_SKIP] = conf_skipmode_banner_y;
}

/*
 * Sets a layer scale.
 */
void set_layer_scale(int layer, float scale_x, float scale_y)
{
	assert(layer >= 0 && layer < STAGE_LAYERS);

	if (scale_x == 0)
		log_info("warning: scale_x = 0");
	if (scale_y == 0)
		log_info("warning: scale_y = 0");

	layer_scale_x[layer] = scale_x;
	layer_scale_y[layer] = scale_y;
}

/*
 * Gets a layer image width.
 */
int get_layer_width(int layer)
{
	assert(layer >= 0 && layer < STAGE_LAYERS);
	assert(layer_image[layer] != NULL);
	return layer_image[layer]->width;
}

/*
 * Gets a layer image height.
 */
int get_layer_height(int layer)
{
	assert(layer >= 0 && layer < STAGE_LAYERS);
	assert(layer_image[layer] != NULL);
	return layer_image[layer]->height;
}

/*
 * Gets a layer alpha.
 */
int get_layer_alpha(int layer)
{
	assert(layer >= 0 && layer < STAGE_LAYERS);
	return layer_alpha[layer];
}

/*
 * Sets a layer alpha.
 */
void set_layer_alpha(int layer, int alpha)
{
	assert(layer >= 0 && layer < STAGE_LAYERS);
	layer_alpha[layer] = alpha;
}

/*
 * Sets a layer belnd mode.
 */
void set_layer_blend(int layer, int blend)
{
	assert(layer >= 0 && layer < STAGE_LAYERS);
	layer_blend[layer] = blend;
}

/*
 * Sets a layer center coordinate.
 */
void set_layer_center(int layer, int x, int y)
{
	assert(layer >= 0 && layer < STAGE_LAYERS);
	layer_center_x[layer] = (float)x;
	layer_center_y[layer] = (float)y;
}

/*
 * Sets a layer rotation.
 */
void set_layer_rotate(int layer, float rad)
{
	assert(layer >= 0 && layer < STAGE_LAYERS);
	layer_rotate[layer] = rad;
}

/*
 * Gets a layer file name.
 */
const char *get_layer_file_name(int layer)
{
	assert(layer >= 0 && layer < STAGE_LAYERS);
	return layer_file_name[layer];
}

/*
 * Sets a layer file name.
 */
bool set_layer_file_name(int layer, const char *file_name)
{
	assert(layer >= 0 && layer < STAGE_LAYERS);
	assert(layer != LAYER_CLICK);
	assert(layer != LAYER_MSG);
	assert(layer != LAYER_NAME);
	assert(layer != LAYER_AUTO);
	assert(layer != LAYER_SKIP);

	if (layer_file_name[layer] != NULL) {
		free(layer_file_name[layer]);
		layer_file_name[layer] = NULL;
	}
	if (file_name != NULL) {
		layer_file_name[layer] = strdup(file_name);
		if (layer_file_name[layer] == NULL) {
			log_memory();
			return false;
		}
	}
	return true;
}

/*
 * Gets a layer image.
 */
struct image *get_layer_image(int layer)
{
	assert(layer >= 0 && layer < STAGE_LAYERS);
	return layer_image[layer];
}

/*
 * Sets a layer image.
 */
void set_layer_image(int layer, struct image *img)
{
	assert(layer >= 0 && layer < STAGE_LAYERS);
	assert(layer != LAYER_CLICK);
	assert(layer != LAYER_MSG);
	assert(layer != LAYER_NAME);
	assert(layer != LAYER_AUTO);
	assert(layer != LAYER_SKIP);

	destroy_layer_image(layer);

	layer_image[layer] = img;
}

/*
 * Sets a layer frame for eye blinking and lip synchronizing.
 */
void set_layer_frame(int layer, int frame)
{
	layer_frame[layer] = frame;
}

/*
 * Clear basic layers.
 */
void clear_stage_basic(void)
{
	int i;

	for (i = 0; i < STAGE_LAYERS; i++) {
		if (i == LAYER_BG)
			continue;
		if (i == LAYER_MSG)
			continue;
		if (i == LAYER_NAME)
			continue;
		if (i == LAYER_CLICK)
			continue;
		if (i == LAYER_AUTO)
			continue;
		if (i == LAYER_SKIP)
			continue;

		set_layer_file_name(i, NULL);
		set_layer_image(i, NULL);
	}

	for (i = LAYER_TEXT1; i <= LAYER_TEXT8; i++) {
		if (layer_text[i] != NULL) {
			free(layer_text[i]);
			layer_text[i] = NULL;
		}
	}}

/*
 * Clear the stage and make it initial state.
 */
void clear_stage(void)
{
	int i;

	for (i = 0; i < STAGE_LAYERS; i++) {
		switch (i) {
		case LAYER_BG:
			set_layer_file_name(i, NULL);
			set_layer_image(i, create_initial_bg());
			set_layer_position(i, 0, 0);
			set_layer_alpha(i, 255);
			break;
		case LAYER_MSG:
			set_layer_position(i, conf_msgbox_x, conf_msgbox_y);
			set_layer_alpha(i, 255);
			show_msgbox(false);
			break;
		case LAYER_NAME:
			set_layer_position(i, conf_namebox_x, conf_namebox_y);
			set_layer_alpha(i, 255);
			show_namebox(false);
			break;
		case LAYER_BG2:		/* fall-thru */
		case LAYER_CHB:		/* fall-thru */
		case LAYER_CHB_EYE:	/* fall-thru */
		case LAYER_CHL:		/* fall-thru */
		case LAYER_CHL_EYE:	/* fall-thru */
		case LAYER_CHLC:	/* fall-thru */
		case LAYER_CHLC_EYE:	/* fall-thru */
		case LAYER_CHR:		/* fall-thru */
		case LAYER_CHR_EYE:	/* fall-thru */
		case LAYER_CHRC:	/* fall-thru */
		case LAYER_CHRC_EYE:	/* fall-thru */
		case LAYER_CHC:		/* fall-thru */
		case LAYER_CHC_EYE:	/* fall-thru */
		case LAYER_CHF:		/* fall-thru */
		case LAYER_CHF_EYE:	/* fall-thru */
		case LAYER_EFFECT1:	/* fall-thru */
		case LAYER_EFFECT2:	/* fall-thru */
		case LAYER_EFFECT3:	/* fall-thru */
		case LAYER_EFFECT4:	/* fall-thru */
		case LAYER_EFFECT5:	/* fall-thru */
		case LAYER_EFFECT6:	/* fall-thru */
		case LAYER_EFFECT7:	/* fall-thru */
		case LAYER_EFFECT8:
			set_layer_file_name(i, NULL);
			set_layer_image(i, NULL);
			set_layer_position(i, 0, 0);
			set_layer_alpha(i, 255);
			break;
		case LAYER_CLICK:
			set_layer_position(i, conf_click_x, conf_click_y);
			set_layer_alpha(i, 255);
			show_click(false);
			break;
		case LAYER_AUTO:
			set_layer_position(i, conf_automode_banner_x, conf_automode_banner_y);
			set_layer_alpha(i, 255);
			show_automode_banner(false);
			break;
		case LAYER_SKIP:
			set_layer_position(i, conf_skipmode_banner_x, conf_skipmode_banner_y);
			set_layer_alpha(i, 255);
			show_skipmode_banner(false);
			break;
		case LAYER_TEXT1:	/* fall-thru */
		case LAYER_TEXT2:	/* fall-thru */
		case LAYER_TEXT3:	/* fall-thru */
		case LAYER_TEXT4:	/* fall-thru */
		case LAYER_TEXT5:	/* fall-thru */
		case LAYER_TEXT6:	/* fall-thru */
		case LAYER_TEXT7:	/* fall-thru */
		case LAYER_TEXT8:
			set_layer_file_name(i, NULL);
			set_layer_image(i, NULL);
			set_layer_position(i, 0, 0);
			set_layer_alpha(i, 255);
			set_layer_text(i, NULL);
			break;
		default:
			assert(0);
			break;
		}
	}
}

/*
 * Conversion of Layer Index and Character Position
 */

/*
 * Converts a character position to a stage layer index.
 */
int chpos_to_layer(int chpos)
{
	switch (chpos) {
	case CH_BACK:
		return LAYER_CHB;
	case CH_LEFT:
		return LAYER_CHL;
	case CH_LEFT_CENTER:
		return LAYER_CHLC;
	case CH_RIGHT:
		return LAYER_CHR;
	case CH_RIGHT_CENTER:
		return LAYER_CHRC;
	case CH_CENTER:
		return LAYER_CHC;
	case CH_FACE:
		return LAYER_CHF;
	default:
		assert(0);
		break;
	}
	return -1;
}

/*
 * Converts a character position to a stage layer index (character eye).
 */
int chpos_to_eye_layer(int chpos)
{
	switch (chpos) {
	case CH_BACK:
		return LAYER_CHB_EYE;
	case CH_LEFT:
		return LAYER_CHL_EYE;
	case CH_LEFT_CENTER:
		return LAYER_CHLC_EYE;
	case CH_RIGHT:
		return LAYER_CHR_EYE;
	case CH_RIGHT_CENTER:
		return LAYER_CHRC_EYE;
	case CH_CENTER:
		return LAYER_CHC_EYE;
	case CH_FACE:
		return LAYER_CHF_EYE;
	default:
		assert(0);
		break;
	}
	return -1;
}

/*
 * Converts a stage layer index to a character position.
 */
int layer_to_chpos(int layer)
{
	assert(layer == LAYER_CHB || layer == LAYER_CHB_EYE ||
	       layer == LAYER_CHL || layer == LAYER_CHL_EYE ||
	       layer == LAYER_CHR || layer == LAYER_CHR_EYE ||
	       layer == LAYER_CHC || layer == LAYER_CHC_EYE ||
	       layer == LAYER_CHRC || layer == LAYER_CHRC_EYE ||
	       layer == LAYER_CHLC || layer == LAYER_CHLC_EYE ||
	       layer == LAYER_CHF || layer == LAYER_CHF_EYE);

	switch (layer) {
	case LAYER_CHB:
	case LAYER_CHB_EYE:
		return CH_BACK;
	case LAYER_CHL:
	case LAYER_CHL_EYE:
		return CH_LEFT;
	case LAYER_CHLC:
	case LAYER_CHLC_EYE:
		return CH_LEFT_CENTER;
	case LAYER_CHR:
	case LAYER_CHR_EYE:
		return CH_RIGHT;
	case LAYER_CHRC:
	case LAYER_CHRC_EYE:
		return CH_RIGHT_CENTER;
	case LAYER_CHC:
	case LAYER_CHC_EYE:
		return CH_CENTER;
	case LAYER_CHF:
	case LAYER_CHF_EYE:
		return CH_FACE;
	default:
		assert(0);
		break;
	}
	return -1;
}

/*
 * Stage rendering to the screen.
 */

/*
 * Renders the stage with all stage layers.
 */
void render_stage(void)
{
	/* We cannot use render_stage() in an FO/FI fading. */
	assert(stage_mode != STAGE_MODE_BG_FADE);
	assert(stage_mode != STAGE_MODE_CH_FADE);
	assert(stage_mode != STAGE_MODE_CHS_FADE);
	assert(stage_mode != STAGE_MODE_SHAKE_FADE);

	/* Update an anime frame. */
	update_anime_frame();

	/* Render stage layers. */
	render_layer_image(LAYER_BG);
	render_layer_image(LAYER_BG2);
	render_layer_image(LAYER_EFFECT5);
	render_layer_image(LAYER_EFFECT6);
	render_layer_image(LAYER_EFFECT7);
	render_layer_image(LAYER_EFFECT8);
	render_layer_image(LAYER_CHB);
	render_layer_image(LAYER_CHB_EYE);
	render_layer_image(LAYER_CHL);
	render_layer_image(LAYER_CHL_EYE);
	render_layer_image(LAYER_CHLC);
	render_layer_image(LAYER_CHLC_EYE);
	render_layer_image(LAYER_CHR);
	render_layer_image(LAYER_CHR_EYE);
	render_layer_image(LAYER_CHRC);
	render_layer_image(LAYER_CHRC_EYE);
	render_layer_image(LAYER_CHC);
	render_layer_image(LAYER_CHC_EYE);
	render_layer_image(LAYER_EFFECT1);
	render_layer_image(LAYER_EFFECT2);
	render_layer_image(LAYER_EFFECT3);
	render_layer_image(LAYER_EFFECT4);
	if (is_msgbox_visible)
		render_layer_image(LAYER_MSG);
	if (is_namebox_visible && !conf_namebox_hidden)
		render_layer_image(LAYER_NAME);
	if (is_msgbox_visible)
		render_layer_image(LAYER_CHF);
	if (is_click_visible)
		render_layer_image(LAYER_CLICK);
	if (is_auto_visible)
		render_layer_image(LAYER_AUTO);
	if (is_skip_visible)
		render_layer_image(LAYER_SKIP);
	render_layer_image(LAYER_TEXT1);
	render_layer_image(LAYER_TEXT2);
	render_layer_image(LAYER_TEXT3);
	render_layer_image(LAYER_TEXT4);
	render_layer_image(LAYER_TEXT5);
	render_layer_image(LAYER_TEXT6);
	render_layer_image(LAYER_TEXT7);
	render_layer_image(LAYER_TEXT8);
}

/*
 * System Menu Rendering
 */

/*
 * システムメニューを描画する
 */
void render_sysmenu(bool is_auto_enabled,
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
		    bool is_custom2_selected)
{
	/* システムメニューの背景を描画する */
	render_image_normal(conf_sysmenu_x,
			    conf_sysmenu_y,
			    sysmenu_idle_image->width,
			    sysmenu_idle_image->height,
			    sysmenu_idle_image,
			    0,
			    0,
			    sysmenu_idle_image->width,
			    sysmenu_idle_image->height,
			    255);

	/* 禁止になっている項目を描画する */
	if (!is_auto_enabled) {
		/* オートの項目(禁止)を描画する */
		render_image_normal(conf_sysmenu_x + conf_sysmenu_auto_x,
				    conf_sysmenu_y + conf_sysmenu_auto_y,
				    conf_sysmenu_auto_width,
				    conf_sysmenu_auto_height,
				    sysmenu_disable_image,
				    conf_sysmenu_auto_x,
				    conf_sysmenu_auto_y,
				    conf_sysmenu_auto_width,
				    conf_sysmenu_auto_height,
				    255);
	}
	if (!is_skip_enabled) {
		/* スキップの項目(禁止)を描画する */
		render_image_normal(conf_sysmenu_x + conf_sysmenu_skip_x,
				    conf_sysmenu_y + conf_sysmenu_skip_y,
				    conf_sysmenu_skip_width,
				    conf_sysmenu_skip_height,
				    sysmenu_disable_image,
				    conf_sysmenu_skip_x,
				    conf_sysmenu_skip_y,
				    conf_sysmenu_skip_width,
				    conf_sysmenu_skip_height,
				    255);
	}
	if (!is_save_load_enabled) {
		/* クイックセーブの項目(禁止)を描画する */
		render_image_normal(conf_sysmenu_x + conf_sysmenu_qsave_x,
				    conf_sysmenu_y + conf_sysmenu_qsave_y,
				    conf_sysmenu_qsave_x,
				    conf_sysmenu_qsave_y,
				    sysmenu_disable_image,
				    conf_sysmenu_qsave_x,
				    conf_sysmenu_qsave_y,
				    conf_sysmenu_qsave_width,
				    conf_sysmenu_qsave_height,
				    255);

		/* クイックロードの項目(禁止)を描画する */
		render_image_normal(conf_sysmenu_x + conf_sysmenu_qload_x,
				    conf_sysmenu_y + conf_sysmenu_qload_y,
				    conf_sysmenu_qload_width,
				    conf_sysmenu_qload_height,
				    sysmenu_disable_image,
				    conf_sysmenu_qload_x,
				    conf_sysmenu_qload_y,
				    conf_sysmenu_qload_width,
				    conf_sysmenu_qload_height,
				    255);

		/* セーブの項目(禁止)を描画する */
		render_image_normal(conf_sysmenu_x + conf_sysmenu_save_x,
				    conf_sysmenu_y + conf_sysmenu_save_y,
				    conf_sysmenu_save_width,
				    conf_sysmenu_save_height,
				    sysmenu_disable_image,
				    conf_sysmenu_save_x,
				    conf_sysmenu_save_y,
				    conf_sysmenu_save_width,
				    conf_sysmenu_save_height,
				    255);

		/* ロードの項目(禁止)を描画する */
		render_image_normal(conf_sysmenu_x + conf_sysmenu_load_x,
				    conf_sysmenu_y + conf_sysmenu_load_y,
				    conf_sysmenu_load_width,
				    conf_sysmenu_load_height,
				    sysmenu_disable_image,
				    conf_sysmenu_load_x,
				    conf_sysmenu_load_y,
				    conf_sysmenu_load_width,
				    conf_sysmenu_load_height,
				    255);
	}
	if (is_save_load_enabled && !is_qload_enabled) {
		/* クイックロードの項目(禁止)を描画する */
		render_image_normal(conf_sysmenu_x + conf_sysmenu_qload_x,
				    conf_sysmenu_y + conf_sysmenu_qload_y,
				    conf_sysmenu_qload_width,
				    conf_sysmenu_qload_height,
				    sysmenu_disable_image,
				    conf_sysmenu_qload_x,
				    conf_sysmenu_qload_y,
				    conf_sysmenu_qload_width,
				    conf_sysmenu_qload_height,
				    255);
	}

	/* 選択されている項目を描画する */
	if (is_qsave_selected) {
		/* クイックセーブの項目(選択)を描画する */
		render_image_normal(conf_sysmenu_x + conf_sysmenu_qsave_x,
				    conf_sysmenu_y + conf_sysmenu_qsave_y,
				    conf_sysmenu_qsave_width,
				    conf_sysmenu_qsave_height,
				    sysmenu_hover_image,
				    conf_sysmenu_qsave_x,
				    conf_sysmenu_qsave_y,
				    conf_sysmenu_qsave_width,
				    conf_sysmenu_qsave_height,
				    255);
	}
	if (is_qload_selected) {
		/* クイックロードの項目(選択)を描画する */
		render_image_normal(conf_sysmenu_x + conf_sysmenu_qload_x,
				    conf_sysmenu_y + conf_sysmenu_qload_y,
				    conf_sysmenu_qload_width,
				    conf_sysmenu_qload_height,
				    sysmenu_hover_image,
				    conf_sysmenu_qload_x,
				    conf_sysmenu_qload_y,
				    conf_sysmenu_qload_width,
				    conf_sysmenu_qload_height,
				    255);
	}
	if (is_save_selected) {
		/* セーブの項目(選択)を描画する */
		render_image_normal(conf_sysmenu_x + conf_sysmenu_save_x,
				    conf_sysmenu_y + conf_sysmenu_save_y,
				    conf_sysmenu_save_width,
				    conf_sysmenu_save_height,
				    sysmenu_hover_image,
				    conf_sysmenu_save_x,
				    conf_sysmenu_save_y,
				    conf_sysmenu_save_width,
				    conf_sysmenu_save_height,
				    255);
	}
	if (is_load_selected) {
		/* ロードの項目(選択)を描画する */
		render_image_normal(conf_sysmenu_x + conf_sysmenu_load_x,
				    conf_sysmenu_y + conf_sysmenu_load_y,
				    conf_sysmenu_load_width,
				    conf_sysmenu_load_height,
				    sysmenu_hover_image,
				    conf_sysmenu_load_x,
				    conf_sysmenu_load_y,
				    conf_sysmenu_load_width,
				    conf_sysmenu_load_height,
				    255);
	}
	if (is_auto_selected) {
		/* オートの項目(選択)を描画する */
		render_image_normal(conf_sysmenu_x + conf_sysmenu_auto_x,
				    conf_sysmenu_y + conf_sysmenu_auto_y,
				    conf_sysmenu_auto_width,
				    conf_sysmenu_auto_height,
				    sysmenu_hover_image,
				    conf_sysmenu_auto_x,
				    conf_sysmenu_auto_y,
				    conf_sysmenu_auto_width,
				    conf_sysmenu_auto_height,
				    255);
	}
	if (is_skip_selected) {
		/* スキップの項目(選択)を描画する */
		render_image_normal(conf_sysmenu_x + conf_sysmenu_skip_x,
				    conf_sysmenu_y + conf_sysmenu_skip_y,
				    conf_sysmenu_skip_width,
				    conf_sysmenu_skip_height,
				    sysmenu_hover_image,
				    conf_sysmenu_skip_x,
				    conf_sysmenu_skip_y,
				    conf_sysmenu_skip_width,
				    conf_sysmenu_skip_height,
				    255);
	}
	if (is_history_selected) {
		/* ヒストリの項目(選択)を描画する */
		render_image_normal(conf_sysmenu_x + conf_sysmenu_history_x,
				    conf_sysmenu_y + conf_sysmenu_history_y,
				    conf_sysmenu_history_width,
				    conf_sysmenu_history_height,
				    sysmenu_hover_image,
				    conf_sysmenu_history_x,
				    conf_sysmenu_history_y,
				    conf_sysmenu_history_width,
				    conf_sysmenu_history_height,
				    255);
	}
	if (is_config_selected) {
		/* コンフィグの項目(選択)を描画する */
		render_image_normal(conf_sysmenu_x + conf_sysmenu_config_x,
				    conf_sysmenu_y + conf_sysmenu_config_y,
				    conf_sysmenu_config_width,
				    conf_sysmenu_config_height,
				    sysmenu_hover_image,
				    conf_sysmenu_config_x,
				    conf_sysmenu_config_y,
				    conf_sysmenu_config_width,
				    conf_sysmenu_config_height,
				    255);
	}
	if (is_custom1_selected) {
		/* コンフィグの項目(選択)を描画する */
		render_image_normal(conf_sysmenu_x + conf_sysmenu_custom1_x,
				    conf_sysmenu_y + conf_sysmenu_custom1_y,
				    conf_sysmenu_custom1_width,
				    conf_sysmenu_custom1_height,
				    sysmenu_hover_image,
				    conf_sysmenu_custom1_x,
				    conf_sysmenu_custom1_y,
				    conf_sysmenu_custom1_width,
				    conf_sysmenu_custom1_height,
				    255);
	}
	if (is_custom2_selected) {
		/* コンフィグの項目(選択)を描画する */
		render_image_normal(conf_sysmenu_x + conf_sysmenu_custom2_x,
				    conf_sysmenu_y + conf_sysmenu_custom2_y,
				    conf_sysmenu_custom2_width,
				    conf_sysmenu_custom2_height,
				    sysmenu_hover_image,
				    conf_sysmenu_custom2_x,
				    conf_sysmenu_custom2_y,
				    conf_sysmenu_custom2_width,
				    conf_sysmenu_custom2_height,
				    255);
	}
}

/*
 * Returns the pointed system menu item with a relative mouse position.
 */
int get_pointed_sysmenu_item(void)
{
	int x, y;

	x = mouse_pos_x - conf_sysmenu_x;
	y = mouse_pos_y - conf_sysmenu_y;

	if (x >= conf_sysmenu_qsave_x &&
	    x < conf_sysmenu_qsave_x + conf_sysmenu_qsave_width &&
	    y >= conf_sysmenu_qsave_y &&
	    y < conf_sysmenu_qsave_y + conf_sysmenu_qsave_height)
		return SYSMENU_QSAVE;

	if (x >= conf_sysmenu_qload_x &&
	    x < conf_sysmenu_qload_x + conf_sysmenu_qload_width &&
	    y >= conf_sysmenu_qload_y &&
	    y < conf_sysmenu_qload_y + conf_sysmenu_qload_height)
		return SYSMENU_QLOAD;

	if (x >= conf_sysmenu_save_x &&
	    x < conf_sysmenu_save_x + conf_sysmenu_save_width &&
	    y >= conf_sysmenu_save_y &&
	    y < conf_sysmenu_save_y + conf_sysmenu_save_height)
		return SYSMENU_SAVE;

	if (x >= conf_sysmenu_load_x &&
	    x < conf_sysmenu_load_x + conf_sysmenu_load_width &&
	    y >= conf_sysmenu_load_y &&
	    y < conf_sysmenu_load_y + conf_sysmenu_load_height)
		return SYSMENU_LOAD;

	if (x >= conf_sysmenu_auto_x &&
	    x < conf_sysmenu_auto_x + conf_sysmenu_auto_width &&
	    y >= conf_sysmenu_auto_y &&
	    y < conf_sysmenu_auto_y + conf_sysmenu_auto_height)
		return SYSMENU_AUTO;

	if (x >= conf_sysmenu_skip_x &&
	    x < conf_sysmenu_skip_x + conf_sysmenu_skip_width &&
	    y >= conf_sysmenu_skip_y &&
	    y < conf_sysmenu_skip_y + conf_sysmenu_skip_height)
		return SYSMENU_SKIP;

	if (x >= conf_sysmenu_history_x &&
	    x < conf_sysmenu_history_x + conf_sysmenu_history_width &&
	    y >= conf_sysmenu_history_y &&
	    y < conf_sysmenu_history_y + conf_sysmenu_history_height)
		return SYSMENU_HISTORY;

	if (x >= conf_sysmenu_config_x &&
	    x < conf_sysmenu_config_x + conf_sysmenu_config_width &&
	    y >= conf_sysmenu_config_y &&
	    y < conf_sysmenu_config_y + conf_sysmenu_config_height)
		return SYSMENU_CONFIG;

	if (x >= conf_sysmenu_custom1_x &&
	    x < conf_sysmenu_custom1_x + conf_sysmenu_custom1_width &&
	    y >= conf_sysmenu_custom1_y &&
	    y < conf_sysmenu_custom1_y + conf_sysmenu_custom1_height)
		return SYSMENU_CUSTOM1;

	if (x >= conf_sysmenu_custom2_x &&
	    x < conf_sysmenu_custom2_x + conf_sysmenu_custom2_width &&
	    y >= conf_sysmenu_custom2_y &&
	    y < conf_sysmenu_custom2_y + conf_sysmenu_custom2_height)
		return SYSMENU_CUSTOM2;

	/* None pointed. */
	return -1;
}

/*
 * Checks if the collapsed system menu is pointed with a absolute mouse position.
 */
bool is_collapsed_sysmenu_pointed(void)
{
	if (mouse_pos_x >= conf_sysmenu_collapsed_x &&
	    mouse_pos_x < conf_sysmenu_collapsed_x + sysmenu_collapsed_idle_image->width &&
	    mouse_pos_y >= conf_sysmenu_collapsed_y &&
	    mouse_pos_y < conf_sysmenu_collapsed_y + sysmenu_collapsed_idle_image->height)
		return true;

	return false;
}

/*
 * 折りたたみシステムメニューを描画する
 */
void render_collapsed_sysmenu(bool is_pointed)
{
	/* 折りたたみシステムメニューの背景を描画する */
	if (!is_pointed) {
		render_image_normal(conf_sysmenu_collapsed_x,
				    conf_sysmenu_collapsed_y,
				    -1, -1,
				    sysmenu_collapsed_idle_image,
				    0, 0,
				    -1, -1,
				    255);
	} else {
		render_image_normal(conf_sysmenu_collapsed_x,
				    conf_sysmenu_collapsed_y,
				    -1, -1,
				    sysmenu_collapsed_hover_image,
				    0, 0,
				    -1, -1,
				    255);
	}
}

/*
 * 折りたたみシステムメニューの座標を取得する
 */
void get_collapsed_sysmenu_rect(int *x, int *y, int *w, int *h)
{
	*x = conf_sysmenu_collapsed_x;
	*y = conf_sysmenu_collapsed_y;
	*w = sysmenu_collapsed_idle_image->width;
	*h = sysmenu_collapsed_idle_image->height;
}

/*
 * セーブデータ用サムネイルの描画
 */

/*
 * セーブデータ用サムネイル画像にステージ全体を描画する
 */
void draw_stage_to_thumb(void)
{
	int i;

	for (i = 0; i < STAGE_LAYERS; i++) {
		if (i == LAYER_MSG)
			if (!is_msgbox_visible)
				continue;
		if (i== LAYER_NAME)
			if (!is_namebox_visible || conf_namebox_hidden)
				continue;
		if (i == LAYER_AUTO)
			continue;
		if (i == LAYER_SKIP)
			continue;
		if (layer_image[i] == NULL)
			continue;
		if (layer_alpha[i] == 0)
			continue;
		draw_image_scale(thumb_image,
				 conf_window_width,
				 conf_window_height,
				 layer_x[i],
				 layer_y[i],
				 layer_image[i]);
	}
}

/*
 * セーブデータ用サムネイル画像にswitchの画像を描画する
 */
void draw_switch_to_thumb(struct image *img, int x, int y)
{
	draw_image_scale(thumb_image, conf_window_width, conf_window_height, x, y, img);
}

/*
 * セーブデータ用サムネイル画像を取得する
 */
struct image *get_thumb_image(void)
{
	return thumb_image;
}

/*
 * フェードモード
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
 * bg用のフェードを開始する
 */
bool start_fade_for_bg(const char *fname, struct image *img, int x, int y,
		       int alpha, int method, struct image *rule_img)
{
	assert(stage_mode == STAGE_MODE_IDLE);

	/* 背景フェードを有効にする */
	stage_mode = STAGE_MODE_BG_FADE;
	fade_method = method;

	/* フェードアウト用のレイヤにステージを描画する */
	draw_fo_common();

	/* 背景レイヤの情報をセットする */
	if (!set_layer_file_name(LAYER_BG, fname))
		return false;
	set_layer_image(LAYER_BG, img);
	set_layer_position(LAYER_BG, x, y);
	set_layer_alpha(LAYER_BG, alpha);

	/* 無効になるキャラクタを破棄する */
	set_layer_file_name(LAYER_CHB, NULL);
	set_layer_file_name(LAYER_CHL, NULL);
	set_layer_file_name(LAYER_CHLC, NULL);
	set_layer_file_name(LAYER_CHR, NULL);
	set_layer_file_name(LAYER_CHRC, NULL);
	set_layer_file_name(LAYER_CHC, NULL);
	set_layer_image(LAYER_CHB, NULL);
	set_layer_image(LAYER_CHL, NULL);
	set_layer_image(LAYER_CHLC, NULL);
	set_layer_image(LAYER_CHR, NULL);
	set_layer_image(LAYER_CHRC, NULL);
	set_layer_image(LAYER_CHC, NULL);

	/* フェードイン用のレイヤに背景を描画する */
	draw_fi_common(conf_msgbox_show_on_bg);

	/* ルールイメージを保持する */
	fade_rule_img = rule_img;

	return true;
}

/*
 * ch用のフェードを開始する
 */
bool start_fade_for_ch(int chpos, const char *fname, struct image *img,
		       int x, int y, int alpha, int method,
		       struct image *rule_img)
{
	int layer;

	assert(stage_mode == STAGE_MODE_IDLE);
	assert(chpos >= 0 && chpos < CH_ALL_LAYERS);

	stage_mode = STAGE_MODE_CH_FADE;
	fade_method = method;

	/* キャラフェードアウトレイヤにステージを描画する */
	draw_fo_common();

	/* キャラを入れ替える */
	layer = chpos_to_layer(chpos);
	if (!set_layer_file_name(layer, fname))
		return false;
	set_layer_image(layer, img);
	set_layer_alpha(layer, alpha);
	set_layer_position(layer, x, y);

	/* キャラフェードインレイヤにステージを描画する */
	draw_fi_common(conf_msgbox_show_on_ch);

	/* ルールイメージを保持する */
	fade_rule_img = rule_img;

	return true;
}

/*
 * chs用のフェードモードを開始する
 */
bool start_fade_for_chs(const bool *stay, const char **fname,
			struct image **img, const int *x, const int *y,
			const int *alpha, int method, struct image *rule_img)
{
	int i, layer;
	const int BG_INDEX = CH_BASIC_LAYERS;

	assert(stage_mode == STAGE_MODE_IDLE);

	/* このフェードではSTAGE_MODE_CHS_FADEを利用する */
	stage_mode = STAGE_MODE_CHS_FADE;
	fade_method = method;

	/* キャラフェードアウトレイヤにステージを描画する */
	draw_fo_common();

	/* キャラを入れ替える */
	for (i = 0; i < CH_BASIC_LAYERS; i++) {
		layer = chpos_to_layer(i);
		if (!stay[i]) {
			if (!set_layer_file_name(layer, fname[i]))
				return false;
			set_layer_image(layer, img[i]);
		}
		set_layer_position(layer, x[i], y[i]);
		set_layer_alpha(layer, alpha[i]);
	}

	/* 背景を入れ替える */
	if (!stay[BG_INDEX]) {
		if (!set_layer_file_name(LAYER_BG, fname[BG_INDEX]))
			return false;
		set_layer_image(LAYER_BG, img[BG_INDEX]);
	}
	set_layer_position(LAYER_BG, x[BG_INDEX], y[BG_INDEX]);

	/* キャラフェードインレイヤにステージを描画する */
	draw_fi_common(conf_msgbox_show_on_ch);

	/* ルールイメージを保持する */
	fade_rule_img = rule_img;

	return true;
}

/*
 * shake用のフェードモードを開始する
 */
void start_fade_for_shake(void)
{
	assert(stage_mode == STAGE_MODE_IDLE);

	stage_mode = STAGE_MODE_SHAKE_FADE;

	/* フェードアウト用のレイヤをクリアする */
	if (conf_window_white)
		clear_image_white(fo_image);
	else
		clear_image_black(fo_image);

	/* フェードイン用のレイヤにステージを描画する */
	draw_fi_common(conf_msgbox_show_on_ch);
}

/*
 * フェードの進捗率を設定する
 */
void set_fade_progress(float progress)
{
	assert(stage_mode == STAGE_MODE_BG_FADE ||
	       stage_mode == STAGE_MODE_CH_FADE ||
	       stage_mode == STAGE_MODE_CHS_FADE);

	/* 進捗率を保存する */
	fi_fo_fade_progress = progress;
}

/*
 * shakeの表示オフセットを設定する
 */
void set_shake_offset(int x, int y)
{
	assert(stage_mode == STAGE_MODE_SHAKE_FADE);

	shake_offset_x = x;
	shake_offset_y = y;
}

/*
 * フェードモードを終了する
 */
void finish_fade(void)
{
	assert(stage_mode == STAGE_MODE_BG_FADE ||
	       stage_mode == STAGE_MODE_CH_FADE ||
	       stage_mode == STAGE_MODE_CHS_FADE ||
	       stage_mode == STAGE_MODE_SHAKE_FADE);

	/* ルールイメージを破棄する */
	if (fade_rule_img != NULL) {
		destroy_image(fade_rule_img);
		fade_rule_img = NULL;
	}

	stage_mode = STAGE_MODE_IDLE;
}

/* FOにステージの内容を描画する */
static void draw_fo_common(void)
{
	if (conf_window_white)
		clear_image_white(fo_image);
	else
		clear_image_black(fo_image);
	draw_layer_image(fo_image, LAYER_BG);
	draw_layer_image(fo_image, LAYER_BG2);
	draw_layer_image(fo_image, LAYER_EFFECT5);
	draw_layer_image(fo_image, LAYER_EFFECT6);
	draw_layer_image(fo_image, LAYER_EFFECT7);
	draw_layer_image(fo_image, LAYER_EFFECT8);
	draw_layer_image(fo_image, LAYER_CHB);
	draw_layer_image(fo_image, LAYER_CHL);
	draw_layer_image(fo_image, LAYER_CHLC);
	draw_layer_image(fo_image, LAYER_CHR);
	draw_layer_image(fo_image, LAYER_CHRC);
	draw_layer_image(fo_image, LAYER_CHC);
	draw_layer_image(fo_image, LAYER_EFFECT1);
	draw_layer_image(fo_image, LAYER_EFFECT2);
	draw_layer_image(fo_image, LAYER_EFFECT3);
	draw_layer_image(fo_image, LAYER_EFFECT4);
	if (is_msgbox_visible)
		draw_layer_image(fo_image, LAYER_MSG);
	if (is_namebox_visible && !conf_namebox_hidden)
		draw_layer_image(fo_image, LAYER_NAME);
	if (is_msgbox_visible)
		draw_layer_image(fo_image, LAYER_CHF);
	if (is_auto_visible)
		draw_layer_image(fo_image, LAYER_AUTO);
	if (is_skip_visible)
		draw_layer_image(fo_image, LAYER_SKIP);
	draw_layer_image(fo_image, LAYER_TEXT1);
	draw_layer_image(fo_image, LAYER_TEXT2);
	draw_layer_image(fo_image, LAYER_TEXT3);
	draw_layer_image(fo_image, LAYER_TEXT4);
	draw_layer_image(fo_image, LAYER_TEXT5);
	draw_layer_image(fo_image, LAYER_TEXT6);
	draw_layer_image(fo_image, LAYER_TEXT7);
	draw_layer_image(fo_image, LAYER_TEXT8);
}

/* FIにステージの内容を描画する */
static void draw_fi_common(bool show_msgbox)
{
	if (conf_window_white)
		clear_image_white(fi_image);
	else
		clear_image_black(fi_image);
	draw_layer_image(fi_image, LAYER_BG);
	draw_layer_image(fi_image, LAYER_BG2);
	draw_layer_image(fi_image, LAYER_EFFECT5);
	draw_layer_image(fi_image, LAYER_EFFECT6);
	draw_layer_image(fi_image, LAYER_EFFECT7);
	draw_layer_image(fi_image, LAYER_EFFECT8);
	draw_layer_image(fi_image, LAYER_CHB);
	draw_layer_image(fi_image, LAYER_CHL);
	draw_layer_image(fi_image, LAYER_CHLC);
	draw_layer_image(fi_image, LAYER_CHR);
	draw_layer_image(fi_image, LAYER_CHRC);
	draw_layer_image(fi_image, LAYER_CHC);
	draw_layer_image(fi_image, LAYER_EFFECT1);
	draw_layer_image(fi_image, LAYER_EFFECT2);
	draw_layer_image(fi_image, LAYER_EFFECT3);
	draw_layer_image(fi_image, LAYER_EFFECT4);
	if (show_msgbox) {
		if (is_msgbox_visible)
			draw_layer_image(fi_image, LAYER_MSG);
		if (is_namebox_visible && !conf_namebox_hidden)
			draw_layer_image(fi_image, LAYER_NAME);
		if (is_msgbox_visible)
			draw_layer_image(fi_image, LAYER_CHF);
	}
	if (is_auto_visible)
		draw_layer_image(fi_image, LAYER_AUTO);
	if (is_skip_visible)
		draw_layer_image(fi_image, LAYER_SKIP);
	draw_layer_image(fi_image, LAYER_TEXT1);
	draw_layer_image(fi_image, LAYER_TEXT2);
	draw_layer_image(fi_image, LAYER_TEXT3);
	draw_layer_image(fi_image, LAYER_TEXT4);
	draw_layer_image(fi_image, LAYER_TEXT5);
	draw_layer_image(fi_image, LAYER_TEXT6);
	draw_layer_image(fi_image, LAYER_TEXT7);
	draw_layer_image(fi_image, LAYER_TEXT8);
}

/*
 * フェードのレンダリングを行う
 */
void render_fade(void)
{
	assert(stage_mode == STAGE_MODE_BG_FADE ||
	       stage_mode == STAGE_MODE_CH_FADE ||
	       stage_mode == STAGE_MODE_CHS_FADE ||
	       stage_mode == STAGE_MODE_SHAKE_FADE);

	if (stage_mode == STAGE_MODE_SHAKE_FADE) {
		render_fade_shake();
		return;
	}

	switch (fade_method) {
	case FADE_METHOD_NORMAL:
		render_fade_normal();
		break;
	case FADE_METHOD_RULE:
		render_fade_rule();
		break;
	case FADE_METHOD_MELT:
		render_fade_melt();
		break;
	case FADE_METHOD_CURTAIN_RIGHT:
		render_fade_curtain_right();
		break;
	case FADE_METHOD_CURTAIN_LEFT:
		render_fade_curtain_left();
		break;
	case FADE_METHOD_CURTAIN_UP:
		render_fade_curtain_up();
		break;
	case FADE_METHOD_CURTAIN_DOWN:
		render_fade_curtain_down();
		break;
	case FADE_METHOD_SLIDE_RIGHT:
		render_fade_slide_right();
		break;
	case FADE_METHOD_SLIDE_LEFT:
		render_fade_slide_left();
		break;
	case FADE_METHOD_SLIDE_UP:
		render_fade_slide_up();
		break;
	case FADE_METHOD_SLIDE_DOWN:
		render_fade_slide_down();
		break;
	case FADE_METHOD_SHUTTER_RIGHT:
		render_fade_shutter_right();
		break;
	case FADE_METHOD_SHUTTER_LEFT:
		render_fade_shutter_left();
		break;
	case FADE_METHOD_SHUTTER_UP:
		render_fade_shutter_up();
		break;
	case FADE_METHOD_SHUTTER_DOWN:
		render_fade_shutter_down();
		break;
	case FADE_METHOD_CLOCKWISE:
		render_fade_clockwise(FADE_METHOD_CLOCKWISE);
		break;
	case FADE_METHOD_COUNTERCLOCKWISE:
		render_fade_counterclockwise(
			FADE_METHOD_COUNTERCLOCKWISE);
		break;
	case FADE_METHOD_CLOCKWISE20:
		render_fade_clockwise(FADE_METHOD_CLOCKWISE20);
		break;
	case FADE_METHOD_COUNTERCLOCKWISE20:
		render_fade_counterclockwise(
			FADE_METHOD_COUNTERCLOCKWISE20);
		break;
	case FADE_METHOD_CLOCKWISE30:
		render_fade_clockwise(FADE_METHOD_CLOCKWISE30);
		break;
	case FADE_METHOD_COUNTERCLOCKWISE30:
		render_fade_counterclockwise(
			FADE_METHOD_COUNTERCLOCKWISE30);
		break;
	case FADE_METHOD_EYE_OPEN:
		render_fade_eye_open();
		break;
	case FADE_METHOD_EYE_CLOSE:
		render_fade_eye_close();
		break;
	case FADE_METHOD_EYE_OPEN_V:
		render_fade_eye_open_v();
		break;
	case FADE_METHOD_EYE_CLOSE_V:
		render_fade_eye_close_v();
		break;
	case FADE_METHOD_SLIT_OPEN:
		render_fade_slit_open();
		break;
	case FADE_METHOD_SLIT_CLOSE:
		render_fade_slit_close();
		break;
	case FADE_METHOD_SLIT_OPEN_V:
		render_fade_slit_open_v();
		break;
	case FADE_METHOD_SLIT_CLOSE_V:
		render_fade_slit_close_v();
		break;
	default:
		assert(INVALID_FADE_METHOD);
		break;
	}
}

/* デフォルトの背景フェードの描画を行う  */
static void render_fade_normal(void)
{
	render_image_normal(0, 0, -1, -1, fo_image, 0, 0, -1, -1, 255);
	render_image_normal(0, 0, -1, -1, fi_image, 0, 0, -1, -1, (int)(fi_fo_fade_progress * 255.0f));
}

/* ルール描画を行う */
static void render_fade_rule(void)
{
	int threshold;

	assert(stage_mode == STAGE_MODE_BG_FADE ||
	       stage_mode == STAGE_MODE_CH_FADE ||
	       stage_mode == STAGE_MODE_CHS_FADE);
	assert(fade_rule_img != NULL);

	/* テンプレートの閾値を求める */
	threshold = (int)(255.0f * fi_fo_fade_progress);

	/* フェードアウトする画像をコピーする */
	render_image_normal(0, 0, -1, -1, fo_image, 0, 0, -1, -1, 255);

	/* フェードインする画像をレンダリングする */
	render_image_rule(fi_image, fade_rule_img, threshold);
}

/* ルール描画(メルト)を行う */
static void render_fade_melt(void)
{
	int threshold;

	assert(stage_mode == STAGE_MODE_BG_FADE ||
	       stage_mode == STAGE_MODE_CH_FADE ||
	       stage_mode == STAGE_MODE_CHS_FADE);
	assert(fade_rule_img != NULL);

	/* テンプレートの閾値を求める */
	threshold = (int)(255.0f * fi_fo_fade_progress);

	/* フェードアウトする画像をコピーする */
	render_image_normal(0, 0, -1, -1, fo_image, 0, 0, -1, -1, 255);

	/* フェードインする画像をレンダリングする */
	render_image_melt(fi_image, fade_rule_img, threshold);
}

/* 右方向カーテンフェードの描画を行う */
static void render_fade_curtain_right(void)
{
	int right, alpha, i;

	/*
	 * カーテンの右端を求める
	 *  - カーテンの右端は0からconf_window_width+CURTAIN_WIDTHになる
	 */
	right = (int)((float)(conf_window_width + CURTAIN_WIDTH) * fi_fo_fade_progress);

	/* カーテンが通り過ぎる前の背景をコピーする */
	if (right < conf_window_width) {
		render_image_normal(right,
				    0,
				    conf_window_width - right,
				    conf_window_height,
				    fo_image,
				    right,
				    0,
				    conf_window_width - right,
				    conf_window_height,
				    255);
	}

	/* カーテンの部分の背景をコピーする */
	render_image_normal(right - CURTAIN_WIDTH,
			    0,
			    CURTAIN_WIDTH,
			    conf_window_height,
			    fo_image,
			    right - CURTAIN_WIDTH,
			    0,
			    CURTAIN_WIDTH,
			    conf_window_height,
			    255);

	/* カーテンが通り過ぎた後の背景を描画する */
	if (right >= CURTAIN_WIDTH) {
		render_image_normal(0,
				    0,
				    right - CURTAIN_WIDTH,
				    conf_window_height,
				    fi_image,
				    0,
				    0,
				    right - CURTAIN_WIDTH,
				    conf_window_height,
				    255);
	}

	/* カーテンを描画する */
	for (alpha = 0, i = right; i >= right - CURTAIN_WIDTH; i--, alpha++) {
		if (i < 0 || i >= conf_window_width)
			continue;
		if (alpha > 255)
			alpha = 255;
		render_image_normal(i,
				    0,
				    2,
				    conf_window_height,
				    fi_image,
				    i,
				    0,
				    2,
				    conf_window_height,
				    alpha);
	}
}

/* 左方向カーテンフェードの描画を行う */
static void render_fade_curtain_left(void)
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
	render_image_normal(0,
			    0,
			    left + CURTAIN_WIDTH,
			    conf_window_height,
			    fo_image,
			    0,
			    0,
			    left + CURTAIN_WIDTH,
			    conf_window_height,
			    255);

	/* カーテンが通り過ぎた後の背景を描画する */
	if (left <= conf_window_width - CURTAIN_WIDTH) {
		render_image_normal(left + CURTAIN_WIDTH,
				    0,
				    conf_window_width - left - CURTAIN_WIDTH,
				    conf_window_height,
				    fi_image,
				    left + CURTAIN_WIDTH,
				    0,
				    conf_window_width - left - CURTAIN_WIDTH,
				    conf_window_height,
				    255);
	}

	/* カーテンを描画する */
	for (alpha = 0, i = left; i <= left + CURTAIN_WIDTH; i++, alpha++) {
		if (i < 0 || i >= conf_window_width)
			continue;
		if (alpha > 255)
			alpha = 255;
		render_image_normal(i,
				    0,
				    1,
				    conf_window_height,
				    fi_image,
				    i,
				    0,
				    1,
				    conf_window_height,
				    alpha);
	}
}

/* 上方向カーテンフェードの描画を行う */
static void render_fade_curtain_up(void)
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
	render_image_normal(0,
			    0,
			    conf_window_width,
			    top + CURTAIN_WIDTH,
			    fo_image,
			    0,
			    0,
			    conf_window_width,
			    top + CURTAIN_WIDTH,
			    255);

	/* カーテンが通り過ぎた後の背景を描画する */
	if (top <= conf_window_height - CURTAIN_WIDTH) {
		render_image_normal(0,
				    top + CURTAIN_WIDTH,
				    conf_window_width,
				    conf_window_height - top - CURTAIN_WIDTH,
				    fi_image,
				    0,
				    top + CURTAIN_WIDTH,
				    conf_window_width,
				    conf_window_height - top - CURTAIN_WIDTH,
				    255);
	}

	/* カーテンを描画する */
	for (alpha = 0, i = top; i <= top + CURTAIN_WIDTH; i++, alpha++) {
		if (i < 0 || i >= conf_window_height)
			continue;
		if (alpha > 255)
			alpha = 255;
		render_image_normal(0,
				    i,
				    conf_window_width,
				    1,
				    fi_image,
				    0,
				    i,
				    conf_window_width,
				    1,
				    alpha);
	}
}

/* 下方向カーテンフェードの描画を行う */
static void render_fade_curtain_down(void)
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
		render_image_normal(0,
				    bottom,
				    conf_window_width,
				    conf_window_height - bottom,
				    fo_image,
				    0,
				    bottom,
				    conf_window_width,
				    conf_window_height - bottom,
				    255);
	}

	/* カーテンの部分の背景をコピーする */
	render_image_normal(0,
			    bottom - CURTAIN_WIDTH,
			    conf_window_width,
			    CURTAIN_WIDTH,
			    fo_image,
			    0,
			    bottom - CURTAIN_WIDTH,
			    conf_window_width,
			    CURTAIN_WIDTH,
			    255);

	/* カーテンが通り過ぎた後の背景を描画する */
	if (bottom >= CURTAIN_WIDTH) {
		render_image_normal(0,
				    0,
				    conf_window_width,
				    bottom - CURTAIN_WIDTH,
				    fi_image,
				    0,
				    0,
				    conf_window_width,
				    bottom - CURTAIN_WIDTH,
				    255);
	}

	/* カーテンを描画する */
	for (alpha = 0, i = bottom; i >= bottom - CURTAIN_WIDTH;
	     i--, alpha++) {
		if (i < 0 || i >= conf_window_height)
			continue;
		if (alpha > 255)
			alpha = 255;
		render_image_normal(0,
				    i,
				    conf_window_width,
				    1,
				    fi_image,
				    0,
				    i,
				    conf_window_width,
				    1,
				    alpha);
	}
}

/* 右方向スライドフェードの描画を行う */
static void render_fade_slide_right(void)
{
	int shift;

	/*
	 * スライドの右端を求める
	 *  - スライドの右端は0からconf_window_widthになる
	 */
	shift = (int)((float)conf_window_width * fi_fo_fade_progress);

	/* 左側の背景を表示する */
	render_image_normal(-(conf_window_width - shift),
			    0,
			    conf_window_width,
			    conf_window_height,
			    fi_image,
			    0,
			    0,
			    conf_window_width,
			    conf_window_height,
			    255);

	/* 右側の背景を表示する */
	render_image_normal(shift,
			    0,
			    conf_window_width,
			    conf_window_height,
			    fo_image,
			    0,
			    0,
			    conf_window_width,
			    conf_window_height,
			    255);
}

/* 左方向スライドフェードの描画を行う */
static void render_fade_slide_left(void)
{
	int shift, left;

	/*
	 * スライドの左端を求める
	 *  - スライドの左端はconf_window_widthから0になる
	 */
	shift = (int)((float)conf_window_width * fi_fo_fade_progress);
	left = conf_window_width - shift;

	/* 右側の背景を表示する */
	render_image_normal(left,
			    0,
			    conf_window_width - left,
			    conf_window_height,
			    fi_image,
			    0,
			    0,
			    conf_window_width - left,
			    conf_window_height,
			    255);

	/* 左側の背景を表示する */
	render_image_normal(-shift,
			    0,
			    conf_window_width,
			    conf_window_height,
			    fo_image,
			    0,
			    0,
			    conf_window_width,
			    conf_window_height,
			    255);
}

/* 上方向スライドフェードの描画を行う */
static void render_fade_slide_up(void)
{
	int shift, top;

	/*
	 * スライドの上端を求める
	 *  - スライドの上端はconf_window_heightから0になる
	 */
	shift = (int)((float)conf_window_height * fi_fo_fade_progress);
	top = conf_window_height - shift;

	/* 上側の背景を表示する */
	render_image_normal(0,
			    -shift,
			    conf_window_width,
			    conf_window_height,
			    fo_image,
			    0,
			    0,
			    conf_window_width,
			    conf_window_height,
			    255);

	/* 下側の背景を表示する */
	render_image_normal(0,
			    top,
			    conf_window_width,
			    conf_window_height,
			    fi_image,
			    0,
			    0,
			    conf_window_width,
			    conf_window_height,
			    255);
}

/* 下方向スライドフェードの描画を行う */
static void render_fade_slide_down(void)
{
	int bottom;

	/*
	 * スライドの下端を求める
	 *  - スライドの下端は0からconf_window_heightになる
	 */
	bottom = (int)((float)conf_window_height * fi_fo_fade_progress);

	/* 上側の背景を表示する */
	render_image_normal(0,
			    0,
			    conf_window_width,
			    bottom,
			    fi_image,
			    0,
			    conf_window_height - bottom,
			    conf_window_width,
			    bottom,
			    255);

	/* 下側の背景を表示する */
	render_image_normal(0,
			    bottom,
			    conf_window_width,
			    conf_window_height - bottom,
			    fo_image,
			    0,
			    0,
			    conf_window_width,
			    conf_window_height - bottom,
			    255);
}

/* 右方向シャッターフェードの描画を行う Right direction shutter fade */
static void render_fade_shutter_right(void)
{
	int right;

	/*
	 * スライドの右端を求める
	 *  - スライドの右端は0からconf_window_widthになる
	 */
	right = (int)((float)conf_window_width * fi_fo_fade_progress);

	/* 左側の背景を表示する */
	render_image_normal(0,
			    0,
			    right,
			    conf_window_height,
			    fi_image,
			    conf_window_width - right,
			    0,
			    right,
			    conf_window_height,
			    255);

	/* 右側の背景を表示する */
	render_image_normal(right,
			    0,
			    conf_window_width - right,
			    conf_window_height,
			    fo_image,
			    right,
			    0,
			    conf_window_width - right,
			    conf_window_height,
			    255);
}

/* 左方向シャッターフェードの描画を行う Left direction shutter fade */
static void render_fade_shutter_left(void)
{
	int left;

	/*
	 * スライドの左端を求める
	 *  - スライドの左端はconf_window_widthから0になる
	 */
	left = conf_window_width - (int)((float)conf_window_width * fi_fo_fade_progress);

	/* 右側の背景を表示する */
	render_image_normal(left,
			    0,
			    conf_window_width - left,
			    conf_window_height,
			    fi_image,
			    0,
			    0,
			    conf_window_width - left,
			    conf_window_height,
			    255);

	/* 左側の背景を表示する */
	render_image_normal(0,
			    0,
			    left,
			    conf_window_height,
			    fo_image,
			    0,
			    0,
			    left,
			    conf_window_height,
			    255);
}

/* 上方向シャッターフェードの描画を行う Up direction shutter fade */
static void render_fade_shutter_up(void)
{
	int top;

	/*
	 * スライドの上端を求める
	 *  - スライドの上端はconf_window_heightから0になる
	 */
	top = conf_window_height - (int)((float)conf_window_height * fi_fo_fade_progress);

	/* 上側の背景を表示する */
	render_image_normal(0,
			    0,
			    conf_window_width,
			    top,
			    fo_image,
			    0,
			    0,
			    conf_window_width,
			    top,
			    255);

	/* 下側の背景を表示する */
	render_image_normal(0,
			    top,
			    conf_window_width,
			    conf_window_height - top,
			    fi_image,
			    0,
			    0,
			    conf_window_width,
			    conf_window_height - top,
			    255);
}

/* 下方向シャッターフェードの描画を行う Down direction shutter fade */
static void render_fade_shutter_down(void)
{
	int bottom;

	/*
	 * スライドの下端を求める
	 *  - スライドの下端は0からconf_window_heightになる
	 */
	bottom = (int)((float)conf_window_height * fi_fo_fade_progress);

	/* 上側の背景を表示する */
	render_image_normal(0,
			    0,
			    conf_window_width,
			    bottom,
			    fi_image,
			    0,
			    conf_window_height - bottom,
			    conf_window_width,
			    bottom,
			    255);

	/* 下側の背景を表示する */
	render_image_normal(0,
			    bottom,
			    conf_window_width,
			    conf_window_height - bottom,
			    fo_image,
			    0,
			    bottom,
			    conf_window_width,
			    conf_window_height - bottom,
			    255);
}

/* 時計回りフェードの描画を行う */
static void render_fade_clockwise(int method)
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
	render_image_normal(0,
			    0,
			    conf_window_width,
			    conf_window_height,
			    fo_image,
			    0,
			    0,
			    conf_window_width,
			    conf_window_height,
			    255);

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
		render_image_normal(center_x,
				    0,
				    half_w,
				    half_h + 1,
				    fi_image,
				    center_x,
				    0,
				    half_w,
				    half_h + 1,
				    255);
	}

	/* 第四象限を埋める */
	if (progress >= 0.5f) {
		render_image_normal(center_x,
				    center_y,
				    half_w,
				    half_h,
				    fi_image,
				    center_x,
				    center_y,
				    half_w,
				    half_h,
				    255);
	}

	/* 第三象限を埋める */
	if (progress >= 0.75f) {
		render_image_normal(0,
				    center_y,
				    half_w + 1,
				    half_h,
				    fi_image,
				    0,
				    center_y,
				    half_w + 1,
				    half_h,
				    255);
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
		render_image_normal(min,
				    i,
				    max - min + 1,
				    1,
				    fi_image,
				    min,
				    i,
				    max - min + 1,
				    1,
				    255);
	}
}

/* 反時計回りフェードの描画を行う */
static void render_fade_counterclockwise(int method)
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
	render_image_normal(0,
			    0,
			    conf_window_width,
			    conf_window_height,
			    fo_image,
			    0,
			    0,
			    conf_window_width,
			    conf_window_height,
			    255);

	/* 時計の針の位置を計算する */
	center_x = conf_window_width / 2;
	center_y = conf_window_height / 2;
	hand_len = (conf_window_width > conf_window_height) ?
		(float)conf_window_width : (float)conf_window_height;
	hand_x = center_x + (int)(hand_len *
				  sinf(2.0f * PI * progress - PI));
	hand_y = center_y + (int)(hand_len *
				  cosf(2.0f * PI * progress - PI));

	/* 第二象限を埋める */
	if (progress >= 0.25f) {
		render_image_normal(0,
				    0,
				    half_w,
				    half_h + 1,
				    fi_image,
				    0,
				    0,
				    half_w,
				    half_h + 1,
				    255);
	}

	/* 第三象限を埋める */
	if (progress >= 0.5f) {
		render_image_normal(0,
				   center_y,
				   half_w + 1,
				   half_h,
				   fi_image,
				   0,
				   center_y + 1,
				   half_w + 1,
				   half_h,
				   255);
	}

	/* 第四象限を埋める */
	if (progress >= 0.75f) {
		render_image_normal(center_x,
				    center_y,
				    half_w,
				    half_h,
				    fi_image,
				    center_x,
				    center_y,
				    half_w,
				    half_h,
				    255);
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
		render_image_normal(min,
				    i,
				    max - min + 1,
				    1,
				    fi_image,
				    min,
				    i,
				    max - min + 1,
				    1,
				    255);
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
static void render_fade_eye_open(void)
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
	render_image_normal(0,
			    0,
			    conf_window_width,
			    conf_window_height,
			    fi_image,
			    0,
			    0,
			    conf_window_width,
			    conf_window_height,
			    255);

	/* 上幕の描画を行う */
	for (i = up, a = 0; i >= 0; i--) {
		render_image_normal(0,
				    i,
				    conf_window_width,
				    1,
				    fo_image,
				    0,
				    i,
				    conf_window_width,
				    1,
				    a);
		a += ALPHA_STEP;
		if (a > 255)
			a = 255;
	}

	/* 下幕の描画を行う */
	for (i = down, a = 0; i <= conf_window_height - 1; i++) {
		render_image_normal(0,
				    i,
				    conf_window_width,
				    1,
				    fo_image,
				    0,
				    i,
				    conf_window_width,
				    1,
				    a);
		a += ALPHA_STEP;
		if (a > 255)
			a = 255;
	}
}

/* 目閉じフェードの描画を行う */
static void render_fade_eye_close(void)
{
	int up, down, i, a;
	const int ALPHA_STEP = 4;

	/* 上幕の下端を求める */
	up = (int)((float)(conf_window_height / 2 - 1) * fi_fo_fade_progress);

	/* 下幕の上端を求める */
	down = (int)((float)(conf_window_height - 1) -
		     (float)(conf_window_height / 2 - 1) * fi_fo_fade_progress);

	/* 幕が通り過ぎる前の背景をコピーする */
	render_image_normal(0,
			    0,
			    conf_window_width,
			    conf_window_height,
			    fo_image,
			    0,
			    0,
			    conf_window_width,
			    conf_window_height,
			    255);

	/* 上幕の描画を行う */
	for (i = up, a = 0; i >= 0; i--) {
		render_image_normal(0,
				    i,
				    conf_window_width,
				    1,
				    fi_image,
				    0,
				    i,
				    conf_window_width,
				    1,
				    a);
		a += ALPHA_STEP;
		if (a > 255)
			a = 255;
	}

	/* 下幕の描画を行う */
	for (i = down, a = 0; i <= conf_window_height - 1; i++) {
		render_image_normal(0,
				    i,
				    conf_window_width,
				    1,
				    fi_image,
				    0,
				    i,
				    conf_window_width,
				    1,
				    a);
		a += ALPHA_STEP;
		if (a > 255)
			a = 255;
	}
}

/* 目開きフェード(垂直)の描画を行う */
static void render_fade_eye_open_v(void)
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
	render_image_normal(0,
			    0,
			    conf_window_width,
			    conf_window_height,
			    fi_image,
			    0,
			    0,
			    conf_window_width,
			    conf_window_height,
			    255);

	/* 左幕の描画を行う */
	for (i = left, a = 0; i >= 0; i--) {
		render_image_normal(i,
				    0,
				    1,
				    conf_window_height,
				    fo_image,
				    i,
				    0,
				    1,
				    conf_window_height,
				    a);
		a += ALPHA_STEP;
		if (a > 255)
			a = 255;
	}

	/* 下幕の描画を行う */
	for (i = right, a = 0; i <= conf_window_width - 1; i++) {
		render_image_normal(i,
				    0,
				    1,
				    conf_window_height,
				    fo_image,
				    i,
				    0,
				    1,
				    conf_window_height,
				    a);
		a += ALPHA_STEP;
		if (a > 255)
			a = 255;
	}
}

/* 目閉じフェード(垂直)の描画を行う */
static void render_fade_eye_close_v(void)
{
	int left, right, i, a;
	const int ALPHA_STEP = 4;

	/* 左幕の右端を求める */
	left = (int)((float)(conf_window_width / 2 - 1) * fi_fo_fade_progress);

	/* 右幕の左端を求める */
	right = (int)((float)(conf_window_width - 1) -
		      (float)(conf_window_width / 2 - 1) * fi_fo_fade_progress);

	/* 幕が通り過ぎる前の背景をコピーする */
	render_image_normal(0,
			    0,
			    conf_window_width,
			    conf_window_height,
			    fo_image,
			    0,
			    0,
			    conf_window_width,
			    conf_window_height,
			    255);

	/* 左幕の描画を行う */
	for (i = left, a = 0; i >= 0; i--) {
		render_image_normal(i,
				    0,
				    1,
				    conf_window_height,
				    fi_image,
				    i,
				    0,
				    1,
				    conf_window_height,
				    a);
		a += ALPHA_STEP;
		if (a > 255)
			a = 255;
	}

	/* 右幕の描画を行う */
	for (i = right, a = 0; i <= conf_window_width - 1; i++) {
		render_image_normal(i,
				    0,
				    1,
				    conf_window_height,
				    fi_image,
				    i,
				    0,
				    1,
				    conf_window_height,
				    a);
		a += ALPHA_STEP;
		if (a > 255)
			a = 255;
	}
}

/* スリット開きフェードの描画を行う */
static void render_fade_slit_open(void)
{
	int up, down;

	/* 上幕の下端を求める */
	up = (int)((float)(conf_window_height / 2 - 1) -
		   (float)(conf_window_height / 2 - 1) * fi_fo_fade_progress);

	/* 下幕の上端を求める */
	down = (int)((float)(conf_window_height / 2) +
		     (float)(conf_window_height / 2 - 1) * fi_fo_fade_progress);

	/* 幕が通り過ぎた後の背景をコピーする */
	render_image_normal(0,
			    0,
			    conf_window_width,
			    conf_window_height,
			    fi_image,
			    0,
			    0,
			    conf_window_width,
			    conf_window_height,
			    255);

	/* 上幕の描画を行う */
	render_image_normal(0,
			    0,
			    conf_window_width,
			    up + 1,
			    fo_image,
			    0,
			    0,
			    conf_window_width,
			    up + 1,
			    255);

	/* 下幕の描画を行う */
	render_image_normal(0,
			    down,
			    conf_window_width,
			    conf_window_height - down + 1,
			    fo_image,
			    0,
			    down,
			    conf_window_width,
			    conf_window_height - down + 1,
			    255);
}

/* スリット閉じフェードの描画を行う */
static void render_fade_slit_close(void)
{
	int up, down;

	/* 上幕の下端を求める */
	up = (int)((float)(conf_window_height / 2 - 1) * fi_fo_fade_progress);

	/* 下幕の上端を求める */
	down = (int)((float)(conf_window_height - 1) -
		     (float)(conf_window_height / 2 - 1) * fi_fo_fade_progress);

	/* 幕が通り過ぎる前の背景をコピーする */
	render_image_normal(0,
			    0,
			    conf_window_width,
			    conf_window_height,
			    fo_image,
			    0,
			    0,
			    conf_window_width,
			    conf_window_height,
			    255);

	/* 上幕の描画を行う */
	render_image_normal(0,
			    0,
			    conf_window_width,
			    up + 1,
			    fi_image,
			    0,
			    0,
			    conf_window_width,
			    up + 1,
			    255);

	/* 下幕の描画を行う */
	render_image_normal(0,
			    down,
			    conf_window_width,
			    conf_window_height - down + 1,
			    fi_image,
			    0,
			    down,
			    conf_window_width,
			    conf_window_height - down + 1,
			    255);
}

/* スリット開きフェード(垂直)の描画を行う */
static void render_fade_slit_open_v(void)
{
	int left, right;

	/* 左幕の右端を求める */
	left = (int)((float)(conf_window_width / 2 - 1) -
		     (float)(conf_window_width / 2 - 1) * fi_fo_fade_progress);

	/* 右幕の左端を求める */
	right = (int)((float)(conf_window_width / 2) +
		      (float)(conf_window_width / 2 - 1) * fi_fo_fade_progress);

	/* 幕が通り過ぎた後の背景をコピーする */
	render_image_normal(0,
			    0,
			    conf_window_width,
			    conf_window_height,
			    fi_image,
			    0,
			    0,
			    conf_window_width,
			    conf_window_height,
			    255);

	/* 左幕の描画を行う */
	render_image_normal(0,
			    0,
			    left + 1,
			    conf_window_height,
			    fo_image,
			    0,
			    0,
			    left + 1,
			    conf_window_height,
			    255);

	/* 下幕の描画を行う */
	render_image_normal(right,
			    0,
			    conf_window_width - right + 1,
			    conf_window_height,
			    fo_image,
			    right,
			    0,
			    conf_window_width - right + 1,
			    conf_window_height,
			    255);
}

/* スリット開きフェード(垂直)の描画を行う */
static void render_fade_slit_close_v(void)
{
	int left, right;

	/* 左幕の右端を求める */
	left = (int)((float)(conf_window_width / 2 - 1) * fi_fo_fade_progress);

	/* 右幕の左端を求める */
	right = (int)((float)(conf_window_width - 1) -
		      (float)(conf_window_width / 2 - 1) * fi_fo_fade_progress);

	/* 幕が通り過ぎる前の背景をコピーする */
	render_image_normal(0,
			    0,
			    conf_window_width,
			    conf_window_height,
			    fo_image,
			    0,
			    0,
			    conf_window_width,
			    conf_window_height,
			    255);

	/* 左幕の描画を行う */
	render_image_normal(0,
			    0,
			    left + 1,
			    conf_window_height,
			    fi_image,
			    0,
			    0,
			    left + 1,
			    conf_window_height,
			    255);

	/* 下幕の描画を行う */
	render_image_normal(right,
			    0,
			    conf_window_width - right + 1,
			    conf_window_height,
			    fi_image,
			    right,
			    0,
			    conf_window_width - right + 1,
			    conf_window_height,
			    255);
}

/* 画面揺らしモードが有効な際のステージ描画を行う */
static void render_fade_shake(void)
{
	/* FOレイヤを描画する */
	render_image_normal(0,
			    0,
			    conf_window_width,
			    conf_window_height,
			    fo_image,
			    0,
			    0,
			    conf_window_width,
			    conf_window_height,
			    255);

	/* FIレイヤを描画する */
	render_image_normal(shake_offset_x,
			    shake_offset_y,
			    conf_window_width,
			    conf_window_height,
			    fi_image,
			    0,
			    0,
			    conf_window_width,
			    conf_window_height,
			    255);

	if (is_auto_visible)
		render_layer_image(LAYER_AUTO);
	if (is_skip_visible)
		render_layer_image(LAYER_SKIP);
}

/*
 * キャラクタの描画
 */

/*
 * キャラ位置にキャラ番号を指定する
 */
void set_ch_name_mapping(int pos, int ch_name_index)
{
	assert(pos >= 0 && pos < CH_BASIC_LAYERS);

	ch_name_mapping[pos] = ch_name_index;
}

/*
 * 発話キャラを設定する
 *  - ch_name_index == -1 であれば誰も発話していない
 */
void set_ch_talking(int ch_name_index)
{
	ch_talking = ch_name_index;
}

/*
 * キャラの自動明暗を発話キャラを元に更新する
 */
void update_ch_dim_by_talking_ch(void)
{
	int i;

	for (i = 0; i < CH_BASIC_LAYERS; i++) {
		if (ch_talking == -1)
			ch_dim[i] = conf_character_focus == 2;
		else if (ch_name_mapping[i] == -1)
			ch_dim[i] = conf_character_focus == 2;
		else if (ch_name_mapping[i] == ch_talking)
			ch_dim[i] = false;
		else if (strncmp(conf_character_file[ch_name_mapping[i]],
				 conf_character_file[ch_talking],
				 strlen(conf_character_file[ch_talking])) == 0)
			ch_dim[i] = false;
		else
			ch_dim[i] = true;
	}
}

/*
 * キャラの明暗を手動で設定する
 */
void force_ch_dim(int chpos, bool is_dim)
{
	assert(chpos >= 0 && chpos < CH_BASIC_LAYERS);

	ch_dim[chpos] = is_dim;
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
	*w = layer_image[LAYER_NAME]->width;
	*h = layer_image[LAYER_NAME]->width;
}

/*
 * 名前ボックスを名前ボックス画像で埋める
 */
void fill_namebox(void)
{
	if (namebox_image == NULL)
		return;

	draw_image_copy(layer_image[LAYER_NAME],
			0, 0,
			namebox_image,
			layer_image[LAYER_NAME]->width,
			layer_image[LAYER_NAME]->height,
			0, 0);
}

/*
 * 名前ボックスの表示・非表示を設定する
 */
void show_namebox(bool show)
{
	is_namebox_visible = show;
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
	*w = layer_image[LAYER_MSG]->width;
	*h = layer_image[LAYER_MSG]->height;
}

/*
 * メッセージボックスの背景を描画する
 */
void fill_msgbox(void)
{
	if (msgbox_bg_image == NULL)
		return;

	draw_image_copy(layer_image[LAYER_MSG],
			0, 0,
			msgbox_bg_image,
			layer_image[LAYER_MSG]->width,
			layer_image[LAYER_MSG]->height,
			0, 0);
}

/*
 * メッセージボックスの背景の矩形を描画する
 */
void fill_msgbox_rect_with_bg(int x, int y, int w, int h)
{
	if (msgbox_bg_image == NULL)
		return;

	draw_image_copy(layer_image[LAYER_MSG], x, y, msgbox_bg_image, w, h, x, y);
}

/*
 * メッセージボックスの前景の矩形を描画する
 */
void fill_msgbox_rect_with_fg(int x, int y, int w, int h)
{
	if (msgbox_fg_image == NULL)
		return;

	draw_image_copy(layer_image[LAYER_MSG], x, y, msgbox_fg_image, w, h, x, y);
}

/*
 * メッセージボックスの表示・非表示を設定する
 */
void show_msgbox(bool show)
{
	is_msgbox_visible = show;
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
	*w = layer_image[LAYER_CLICK]->width;
	*h = layer_image[LAYER_CLICK]->height;
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
 * スイッチ(@choose, @switch)の描画
 */

/*
 * スイッチの矩形を取得する
 */
void get_switch_rect(int index, int *x, int *y, int *w, int *h)
{
	int use_image_index, width, height;

	assert(index >= 0 && index < 8);

	if (index != 0 && switch_bg_image[index] == NULL)
		use_image_index = 0;
	else
		use_image_index = index;

	width = switch_bg_image[use_image_index]->width;
	height = switch_bg_image[use_image_index]->height;

	if (use_image_index == 0) {
		*x = conf_switch_x[0];
		*y = conf_switch_y[0] + (height + conf_switch_margin_y) * index;
		*w = width;
		*h = height;
	} else {
		*x = conf_switch_x[use_image_index];
		*y = conf_switch_y[use_image_index];
		*w = width;
		*h = height;
	}
}

/*
 * NEWSの矩形を取得する
 */
void get_news_rect(int index, int *x, int *y, int *w, int *h)
{
	const int NORTH = 0;
	const int EAST = 1;
	const int WEST = 2;
	const int SOUTH = 3;
	const int SWITCH_BASE = 4;

	struct image *bg;

	bg = news_bg_image != NULL ? news_bg_image : switch_bg_image[0];

	if (index == NORTH) {
		*w = bg->width;
		*h = bg->height;
		*x = (conf_window_width - *w) / 2;
		*y = conf_switch_y[0];
	} else if (index == EAST) {
		*w = bg->width;
		*h = bg->height;
		*x = conf_window_width - *w - conf_news_margin;
		*y = conf_switch_y[0] + *h + conf_news_margin;
	} else if (index == WEST) {
		*w = bg->width;
		*h = bg->height;
		*x = conf_news_margin;
		*y = conf_switch_y[0] + *h + conf_news_margin;
	} else if (index == SOUTH) {
		*w = bg->width;
		*h = bg->height;
		*x = (conf_window_width - *w) / 2;
		*y = conf_switch_y[0] + (*h + conf_news_margin) * 2;
	} else {
		*w = switch_bg_image[0]->width;
		*h = switch_bg_image[0]->height;
		*x = conf_switch_x[0];
		*y = conf_switch_y[0] + (*h + conf_news_margin) * 3 +
			(switch_bg_image[0]->height + conf_switch_margin_y) * (index - SWITCH_BASE);
	}
}

/*
 * スイッチの非選択イメージを描画する
 */
void draw_switch_bg_image(struct image *target, int index)
{
	if (switch_bg_image[index] == NULL)
		index = 0;

	draw_image_copy(target, 0, 0, switch_bg_image[index],
			switch_bg_image[index]->width,
			switch_bg_image[index]->height,
			0, 0);
}

/*
 * スイッチの選択イメージを描画する
 */
void draw_switch_fg_image(struct image *target, int index)
{
	if (switch_fg_image[index] == NULL)
		index = 0;

	draw_image_copy(target, 0, 0, switch_fg_image[index],
			switch_fg_image[index]->width,
			switch_fg_image[index]->height,
			0, 0);
}

/*
 * NEWSの非選択イメージを描画する
 */
void draw_news_bg_image(struct image *target)
{
	struct image *img;

	img = news_bg_image != NULL ? news_bg_image : switch_bg_image[0];

	draw_image_copy(target, 0, 0, img, img->width, img->height, 0, 0);
}

/*
 * NEWSの選択イメージを描画する
 */
void draw_news_fg_image(struct image *target)
{
	struct image *img;

	img = news_fg_image != NULL ? news_fg_image : switch_fg_image[0];

	draw_image_copy(target, 0, 0, img, img->width, img->height, 0, 0);
}

/*
 * バナーの描画
 */

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
	struct image *base_img;
	int src_x, src_width;

	assert(layer >= 0 && layer < STAGE_LAYERS);

	/* 背景イメージは必ずセットされている必要がある */
	if (layer == LAYER_BG)
		assert(layer_image[LAYER_BG] != NULL);

	/* イメージがセットされていなければ描画しない */
	if (layer_image[layer] == NULL)
		return;

	/* クリックアニメーションが非表示のときは描画しない */
	if (layer == LAYER_CLICK && conf_click_disable)
		return;

	/* 目パチのフレームを計算する */
	if (layer == LAYER_CHB_EYE || layer == LAYER_CHL_EYE ||
	    layer == LAYER_CHLC_EYE || layer == LAYER_CHC_EYE ||
	    layer == LAYER_CHRC_EYE || layer == LAYER_CHR_EYE) {
		base_img = layer_image[chpos_to_layer(layer_to_chpos(layer))];
		if (base_img == NULL)
			return;
		src_width = base_img->width;
		src_x = src_width * layer_frame[layer];
	} else {
		src_width = layer_image[layer]->width;
		src_x = 0;
	}

	/* 3Dの場合 */
	if (layer_rotate[layer] != 0 ||
	    layer_scale_x[layer] != 1.0f ||
	    layer_scale_y[layer] != 1.0f) {
		float x1 = 0;
		float y1 = 0;
		float x2 = (float)layer_image[layer]->width - 1.0f;
		float y2 = 0;
		float x3 = 0;
		float y3 = (float)layer_image[layer]->height - 1.0f;;
		float x4 = (float)layer_image[layer]->width - 1.0f;
		float y4 = (float)layer_image[layer]->height - 1.0f;
		float center_x = (float)layer_center_x[layer];
		float center_y = (float)layer_center_y[layer];
		float rad = (float)layer_rotate[layer];

		/* 1. Shift for the centering. */
		x1 -= center_x;
		y1 -= center_y;
		x2 -= center_x;
		y2 -= center_y;
		x3 -= center_x;
		y3 -= center_y;
		x4 -= center_x;
		y4 -= center_y;

		/* 2. Scale. */
		x1 *= layer_scale_x[layer];
		y1 *= layer_scale_y[layer];
		x2 *= layer_scale_x[layer];
		y2 *= layer_scale_y[layer];
		x3 *= layer_scale_x[layer];
		y3 *= layer_scale_y[layer];
		x4 *= layer_scale_x[layer];
		y4 *= layer_scale_y[layer];

		/* 3. Rotate. */
		if (rad != 0) {
			float tmp_x, tmp_y;

			tmp_x = x1;
			tmp_y = y1;
			x1 = tmp_x * cosf(rad) - tmp_y * sinf(rad);
			y1 = tmp_x * sinf(rad) + tmp_y * cosf(rad);

			tmp_x = x2;
			tmp_y = y2;
			x2 = tmp_x * cosf(rad) - tmp_y * sinf(rad);
			y2 = tmp_x * sinf(rad) + tmp_y * cosf(rad);

			tmp_x = x3;
			tmp_y = y3;
			x3 = tmp_x * cosf(rad) - tmp_y * sinf(rad);
			y3 = tmp_x * sinf(rad) + tmp_y * cosf(rad);

			tmp_x = x4;
			tmp_y = y4;
			x4 = tmp_x * cosf(rad) - tmp_y * sinf(rad);
			y4 = tmp_x * sinf(rad) + tmp_y * cosf(rad);
		}

		/* 4. Shift again for the centering. */
		x1 += center_x;
		y1 += center_y;
		x2 += center_x;
		y2 += center_y;
		x3 += center_x;
		y3 += center_y;
		x4 += center_x;
		y4 += center_y;

		/* 5. Shift for the layer position. */
		x1 += (float)layer_x[layer];
		y1 += (float)layer_y[layer];
		x2 += (float)layer_x[layer];
		y2 += (float)layer_y[layer];
		x3 += (float)layer_x[layer];
		y3 += (float)layer_y[layer];
		x4 += (float)layer_x[layer];
		y4 += (float)layer_y[layer];

		/* Render. */
		switch (layer_blend[layer]) {
		case BLENDMODE_NORMAL:
			render_image_3d_normal(x1, y1, x2, y2, x3, y3, x4, y4,
					       layer_image[layer],
					       0, 0,
					       layer_image[layer]->width,
					       layer_image[layer]->height,
					       layer_alpha[layer]);
			break;
		case BLENDMODE_ADD:
			render_image_3d_add(x1, y1, x2, y2, x3, y3, x4, y4,
					    layer_image[layer],
					    0, 0,
					    layer_image[layer]->width,
					    layer_image[layer]->height,
					    layer_alpha[layer]);
			break;
		default:
			break;
		}
		return;
	}

	/* キャラクタレイヤを暗く描画する場合 */
	if (layer >= LAYER_CHB && layer <= LAYER_CHC &&
	    ch_dim[layer_to_chpos(layer)]) {
		render_image_dim(layer_x[layer],
				 layer_y[layer],
				 (int)((float)src_width * layer_scale_x[layer]),
				 (int)((float)layer_image[layer]->height * layer_scale_y[layer]),
				 layer_image[layer],
				 src_x,
				 0,
				 src_width,
				 layer_image[layer]->height,
				 layer_alpha[layer]);
		return;
	}

	/* それ以外の描画の場合 */
	render_image_normal(layer_x[layer],
			    layer_y[layer],
			    (int)((float)src_width * layer_scale_x[layer]),
			    (int)((float)layer_image[layer]->height * layer_scale_y[layer]),
			    layer_image[layer],
			    src_x,
			    0,
			    src_width,
			    layer_image[layer]->height,
			    layer_alpha[layer]);
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

	/* 背景レイヤの場合 */
	if (layer == LAYER_BG) {
		draw_image_copy(target,
				layer_x[layer],
				layer_y[layer],
				layer_image[layer],
				layer_image[layer]->width,
				layer_image[layer]->height,
				0, 0);
		return;
	}

	/* キャラクタレイヤを暗く描画する場合 */
	if (layer >= LAYER_CHB && layer <= LAYER_CHC &&
	    ch_dim[layer_to_chpos(layer)]) {
		draw_image_dim(target,
			       layer_x[layer],
			       layer_y[layer],
			       layer_image[layer],
			       layer_image[layer]->width,
			       layer_image[layer]->height,
			       0, 0,
			       layer_alpha[layer]);
		return;
	}

	/* 普通に描画する */
	draw_image_normal(target,
			  layer_x[layer],
			  layer_y[layer],
			  layer_image[layer],
			  layer_image[layer]->width,
			  layer_image[layer]->height,
			  0, 0,
			  layer_alpha[layer]);
}

/*
 * cmd_switch.c (TODO: remove)
 */

/*
 * Renders the entire FO image and a specified rectangle of the FI image to the screen.
 */
void render_fo_all_and_fi_rect(int x, int y, int w, int h)
{
	assert(stage_mode == STAGE_MODE_IDLE);

	/* 背景を描画する */
	render_image_normal(0,
			    0,
			    -1,
			    -1,
			    fo_image,
			    0,
			    0,
			    -1,
			    -1,
			    255);

	/* ボタンを描画する */
	render_image_normal(x,
			    y,
			    w,
			    h,
			    fi_image,
			    x,
			    y,
			    w,
			    h,
			    255);
}

/*
 * セーブスロットのNEW画像の描画
 */

/* Renders a NEW image of save slots. */
void render_savenew(int x, int y, int alpha)
{
	if (savenew_image == NULL)
		return;

	render_image_normal(x,
			    y,
			    savenew_image->width,
			    savenew_image->height,
			    savenew_image,
			    0,
			    0,
			    savenew_image->width,
			    savenew_image->height,
			    alpha);
}

/*
 * Kirakira Effect
 */

/*
 * Starts the Kirakira effect.
 */
void start_kirakira(int x, int y)
{
	int w, h;

	kirakira_x = x;
	kirakira_y = y;

	if (kirakira_image[0] != NULL) {
		w = kirakira_image[0]->width;
		h = kirakira_image[0]->height;
		kirakira_x -= w / 2;
		kirakira_y -= h / 2;
	}

	reset_lap_timer(&sw_kirakira);
}

/*
 * Renders a Kirakira effect frame.
 */
void render_kirakira(void)
{
	float lap, frame_time;
	int index;

	frame_time = conf_kirakira_frame == 0 ? 0.333f : conf_kirakira_frame;

	lap = (float)get_lap_timer_millisec(&sw_kirakira) / 1000.0f;
	index = (int)(lap / frame_time);
	if (index < 0 || index >= KIRAKIRA_FRAME_COUNT)
		return;
	if (kirakira_image[index] == NULL)
		return;

	if (conf_kirakira_on == 1) {
		render_image_normal(kirakira_x,
				    kirakira_y,
				    -1,
				    -1,
				    kirakira_image[index],
				    0,
				    0,
				    -1,
				    -1,
				    255);
	} else {
		render_image_add(kirakira_x,
				 kirakira_y,
				 -1,
				 -1,
				 kirakira_image[index],
				 0,
				 0,
				 -1,
				 -1,
				 255);
	}
}

/*
 * Text Layers
 */

/*
 * テキストレイヤのテキストを取得する
 */
const char *get_layer_text(int layer)
{
	assert(layer >= LAYER_TEXT1);
	assert(layer <= LAYER_TEXT8);

	return layer_text[layer];
}

/*
 * テキストレイヤのテキストを設定する
 */
bool set_layer_text(int layer, const char *msg)
{
	assert(layer >= LAYER_TEXT1);
	assert(layer <= LAYER_TEXT8);

	if (layer_text[layer] != NULL) {
		free(layer_text[layer]);
		layer_text[layer] = NULL;
	}

	if (msg != NULL && strcmp(msg, "") != 0) {
		layer_text[layer] = strdup(msg);
		if (layer_text[layer] == NULL) {
			log_memory();
			return false;
		}
	}

	return true;
}

/*
 * for debug
 */
#include <png.h>
#if defined(__GNUC__) && !defined(__llvm__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wclobbered"
#endif
void write_layers_to_files(void)
{
	char fname[128];
	png_structp png;
	png_infop info;
	FILE *png_fp;
	static png_bytep *row_pointers;
	int y, i;

	for (i = 0; i < STAGE_LAYERS; i++) {
		sprintf(fname, "debug-layer-%02d.png", i);
		remove(fname);

		if (layer_image[i] == NULL)
			continue;

		row_pointers = malloc(sizeof(png_bytep) * (size_t)layer_image[i]->height);
		if (row_pointers == NULL) {
			log_memory();
			return;
		}
		for (y = 0; y < layer_image[i]->height; y++)
			row_pointers[y] = (png_bytep)&layer_image[i]->pixels[layer_image[i]->width * y];

		/* PNGファイルをオープンする */
		png_fp = fopen(fname, "wb");
		if (png_fp == NULL) {
			log_file_open(fname);
			return;
		}

		/* PNGを書き出す */
		png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
		if (png == NULL) {
			log_api_error("png_create_write_struct");
			fclose(png_fp);
			return;
		}
		info = png_create_info_struct(png);
		if (info == NULL) {
			log_api_error("png_create_info_struct");
			png_destroy_write_struct(&png, NULL);
			return;
		}
		if (setjmp(png_jmpbuf(png))) {
			log_error("Failed to write png file.");
			png_destroy_write_struct(&png, &info);
			return;
		}

		png_init_io(png, png_fp);
		png_set_IHDR(png, info,
			     (png_uint_32)layer_image[i]->width,
			     (png_uint_32)layer_image[i]->height,
			     8, PNG_COLOR_TYPE_RGBA, PNG_INTERLACE_NONE,
			     PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
		png_write_info(png, info);
		png_write_image(png, row_pointers);
		png_write_end(png, NULL);
		png_destroy_write_struct(&png, &info);

		/* PNGファイルをクローズする */
		fclose(png_fp);

		free(row_pointers);
	}
}
#if defined(__GNUC__) && !defined(__llvm__)
#pragma GCC diagnostic pop
#endif
