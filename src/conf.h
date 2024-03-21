/* -*- coding: utf-8; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Suika2
 * Copyright (C) 2001-2022, Keiichi Tabata. All rights reserved.
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
#include "vars.h"
#include "mixer.h"

/*
 * 言語の設定
 */
extern char *conf_language_jp;
extern char *conf_language_en;
extern char *conf_language_fr;
extern char *conf_language_de;
extern char *conf_language_es;
extern char *conf_language_it;
extern char *conf_language_el;
extern char *conf_language_ru;
extern char *conf_language_other;

/* 言語の設定から導出されたロケール */
enum language_code {
	LOCALE_EN,
	LOCALE_FR,
	LOCALE_DE,
	LOCALE_ES,
	LOCALE_IT,
	LOCALE_EL,
	LOCALE_RU,
	LOCALE_ZH,
	LOCALE_TW,
	LOCALE_JA,
	LOCALE_OTHER,
};
extern int conf_locale;
extern const char *conf_locale_mapped;

/*
 * ウィンドウの設定
 */
extern char *conf_window_title;
extern int conf_window_width;
extern int conf_window_height;
extern int conf_window_white;
extern int conf_window_menubar;
extern int conf_window_resize;
extern int conf_window_default_width;
extern int conf_window_default_height;

/*
 * フォントの設定
 */
extern int conf_font_select;
extern char *conf_font_global_file;
extern char *conf_font_main_file;
extern char *conf_font_alt1_file;
extern char *conf_font_alt2_file;
extern int conf_font_size;
extern int conf_font_color_r;
extern int conf_font_color_g;
extern int conf_font_color_b;
extern int conf_font_outline_remove;
extern int conf_font_outline_add;
extern int conf_font_outline_color_r;
extern int conf_font_outline_color_g;
extern int conf_font_outline_color_b;
extern int conf_font_ruby_size;

/*
 * 名前ボックスの設定
 */
extern char *conf_namebox_file;
extern int conf_namebox_font_select;
extern int conf_namebox_font_size;
extern int conf_namebox_font_outline;
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
extern int conf_msgbox_margin_bottom;
extern int conf_msgbox_margin_line;
extern int conf_msgbox_margin_char;
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
extern int conf_msgbox_dim;
extern int conf_msgbox_dim_color_r;
extern int conf_msgbox_dim_color_g;
extern int conf_msgbox_dim_color_b;
extern int conf_msgbox_dim_color_outline_r;
extern int conf_msgbox_dim_color_outline_g;
extern int conf_msgbox_dim_color_outline_b;
extern int conf_msgbox_seen_color;
extern int conf_msgbox_seen_color_r;
extern int conf_msgbox_seen_color_g;
extern int conf_msgbox_seen_color_b;
extern int conf_msgbox_seen_outline_color_r;
extern int conf_msgbox_seen_outline_color_g;
extern int conf_msgbox_seen_outline_color_b;
extern int conf_msgbox_tategaki;
extern int conf_msgbox_nowait;
extern char *conf_msgbox_history_control;
extern int conf_msgbox_fill;
extern int conf_msgbox_fill_color_a;
extern int conf_msgbox_fill_color_r;
extern int conf_msgbox_fill_color_g;
extern int conf_msgbox_fill_color_b;

/*
 * クリックアニメーションの設定
 */
extern int conf_click_x;
extern int conf_click_y;
extern int conf_click_move;
extern char *conf_click_file[16];
extern float conf_click_interval;
extern int click_frames;
extern int conf_click_disable;

/*
 * スイッチの設定
 */
extern char *conf_switch_bg_file[10];
extern char *conf_switch_fg_file[10];
extern int conf_switch_font_select;
extern int conf_switch_font_size;
extern int conf_switch_font_outline;
extern int conf_switch_x[10];
extern int conf_switch_y[10];
extern int conf_switch_margin_y;
extern int conf_switch_text_margin_y;
extern int conf_switch_color_inactive;
extern int conf_switch_color_inactive_body_r;
extern int conf_switch_color_inactive_body_g;
extern int conf_switch_color_inactive_body_b;
extern int conf_switch_color_inactive_outline_r;
extern int conf_switch_color_inactive_outline_g;
extern int conf_switch_color_inactive_outline_b;
extern int conf_switch_color_active;
extern int conf_switch_color_active_body_r;
extern int conf_switch_color_active_body_g;
extern int conf_switch_color_active_body_b;
extern int conf_switch_color_active_outline_r;
extern int conf_switch_color_active_outline_g;
extern int conf_switch_color_active_outline_b;
extern float conf_switch_timed;
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
 * セーブ・ロード画面の設定
 */
extern int conf_save_data_thumb_width;
extern int conf_save_data_thumb_height;
extern char *conf_save_data_new;

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
extern int conf_sysmenu_custom1_x;
extern int conf_sysmenu_custom1_y;
extern int conf_sysmenu_custom1_width;
extern int conf_sysmenu_custom1_height;
extern char *conf_sysmenu_custom1_gosub;
extern int conf_sysmenu_custom2_x;
extern int conf_sysmenu_custom2_y;
extern int conf_sysmenu_custom2_width;
extern int conf_sysmenu_custom2_height;
extern char *conf_sysmenu_custom2_gosub;
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
extern char *conf_sysmenu_custom1_se;
extern char *conf_sysmenu_custom2_se;
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
 * GUIの設定
 */
