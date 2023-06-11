/* -*- coding: utf-8; tab-width: 8; indent-tabs-mode: t; -*- */

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

#include "suika.h"

/*
 * 言語の設定
 */
char *conf_language_en;
char *conf_language_fr;
char *conf_language_de;
char *conf_language_es;
char *conf_language_it;
char *conf_language_el;
char *conf_language_ru;
char *conf_language_zh;
char *conf_language_tw;
char *conf_language_ja;
char *conf_language_other;

/* 下記の変数はconfig.txtに記述されず、実行環境と上記の設定から導出される */
int conf_locale;
const char *conf_locale_mapped;

/*
 * ウィンドウの設定
 */
char *conf_window_title;
int conf_window_width;
int conf_window_height;
int conf_window_white;
int conf_window_menubar;

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
int conf_font_outline_remove;

/*
 * 名前ボックスの設定
 */
char *conf_namebox_file;
int conf_namebox_x;
int conf_namebox_y;
int conf_namebox_margin_top;
int conf_namebox_centering_no;
int conf_namebox_margin_left;
int conf_namebox_hidden;

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
int conf_msgbox_btn_config_x;
int conf_msgbox_btn_config_y;
int conf_msgbox_btn_config_width;
int conf_msgbox_btn_config_height;
int conf_msgbox_btn_hide_x;
int conf_msgbox_btn_hide_y;
int conf_msgbox_btn_hide_width;
int conf_msgbox_btn_hide_height;
char *conf_msgbox_btn_qsave_se;
char *conf_msgbox_btn_qload_se;
char *conf_msgbox_btn_save_se;
char *conf_msgbox_btn_load_se;
char *conf_msgbox_btn_auto_se;
char *conf_msgbox_btn_skip_se;
char *conf_msgbox_btn_history_se;
char *conf_msgbox_btn_config_se;
char *conf_msgbox_btn_change_se;
char *conf_msgbox_save_se;
char *conf_msgbox_history_se;
char *conf_msgbox_config_se;
char *conf_msgbox_hide_se;
char *conf_msgbox_show_se;
char *conf_msgbox_auto_cancel_se;
char *conf_msgbox_skip_cancel_se;
int conf_msgbox_skip_unseen;
int conf_msgbox_dim;
int conf_msgbox_dim_color_r;
int conf_msgbox_dim_color_g;
int conf_msgbox_dim_color_b;
int conf_msgbox_dim_color_outline_r;
int conf_msgbox_dim_color_outline_g;
int conf_msgbox_dim_color_outline_b;

/*
 * クリックアニメーションの設定
 */
int conf_click_x;
int conf_click_y;
int conf_click_move;
char *conf_click_file1;
char *conf_click_file2;
char *conf_click_file3;
char *conf_click_file4;
char *conf_click_file5;
char *conf_click_file6;
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
int conf_switch_color_active;
int conf_switch_color_active_body_r;
int conf_switch_color_active_body_g;
int conf_switch_color_active_body_b;
int conf_switch_color_active_outline_r;
int conf_switch_color_active_outline_g;
int conf_switch_color_active_outline_b;
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
int conf_save_data_thumb_width;
int conf_save_data_thumb_height;
int conf_save_data_delete_x;
int conf_save_data_delete_y;
int conf_save_data_delete_width;
int conf_save_data_delete_height;
int conf_save_data1_x;
int conf_save_data1_y;
int conf_save_data2_x;
int conf_save_data2_y;
int conf_save_data3_x;
int conf_save_data3_y;
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
 * メニュー(@menu)の設定
 */
char *conf_menu_change_se;

/* 
 * システムメニューの設定
 */
