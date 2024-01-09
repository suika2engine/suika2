/* -*- coding: utf-8; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Suika2
 * Copyright (C) 2001-2023, Keiichi Tabata. All rights reserved.
 */

/*
 * [Changes]
 *  - 2016-06-14 作成
 *  - 2017-08-14 スイッチに対応
 *  - 2021-06-05 背景フェードの追加
 *  - 2021-06-10 マスクつき描画の対応
 *  - 2021-06-10 キャラクタのアルファ値に対応
 *  - 2021-06-12 画面揺らしモードに対応
 *  - 2021-07-25 エフェクトを追加
 *  - 2022-07-16 システムメニューを追加
 *  - 2022-10-20 キャラ顔絵を追加
 *  - 2023-09-03 キラキラエフェクトを追加
 *  - 2023-09-14 テキスト・エフェクトレイヤを追加、描画処理を整理
 *  - 2023-10-10 エフェクトレイヤを移動/追加
 *  - 2023-12-16 スケーリング対応
 */

/*
 * draw_*(): draw image(s) to an image. (CPU memory to CPU memory)
 * render_*(): draw image(s) to the screen. (VRAM to VRAM)
 */

#ifndef SUIKA_STAGE_H
#define SUIKA_STAGE_H

#include "image.h"

/*
 * ステージのレイヤ
 */
enum {
	/* 背景レイヤ */
	LAYER_BG,

	/* 背景2レイヤ */
	LAYER_BG2,

	/* エフェクトレイヤ */
	LAYER_EFFECT5,
	LAYER_EFFECT6,
	LAYER_EFFECT7,
	LAYER_EFFECT8,

	/* キャラクタレイヤ(顔以外) */
	LAYER_CHB,
	LAYER_CHL,
	LAYER_CHLC,
	LAYER_CHR,
	LAYER_CHRC,
	LAYER_CHC,

	/* エフェクトレイヤ */
	LAYER_EFFECT1,
	LAYER_EFFECT2,
	LAYER_EFFECT3,
	LAYER_EFFECT4,

	/* メッセージレイヤ */
	LAYER_MSG,	/* 特殊: ユーザがロードできない */

	/* 名前レイヤ */
	LAYER_NAME,	/* 特殊: ユーザがロードできない */

	/* キャラクタレイヤ(顔) */
	LAYER_CHF,

	/* クリックアニメーション */
	LAYER_CLICK,	/* 特殊: click_image[i]への参照 */

	/* オートモードバナー */
	LAYER_AUTO,

	/* スキップモードバナー */
	LAYER_SKIP,

	/* ステータスレイヤ */
	LAYER_TEXT1,
	LAYER_TEXT2,
	LAYER_TEXT3,
	LAYER_TEXT4,
	LAYER_TEXT5,
	LAYER_TEXT6,
	LAYER_TEXT7,
	LAYER_TEXT8,

	/* 総レイヤ数 */
	STAGE_LAYERS,
};

/*
 * テキストレイヤの数
 */
#define TEXT_LAYERS		(8)

/*
 * エフェクトレイヤの数
 */
#define EFFECT_LAYERS		(4)

/*
 * クリック待ちアニメーションのフレーム数
 *  - クリック待ちプロンプト
 *  - 最大16フレームの可変長
 */
#define CLICK_FRAMES		(16)

/*
 * キャラクタの位置
 */
enum ch_position {
	CH_BACK,
	CH_LEFT,
	CH_LEFT_CENTER,
	CH_RIGHT,
	CH_RIGHT_CENTER,
	CH_CENTER,
	CH_FACE,
	CH_BASIC_LAYERS = 6,
	CH_ALL_LAYERS = 7,
};

/*
 * フェードメソッド
 */
enum fade_method {
	FADE_METHOD_INVALID,
	FADE_METHOD_NORMAL,
	FADE_METHOD_CURTAIN_RIGHT,
	FADE_METHOD_CURTAIN_LEFT,
	FADE_METHOD_CURTAIN_UP,
	FADE_METHOD_CURTAIN_DOWN,
	FADE_METHOD_SLIDE_RIGHT,
	FADE_METHOD_SLIDE_LEFT,
	FADE_METHOD_SLIDE_UP,
	FADE_METHOD_SLIDE_DOWN,
	FADE_METHOD_SHUTTER_RIGHT,
	FADE_METHOD_SHUTTER_LEFT,
	FADE_METHOD_SHUTTER_UP,
	FADE_METHOD_SHUTTER_DOWN,
	FADE_METHOD_CLOCKWISE,
	FADE_METHOD_COUNTERCLOCKWISE,
	FADE_METHOD_CLOCKWISE20,
	FADE_METHOD_COUNTERCLOCKWISE20,
	FADE_METHOD_CLOCKWISE30,
	FADE_METHOD_COUNTERCLOCKWISE30,
	FADE_METHOD_EYE_OPEN,
	FADE_METHOD_EYE_CLOSE,
	FADE_METHOD_EYE_OPEN_V,
	FADE_METHOD_EYE_CLOSE_V,
	FADE_METHOD_SLIT_OPEN,
	FADE_METHOD_SLIT_CLOSE,
	FADE_METHOD_SLIT_OPEN_V,
	FADE_METHOD_SLIT_CLOSE_V,
	FADE_METHOD_RULE,
	FADE_METHOD_MELT,
};

