/* -*- coding: utf-8-with-signature; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Suika 2
 * Copyright (C) 2001-2016, TABATA Keiichi. All rights reserved.
 */

/*
 * [Changes]
 *  - 2016/06/25 作成
 */

#include "suika.h"

/*
 * ウィンドウの設定
 */
char *conf_window_title;
int conf_window_width;
int conf_window_height;
bool conf_window_white;

/*
 * フォントの設定
 */
char *conf_font_file;
int conf_font_size;
int conf_font_color_r;
int conf_font_color_g;
int conf_font_color_b;

/*
 * 名前ボックスの設定
 */
char *conf_namebox_file;
int conf_namebox_x;
int conf_namebox_y;
int conf_namebox_margin_left;
int conf_namebox_margin_top;

/*
 * メッセージボックスの設定
 */
char *conf_msgbox_file;
int conf_msgbox_x;
int conf_msgbox_y;
int conf_msgbox_margin_left;
int conf_msgbox_margin_top;
int conf_msgbox_margin_right;
int conf_msgbox_margin_line;
float conf_msgbox_speed;

/*
 * クリックアニメーションの設定
 */
char *conf_click_file;
int conf_click_x;
int conf_click_y;
float conf_click_interval;

/*
 * 選択肢ボックスの設定
 */
char *conf_selbox_fg_file;
char *conf_selbox_bg_file;
int conf_selbox_x;
int conf_selbox_y;
int conf_selbox_margin_y;

/*
 * スイッチボックスの設定
 */
char *conf_switch_bg_file;
char *conf_switch_fg_file;
int conf_switch_x;
int conf_switch_y;
int conf_switch_margin_y;
int conf_switch_text_margin_y;

/*
 * セーブ・ロード画面の設定
 */
char *conf_save_bg_file;
char *conf_save_fg_file;
int conf_save_save_x;
int conf_save_save_y;
int conf_save_save_width;
int conf_save_save_height;
int conf_save_load_x;
int conf_save_load_y;
int conf_save_load_width;
int conf_save_load_height;
int conf_save_prev_x;
int conf_save_prev_y;
int conf_save_prev_width;
int conf_save_prev_height;
int conf_save_next_x;
int conf_save_next_y;
int conf_save_next_width;
int conf_save_next_height;
int conf_save_data_width;
int conf_save_data_height;
int conf_save_data_margin_left;
int conf_save_data_margin_top;
int conf_save_data1_x;
int conf_save_data1_y;
int conf_save_data2_x;
int conf_save_data2_y;
int conf_save_data3_x;
int conf_save_data3_y;
int conf_save_data4_x;
int conf_save_data4_y;
int conf_save_data5_x;
int conf_save_data5_y;
int conf_save_data6_x;
int conf_save_data6_y;

/*
 * ヒストリ画面の設定
 */
int conf_history_color_r;
int conf_history_color_g;
int conf_history_color_b;
int conf_history_color_a;
int conf_history_margin_line;
int conf_history_margin_left;
int conf_history_margin_top;
int conf_history_margin_right;
int conf_history_margin_bottom;

/*
 * 1行のサイズ
 */
#define BUF_SIZE	(1024)

/*
 * 変換ルールテーブル
 */
