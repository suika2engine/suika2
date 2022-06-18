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
int conf_font_outline_size;
int conf_font_outline_color_r;
int conf_font_outline_color_g;
int conf_font_outline_color_b;

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
char *conf_msgbox_bg_file;
char *conf_msgbox_fg_file;
int conf_msgbox_x;
int conf_msgbox_y;
int conf_msgbox_margin_left;
int conf_msgbox_margin_top;
int conf_msgbox_margin_right;
int conf_msgbox_margin_line;
float conf_msgbox_speed;
float conf_msgbox_auto_speed;
int conf_msgbox_btn_qsave_x;
int conf_msgbox_btn_qsave_y;
int conf_msgbox_btn_qsave_width;
int conf_msgbox_btn_qsave_height;
int conf_msgbox_btn_qload_x;
int conf_msgbox_btn_qload_y;
int conf_msgbox_btn_qload_width;
int conf_msgbox_btn_qload_height;
int conf_msgbox_btn_save_x;
int conf_msgbox_btn_save_y;
int conf_msgbox_btn_save_width;
int conf_msgbox_btn_save_height;
int conf_msgbox_btn_load_x;
int conf_msgbox_btn_load_y;
int conf_msgbox_btn_load_width;
int conf_msgbox_btn_load_height;
int conf_msgbox_btn_auto_x;
int conf_msgbox_btn_auto_y;
int conf_msgbox_btn_auto_width;
int conf_msgbox_btn_auto_height;
int conf_msgbox_btn_skip_x;
int conf_msgbox_btn_skip_y;
int conf_msgbox_btn_skip_width;
int conf_msgbox_btn_skip_height;
int conf_msgbox_btn_history_x;
int conf_msgbox_btn_history_y;
int conf_msgbox_btn_history_width;
int conf_msgbox_btn_history_height;
char *conf_msgbox_btn_qsave_se;
char *conf_msgbox_btn_qload_se;
char *conf_msgbox_btn_save_se;
char *conf_msgbox_btn_load_se;
char *conf_msgbox_btn_auto_se;
char *conf_msgbox_btn_skip_se;
char *conf_msgbox_btn_history_se;
char *conf_msgbox_btn_change_se;
char *conf_msgbox_save_se;
char *conf_msgbox_history_se;
char *conf_msgbox_auto_cancel_se;
char *conf_msgbox_skip_cancel_se;
int conf_msgbox_skip_unseen;

/*
 * クリックアニメーションの設定
 */
char *conf_click_file;
int conf_click_x;
int conf_click_y;
float conf_click_interval;

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
char *conf_switch_change_se;

/*
 * NEWSの設定
 */
char *conf_news_bg_file;
char *conf_news_fg_file;
int conf_news_margin;
int conf_news_text_margin_y;
char *conf_news_change_se;

/*
 * 回想画面(@retrospect)の設定
 */
char *conf_retrospect_change_se;

/*
 * セーブ・ロード画面の設定
 */
char *conf_save_save_bg_file;
char *conf_save_save_fg_file;
char *conf_save_load_bg_file;
char *conf_save_load_fg_file;
char *conf_save_prev_se;
int conf_save_prev_x;
int conf_save_prev_y;
int conf_save_prev_width;
int conf_save_prev_height;
char *conf_save_next_se;
int conf_save_next_x;
int conf_save_next_y;
int conf_save_next_width;
int conf_save_next_height;
char *conf_save_data_save_se;
char *conf_save_data_load_se;
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
char *conf_save_exit_se;
int conf_save_exit_x;
int conf_save_exit_y;
int conf_save_exit_width;
int conf_save_exit_height;
char *conf_save_title_se;
int conf_save_title_x;
int conf_save_title_y;
int conf_save_title_width;
int conf_save_title_height;
char *conf_save_title_txt;
char *conf_save_loadtosave_se;
char *conf_save_savetoload_se;
char *conf_save_cancel_save_se;
char *conf_save_cancel_load_se;
char *conf_save_change_se;

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
char *conf_history_cancel_se;

/*
 * メニューの設定
 */
char *conf_menu_change_se;

/*
 * セリフの色付け
 */