/*
 * システムメニューのボタンのインデックス
 */
enum {
	SYSMENU_NONE = -1,
        SYSMENU_QSAVE = 0,
	SYSMENU_QLOAD = 1,
	SYSMENU_SAVE = 2,
	SYSMENU_LOAD = 3,
	SYSMENU_AUTO = 4,
	SYSMENU_SKIP = 5,
	SYSMENU_HISTORY = 6,
	SYSMENU_CONFIG = 7,
	SYSMENU_CUSTOM1 = 8,
	SYSMENU_CUSTOM2 = 9,
	SYSMENU_COUNT = 10,
};

/*
 * 初期化
 */

/* ステージの初期化処理をする */
bool init_stage(void);

/* ステージのリロードを行う */
bool reload_stage(void);

/* 起動・ロード直後の一時的な背景を作成する */
struct image *create_initial_bg(void);

/* メッセージボックスと名前ボックスを更新する */
bool update_msgbox_and_namebox(void);

/* ステージの終了処理を行う */
void cleanup_stage(void);

/*
 * Basic Functionality
 */

/* Gets a layer x position. */
int get_layer_x(int layer);

/* Gets a layer y position. */
int get_layer_y(int layer);

/* Sets a layer position. */
void set_layer_position(int layer, int x, int y);

/* Sets a layer scale. */
void set_layer_scale(int layer, float scale_x, float scale_y);

/* Gets a layer image width. */
int get_layer_width(int layer);

/* Gets a layer image height. */
int get_layer_height(int layer);

/* Get a layer alpha. */
int get_layer_alpha(int layer);

/* Sets a layer alpha. */
void set_layer_alpha(int layer, int alpha);

/* Gets a layer file name. */
const char *get_layer_file_name(int layer);

/* Sets a layer file name. */
bool set_layer_file_name(int layer, const char *file_name);

/* Gets a layer image for a glyph drawing. */
struct image *get_layer_image(int layer);

/* Sets a layer image for a load.*/
void set_layer_image(int layer, struct image *img);

/* Clear basic layers. */
void clear_stage_basic(void);

/* Clear the stage and make it initial state. */
void clear_stage(void);

/*
 * Conversion of Layer Index and Character Position
 */

/* Convert a character position to a stage layer index. */
int chpos_to_layer(int chpos);

/* Convert a stage layer index to a character position. */
int layer_to_chpos(int chpos);

/*
 * Stage Rendering
 */

/* Renders the stage with all stage layers. */
void render_stage(void);

/*
 * cmd_switch.c
 */

/*  Renders the entire FO image and a specified rectangle of the FI image to the screen. (TODO: remove) */
void render_fo_all_and_fi_rect(int x, int y, int w, int h);

/*
 * System Menu Rendering and Pointing Check
 */

/* Renders the system menu to the screen. */
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
		    bool is_custom2_selected);

/* 折りたたみシステムメニューを描画する */
void render_collapsed_sysmenu(bool is_pointed);

/* Returns the pointed system menu item. */
int get_pointed_sysmenu_item(void);

/* Checks if the collapsed system menu is pointed. */
bool is_collapsed_sysmenu_pointed(void);

/*
 * セーブデータ用サムネイルの描画
 */

/* セーブデータ用サムネイル画像にステージ全体を描画する */
void draw_stage_to_thumb(void);

/* セーブデータ用サムネイル画像にswitchの画像を描画する */
void draw_switch_to_thumb(struct image *img, int x, int y);

/* セーブデータ用サムネイル画像を取得する */
struct image *get_thumb_image(void);

/*
 * フェード
 */

/* 文字列からフェードメソッドを取得する */
int get_fade_method(const char *method);

/* bg用のフェードを開始する */
bool start_fade_for_bg(const char *fname, struct image *img, int x, int y,
		       int alpha, int method, struct image *rule_img);