struct rule {
	const char *key;
	char type;
	void *val;
	bool omissible;
	bool loaded;
} rule_tbl[] = {
	{"window.title", 's', &conf_window_title, false, false},
	{"window.width", 'i', &conf_window_width, false, false},
	{"window.height", 'i', &conf_window_height, false, false},
	{"window.white", 'i', &conf_window_white, false, false},
	{"font.file", 's', &conf_font_file, false, false},
	{"font.size", 'i', &conf_font_size, false, false},
	{"font.color.r", 'i', &conf_font_color_r, false, false},
	{"font.color.g", 'i', &conf_font_color_g, false, false},
	{"font.color.b", 'i', &conf_font_color_b, false, false},
	{"namebox.file", 's', &conf_namebox_file, false, false},
	{"namebox.x", 'i', &conf_namebox_x, false, false},
	{"namebox.y", 'i', &conf_namebox_y, false, false},
	{"namebox.margin.left", 'i', &conf_namebox_margin_left, false, false},
	{"namebox.margin.top", 'i', &conf_namebox_margin_top, false, false},
	{"msgbox.file", 's', &conf_msgbox_file, false, false},
	{"msgbox.x", 'i', &conf_msgbox_x, false, false},
	{"msgbox.y", 'i', &conf_msgbox_y, false, false},
	{"msgbox.margin.left", 'i', &conf_msgbox_margin_left, false, false},
	{"msgbox.margin.top", 'i', &conf_msgbox_margin_top, false, false},
	{"msgbox.margin.right", 'i', &conf_msgbox_margin_right, false, false},
	{"msgbox.margin.line", 'i', &conf_msgbox_margin_line, false, false},
	{"msgbox.speed", 'f', &conf_msgbox_speed, false, false},
	{"click.file", 's', &conf_click_file, false, false},
	{"click.x", 'i', &conf_click_x, false, false},
	{"click.y", 'i', &conf_click_y, false, false},
	{"click.interval", 'f', &conf_click_interval, false, false},
	{"selbox.bg.file", 's', &conf_selbox_bg_file, false, false},
	{"selbox.fg.file", 's', &conf_selbox_fg_file, false, false},
	{"selbox.x", 'i', &conf_selbox_x, false, false},
	{"selbox.y", 'i', &conf_selbox_y, false, false},
	{"selbox.margin.y", 'i', &conf_selbox_margin_y, false, false},
	{"switch.bg.file", 's', &conf_switch_bg_file, false, false},
	{"switch.fg.file", 's', &conf_switch_fg_file, false, false},
	{"switch.x", 'i', &conf_switch_x, false, false},
	{"switch.y", 'i', &conf_switch_y, false, false},
	{"switch.margin.y", 'i', &conf_switch_margin_y, false, false},
	{"switch.text.margin.y", 'i', &conf_switch_text_margin_y, false, false},
	{"save.bg.file", 's', &conf_save_bg_file, false, false},
	{"save.fg.file", 's', &conf_save_fg_file, false, false},
	{"save.save.x", 'i', &conf_save_save_x, false, false},
	{"save.save.y", 'i', &conf_save_save_y, false, false},
	{"save.save.width", 'i', &conf_save_save_width, false, false},
	{"save.save.height", 'i', &conf_save_save_height, false, false},
	{"save.load.x", 'i', &conf_save_load_x, false, false},
	{"save.load.y", 'i', &conf_save_load_y, false, false},
	{"save.load.width", 'i', &conf_save_load_width, false, false},
	{"save.load.height", 'i', &conf_save_load_height, false, false},
	{"save.prev.x", 'i', &conf_save_prev_x, false, false},
	{"save.prev.y", 'i', &conf_save_prev_y, false, false},
	{"save.prev.width", 'i', &conf_save_prev_width, false, false},
	{"save.prev.height", 'i', &conf_save_prev_height, false, false},
	{"save.next.x", 'i', &conf_save_next_x, false, false},
	{"save.next.y", 'i', &conf_save_next_y, false, false},
	{"save.next.width", 'i', &conf_save_next_width, false, false},
	{"save.next.height", 'i', &conf_save_next_height, false, false},
	{"save.data.width", 'i', &conf_save_data_width, false, false},
	{"save.data.height", 'i', &conf_save_data_height, false, false},
	{"save.data.margin.left", 'i', &conf_save_data_margin_left, false, false},
	{"save.data.margin.top", 'i', &conf_save_data_margin_top, false, false},
	{"save.data1.x", 'i', &conf_save_data1_x, false, false},
	{"save.data1.y", 'i', &conf_save_data1_y, false, false},
	{"save.data2.x", 'i', &conf_save_data2_x, false, false},
	{"save.data2.y", 'i', &conf_save_data2_y, false, false},
	{"save.data3.x", 'i', &conf_save_data3_x, false, false},
	{"save.data3.y", 'i', &conf_save_data3_y, false, false},
	{"save.data4.x", 'i', &conf_save_data4_x, false, false},
	{"save.data4.y", 'i', &conf_save_data4_y, false, false},
	{"save.data5.x", 'i', &conf_save_data5_x, false, false},
	{"save.data5.y", 'i', &conf_save_data5_y, false, false},
	{"save.data6.x", 'i', &conf_save_data6_x, false, false},
	{"save.data6.y", 'i', &conf_save_data6_y, false, false},
	{"history.color.r", 'i', &conf_history_color_r, false, false},
	{"history.color.g", 'i', &conf_history_color_g, false, false},
	{"history.color.b", 'i', &conf_history_color_b, false, false},
	{"history.color.a", 'i', &conf_history_color_a, false, false},
	{"history.margin.line", 'i', &conf_history_margin_line, false, false},
	{"history.margin.left", 'i', &conf_history_margin_left, false, false},
	{"history.margin.top", 'i', &conf_history_margin_top, false, false},
	{"history.margin.right", 'i', &conf_history_margin_right, false, false},
	{"history.margin.bottom", 'i', &conf_history_margin_bottom, false, false},
};