int conf_sysmenu_x;
int conf_sysmenu_y;
char *conf_sysmenu_idle_file;
char *conf_sysmenu_hover_file;
char *conf_sysmenu_disable_file;
int conf_sysmenu_qsave_x;
int conf_sysmenu_qsave_y;
int conf_sysmenu_qsave_width;
int conf_sysmenu_qsave_height;
int conf_sysmenu_qload_x;
int conf_sysmenu_qload_y;
int conf_sysmenu_qload_width;
int conf_sysmenu_qload_height;
int conf_sysmenu_save_x;
int conf_sysmenu_save_y;
int conf_sysmenu_save_width;
int conf_sysmenu_save_height;
int conf_sysmenu_load_x;
int conf_sysmenu_load_y;
int conf_sysmenu_load_width;
int conf_sysmenu_load_height;
int conf_sysmenu_auto_x;
int conf_sysmenu_auto_y;
int conf_sysmenu_auto_width;
int conf_sysmenu_auto_height;
int conf_sysmenu_skip_x;
int conf_sysmenu_skip_y;
int conf_sysmenu_skip_width;
int conf_sysmenu_skip_height;
int conf_sysmenu_history_x;
int conf_sysmenu_history_y;
int conf_sysmenu_history_width;
int conf_sysmenu_history_height;
int conf_sysmenu_config_x;
int conf_sysmenu_config_y;
int conf_sysmenu_config_width;
int conf_sysmenu_config_height;
char *conf_sysmenu_enter_se;
char *conf_sysmenu_leave_se;
char *conf_sysmenu_change_se;
char *conf_sysmenu_qsave_se;
char *conf_sysmenu_qload_se;
char *conf_sysmenu_save_se;
char *conf_sysmenu_load_se;
char *conf_sysmenu_auto_se;
char *conf_sysmenu_skip_se;
char *conf_sysmenu_history_se;
char *conf_sysmenu_config_se;
int conf_sysmenu_collapsed_x;
int conf_sysmenu_collapsed_y;
char *conf_sysmenu_collapsed_idle_file;
char *conf_sysmenu_collapsed_hover_file;
char *conf_sysmenu_collapsed_se;
int conf_sysmenu_hidden;

/*
 * オートモードの設定
 */
char *conf_automode_banner_file;
int conf_automode_banner_x;
int conf_automode_banner_y;
float conf_automode_speed;

/*
 * スキップモードの設定
 */
char *conf_skipmode_banner_file;
int conf_skipmode_banner_x;
int conf_skipmode_banner_y;

/*
 * サウンドの設定
 */
float conf_sound_vol_bgm;
float conf_sound_vol_voice;
float conf_sound_vol_se;
float conf_sound_vol_character;
char *conf_sound_character_name[CH_VOL_SLOTS]; /* index0は未使用 */

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

/* フルスクリーンにしない */
int conf_window_fullscreen_disable;

/* 最大化ボタンを無効にする */
int conf_window_maximize_disable;

/* ウィンドウタイトルと章タイトルの間の文字列 */
char *conf_window_title_separator;

/* 章タイトルをウィンドウタイトルに反映しない */
int conf_window_title_chapter_disable;

/* クリックアニメーションを点滅させない */
int conf_click_disable;

/* キャラクタの変更中にメッセージボックスを隠さない */
int conf_msgbox_show_on_ch;

/* 背景の変更中にメッセージボックスを隠さない */
int conf_msgbox_show_on_bg;

/* ビープの調整 */
float conf_beep_adjustment;

/* セリフをカギカッコで囲う */
int conf_serif_quote;

/* ch,cha,chs,bgの最中に折りたたみメッセージボックスを表示する */
int conf_sysmenu_transition;

