/* -*- coding: utf-8-with-signature; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Suika 2
 * Copyright (C) 2001-2021, TABATA Keiichi. All rights reserved.
 */

/*
 * [Changes]
 *  - 2016/06/25 作成
 *  - 2017/08/14 スイッチに対応
 *  - 2019/09/17 NEWSに対応
 */

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "suika.h"

/*
 * 言語の設定
 */
char *conf_language;

/*
 * ウィンドウの設定
 */
char *conf_window_title;
int conf_window_width;
int conf_window_height;
int conf_window_white;

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
 * スイッチの設定
 */
char *conf_switch_bg_file;
char *conf_switch_fg_file;
int conf_switch_x;
int conf_switch_y;
int conf_switch_margin_y;
int conf_switch_text_margin_y;
char *conf_switch_parent_click_se_file;
char *conf_switch_child_click_se_file;

/*
 * NEWSの設定
 */
char *conf_news_bg_file;
char *conf_news_fg_file;
int conf_news_margin;
int conf_news_text_margin_y;

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
int conf_save_exit_x;
int conf_save_exit_y;
int conf_save_exit_width;
int conf_save_exit_height;
int conf_save_title_x;
int conf_save_title_y;
int conf_save_title_width;
int conf_save_title_height;
char *conf_save_title_txt;

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
 * セリフの色付け
 */
char *conf_serif_color_name[SERIF_COLOR_COUNT];
int conf_serif_color_r[SERIF_COLOR_COUNT];
int conf_serif_color_g[SERIF_COLOR_COUNT];
int conf_serif_color_b[SERIF_COLOR_COUNT];

/*
 * その他の設定
 */

