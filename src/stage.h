/* -*- coding: utf-8; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Suika 2
 * Copyright (C) 2001-2023, TABATA Keiichi. All rights reserved.
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

	/* ステータスレイヤ */
	LAYER_TEXT1,
	LAYER_TEXT2,
	LAYER_TEXT3,
	LAYER_TEXT4,
	LAYER_TEXT5,
	LAYER_TEXT6,
	LAYER_TEXT7,
	LAYER_TEXT8,

	/* エフェクトレイヤ */
	LAYER_EFFECT1,
	LAYER_EFFECT2,
	LAYER_EFFECT3,
	LAYER_EFFECT4,

	/*
	 * フェードアウト用レイヤで、次の場合に有効:
	 *  - 背景/キャラフェード時
	 *  - switch.cの描画
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

	/* 総レイヤ数 */
	STAGE_LAYERS
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
	CH_BACK = 0,
	CH_LEFT = 1,
	CH_RIGHT = 2,
	CH_CENTER = 3,
	CH_BASIC_LAYERS = 4,
	CH_FACE = 4,
	CH_ALL_LAYERS = 5,
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
 * 初期化
 */

/* ステージの初期化処理をする */
bool init_stage(void);

/* ステージの終了処理を行う */
void cleanup_stage(void);

/* 起動・ロード直後の一時的な背景を作成する */
struct image *create_initial_bg(void);

/* ステージのリロードを行う */
bool reload_stage(void);

/*
 * 基本
 */

/* レイヤイメージを取得する */
struct image *get_layer_image(int layer);

/* レイヤイメージを取得する */
void set_layer_image(int layer, struct image *img);

/* レイヤーのX座標を取得する */
int get_layer_x(int layer);

/* レイヤーのY座標を取得する */
int get_layer_y(int layer);

/* レイヤーの座標を設定する */
void set_layer_position(int layer, int x, int y);

/* レイヤーのアルファ値を取得する */
int get_layer_alpha(int layer);

/* レイヤーのアルファ値を取得する */
void set_layer_alpha(int layer, int alpha);

/* レイヤーのファイル名を取得する */
const char *get_layer_file_name(int layer);

/* レイヤーのファイル名を設定する */
bool set_layer_file_name(int layer, const char *file_name);

/* キャラ位置からステージレイヤへ変換する */
int chpos_to_layer(int chpos);

/* キャラ位置からアニメレイヤへ変換する */
int chpos_to_anime_layer(int chpos);

/* ステージレイヤからキャラ位置へ変換する */
int layer_to_chpos(int chpos);

/* ステージレイヤからアニメレイヤに変換する */
int stage_layer_to_anime_layer(int stage_layer);

/* ステージをクリアする */
void clear_stage(void);

/*
 * ステージ描画
 */

/* ステージ全体を描画する */
void draw_stage(void);

/*
 * ステージ全体を描画する(GPU用)
 *  - GPU利用時はdraw_stage()を呼び出し、そうでなければ何もしない
 *  - 参考: CPU描画時は変更部分のみを描画、GPU描画時は毎フレーム再描画
 */
void draw_stage_keep(void);

/*
 * ステージの矩形を描画する
 *  - GPU利用時は画面全体が再描画される
 */
void draw_stage_rect(int x, int y, int w, int h);

/* ステージの背景(FO)全体と、前景(FI)の矩形を描画する */
void draw_fo_all_and_fi_rect(int x, int y, int w, int h);

/* ステージの背景(FO)全体と、前景(FI)の矩形を描画する(GPU用) */
void draw_fo_all_and_fi_rect_accelerated(int x, int y, int w, int h);

/* CPU描画の場合はFOのうち1矩形、GPU描画の場合はFO全体を描画する */
void draw_fo_rect_accelerated(int x, int y, int w, int h);

/* ステージの背景(FO)と前景(FI)にステージ全体を描画する */
void draw_stage_fo_fi(void);

