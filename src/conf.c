/* -*- coding: utf-8; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Suika2
 * Copyright (C) 2001-2021, Keiichi Tabata. All rights reserved.
 */

/*
 * [Changes]
 *  - 2016/06/25 作成
 *  - 2017/08/14 スイッチに対応
 *  - 2019/09/17 NEWSに対応
 */

#include "suika.h"
#include <math.h>	/* NAN */

/* False assertion */
#define CONFIG_KEY_NOT_FOUND	(0)
#define INVALID_CONFIG_TYPE	(0)

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

/* テスト用にシステムロケールを上書き (conf.hでexternしていない) */
char *conf_locale_force;

/*
 * ウィンドウの設定
 */
char *conf_window_title;
int conf_window_width;
int conf_window_height;
int conf_window_white;
int conf_window_menubar;
int conf_window_resize;
int conf_window_default_width;
int conf_window_default_height;

/*
 * フォントの設定
 */
int conf_font_select;
char *conf_font_global_file;
char *conf_font_main_file;
char *conf_font_alt1_file;
char *conf_font_alt2_file;
int conf_font_size;
int conf_font_color_r;
int conf_font_color_g;
int conf_font_color_b;
int conf_font_outline_remove;
int conf_font_outline_color_r;
int conf_font_outline_color_g;
int conf_font_outline_color_b;
int conf_font_ruby_size;

/*
 * 名前ボックスの設定
 */
char *conf_namebox_file;
int conf_namebox_font_select;
int conf_namebox_font_size;
int conf_namebox_font_outline;
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
int conf_msgbox_margin_bottom;
int conf_msgbox_margin_line;
int conf_msgbox_margin_char;
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
int conf_msgbox_seen_color;
int conf_msgbox_seen_color_r;
int conf_msgbox_seen_color_g;
int conf_msgbox_seen_color_b;
int conf_msgbox_seen_outline_color_r;
int conf_msgbox_seen_outline_color_g;
int conf_msgbox_seen_outline_color_b;
int conf_msgbox_tategaki;
int conf_msgbox_nowait;

/*
 * クリックアニメーションの設定
 */
int conf_click_x;
int conf_click_y;
int conf_click_move;
char *conf_click_file[16];
float conf_click_interval;
int click_frames;

/*
 * スイッチの設定
 */
char *conf_switch_bg_file[8];
char *conf_switch_fg_file[8];
int conf_switch_font_select;
int conf_switch_font_size;
int conf_switch_font_outline;
int conf_switch_x[8];
int conf_switch_y[8];
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
 * セーブ・ロード画面の設定
 */
int conf_save_data_thumb_width;
int conf_save_data_thumb_height;

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
int conf_sysmenu_custom1_x;
int conf_sysmenu_custom1_y;
int conf_sysmenu_custom1_width;
int conf_sysmenu_custom1_height;
char *conf_sysmenu_custom1_gosub;
int conf_sysmenu_custom2_x;
int conf_sysmenu_custom2_y;
int conf_sysmenu_custom2_width;
int conf_sysmenu_custom2_height;
char *conf_sysmenu_custom2_gosub;
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
char *conf_sysmenu_custom1_se;
char *conf_sysmenu_custom2_se;
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
 * GUIの設定
 */
int conf_gui_ruby;
int conf_gui_save_font_select;
int conf_gui_save_font_size;
int conf_gui_save_font_outline;
int conf_gui_save_font_ruby_size;
int conf_gui_save_tategaki;
int conf_gui_history_font_select;
int conf_gui_history_font_size;
int conf_gui_history_font_outline;
int conf_gui_history_font_ruby_size;
int conf_gui_history_margin_line;
int conf_gui_history_tategaki;
int conf_gui_history_disable_color;
int conf_gui_history_font_color_r;
int conf_gui_history_font_color_g;
int conf_gui_history_font_color_b;
int conf_gui_history_font_outline_color_r;
int conf_gui_history_font_outline_color_g;
int conf_gui_history_font_outline_color_b;
char *conf_gui_history_quote_start;
char *conf_gui_history_quote_end;
int conf_gui_preview_tategaki;

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
 * キャラクタの名前とファイル名のマッピング
 */

int conf_character_focus;
char *conf_character_name[CHARACTER_MAP_COUNT];
char *conf_character_file[CHARACTER_MAP_COUNT];

/*
 * ステージのマージン(キャラクタレイヤの位置補正)
 */

int conf_stage_ch_margin_bottom;
int conf_stage_ch_margin_left;
int conf_stage_ch_margin_right;

/*
 * キラキラエフェクトの設定
 */

int conf_kirakira_on;
float conf_kirakira_frame;
char *conf_kirakira_file[KIRAKIRA_FRAME_COUNT];

/*
 * config.txtには公開されないコンフィグ
 */

/* 最後にセーブ/ロードしたページ */
int conf_gui_save_last_page;


/*
 * アクセシビリティ
 */

/* Text-To-Speechを有効にする */
int conf_tts_enable;

/* Text-To-Speechをユーザが有効にできる */
int conf_tts_user;

/* クリックアニメーションを点滅させない */
int conf_click_disable;

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

/* キャラクタの変更中にメッセージボックスを隠さない */
int conf_msgbox_show_on_ch;

/* 背景の変更中にメッセージボックスを隠さない */
int conf_msgbox_show_on_bg;

/* 選択肢の表示中にメッセージボックスを隠さない */
int conf_msgbox_show_on_choose;

/* ビープの調整 */
float conf_beep_adjustment;

/* セリフをカギカッコで囲う */
int conf_serif_quote;

/* ch,cha,chs,bgの最中に折りたたみメッセージボックスを表示する */
int conf_sysmenu_transition;

/* 上キーでヒストリを表示しない */
int conf_msgbox_history_disable;

/* キャラのセリフの名前だけ色を変更する */
int conf_serif_color_name_only;

/* リリース版であるか */
int conf_release;

/* Web公開時のセーブフォルダ名 */
char *conf_sav_name;

/*
 * 1行のサイズ
 */
#define BUF_SIZE	(1024)

/*
 * コンフィグのテーブル
 */

#define OPTIONAL	true
#define MUST		false
#define SAVE		true
#define NOSAVE		false

