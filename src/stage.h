/* -*- coding: utf-8-with-signature; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Suika 2
 * Copyright (C) 2001-2021, TABATA Keiichi. All rights reserved.
 */

/*
 * [Changes]
 *  - 2016-06-14 作成
 *  - 2017-08-14 スイッチに対応
 *  - 2021-06-05 背景フェードの追加
 *  - 2021-06-10 マスクつき描画の対応
 *  - 2021-06-10 キャラクタのアルファ値に対応
 *  - 2021-06-12 画面揺らしモードに対応
 *  - 2021-07-25 エフェクトを増やした Add effects.
 */

#ifndef SUIKA_STAGE_H
#define SUIKA_STAGE_H

#include "image.h"

/* キャラクタの立ち位置 */
enum ch_position {
	CH_BACK,
	CH_LEFT,
	CH_RIGHT,
	CH_CENTER,
	CH_LAYERS
};

/* フェードメソッド */
enum fade_method {
	FADE_METHOD_INVALID,
	FADE_METHOD_NORMAL,
	FADE_METHOD_MASK,
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
};

/*
 * 初期化
 */

/* ステージの初期化処理をする */
bool init_stage(void);

/* ステージの終了処理を行う */
void cleanup_stage(void);

/*
 * ステージ描画
 */

/* ステージ全体を描画する */
void draw_stage(void);

/* ステージの矩形を描画する */
void draw_stage_rect(int x, int y, int w, int h);

/* 背景フェードモードが有効な際のステージ描画を行う */
void draw_stage_bg_fade(int fade_method);

/* キャラフェードモードが有効な際のステージ描画を行う */
void draw_stage_ch_fade(int fade_method);

/* 画面揺らしモードが有効な際のステージ描画を行う */
void draw_stage_shake(void);

/* ステージの背景(FO)全体と、前景(FI)のうち2矩形を描画する */
void draw_stage_with_buttons(int x1, int y1, int w1, int h1, int x2, int y2,
			     int w2, int h2);

/* ステージの背景(FO)のうち1矩形と、前景(FI)のうち1矩形を描画する */
void draw_stage_rect_with_buttons(int old_x1, int old_y1, int old_w1,
				  int old_h1, int new_x2, int new_y2,
				  int new_w2, int new_h2);

/* ステージの指定された矩形とスイッチを描画する */
void draw_stage_rect_with_switch(int x, int y, int w, int h);

/* ステージの背景(FO)と前景(FI)を描画する */
void draw_stage_history(void);

/*
 * 背景の変更
 */

/* 文字列からフェードメソッドを取得する */
int get_fade_method(const char *method);

/* 背景のファイル名を設定する */
bool set_bg_file_name(const char *file);

/* 背景のファイル名を取得する */
const char *get_bg_file_name(void);

/* 背景をフェードせずにただちに切り替える */
void change_bg_immediately(struct image *img);

/* 背景フェードモードを開始する */
void start_bg_fade(struct image *img);

/* 背景フェードモードの進捗率を設定する */
void set_bg_fade_progress(float progress);

/* 背景フェードモードを終了する */
void stop_bg_fade(void);

/*
 * キャラの変更
 */

/* キャラファイル名を設定する */
bool set_ch_file_name(int pos, const char *file);

/* キャラのファイル名を取得する */
const char *get_ch_file_name(int pos);

/* キャラの座標を取得する */
void get_ch_position(int pos, int *x, int *y);

/* キャラのアルファ値を取得する */
int get_ch_alpha(int pos);

/* キャラをフェードせずにただちに切り替える */
void change_ch_immediately(int pos, struct image *img, int x, int y,
			   int alpha);

/* キャラの位置とアルファを設定する */
void change_ch_attributes(int pos, int x, int y, int alpha);

/* キャラフェードモードを開始する */
void start_ch_fade(int pos, struct image *img, int x, int y, int alpha);

/* キャラフェードモードの進捗率を設定する */
void set_ch_fade_progress(float progress);

/* キャラフェードモードを終了する */
void stop_ch_fade(void);

/* キャラフェードモード(複数,背景も)を開始する */
void start_ch_fade_multi(const bool *stay, struct image **img, const int *x,
			 const int *y);

/*
 * キャラアニメ
 */

/* キャラアニメを開始する */
void start_ch_anime(int pos, int to_x, int to_y, int to_alpha);

/* キャラアニメモードの進捗率を設定する */
void set_ch_anime_progress(float progress);

