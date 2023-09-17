/* -*- coding: utf-8; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Suika 2
 * Copyright (C) 2001-2022, TABATA Keiichi. All rights reserved.
 */

/*
 * ヒストリ画面の実行
 *
 * [Changes]
 *  - 2016/07/09 作成
 *  - 2022/08/08 GUIに機能を移管
 */

#include "suika.h"

/* テキストのサイズ(名前とメッセージを連結するため) */
#define TEXT_SIZE	(1024)

/* 表示する履歴の数 */
#define HISTORY_SIZE	(100)

/* ヒストリ項目 */
static struct history {
	char *text;
	char *voice;
	int y_top;
	int y_bottom;
} history[HISTORY_SIZE];

/* ヒストリ項目の個数 */
static int history_count;

/* ヒストリ項目の先頭 */
static int history_index;

/* 一時領域 */
static char tmp_text[TEXT_SIZE];

/*
 * 初期化
 */

/*
 * ヒストリに関する初期化処理を行う
 */
bool init_history(void)
{
	/* Android NDK用に初期化を行う */
	clear_history();

	return true;
}

/*
 * ヒストリに関する終了処理を行う
 */
void cleanup_history(void)
{
	clear_history();
}

/*
 * cmd_message.cからのメッセージ登録
 */

/*
 * メッセージを登録する
 */
bool register_message(const char *name, const char *msg, const char *voice,
		      pixel_t body_color, pixel_t body_outline_color,
		      pixel_t name_color, pixel_t name_outline_color)
{
	struct history *h;

	/* 格納位置を求める */
	h = &history[history_index];

	/* 以前の情報を消去する */
	if (h->text != NULL) {
		free(h->text);
		h->text = NULL;
	}
	if (h->voice != NULL) {
		free(h->voice);
		h->voice = NULL;
	}

	/* ボイスが指定されている場合 */
	if (voice != NULL && strcmp(voice, "") != 0) {
		h->voice = strdup(voice);
		if (h->voice == NULL) {
			log_memory();
			return false;
		}
	}

	/* 色のアルファ値をゼロにする */
	body_color &= 0xffffff;
	body_outline_color &= 0xffffff;
	name_color &= 0xffffff;
	name_outline_color &= 0xffffff;

	/* ヒストリの色を使う場合 */
	if (conf_gui_history_disable_color == 2) {
		body_color = make_pixel_slow(0,
					     (pixel_t)conf_gui_history_font_color_r,
					     (pixel_t)conf_gui_history_font_color_g,
					     (pixel_t)conf_gui_history_font_color_b);
		body_outline_color = make_pixel_slow(0,
					     (pixel_t)conf_gui_history_font_outline_color_r,
					     (pixel_t)conf_gui_history_font_outline_color_g,
					     (pixel_t)conf_gui_history_font_outline_color_b);
		name_color = body_color;
		name_outline_color = body_outline_color;
	}

	/* 名前が指定されいる場合 */
	if (name != NULL) {
		/* "名前「メッセージ」"の形式に連結して保存する */
		if (conf_locale == LOCALE_JA || conf_serif_quote) {
			/* 日本語 */
			if (!is_quoted_serif(msg)) {
				/* カッコがない場合 */
				if (!conf_msgbox_tategaki) {
					snprintf(tmp_text, TEXT_SIZE,
						 "\\#{%06x}%s"
						 U8("\\#{%06x}「%s」"),
						 name_color, name,
						 body_color, msg);
				} else {
					snprintf(tmp_text, TEXT_SIZE,
						 "\\#{%06x}%s"
						 U8("\\#{%06x}﹁%s﹂"),
						 name_color, name,
						 body_color, msg);
				}
			} else {
				/* すでにカッコがある場合 */
				snprintf(tmp_text, TEXT_SIZE,
					 U8("\\#{%06x}%s\\#{%06x}%s"),
					 name_color, name, body_color, msg);
			}
		} else {
			/* 日本語以外 */
			snprintf(tmp_text, TEXT_SIZE,
				 "\\#{%06x}%s\\#{%06x}: %s",
				 name_color, name, body_color, msg);
		}
		h->text = strdup(tmp_text);
		if (h->text == NULL) {
			log_memory();
			return false;
		}
	} else {
		/* メッセージのみを保存する */
		h->text = strdup(msg);
		if (h->text == NULL) {
			log_memory();
			return false;
		}
	}

	/* 格納位置を更新する */
	history_index = (history_index + 1) % HISTORY_SIZE;
	history_count = (history_count + 1) >= HISTORY_SIZE ? HISTORY_SIZE :
			(history_count + 1);

	UNUSED_PARAMETER(body_outline_color);
	UNUSED_PARAMETER(name_outline_color);

    return true;
}

/*
 * ロード時のクリア
 */

/*
 * ヒストリをクリアする
 */
void clear_history(void)
{
	int i;

	for (i = 0; i < HISTORY_SIZE; i++) {
		if (history[i].text != NULL) {
			free(history[i].text);
			history[i].text = NULL;
		}

		if (history[i].voice != NULL) {
			free(history[i].voice);
			history[i].voice = NULL;
		}
	}

	history_count = 0;
	history_index = 0;
}

/*
 * 取得
 */

/*
 * ヒストリの数を取得する
 */
int get_history_count(void)
{
	return history_count;
}

/*
 * ヒストリのメッセージを取得する
 */
const char *get_history_message(int offset)
{
	int index;

	assert(offset >= 0);
	assert(offset < history_count);

	if (history_index - offset - 1 < 0)
		index = HISTORY_SIZE + (history_index - offset - 1);
	else
		index = history_index - offset - 1;
	assert(index >= 0 && index < history_count);

	return history[index].text;
}

/*
 * ヒストリのボイスを取得する
 */
const char *get_history_voice(int offset)
{
	int index;

	assert(offset >= 0);
	assert(offset < history_count);

	if (history_index - 1 - offset < 0)
		index = HISTORY_SIZE + (history_index - offset - 1);
	else
		index = history_index - offset - 1;
	assert(index >= 0 && index < history_count);

	return history[index].voice;
}

/*
 * セリフがカッコで始まりカッコで終わるかチェックする
 */
bool is_quoted_serif(const char *msg)
{
	struct item {
		const char *prefix;
		const char *suffix;
	} items[] = {
		{U8("（"), U8("）")},
		{U8("「"), U8("」")},
		{U8("『"), U8("』")},
		{U8("『"), U8("』")},
		{U8("︵"), U8("︶")},
		{U8("﹁"), U8("﹂")},
		{U8("﹃"), U8("﹄")}
	};

	size_t i;

	for (i = 0; i < sizeof(items) / sizeof(struct item); i++) {
		if (strncmp(msg, items[i].prefix, strlen(items[i].prefix)) == 0 &&
		    strncmp(msg + strlen(msg) - strlen(items[i].suffix), items[i].suffix,
		    strlen(items[i].suffix)) == 0)
			return true;
	}

	return false;
}