extern int conf_gui_ruby;
extern int conf_gui_save_font_select;
extern int conf_gui_save_font_size;
extern int conf_gui_save_font_outline;
extern int conf_gui_save_font_ruby_size;
extern int conf_gui_save_tategaki;
extern int conf_gui_save_clear_color_r;
extern int conf_gui_save_clear_color_g;
extern int conf_gui_save_clear_color_b;
extern int conf_gui_history_font_select;
extern int conf_gui_history_font_size;
extern int conf_gui_history_font_outline;
extern int conf_gui_history_font_ruby_size;
extern int conf_gui_history_margin_line;
extern int conf_gui_history_tategaki;
extern int conf_gui_history_disable_color;
extern int conf_gui_history_font_color_r;
extern int conf_gui_history_font_color_g;
extern int conf_gui_history_font_color_b;
extern int conf_gui_history_font_outline_color_r;
extern int conf_gui_history_font_outline_color_g;
extern int conf_gui_history_font_outline_color_b;
extern int conf_gui_history_clear_color_r;
extern int conf_gui_history_clear_color_g;
extern int conf_gui_history_clear_color_b;
extern int conf_gui_history_oneline;
extern char *conf_gui_history_quote_prefix;
extern char *conf_gui_history_quote_start;
extern char *conf_gui_history_quote_end;
extern int conf_gui_history_ignore_last;
extern int conf_gui_preview_tategaki;

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
 * キャラクタの名前とファイル名のマッピング
 */

#define CHARACTER_MAP_COUNT	(32)

extern int conf_character_focus;
extern char *conf_character_name[CHARACTER_MAP_COUNT];
extern char *conf_character_file[CHARACTER_MAP_COUNT];

/*
 * 目パチの設定
 */

extern float conf_character_eyeblink_interval;
extern float conf_character_eyeblink_frame;

/*
 * ステージのマージン(キャラクタレイヤの位置補正)
 */

extern int conf_stage_ch_margin_bottom;
extern int conf_stage_ch_margin_left;
extern int conf_stage_ch_margin_right;

/*
 * カーソル
 */

extern char *conf_cursor;

/*
 * キラキラエフェクトの設定
 */

#define KIRAKIRA_FRAME_COUNT	(16)

extern int conf_kirakira_on;
extern float conf_kirakira_frame;
extern char *conf_kirakira_file[KIRAKIRA_FRAME_COUNT];

/*
 * 変数名
 */

#define NAMED_VAR_COUNT		(100)

extern char *conf_var_name[VAR_SIZE];

/*
 * エモーティコン
 */

#define EMOTICON_COUNT		(16)

extern char *conf_emoticon_name[EMOTICON_COUNT];
extern char *conf_emoticon_file[EMOTICON_COUNT];

/*
 * config.txtには公開されないコンフィグ
 */

/* 最後にセーブ/ロードしたページ */
extern int conf_gui_save_last_page;

/*
 * アクセシビリティ
 */
extern int conf_tts_enable;
extern int conf_tts_user;

/*
 * その他の設定
 */
extern int conf_voice_stop_off;
extern int conf_window_fullscreen_disable;
extern int conf_window_maximize_disable;
extern char *conf_window_title_separator;
extern int conf_window_title_chapter_disable;
extern int conf_msgbox_show_on_ch;
extern int conf_msgbox_show_on_bg;
extern int conf_msgbox_show_on_choose;
extern float conf_beep_adjustment;
extern int conf_serif_quote;
extern int conf_sysmenu_transition;
extern int conf_msgbox_history_disable;
extern int conf_serif_color_name_only;
extern int conf_release;
extern char *conf_sav_name;

/* conf_localeを設定する */
void init_locale_code(void);

/* コンフィグの初期化処理を行う */
bool init_conf(void);

/* コンフィグの終了処理を行う */
void cleanup_conf(void);

/* コンフィグの値を元に各種設定を初期値にする */
bool apply_initial_values(void);

/* コンフィグを書き換える */
bool overwrite_config(const char *key, const char *val);

/* セーブデータに書き出すコンフィグの値を取得する */
const char *get_config_key_for_save_data(int index);

/* コンフィグをグローバルセーブデータにするかを取得する */
bool is_config_key_global(const char *key);

/* コンフィグの型を取得する('s', 'i', 'f') */
char get_config_type_for_key(const char *key);

/* 文字列型のコンフィグ値を取得する */
const char *get_string_config_value_for_key(const char *key);

/* 整数型のコンフィグ値を取得する */
int get_int_config_value_for_key(const char *key);

/* 浮動小数点数型のコンフィグ値を取得する */
float get_float_config_value_for_key(const char *key);

#endif
