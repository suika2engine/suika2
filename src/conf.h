/* -*- coding: utf-8-with-signature; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Suika 2
 * Copyright (C) 2001-2022, TABATA Keiichi. All rights reserved.
 */

/*
 * [Changes]
 *  - 2016/06/25 作成
 *  - 2017/08/14 スイッチに対応
 *  - 2017/09/25 セリフの色付けに対応
 *  - 2017/10/31 スイッチのクリック音に対応
 *  - 2019/09/17 NEWSに対応
 *  - 2022/07/19 システムメニューに対応
 */

#ifndef SUIKA_CONF_H
#define SUIKA_CONF_H

#include "types.h"
#include "mixer.h"

/*
 * 言語の設定
 */
extern int conf_i18n;
extern char *conf_language_jp;
extern char *conf_language_en;
extern char *conf_language_fr;
extern char *conf_language_de;
extern char *conf_language_es;
extern char *conf_language_it;
extern char *conf_language_el;
extern char *conf_language_ru;
extern char *conf_language_other;
extern char *conf_language;

/*
 * ウィンドウの設定
 */
extern char *conf_window_title;
extern int conf_window_width;
extern int conf_window_height;
extern int conf_window_white;

/*
 * フォントの設定
 */
extern char *conf_font_file;
extern int conf_font_size;
extern int conf_font_color_r;
extern int conf_font_color_g;
extern int conf_font_color_b;
extern int conf_font_outline_color_r;
extern int conf_font_outline_color_g;
extern int conf_font_outline_color_b;
extern int conf_font_outline_remove;

/*
 * 名前ボックスの設定
 */
extern char *conf_namebox_file;
extern int conf_namebox_x;
extern int conf_namebox_y;
extern int conf_namebox_margin_top;
extern int conf_namebox_centering_no;
extern int conf_namebox_margin_left;
extern int conf_namebox_hidden;

/*
 * メッセージボックスの設定
 */
extern char *conf_msgbox_bg_file;
extern char *conf_msgbox_fg_file;
extern int conf_msgbox_x;
extern int conf_msgbox_y;
extern int conf_msgbox_margin_left;
extern int conf_msgbox_margin_top;
extern int conf_msgbox_margin_right;
extern int conf_msgbox_margin_line;
extern float conf_msgbox_speed;
extern int conf_msgbox_btn_qsave_x;
extern int conf_msgbox_btn_qsave_y;
extern int conf_msgbox_btn_qsave_width;
extern int conf_msgbox_btn_qsave_height;
extern int conf_msgbox_btn_qload_x;
extern int conf_msgbox_btn_qload_y;
extern int conf_msgbox_btn_qload_width;
extern int conf_msgbox_btn_qload_height;
extern int conf_msgbox_btn_save_x;
extern int conf_msgbox_btn_save_y;
extern int conf_msgbox_btn_save_width;
extern int conf_msgbox_btn_save_height;
extern int conf_msgbox_btn_load_x;
extern int conf_msgbox_btn_load_y;
extern int conf_msgbox_btn_load_width;
extern int conf_msgbox_btn_load_height;
extern int conf_msgbox_btn_auto_x;
extern int conf_msgbox_btn_auto_y;
extern int conf_msgbox_btn_auto_width;
extern int conf_msgbox_btn_auto_height;
extern int conf_msgbox_btn_skip_x;
extern int conf_msgbox_btn_skip_y;
extern int conf_msgbox_btn_skip_width;
extern int conf_msgbox_btn_skip_height;
extern int conf_msgbox_btn_history_x;
extern int conf_msgbox_btn_history_y;
extern int conf_msgbox_btn_history_width;
extern int conf_msgbox_btn_history_height;
extern int conf_msgbox_btn_config_x;
extern int conf_msgbox_btn_config_y;
extern int conf_msgbox_btn_config_width;
extern int conf_msgbox_btn_config_height;
extern int conf_msgbox_btn_hide_x;
extern int conf_msgbox_btn_hide_y;
extern int conf_msgbox_btn_hide_width;
extern int conf_msgbox_btn_hide_height;
extern char *conf_msgbox_btn_qsave_se;
extern char *conf_msgbox_btn_qload_se;
extern char *conf_msgbox_btn_save_se;
extern char *conf_msgbox_btn_load_se;
extern char *conf_msgbox_btn_auto_se;
extern char *conf_msgbox_btn_skip_se;
extern char *conf_msgbox_btn_history_se;
extern char *conf_msgbox_btn_config_se;
extern char *conf_msgbox_btn_change_se;
extern char *conf_msgbox_save_se;
extern char *conf_msgbox_history_se;
extern char *conf_msgbox_config_se;
extern char *conf_msgbox_hide_se;
extern char *conf_msgbox_show_se;
extern char *conf_msgbox_auto_cancel_se;
extern char *conf_msgbox_skip_cancel_se;
extern int conf_msgbox_skip_unseen;

/*
 * クリックアニメーションの設定
 */
extern int conf_click_x;
extern int conf_click_y;
extern int conf_click_move;
extern char *conf_click_file1;
extern char *conf_click_file2;
extern char *conf_click_file3;
extern char *conf_click_file4;
extern char *conf_click_file5;
extern char *conf_click_file6;
extern float conf_click_interval;

/*
 * 選択肢ボックスの設定
 */
extern char *conf_selbox_bg_file;
extern char *conf_selbox_fg_file;
extern int conf_selbox_x;
extern int conf_selbox_y;
extern int conf_selbox_margin_y;
extern char *conf_selbox_change_se;

/*
 * スイッチの設定
 */