#define RULE_TBL_SIZE	(sizeof(rule_tbl) / sizeof(struct rule))

/*
 * 前方参照
 */
static bool read_conf(void);
static bool save_value(const char *k, const char *v);
static bool check_conf(void);

/*
 * コンフィグの初期化処理を行う
 */
bool init_conf(void)
{
	/* コンフィグを読み込む */
	if (!read_conf())
		return false;

	/* 読み込まれなかった文字列コンフィグをチェックする */
	if (!check_conf())
		return false;

	return true;
}

/* コンフィグを読み込む */
static bool read_conf(void)
{
	char buf[BUF_SIZE];
	struct rfile *rf;
	char *k, *v;

	rf = open_rfile(CONF_DIR, PROP_FILE, false);
	if (rf == NULL)
		return false;

	while (gets_rfile(rf, buf, sizeof(buf)) != NULL) {
		if (buf[0] == '#' || buf[0] == '\0')
			continue;

		/* キーを取得する */
		k = strtok(buf, "=");
		if (k == NULL || k[0] == '\0')
			continue;

		/* 値を取得する */
		v = strtok(NULL, "=");
		if (v == NULL || v[0] == '\0')
			continue;

		/* 値を保存する */
		if (!save_value(k, v))
			return false;
	}
	return true;
}

/* 値を保存する */
static bool save_value(const char *k, const char *v)
{
	char *dup;
	size_t i;

	/* ルールテーブルからキーを探して値を保存する */
	for (i = 0; i < RULE_TBL_SIZE; i++) {
		/* キーが一致しなければ次のルールへ */
		if (strcmp(rule_tbl[i].key, k) != 0)
			continue;

		/* 型ごとに変換する */
		if (rule_tbl[i].type == 'i') {
			*(int *)rule_tbl[i].val = atoi(v);
		} else if (rule_tbl[i].type == 'f') {
			*(float *)rule_tbl[i].val = (float)atof(v);
		} else {
			/* 文字列の場合は複製する */
			dup = strdup(v);
			if (dup == NULL) {
				log_memory();
				return false;
			}
			*(char **)rule_tbl[i].val = dup;
		}

		rule_tbl[i].loaded = true;

		return true;
	}
	log_unknown_conf(k);
	return false;
}

/* 読み込まれなかった必須コンフィグをチェックする */
bool check_conf(void)
{
	size_t i;

	for (i = 0; i < RULE_TBL_SIZE; i++) {
		if (!rule_tbl[i].omissible && !rule_tbl[i].loaded) {
			log_undefined_conf(rule_tbl[i].key);
			return false;
		}
	}
	return true;
}

/*
 * コンフィグの終了処理を行う
 */
void cleanup_conf(void)
{
	size_t i;

	/* 文字列のプロパティは解放する */
	for (i = 0; i < RULE_TBL_SIZE; i++) {
		if (rule_tbl[i].type == 's' && rule_tbl[i].val != NULL) {
			free(*(char **)rule_tbl[i].val);
			*(char **)rule_tbl[i].val = NULL;
		}
	}
}