/* キャラアニメモードを終了する */
void stop_ch_anime(void);

/*
 * 画面揺らしモード
 */

/* 画面揺らしモードを開始する */
void start_shake(void);

/* 画面揺らしモードのオフセットを設定する */
void set_shake_offset(int x, int y);

/* 画面揺らしモードを終了する */
void stop_shake(void);

/*
 * 名前ボックスの描画
 */

/* 名前ボックスの矩形を取得する */
void get_namebox_rect(int *x, int *y, int *w, int *h);

/* 名前ボックスをクリアする */
void clear_namebox(void);

/* 名前ボックスの表示・非表示を設定する */
void show_namebox(bool show);

/* 名前ボックスに文字を描画する */
int draw_char_on_namebox(int x, int y, uint32_t wc, pixel_t color,
			 pixel_t outline_color);

/*
 * メッセージボックスの描画
 */

/* メッセージボックスの矩形を取得する */
void get_msgbox_rect(int *x, int *y, int *w, int *h);

/* メッセージボックスを背景でクリアする */
void clear_msgbox(void);

/* メッセージボックスの矩形を背景でクリアする */
void clear_msgbox_rect_with_bg(int x, int y, int w, int h);

/* メッセージボックスの矩形を前景でクリアする */
void clear_msgbox_rect_with_fg(int x, int y, int w, int h);

/* メッセージボックスの表示・非表示を設定する */
void show_msgbox(bool show);

/* メッセージボックスに文字を描画する */
int draw_char_on_msgbox(int x, int y, uint32_t wc, pixel_t color,
			pixel_t outline_color);

/*
 * クリックアニメーションの描画
 */

/* クリックアニメーションの矩形を取得する */
void get_click_rect(int *x, int *y, int *w, int *h);

/* クリックアニメーションの表示・非表示を設定する */
void show_click(bool show);

/*
 * 選択肢ボックスの描画
 */

/* 選択肢ボックスの矩形を取得する */
void get_selbox_rect(int *x, int *y, int *w, int *h);

/* 選択肢ボックスをクリアする */
void clear_selbox(int fg_x, int fg_y, int fg_w, int fg_h);

/* 選択肢ボックスの表示・非表示を設定する */
void show_selbox(bool show);

/* 選択肢ボックスに文字を描画する */
int draw_char_on_selbox(int x, int y, uint32_t wc);

/*
 * スイッチの描画
 */

/* スイッチの親選択肢の矩形を取得する */
void get_switch_rect(int index, int *x, int *y, int *w, int *h);

/* FIレイヤにスイッチの非選択イメージを描画する */
void draw_switch_bg_image(int x, int y);

/* FIレイヤにスイッチの選択イメージを描画する */
void draw_switch_fg_image(int x, int y);

/*
 * NEWSの描画
 */

/* NEWSの親選択肢の矩形を取得する */
void get_news_rect(int index, int *x, int *y, int *w, int *h);

/* FIレイヤにNEWSの非選択イメージを描画する */
void draw_news_bg_image(int x, int y);

/* FIレイヤにNEWSの選択イメージを描画する */
void draw_news_fg_image(int x, int y);

/*
 * セーブ画面の描画
 */

/* セーブ画面用にFI/FOレイヤをクリアする */
void clear_save_stage(void);

/* FO/FIの2レイヤに文字を描画する */
int draw_char_on_fo_fi(int x, int y, uint32_t wc);

/*
 * メニュー画面・CG回想画面の描画
 */

/* FOレイヤにイメージを描画する */
void draw_image_to_fo(struct image *img);

/* FIレイヤにイメージを描画する */
void draw_image_to_fi(struct image *img);

/* FOレイヤに矩形を描画する */
void draw_rect_to_fo(int x, int y, int w, int h, pixel_t color);

/*
 * ヒストリ画面の表示
 */

/* FOレイヤにステージを描画する */
void draw_history_fo(void);

/* FIレイヤを色で塗り潰す */
void draw_history_fi(pixel_t color);

/* FIレイヤに文字を描画する */
void draw_char_on_fi(int x, int y, uint32_t wc, int *w, int *h);

/*
 * 更新領域の計算
 */

/* 2つの矩形を囲う矩形を求める */
void union_rect(int *x, int *y, int *w, int *h, int x1, int y1, int w1, int h1,
		int x2, int y2, int w2, int h2);

#endif