static struct rule {
	const char *key;
	char type;
	void *var_ptr;
	bool optional;
	bool save;
} rule_tbl[] = {
	{"language.en", 's', &conf_language_en, OPTIONAL, NOSAVE},
	{"language.fr", 's', &conf_language_fr, OPTIONAL, NOSAVE},
	{"language.de", 's', &conf_language_de, OPTIONAL, NOSAVE},
	{"language.es", 's', &conf_language_es, OPTIONAL, NOSAVE},
	{"language.it", 's', &conf_language_it, OPTIONAL, NOSAVE},
	{"language.el", 's', &conf_language_el, OPTIONAL, NOSAVE},
	{"language.ru", 's', &conf_language_ru, OPTIONAL, NOSAVE},
	{"language.zh", 's', &conf_language_zh, OPTIONAL, NOSAVE},
	{"language.tw", 's', &conf_language_tw, OPTIONAL, NOSAVE},
	{"language.ja", 's', &conf_language_ja, OPTIONAL, NOSAVE},
	{"language.other", 's', &conf_language_other, OPTIONAL, NOSAVE},
	{"locale.force", 's', &conf_locale_force, OPTIONAL, SAVE},
	{"window.title", 's', &conf_window_title, MUST, NOSAVE},
	{"window.width", 'i', &conf_window_width, MUST, NOSAVE},
	{"window.height", 'i', &conf_window_height, MUST, NOSAVE},
	{"window.white", 'i', &conf_window_white, MUST, NOSAVE},
	{"window.menubar", 'i', &conf_window_menubar, OPTIONAL, NOSAVE},
	{"window.resize", 'i', &conf_window_resize, OPTIONAL, NOSAVE},
	{"window.default.width", 'i', &conf_window_default_width, OPTIONAL, NOSAVE},
	{"window.default.height", 'i', &conf_window_default_height, OPTIONAL, NOSAVE},
	{"font.select", 'i', &conf_font_select, OPTIONAL, SAVE},
	{"font.file", 's', &conf_font_global_file, MUST, SAVE},
	{"font.main.file", 's', &conf_font_main_file, OPTIONAL, NOSAVE},
	{"font.alt1.file", 's', &conf_font_alt1_file, OPTIONAL, NOSAVE},
	{"font.alt2.file", 's', &conf_font_alt2_file, OPTIONAL, NOSAVE},
	{"font.size", 'i', &conf_font_size, MUST, SAVE},
	{"font.color.r", 'i', &conf_font_color_r, MUST, SAVE},
	{"font.color.g", 'i', &conf_font_color_g, MUST, SAVE},
	{"font.color.b", 'i', &conf_font_color_b, MUST, SAVE},
	{"font.outline.remove", 'i', &conf_font_outline_remove, OPTIONAL, SAVE},
	{"font.outline.color.r", 'i', &conf_font_outline_color_r, OPTIONAL, SAVE},
	{"font.outline.color.g", 'i', &conf_font_outline_color_g, OPTIONAL, SAVE},
	{"font.outline.color.b", 'i', &conf_font_outline_color_b, OPTIONAL, SAVE},
	{"font.ruby.size", 'i', &conf_font_ruby_size, OPTIONAL, SAVE},
	{"namebox.file", 's', &conf_namebox_file, MUST, SAVE},
	{"namebox.font.select", 'i', &conf_namebox_font_select, OPTIONAL, SAVE},
	{"namebox.font.size", 'i', &conf_namebox_font_size, OPTIONAL, SAVE},
	{"namebox.font.outline", 'i', &conf_namebox_font_outline, OPTIONAL, SAVE},
	{"namebox.x", 'i', &conf_namebox_x, MUST, SAVE},
	{"namebox.y", 'i', &conf_namebox_y, MUST, SAVE},
	{"namebox.margin.top", 'i', &conf_namebox_margin_top, MUST, SAVE},
	{"namebox.centering.no", 'i', &conf_namebox_centering_no, OPTIONAL, SAVE},
	{"namebox.margin.left", 'i', &conf_namebox_margin_left, OPTIONAL, SAVE},
	{"namebox.hidden", 'i', &conf_namebox_hidden, OPTIONAL, SAVE},
	{"msgbox.bg.file", 's', &conf_msgbox_bg_file, MUST, SAVE},
	{"msgbox.fg.file", 's', &conf_msgbox_fg_file, MUST, SAVE},
	{"msgbox.x", 'i', &conf_msgbox_x, MUST, SAVE},
	{"msgbox.y", 'i', &conf_msgbox_y, MUST, SAVE},
	{"msgbox.margin.left", 'i', &conf_msgbox_margin_left, MUST, SAVE},
	{"msgbox.margin.top", 'i', &conf_msgbox_margin_top, MUST, SAVE},
	{"msgbox.margin.right", 'i', &conf_msgbox_margin_right, MUST, SAVE},
	{"msgbox.margin.bottom", 'i', &conf_msgbox_margin_bottom, OPTIONAL, SAVE},
	{"msgbox.margin.line", 'i', &conf_msgbox_margin_line, MUST, SAVE},
	{"msgbox.margin.char", 'i', &conf_msgbox_margin_char, OPTIONAL, SAVE},
	{"msgbox.speed", 'f', &conf_msgbox_speed, MUST, SAVE},
	{"msgbox.btn.qsave.x", 'i', &conf_msgbox_btn_qsave_x, OPTIONAL, SAVE},
	{"msgbox.btn.qsave.y", 'i', &conf_msgbox_btn_qsave_y, OPTIONAL, SAVE},
	{"msgbox.btn.qsave.width", 'i', &conf_msgbox_btn_qsave_width, OPTIONAL, SAVE},
	{"msgbox.btn.qsave.height", 'i', &conf_msgbox_btn_qsave_height, OPTIONAL, SAVE},
	{"msgbox.btn.qload.x", 'i', &conf_msgbox_btn_qload_x, OPTIONAL, SAVE},
	{"msgbox.btn.qload.y", 'i', &conf_msgbox_btn_qload_y, OPTIONAL, SAVE},
	{"msgbox.btn.qload.width", 'i', &conf_msgbox_btn_qload_width, OPTIONAL, SAVE},
	{"msgbox.btn.qload.height", 'i', &conf_msgbox_btn_qload_height, OPTIONAL, SAVE},
	{"msgbox.btn.save.x", 'i', &conf_msgbox_btn_save_x, OPTIONAL, SAVE},
	{"msgbox.btn.save.y", 'i', &conf_msgbox_btn_save_y, OPTIONAL, SAVE},
	{"msgbox.btn.save.width", 'i', &conf_msgbox_btn_save_width, OPTIONAL, SAVE},
	{"msgbox.btn.save.height", 'i', &conf_msgbox_btn_save_height, OPTIONAL, SAVE},
	{"msgbox.btn.load.x", 'i', &conf_msgbox_btn_load_x, OPTIONAL, SAVE},
	{"msgbox.btn.load.y", 'i', &conf_msgbox_btn_load_y, OPTIONAL, SAVE},
	{"msgbox.btn.load.width", 'i', &conf_msgbox_btn_load_width, OPTIONAL, SAVE},
	{"msgbox.btn.load.height", 'i', &conf_msgbox_btn_load_height, OPTIONAL, SAVE},
	{"msgbox.btn.auto.x", 'i', &conf_msgbox_btn_auto_x, OPTIONAL, SAVE},
	{"msgbox.btn.auto.y", 'i', &conf_msgbox_btn_auto_y, OPTIONAL, SAVE},
	{"msgbox.btn.auto.width", 'i', &conf_msgbox_btn_auto_width, OPTIONAL, SAVE},
	{"msgbox.btn.auto.height", 'i', &conf_msgbox_btn_auto_height, OPTIONAL, SAVE},
	{"msgbox.btn.skip.se", 's', &conf_msgbox_btn_skip_se, OPTIONAL, SAVE},
	{"msgbox.btn.skip.x", 'i', &conf_msgbox_btn_skip_x, OPTIONAL, SAVE},
	{"msgbox.btn.skip.y", 'i', &conf_msgbox_btn_skip_y, OPTIONAL, SAVE},
	{"msgbox.btn.skip.width", 'i', &conf_msgbox_btn_skip_width, OPTIONAL, SAVE},
	{"msgbox.btn.skip.height", 'i', &conf_msgbox_btn_skip_height, OPTIONAL, SAVE},
	{"msgbox.btn.history.x", 'i', &conf_msgbox_btn_history_x, OPTIONAL, SAVE},
	{"msgbox.btn.history.y", 'i', &conf_msgbox_btn_history_y, OPTIONAL, SAVE},
	{"msgbox.btn.history.width", 'i', &conf_msgbox_btn_history_width, OPTIONAL, SAVE},
	{"msgbox.btn.history.height", 'i', &conf_msgbox_btn_history_height, OPTIONAL, SAVE},
	{"msgbox.btn.config.x", 'i', &conf_msgbox_btn_config_x, OPTIONAL, SAVE},
	{"msgbox.btn.config.y", 'i', &conf_msgbox_btn_config_y, OPTIONAL, SAVE},
	{"msgbox.btn.config.width", 'i', &conf_msgbox_btn_config_width, OPTIONAL, SAVE},
	{"msgbox.btn.config.height", 'i', &conf_msgbox_btn_config_height, OPTIONAL, SAVE},
	{"msgbox.btn.hide.x", 'i', &conf_msgbox_btn_hide_x, OPTIONAL, SAVE},
	{"msgbox.btn.hide.y", 'i', &conf_msgbox_btn_hide_y, OPTIONAL, SAVE},
	{"msgbox.btn.hide.width", 'i', &conf_msgbox_btn_hide_width, OPTIONAL, SAVE},
	{"msgbox.btn.hide.height", 'i', &conf_msgbox_btn_hide_height, OPTIONAL, SAVE},
	{"msgbox.btn.qsave.se", 's', &conf_msgbox_btn_qsave_se, OPTIONAL, SAVE},
	{"msgbox.btn.qload.se", 's', &conf_msgbox_btn_qload_se, OPTIONAL, SAVE},
	{"msgbox.btn.save.se", 's', &conf_msgbox_btn_save_se, OPTIONAL, SAVE},
	{"msgbox.btn.load.se", 's', &conf_msgbox_btn_load_se, OPTIONAL, SAVE},
	{"msgbox.btn.auto.se", 's', &conf_msgbox_btn_auto_se, OPTIONAL, SAVE},
	{"msgbox.btn.history.se", 's', &conf_msgbox_btn_history_se, OPTIONAL, SAVE},
	{"msgbox.btn.config.se", 's', &conf_msgbox_btn_config_se, OPTIONAL, SAVE},
	{"msgbox.btn.change.se", 's', &conf_msgbox_btn_change_se, OPTIONAL, SAVE},
	{"msgbox.history.se", 's', &conf_msgbox_history_se, OPTIONAL, SAVE},
	{"msgbox.config.se", 's', &conf_msgbox_config_se, OPTIONAL, SAVE},
	{"msgbox.auto.cancel.se", 's', &conf_msgbox_auto_cancel_se, OPTIONAL, SAVE},
	{"msgbox.skip.cancel.se", 's', &conf_msgbox_skip_cancel_se, OPTIONAL, SAVE},
	{"msgbox.hide.se", 's', &conf_msgbox_hide_se, OPTIONAL, SAVE},
	{"msgbox.show.se", 's', &conf_msgbox_show_se, OPTIONAL, SAVE},
	{"msgbox.skip.unseen", 'i', &conf_msgbox_skip_unseen, OPTIONAL, SAVE},
	{"msgbox.dim", 'i', &conf_msgbox_dim, OPTIONAL, SAVE},
	{"msgbox.dim.color.r", 'i', &conf_msgbox_dim_color_r, OPTIONAL, SAVE},
	{"msgbox.dim.color.g", 'i', &conf_msgbox_dim_color_g, OPTIONAL, SAVE},
	{"msgbox.dim.color.b", 'i', &conf_msgbox_dim_color_b, OPTIONAL, SAVE},
	{"msgbox.dim.color.outline.r", 'i', &conf_msgbox_dim_color_outline_r, OPTIONAL, SAVE},
	{"msgbox.dim.color.outline.g", 'i', &conf_msgbox_dim_color_outline_g, OPTIONAL, SAVE},
	{"msgbox.dim.color.outline.b", 'i', &conf_msgbox_dim_color_outline_b, OPTIONAL, SAVE},
	{"msgbox.seen.color", 'i', &conf_msgbox_seen_color, OPTIONAL, SAVE},
	{"msgbox.seen.color.r", 'i', &conf_msgbox_seen_color_r, OPTIONAL, SAVE},
	{"msgbox.seen.color.g", 'i', &conf_msgbox_seen_color_g, OPTIONAL, SAVE},
	{"msgbox.seen.color.b", 'i', &conf_msgbox_seen_color_b, OPTIONAL, SAVE},
	{"msgbox.seen.outline.color.r", 'i', &conf_msgbox_seen_outline_color_r, OPTIONAL, SAVE},
	{"msgbox.seen.outline.color.g", 'i', &conf_msgbox_seen_outline_color_g, OPTIONAL, SAVE},
	{"msgbox.seen.outline.color.b", 'i', &conf_msgbox_seen_outline_color_b, OPTIONAL, SAVE},
	{"msgbox.tategaki", 'i', &conf_msgbox_tategaki, OPTIONAL, SAVE},
	{"msgbox.nowait", 'i', &conf_msgbox_nowait, OPTIONAL, SAVE},
	{"click.x", 'i', &conf_click_x, MUST, SAVE},
	{"click.y", 'i', &conf_click_y, MUST, SAVE},
	{"click.move", 'i', &conf_click_move, OPTIONAL, SAVE},
	{"click.file1", 's', &conf_click_file[0], MUST, SAVE},
	{"click.file2", 's', &conf_click_file[1], OPTIONAL, SAVE},
	{"click.file3", 's', &conf_click_file[2], OPTIONAL, SAVE},
	{"click.file4", 's', &conf_click_file[3], OPTIONAL, SAVE},
	{"click.file5", 's', &conf_click_file[4], OPTIONAL, SAVE},
	{"click.file6", 's', &conf_click_file[5], OPTIONAL, SAVE},
	{"click.file7", 's', &conf_click_file[6], OPTIONAL, SAVE},
	{"click.file8", 's', &conf_click_file[7], OPTIONAL, SAVE},
	{"click.file9", 's', &conf_click_file[8], OPTIONAL, SAVE},
	{"click.file10", 's', &conf_click_file[9], OPTIONAL, SAVE},
	{"click.file11", 's', &conf_click_file[10], OPTIONAL, SAVE},
	{"click.file12", 's', &conf_click_file[11], OPTIONAL, SAVE},
	{"click.file13", 's', &conf_click_file[12], OPTIONAL, SAVE},
	{"click.file14", 's', &conf_click_file[13], OPTIONAL, SAVE},
	{"click.file15", 's', &conf_click_file[14], OPTIONAL, SAVE},
	{"click.file16", 's', &conf_click_file[15], OPTIONAL, SAVE},
	{"click.interval", 'f', &conf_click_interval, MUST, SAVE},
	{"switch.bg.file", 's', &conf_switch_bg_file[0], MUST, SAVE},
	{"switch.fg.file", 's', &conf_switch_fg_file[0], MUST, SAVE},
	{"switch.font.select", 'i', &conf_switch_font_select, OPTIONAL, SAVE},
	{"switch.font.size", 'i', &conf_switch_font_size, OPTIONAL, SAVE},
	{"switch.font.outline", 'i', &conf_switch_font_outline, OPTIONAL, SAVE},
	{"switch.x", 'i', &conf_switch_x[0], MUST, SAVE},
	{"switch.y", 'i', &conf_switch_y[0], MUST, SAVE},
	{"switch.margin.y", 'i', &conf_switch_margin_y, MUST, SAVE},
	{"switch.text.margin.y", 'i', &conf_switch_text_margin_y, MUST, SAVE},
	{"switch.color.active", 'i', &conf_switch_color_active, OPTIONAL, SAVE},
	{"switch.color.active.body.r", 'i', &conf_switch_color_active_body_r, OPTIONAL, SAVE},
	{"switch.color.active.body.g", 'i', &conf_switch_color_active_body_g, OPTIONAL, SAVE},
	{"switch.color.active.body.b", 'i', &conf_switch_color_active_body_b, OPTIONAL, SAVE},
	{"switch.color.active.outline.r", 'i', &conf_switch_color_active_outline_r, OPTIONAL, SAVE},
	{"switch.color.active.outline.g", 'i', &conf_switch_color_active_outline_g, OPTIONAL, SAVE},
	{"switch.color.active.outline.b", 'i', &conf_switch_color_active_outline_b, OPTIONAL, SAVE},
	{"switch.parent.click.se.file", 's', &conf_switch_parent_click_se_file, OPTIONAL, SAVE},
	{"switch.child.click.se.file", 's', &conf_switch_child_click_se_file, OPTIONAL, SAVE},
	{"switch.change.se", 's', &conf_switch_change_se, OPTIONAL, SAVE},
	{"switch.bg.file2", 's', &conf_switch_bg_file[1], OPTIONAL, SAVE},
	{"switch.fg.file2", 's', &conf_switch_fg_file[1], OPTIONAL, SAVE},
	{"switch.x2", 'i', &conf_switch_x[1], OPTIONAL, SAVE},
	{"switch.y2", 'i', &conf_switch_y[1], OPTIONAL, SAVE},
	{"switch.bg.file3", 's', &conf_switch_bg_file[2], OPTIONAL, SAVE},
	{"switch.fg.file3", 's', &conf_switch_fg_file[2], OPTIONAL, SAVE},
	{"switch.x3", 'i', &conf_switch_x[2], OPTIONAL, SAVE},
	{"switch.y3", 'i', &conf_switch_y[2], OPTIONAL, SAVE},
	{"switch.bg.file4", 's', &conf_switch_bg_file[3], OPTIONAL, SAVE},
	{"switch.fg.file4", 's', &conf_switch_fg_file[3], OPTIONAL, SAVE},
	{"switch.x4", 'i', &conf_switch_x[3], OPTIONAL, SAVE},
	{"switch.y4", 'i', &conf_switch_y[3], OPTIONAL, SAVE},
	{"switch.bg.file5", 's', &conf_switch_bg_file[4], OPTIONAL, SAVE},
	{"switch.fg.file5", 's', &conf_switch_fg_file[4], OPTIONAL, SAVE},
	{"switch.x5", 'i', &conf_switch_x[4], OPTIONAL, SAVE},
	{"switch.y5", 'i', &conf_switch_y[4], OPTIONAL, SAVE},
	{"switch.bg.file6", 's', &conf_switch_bg_file[5], OPTIONAL, SAVE},
	{"switch.fg.file6", 's', &conf_switch_fg_file[5], OPTIONAL, SAVE},
	{"switch.x6", 'i', &conf_switch_x[5], OPTIONAL, SAVE},
	{"switch.y6", 'i', &conf_switch_y[5], OPTIONAL, SAVE},
	{"switch.bg.file7", 's', &conf_switch_bg_file[6], OPTIONAL, SAVE},
	{"switch.fg.file7", 's', &conf_switch_fg_file[6], OPTIONAL, SAVE},
	{"switch.x7", 'i', &conf_switch_x[6], OPTIONAL, SAVE},
	{"switch.y7", 'i', &conf_switch_y[6], OPTIONAL, SAVE},
	{"switch.bg.file8", 's', &conf_switch_bg_file[7], OPTIONAL, SAVE},
	{"switch.fg.file8", 's', &conf_switch_fg_file[7], OPTIONAL, SAVE},
	{"switch.x8", 'i', &conf_switch_x[7], OPTIONAL, SAVE},
	{"switch.y8", 'i', &conf_switch_y[7], OPTIONAL, SAVE},
	{"news.bg.file", 's', &conf_news_bg_file, OPTIONAL, SAVE},
	{"news.fg.file", 's', &conf_news_fg_file, OPTIONAL, SAVE},
	{"news.margin", 'i', &conf_news_margin, OPTIONAL, SAVE},
	{"news.text.margin.y", 'i', &conf_news_text_margin_y, OPTIONAL, SAVE},
	{"news.change.se", 's', &conf_news_change_se, OPTIONAL, SAVE},
	{"save.data.thumb.width", 'i', &conf_save_data_thumb_width, MUST, NOSAVE},
	{"save.data.thumb.height", 'i', &conf_save_data_thumb_height, MUST, NOSAVE},
	{"sysmenu.x", 'i', &conf_sysmenu_x, MUST, SAVE},
	{"sysmenu.y", 'i', &conf_sysmenu_y, MUST, SAVE},
	{"sysmenu.idle.file", 's', &conf_sysmenu_idle_file, MUST, SAVE},
	{"sysmenu.hover.file", 's', &conf_sysmenu_hover_file, MUST, SAVE},
	{"sysmenu.disable.file", 's', &conf_sysmenu_disable_file, MUST, SAVE},
	{"sysmenu.qsave.x", 'i', &conf_sysmenu_qsave_x, MUST, SAVE},
	{"sysmenu.qsave.y", 'i', &conf_sysmenu_qsave_y, MUST, SAVE},
	{"sysmenu.qsave.width", 'i', &conf_sysmenu_qsave_width, MUST, SAVE},
	{"sysmenu.qsave.height", 'i', &conf_sysmenu_qsave_height, MUST, SAVE},
	{"sysmenu.qload.x", 'i', &conf_sysmenu_qload_x, MUST, SAVE},
	{"sysmenu.qload.y", 'i', &conf_sysmenu_qload_y, MUST, SAVE},
	{"sysmenu.qload.width", 'i', &conf_sysmenu_qload_width, MUST, SAVE},
	{"sysmenu.qload.height", 'i', &conf_sysmenu_qload_height, MUST, SAVE},
	{"sysmenu.save.x", 'i', &conf_sysmenu_save_x, MUST, SAVE},
	{"sysmenu.save.y", 'i', &conf_sysmenu_save_y, MUST, SAVE},
	{"sysmenu.save.width", 'i', &conf_sysmenu_save_width, MUST, SAVE},
	{"sysmenu.save.height", 'i', &conf_sysmenu_save_height, MUST, SAVE},
	{"sysmenu.load.x", 'i', &conf_sysmenu_load_x, MUST, SAVE},
	{"sysmenu.load.y", 'i', &conf_sysmenu_load_y, MUST, SAVE},
	{"sysmenu.load.width", 'i', &conf_sysmenu_load_width, MUST, SAVE},
	{"sysmenu.load.height", 'i', &conf_sysmenu_load_height, MUST, SAVE},
	{"sysmenu.auto.x", 'i', &conf_sysmenu_auto_x, MUST, SAVE},
	{"sysmenu.auto.y", 'i', &conf_sysmenu_auto_y, MUST, SAVE},
	{"sysmenu.auto.width", 'i', &conf_sysmenu_auto_width, MUST, SAVE},
	{"sysmenu.auto.height", 'i', &conf_sysmenu_auto_height, MUST, SAVE},
	{"sysmenu.skip.x", 'i', &conf_sysmenu_skip_x, MUST, SAVE},
	{"sysmenu.skip.y", 'i', &conf_sysmenu_skip_y, MUST, SAVE},
	{"sysmenu.skip.width", 'i', &conf_sysmenu_skip_width, MUST, SAVE},
	{"sysmenu.skip.height", 'i', &conf_sysmenu_skip_height, MUST, SAVE},
	{"sysmenu.history.x", 'i', &conf_sysmenu_history_x, MUST, SAVE},
	{"sysmenu.history.y", 'i', &conf_sysmenu_history_y, MUST, SAVE},
	{"sysmenu.history.width", 'i', &conf_sysmenu_history_width, MUST, SAVE},
	{"sysmenu.history.height", 'i', &conf_sysmenu_history_height, MUST, SAVE},
	{"sysmenu.config.x", 'i', &conf_sysmenu_config_x, MUST, SAVE},
	{"sysmenu.config.y", 'i', &conf_sysmenu_config_y, MUST, SAVE},
	{"sysmenu.config.width", 'i', &conf_sysmenu_config_width, MUST, SAVE},
	{"sysmenu.config.height", 'i', &conf_sysmenu_config_height, MUST, SAVE},
	{"sysmenu.custom1.x", 'i', &conf_sysmenu_custom1_x, OPTIONAL, SAVE},
	{"sysmenu.custom1.y", 'i', &conf_sysmenu_custom1_y, OPTIONAL, SAVE},
	{"sysmenu.custom1.width", 'i', &conf_sysmenu_custom1_width, OPTIONAL, SAVE},
	{"sysmenu.custom1.height", 'i', &conf_sysmenu_custom1_height, OPTIONAL, SAVE},
	{"sysmenu.custom1.gosub", 's', &conf_sysmenu_custom1_gosub, OPTIONAL, SAVE},
	{"sysmenu.custom2.x", 'i', &conf_sysmenu_custom2_x, OPTIONAL, SAVE},
	{"sysmenu.custom2.y", 'i', &conf_sysmenu_custom2_y, OPTIONAL, SAVE},
	{"sysmenu.custom2.width", 'i', &conf_sysmenu_custom2_width, OPTIONAL, SAVE},
	{"sysmenu.custom2.height", 'i', &conf_sysmenu_custom2_height, OPTIONAL, SAVE},
	{"sysmenu.custom2.gosub", 's', &conf_sysmenu_custom2_gosub, OPTIONAL, SAVE},
	{"sysmenu.enter.se", 's', &conf_sysmenu_enter_se, OPTIONAL, SAVE},
	{"sysmenu.leave.se", 's', &conf_sysmenu_leave_se, OPTIONAL, SAVE},
	{"sysmenu.change.se", 's', &conf_sysmenu_change_se, OPTIONAL, SAVE},
	{"sysmenu.qsave.se", 's', &conf_sysmenu_qsave_se, OPTIONAL, SAVE},
	{"sysmenu.qload.se", 's', &conf_sysmenu_qload_se, OPTIONAL, SAVE},
	{"sysmenu.save.se", 's', &conf_sysmenu_save_se, OPTIONAL, SAVE},
	{"sysmenu.load.se", 's', &conf_sysmenu_load_se, OPTIONAL, SAVE},
	{"sysmenu.auto.se", 's', &conf_sysmenu_auto_se, OPTIONAL, SAVE},
	{"sysmenu.skip.se", 's', &conf_sysmenu_skip_se, OPTIONAL, SAVE},
	{"sysmenu.history.se", 's', &conf_sysmenu_history_se, OPTIONAL, SAVE},
	{"sysmenu.config.se", 's', &conf_sysmenu_config_se, OPTIONAL, SAVE},
	{"sysmenu.custom1.se", 's', &conf_sysmenu_custom1_se, OPTIONAL, SAVE},
	{"sysmenu.custom2.se", 's', &conf_sysmenu_custom2_se, OPTIONAL, SAVE},
	{"sysmenu.hidden", 'i', &conf_sysmenu_hidden, OPTIONAL, SAVE},
	{"automode.banner.file", 's', &conf_automode_banner_file, MUST, SAVE},
	{"automode.banner.x", 'i', &conf_automode_banner_x, MUST, SAVE},
	{"automode.banner.y", 'i', &conf_automode_banner_y, MUST, SAVE},
	{"automode.speed", 'f', &conf_automode_speed, MUST, SAVE},
	{"skipmode.banner.file", 's', &conf_skipmode_banner_file, MUST, SAVE},
	{"skipmode.banner.x", 'i', &conf_skipmode_banner_x, MUST, SAVE},
	{"skipmode.banner.y", 'i', &conf_skipmode_banner_y, MUST, SAVE},
	{"sysmenu.collapsed.x", 'i', &conf_sysmenu_collapsed_x, MUST, SAVE},
	{"sysmenu.collapsed.y", 'i', &conf_sysmenu_collapsed_y, MUST, SAVE},
	{"sysmenu.collapsed.idle.file", 's', &conf_sysmenu_collapsed_idle_file, MUST, SAVE},
	{"sysmenu.collapsed.hover.file", 's', &conf_sysmenu_collapsed_hover_file, MUST, SAVE},
	{"sysmenu.collapsed.se", 's', &conf_sysmenu_collapsed_se, OPTIONAL, SAVE},
	{"gui.ruby", 'i', &conf_gui_ruby, OPTIONAL, SAVE},
	{"gui.save.font.select", 'i', &conf_gui_save_font_select, OPTIONAL, SAVE},
	{"gui.save.font.size", 'i', &conf_gui_save_font_size, OPTIONAL, SAVE},
	{"gui.save.font.outline", 'i', &conf_gui_save_font_outline, OPTIONAL, SAVE},
	{"gui.save.font.ruby.size", 'i', &conf_gui_save_font_ruby_size, OPTIONAL, SAVE},
	{"gui.save.tategaki", 'i', &conf_gui_save_tategaki, OPTIONAL, SAVE},
	{"gui.history.font.select", 'i', &conf_gui_history_font_select, OPTIONAL, SAVE},
	{"gui.history.font.size", 'i', &conf_gui_history_font_size, OPTIONAL, SAVE},
	{"gui.history.font.outline", 'i', &conf_gui_history_font_outline, OPTIONAL, SAVE},
	{"gui.history.font.ruby.size", 'i', &conf_gui_history_font_ruby_size, OPTIONAL, SAVE},
	{"gui.history.margin.line", 'i', &conf_gui_history_margin_line, OPTIONAL, SAVE},
	{"gui.history.disable.color", 'i', &conf_gui_history_disable_color, OPTIONAL, SAVE},
	{"gui.history.tategaki", 'i', &conf_gui_history_tategaki, OPTIONAL, SAVE},
	{"gui.history.font.color.r", 'i', &conf_gui_history_font_color_r, OPTIONAL, SAVE},
	{"gui.history.font.color.g", 'i', &conf_gui_history_font_color_g, OPTIONAL, SAVE},
	{"gui.history.font.color.b", 'i', &conf_gui_history_font_color_b, OPTIONAL, SAVE},
	{"gui.history.font.outline.color.r", 'i', &conf_gui_history_font_outline_color_r, OPTIONAL, SAVE},
	{"gui.history.font.outline.color.g", 'i', &conf_gui_history_font_outline_color_g, OPTIONAL, SAVE},
	{"gui.history.font.outline.color.b", 'i', &conf_gui_history_font_outline_color_b, OPTIONAL, SAVE},
	{"gui.history.quote.start", 's', &conf_gui_history_quote_start, OPTIONAL, SAVE},
	{"gui.history.quote.end", 's', &conf_gui_history_quote_end, OPTIONAL, SAVE},
	{"gui.preview.tategaki", 'i', &conf_gui_preview_tategaki, OPTIONAL, SAVE},
	/* 下記は初期音量なのでセーブしない */
	{"sound.vol.bgm", 'f', &conf_sound_vol_bgm, MUST, NOSAVE},
	{"sound.vol.voice", 'f', &conf_sound_vol_voice, MUST, NOSAVE},
	{"sound.vol.se", 'f', &conf_sound_vol_se, MUST, NOSAVE},
	{"sound.vol.character", 'f', &conf_sound_vol_character, MUST, NOSAVE},
	{"sound.character.name1", 's', &conf_sound_character_name[1], OPTIONAL, NOSAVE},
	{"sound.character.name2", 's', &conf_sound_character_name[2], OPTIONAL, NOSAVE},
	{"sound.character.name3", 's', &conf_sound_character_name[3], OPTIONAL, NOSAVE},
	{"sound.character.name4", 's', &conf_sound_character_name[4], OPTIONAL, NOSAVE},
	{"sound.character.name5", 's', &conf_sound_character_name[5], OPTIONAL, NOSAVE},
	{"sound.character.name6", 's', &conf_sound_character_name[6], OPTIONAL, NOSAVE},
	{"sound.character.name7", 's', &conf_sound_character_name[7], OPTIONAL, NOSAVE},
	{"sound.character.name8", 's', &conf_sound_character_name[8], OPTIONAL, NOSAVE},
	{"sound.character.name9", 's', &conf_sound_character_name[9], OPTIONAL, NOSAVE},
	{"sound.character.name10", 's', &conf_sound_character_name[10], OPTIONAL, NOSAVE},
	{"sound.character.name11", 's', &conf_sound_character_name[11], OPTIONAL, NOSAVE},
	{"sound.character.name12", 's', &conf_sound_character_name[12], OPTIONAL, NOSAVE},
	{"sound.character.name13", 's', &conf_sound_character_name[13], OPTIONAL, NOSAVE},
	{"sound.character.name14", 's', &conf_sound_character_name[14], OPTIONAL, NOSAVE},
	{"sound.character.name15", 's', &conf_sound_character_name[15], OPTIONAL, NOSAVE},
	/* 初期音量はここまで */
	/* <!-- 下記はジェネレータで出力したもの */
	{"serif.color1.name", 's', &conf_serif_color_name[0], OPTIONAL, NOSAVE},
	{"serif.color1.r", 'i', &conf_serif_color_r[0], OPTIONAL, NOSAVE},
	{"serif.color1.g", 'i', &conf_serif_color_g[0], OPTIONAL, NOSAVE},
	{"serif.color1.b", 'i', &conf_serif_color_b[0], OPTIONAL, NOSAVE},
	{"serif.color1.outline.r", 'i', &conf_serif_outline_color_r[0], OPTIONAL, NOSAVE},
	{"serif.color1.outline.g", 'i', &conf_serif_outline_color_g[0], OPTIONAL, NOSAVE},
	{"serif.color1.outline.b", 'i', &conf_serif_outline_color_b[0], OPTIONAL, NOSAVE},
	{"serif.color2.name", 's', &conf_serif_color_name[1], OPTIONAL, NOSAVE},
	{"serif.color2.r", 'i', &conf_serif_color_r[1], OPTIONAL, NOSAVE},
	{"serif.color2.g", 'i', &conf_serif_color_g[1], OPTIONAL, NOSAVE},
	{"serif.color2.b", 'i', &conf_serif_color_b[1], OPTIONAL, NOSAVE},
	{"serif.color2.outline.r", 'i', &conf_serif_outline_color_r[1], OPTIONAL, NOSAVE},
	{"serif.color2.outline.g", 'i', &conf_serif_outline_color_g[1], OPTIONAL, NOSAVE},
	{"serif.color2.outline.b", 'i', &conf_serif_outline_color_b[1], OPTIONAL, NOSAVE},
	{"serif.color3.name", 's', &conf_serif_color_name[2], OPTIONAL, NOSAVE},
	{"serif.color3.r", 'i', &conf_serif_color_r[2], OPTIONAL, NOSAVE},
	{"serif.color3.g", 'i', &conf_serif_color_g[2], OPTIONAL, NOSAVE},
	{"serif.color3.b", 'i', &conf_serif_color_b[2], OPTIONAL, NOSAVE},
	{"serif.color3.outline.r", 'i', &conf_serif_outline_color_r[2], OPTIONAL, NOSAVE},
	{"serif.color3.outline.g", 'i', &conf_serif_outline_color_g[2], OPTIONAL, NOSAVE},
	{"serif.color3.outline.b", 'i', &conf_serif_outline_color_b[2], OPTIONAL, NOSAVE},
	{"serif.color4.name", 's', &conf_serif_color_name[3], OPTIONAL, NOSAVE},
	{"serif.color4.r", 'i', &conf_serif_color_r[3], OPTIONAL, NOSAVE},
	{"serif.color4.g", 'i', &conf_serif_color_g[3], OPTIONAL, NOSAVE},
	{"serif.color4.b", 'i', &conf_serif_color_b[3], OPTIONAL, NOSAVE},
	{"serif.color4.outline.r", 'i', &conf_serif_outline_color_r[3], OPTIONAL, NOSAVE},
	{"serif.color4.outline.g", 'i', &conf_serif_outline_color_g[3], OPTIONAL, NOSAVE},
	{"serif.color4.outline.b", 'i', &conf_serif_outline_color_b[3], OPTIONAL, NOSAVE},
	{"serif.color5.name", 's', &conf_serif_color_name[4], OPTIONAL, NOSAVE},
	{"serif.color5.r", 'i', &conf_serif_color_r[4], OPTIONAL, NOSAVE},
	{"serif.color5.g", 'i', &conf_serif_color_g[4], OPTIONAL, NOSAVE},
	{"serif.color5.b", 'i', &conf_serif_color_b[4], OPTIONAL, NOSAVE},
	{"serif.color5.outline.r", 'i', &conf_serif_outline_color_r[4], OPTIONAL, NOSAVE},
	{"serif.color5.outline.g", 'i', &conf_serif_outline_color_g[4], OPTIONAL, NOSAVE},
	{"serif.color5.outline.b", 'i', &conf_serif_outline_color_b[4], OPTIONAL, NOSAVE},
	{"serif.color6.name", 's', &conf_serif_color_name[5], OPTIONAL, NOSAVE},
	{"serif.color6.r", 'i', &conf_serif_color_r[5], OPTIONAL, NOSAVE},
	{"serif.color6.g", 'i', &conf_serif_color_g[5], OPTIONAL, NOSAVE},
	{"serif.color6.b", 'i', &conf_serif_color_b[5], OPTIONAL, NOSAVE},
	{"serif.color6.outline.r", 'i', &conf_serif_outline_color_r[5], OPTIONAL, NOSAVE},
	{"serif.color6.outline.g", 'i', &conf_serif_outline_color_g[5], OPTIONAL, NOSAVE},
	{"serif.color6.outline.b", 'i', &conf_serif_outline_color_b[5], OPTIONAL, NOSAVE},
	{"serif.color7.name", 's', &conf_serif_color_name[6], OPTIONAL, NOSAVE},
	{"serif.color7.r", 'i', &conf_serif_color_r[6], OPTIONAL, NOSAVE},
	{"serif.color7.g", 'i', &conf_serif_color_g[6], OPTIONAL, NOSAVE},
	{"serif.color7.b", 'i', &conf_serif_color_b[6], OPTIONAL, NOSAVE},
	{"serif.color7.outline.r", 'i', &conf_serif_outline_color_r[6], OPTIONAL, NOSAVE},
	{"serif.color7.outline.g", 'i', &conf_serif_outline_color_g[6], OPTIONAL, NOSAVE},
	{"serif.color7.outline.b", 'i', &conf_serif_outline_color_b[6], OPTIONAL, NOSAVE},
	{"serif.color8.name", 's', &conf_serif_color_name[7], OPTIONAL, NOSAVE},
	{"serif.color8.r", 'i', &conf_serif_color_r[7], OPTIONAL, NOSAVE},
	{"serif.color8.g", 'i', &conf_serif_color_g[7], OPTIONAL, NOSAVE},
	{"serif.color8.b", 'i', &conf_serif_color_b[7], OPTIONAL, NOSAVE},
	{"serif.color8.outline.r", 'i', &conf_serif_outline_color_r[7], OPTIONAL, NOSAVE},
	{"serif.color8.outline.g", 'i', &conf_serif_outline_color_g[7], OPTIONAL, NOSAVE},
	{"serif.color8.outline.b", 'i', &conf_serif_outline_color_b[7], OPTIONAL, NOSAVE},
	{"serif.color9.name", 's', &conf_serif_color_name[8], OPTIONAL, NOSAVE},
	{"serif.color9.r", 'i', &conf_serif_color_r[8], OPTIONAL, NOSAVE},
	{"serif.color9.g", 'i', &conf_serif_color_g[8], OPTIONAL, NOSAVE},
	{"serif.color9.b", 'i', &conf_serif_color_b[8], OPTIONAL, NOSAVE},
	{"serif.color9.outline.r", 'i', &conf_serif_outline_color_r[8], OPTIONAL, NOSAVE},
	{"serif.color9.outline.g", 'i', &conf_serif_outline_color_g[8], OPTIONAL, NOSAVE},
	{"serif.color9.outline.b", 'i', &conf_serif_outline_color_b[8], OPTIONAL, NOSAVE},
	{"serif.color10.name", 's', &conf_serif_color_name[9], OPTIONAL, NOSAVE},
	{"serif.color10.r", 'i', &conf_serif_color_r[9], OPTIONAL, NOSAVE},
	{"serif.color10.g", 'i', &conf_serif_color_g[9], OPTIONAL, NOSAVE},
	{"serif.color10.b", 'i', &conf_serif_color_b[9], OPTIONAL, NOSAVE},
	{"serif.color10.outline.r", 'i', &conf_serif_outline_color_r[9], OPTIONAL, NOSAVE},
	{"serif.color10.outline.g", 'i', &conf_serif_outline_color_g[9], OPTIONAL, NOSAVE},
	{"serif.color10.outline.b", 'i', &conf_serif_outline_color_b[9], OPTIONAL, NOSAVE},
	{"serif.color11.name", 's', &conf_serif_color_name[10], OPTIONAL, NOSAVE},
	{"serif.color11.r", 'i', &conf_serif_color_r[10], OPTIONAL, NOSAVE},
	{"serif.color11.g", 'i', &conf_serif_color_g[10], OPTIONAL, NOSAVE},
	{"serif.color11.b", 'i', &conf_serif_color_b[10], OPTIONAL, NOSAVE},
	{"serif.color11.outline.r", 'i', &conf_serif_outline_color_r[10], OPTIONAL, NOSAVE},
	{"serif.color11.outline.g", 'i', &conf_serif_outline_color_g[10], OPTIONAL, NOSAVE},
	{"serif.color11.outline.b", 'i', &conf_serif_outline_color_b[10], OPTIONAL, NOSAVE},
	{"serif.color12.name", 's', &conf_serif_color_name[11], OPTIONAL, NOSAVE},
	{"serif.color12.r", 'i', &conf_serif_color_r[11], OPTIONAL, NOSAVE},
	{"serif.color12.g", 'i', &conf_serif_color_g[11], OPTIONAL, NOSAVE},
	{"serif.color12.b", 'i', &conf_serif_color_b[11], OPTIONAL, NOSAVE},
	{"serif.color12.outline.r", 'i', &conf_serif_outline_color_r[11], OPTIONAL, NOSAVE},
	{"serif.color12.outline.g", 'i', &conf_serif_outline_color_g[11], OPTIONAL, NOSAVE},
	{"serif.color12.outline.b", 'i', &conf_serif_outline_color_b[11], OPTIONAL, NOSAVE},
	{"serif.color13.name", 's', &conf_serif_color_name[12], OPTIONAL, NOSAVE},
	{"serif.color13.r", 'i', &conf_serif_color_r[12], OPTIONAL, NOSAVE},
	{"serif.color13.g", 'i', &conf_serif_color_g[12], OPTIONAL, NOSAVE},
	{"serif.color13.b", 'i', &conf_serif_color_b[12], OPTIONAL, NOSAVE},
	{"serif.color13.outline.r", 'i', &conf_serif_outline_color_r[12], OPTIONAL, NOSAVE},
	{"serif.color13.outline.g", 'i', &conf_serif_outline_color_g[12], OPTIONAL, NOSAVE},
	{"serif.color13.outline.b", 'i', &conf_serif_outline_color_b[12], OPTIONAL, NOSAVE},
	{"serif.color14.name", 's', &conf_serif_color_name[13], OPTIONAL, NOSAVE},
	{"serif.color14.r", 'i', &conf_serif_color_r[13], OPTIONAL, NOSAVE},
	{"serif.color14.g", 'i', &conf_serif_color_g[13], OPTIONAL, NOSAVE},
	{"serif.color14.b", 'i', &conf_serif_color_b[13], OPTIONAL, NOSAVE},
	{"serif.color14.outline.r", 'i', &conf_serif_outline_color_r[13], OPTIONAL, NOSAVE},
	{"serif.color14.outline.g", 'i', &conf_serif_outline_color_g[13], OPTIONAL, NOSAVE},
	{"serif.color14.outline.b", 'i', &conf_serif_outline_color_b[13], OPTIONAL, NOSAVE},
	{"serif.color15.name", 's', &conf_serif_color_name[14], OPTIONAL, NOSAVE},
	{"serif.color15.r", 'i', &conf_serif_color_r[14], OPTIONAL, NOSAVE},
	{"serif.color15.g", 'i', &conf_serif_color_g[14], OPTIONAL, NOSAVE},
	{"serif.color15.b", 'i', &conf_serif_color_b[14], OPTIONAL, NOSAVE},
	{"serif.color15.outline.r", 'i', &conf_serif_outline_color_r[14], OPTIONAL, NOSAVE},
	{"serif.color15.outline.g", 'i', &conf_serif_outline_color_g[14], OPTIONAL, NOSAVE},
	{"serif.color15.outline.b", 'i', &conf_serif_outline_color_b[14], OPTIONAL, NOSAVE},
	{"serif.color16.name", 's', &conf_serif_color_name[15], OPTIONAL, NOSAVE},
	{"serif.color16.r", 'i', &conf_serif_color_r[15], OPTIONAL, NOSAVE},
	{"serif.color16.g", 'i', &conf_serif_color_g[15], OPTIONAL, NOSAVE},
	{"serif.color16.b", 'i', &conf_serif_color_b[15], OPTIONAL, NOSAVE},
	{"serif.color16.outline.r", 'i', &conf_serif_outline_color_r[15], OPTIONAL, NOSAVE},
	{"serif.color16.outline.g", 'i', &conf_serif_outline_color_g[15], OPTIONAL, NOSAVE},
	{"serif.color16.outline.b", 'i', &conf_serif_outline_color_b[15], OPTIONAL, NOSAVE},
	{"serif.color17.name", 's', &conf_serif_color_name[16], OPTIONAL, NOSAVE},
	{"serif.color17.r", 'i', &conf_serif_color_r[16], OPTIONAL, NOSAVE},
	{"serif.color17.g", 'i', &conf_serif_color_g[16], OPTIONAL, NOSAVE},
	{"serif.color17.b", 'i', &conf_serif_color_b[16], OPTIONAL, NOSAVE},
	{"serif.color17.outline.r", 'i', &conf_serif_outline_color_r[16], OPTIONAL, NOSAVE},
	{"serif.color17.outline.g", 'i', &conf_serif_outline_color_g[16], OPTIONAL, NOSAVE},
	{"serif.color17.outline.b", 'i', &conf_serif_outline_color_b[16], OPTIONAL, NOSAVE},
	{"serif.color18.name", 's', &conf_serif_color_name[17], OPTIONAL, NOSAVE},
	{"serif.color18.r", 'i', &conf_serif_color_r[17], OPTIONAL, NOSAVE},
	{"serif.color18.g", 'i', &conf_serif_color_g[17], OPTIONAL, NOSAVE},
	{"serif.color18.b", 'i', &conf_serif_color_b[17], OPTIONAL, NOSAVE},
	{"serif.color18.outline.r", 'i', &conf_serif_outline_color_r[17], OPTIONAL, NOSAVE},
	{"serif.color18.outline.g", 'i', &conf_serif_outline_color_g[17], OPTIONAL, NOSAVE},
	{"serif.color18.outline.b", 'i', &conf_serif_outline_color_b[17], OPTIONAL, NOSAVE},
	{"serif.color19.name", 's', &conf_serif_color_name[18], OPTIONAL, NOSAVE},
	{"serif.color19.r", 'i', &conf_serif_color_r[18], OPTIONAL, NOSAVE},
	{"serif.color19.g", 'i', &conf_serif_color_g[18], OPTIONAL, NOSAVE},
	{"serif.color19.b", 'i', &conf_serif_color_b[18], OPTIONAL, NOSAVE},
	{"serif.color19.outline.r", 'i', &conf_serif_outline_color_r[18], OPTIONAL, NOSAVE},
	{"serif.color19.outline.g", 'i', &conf_serif_outline_color_g[18], OPTIONAL, NOSAVE},
	{"serif.color19.outline.b", 'i', &conf_serif_outline_color_b[18], OPTIONAL, NOSAVE},
	{"serif.color20.name", 's', &conf_serif_color_name[19], OPTIONAL, NOSAVE},
	{"serif.color20.r", 'i', &conf_serif_color_r[19], OPTIONAL, NOSAVE},
	{"serif.color20.g", 'i', &conf_serif_color_g[19], OPTIONAL, NOSAVE},
	{"serif.color20.b", 'i', &conf_serif_color_b[19], OPTIONAL, NOSAVE},
	{"serif.color20.outline.r", 'i', &conf_serif_outline_color_r[19], OPTIONAL, NOSAVE},
	{"serif.color20.outline.g", 'i', &conf_serif_outline_color_g[19], OPTIONAL, NOSAVE},
	{"serif.color20.outline.b", 'i', &conf_serif_outline_color_b[19], OPTIONAL, NOSAVE},
	{"serif.color21.name", 's', &conf_serif_color_name[20], OPTIONAL, NOSAVE},
	{"serif.color21.r", 'i', &conf_serif_color_r[20], OPTIONAL, NOSAVE},
	{"serif.color21.g", 'i', &conf_serif_color_g[20], OPTIONAL, NOSAVE},
	{"serif.color21.b", 'i', &conf_serif_color_b[20], OPTIONAL, NOSAVE},
	{"serif.color21.outline.r", 'i', &conf_serif_outline_color_r[20], OPTIONAL, NOSAVE},
	{"serif.color21.outline.g", 'i', &conf_serif_outline_color_g[20], OPTIONAL, NOSAVE},
	{"serif.color21.outline.b", 'i', &conf_serif_outline_color_b[20], OPTIONAL, NOSAVE},
	{"serif.color22.name", 's', &conf_serif_color_name[21], OPTIONAL, NOSAVE},
	{"serif.color22.r", 'i', &conf_serif_color_r[21], OPTIONAL, NOSAVE},
	{"serif.color22.g", 'i', &conf_serif_color_g[21], OPTIONAL, NOSAVE},
	{"serif.color22.b", 'i', &conf_serif_color_b[21], OPTIONAL, NOSAVE},
	{"serif.color22.outline.r", 'i', &conf_serif_outline_color_r[21], OPTIONAL, NOSAVE},
	{"serif.color22.outline.g", 'i', &conf_serif_outline_color_g[21], OPTIONAL, NOSAVE},
	{"serif.color22.outline.b", 'i', &conf_serif_outline_color_b[21], OPTIONAL, NOSAVE},
	{"serif.color23.name", 's', &conf_serif_color_name[22], OPTIONAL, NOSAVE},
	{"serif.color23.r", 'i', &conf_serif_color_r[22], OPTIONAL, NOSAVE},
	{"serif.color23.g", 'i', &conf_serif_color_g[22], OPTIONAL, NOSAVE},
	{"serif.color23.b", 'i', &conf_serif_color_b[22], OPTIONAL, NOSAVE},
	{"serif.color23.outline.r", 'i', &conf_serif_outline_color_r[22], OPTIONAL, NOSAVE},
	{"serif.color23.outline.g", 'i', &conf_serif_outline_color_g[22], OPTIONAL, NOSAVE},
	{"serif.color23.outline.b", 'i', &conf_serif_outline_color_b[22], OPTIONAL, NOSAVE},
	{"serif.color24.name", 's', &conf_serif_color_name[23], OPTIONAL, NOSAVE},
	{"serif.color24.r", 'i', &conf_serif_color_r[23], OPTIONAL, NOSAVE},
	{"serif.color24.g", 'i', &conf_serif_color_g[23], OPTIONAL, NOSAVE},
	{"serif.color24.b", 'i', &conf_serif_color_b[23], OPTIONAL, NOSAVE},
	{"serif.color24.outline.r", 'i', &conf_serif_outline_color_r[23], OPTIONAL, NOSAVE},
	{"serif.color24.outline.g", 'i', &conf_serif_outline_color_g[23], OPTIONAL, NOSAVE},
	{"serif.color24.outline.b", 'i', &conf_serif_outline_color_b[23], OPTIONAL, NOSAVE},
	{"serif.color25.name", 's', &conf_serif_color_name[24], OPTIONAL, NOSAVE},
	{"serif.color25.r", 'i', &conf_serif_color_r[24], OPTIONAL, NOSAVE},
	{"serif.color25.g", 'i', &conf_serif_color_g[24], OPTIONAL, NOSAVE},
	{"serif.color25.b", 'i', &conf_serif_color_b[24], OPTIONAL, NOSAVE},
	{"serif.color25.outline.r", 'i', &conf_serif_outline_color_r[24], OPTIONAL, NOSAVE},
	{"serif.color25.outline.g", 'i', &conf_serif_outline_color_g[24], OPTIONAL, NOSAVE},
	{"serif.color25.outline.b", 'i', &conf_serif_outline_color_b[24], OPTIONAL, NOSAVE},
	{"serif.color26.name", 's', &conf_serif_color_name[25], OPTIONAL, NOSAVE},
	{"serif.color26.r", 'i', &conf_serif_color_r[25], OPTIONAL, NOSAVE},
	{"serif.color26.g", 'i', &conf_serif_color_g[25], OPTIONAL, NOSAVE},
	{"serif.color26.b", 'i', &conf_serif_color_b[25], OPTIONAL, NOSAVE},
	{"serif.color26.outline.r", 'i', &conf_serif_outline_color_r[25], OPTIONAL, NOSAVE},
	{"serif.color26.outline.g", 'i', &conf_serif_outline_color_g[25], OPTIONAL, NOSAVE},
	{"serif.color26.outline.b", 'i', &conf_serif_outline_color_b[25], OPTIONAL, NOSAVE},
	{"serif.color27.name", 's', &conf_serif_color_name[26], OPTIONAL, NOSAVE},
	{"serif.color27.r", 'i', &conf_serif_color_r[26], OPTIONAL, NOSAVE},
	{"serif.color27.g", 'i', &conf_serif_color_g[26], OPTIONAL, NOSAVE},
	{"serif.color27.b", 'i', &conf_serif_color_b[26], OPTIONAL, NOSAVE},
	{"serif.color27.outline.r", 'i', &conf_serif_outline_color_r[26], OPTIONAL, NOSAVE},
	{"serif.color27.outline.g", 'i', &conf_serif_outline_color_g[26], OPTIONAL, NOSAVE},
	{"serif.color27.outline.b", 'i', &conf_serif_outline_color_b[26], OPTIONAL, NOSAVE},
	{"serif.color28.name", 's', &conf_serif_color_name[27], OPTIONAL, NOSAVE},
	{"serif.color28.r", 'i', &conf_serif_color_r[27], OPTIONAL, NOSAVE},
	{"serif.color28.g", 'i', &conf_serif_color_g[27], OPTIONAL, NOSAVE},
	{"serif.color28.b", 'i', &conf_serif_color_b[27], OPTIONAL, NOSAVE},
	{"serif.color28.outline.r", 'i', &conf_serif_outline_color_r[27], OPTIONAL, NOSAVE},
	{"serif.color28.outline.g", 'i', &conf_serif_outline_color_g[27], OPTIONAL, NOSAVE},
	{"serif.color28.outline.b", 'i', &conf_serif_outline_color_b[27], OPTIONAL, NOSAVE},
	{"serif.color29.name", 's', &conf_serif_color_name[28], OPTIONAL, NOSAVE},
	{"serif.color29.r", 'i', &conf_serif_color_r[28], OPTIONAL, NOSAVE},
	{"serif.color29.g", 'i', &conf_serif_color_g[28], OPTIONAL, NOSAVE},
	{"serif.color29.b", 'i', &conf_serif_color_b[28], OPTIONAL, NOSAVE},
	{"serif.color29.outline.r", 'i', &conf_serif_outline_color_r[28], OPTIONAL, NOSAVE},
	{"serif.color29.outline.g", 'i', &conf_serif_outline_color_g[28], OPTIONAL, NOSAVE},
	{"serif.color29.outline.b", 'i', &conf_serif_outline_color_b[28], OPTIONAL, NOSAVE},
	{"serif.color30.name", 's', &conf_serif_color_name[29], OPTIONAL, NOSAVE},
	{"serif.color30.r", 'i', &conf_serif_color_r[29], OPTIONAL, NOSAVE},
	{"serif.color30.g", 'i', &conf_serif_color_g[29], OPTIONAL, NOSAVE},
	{"serif.color30.b", 'i', &conf_serif_color_b[29], OPTIONAL, NOSAVE},
	{"serif.color30.outline.r", 'i', &conf_serif_outline_color_r[29], OPTIONAL, NOSAVE},
	{"serif.color30.outline.g", 'i', &conf_serif_outline_color_g[29], OPTIONAL, NOSAVE},
	{"serif.color30.outline.b", 'i', &conf_serif_outline_color_b[29], OPTIONAL, NOSAVE},
	{"serif.color31.name", 's', &conf_serif_color_name[30], OPTIONAL, NOSAVE},
	{"serif.color31.r", 'i', &conf_serif_color_r[30], OPTIONAL, NOSAVE},
	{"serif.color31.g", 'i', &conf_serif_color_g[30], OPTIONAL, NOSAVE},
	{"serif.color31.b", 'i', &conf_serif_color_b[30], OPTIONAL, NOSAVE},
	{"serif.color31.outline.r", 'i', &conf_serif_outline_color_r[30], OPTIONAL, NOSAVE},
	{"serif.color31.outline.g", 'i', &conf_serif_outline_color_g[30], OPTIONAL, NOSAVE},
	{"serif.color31.outline.b", 'i', &conf_serif_outline_color_b[30], OPTIONAL, NOSAVE},
	{"serif.color32.name", 's', &conf_serif_color_name[31], OPTIONAL, NOSAVE},
	{"serif.color32.r", 'i', &conf_serif_color_r[31], OPTIONAL, NOSAVE},
	{"serif.color32.g", 'i', &conf_serif_color_g[31], OPTIONAL, NOSAVE},
	{"serif.color32.b", 'i', &conf_serif_color_b[31], OPTIONAL, NOSAVE},
	{"serif.color32.outline.r", 'i', &conf_serif_outline_color_r[31], OPTIONAL, NOSAVE},
	{"serif.color32.outline.g", 'i', &conf_serif_outline_color_g[31], OPTIONAL, NOSAVE},
	{"serif.color32.outline.b", 'i', &conf_serif_outline_color_b[31], OPTIONAL, NOSAVE},
	{"serif.color33.name", 's', &conf_serif_color_name[32], OPTIONAL, NOSAVE},
	{"serif.color33.r", 'i', &conf_serif_color_r[32], OPTIONAL, NOSAVE},
	{"serif.color33.g", 'i', &conf_serif_color_g[32], OPTIONAL, NOSAVE},
	{"serif.color33.b", 'i', &conf_serif_color_b[32], OPTIONAL, NOSAVE},
	{"serif.color33.outline.r", 'i', &conf_serif_outline_color_r[32], OPTIONAL, NOSAVE},
	{"serif.color33.outline.g", 'i', &conf_serif_outline_color_g[32], OPTIONAL, NOSAVE},
	{"serif.color33.outline.b", 'i', &conf_serif_outline_color_b[32], OPTIONAL, NOSAVE},
	{"serif.color34.name", 's', &conf_serif_color_name[33], OPTIONAL, NOSAVE},
	{"serif.color34.r", 'i', &conf_serif_color_r[33], OPTIONAL, NOSAVE},
	{"serif.color34.g", 'i', &conf_serif_color_g[33], OPTIONAL, NOSAVE},
	{"serif.color34.b", 'i', &conf_serif_color_b[33], OPTIONAL, NOSAVE},
	{"serif.color34.outline.r", 'i', &conf_serif_outline_color_r[33], OPTIONAL, NOSAVE},
	{"serif.color34.outline.g", 'i', &conf_serif_outline_color_g[33], OPTIONAL, NOSAVE},
	{"serif.color34.outline.b", 'i', &conf_serif_outline_color_b[33], OPTIONAL, NOSAVE},
	{"serif.color35.name", 's', &conf_serif_color_name[34], OPTIONAL, NOSAVE},
	{"serif.color35.r", 'i', &conf_serif_color_r[34], OPTIONAL, NOSAVE},
	{"serif.color35.g", 'i', &conf_serif_color_g[34], OPTIONAL, NOSAVE},
	{"serif.color35.b", 'i', &conf_serif_color_b[34], OPTIONAL, NOSAVE},
	{"serif.color35.outline.r", 'i', &conf_serif_outline_color_r[34], OPTIONAL, NOSAVE},
	{"serif.color35.outline.g", 'i', &conf_serif_outline_color_g[34], OPTIONAL, NOSAVE},
	{"serif.color35.outline.b", 'i', &conf_serif_outline_color_b[34], OPTIONAL, NOSAVE},
	{"serif.color36.name", 's', &conf_serif_color_name[35], OPTIONAL, NOSAVE},
	{"serif.color36.r", 'i', &conf_serif_color_r[35], OPTIONAL, NOSAVE},
	{"serif.color36.g", 'i', &conf_serif_color_g[35], OPTIONAL, NOSAVE},
	{"serif.color36.b", 'i', &conf_serif_color_b[35], OPTIONAL, NOSAVE},
	{"serif.color36.outline.r", 'i', &conf_serif_outline_color_r[35], OPTIONAL, NOSAVE},
	{"serif.color36.outline.g", 'i', &conf_serif_outline_color_g[35], OPTIONAL, NOSAVE},
	{"serif.color36.outline.b", 'i', &conf_serif_outline_color_b[35], OPTIONAL, NOSAVE},
	{"serif.color37.name", 's', &conf_serif_color_name[36], OPTIONAL, NOSAVE},
	{"serif.color37.r", 'i', &conf_serif_color_r[36], OPTIONAL, NOSAVE},
	{"serif.color37.g", 'i', &conf_serif_color_g[36], OPTIONAL, NOSAVE},
	{"serif.color37.b", 'i', &conf_serif_color_b[36], OPTIONAL, NOSAVE},
	{"serif.color37.outline.r", 'i', &conf_serif_outline_color_r[36], OPTIONAL, NOSAVE},
	{"serif.color37.outline.g", 'i', &conf_serif_outline_color_g[36], OPTIONAL, NOSAVE},
	{"serif.color37.outline.b", 'i', &conf_serif_outline_color_b[36], OPTIONAL, NOSAVE},
	{"serif.color38.name", 's', &conf_serif_color_name[37], OPTIONAL, NOSAVE},
	{"serif.color38.r", 'i', &conf_serif_color_r[37], OPTIONAL, NOSAVE},
	{"serif.color38.g", 'i', &conf_serif_color_g[37], OPTIONAL, NOSAVE},
	{"serif.color38.b", 'i', &conf_serif_color_b[37], OPTIONAL, NOSAVE},
	{"serif.color38.outline.r", 'i', &conf_serif_outline_color_r[37], OPTIONAL, NOSAVE},
	{"serif.color38.outline.g", 'i', &conf_serif_outline_color_g[37], OPTIONAL, NOSAVE},
	{"serif.color38.outline.b", 'i', &conf_serif_outline_color_b[37], OPTIONAL, NOSAVE},
	{"serif.color39.name", 's', &conf_serif_color_name[38], OPTIONAL, NOSAVE},
	{"serif.color39.r", 'i', &conf_serif_color_r[38], OPTIONAL, NOSAVE},
	{"serif.color39.g", 'i', &conf_serif_color_g[38], OPTIONAL, NOSAVE},
	{"serif.color39.b", 'i', &conf_serif_color_b[38], OPTIONAL, NOSAVE},
	{"serif.color39.outline.r", 'i', &conf_serif_outline_color_r[38], OPTIONAL, NOSAVE},
	{"serif.color39.outline.g", 'i', &conf_serif_outline_color_g[38], OPTIONAL, NOSAVE},
	{"serif.color39.outline.b", 'i', &conf_serif_outline_color_b[38], OPTIONAL, NOSAVE},
	{"serif.color40.name", 's', &conf_serif_color_name[39], OPTIONAL, NOSAVE},
	{"serif.color40.r", 'i', &conf_serif_color_r[39], OPTIONAL, NOSAVE},
	{"serif.color40.g", 'i', &conf_serif_color_g[39], OPTIONAL, NOSAVE},
	{"serif.color40.b", 'i', &conf_serif_color_b[39], OPTIONAL, NOSAVE},
	{"serif.color40.outline.r", 'i', &conf_serif_outline_color_r[39], OPTIONAL, NOSAVE},
	{"serif.color40.outline.g", 'i', &conf_serif_outline_color_g[39], OPTIONAL, NOSAVE},
	{"serif.color40.outline.b", 'i', &conf_serif_outline_color_b[39], OPTIONAL, NOSAVE},
	{"serif.color41.name", 's', &conf_serif_color_name[40], OPTIONAL, NOSAVE},
	{"serif.color41.r", 'i', &conf_serif_color_r[40], OPTIONAL, NOSAVE},
	{"serif.color41.g", 'i', &conf_serif_color_g[40], OPTIONAL, NOSAVE},
	{"serif.color41.b", 'i', &conf_serif_color_b[40], OPTIONAL, NOSAVE},
	{"serif.color41.outline.r", 'i', &conf_serif_outline_color_r[40], OPTIONAL, NOSAVE},
	{"serif.color41.outline.g", 'i', &conf_serif_outline_color_g[40], OPTIONAL, NOSAVE},
	{"serif.color41.outline.b", 'i', &conf_serif_outline_color_b[40], OPTIONAL, NOSAVE},
	{"serif.color42.name", 's', &conf_serif_color_name[41], OPTIONAL, NOSAVE},
	{"serif.color42.r", 'i', &conf_serif_color_r[41], OPTIONAL, NOSAVE},
	{"serif.color42.g", 'i', &conf_serif_color_g[41], OPTIONAL, NOSAVE},
	{"serif.color42.b", 'i', &conf_serif_color_b[41], OPTIONAL, NOSAVE},
	{"serif.color42.outline.r", 'i', &conf_serif_outline_color_r[41], OPTIONAL, NOSAVE},
	{"serif.color42.outline.g", 'i', &conf_serif_outline_color_g[41], OPTIONAL, NOSAVE},
	{"serif.color42.outline.b", 'i', &conf_serif_outline_color_b[41], OPTIONAL, NOSAVE},
	{"serif.color43.name", 's', &conf_serif_color_name[42], OPTIONAL, NOSAVE},
	{"serif.color43.r", 'i', &conf_serif_color_r[42], OPTIONAL, NOSAVE},
	{"serif.color43.g", 'i', &conf_serif_color_g[42], OPTIONAL, NOSAVE},
	{"serif.color43.b", 'i', &conf_serif_color_b[42], OPTIONAL, NOSAVE},
	{"serif.color43.outline.r", 'i', &conf_serif_outline_color_r[42], OPTIONAL, NOSAVE},
	{"serif.color43.outline.g", 'i', &conf_serif_outline_color_g[42], OPTIONAL, NOSAVE},
	{"serif.color43.outline.b", 'i', &conf_serif_outline_color_b[42], OPTIONAL, NOSAVE},
	{"serif.color44.name", 's', &conf_serif_color_name[43], OPTIONAL, NOSAVE},
	{"serif.color44.r", 'i', &conf_serif_color_r[43], OPTIONAL, NOSAVE},
	{"serif.color44.g", 'i', &conf_serif_color_g[43], OPTIONAL, NOSAVE},
	{"serif.color44.b", 'i', &conf_serif_color_b[43], OPTIONAL, NOSAVE},
	{"serif.color44.outline.r", 'i', &conf_serif_outline_color_r[43], OPTIONAL, NOSAVE},
	{"serif.color44.outline.g", 'i', &conf_serif_outline_color_g[43], OPTIONAL, NOSAVE},
	{"serif.color44.outline.b", 'i', &conf_serif_outline_color_b[43], OPTIONAL, NOSAVE},
	{"serif.color45.name", 's', &conf_serif_color_name[44], OPTIONAL, NOSAVE},
	{"serif.color45.r", 'i', &conf_serif_color_r[44], OPTIONAL, NOSAVE},
	{"serif.color45.g", 'i', &conf_serif_color_g[44], OPTIONAL, NOSAVE},
	{"serif.color45.b", 'i', &conf_serif_color_b[44], OPTIONAL, NOSAVE},
	{"serif.color45.outline.r", 'i', &conf_serif_outline_color_r[44], OPTIONAL, NOSAVE},
	{"serif.color45.outline.g", 'i', &conf_serif_outline_color_g[44], OPTIONAL, NOSAVE},
	{"serif.color45.outline.b", 'i', &conf_serif_outline_color_b[44], OPTIONAL, NOSAVE},
	{"serif.color46.name", 's', &conf_serif_color_name[45], OPTIONAL, NOSAVE},
	{"serif.color46.r", 'i', &conf_serif_color_r[45], OPTIONAL, NOSAVE},
	{"serif.color46.g", 'i', &conf_serif_color_g[45], OPTIONAL, NOSAVE},
	{"serif.color46.b", 'i', &conf_serif_color_b[45], OPTIONAL, NOSAVE},
	{"serif.color46.outline.r", 'i', &conf_serif_outline_color_r[45], OPTIONAL, NOSAVE},
	{"serif.color46.outline.g", 'i', &conf_serif_outline_color_g[45], OPTIONAL, NOSAVE},
	{"serif.color46.outline.b", 'i', &conf_serif_outline_color_b[45], OPTIONAL, NOSAVE},
	{"serif.color47.name", 's', &conf_serif_color_name[46], OPTIONAL, NOSAVE},
	{"serif.color47.r", 'i', &conf_serif_color_r[46], OPTIONAL, NOSAVE},
	{"serif.color47.g", 'i', &conf_serif_color_g[46], OPTIONAL, NOSAVE},
	{"serif.color47.b", 'i', &conf_serif_color_b[46], OPTIONAL, NOSAVE},
	{"serif.color47.outline.r", 'i', &conf_serif_outline_color_r[46], OPTIONAL, NOSAVE},
	{"serif.color47.outline.g", 'i', &conf_serif_outline_color_g[46], OPTIONAL, NOSAVE},
	{"serif.color47.outline.b", 'i', &conf_serif_outline_color_b[46], OPTIONAL, NOSAVE},
	{"serif.color48.name", 's', &conf_serif_color_name[47], OPTIONAL, NOSAVE},
	{"serif.color48.r", 'i', &conf_serif_color_r[47], OPTIONAL, NOSAVE},
	{"serif.color48.g", 'i', &conf_serif_color_g[47], OPTIONAL, NOSAVE},
	{"serif.color48.b", 'i', &conf_serif_color_b[47], OPTIONAL, NOSAVE},
	{"serif.color48.outline.r", 'i', &conf_serif_outline_color_r[47], OPTIONAL, NOSAVE},
	{"serif.color48.outline.g", 'i', &conf_serif_outline_color_g[47], OPTIONAL, NOSAVE},
	{"serif.color48.outline.b", 'i', &conf_serif_outline_color_b[47], OPTIONAL, NOSAVE},
	{"serif.color49.name", 's', &conf_serif_color_name[48], OPTIONAL, NOSAVE},
	{"serif.color49.r", 'i', &conf_serif_color_r[48], OPTIONAL, NOSAVE},
	{"serif.color49.g", 'i', &conf_serif_color_g[48], OPTIONAL, NOSAVE},
	{"serif.color49.b", 'i', &conf_serif_color_b[48], OPTIONAL, NOSAVE},
	{"serif.color49.outline.r", 'i', &conf_serif_outline_color_r[48], OPTIONAL, NOSAVE},
	{"serif.color49.outline.g", 'i', &conf_serif_outline_color_g[48], OPTIONAL, NOSAVE},
	{"serif.color49.outline.b", 'i', &conf_serif_outline_color_b[48], OPTIONAL, NOSAVE},
	{"serif.color50.name", 's', &conf_serif_color_name[49], OPTIONAL, NOSAVE},
	{"serif.color50.r", 'i', &conf_serif_color_r[49], OPTIONAL, NOSAVE},
	{"serif.color50.g", 'i', &conf_serif_color_g[49], OPTIONAL, NOSAVE},
	{"serif.color50.b", 'i', &conf_serif_color_b[49], OPTIONAL, NOSAVE},
	{"serif.color50.outline.r", 'i', &conf_serif_outline_color_r[49], OPTIONAL, NOSAVE},
	{"serif.color50.outline.g", 'i', &conf_serif_outline_color_g[49], OPTIONAL, NOSAVE},
	{"serif.color50.outline.b", 'i', &conf_serif_outline_color_b[49], OPTIONAL, NOSAVE},
	{"serif.color51.name", 's', &conf_serif_color_name[50], OPTIONAL, NOSAVE},
	{"serif.color51.r", 'i', &conf_serif_color_r[50], OPTIONAL, NOSAVE},
	{"serif.color51.g", 'i', &conf_serif_color_g[50], OPTIONAL, NOSAVE},
	{"serif.color51.b", 'i', &conf_serif_color_b[50], OPTIONAL, NOSAVE},
	{"serif.color51.outline.r", 'i', &conf_serif_outline_color_r[50], OPTIONAL, NOSAVE},
	{"serif.color51.outline.g", 'i', &conf_serif_outline_color_g[50], OPTIONAL, NOSAVE},
	{"serif.color51.outline.b", 'i', &conf_serif_outline_color_b[50], OPTIONAL, NOSAVE},
	{"serif.color52.name", 's', &conf_serif_color_name[51], OPTIONAL, NOSAVE},
	{"serif.color52.r", 'i', &conf_serif_color_r[51], OPTIONAL, NOSAVE},
	{"serif.color52.g", 'i', &conf_serif_color_g[51], OPTIONAL, NOSAVE},
	{"serif.color52.b", 'i', &conf_serif_color_b[51], OPTIONAL, NOSAVE},
	{"serif.color52.outline.r", 'i', &conf_serif_outline_color_r[51], OPTIONAL, NOSAVE},
	{"serif.color52.outline.g", 'i', &conf_serif_outline_color_g[51], OPTIONAL, NOSAVE},
	{"serif.color52.outline.b", 'i', &conf_serif_outline_color_b[51], OPTIONAL, NOSAVE},
	{"serif.color53.name", 's', &conf_serif_color_name[52], OPTIONAL, NOSAVE},
	{"serif.color53.r", 'i', &conf_serif_color_r[52], OPTIONAL, NOSAVE},
	{"serif.color53.g", 'i', &conf_serif_color_g[52], OPTIONAL, NOSAVE},
	{"serif.color53.b", 'i', &conf_serif_color_b[52], OPTIONAL, NOSAVE},
	{"serif.color53.outline.r", 'i', &conf_serif_outline_color_r[52], OPTIONAL, NOSAVE},
	{"serif.color53.outline.g", 'i', &conf_serif_outline_color_g[52], OPTIONAL, NOSAVE},
	{"serif.color53.outline.b", 'i', &conf_serif_outline_color_b[52], OPTIONAL, NOSAVE},
	{"serif.color54.name", 's', &conf_serif_color_name[53], OPTIONAL, NOSAVE},
	{"serif.color54.r", 'i', &conf_serif_color_r[53], OPTIONAL, NOSAVE},
	{"serif.color54.g", 'i', &conf_serif_color_g[53], OPTIONAL, NOSAVE},
	{"serif.color54.b", 'i', &conf_serif_color_b[53], OPTIONAL, NOSAVE},
	{"serif.color54.outline.r", 'i', &conf_serif_outline_color_r[53], OPTIONAL, NOSAVE},
	{"serif.color54.outline.g", 'i', &conf_serif_outline_color_g[53], OPTIONAL, NOSAVE},
	{"serif.color54.outline.b", 'i', &conf_serif_outline_color_b[53], OPTIONAL, NOSAVE},
	{"serif.color55.name", 's', &conf_serif_color_name[54], OPTIONAL, NOSAVE},
	{"serif.color55.r", 'i', &conf_serif_color_r[54], OPTIONAL, NOSAVE},
	{"serif.color55.g", 'i', &conf_serif_color_g[54], OPTIONAL, NOSAVE},
	{"serif.color55.b", 'i', &conf_serif_color_b[54], OPTIONAL, NOSAVE},
	{"serif.color55.outline.r", 'i', &conf_serif_outline_color_r[54], OPTIONAL, NOSAVE},
	{"serif.color55.outline.g", 'i', &conf_serif_outline_color_g[54], OPTIONAL, NOSAVE},
	{"serif.color55.outline.b", 'i', &conf_serif_outline_color_b[54], OPTIONAL, NOSAVE},
	{"serif.color56.name", 's', &conf_serif_color_name[55], OPTIONAL, NOSAVE},
	{"serif.color56.r", 'i', &conf_serif_color_r[55], OPTIONAL, NOSAVE},
	{"serif.color56.g", 'i', &conf_serif_color_g[55], OPTIONAL, NOSAVE},
	{"serif.color56.b", 'i', &conf_serif_color_b[55], OPTIONAL, NOSAVE},
	{"serif.color56.outline.r", 'i', &conf_serif_outline_color_r[55], OPTIONAL, NOSAVE},
	{"serif.color56.outline.g", 'i', &conf_serif_outline_color_g[55], OPTIONAL, NOSAVE},
	{"serif.color56.outline.b", 'i', &conf_serif_outline_color_b[55], OPTIONAL, NOSAVE},
	{"serif.color57.name", 's', &conf_serif_color_name[56], OPTIONAL, NOSAVE},
	{"serif.color57.r", 'i', &conf_serif_color_r[56], OPTIONAL, NOSAVE},
	{"serif.color57.g", 'i', &conf_serif_color_g[56], OPTIONAL, NOSAVE},
	{"serif.color57.b", 'i', &conf_serif_color_b[56], OPTIONAL, NOSAVE},
	{"serif.color57.outline.r", 'i', &conf_serif_outline_color_r[56], OPTIONAL, NOSAVE},
	{"serif.color57.outline.g", 'i', &conf_serif_outline_color_g[56], OPTIONAL, NOSAVE},
	{"serif.color57.outline.b", 'i', &conf_serif_outline_color_b[56], OPTIONAL, NOSAVE},
	{"serif.color58.name", 's', &conf_serif_color_name[57], OPTIONAL, NOSAVE},
	{"serif.color58.r", 'i', &conf_serif_color_r[57], OPTIONAL, NOSAVE},
	{"serif.color58.g", 'i', &conf_serif_color_g[57], OPTIONAL, NOSAVE},
	{"serif.color58.b", 'i', &conf_serif_color_b[57], OPTIONAL, NOSAVE},
	{"serif.color58.outline.r", 'i', &conf_serif_outline_color_r[57], OPTIONAL, NOSAVE},
	{"serif.color58.outline.g", 'i', &conf_serif_outline_color_g[57], OPTIONAL, NOSAVE},
	{"serif.color58.outline.b", 'i', &conf_serif_outline_color_b[57], OPTIONAL, NOSAVE},
	{"serif.color59.name", 's', &conf_serif_color_name[58], OPTIONAL, NOSAVE},
	{"serif.color59.r", 'i', &conf_serif_color_r[58], OPTIONAL, NOSAVE},
	{"serif.color59.g", 'i', &conf_serif_color_g[58], OPTIONAL, NOSAVE},
	{"serif.color59.b", 'i', &conf_serif_color_b[58], OPTIONAL, NOSAVE},
	{"serif.color59.outline.r", 'i', &conf_serif_outline_color_r[58], OPTIONAL, NOSAVE},
	{"serif.color59.outline.g", 'i', &conf_serif_outline_color_g[58], OPTIONAL, NOSAVE},
	{"serif.color59.outline.b", 'i', &conf_serif_outline_color_b[58], OPTIONAL, NOSAVE},
	{"serif.color60.name", 's', &conf_serif_color_name[59], OPTIONAL, NOSAVE},
	{"serif.color60.r", 'i', &conf_serif_color_r[59], OPTIONAL, NOSAVE},
	{"serif.color60.g", 'i', &conf_serif_color_g[59], OPTIONAL, NOSAVE},
	{"serif.color60.b", 'i', &conf_serif_color_b[59], OPTIONAL, NOSAVE},
	{"serif.color60.outline.r", 'i', &conf_serif_outline_color_r[59], OPTIONAL, NOSAVE},
	{"serif.color60.outline.g", 'i', &conf_serif_outline_color_g[59], OPTIONAL, NOSAVE},
	{"serif.color60.outline.b", 'i', &conf_serif_outline_color_b[59], OPTIONAL, NOSAVE},
	{"serif.color61.name", 's', &conf_serif_color_name[60], OPTIONAL, NOSAVE},
	{"serif.color61.r", 'i', &conf_serif_color_r[60], OPTIONAL, NOSAVE},
	{"serif.color61.g", 'i', &conf_serif_color_g[60], OPTIONAL, NOSAVE},
	{"serif.color61.b", 'i', &conf_serif_color_b[60], OPTIONAL, NOSAVE},
	{"serif.color61.outline.r", 'i', &conf_serif_outline_color_r[60], OPTIONAL, NOSAVE},
	{"serif.color61.outline.g", 'i', &conf_serif_outline_color_g[60], OPTIONAL, NOSAVE},
	{"serif.color61.outline.b", 'i', &conf_serif_outline_color_b[60], OPTIONAL, NOSAVE},
	{"serif.color62.name", 's', &conf_serif_color_name[61], OPTIONAL, NOSAVE},
	{"serif.color62.r", 'i', &conf_serif_color_r[61], OPTIONAL, NOSAVE},
	{"serif.color62.g", 'i', &conf_serif_color_g[61], OPTIONAL, NOSAVE},
	{"serif.color62.b", 'i', &conf_serif_color_b[61], OPTIONAL, NOSAVE},
	{"serif.color62.outline.r", 'i', &conf_serif_outline_color_r[61], OPTIONAL, NOSAVE},
	{"serif.color62.outline.g", 'i', &conf_serif_outline_color_g[61], OPTIONAL, NOSAVE},
	{"serif.color62.outline.b", 'i', &conf_serif_outline_color_b[61], OPTIONAL, NOSAVE},
	{"serif.color63.name", 's', &conf_serif_color_name[62], OPTIONAL, NOSAVE},
	{"serif.color63.r", 'i', &conf_serif_color_r[62], OPTIONAL, NOSAVE},
	{"serif.color63.g", 'i', &conf_serif_color_g[62], OPTIONAL, NOSAVE},
	{"serif.color63.b", 'i', &conf_serif_color_b[62], OPTIONAL, NOSAVE},
	{"serif.color63.outline.r", 'i', &conf_serif_outline_color_r[62], OPTIONAL, NOSAVE},
	{"serif.color63.outline.g", 'i', &conf_serif_outline_color_g[62], OPTIONAL, NOSAVE},
	{"serif.color63.outline.b", 'i', &conf_serif_outline_color_b[62], OPTIONAL, NOSAVE},
	{"serif.color64.name", 's', &conf_serif_color_name[63], OPTIONAL, NOSAVE},
	{"serif.color64.r", 'i', &conf_serif_color_r[63], OPTIONAL, NOSAVE},
	{"serif.color64.g", 'i', &conf_serif_color_g[63], OPTIONAL, NOSAVE},
	{"serif.color64.b", 'i', &conf_serif_color_b[63], OPTIONAL, NOSAVE},
	{"serif.color64.outline.r", 'i', &conf_serif_outline_color_r[63], OPTIONAL, NOSAVE},
	{"serif.color64.outline.g", 'i', &conf_serif_outline_color_g[63], OPTIONAL, NOSAVE},
	{"serif.color64.outline.b", 'i', &conf_serif_outline_color_b[63], OPTIONAL, NOSAVE},
	/* ジェネレータで出力したコードはここまで --> */
	{"character.focus", 'i', &conf_character_focus, OPTIONAL, NOSAVE},
	{"character.name1", 's', &conf_character_name[0], OPTIONAL, NOSAVE},
	{"character.file1", 's', &conf_character_file[0], OPTIONAL, NOSAVE},
	{"character.name2", 's', &conf_character_name[1], OPTIONAL, NOSAVE},
	{"character.file2", 's', &conf_character_file[1], OPTIONAL, NOSAVE},
	{"character.name3", 's', &conf_character_name[2], OPTIONAL, NOSAVE},
	{"character.file3", 's', &conf_character_file[2], OPTIONAL, NOSAVE},
	{"character.name4", 's', &conf_character_name[3], OPTIONAL, NOSAVE},
	{"character.file4", 's', &conf_character_file[3], OPTIONAL, NOSAVE},
	{"character.name5", 's', &conf_character_name[4], OPTIONAL, NOSAVE},
	{"character.file5", 's', &conf_character_file[4], OPTIONAL, NOSAVE},
	{"character.name6", 's', &conf_character_name[5], OPTIONAL, NOSAVE},
	{"character.file6", 's', &conf_character_file[5], OPTIONAL, NOSAVE},
	{"character.name7", 's', &conf_character_name[6], OPTIONAL, NOSAVE},
	{"character.file7", 's', &conf_character_file[6], OPTIONAL, NOSAVE},
	{"character.name8", 's', &conf_character_name[7], OPTIONAL, NOSAVE},
	{"character.file8", 's', &conf_character_file[7], OPTIONAL, NOSAVE},
	{"character.name9", 's', &conf_character_name[8], OPTIONAL, NOSAVE},
	{"character.file9", 's', &conf_character_file[8], OPTIONAL, NOSAVE},
	{"character.name10", 's', &conf_character_name[9], OPTIONAL, NOSAVE},
	{"character.file10", 's', &conf_character_file[9], OPTIONAL, NOSAVE},
	{"character.name11", 's', &conf_character_name[10], OPTIONAL, NOSAVE},
	{"character.file11", 's', &conf_character_file[10], OPTIONAL, NOSAVE},
	{"character.name12", 's', &conf_character_name[11], OPTIONAL, NOSAVE},
	{"character.file12", 's', &conf_character_file[11], OPTIONAL, NOSAVE},
	{"character.name13", 's', &conf_character_name[12], OPTIONAL, NOSAVE},
	{"character.file13", 's', &conf_character_file[12], OPTIONAL, NOSAVE},
	{"character.name14", 's', &conf_character_name[13], OPTIONAL, NOSAVE},
	{"character.file14", 's', &conf_character_file[13], OPTIONAL, NOSAVE},
	{"character.name15", 's', &conf_character_name[14], OPTIONAL, NOSAVE},
	{"character.file15", 's', &conf_character_file[14], OPTIONAL, NOSAVE},
	{"character.name16", 's', &conf_character_name[15], OPTIONAL, NOSAVE},
	{"character.file16", 's', &conf_character_file[15], OPTIONAL, NOSAVE},
	{"character.name17", 's', &conf_character_name[16], OPTIONAL, NOSAVE},
	{"character.file17", 's', &conf_character_file[16], OPTIONAL, NOSAVE},
	{"character.name18", 's', &conf_character_name[17], OPTIONAL, NOSAVE},
	{"character.file18", 's', &conf_character_file[17], OPTIONAL, NOSAVE},
	{"character.name19", 's', &conf_character_name[18], OPTIONAL, NOSAVE},
	{"character.file19", 's', &conf_character_file[18], OPTIONAL, NOSAVE},
	{"character.name20", 's', &conf_character_name[19], OPTIONAL, NOSAVE},
	{"character.file20", 's', &conf_character_file[19], OPTIONAL, NOSAVE},
	{"character.name21", 's', &conf_character_name[20], OPTIONAL, NOSAVE},
	{"character.file21", 's', &conf_character_file[20], OPTIONAL, NOSAVE},
	{"character.name22", 's', &conf_character_name[21], OPTIONAL, NOSAVE},
	{"character.file22", 's', &conf_character_file[21], OPTIONAL, NOSAVE},
	{"character.name23", 's', &conf_character_name[22], OPTIONAL, NOSAVE},
	{"character.file23", 's', &conf_character_file[22], OPTIONAL, NOSAVE},
	{"character.name24", 's', &conf_character_name[23], OPTIONAL, NOSAVE},
	{"character.file24", 's', &conf_character_file[23], OPTIONAL, NOSAVE},
	{"character.name25", 's', &conf_character_name[24], OPTIONAL, NOSAVE},
	{"character.file25", 's', &conf_character_file[24], OPTIONAL, NOSAVE},
	{"character.name26", 's', &conf_character_name[25], OPTIONAL, NOSAVE},
	{"character.file26", 's', &conf_character_file[25], OPTIONAL, NOSAVE},
	{"character.name27", 's', &conf_character_name[26], OPTIONAL, NOSAVE},
	{"character.file27", 's', &conf_character_file[26], OPTIONAL, NOSAVE},
	{"character.name28", 's', &conf_character_name[27], OPTIONAL, NOSAVE},
	{"character.file28", 's', &conf_character_file[27], OPTIONAL, NOSAVE},
	{"character.name29", 's', &conf_character_name[28], OPTIONAL, NOSAVE},
	{"character.file29", 's', &conf_character_file[28], OPTIONAL, NOSAVE},
	{"character.name30", 's', &conf_character_name[29], OPTIONAL, NOSAVE},
	{"character.file30", 's', &conf_character_file[29], OPTIONAL, NOSAVE},
	{"character.name31", 's', &conf_character_name[30], OPTIONAL, NOSAVE},
	{"character.file31", 's', &conf_character_file[30], OPTIONAL, NOSAVE},
	{"character.name32", 's', &conf_character_name[31], OPTIONAL, NOSAVE},
	{"character.file32", 's', &conf_character_file[31], OPTIONAL, NOSAVE},
	{"stage.ch.margin.bottom", 'i', &conf_stage_ch_margin_bottom, OPTIONAL, SAVE},
	{"stage.ch.margin.left", 'i', &conf_stage_ch_margin_left, OPTIONAL, SAVE},
	{"stage.ch.margin.right", 'i', &conf_stage_ch_margin_right, OPTIONAL, SAVE},
	{"kirakira.on", 'i', &conf_kirakira_on, OPTIONAL, SAVE},
	{"kirakira.frame", 'f', &conf_kirakira_frame, OPTIONAL, SAVE},
	{"kirakira.file1", 's', &conf_kirakira_file[0], OPTIONAL, SAVE},
	{"kirakira.file2", 's', &conf_kirakira_file[1], OPTIONAL, SAVE},
	{"kirakira.file3", 's', &conf_kirakira_file[2], OPTIONAL, SAVE},
	{"kirakira.file4", 's', &conf_kirakira_file[3], OPTIONAL, SAVE},
	{"kirakira.file5", 's', &conf_kirakira_file[4], OPTIONAL, SAVE},
	{"kirakira.file6", 's', &conf_kirakira_file[5], OPTIONAL, SAVE},
	{"kirakira.file7", 's', &conf_kirakira_file[6], OPTIONAL, SAVE},
	{"kirakira.file8", 's', &conf_kirakira_file[7], OPTIONAL, SAVE},
	{"kirakira.file9", 's', &conf_kirakira_file[8], OPTIONAL, SAVE},
	{"kirakira.file10", 's', &conf_kirakira_file[9], OPTIONAL, SAVE},
	{"kirakira.file11", 's', &conf_kirakira_file[10], OPTIONAL, SAVE},
	{"kirakira.file12", 's', &conf_kirakira_file[11], OPTIONAL, SAVE},
	{"kirakira.file13", 's', &conf_kirakira_file[12], OPTIONAL, SAVE},
	{"kirakira.file14", 's', &conf_kirakira_file[13], OPTIONAL, SAVE},
	{"kirakira.file15", 's', &conf_kirakira_file[14], OPTIONAL, SAVE},
	{"kirakira.file16", 's', &conf_kirakira_file[15], OPTIONAL, SAVE},
	{"tts.enable", 'i', &conf_tts_enable, OPTIONAL, SAVE},
	{"tts.user", 'i', &conf_tts_user, OPTIONAL, SAVE},
	{"click.disable", 'i', &conf_click_disable, OPTIONAL, SAVE},
	{"gui.save.last.page", 'i', &conf_gui_save_last_page, OPTIONAL, SAVE},
	{"voice.stop.off", 'i', &conf_voice_stop_off, OPTIONAL, SAVE},
	{"window.fullscreen.disable", 'i', &conf_window_fullscreen_disable, OPTIONAL, SAVE},
	{"window.maximize.disable", 'i', &conf_window_maximize_disable, OPTIONAL, SAVE},
	{"window.title.separator", 's', &conf_window_title_separator, OPTIONAL, SAVE},
	{"window.title.chapter.disable", 'i', &conf_window_title_chapter_disable, OPTIONAL, SAVE},
	{"msgbox.show.on.ch", 'i', &conf_msgbox_show_on_ch, OPTIONAL, SAVE},
	{"msgbox.show.on.bg", 'i', &conf_msgbox_show_on_bg, OPTIONAL, SAVE},
	{"msgbox.show.on.choose", 'i', &conf_msgbox_show_on_choose, OPTIONAL, SAVE},
	{"beep.adjustment", 'f', &conf_beep_adjustment, OPTIONAL, NOSAVE},
	{"serif.quote", 'i', &conf_serif_quote, OPTIONAL, SAVE},
	{"sysmenu.transition", 'i', &conf_sysmenu_transition, OPTIONAL, SAVE},
	{"msgbox.history.disable", 'i', &conf_msgbox_history_disable, OPTIONAL, SAVE},
	{"serif.color.name.only", 'i', &conf_serif_color_name_only, OPTIONAL, SAVE},
	{"sav.name", 's', &conf_sav_name, OPTIONAL, NOSAVE},
	{"release", 'i', &conf_release, OPTIONAL, NOSAVE},
};