/* リリース版であるか */
int conf_release;

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
	{"i18n", 'i', NULL, true, false}, /* deprecated */
	{"language.en", 's', &conf_language_en, true, false},
	{"language.fr", 's', &conf_language_fr, true, false},
	{"language.de", 's', &conf_language_de, true, false},
	{"language.es", 's', &conf_language_es, true, false},
	{"language.it", 's', &conf_language_it, true, false},
	{"language.el", 's', &conf_language_el, true, false},
	{"language.ru", 's', &conf_language_ru, true, false},
	{"language.zh", 's', &conf_language_zh, true, false},
	{"language.tw", 's', &conf_language_tw, true, false},
	{"language.ja", 's', &conf_language_ja, true, false},
	{"language.other", 's', &conf_language_other, true, false},
	{"window.title", 's', &conf_window_title, false, false},
	{"window.width", 'i', &conf_window_width, false, false},
	{"window.height", 'i', &conf_window_height, false, false},
	{"window.white", 'i', &conf_window_white, false, false},
	{"window.menubar", 'i', &conf_window_menubar, true, false},
	{"font.file", 's', &conf_font_file, false, false},
	{"font.size", 'i', &conf_font_size, false, false},
	{"font.color.r", 'i', &conf_font_color_r, false, false},
	{"font.color.g", 'i', &conf_font_color_g, false, false},
	{"font.color.b", 'i', &conf_font_color_b, false, false},
	{"font.outline.color.r", 'i', &conf_font_outline_color_r, true, false},
	{"font.outline.color.g", 'i', &conf_font_outline_color_g, true, false},
	{"font.outline.color.b", 'i', &conf_font_outline_color_b, true, false},
	{"font.outline.remove", 'i', &conf_font_outline_remove, true, false},
	{"namebox.file", 's', &conf_namebox_file, false, false},
	{"namebox.x", 'i', &conf_namebox_x, false, false},
	{"namebox.y", 'i', &conf_namebox_y, false, false},
	{"namebox.margin.top", 'i', &conf_namebox_margin_top, false, false},
	{"namebox.centering.no", 'i', &conf_namebox_centering_no, true, false},
	{"namebox.margin.left", 'i', &conf_namebox_margin_left, true, false},
	{"namebox.hidden", 'i', &conf_namebox_hidden, true, false},
	{"msgbox.bg.file", 's', &conf_msgbox_bg_file, false, false},
	{"msgbox.fg.file", 's', &conf_msgbox_fg_file, false, false},
	{"msgbox.x", 'i', &conf_msgbox_x, false, false},
	{"msgbox.y", 'i', &conf_msgbox_y, false, false},
	{"msgbox.margin.left", 'i', &conf_msgbox_margin_left, false, false},
	{"msgbox.margin.top", 'i', &conf_msgbox_margin_top, false, false},
	{"msgbox.margin.right", 'i', &conf_msgbox_margin_right, false, false},
	{"msgbox.margin.line", 'i', &conf_msgbox_margin_line, false, false},
	{"msgbox.speed", 'f', &conf_msgbox_speed, false, false},
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
	{"msgbox.btn.config.x", 'i', &conf_msgbox_btn_config_x, true, false},
	{"msgbox.btn.config.y", 'i', &conf_msgbox_btn_config_y, true, false},
	{"msgbox.btn.config.width", 'i', &conf_msgbox_btn_config_width, true, false},
	{"msgbox.btn.config.height", 'i', &conf_msgbox_btn_config_height, true, false},
	{"msgbox.btn.hide.x", 'i', &conf_msgbox_btn_hide_x, true, false},
	{"msgbox.btn.hide.y", 'i', &conf_msgbox_btn_hide_y, true, false},
	{"msgbox.btn.hide.width", 'i', &conf_msgbox_btn_hide_width, true, false},
	{"msgbox.btn.hide.height", 'i', &conf_msgbox_btn_hide_height, true, false},
	{"msgbox.btn.qsave.se", 's', &conf_msgbox_btn_qsave_se, true, false},
	{"msgbox.btn.qload.se", 's', &conf_msgbox_btn_qload_se, true, false},
	{"msgbox.btn.save.se", 's', &conf_msgbox_btn_save_se, true, false},
	{"msgbox.btn.load.se", 's', &conf_msgbox_btn_load_se, true, false},
	{"msgbox.btn.auto.se", 's', &conf_msgbox_btn_auto_se, true, false},
	{"msgbox.btn.history.se", 's', &conf_msgbox_btn_history_se, true, false},
	{"msgbox.btn.config.se", 's', &conf_msgbox_btn_config_se, true, false},
	{"msgbox.btn.change.se", 's', &conf_msgbox_btn_change_se, true, false},
	{"msgbox.save.se", 's', &conf_msgbox_save_se, true, false}, /* TODO: remove */
	{"msgbox.history.se", 's', &conf_msgbox_history_se, true, false},
	{"msgbox.config.se", 's', &conf_msgbox_config_se, true, false},
	{"msgbox.auto.cancel.se", 's', &conf_msgbox_auto_cancel_se, true, false},
	{"msgbox.skip.cancel.se", 's', &conf_msgbox_skip_cancel_se, true, false},
	{"msgbox.hide.se", 's', &conf_msgbox_hide_se, true, false},
	{"msgbox.show.se", 's', &conf_msgbox_show_se, true, false},
	{"msgbox.skip.unseen", 'i', &conf_msgbox_skip_unseen, true, false},
	{"msgbox.dim", 'i', &conf_msgbox_dim, true, false},
	{"msgbox.dim.color.r", 'i', &conf_msgbox_dim_color_r, true, false},
	{"msgbox.dim.color.g", 'i', &conf_msgbox_dim_color_g, true, false},
	{"msgbox.dim.color.b", 'i', &conf_msgbox_dim_color_b, true, false},
	{"msgbox.dim.color.outline.r", 'i', &conf_msgbox_dim_color_outline_r, true, false},
	{"msgbox.dim.color.outline.g", 'i', &conf_msgbox_dim_color_outline_g, true, false},
	{"msgbox.dim.color.outline.b", 'i', &conf_msgbox_dim_color_outline_b, true, false},
	{"click.x", 'i', &conf_click_x, false, false},
	{"click.y", 'i', &conf_click_y, false, false},
	{"click.move", 'i', &conf_click_move, true, false},
	{"click.file1", 's', &conf_click_file1, false, false},
	{"click.file2", 's', &conf_click_file2, true, false},
	{"click.file3", 's', &conf_click_file3, true, false},
	{"click.file4", 's', &conf_click_file4, true, false},
	{"click.file5", 's', &conf_click_file5, true, false},
	{"click.file6", 's', &conf_click_file6, true, false},
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
	{"switch.color.active", 'i', &conf_switch_color_active, true, false},
	{"switch.color.active.body.r", 'i', &conf_switch_color_active_body_r, true, false},
	{"switch.color.active.body.g", 'i', &conf_switch_color_active_body_g, true, false},
	{"switch.color.active.body.b", 'i', &conf_switch_color_active_body_b, true, false},
	{"switch.color.active.outline.r", 'i', &conf_switch_color_active_outline_r, true, false},
	{"switch.color.active.outline.g", 'i', &conf_switch_color_active_outline_g, true, false},
	{"switch.color.active.outline.b", 'i', &conf_switch_color_active_outline_b, true, false},
	{"switch.parent.click.se.file", 's', &conf_switch_parent_click_se_file, true, false},
	{"switch.child.click.se.file", 's', &conf_switch_child_click_se_file, true, false},
	{"switch.change.se", 's', &conf_switch_change_se, true, false},
	{"news.bg.file", 's', &conf_news_bg_file, true, false},
	{"news.fg.file", 's', &conf_news_fg_file, true, false},
	{"news.margin", 'i', &conf_news_margin, true, false},
	{"news.text.margin.y", 'i', &conf_news_text_margin_y, true, false},
	{"news.change.se", 's', &conf_news_change_se, true, false},
	{"retrospect.change.se", 's', &conf_retrospect_change_se, true, false},
	{"save.data.thumb.width", 'i', &conf_save_data_thumb_width, false, false},
	{"save.data.thumb.height", 'i', &conf_save_data_thumb_height, false, false},
	{"menu.change.se", 's', &conf_menu_change_se, true, false},
	{"sysmenu.x", 'i', &conf_sysmenu_x, false, false},
	{"sysmenu.y", 'i', &conf_sysmenu_y, false, false},
	{"sysmenu.idle.file", 's', &conf_sysmenu_idle_file, false, false},
	{"sysmenu.hover.file", 's', &conf_sysmenu_hover_file, false, false},
	{"sysmenu.disable.file", 's', &conf_sysmenu_disable_file, false, false},
	{"sysmenu.qsave.x", 'i', &conf_sysmenu_qsave_x, false, false},
	{"sysmenu.qsave.y", 'i', &conf_sysmenu_qsave_y, false, false},
	{"sysmenu.qsave.width", 'i', &conf_sysmenu_qsave_width, false, false},
	{"sysmenu.qsave.height", 'i', &conf_sysmenu_qsave_height, false, false},
	{"sysmenu.qload.x", 'i', &conf_sysmenu_qload_x, false, false},
	{"sysmenu.qload.y", 'i', &conf_sysmenu_qload_y, false, false},
	{"sysmenu.qload.width", 'i', &conf_sysmenu_qload_width, false, false},
	{"sysmenu.qload.height", 'i', &conf_sysmenu_qload_height, false, false},
	{"sysmenu.save.x", 'i', &conf_sysmenu_save_x, false, false},
	{"sysmenu.save.y", 'i', &conf_sysmenu_save_y, false, false},
	{"sysmenu.save.width", 'i', &conf_sysmenu_save_width, false, false},
	{"sysmenu.save.height", 'i', &conf_sysmenu_save_height, false, false},
	{"sysmenu.load.x", 'i', &conf_sysmenu_load_x, false, false},
	{"sysmenu.load.y", 'i', &conf_sysmenu_load_y, false, false},
	{"sysmenu.load.width", 'i', &conf_sysmenu_load_width, false, false},
	{"sysmenu.load.height", 'i', &conf_sysmenu_load_height, false, false},
	{"sysmenu.auto.x", 'i', &conf_sysmenu_auto_x, false, false},
	{"sysmenu.auto.y", 'i', &conf_sysmenu_auto_y, false, false},
	{"sysmenu.auto.width", 'i', &conf_sysmenu_auto_width, false, false},
	{"sysmenu.auto.height", 'i', &conf_sysmenu_auto_height, false, false},
	{"sysmenu.skip.x", 'i', &conf_sysmenu_skip_x, false, false},
	{"sysmenu.skip.y", 'i', &conf_sysmenu_skip_y, false, false},
	{"sysmenu.skip.width", 'i', &conf_sysmenu_skip_width, false, false},
	{"sysmenu.skip.height", 'i', &conf_sysmenu_skip_height, false, false},
	{"sysmenu.history.x", 'i', &conf_sysmenu_history_x, false, false},
	{"sysmenu.history.y", 'i', &conf_sysmenu_history_y, false, false},
	{"sysmenu.history.width", 'i', &conf_sysmenu_history_width, false, false},
	{"sysmenu.history.height", 'i', &conf_sysmenu_history_height, false, false},
	{"sysmenu.config.x", 'i', &conf_sysmenu_config_x, false, false},
	{"sysmenu.config.y", 'i', &conf_sysmenu_config_y, false, false},
	{"sysmenu.config.width", 'i', &conf_sysmenu_config_width, false, false},
	{"sysmenu.config.height", 'i', &conf_sysmenu_config_height, false, false},
	{"sysmenu.enter.se", 's', &conf_sysmenu_enter_se, true, false},
	{"sysmenu.leave.se", 's', &conf_sysmenu_leave_se, true, false},
	{"sysmenu.change.se", 's', &conf_sysmenu_change_se, true, false},
	{"sysmenu.qsave.se", 's', &conf_sysmenu_qsave_se, true, false},
	{"sysmenu.qload.se", 's', &conf_sysmenu_qload_se, true, false},
	{"sysmenu.save.se", 's', &conf_sysmenu_save_se, true, false},
	{"sysmenu.load.se", 's', &conf_sysmenu_load_se, true, false},
	{"sysmenu.auto.se", 's', &conf_sysmenu_auto_se, true, false},
	{"sysmenu.skip.se", 's', &conf_sysmenu_skip_se, true, false},
	{"sysmenu.history.se", 's', &conf_sysmenu_history_se, true, false},
	{"sysmenu.config.se", 's', &conf_sysmenu_config_se, true, false},
	{"sysmenu.config.se", 's', &conf_sysmenu_config_se, true, false},
	{"sysmenu.hidden", 'i', &conf_sysmenu_hidden, true, false},
	{"automode.banner.file", 's', &conf_automode_banner_file, false, false},
	{"automode.banner.x", 'i', &conf_automode_banner_x, false, false},
	{"automode.banner.y", 'i', &conf_automode_banner_y, false, false},
	{"automode.speed", 'f', &conf_automode_speed, false, false},
	{"skipmode.banner.file", 's', &conf_skipmode_banner_file, false, false},
	{"skipmode.banner.x", 'i', &conf_skipmode_banner_x, false, false},
	{"skipmode.banner.y", 'i', &conf_skipmode_banner_y, false, false},
	{"sysmenu.collapsed.x", 'i', &conf_sysmenu_collapsed_x, false, false},
	{"sysmenu.collapsed.y", 'i', &conf_sysmenu_collapsed_y, false, false},
	{"sysmenu.collapsed.idle.file", 's', &conf_sysmenu_collapsed_idle_file, false, false},
	{"sysmenu.collapsed.hover.file", 's', &conf_sysmenu_collapsed_hover_file, false, false},
	{"sysmenu.collapsed.se", 's', &conf_sysmenu_collapsed_se, true, false},
	{"sound.vol.bgm", 'f', &conf_sound_vol_bgm, false, false},
	{"sound.vol.voice", 'f', &conf_sound_vol_voice, false, false},
	{"sound.vol.se", 'f', &conf_sound_vol_se, false, false},
	{"sound.vol.character", 'f', &conf_sound_vol_character, false, false},
	{"sound.character.name1", 's', &conf_sound_character_name[1], true, false},
	{"sound.character.name2", 's', &conf_sound_character_name[2], true, false},
	{"sound.character.name3", 's', &conf_sound_character_name[3], true, false},
	{"sound.character.name4", 's', &conf_sound_character_name[4], true, false},
	{"sound.character.name5", 's', &conf_sound_character_name[5], true, false},
	{"sound.character.name6", 's', &conf_sound_character_name[6], true, false},
	{"sound.character.name7", 's', &conf_sound_character_name[7], true, false},
	{"sound.character.name8", 's', &conf_sound_character_name[8], true, false},
	{"sound.character.name9", 's', &conf_sound_character_name[9], true, false},
	{"sound.character.name10", 's', &conf_sound_character_name[10], true, false},
	{"sound.character.name11", 's', &conf_sound_character_name[11], true, false},
	{"sound.character.name12", 's', &conf_sound_character_name[12], true, false},
	{"sound.character.name13", 's', &conf_sound_character_name[13], true, false},
	{"sound.character.name14", 's', &conf_sound_character_name[14], true, false},
	{"sound.character.name15", 's', &conf_sound_character_name[15], true, false},
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
	{"ui.msg.quit", 's', NULL, true, false}, /* deprecated */
	{"ui.msg.title", 's', NULL, true, false}, /* deprecated */
	{"ui.msg.delete", 's', NULL, true, false}, /* deprecated */
	{"ui.msg.overwrite", 's', NULL, true, false}, /* deprecated */
	{"ui.msg.default", 's', NULL, true, false}, /* deprecated */
	{"voice.stop.off", 'i', &conf_voice_stop_off, true, false},
	{"window.fullscreen.disable", 'i', &conf_window_fullscreen_disable, true, false},
	{"window.maximize.disable", 'i', &conf_window_maximize_disable, true, false},
	{"window.title.separator", 's', &conf_window_title_separator, true, false},
	{"window.title.chapter.disable", 'i', &conf_window_title_chapter_disable, true, false},
	{"click.disable", 'i', &conf_click_disable, true, false},
	{"msgbox.show.on.ch", 'i', &conf_msgbox_show_on_ch, true, false},
	{"msgbox.show.on.bg", 'i', &conf_msgbox_show_on_bg, true, false},
	{"beep.adjustment", 'f', &conf_beep_adjustment, true, false},
	{"serif.quote", 'i', &conf_serif_quote, true, false},
	{"sysmenu.transition", 'i', &conf_sysmenu_transition, true, false},
	{"release", 'i', &conf_release, true, false},
};