extern char *conf_switch_bg_file;
extern char *conf_switch_fg_file;
extern int conf_switch_x;
extern int conf_switch_y;
extern int conf_switch_margin_y;
extern int conf_switch_text_margin_y;
extern char *conf_switch_parent_click_se_file;
extern char *conf_switch_child_click_se_file;
extern char *conf_switch_change_se;

/*
 * NEWSの設定
 */
extern char *conf_news_bg_file;
extern char *conf_news_fg_file;
extern int conf_news_margin;
extern int conf_news_text_margin_y;
extern char *conf_news_change_se;

/*
 * 回想画面(@retrospect)の設定
 */
extern char *conf_retrospect_change_se;

/*
 * セーブ・ロード画面の設定
 */
extern int conf_save_data_thumb_width;
extern int conf_save_data_thumb_height;

/*
 * メニュー(@menu)の設定
 */
extern char *conf_menu_change_se;

/* 
 * システムメニューの設定
 */
extern int conf_sysmenu_x;
extern int conf_sysmenu_y;
extern char *conf_sysmenu_idle_file;
extern char *conf_sysmenu_hover_file;
extern char *conf_sysmenu_disable_file;
extern int conf_sysmenu_qsave_x;
extern int conf_sysmenu_qsave_y;
extern int conf_sysmenu_qsave_width;
extern int conf_sysmenu_qsave_height;
extern int conf_sysmenu_qload_x;
extern int conf_sysmenu_qload_y;
extern int conf_sysmenu_qload_width;
extern int conf_sysmenu_qload_height;
extern int conf_sysmenu_save_x;
extern int conf_sysmenu_save_y;
extern int conf_sysmenu_save_width;
extern int conf_sysmenu_save_height;
extern int conf_sysmenu_load_x;
extern int conf_sysmenu_load_y;
extern int conf_sysmenu_load_width;
extern int conf_sysmenu_load_height;
extern int conf_sysmenu_auto_x;
extern int conf_sysmenu_auto_y;
extern int conf_sysmenu_auto_width;
extern int conf_sysmenu_auto_height;
extern int conf_sysmenu_skip_x;
extern int conf_sysmenu_skip_y;
extern int conf_sysmenu_skip_width;
extern int conf_sysmenu_skip_height;
extern int conf_sysmenu_history_x;
extern int conf_sysmenu_history_y;
extern int conf_sysmenu_history_width;
extern int conf_sysmenu_history_height;
extern int conf_sysmenu_config_x;
extern int conf_sysmenu_config_y;
extern int conf_sysmenu_config_width;
extern int conf_sysmenu_config_height;
extern char *conf_sysmenu_enter_se;
extern char *conf_sysmenu_leave_se;
extern char *conf_sysmenu_change_se;
extern char *conf_sysmenu_qsave_se;
extern char *conf_sysmenu_qload_se;
extern char *conf_sysmenu_save_se;
extern char *conf_sysmenu_load_se;
extern char *conf_sysmenu_auto_se;
extern char *conf_sysmenu_skip_se;
extern char *conf_sysmenu_history_se;
extern char *conf_sysmenu_config_se;
extern int conf_sysmenu_collapsed_x;
extern int conf_sysmenu_collapsed_y;
extern char *conf_sysmenu_collapsed_idle_file;
extern char *conf_sysmenu_collapsed_hover_file;
extern char *conf_sysmenu_collapsed_se;
extern int conf_sysmenu_hidden;

/*
 * オートモードの設定
 */
extern char *conf_automode_banner_file;
extern int conf_automode_banner_x;
extern int conf_automode_banner_y;
extern float conf_automode_speed;

/*
 * スキップモードの設定
 */
extern char *conf_skipmode_banner_file;
extern int conf_skipmode_banner_x;
extern int conf_skipmode_banner_y;

/*
 * サウンドの設定
 */
extern float conf_sound_vol_bgm;
extern float conf_sound_vol_voice;
extern float conf_sound_vol_se;
extern float conf_sound_vol_character;
extern char *conf_sound_character_name[CH_VOL_SLOTS]; /* index0は未使用 */

/*
 * セリフの色付け
 */
#define SERIF_COLOR_COUNT	(64)

extern char *conf_serif_color_name[SERIF_COLOR_COUNT];
extern int conf_serif_color_r[SERIF_COLOR_COUNT];
extern int conf_serif_color_g[SERIF_COLOR_COUNT];
extern int conf_serif_color_b[SERIF_COLOR_COUNT];
extern int conf_serif_outline_color_r[SERIF_COLOR_COUNT];
extern int conf_serif_outline_color_g[SERIF_COLOR_COUNT];
extern int conf_serif_outline_color_b[SERIF_COLOR_COUNT];

/*
 * UIメッセージ
 */
extern char *conf_ui_msg_quit;
extern char *conf_ui_msg_title;
extern char *conf_ui_msg_delete;
extern char *conf_ui_msg_overwrite;
extern char *conf_ui_msg_default;

/*
 * その他の設定
 */
extern int conf_voice_stop_off;
extern int conf_window_fullscreen_disable;
extern int conf_window_maximize_disable;
extern char *conf_window_title_separator;
extern int conf_window_title_chapter_disable;
extern int conf_click_disable;
extern int conf_msgbox_show_on_ch;
extern float conf_beep_adjustment;
extern int conf_release;

/* コンフィグの初期化処理を行う */
bool init_conf(void);

/* コンフィグの終了処理を行う */
void cleanup_conf(void);

/* コンフィグの値を元に各種設定を初期値にする */
bool apply_initial_values(void);

#endif