#define RULE_TBL_SIZE	((int)(sizeof(rule_tbl) / sizeof(struct rule)))

/* グローバルセーブデータに保存するキーのテーブル */
const char *global_tbl[] = {
	"font.file",
	"locale.force",
	"gui.save.last.page",
};

#define GLOBAL_TBL_SIZE	((int)(sizeof(global_tbl) / sizeof(const char*)))

/* 各コンフィグがロード済みかどうか */
static bool loaded_tbl[RULE_TBL_SIZE];

/*
 * 前方参照
 */
static bool read_conf(void);
static bool save_value(const char *k, const char *v);
static bool check_conf(void);
static void set_locale_mapping(void);
static bool overwrite_config_locale_force(const char *val);
static bool overwrite_config_font_file(const char *val);

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

	rf = open_rfile(CONF_DIR, CONFIG_FILE, false);
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
	int i;

	/* ルールテーブルからキーを探して値を保存する */
	for (i = 0; i < RULE_TBL_SIZE; i++) {
		/* キーが一致しなければ次のルールへ */
		if (strcmp(rule_tbl[i].key, k) != 0)
			continue;

		/* すでに値が設定されたキーの場合 */
		if (loaded_tbl[i]) {
			log_duplicated_conf(k);
			return false;
		}

		/* 保存されない(無視される)キーの場合 */
		if (rule_tbl[i].var_ptr == NULL)
			return true;

		/* 型ごとに変換する */
		if (rule_tbl[i].type == 'i') {
			*(int *)rule_tbl[i].var_ptr = atoi(v);
		} else if (rule_tbl[i].type == 'f') {
			*(float *)rule_tbl[i].var_ptr = (float)atof(v);
		} else if (rule_tbl[i].type == 's') {
			/* 文字列の場合は複製する */
			dup = strdup(v);
			if (dup == NULL) {
				log_memory();
				return false;
			}
			*(char **)rule_tbl[i].var_ptr = dup;
		} else {
			assert(0);
		}

		loaded_tbl[i] = true;

		return true;
	}
	log_unknown_conf(k);
	return false;
}