#define RULE_TBL_SIZE	(sizeof(rule_tbl) / sizeof(struct rule))

/*
 * 前方参照
 */
static bool read_conf(void);
static bool save_value(const char *k, const char *v);
static bool check_conf(void);
static void set_locale_mapping(void);

/*
 * コンフィグの初期化処理を行う
 */
bool init_conf(void)
{
	/* Androidの場合のために再初期化する */
	cleanup_conf();

	/* コンフィグを読み込む */
	if (!read_conf())
		return false;

	/* 読み込まれなかった必須コンフィグをチェックする */
	if (!check_conf())
		return false;

	/* コンフィグの値を元に各種設定を初期値にする */
	if (!apply_initial_values())
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
		if (v == NULL || v[0] == '\0') {
			log_empty_conf_string(k);
			return false;
		}

		/* 値を保存する */
		if (!save_value(k, v)) {
			close_rfile(rf);
			return false;
		}
	}
	close_rfile(rf);
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
			return true;

		/* 型ごとに変換する */
		if (rule_tbl[i].type == 'i') {
			*(int *)rule_tbl[i].val = atoi(v);
		} else if (rule_tbl[i].type == 'f') {
			*(float *)rule_tbl[i].val = (float)atof(v);
		} else if (rule_tbl[i].type == 's') {
			/* 文字列の場合は複製する */
			dup = strdup(v);
			if (dup == NULL) {
				log_memory();
				return false;
			}
			*(char **)rule_tbl[i].val = dup;
		} else {
			assert(0);
		}

		rule_tbl[i].loaded = true;

		return true;
	}
	log_unknown_conf(k);
	return false;
}