/* クリックでボイスを止めない */
int conf_voice_stop_off;

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
	{"language", 's', &conf_language, true, false},
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
	{"switch.parent.click.se.file", 's', &conf_switch_parent_click_se_file, true, false},
	{"switch.child.click.se.file", 's', &conf_switch_child_click_se_file, true, false},
	{"news.bg.file", 's', &conf_news_bg_file, false, false},
	{"news.fg.file", 's', &conf_news_fg_file, false, false},
	{"news.margin", 'i', &conf_news_margin, false, false},
	{"news.text.margin.y", 'i', &conf_news_text_margin_y, false, false},
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
	{"save.exit.x", 'i', &conf_save_exit_x, false, false},
	{"save.exit.y", 'i', &conf_save_exit_y, false, false},
	{"save.exit.width", 'i', &conf_save_exit_width, false, false},
	{"save.exit.height", 'i',  &conf_save_exit_height, false, false},
	{"save.title.x", 'i', &conf_save_title_x, false, false},
	{"save.title.y", 'i', &conf_save_title_y, false, false},
	{"save.title.width", 'i', &conf_save_title_width, false, false},
	{"save.title.height", 'i', &conf_save_title_height, false, false},
	{"save.title.txt", 's', &conf_save_title_txt, false, false},
	{"history.color.r", 'i', &conf_history_color_r, false, false},
	{"history.color.g", 'i', &conf_history_color_g, false, false},
	{"history.color.b", 'i', &conf_history_color_b, false, false},
	{"history.color.a", 'i', &conf_history_color_a, false, false},
	{"history.margin.line", 'i', &conf_history_margin_line, false, false},
	{"history.margin.left", 'i', &conf_history_margin_left, false, false},
	{"history.margin.top", 'i', &conf_history_margin_top, false, false},
	{"history.margin.right", 'i', &conf_history_margin_right, false, false},
	{"history.margin.bottom", 'i', &conf_history_margin_bottom, false, false},
	{"serif.color1.name", 's', &conf_serif_color_name[0], true, false},
	{"serif.color1.r", 'i', &conf_serif_color_r[0], true, false},
	{"serif.color1.g", 'i', &conf_serif_color_g[0], true, false},
	{"serif.color1.b", 'i', &conf_serif_color_b[0], true, false},
	{"serif.color2.name", 's', &conf_serif_color_name[1], true, false},
	{"serif.color2.r", 'i', &conf_serif_color_r[1], true, false},
	{"serif.color2.g", 'i', &conf_serif_color_g[1], true, false},
	{"serif.color2.b", 'i', &conf_serif_color_b[1], true, false},
	{"serif.color3.name", 's', &conf_serif_color_name[2], true, false},
	{"serif.color3.r", 'i', &conf_serif_color_r[2], true, false},
	{"serif.color3.g", 'i', &conf_serif_color_g[2], true, false},
	{"serif.color3.b", 'i', &conf_serif_color_b[2], true, false},
	{"serif.color4.name", 's', &conf_serif_color_name[3], true, false},
	{"serif.color4.r", 'i', &conf_serif_color_r[3], true, false},
	{"serif.color4.g", 'i', &conf_serif_color_g[3], true, false},
	{"serif.color4.b", 'i', &conf_serif_color_b[3], true, false},
	{"serif.color5.name", 's', &conf_serif_color_name[4], true, false},
	{"serif.color5.r", 'i', &conf_serif_color_r[4], true, false},
	{"serif.color5.g", 'i', &conf_serif_color_g[4], true, false},
	{"serif.color5.b", 'i', &conf_serif_color_b[4], true, false},
	{"serif.color6.name", 's', &conf_serif_color_name[5], true, false},
	{"serif.color6.r", 'i', &conf_serif_color_r[5], true, false},
	{"serif.color6.g", 'i', &conf_serif_color_g[5], true, false},
	{"serif.color6.b", 'i', &conf_serif_color_b[5], true, false},
	{"serif.color7.name", 's', &conf_serif_color_name[6], true, false},
	{"serif.color7.r", 'i', &conf_serif_color_r[6], true, false},
	{"serif.color7.g", 'i', &conf_serif_color_g[6], true, false},
	{"serif.color7.b", 'i', &conf_serif_color_b[6], true, false},
	{"serif.color8.name", 's', &conf_serif_color_name[7], true, false},
	{"serif.color8.r", 'i', &conf_serif_color_r[7], true, false},
	{"serif.color8.g", 'i', &conf_serif_color_g[7], true, false},
	{"serif.color8.b", 'i', &conf_serif_color_b[7], true, false},
	{"serif.color9.name", 's', &conf_serif_color_name[8], true, false},
	{"serif.color9.r", 'i', &conf_serif_color_r[8], true, false},
	{"serif.color9.g", 'i', &conf_serif_color_g[8], true, false},
	{"serif.color9.b", 'i', &conf_serif_color_b[8], true, false},
	{"serif.color10.name", 's', &conf_serif_color_name[9], true, false},
	{"serif.color10.r", 'i', &conf_serif_color_r[9], true, false},
	{"serif.color10.g", 'i', &conf_serif_color_g[9], true, false},
	{"serif.color10.b", 'i', &conf_serif_color_b[9], true, false},
	{"serif.color11.name", 's', &conf_serif_color_name[10], true, false},
	{"serif.color11.r", 'i', &conf_serif_color_r[10], true, false},
	{"serif.color11.g", 'i', &conf_serif_color_g[10], true, false},
	{"serif.color11.b", 'i', &conf_serif_color_b[10], true, false},
	{"serif.color12.name", 's', &conf_serif_color_name[11], true, false},
	{"serif.color12.r", 'i', &conf_serif_color_r[11], true, false},
	{"serif.color12.g", 'i', &conf_serif_color_g[11], true, false},
	{"serif.color12.b", 'i', &conf_serif_color_b[11], true, false},
	{"serif.color13.name", 's', &conf_serif_color_name[12], true, false},
	{"serif.color13.r", 'i', &conf_serif_color_r[12], true, false},
	{"serif.color13.g", 'i', &conf_serif_color_g[12], true, false},
	{"serif.color13.b", 'i', &conf_serif_color_b[12], true, false},
	{"serif.color14.name", 's', &conf_serif_color_name[13], true, false},
	{"serif.color14.r", 'i', &conf_serif_color_r[13], true, false},
	{"serif.color14.g", 'i', &conf_serif_color_g[13], true, false},
	{"serif.color14.b", 'i', &conf_serif_color_b[13], true, false},
	{"serif.color15.name", 's', &conf_serif_color_name[14], true, false},
	{"serif.color15.r", 'i', &conf_serif_color_r[14], true, false},
	{"serif.color15.g", 'i', &conf_serif_color_g[14], true, false},
	{"serif.color15.b", 'i', &conf_serif_color_b[14], true, false},
	{"serif.color16.name", 's', &conf_serif_color_name[15], true, false},
	{"serif.color16.r", 'i', &conf_serif_color_r[15], true, false},
	{"serif.color16.g", 'i', &conf_serif_color_g[15], true, false},
	{"serif.color16.b", 'i', &conf_serif_color_b[15], true, false},
	{"serif.color17.name", 's', &conf_serif_color_name[16], true, false},
	{"serif.color17.r", 'i', &conf_serif_color_r[16], true, false},
	{"serif.color17.g", 'i', &conf_serif_color_g[16], true, false},
	{"serif.color17.b", 'i', &conf_serif_color_b[16], true, false},
	{"serif.color18.name", 's', &conf_serif_color_name[17], true, false},
	{"serif.color18.r", 'i', &conf_serif_color_r[17], true, false},
	{"serif.color18.g", 'i', &conf_serif_color_g[17], true, false},
	{"serif.color18.b", 'i', &conf_serif_color_b[17], true, false},
	{"serif.color19.name", 's', &conf_serif_color_name[18], true, false},
	{"serif.color19.r", 'i', &conf_serif_color_r[18], true, false},
	{"serif.color19.g", 'i', &conf_serif_color_g[18], true, false},
	{"serif.color19.b", 'i', &conf_serif_color_b[18], true, false},
	{"serif.color20.name", 's', &conf_serif_color_name[19], true, false},
	{"serif.color20.r", 'i', &conf_serif_color_r[19], true, false},
	{"serif.color20.g", 'i', &conf_serif_color_g[19], true, false},
	{"serif.color20.b", 'i', &conf_serif_color_b[19], true, false},
	{"serif.color21.name", 's', &conf_serif_color_name[20], true, false},
	{"serif.color21.r", 'i', &conf_serif_color_r[20], true, false},
	{"serif.color21.g", 'i', &conf_serif_color_g[20], true, false},
	{"serif.color21.b", 'i', &conf_serif_color_b[20], true, false},
	{"serif.color22.name", 's', &conf_serif_color_name[21], true, false},
	{"serif.color22.r", 'i', &conf_serif_color_r[21], true, false},
	{"serif.color22.g", 'i', &conf_serif_color_g[21], true, false},
	{"serif.color22.b", 'i', &conf_serif_color_b[21], true, false},
	{"serif.color23.name", 's', &conf_serif_color_name[22], true, false},
	{"serif.color23.r", 'i', &conf_serif_color_r[22], true, false},
	{"serif.color23.g", 'i', &conf_serif_color_g[22], true, false},
	{"serif.color23.b", 'i', &conf_serif_color_b[22], true, false},
	{"serif.color24.name", 's', &conf_serif_color_name[23], true, false},
	{"serif.color24.r", 'i', &conf_serif_color_r[23], true, false},
	{"serif.color24.g", 'i', &conf_serif_color_g[23], true, false},
	{"serif.color24.b", 'i', &conf_serif_color_b[23], true, false},
	{"serif.color25.name", 's', &conf_serif_color_name[24], true, false},
	{"serif.color25.r", 'i', &conf_serif_color_r[24], true, false},
	{"serif.color25.g", 'i', &conf_serif_color_g[24], true, false},
	{"serif.color25.b", 'i', &conf_serif_color_b[24], true, false},
	{"serif.color26.name", 's', &conf_serif_color_name[25], true, false},
	{"serif.color26.r", 'i', &conf_serif_color_r[25], true, false},
	{"serif.color26.g", 'i', &conf_serif_color_g[25], true, false},
	{"serif.color26.b", 'i', &conf_serif_color_b[25], true, false},
	{"serif.color27.name", 's', &conf_serif_color_name[26], true, false},
	{"serif.color27.r", 'i', &conf_serif_color_r[26], true, false},
	{"serif.color27.g", 'i', &conf_serif_color_g[26], true, false},
	{"serif.color27.b", 'i', &conf_serif_color_b[26], true, false},
	{"serif.color28.name", 's', &conf_serif_color_name[27], true, false},
	{"serif.color28.r", 'i', &conf_serif_color_r[27], true, false},
	{"serif.color28.g", 'i', &conf_serif_color_g[27], true, false},
	{"serif.color28.b", 'i', &conf_serif_color_b[27], true, false},
	{"serif.color29.name", 's', &conf_serif_color_name[28], true, false},
	{"serif.color29.r", 'i', &conf_serif_color_r[28], true, false},
	{"serif.color29.g", 'i', &conf_serif_color_g[28], true, false},
	{"serif.color29.b", 'i', &conf_serif_color_b[28], true, false},
	{"serif.color30.name", 's', &conf_serif_color_name[29], true, false},
	{"serif.color30.r", 'i', &conf_serif_color_r[29], true, false},
	{"serif.color30.g", 'i', &conf_serif_color_g[29], true, false},
	{"serif.color30.b", 'i', &conf_serif_color_b[29], true, false},
	{"serif.color31.name", 's', &conf_serif_color_name[30], true, false},
	{"serif.color31.r", 'i', &conf_serif_color_r[30], true, false},
	{"serif.color31.g", 'i', &conf_serif_color_g[30], true, false},
	{"serif.color31.b", 'i', &conf_serif_color_b[30], true, false},
	{"serif.color32.name", 's', &conf_serif_color_name[31], true, false},
	{"serif.color32.r", 'i', &conf_serif_color_r[31], true, false},
	{"serif.color32.g", 'i', &conf_serif_color_g[31], true, false},
	{"serif.color32.b", 'i', &conf_serif_color_b[31], true, false},
	{"voice.stop.off", 'i', &conf_voice_stop_off, true, false},
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

	/* 読み込まれなかった必須コンフィグをチェックする */
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