/*
 * システムメニューの描画
 */

/* システムメニューを描画する */
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
			int *x, int *y, int *w, int *h);

/* システムメニューの座標を取得する */
void get_sysmenu_rect(int *x, int *y, int *w, int *h);

/* 折りたたみシステムメニューを描画する */
void draw_stage_collapsed_sysmenu(bool is_pointed, int *x, int *y, int *w,
				  int *h);

/* 折りたたみシステムメニューの座標を取得する */
void get_collapsed_sysmenu_rect(int *x, int *y, int *w, int *h);

/*
 * セーブデータ用サムネイルの描画
 */

/* セーブデータ用サムネイル画像にステージ全体を描画する */
void draw_stage_to_thumb(void);

/* セーブデータ用サムネイル画像にFO全体を描画する */
void draw_stage_fo_thumb(void);

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
void draw_fade(void);

/* フェードを終了する */
void finish_fade(void);

/*
 * キャラの変更
 */

/* キャラを暗くするかを設定する */
void set_ch_dim(int pos, bool dim);

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

/* スイッチの親選択肢の矩形を取得する */
void get_switch_rect(int index, int *x, int *y, int *w, int *h);

/* FOレイヤにスイッチの非選択イメージを描画する */
void draw_switch_bg_image(int x, int y);

/* FIレイヤにスイッチの選択イメージを描画する */
void draw_switch_fg_image(int x, int y);

/* NEWSの親選択肢の矩形を取得する */
void get_news_rect(int index, int *x, int *y, int *w, int *h);

/* FIレイヤにNEWSの非選択イメージを描画する */
void draw_news_bg_image(int x, int y);

/* FIレイヤにNEWSの選択イメージを描画する */
void draw_news_fg_image(int x, int y);

/* FO/FIの2レイヤに画像を描画する */
void draw_image_on_fo_fi(int x, int y, struct image *img);

/*
 * 文字描画
 */

/* 文字を描画する前にレイヤをロックする */
void lock_layers_for_msgdraw(int layer, int additional_layer);

/* 文字を描画した後にレイヤをアンロックする */
void unlock_layers_for_msgdraw(int layer, int additional_layer);

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
 * GUI
 */

/* GUIの画像を削除する */
void remove_gui_images(void);

/* GUIのidle画像を読み込む */
bool load_gui_idle_image(const char *file);

/* GUIのhover画像を読み込む */
bool load_gui_hover_image(const char *file);

/* GUIのactive画像を読み込む */
bool load_gui_active_image(const char *file);

/* GUIのイメージがすべて揃っているか調べる */
bool check_stage_gui_images(void);

/* GUIのidle画像を描画する */
void draw_stage_gui_idle(void);

/* GUIのhover画像を描画する */
void draw_stage_gui_hover(int x, int y, int w, int h);

/* GUIのactive画像を描画する */
void draw_stage_gui_active(int x, int y, int w, int h, int sx, int sy);

/* GUIのidle画像の内容を仮のBGレイヤに設定する */
bool create_temporary_bg_for_gui(void);

/*
 * キラキラエフェクト
 */

/* キラキラエフェクトを開始する */
void start_kirakira(int x, int y);

/* キラキラエフェクトを描画する */
void draw_kirakira(void);

/*
 * テキストレイヤ
 */

/* テキストレイヤのテキストを取得する */
const char *get_layer_text(int text_layer_index);

/* テキストレイヤのテキストを設定する */
bool set_layer_text(int text_layer_index, const char *msg);

/*
 * 更新領域の計算
 */

/* 2つの矩形を囲う矩形を求める */
void union_rect(int *x, int *y, int *w, int *h, int x1, int y1, int w1, int h1,
		int x2, int y2, int w2, int h2);

/*
 * コンフィグの動的な変更
 */

/* メッセージボックスと名前ボックスを更新する */
bool update_msgbox_and_namebox(void);

#endif