/* 読み込まれなかった必須コンフィグをチェックする */
static bool check_conf(void)
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
		rule_tbl[i].loaded = false;
		if (rule_tbl[i].type == 's' && rule_tbl[i].val != NULL) {
			free(*(char **)rule_tbl[i].val);
			*(char **)rule_tbl[i].val = NULL;
		}
	}
}

/*
 * コンフィグの値を元に各種設定を初期値にする
 */
bool apply_initial_values(void)
{
	int i;

	/* ロケールのマッピングを行う */
	set_locale_mapping();

	/* グローバルボリュームをセットする */
	set_mixer_global_volume(BGM_STREAM, conf_sound_vol_bgm);
	set_mixer_global_volume(VOICE_STREAM, conf_sound_vol_voice);
	set_mixer_global_volume(SE_STREAM, conf_sound_vol_se);

	/* キャラクタボリュームをセットする */
	for (i = 0; i < CH_VOL_SLOTS; i++)
		set_character_volume(i, conf_sound_vol_character);

	/* フォントファイル名をセットする */
	if (!set_font_file_name(conf_font_file))
		return false;

	return true;
}

/* ロケールを整数に変換する */
void init_locale_code(void)
{
	const char *locale;

	locale = get_system_locale();

	/* ロケール名を整数値に変換する */
	if (strcmp(locale, "en") == 0)
		conf_locale = LOCALE_EN;
	else if (strcmp(locale, "fr") == 0)
		conf_locale = LOCALE_FR;
	else if (strcmp(locale, "de") == 0)
		conf_locale = LOCALE_DE;
	else if (strcmp(locale, "es") == 0)
		conf_locale = LOCALE_ES;
	else if (strcmp(locale, "it") == 0)
		conf_locale = LOCALE_IT;
	else if (strcmp(locale, "el") == 0)
		conf_locale = LOCALE_EL;
	else if (strcmp(locale, "ru") == 0)
		conf_locale = LOCALE_RU;
	else if (strcmp(locale, "zh") == 0)
		conf_locale = LOCALE_ZH;
	else if (strcmp(locale, "tw") == 0)
		conf_locale = LOCALE_TW;
	else if (strcmp(locale, "ja") == 0)
		conf_locale = LOCALE_JA;
	else
		conf_locale = LOCALE_OTHER;
}

