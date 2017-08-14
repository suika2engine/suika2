/* -*- coding: utf-8-with-signature; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Suika 2
 * Copyright (C) 2001-2017, TABATA Keiichi. All rights reserved.
 */

/*
 * [Changes]
 *  - 2016-06-14 作成
 *  - 2017-08-14 スイッチ対応
 */

#ifndef SUIKA_STAGE_H
#define SUIKA_STAGE_H

#include "image.h"

enum ch_position {
	CH_BACK,
	CH_LEFT,
	CH_RIGHT,
	CH_CENTER,
	CH_LAYERS
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
void draw_stage_bg_fade(bool is_curtain);

/* キャラフェードモードが有効な際のステージ描画を行う */
void draw_stage_ch_fade(void);

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

/* キャラをフェードせずにただちに切り替える */
void change_ch_immediately(int pos, struct image *img, int x, int y);

/* キャラフェードモードを開始する */
void start_ch_fade(int layer, struct image *img, int x, int y);

/* キャラフェードモードの進捗率を設定する */
void set_ch_fade_progress(float progress);

/* キャラフェードモードを終了する */
void stop_ch_fade(void);

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
int draw_char_on_namebox(int x, int y, uint32_t wc);

/*
 * メッセージボックスの描画
 */

/* メッセージボックスの矩形を取得する */
void get_msgbox_rect(int *x, int *y, int *w, int *h);

/* メッセージボックスをクリアする */
void clear_msgbox(void);

/* メッセージボックスの表示・非表示を設定する */
void show_msgbox(bool show);

/* メッセージボックスに文字を描画する */
int draw_char_on_msgbox(int x, int y, uint32_t wc);

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
