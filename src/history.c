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
bool register_message(const char *name, const char *msg, const char *voice)
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

	/* 名前が指定されいる場合 */
	if (name != NULL) {
		/* "名前「メッセージ」"の形式に連結して保存する */
		if (conf_locale == LOCALE_JA) {
			snprintf(tmp_text, TEXT_SIZE, U8("%s「%s」"),
				 name, msg);
		} else {
			snprintf(tmp_text, TEXT_SIZE, "%s: %s", name, msg);
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