/* ロケールのマッピングを行う */
static void set_locale_mapping(void)
{
	const char *locale;
	int i;

	struct {
		const char *locale_name;
		char **config;
	} tbl[] = {
		{"en", &conf_language_en},
		{"fr", &conf_language_fr},
		{"de", &conf_language_de},
		{"es", &conf_language_es},
		{"it", &conf_language_it},
		{"el", &conf_language_el},
		{"ru", &conf_language_ru},
		{"zh", &conf_language_zh},
		{"tw", &conf_language_tw},
		{"ja", &conf_language_ja},
	};

	/* システムのロケールを取得する */
	locale = get_system_locale();
	assert(locale != NULL);

	/* システムのロケールと一致するコンフィグを探す */
	for (i = 0; i < (int)(sizeof(tbl) / sizeof(tbl[0])); i++) {
		if (strcmp(locale, tbl[i].locale_name) == 0) {
			/* ロケールマッピングが指定されていれば設定する */
			if (*tbl[i].config != NULL &&
			    strcmp(*tbl[i].config, "") != 0)
				conf_locale_mapped = *tbl[i].config;
			else
				conf_locale_mapped = "en";
			return;
		}
	}

	/* システムのロケールがコンフィグになく、otherが指定されている場合 */
	if (conf_language_other != NULL &&
	    strcmp(conf_language_other, "") != 0) {
		conf_locale_mapped = conf_language_other;
		return;
	}

	/* otherも指定されていない場合、enにフォールバックする */
	conf_locale_mapped = "en";
}

/*
 * コンフィグを書き換える
 */
bool overwrite_config(const char *key, const char *val)
{
	char *s;
	int i;

	for (i = 0; i < (int)RULE_TBL_SIZE; i++) {
		if (strcmp(rule_tbl[i].key, key) == 0) {
			switch (rule_tbl[i].type) {
			case 'i':
				*(int *)rule_tbl[i].val = atoi(val);
				break;
			case 'f':
				*(float *)rule_tbl[i].val = (float)atof(val);
				break;
			case 's':
				s = strdup(val);
				if (s == NULL) {
					log_memory();
					return false;
				}

				if (rule_tbl[i].val != NULL)
					free(*(char **)rule_tbl[i].val);

				*(char **)rule_tbl[i].val = s;
				break;
			default:
				assert(0);
				break;
			}
			return true;
		}
	}
	return false;
}