char *conf_serif_color_name[SERIF_COLOR_COUNT];
int conf_serif_color_r[SERIF_COLOR_COUNT];
int conf_serif_color_g[SERIF_COLOR_COUNT];
int conf_serif_color_b[SERIF_COLOR_COUNT];
int conf_serif_outline_color_r[SERIF_COLOR_COUNT];
int conf_serif_outline_color_g[SERIF_COLOR_COUNT];
int conf_serif_outline_color_b[SERIF_COLOR_COUNT];

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
	{"font.outline.color.r", 'i', &conf_font_outline_color_r, true, false},
	{"font.outline.color.g", 'i', &conf_font_outline_color_g, true, false},
	{"font.outline.color.b", 'i', &conf_font_outline_color_b, true, false},
	{"namebox.file", 's', &conf_namebox_file, false, false},
	{"namebox.x", 'i', &conf_namebox_x, false, false},
	{"namebox.y", 'i', &conf_namebox_y, false, false},
	{"namebox.margin.top", 'i', &conf_namebox_margin_top, false, false},
	{"msgbox.bg.file", 's', &conf_msgbox_bg_file, false, false},
	{"msgbox.fg.file", 's', &conf_msgbox_fg_file, false, false},
	{"msgbox.x", 'i', &conf_msgbox_x, false, false},
	{"msgbox.y", 'i', &conf_msgbox_y, false, false},
	{"msgbox.margin.left", 'i', &conf_msgbox_margin_left, false, false},
	{"msgbox.margin.top", 'i', &conf_msgbox_margin_top, false, false},
	{"msgbox.margin.right", 'i', &conf_msgbox_margin_right, false, false},
	{"msgbox.margin.line", 'i', &conf_msgbox_margin_line, false, false},
	{"msgbox.speed", 'f', &conf_msgbox_speed, false, false},
	{"msgbox.auto.speed", 'f', &conf_msgbox_auto_speed, true, false},
	{"msgbox.btn.qsave.x", 'i', &conf_msgbox_btn_qsave_x, true, false},
	{"msgbox.btn.qsave.y", 'i', &conf_msgbox_btn_qsave_y, true, false},
	{"msgbox.btn.qsave.width", 'i', &conf_msgbox_btn_qsave_width, true, false},
	{"msgbox.btn.qsave.height", 'i', &conf_msgbox_btn_qsave_height, true, false},
	{"msgbox.btn.qload.x", 'i', &conf_msgbox_btn_qload_x, true, false},
	{"msgbox.btn.qload.y", 'i', &conf_msgbox_btn_qload_y, true, false},
	{"msgbox.btn.qload.width", 'i', &conf_msgbox_btn_qload_width, true, false},
	{"msgbox.btn.qload.height", 'i', &conf_msgbox_btn_qload_height, true, false},
	{"msgbox.btn.save.x", 'i', &conf_msgbox_btn_save_x, true, false},
	{"msgbox.btn.save.y", 'i', &conf_msgbox_btn_save_y, true, false},
	{"msgbox.btn.save.width", 'i', &conf_msgbox_btn_save_width, true, false},
	{"msgbox.btn.save.height", 'i', &conf_msgbox_btn_save_height, true, false},
	{"msgbox.btn.load.x", 'i', &conf_msgbox_btn_load_x, true, false},
	{"msgbox.btn.load.y", 'i', &conf_msgbox_btn_load_y, true, false},
	{"msgbox.btn.load.width", 'i', &conf_msgbox_btn_load_width, true, false},
	{"msgbox.btn.load.height", 'i', &conf_msgbox_btn_load_height, true, false},
	{"msgbox.btn.auto.x", 'i', &conf_msgbox_btn_auto_x, true, false},
	{"msgbox.btn.auto.y", 'i', &conf_msgbox_btn_auto_y, true, false},
	{"msgbox.btn.auto.width", 'i', &conf_msgbox_btn_auto_width, true, false},
	{"msgbox.btn.auto.height", 'i', &conf_msgbox_btn_auto_height, true, false},
	{"msgbox.btn.skip.se", 's', &conf_msgbox_btn_skip_se, true, false},
	{"msgbox.btn.skip.x", 'i', &conf_msgbox_btn_skip_x, true, false},
	{"msgbox.btn.skip.y", 'i', &conf_msgbox_btn_skip_y, true, false},
	{"msgbox.btn.skip.width", 'i', &conf_msgbox_btn_skip_width, true, false},
	{"msgbox.btn.skip.height", 'i', &conf_msgbox_btn_skip_height, true, false},
	{"msgbox.btn.history.x", 'i', &conf_msgbox_btn_history_x, true, false},
	{"msgbox.btn.history.y", 'i', &conf_msgbox_btn_history_y, true, false},
	{"msgbox.btn.history.width", 'i', &conf_msgbox_btn_history_width, true, false},
	{"msgbox.btn.history.height", 'i', &conf_msgbox_btn_history_height, true, false},
	{"msgbox.btn.qsave.se", 's', &conf_msgbox_btn_qsave_se, true, false},
	{"msgbox.btn.qload.se", 's', &conf_msgbox_btn_qload_se, true, false},
	{"msgbox.btn.save.se", 's', &conf_msgbox_btn_save_se, true, false},
	{"msgbox.btn.load.se", 's', &conf_msgbox_btn_load_se, true, false},
	{"msgbox.btn.auto.se", 's', &conf_msgbox_btn_auto_se, true, false},
	{"msgbox.btn.history.se", 's', &conf_msgbox_btn_history_se, true, false},
	{"msgbox.btn.change.se", 's', &conf_msgbox_btn_change_se, true, false},
	{"msgbox.save.se", 's', &conf_msgbox_save_se, true, false},
	{"msgbox.history.se", 's', &conf_msgbox_history_se, true, false},
	{"msgbox.auto.cancel.se", 's', &conf_msgbox_auto_cancel_se, true, false},
	{"msgbox.skip.cancel.se", 's', &conf_msgbox_skip_cancel_se, true, false},
	{"msgbox.skip.unseen", 'i', &conf_msgbox_skip_unseen, true, false},
	{"click.file", 's', &conf_click_file, false, false},
	{"click.x", 'i', &conf_click_x, false, false},
	{"click.y", 'i', &conf_click_y, false, false},
	{"click.interval", 'f', &conf_click_interval, false, false},
	{"selbox.bg.file", 's', NULL, true, false},
	{"selbox.fg.file", 's', NULL, true, false},
	{"selbox.x", 'i', NULL, true, false},
	{"selbox.y", 'i', NULL, true, false},
	{"selbox.margin.y", 'i', NULL, true, false},
	{"selbox.change.se", 's', NULL, true, false},
	{"switch.bg.file", 's', &conf_switch_bg_file, false, false},
	{"switch.fg.file", 's', &conf_switch_fg_file, false, false},
	{"switch.x", 'i', &conf_switch_x, false, false},
	{"switch.y", 'i', &conf_switch_y, false, false},
	{"switch.margin.y", 'i', &conf_switch_margin_y, false, false},
	{"switch.text.margin.y", 'i', &conf_switch_text_margin_y, false, false},
	{"switch.parent.click.se.file", 's', &conf_switch_parent_click_se_file, true, false},
	{"switch.child.click.se.file", 's', &conf_switch_child_click_se_file, true, false},
	{"switch.change.se", 's', &conf_switch_change_se, true, false},
	{"news.bg.file", 's', &conf_news_bg_file, true, false},
	{"news.fg.file", 's', &conf_news_fg_file, true, false},
	{"news.margin", 'i', &conf_news_margin, true, false},
	{"news.text.margin.y", 'i', &conf_news_text_margin_y, true, false},
	{"news.change.se", 's', &conf_news_change_se, true, false},
	{"retrospect.change.se", 's', &conf_retrospect_change_se, true, false},
	{"save.save.bg.file", 's', &conf_save_save_bg_file, false, false},
	{"save.save.fg.file", 's', &conf_save_save_fg_file, false, false},
	{"save.load.bg.file", 's', &conf_save_load_bg_file, false, false},
	{"save.load.fg.file", 's', &conf_save_load_fg_file, false, false},
	{"save.prev.se", 's', &conf_save_prev_se, true, false},
	{"save.prev.x", 'i', &conf_save_prev_x, false, false},
	{"save.prev.y", 'i', &conf_save_prev_y, false, false},
	{"save.prev.width", 'i', &conf_save_prev_width, false, false},
	{"save.prev.height", 'i', &conf_save_prev_height, false, false},
	{"save.next.se", 's', &conf_save_next_se, true, false},
	{"save.next.x", 'i', &conf_save_next_x, false, false},
	{"save.next.y", 'i', &conf_save_next_y, false, false},
	{"save.next.width", 'i', &conf_save_next_width, false, false},
	{"save.next.height", 'i', &conf_save_next_height, false, false},
	{"save.data.save.se", 's', &conf_save_data_save_se, true, false},
	{"save.data.load.se", 's', &conf_save_data_load_se, true, false},
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
	{"save.exit.se", 's', &conf_save_exit_se, true, false},
	{"save.exit.x", 'i', &conf_save_exit_x, false, false},
	{"save.exit.y", 'i', &conf_save_exit_y, false, false},
	{"save.exit.width", 'i', &conf_save_exit_width, false, false},
	{"save.exit.height", 'i',  &conf_save_exit_height, false, false},
	{"save.title.se", 's', &conf_save_title_se, true, false},
	{"save.title.x", 'i', &conf_save_title_x, false, false},
	{"save.title.y", 'i', &conf_save_title_y, false, false},
	{"save.title.width", 'i', &conf_save_title_width, false, false},
	{"save.title.height", 'i', &conf_save_title_height, false, false},
	{"save.title.txt", 's', &conf_save_title_txt, false, false},
	{"save.loadtosave.se", 's', &conf_save_loadtosave_se, true, false},
	{"save.savetoload.se", 's', &conf_save_savetoload_se, true, false},
	{"save.cancel.save.se", 's', &conf_save_cancel_save_se, true, false},
	{"save.cancel.load.se", 's', &conf_save_cancel_load_se, true, false},
	{"save.change.se", 's', &conf_save_change_se, true, false},
	{"history.color.r", 'i', &conf_history_color_r, false, false},
	{"history.color.g", 'i', &conf_history_color_g, false, false},
	{"history.color.b", 'i', &conf_history_color_b, false, false},
	{"history.color.a", 'i', &conf_history_color_a, false, false},
	{"history.margin.line", 'i', &conf_history_margin_line, false, false},
	{"history.margin.left", 'i', &conf_history_margin_left, false, false},
	{"history.margin.top", 'i', &conf_history_margin_top, false, false},
	{"history.margin.right", 'i', &conf_history_margin_right, false, false},
	{"history.margin.bottom", 'i', &conf_history_margin_bottom, false, false},
	{"history.cancel.se", 's', &conf_history_cancel_se, true, false},
	{"menu.change.se", 's', &conf_menu_change_se, true, false},
	/* start codegen */
	{"serif.color1.name", 's', &conf_serif_color_name[0], true, false},
	{"serif.color1.r", 'i', &conf_serif_color_r[0], true, false},
	{"serif.color1.g", 'i', &conf_serif_color_g[0], true, false},
	{"serif.color1.b", 'i', &conf_serif_color_b[0], true, false},
	{"serif.color1.outline.r", 'i', &conf_serif_outline_color_r[0], true, false},
	{"serif.color1.outline.g", 'i', &conf_serif_outline_color_g[0], true, false},
	{"serif.color1.outline.b", 'i', &conf_serif_outline_color_b[0], true, false},
	{"serif.color2.name", 's', &conf_serif_color_name[1], true, false},
	{"serif.color2.r", 'i', &conf_serif_color_r[1], true, false},
	{"serif.color2.g", 'i', &conf_serif_color_g[1], true, false},
	{"serif.color2.b", 'i', &conf_serif_color_b[1], true, false},
	{"serif.color2.outline.r", 'i', &conf_serif_outline_color_r[1], true, false},
	{"serif.color2.outline.g", 'i', &conf_serif_outline_color_g[1], true, false},
	{"serif.color2.outline.b", 'i', &conf_serif_outline_color_b[1], true, false},
	{"serif.color3.name", 's', &conf_serif_color_name[2], true, false},
	{"serif.color3.r", 'i', &conf_serif_color_r[2], true, false},
	{"serif.color3.g", 'i', &conf_serif_color_g[2], true, false},
	{"serif.color3.b", 'i', &conf_serif_color_b[2], true, false},
	{"serif.color3.outline.r", 'i', &conf_serif_outline_color_r[2], true, false},
	{"serif.color3.outline.g", 'i', &conf_serif_outline_color_g[2], true, false},
	{"serif.color3.outline.b", 'i', &conf_serif_outline_color_b[2], true, false},
	{"serif.color4.name", 's', &conf_serif_color_name[3], true, false},
	{"serif.color4.r", 'i', &conf_serif_color_r[3], true, false},
	{"serif.color4.g", 'i', &conf_serif_color_g[3], true, false},
	{"serif.color4.b", 'i', &conf_serif_color_b[3], true, false},
	{"serif.color4.outline.r", 'i', &conf_serif_outline_color_r[3], true, false},
	{"serif.color4.outline.g", 'i', &conf_serif_outline_color_g[3], true, false},
	{"serif.color4.outline.b", 'i', &conf_serif_outline_color_b[3], true, false},
	{"serif.color5.name", 's', &conf_serif_color_name[4], true, false},
	{"serif.color5.r", 'i', &conf_serif_color_r[4], true, false},
	{"serif.color5.g", 'i', &conf_serif_color_g[4], true, false},
	{"serif.color5.b", 'i', &conf_serif_color_b[4], true, false},
	{"serif.color5.outline.r", 'i', &conf_serif_outline_color_r[4], true, false},
	{"serif.color5.outline.g", 'i', &conf_serif_outline_color_g[4], true, false},
	{"serif.color5.outline.b", 'i', &conf_serif_outline_color_b[4], true, false},
	{"serif.color6.name", 's', &conf_serif_color_name[5], true, false},
	{"serif.color6.r", 'i', &conf_serif_color_r[5], true, false},
	{"serif.color6.g", 'i', &conf_serif_color_g[5], true, false},
	{"serif.color6.b", 'i', &conf_serif_color_b[5], true, false},
	{"serif.color6.outline.r", 'i', &conf_serif_outline_color_r[5], true, false},
	{"serif.color6.outline.g", 'i', &conf_serif_outline_color_g[5], true, false},
	{"serif.color6.outline.b", 'i', &conf_serif_outline_color_b[5], true, false},
	{"serif.color7.name", 's', &conf_serif_color_name[6], true, false},
	{"serif.color7.r", 'i', &conf_serif_color_r[6], true, false},
	{"serif.color7.g", 'i', &conf_serif_color_g[6], true, false},
	{"serif.color7.b", 'i', &conf_serif_color_b[6], true, false},
	{"serif.color7.outline.r", 'i', &conf_serif_outline_color_r[6], true, false},
	{"serif.color7.outline.g", 'i', &conf_serif_outline_color_g[6], true, false},
	{"serif.color7.outline.b", 'i', &conf_serif_outline_color_b[6], true, false},
	{"serif.color8.name", 's', &conf_serif_color_name[7], true, false},
	{"serif.color8.r", 'i', &conf_serif_color_r[7], true, false},
	{"serif.color8.g", 'i', &conf_serif_color_g[7], true, false},
	{"serif.color8.b", 'i', &conf_serif_color_b[7], true, false},
	{"serif.color8.outline.r", 'i', &conf_serif_outline_color_r[7], true, false},
	{"serif.color8.outline.g", 'i', &conf_serif_outline_color_g[7], true, false},
	{"serif.color8.outline.b", 'i', &conf_serif_outline_color_b[7], true, false},
	{"serif.color9.name", 's', &conf_serif_color_name[8], true, false},
	{"serif.color9.r", 'i', &conf_serif_color_r[8], true, false},
	{"serif.color9.g", 'i', &conf_serif_color_g[8], true, false},
	{"serif.color9.b", 'i', &conf_serif_color_b[8], true, false},
	{"serif.color9.outline.r", 'i', &conf_serif_outline_color_r[8], true, false},
	{"serif.color9.outline.g", 'i', &conf_serif_outline_color_g[8], true, false},
	{"serif.color9.outline.b", 'i', &conf_serif_outline_color_b[8], true, false},
	{"serif.color10.name", 's', &conf_serif_color_name[9], true, false},
	{"serif.color10.r", 'i', &conf_serif_color_r[9], true, false},
	{"serif.color10.g", 'i', &conf_serif_color_g[9], true, false},
	{"serif.color10.b", 'i', &conf_serif_color_b[9], true, false},
	{"serif.color10.outline.r", 'i', &conf_serif_outline_color_r[9], true, false},
	{"serif.color10.outline.g", 'i', &conf_serif_outline_color_g[9], true, false},
	{"serif.color10.outline.b", 'i', &conf_serif_outline_color_b[9], true, false},
	{"serif.color11.name", 's', &conf_serif_color_name[10], true, false},
	{"serif.color11.r", 'i', &conf_serif_color_r[10], true, false},
	{"serif.color11.g", 'i', &conf_serif_color_g[10], true, false},
	{"serif.color11.b", 'i', &conf_serif_color_b[10], true, false},
	{"serif.color11.outline.r", 'i', &conf_serif_outline_color_r[10], true, false},
	{"serif.color11.outline.g", 'i', &conf_serif_outline_color_g[10], true, false},
	{"serif.color11.outline.b", 'i', &conf_serif_outline_color_b[10], true, false},
	{"serif.color12.name", 's', &conf_serif_color_name[11], true, false},
	{"serif.color12.r", 'i', &conf_serif_color_r[11], true, false},
	{"serif.color12.g", 'i', &conf_serif_color_g[11], true, false},
	{"serif.color12.b", 'i', &conf_serif_color_b[11], true, false},
	{"serif.color12.outline.r", 'i', &conf_serif_outline_color_r[11], true, false},
	{"serif.color12.outline.g", 'i', &conf_serif_outline_color_g[11], true, false},
	{"serif.color12.outline.b", 'i', &conf_serif_outline_color_b[11], true, false},
	{"serif.color13.name", 's', &conf_serif_color_name[12], true, false},
	{"serif.color13.r", 'i', &conf_serif_color_r[12], true, false},
	{"serif.color13.g", 'i', &conf_serif_color_g[12], true, false},
	{"serif.color13.b", 'i', &conf_serif_color_b[12], true, false},
	{"serif.color13.outline.r", 'i', &conf_serif_outline_color_r[12], true, false},
	{"serif.color13.outline.g", 'i', &conf_serif_outline_color_g[12], true, false},
	{"serif.color13.outline.b", 'i', &conf_serif_outline_color_b[12], true, false},
	{"serif.color14.name", 's', &conf_serif_color_name[13], true, false},
	{"serif.color14.r", 'i', &conf_serif_color_r[13], true, false},
	{"serif.color14.g", 'i', &conf_serif_color_g[13], true, false},
	{"serif.color14.b", 'i', &conf_serif_color_b[13], true, false},
	{"serif.color14.outline.r", 'i', &conf_serif_outline_color_r[13], true, false},
	{"serif.color14.outline.g", 'i', &conf_serif_outline_color_g[13], true, false},
	{"serif.color14.outline.b", 'i', &conf_serif_outline_color_b[13], true, false},
	{"serif.color15.name", 's', &conf_serif_color_name[14], true, false},
	{"serif.color15.r", 'i', &conf_serif_color_r[14], true, false},
	{"serif.color15.g", 'i', &conf_serif_color_g[14], true, false},
	{"serif.color15.b", 'i', &conf_serif_color_b[14], true, false},
	{"serif.color15.outline.r", 'i', &conf_serif_outline_color_r[14], true, false},
	{"serif.color15.outline.g", 'i', &conf_serif_outline_color_g[14], true, false},
	{"serif.color15.outline.b", 'i', &conf_serif_outline_color_b[14], true, false},
	{"serif.color16.name", 's', &conf_serif_color_name[15], true, false},
	{"serif.color16.r", 'i', &conf_serif_color_r[15], true, false},
	{"serif.color16.g", 'i', &conf_serif_color_g[15], true, false},
	{"serif.color16.b", 'i', &conf_serif_color_b[15], true, false},
	{"serif.color16.outline.r", 'i', &conf_serif_outline_color_r[15], true, false},
	{"serif.color16.outline.g", 'i', &conf_serif_outline_color_g[15], true, false},
	{"serif.color16.outline.b", 'i', &conf_serif_outline_color_b[15], true, false},
	{"serif.color17.name", 's', &conf_serif_color_name[16], true, false},
	{"serif.color17.r", 'i', &conf_serif_color_r[16], true, false},
	{"serif.color17.g", 'i', &conf_serif_color_g[16], true, false},
	{"serif.color17.b", 'i', &conf_serif_color_b[16], true, false},
	{"serif.color17.outline.r", 'i', &conf_serif_outline_color_r[16], true, false},
	{"serif.color17.outline.g", 'i', &conf_serif_outline_color_g[16], true, false},
	{"serif.color17.outline.b", 'i', &conf_serif_outline_color_b[16], true, false},
	{"serif.color18.name", 's', &conf_serif_color_name[17], true, false},
	{"serif.color18.r", 'i', &conf_serif_color_r[17], true, false},
	{"serif.color18.g", 'i', &conf_serif_color_g[17], true, false},
	{"serif.color18.b", 'i', &conf_serif_color_b[17], true, false},
	{"serif.color18.outline.r", 'i', &conf_serif_outline_color_r[17], true, false},
	{"serif.color18.outline.g", 'i', &conf_serif_outline_color_g[17], true, false},
	{"serif.color18.outline.b", 'i', &conf_serif_outline_color_b[17], true, false},
	{"serif.color19.name", 's', &conf_serif_color_name[18], true, false},
	{"serif.color19.r", 'i', &conf_serif_color_r[18], true, false},
	{"serif.color19.g", 'i', &conf_serif_color_g[18], true, false},
	{"serif.color19.b", 'i', &conf_serif_color_b[18], true, false},
	{"serif.color19.outline.r", 'i', &conf_serif_outline_color_r[18], true, false},
	{"serif.color19.outline.g", 'i', &conf_serif_outline_color_g[18], true, false},
	{"serif.color19.outline.b", 'i', &conf_serif_outline_color_b[18], true, false},
	{"serif.color20.name", 's', &conf_serif_color_name[19], true, false},
	{"serif.color20.r", 'i', &conf_serif_color_r[19], true, false},
	{"serif.color20.g", 'i', &conf_serif_color_g[19], true, false},
	{"serif.color20.b", 'i', &conf_serif_color_b[19], true, false},
	{"serif.color20.outline.r", 'i', &conf_serif_outline_color_r[19], true, false},
	{"serif.color20.outline.g", 'i', &conf_serif_outline_color_g[19], true, false},
	{"serif.color20.outline.b", 'i', &conf_serif_outline_color_b[19], true, false},
	{"serif.color21.name", 's', &conf_serif_color_name[20], true, false},
	{"serif.color21.r", 'i', &conf_serif_color_r[20], true, false},
	{"serif.color21.g", 'i', &conf_serif_color_g[20], true, false},
	{"serif.color21.b", 'i', &conf_serif_color_b[20], true, false},
	{"serif.color21.outline.r", 'i', &conf_serif_outline_color_r[20], true, false},
	{"serif.color21.outline.g", 'i', &conf_serif_outline_color_g[20], true, false},
	{"serif.color21.outline.b", 'i', &conf_serif_outline_color_b[20], true, false},
	{"serif.color22.name", 's', &conf_serif_color_name[21], true, false},
	{"serif.color22.r", 'i', &conf_serif_color_r[21], true, false},
	{"serif.color22.g", 'i', &conf_serif_color_g[21], true, false},
	{"serif.color22.b", 'i', &conf_serif_color_b[21], true, false},
	{"serif.color22.outline.r", 'i', &conf_serif_outline_color_r[21], true, false},
	{"serif.color22.outline.g", 'i', &conf_serif_outline_color_g[21], true, false},
	{"serif.color22.outline.b", 'i', &conf_serif_outline_color_b[21], true, false},
	{"serif.color23.name", 's', &conf_serif_color_name[22], true, false},
	{"serif.color23.r", 'i', &conf_serif_color_r[22], true, false},
	{"serif.color23.g", 'i', &conf_serif_color_g[22], true, false},
	{"serif.color23.b", 'i', &conf_serif_color_b[22], true, false},
	{"serif.color23.outline.r", 'i', &conf_serif_outline_color_r[22], true, false},
	{"serif.color23.outline.g", 'i', &conf_serif_outline_color_g[22], true, false},
	{"serif.color23.outline.b", 'i', &conf_serif_outline_color_b[22], true, false},
	{"serif.color24.name", 's', &conf_serif_color_name[23], true, false},
	{"serif.color24.r", 'i', &conf_serif_color_r[23], true, false},
	{"serif.color24.g", 'i', &conf_serif_color_g[23], true, false},
	{"serif.color24.b", 'i', &conf_serif_color_b[23], true, false},
	{"serif.color24.outline.r", 'i', &conf_serif_outline_color_r[23], true, false},
	{"serif.color24.outline.g", 'i', &conf_serif_outline_color_g[23], true, false},
	{"serif.color24.outline.b", 'i', &conf_serif_outline_color_b[23], true, false},
	{"serif.color25.name", 's', &conf_serif_color_name[24], true, false},
	{"serif.color25.r", 'i', &conf_serif_color_r[24], true, false},
	{"serif.color25.g", 'i', &conf_serif_color_g[24], true, false},
	{"serif.color25.b", 'i', &conf_serif_color_b[24], true, false},
	{"serif.color25.outline.r", 'i', &conf_serif_outline_color_r[24], true, false},
	{"serif.color25.outline.g", 'i', &conf_serif_outline_color_g[24], true, false},
	{"serif.color25.outline.b", 'i', &conf_serif_outline_color_b[24], true, false},
	{"serif.color26.name", 's', &conf_serif_color_name[25], true, false},
	{"serif.color26.r", 'i', &conf_serif_color_r[25], true, false},
	{"serif.color26.g", 'i', &conf_serif_color_g[25], true, false},
	{"serif.color26.b", 'i', &conf_serif_color_b[25], true, false},
	{"serif.color26.outline.r", 'i', &conf_serif_outline_color_r[25], true, false},
	{"serif.color26.outline.g", 'i', &conf_serif_outline_color_g[25], true, false},
	{"serif.color26.outline.b", 'i', &conf_serif_outline_color_b[25], true, false},
	{"serif.color27.name", 's', &conf_serif_color_name[26], true, false},
	{"serif.color27.r", 'i', &conf_serif_color_r[26], true, false},
	{"serif.color27.g", 'i', &conf_serif_color_g[26], true, false},
	{"serif.color27.b", 'i', &conf_serif_color_b[26], true, false},
	{"serif.color27.outline.r", 'i', &conf_serif_outline_color_r[26], true, false},
	{"serif.color27.outline.g", 'i', &conf_serif_outline_color_g[26], true, false},
	{"serif.color27.outline.b", 'i', &conf_serif_outline_color_b[26], true, false},
	{"serif.color28.name", 's', &conf_serif_color_name[27], true, false},
	{"serif.color28.r", 'i', &conf_serif_color_r[27], true, false},
	{"serif.color28.g", 'i', &conf_serif_color_g[27], true, false},
	{"serif.color28.b", 'i', &conf_serif_color_b[27], true, false},
	{"serif.color28.outline.r", 'i', &conf_serif_outline_color_r[27], true, false},
	{"serif.color28.outline.g", 'i', &conf_serif_outline_color_g[27], true, false},
	{"serif.color28.outline.b", 'i', &conf_serif_outline_color_b[27], true, false},
	{"serif.color29.name", 's', &conf_serif_color_name[28], true, false},
	{"serif.color29.r", 'i', &conf_serif_color_r[28], true, false},
	{"serif.color29.g", 'i', &conf_serif_color_g[28], true, false},
	{"serif.color29.b", 'i', &conf_serif_color_b[28], true, false},
	{"serif.color29.outline.r", 'i', &conf_serif_outline_color_r[28], true, false},
	{"serif.color29.outline.g", 'i', &conf_serif_outline_color_g[28], true, false},
	{"serif.color29.outline.b", 'i', &conf_serif_outline_color_b[28], true, false},
	{"serif.color30.name", 's', &conf_serif_color_name[29], true, false},
	{"serif.color30.r", 'i', &conf_serif_color_r[29], true, false},
	{"serif.color30.g", 'i', &conf_serif_color_g[29], true, false},
	{"serif.color30.b", 'i', &conf_serif_color_b[29], true, false},
	{"serif.color30.outline.r", 'i', &conf_serif_outline_color_r[29], true, false},
	{"serif.color30.outline.g", 'i', &conf_serif_outline_color_g[29], true, false},
	{"serif.color30.outline.b", 'i', &conf_serif_outline_color_b[29], true, false},
	{"serif.color31.name", 's', &conf_serif_color_name[30], true, false},
	{"serif.color31.r", 'i', &conf_serif_color_r[30], true, false},
	{"serif.color31.g", 'i', &conf_serif_color_g[30], true, false},
	{"serif.color31.b", 'i', &conf_serif_color_b[30], true, false},
	{"serif.color31.outline.r", 'i', &conf_serif_outline_color_r[30], true, false},
	{"serif.color31.outline.g", 'i', &conf_serif_outline_color_g[30], true, false},
	{"serif.color31.outline.b", 'i', &conf_serif_outline_color_b[30], true, false},
	{"serif.color32.name", 's', &conf_serif_color_name[31], true, false},
	{"serif.color32.r", 'i', &conf_serif_color_r[31], true, false},
	{"serif.color32.g", 'i', &conf_serif_color_g[31], true, false},
	{"serif.color32.b", 'i', &conf_serif_color_b[31], true, false},
	{"serif.color32.outline.r", 'i', &conf_serif_outline_color_r[31], true, false},
	{"serif.color32.outline.g", 'i', &conf_serif_outline_color_g[31], true, false},
	{"serif.color32.outline.b", 'i', &conf_serif_outline_color_b[31], true, false},
	{"serif.color33.name", 's', &conf_serif_color_name[32], true, false},
	{"serif.color33.r", 'i', &conf_serif_color_r[32], true, false},
	{"serif.color33.g", 'i', &conf_serif_color_g[32], true, false},
	{"serif.color33.b", 'i', &conf_serif_color_b[32], true, false},
	{"serif.color33.outline.r", 'i', &conf_serif_outline_color_r[32], true, false},
	{"serif.color33.outline.g", 'i', &conf_serif_outline_color_g[32], true, false},
	{"serif.color33.outline.b", 'i', &conf_serif_outline_color_b[32], true, false},
	{"serif.color34.name", 's', &conf_serif_color_name[33], true, false},
	{"serif.color34.r", 'i', &conf_serif_color_r[33], true, false},
	{"serif.color34.g", 'i', &conf_serif_color_g[33], true, false},
	{"serif.color34.b", 'i', &conf_serif_color_b[33], true, false},
	{"serif.color34.outline.r", 'i', &conf_serif_outline_color_r[33], true, false},
	{"serif.color34.outline.g", 'i', &conf_serif_outline_color_g[33], true, false},
	{"serif.color34.outline.b", 'i', &conf_serif_outline_color_b[33], true, false},
	{"serif.color35.name", 's', &conf_serif_color_name[34], true, false},
	{"serif.color35.r", 'i', &conf_serif_color_r[34], true, false},
	{"serif.color35.g", 'i', &conf_serif_color_g[34], true, false},
	{"serif.color35.b", 'i', &conf_serif_color_b[34], true, false},
	{"serif.color35.outline.r", 'i', &conf_serif_outline_color_r[34], true, false},
	{"serif.color35.outline.g", 'i', &conf_serif_outline_color_g[34], true, false},
	{"serif.color35.outline.b", 'i', &conf_serif_outline_color_b[34], true, false},
	{"serif.color36.name", 's', &conf_serif_color_name[35], true, false},
	{"serif.color36.r", 'i', &conf_serif_color_r[35], true, false},
	{"serif.color36.g", 'i', &conf_serif_color_g[35], true, false},
	{"serif.color36.b", 'i', &conf_serif_color_b[35], true, false},
	{"serif.color36.outline.r", 'i', &conf_serif_outline_color_r[35], true, false},
	{"serif.color36.outline.g", 'i', &conf_serif_outline_color_g[35], true, false},
	{"serif.color36.outline.b", 'i', &conf_serif_outline_color_b[35], true, false},
	{"serif.color37.name", 's', &conf_serif_color_name[36], true, false},
	{"serif.color37.r", 'i', &conf_serif_color_r[36], true, false},
	{"serif.color37.g", 'i', &conf_serif_color_g[36], true, false},
	{"serif.color37.b", 'i', &conf_serif_color_b[36], true, false},
	{"serif.color37.outline.r", 'i', &conf_serif_outline_color_r[36], true, false},
	{"serif.color37.outline.g", 'i', &conf_serif_outline_color_g[36], true, false},
	{"serif.color37.outline.b", 'i', &conf_serif_outline_color_b[36], true, false},
	{"serif.color38.name", 's', &conf_serif_color_name[37], true, false},
	{"serif.color38.r", 'i', &conf_serif_color_r[37], true, false},
	{"serif.color38.g", 'i', &conf_serif_color_g[37], true, false},
	{"serif.color38.b", 'i', &conf_serif_color_b[37], true, false},
	{"serif.color38.outline.r", 'i', &conf_serif_outline_color_r[37], true, false},
	{"serif.color38.outline.g", 'i', &conf_serif_outline_color_g[37], true, false},
	{"serif.color38.outline.b", 'i', &conf_serif_outline_color_b[37], true, false},
	{"serif.color39.name", 's', &conf_serif_color_name[38], true, false},
	{"serif.color39.r", 'i', &conf_serif_color_r[38], true, false},
	{"serif.color39.g", 'i', &conf_serif_color_g[38], true, false},
	{"serif.color39.b", 'i', &conf_serif_color_b[38], true, false},
	{"serif.color39.outline.r", 'i', &conf_serif_outline_color_r[38], true, false},
	{"serif.color39.outline.g", 'i', &conf_serif_outline_color_g[38], true, false},
	{"serif.color39.outline.b", 'i', &conf_serif_outline_color_b[38], true, false},
	{"serif.color40.name", 's', &conf_serif_color_name[39], true, false},
	{"serif.color40.r", 'i', &conf_serif_color_r[39], true, false},
	{"serif.color40.g", 'i', &conf_serif_color_g[39], true, false},
	{"serif.color40.b", 'i', &conf_serif_color_b[39], true, false},
	{"serif.color40.outline.r", 'i', &conf_serif_outline_color_r[39], true, false},
	{"serif.color40.outline.g", 'i', &conf_serif_outline_color_g[39], true, false},
	{"serif.color40.outline.b", 'i', &conf_serif_outline_color_b[39], true, false},
	{"serif.color41.name", 's', &conf_serif_color_name[40], true, false},
	{"serif.color41.r", 'i', &conf_serif_color_r[40], true, false},
	{"serif.color41.g", 'i', &conf_serif_color_g[40], true, false},
	{"serif.color41.b", 'i', &conf_serif_color_b[40], true, false},
	{"serif.color41.outline.r", 'i', &conf_serif_outline_color_r[40], true, false},
	{"serif.color41.outline.g", 'i', &conf_serif_outline_color_g[40], true, false},
	{"serif.color41.outline.b", 'i', &conf_serif_outline_color_b[40], true, false},
	{"serif.color42.name", 's', &conf_serif_color_name[41], true, false},
	{"serif.color42.r", 'i', &conf_serif_color_r[41], true, false},
	{"serif.color42.g", 'i', &conf_serif_color_g[41], true, false},
	{"serif.color42.b", 'i', &conf_serif_color_b[41], true, false},
	{"serif.color42.outline.r", 'i', &conf_serif_outline_color_r[41], true, false},
	{"serif.color42.outline.g", 'i', &conf_serif_outline_color_g[41], true, false},
	{"serif.color42.outline.b", 'i', &conf_serif_outline_color_b[41], true, false},
	{"serif.color43.name", 's', &conf_serif_color_name[42], true, false},
	{"serif.color43.r", 'i', &conf_serif_color_r[42], true, false},
	{"serif.color43.g", 'i', &conf_serif_color_g[42], true, false},
	{"serif.color43.b", 'i', &conf_serif_color_b[42], true, false},
	{"serif.color43.outline.r", 'i', &conf_serif_outline_color_r[42], true, false},
	{"serif.color43.outline.g", 'i', &conf_serif_outline_color_g[42], true, false},
	{"serif.color43.outline.b", 'i', &conf_serif_outline_color_b[42], true, false},
	{"serif.color44.name", 's', &conf_serif_color_name[43], true, false},
	{"serif.color44.r", 'i', &conf_serif_color_r[43], true, false},
	{"serif.color44.g", 'i', &conf_serif_color_g[43], true, false},
	{"serif.color44.b", 'i', &conf_serif_color_b[43], true, false},
	{"serif.color44.outline.r", 'i', &conf_serif_outline_color_r[43], true, false},
	{"serif.color44.outline.g", 'i', &conf_serif_outline_color_g[43], true, false},
	{"serif.color44.outline.b", 'i', &conf_serif_outline_color_b[43], true, false},
	{"serif.color45.name", 's', &conf_serif_color_name[44], true, false},
	{"serif.color45.r", 'i', &conf_serif_color_r[44], true, false},
	{"serif.color45.g", 'i', &conf_serif_color_g[44], true, false},
	{"serif.color45.b", 'i', &conf_serif_color_b[44], true, false},
	{"serif.color45.outline.r", 'i', &conf_serif_outline_color_r[44], true, false},
	{"serif.color45.outline.g", 'i', &conf_serif_outline_color_g[44], true, false},
	{"serif.color45.outline.b", 'i', &conf_serif_outline_color_b[44], true, false},
	{"serif.color46.name", 's', &conf_serif_color_name[45], true, false},
	{"serif.color46.r", 'i', &conf_serif_color_r[45], true, false},
	{"serif.color46.g", 'i', &conf_serif_color_g[45], true, false},
	{"serif.color46.b", 'i', &conf_serif_color_b[45], true, false},
	{"serif.color46.outline.r", 'i', &conf_serif_outline_color_r[45], true, false},
	{"serif.color46.outline.g", 'i', &conf_serif_outline_color_g[45], true, false},
	{"serif.color46.outline.b", 'i', &conf_serif_outline_color_b[45], true, false},
	{"serif.color47.name", 's', &conf_serif_color_name[46], true, false},
	{"serif.color47.r", 'i', &conf_serif_color_r[46], true, false},
	{"serif.color47.g", 'i', &conf_serif_color_g[46], true, false},
	{"serif.color47.b", 'i', &conf_serif_color_b[46], true, false},
	{"serif.color47.outline.r", 'i', &conf_serif_outline_color_r[46], true, false},
	{"serif.color47.outline.g", 'i', &conf_serif_outline_color_g[46], true, false},
	{"serif.color47.outline.b", 'i', &conf_serif_outline_color_b[46], true, false},
	{"serif.color48.name", 's', &conf_serif_color_name[47], true, false},
	{"serif.color48.r", 'i', &conf_serif_color_r[47], true, false},
	{"serif.color48.g", 'i', &conf_serif_color_g[47], true, false},
	{"serif.color48.b", 'i', &conf_serif_color_b[47], true, false},
	{"serif.color48.outline.r", 'i', &conf_serif_outline_color_r[47], true, false},
	{"serif.color48.outline.g", 'i', &conf_serif_outline_color_g[47], true, false},
	{"serif.color48.outline.b", 'i', &conf_serif_outline_color_b[47], true, false},
	{"serif.color49.name", 's', &conf_serif_color_name[48], true, false},
	{"serif.color49.r", 'i', &conf_serif_color_r[48], true, false},
	{"serif.color49.g", 'i', &conf_serif_color_g[48], true, false},
	{"serif.color49.b", 'i', &conf_serif_color_b[48], true, false},
	{"serif.color49.outline.r", 'i', &conf_serif_outline_color_r[48], true, false},
	{"serif.color49.outline.g", 'i', &conf_serif_outline_color_g[48], true, false},
	{"serif.color49.outline.b", 'i', &conf_serif_outline_color_b[48], true, false},
	{"serif.color50.name", 's', &conf_serif_color_name[49], true, false},
	{"serif.color50.r", 'i', &conf_serif_color_r[49], true, false},
	{"serif.color50.g", 'i', &conf_serif_color_g[49], true, false},
	{"serif.color50.b", 'i', &conf_serif_color_b[49], true, false},
	{"serif.color50.outline.r", 'i', &conf_serif_outline_color_r[49], true, false},
	{"serif.color50.outline.g", 'i', &conf_serif_outline_color_g[49], true, false},
	{"serif.color50.outline.b", 'i', &conf_serif_outline_color_b[49], true, false},
	{"serif.color51.name", 's', &conf_serif_color_name[50], true, false},
	{"serif.color51.r", 'i', &conf_serif_color_r[50], true, false},
	{"serif.color51.g", 'i', &conf_serif_color_g[50], true, false},
	{"serif.color51.b", 'i', &conf_serif_color_b[50], true, false},
	{"serif.color51.outline.r", 'i', &conf_serif_outline_color_r[50], true, false},
	{"serif.color51.outline.g", 'i', &conf_serif_outline_color_g[50], true, false},
	{"serif.color51.outline.b", 'i', &conf_serif_outline_color_b[50], true, false},
	{"serif.color52.name", 's', &conf_serif_color_name[51], true, false},
	{"serif.color52.r", 'i', &conf_serif_color_r[51], true, false},
	{"serif.color52.g", 'i', &conf_serif_color_g[51], true, false},
	{"serif.color52.b", 'i', &conf_serif_color_b[51], true, false},
	{"serif.color52.outline.r", 'i', &conf_serif_outline_color_r[51], true, false},
	{"serif.color52.outline.g", 'i', &conf_serif_outline_color_g[51], true, false},
	{"serif.color52.outline.b", 'i', &conf_serif_outline_color_b[51], true, false},
	{"serif.color53.name", 's', &conf_serif_color_name[52], true, false},
	{"serif.color53.r", 'i', &conf_serif_color_r[52], true, false},
	{"serif.color53.g", 'i', &conf_serif_color_g[52], true, false},
	{"serif.color53.b", 'i', &conf_serif_color_b[52], true, false},
	{"serif.color53.outline.r", 'i', &conf_serif_outline_color_r[52], true, false},
	{"serif.color53.outline.g", 'i', &conf_serif_outline_color_g[52], true, false},
	{"serif.color53.outline.b", 'i', &conf_serif_outline_color_b[52], true, false},
	{"serif.color54.name", 's', &conf_serif_color_name[53], true, false},
	{"serif.color54.r", 'i', &conf_serif_color_r[53], true, false},
	{"serif.color54.g", 'i', &conf_serif_color_g[53], true, false},
	{"serif.color54.b", 'i', &conf_serif_color_b[53], true, false},
	{"serif.color54.outline.r", 'i', &conf_serif_outline_color_r[53], true, false},
	{"serif.color54.outline.g", 'i', &conf_serif_outline_color_g[53], true, false},
	{"serif.color54.outline.b", 'i', &conf_serif_outline_color_b[53], true, false},
	{"serif.color55.name", 's', &conf_serif_color_name[54], true, false},
	{"serif.color55.r", 'i', &conf_serif_color_r[54], true, false},
	{"serif.color55.g", 'i', &conf_serif_color_g[54], true, false},
	{"serif.color55.b", 'i', &conf_serif_color_b[54], true, false},
	{"serif.color55.outline.r", 'i', &conf_serif_outline_color_r[54], true, false},
	{"serif.color55.outline.g", 'i', &conf_serif_outline_color_g[54], true, false},
	{"serif.color55.outline.b", 'i', &conf_serif_outline_color_b[54], true, false},
	{"serif.color56.name", 's', &conf_serif_color_name[55], true, false},
	{"serif.color56.r", 'i', &conf_serif_color_r[55], true, false},
	{"serif.color56.g", 'i', &conf_serif_color_g[55], true, false},
	{"serif.color56.b", 'i', &conf_serif_color_b[55], true, false},
	{"serif.color56.outline.r", 'i', &conf_serif_outline_color_r[55], true, false},
	{"serif.color56.outline.g", 'i', &conf_serif_outline_color_g[55], true, false},
	{"serif.color56.outline.b", 'i', &conf_serif_outline_color_b[55], true, false},
	{"serif.color57.name", 's', &conf_serif_color_name[56], true, false},
	{"serif.color57.r", 'i', &conf_serif_color_r[56], true, false},
	{"serif.color57.g", 'i', &conf_serif_color_g[56], true, false},
	{"serif.color57.b", 'i', &conf_serif_color_b[56], true, false},
	{"serif.color57.outline.r", 'i', &conf_serif_outline_color_r[56], true, false},
	{"serif.color57.outline.g", 'i', &conf_serif_outline_color_g[56], true, false},
	{"serif.color57.outline.b", 'i', &conf_serif_outline_color_b[56], true, false},
	{"serif.color58.name", 's', &conf_serif_color_name[57], true, false},
	{"serif.color58.r", 'i', &conf_serif_color_r[57], true, false},
	{"serif.color58.g", 'i', &conf_serif_color_g[57], true, false},
	{"serif.color58.b", 'i', &conf_serif_color_b[57], true, false},
	{"serif.color58.outline.r", 'i', &conf_serif_outline_color_r[57], true, false},
	{"serif.color58.outline.g", 'i', &conf_serif_outline_color_g[57], true, false},
	{"serif.color58.outline.b", 'i', &conf_serif_outline_color_b[57], true, false},
	{"serif.color59.name", 's', &conf_serif_color_name[58], true, false},
	{"serif.color59.r", 'i', &conf_serif_color_r[58], true, false},
	{"serif.color59.g", 'i', &conf_serif_color_g[58], true, false},
	{"serif.color59.b", 'i', &conf_serif_color_b[58], true, false},
	{"serif.color59.outline.r", 'i', &conf_serif_outline_color_r[58], true, false},
	{"serif.color59.outline.g", 'i', &conf_serif_outline_color_g[58], true, false},
	{"serif.color59.outline.b", 'i', &conf_serif_outline_color_b[58], true, false},
	{"serif.color60.name", 's', &conf_serif_color_name[59], true, false},
	{"serif.color60.r", 'i', &conf_serif_color_r[59], true, false},
	{"serif.color60.g", 'i', &conf_serif_color_g[59], true, false},
	{"serif.color60.b", 'i', &conf_serif_color_b[59], true, false},
	{"serif.color60.outline.r", 'i', &conf_serif_outline_color_r[59], true, false},
	{"serif.color60.outline.g", 'i', &conf_serif_outline_color_g[59], true, false},
	{"serif.color60.outline.b", 'i', &conf_serif_outline_color_b[59], true, false},
	{"serif.color61.name", 's', &conf_serif_color_name[60], true, false},
	{"serif.color61.r", 'i', &conf_serif_color_r[60], true, false},
	{"serif.color61.g", 'i', &conf_serif_color_g[60], true, false},
	{"serif.color61.b", 'i', &conf_serif_color_b[60], true, false},
	{"serif.color61.outline.r", 'i', &conf_serif_outline_color_r[60], true, false},
	{"serif.color61.outline.g", 'i', &conf_serif_outline_color_g[60], true, false},
	{"serif.color61.outline.b", 'i', &conf_serif_outline_color_b[60], true, false},
	{"serif.color62.name", 's', &conf_serif_color_name[61], true, false},
	{"serif.color62.r", 'i', &conf_serif_color_r[61], true, false},
	{"serif.color62.g", 'i', &conf_serif_color_g[61], true, false},
	{"serif.color62.b", 'i', &conf_serif_color_b[61], true, false},
	{"serif.color62.outline.r", 'i', &conf_serif_outline_color_r[61], true, false},
	{"serif.color62.outline.g", 'i', &conf_serif_outline_color_g[61], true, false},
	{"serif.color62.outline.b", 'i', &conf_serif_outline_color_b[61], true, false},
	{"serif.color63.name", 's', &conf_serif_color_name[62], true, false},
	{"serif.color63.r", 'i', &conf_serif_color_r[62], true, false},
	{"serif.color63.g", 'i', &conf_serif_color_g[62], true, false},
	{"serif.color63.b", 'i', &conf_serif_color_b[62], true, false},
	{"serif.color63.outline.r", 'i', &conf_serif_outline_color_r[62], true, false},
	{"serif.color63.outline.g", 'i', &conf_serif_outline_color_g[62], true, false},
	{"serif.color63.outline.b", 'i', &conf_serif_outline_color_b[62], true, false},
	{"serif.color64.name", 's', &conf_serif_color_name[63], true, false},
	{"serif.color64.r", 'i', &conf_serif_color_r[63], true, false},
	{"serif.color64.g", 'i', &conf_serif_color_g[63], true, false},
	{"serif.color64.b", 'i', &conf_serif_color_b[63], true, false},
	{"serif.color64.outline.r", 'i', &conf_serif_outline_color_r[63], true, false},
	{"serif.color64.outline.g", 'i', &conf_serif_outline_color_g[63], true, false},
	{"serif.color64.outline.b", 'i', &conf_serif_outline_color_b[63], true, false},
	/* end codegen */
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

		/* すでに値が設定されたキーの場合 */
		if (rule_tbl[i].loaded) {
			log_duplicated_conf(k);
			return false;
		}

		/* 保存されない(無視される)キーの場合 */
		if (rule_tbl[i].val == NULL)
			return true;;

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