/* ch用のフェードを開始する*/
bool start_fade_for_ch(int chpos, const char *fname, struct image *img,
		       int x, int y, int alpha, int method,
		       struct image *rule_img);

/* chs用のフェードモードを開始する */
bool start_fade_for_chs(const bool *stay, const char **fname,
			struct image **img, const int *x, const int *y,
			const int *alpha, int method, struct image *rule_img);

/* shake用のフェードモードを開始する */
void start_fade_for_shake(void);

/* フェードの進捗率を設定する */
void set_fade_progress(float progress);

/* shakeの表示オフセットを設定する */
void set_shake_offset(int x, int y);

/* フェードの描画を行う */
void render_fade(void);

/* フェードを終了する */
void finish_fade(void);

/*
 * キャラの変更
 */

/* キャラ位置にキャラ番号を指定する */
void set_ch_name_mapping(int chpos, int ch_name_index);

/* 発話キャラを設定する */
void set_ch_talking(int ch_name_index);

/* キャラの自動明暗を発話キャラを元に更新する */
void update_ch_dim_by_talking_ch(void);

/* キャラの明暗を手動で設定する */
void force_ch_dim(int chpos, bool is_dim);

/*
 * 名前ボックスの描画
 */

/* 名前ボックスの矩形を取得する */
void get_namebox_rect(int *x, int *y, int *w, int *h);

/* 名前ボックスを名前ボックス画像で埋める */
void fill_namebox(void);

/* 名前ボックスの表示・非表示を設定する */
void show_namebox(bool show);

/*
 * メッセージボックスの描画
 */

/* メッセージボックスの矩形を取得する */
void get_msgbox_rect(int *x, int *y, int *w, int *h);

/* メッセージボックスの背景を描画する */
void fill_msgbox(void);

/* メッセージボックスの背景の矩形を描画する */
void fill_msgbox_rect_with_bg(int x, int y, int w, int h);

/* メッセージボックスの前景の矩形を描画する */
void fill_msgbox_rect_with_fg(int x, int y, int w, int h);

/* メッセージボックスの表示・非表示を設定する */
void show_msgbox(bool show);

/*
 * クリックアニメーションの描画
 */

/* クリックアニメーションの矩形を取得する */
void get_click_rect(int *x, int *y, int *w, int *h);

/* クリックアニメーションの位置を設定する */
void set_click_position(int x, int y);

/* クリックアニメーションの表示・非表示を設定する */
void show_click(bool show);

/* クリックアニメーションのフレーム番号を指定する */
void set_click_index(int index);

/*
 * スイッチ(@choose, @select, @switch, @news)の描画
 */

/* スイッチの矩形を取得する */
void get_switch_rect(int index, int *x, int *y, int *w, int *h);

/* NEWSの矩形を取得する */
void get_news_rect(int index, int *x, int *y, int *w, int *h);

/* スイッチの非選択イメージを描画する */
void draw_switch_bg_image(struct image *target, int index);

/* スイッチの選択イメージを描画する */
void draw_switch_fg_image(struct image *target, int index);

/* NEWSの非選択イメージを描画する */
void draw_news_bg_image(struct image *target);

/* NEWSの選択イメージを描画する */
void draw_news_fg_image(struct image *target);

/*
 * 文字描画
 */

/* レイヤに文字を描画する */
bool draw_char_on_layer(int layer, int x, int y, uint32_t wc, pixel_t color,
			pixel_t outline_color, int base_font_size,
			bool is_dimming, int *ret_width, int *ret_height,
			int *union_x, int *union_y, int *union_w, int *union_h);

/*
 * バナーの描画
 */

/* オートモードバナーの矩形を取得する */
void get_automode_banner_rect(int *x, int *y, int *w, int *h);

/* スキップモードバナーの矩形を取得する */
void get_skipmode_banner_rect(int *x, int *y, int *w, int *h);

/* オートモードバナーの表示・非表示を設定する */
void show_automode_banner(bool show);

/* スキップモードバナーの表示・非表示を設定する */
void show_skipmode_banner(bool show);

/*
 * キラキラエフェクト
 */

/* キラキラエフェクトを開始する */
void start_kirakira(int x, int y);

/* キラキラエフェクトを描画する */
void render_kirakira(void);

/*
 * Text Layers
 */

/* テキストレイヤのテキストを取得する */
const char *get_layer_text(int text_layer_index);

/* テキストレイヤのテキストを設定する */
bool set_layer_text(int text_layer_index, const char *msg);

/*
 * Debug
 */
void write_layers_to_files(void);

#endif