/* 読み込まれなかった必須コンフィグをチェックする */
static bool check_conf(void)
{
	int i;

	for (i = 0; i < RULE_TBL_SIZE; i++) {
		if (!rule_tbl[i].optional && !loaded_tbl[i]) {
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
	int i;

	/* 文字列のプロパティは解放する */
	for (i = 0; i < RULE_TBL_SIZE; i++) {
		loaded_tbl[i] = false;
		if (rule_tbl[i].type == 's' && rule_tbl[i].var_ptr != NULL) {
			free(*(char **)rule_tbl[i].var_ptr);
			*(char **)rule_tbl[i].var_ptr = NULL;
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
	if (conf_locale_force != NULL) {
		/* ロケールの上書きが行われたので、再初期化する */
		init_locale_code();
	}

	/*
	 * グローバルボリュームをセットする
	 *  - この値はグローバルセーブデータがある場合はload_global_data()にて
	 *    上書きされる
	 */
	set_mixer_global_volume(BGM_STREAM, conf_sound_vol_bgm);
	set_mixer_global_volume(VOICE_STREAM, conf_sound_vol_voice);
	set_mixer_global_volume(SE_STREAM, conf_sound_vol_se);

	/* キャラクタボリュームをセットする */
	for (i = 0; i < CH_VOL_SLOTS; i++)
		set_character_volume(i, conf_sound_vol_character);

	/* クリックアニメーションのフレーム数をカウントする */
	for (click_frames = 16; click_frames > 1; click_frames--)
		if (conf_click_file[click_frames - 1] != NULL)
			break;

#ifdef USE_DEBUGGER
	conf_window_resize = 1;
#endif
	return true;
}

/* ロケールを整数に変換する */
void init_locale_code(void)
{
	const char *locale;

	/*
	 * この関数は各プラットフォームの初期化コードから呼ばれる。
	 * その際、コンフィグはロードされていないので、システムのロケールを
	 * 必ず使う。コンフィグのロード後に、locale.forceがある場合は、
	 * もう一度この関数が呼び出される。
	 */
	if (conf_locale_force == NULL)
		locale = get_system_locale();
	else
		locale = conf_locale_force;

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
	if (conf_locale_force == NULL)
		locale = get_system_locale();
	else
		locale = conf_locale_force;
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
	int i;
	char *s;

	assert(key != NULL);
	assert(val != NULL);

	/* 特別扱いするキーの場合を処理する */
	if (strcmp(key, "locale.force") == 0)
		return overwrite_config_locale_force(val);
	else if (strcmp(key, "font.file") == 0)
		return overwrite_config_font_file(val);

	/* 一般扱いのキーの場合を処理する */
	for (i = 0; i < RULE_TBL_SIZE; i++) {
		/* キーがみつかった場合 */
		if (strcmp(rule_tbl[i].key, key) == 0)
			break;
	}
	if (i == RULE_TBL_SIZE)
		return false;

	/* 無視されるキーである場合 */
	if (rule_tbl[i].var_ptr == NULL)
		return true;

	/* キーの型ごとに処理する */
	switch (rule_tbl[i].type) {
	case 'i':
		/* var_ptrはint型変数へのポインタ */
		*(int *)rule_tbl[i].var_ptr = atoi(val);
		break;
	case 'f':
		/* var_ptrはfloat型変数へのポインタ */
		*(float *)rule_tbl[i].var_ptr = (float)atof(val);
		break;
	case 's':
		/* 既存の文字列を解放する */
		if (*(char **)rule_tbl[i].var_ptr != NULL) {
			free(*(char **)rule_tbl[i].var_ptr);
			*(char **)rule_tbl[i].var_ptr = NULL;
		}

		/* 文字列のコピーを作成して代入する */
		if (strcmp(val, "") != 0) {
			s = strdup(val);
			if (s == NULL) {
				log_memory();
				return false;
			}
			*(char **)rule_tbl[i].var_ptr = s;
		}
		break;
	default:
		assert(INVALID_CONFIG_TYPE);
		break;
	}

	/* レイヤー座標の範囲を行う */
	update_layer_position_by_config();

	/* 画像読み込みの後処理を行う */
	if (strcmp(key, "msgbox.bg.file") == 0)
		update_msgbox(false);
	else if (strcmp(key, "msgbox.fg.file") == 0)
		update_msgbox(false);
	else if (strcmp(key, "namebox.file") == 0)
		update_namebox();
	else if (strcmp(key, "switch.bg.file") == 0)
		update_switchbox(false, 0);
	else if (strcmp(key, "switch.fg.file") == 0)
		update_switchbox(true, 0);
	else if (strcmp(key, "switch.bg.file2") == 0)
		update_switchbox(false, 1);
	else if (strcmp(key, "switch.fg.file2") == 0)
		update_switchbox(true, 1);
	else if (strcmp(key, "switch.bg.file3") == 0)
		update_switchbox(false, 2);
	else if (strcmp(key, "switch.fg.file3") == 0)
		update_switchbox(true, 2);
	else if (strcmp(key, "switch.bg.file4") == 0)
		update_switchbox(false, 3);
	else if (strcmp(key, "switch.fg.file4") == 0)
		update_switchbox(true, 3);
	else if (strcmp(key, "switch.bg.file5") == 0)
		update_switchbox(false, 4);
	else if (strcmp(key, "switch.fg.file5") == 0)
		update_switchbox(true, 4);
	else if (strcmp(key, "switch.bg.file6") == 0)
		update_switchbox(false, 5);
	else if (strcmp(key, "switch.fg.file6") == 0)
		update_switchbox(true, 5);
	else if (strcmp(key, "switch.bg.file7") == 0)
		update_switchbox(false, 6);
	else if (strcmp(key, "switch.fg.file7") == 0)
		update_switchbox(true, 6);
	else if (strcmp(key, "switch.bg.file8") == 0)
		update_switchbox(false, 7);
	else if (strcmp(key, "switch.fg.file8") == 0)
		update_switchbox(true, 7);

	return true;
}

/* locale.forceコンフィグの書き換えを行う */
static bool overwrite_config_locale_force(const char *val)
{
	assert(val != NULL);

	if (conf_locale_force != NULL) {
		free(conf_locale_force);
		conf_locale_force = NULL;
	}

	if (strcmp(val, "") != 0) {
		conf_locale_force = strdup(val);
		if (conf_locale_force == NULL) {
			log_memory();
			return false;
		}
	}

	init_locale_code();
	set_locale_mapping();
	return true;
}

/* font.fileコンフィグの書き換えを行う */
static bool overwrite_config_font_file(const char *val)
{
	assert(val != NULL);

	/* font.fileの書き換えを行う */
	if (strcmp(val, "") != 0) {
		assert(conf_font_global_file != NULL);
		free(conf_font_global_file);
		conf_font_global_file = strdup(val);
		if (conf_font_global_file == NULL) {
			log_memory();
			return false;
		}
	}

	/* フォントサブシステムを更新する */
	if (!update_global_font())
		return false;

	return true;
}

/*
 * コンフィグのセーブデータへの書き出し関連
 */

/*
 * セーブデータに書き出すコンフィグの値を取得する
 */
const char *get_config_key_for_save_data(int index)
{
	int i, save_key_count;

	save_key_count = 0;
	for (i = 0; i < RULE_TBL_SIZE; i++) {
		if (!rule_tbl[i].save)
			continue;
		if (save_key_count == index)
			return rule_tbl[i].key;
		save_key_count++;
	}
	return NULL;
}

/*
 * コンフィグをグローバルセーブデータにするかを取得する
 */
bool is_config_key_global(const char *key)
{
	int i;

	for (i = 0; i < GLOBAL_TBL_SIZE; i++)
		if (strcmp(global_tbl[i], key) == 0)
			return true;
	return false;
}

/*
 * コンフィグの型を取得する('s', 'i', 'f')
 */
char get_config_type_for_key(const char *key)
{
	int i;

	assert(key != NULL);

	for (i = 0; i < RULE_TBL_SIZE; i++)
		if (strcmp(rule_tbl[i].key, key) == 0)
			return rule_tbl[i].type;

	assert(CONFIG_KEY_NOT_FOUND);
	return '?';
}

/*
 * 文字列型のコンフィグ値を取得する
 */
const char *get_string_config_value_for_key(const char *key)
{
	int i;

	assert(key != NULL);
	for (i = 0; i < RULE_TBL_SIZE; i++) {
		if (strcmp(rule_tbl[i].key, key) == 0) {
			assert(rule_tbl[i].type == 's');
			if (rule_tbl[i].var_ptr == NULL)
				return "";
			return *(char **)rule_tbl[i].var_ptr;
		}
	}
	assert(CONFIG_KEY_NOT_FOUND);
	return NULL;
}

/*
 * 整数型のコンフィグ値を取得する
 */
int get_int_config_value_for_key(const char *key)
{
	int i;

	assert(key != NULL);
	for (i = 0; i < RULE_TBL_SIZE; i++) {
		if (strcmp(rule_tbl[i].key, key) == 0) {
			assert(rule_tbl[i].type == 'i');
			assert(rule_tbl[i].var_ptr != NULL);
			return *(int *)rule_tbl[i].var_ptr;
		}
	}
	assert(CONFIG_KEY_NOT_FOUND);
	return -1;
}

/*
 * 浮動小数点数型のコンフィグ値を取得する
 */
float get_float_config_value_for_key(const char *key)
{
	int i;

	assert(key != NULL);
	for (i = 0; i < RULE_TBL_SIZE; i++) {
		if (strcmp(rule_tbl[i].key, key) == 0) {
			assert(rule_tbl[i].type == 'f');
			assert(rule_tbl[i].var_ptr != NULL);
			return *(float *)rule_tbl[i].var_ptr;
		}
	}
	assert(CONFIG_KEY_NOT_FOUND);
	return NAN;
}
