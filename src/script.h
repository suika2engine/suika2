/* -*- coding: utf-8; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Suika2
 * Copyright (C) 2001-2023, Keiichi Tabata. All rights reserved.
 */

/*
 * Scenario Script Model
 *
 * [Changes]
 *  - 2016-06-01 Created
 *  - 2017-08-14 Added @switch
 *  - 2018-07-21 Added @gosub
 *  - 2019-09-17 Added @news
 *  - 2021-06-05 Added effects to @bg
 *  - 2021-06-05 Added system wide volumes to @vol
 *  - 2021-06-05 Added @menu button count
 *  - 2021-06-06 Added voice output to @se
 *  - 2021-06-10 Added the mask effect to @bg and @ch
 *  - 2021-06-10 Added effect, offset and alpha parameters to @ch
 *  - 2021-06-10 Added @cha
 *  - 2021-06-12 Added @shake
 *  - 2021-06-15 Added @setsave
 *  - 2021-07-07 Added @goto $SAVE
 *  - 2021-07-19 Added @chs
 *  - 2022-05-11 Added @video
 *  - 2022-06-05 Added @skip
 *  - 2022-06-06 Added the debugger support
 *  - 2022-06-17 Added @choose
 *  - 2022-07-04 Added @chapter
 *  - 2022-07-29 Added @gui
 *  - 2022-10-19 Added locales
 *  - 2023-01-06 Removed unused parameter indices
 *  - 2023-01-06 Added Japanese command names, parameter names and quotations
 *  - 2023-01-14 Added startup file and line (later removed)
 *  - 2023-08-14 Added using
 *  - 2022-08-14 Added @ichoose
 *  - 2023-08-20 Added @anime
 *  - 2023-08-27 Added structured syntax (SMODE)
 *  - 2023-08-27 Added @setconfig
 *  - 2023-08-31 Added @chsx
 *  - 2023-09-14 Added @pencil
 *  - 2023-10-21 Supported dynamic update of script model
 */

#ifndef SUIKA_SCRIPT_H
#define SUIKA_SCRIPT_H

#include "types.h"

/* Maximum command number. */
#define SCRIPT_CMD_SIZE	(65536)

/* コマンド構造体 */
struct command;

/* コマンドの種類 */
enum command_type {
	COMMAND_INVALID = 0,	/* invalid value */
	COMMAND_NULL,		/* NULL command (for reparse internal) */
	COMMAND_LABEL,
	COMMAND_SERIF,
	COMMAND_MESSAGE,
	COMMAND_BG,
	COMMAND_BGM,
	COMMAND_CH,
	COMMAND_CLICK,
	COMMAND_WAIT,
	COMMAND_GOTO,
	COMMAND_LOAD,
	COMMAND_VOL,
	COMMAND_SET,
	COMMAND_IF,
	COMMAND_SELECT,
	COMMAND_SE,
	COMMAND_MENU,
	COMMAND_RETROSPECT,
	COMMAND_SWITCH,
	COMMAND_GOSUB,
	COMMAND_RETURN,
	COMMAND_NEWS,
	COMMAND_CHA,
	COMMAND_SHAKE,
	COMMAND_SETSAVE,
	COMMAND_CHS,
	COMMAND_VIDEO,
	COMMAND_SKIP,
	COMMAND_CHOOSE,
	COMMAND_CHAPTER,
	COMMAND_GUI,
	COMMAND_WMS,
	COMMAND_ICHOOSE,
	COMMAND_ANIME,
	COMMAND_UNLESS,
	COMMAND_LABELEDGOTO,
	COMMAND_SETCONFIG,
	COMMAND_CHSX,
	COMMAND_PENCIL,
	COMMAND_LAYER,
	COMMAND_MAX		/* invalid value */
};

/* labelコマンドのパラメータ */
enum label_command_param {
	LABEL_PARAM_LABEL = 0,
};

/* メッセージコマンドのパラメータ */
enum message_command_param {
	MESSAGE_PARAM_MESSAGE,
};

/* セリフコマンドのパラメータ */
enum serif_command_param {
	SERIF_PARAM_NAME = 1,
	SERIF_PARAM_VOICE,
	SERIF_PARAM_MESSAGE,
};

/* bgコマンドのパラメータ */
enum bg_command_param {
	BG_PARAM_FILE = 1,
	BG_PARAM_SPAN,
	BG_PARAM_METHOD,
	BG_PARAM_X,
	BG_PARAM_Y,
};

/* bgmコマンドのパラメータ */
enum bgm_command_param {
	BGM_PARAM_FILE = 1,
	BGM_PARAM_ONCE,
};

/* chコマンドのパラメータ */
enum ch_command_param {
	CH_PARAM_POS = 1,
	CH_PARAM_FILE,
	CH_PARAM_SPAN,
	CH_PARAM_METHOD,
	CH_PARAM_OFFSET_X,
	CH_PARAM_OFFSET_Y,
	CH_PARAM_ALPHA,
};

/* waitコマンドのパラメータ */
enum wait_command_param {
	WAIT_PARAM_SPAN = 1,
	WAIT_PARAM_OPT,
};

/* gotoコマンドのパラメータ */
enum goto_command_param {
	GOTO_PARAM_LABEL = 1,
};

/* loadコマンドのパラメータ */
enum load_command_param {
	LOAD_PARAM_FILE = 1,
	LOAD_PARAM_LABEL,
};

/* volコマンドのパラメータ */
enum vol_command_param {
	VOL_PARAM_STREAM = 1,
	VOL_PARAM_VOL,
	VOL_PARAM_SPAN,
};

/* setコマンドのパラメータ */
enum set_command_param {
	SET_PARAM_LHS = 1,
	SET_PARAM_OP,
	SET_PARAM_RHS,
};

/* ifコマンドのパラメータ */
enum if_command_param {
	IF_PARAM_LHS = 1,
	IF_PARAM_OP,
	IF_PARAM_RHS,
	IF_PARAM_LABEL,
};

/* selectコマンドのパラメータ */
enum select_command_param {
	SELECT_PARAM_LABEL1 = 1,
	SELECT_PARAM_LABEL2,
	SELECT_PARAM_LABEL3,
	SELECT_PARAM_TEXT1,
	SELECT_PARAM_TEXT2,
	SELECT_PARAM_TEXT3,
};

/* seコマンドのパラメータ */
enum se_command_param {
	SE_PARAM_FILE = 1,
	SE_PARAM_OPTION,
};

/* switchコマンドのパラメータ */
enum switch_command_param {
	SWITCH_PARAM_PARENT_M1 = 1,
	SWITCH_PARAM_PARENT_M2,
	SWITCH_PARAM_PARENT_M3,
	SWITCH_PARAM_PARENT_M4,
	SWITCH_PARAM_PARENT_M5,
	SWITCH_PARAM_PARENT_M6,
	SWITCH_PARAM_PARENT_M7,
	SWITCH_PARAM_PARENT_M8,
	SWITCH_PARAM_CHILD1_L1,
	SWITCH_PARAM_CHILD1_M1,
	SWITCH_PARAM_CHILD1_L2,
	SWITCH_PARAM_CHILD1_M2,
	SWITCH_PARAM_CHILD1_L3,
	SWITCH_PARAM_CHILD1_M3,
	SWITCH_PARAM_CHILD1_L4,
	SWITCH_PARAM_CHILD1_M4,
	SWITCH_PARAM_CHILD1_L5,
	SWITCH_PARAM_CHILD1_M5,
	SWITCH_PARAM_CHILD1_L6,
	SWITCH_PARAM_CHILD1_M6,
	SWITCH_PARAM_CHILD1_L7,
	SWITCH_PARAM_CHILD1_M7,
	SWITCH_PARAM_CHILD1_L8,
	SWITCH_PARAM_CHILD1_M8,
	SWITCH_PARAM_CHILD2_L1,
	SWITCH_PARAM_CHILD2_M1,
	SWITCH_PARAM_CHILD2_L2,
	SWITCH_PARAM_CHILD2_M2,
	SWITCH_PARAM_CHILD2_L3,
	SWITCH_PARAM_CHILD2_M3,
	SWITCH_PARAM_CHILD2_L4,
	SWITCH_PARAM_CHILD2_M4,
	SWITCH_PARAM_CHILD2_L5,
	SWITCH_PARAM_CHILD2_M5,
	SWITCH_PARAM_CHILD2_L6,
	SWITCH_PARAM_CHILD2_M6,
	SWITCH_PARAM_CHILD2_L7,
	SWITCH_PARAM_CHILD2_M7,
	SWITCH_PARAM_CHILD2_L8,
	SWITCH_PARAM_CHILD2_M8,
	SWITCH_PARAM_CHILD3_L1,
	SWITCH_PARAM_CHILD3_M1,
	SWITCH_PARAM_CHILD3_L2,
	SWITCH_PARAM_CHILD3_M2,
	SWITCH_PARAM_CHILD3_L3,
	SWITCH_PARAM_CHILD3_M3,
	SWITCH_PARAM_CHILD3_L4,
	SWITCH_PARAM_CHILD3_M4,
	SWITCH_PARAM_CHILD3_L5,
	SWITCH_PARAM_CHILD3_M5,
	SWITCH_PARAM_CHILD3_L6,
	SWITCH_PARAM_CHILD3_M6,
	SWITCH_PARAM_CHILD3_L7,
	SWITCH_PARAM_CHILD3_M7,
	SWITCH_PARAM_CHILD3_L8,
	SWITCH_PARAM_CHILD3_M8,
	SWITCH_PARAM_CHILD4_L1,
	SWITCH_PARAM_CHILD4_M1,
	SWITCH_PARAM_CHILD4_L2,
	SWITCH_PARAM_CHILD4_M2,
	SWITCH_PARAM_CHILD4_L3,
	SWITCH_PARAM_CHILD4_M3,
	SWITCH_PARAM_CHILD4_L4,
	SWITCH_PARAM_CHILD4_M4,
	SWITCH_PARAM_CHILD4_L5,
	SWITCH_PARAM_CHILD4_M5,
	SWITCH_PARAM_CHILD4_L6,
	SWITCH_PARAM_CHILD4_M6,
	SWITCH_PARAM_CHILD4_L7,
	SWITCH_PARAM_CHILD4_M7,
	SWITCH_PARAM_CHILD4_L8,
	SWITCH_PARAM_CHILD4_M8,
	SWITCH_PARAM_CHILD5_L1,
	SWITCH_PARAM_CHILD5_M1,
	SWITCH_PARAM_CHILD5_L2,
	SWITCH_PARAM_CHILD5_M2,
	SWITCH_PARAM_CHILD5_L3,
	SWITCH_PARAM_CHILD5_M3,
	SWITCH_PARAM_CHILD5_L4,
	SWITCH_PARAM_CHILD5_M4,
	SWITCH_PARAM_CHILD5_L5,
	SWITCH_PARAM_CHILD5_M5,
	SWITCH_PARAM_CHILD5_L6,
	SWITCH_PARAM_CHILD5_M6,
	SWITCH_PARAM_CHILD5_L7,
	SWITCH_PARAM_CHILD5_M7,
	SWITCH_PARAM_CHILD5_L8,
	SWITCH_PARAM_CHILD5_M8,
	SWITCH_PARAM_CHILD6_L1,
	SWITCH_PARAM_CHILD6_M1,
	SWITCH_PARAM_CHILD6_L2,
	SWITCH_PARAM_CHILD6_M2,
	SWITCH_PARAM_CHILD6_L3,
	SWITCH_PARAM_CHILD6_M3,
	SWITCH_PARAM_CHILD6_L4,
	SWITCH_PARAM_CHILD6_M4,
	SWITCH_PARAM_CHILD6_L5,
	SWITCH_PARAM_CHILD6_M5,
	SWITCH_PARAM_CHILD6_L6,
	SWITCH_PARAM_CHILD6_M6,
	SWITCH_PARAM_CHILD6_L7,
	SWITCH_PARAM_CHILD6_M7,
	SWITCH_PARAM_CHILD6_L8,
	SWITCH_PARAM_CHILD6_M8,
	SWITCH_PARAM_CHILD7_L1,
	SWITCH_PARAM_CHILD7_M1,
	SWITCH_PARAM_CHILD7_L2,
	SWITCH_PARAM_CHILD7_M2,
	SWITCH_PARAM_CHILD7_L3,
	SWITCH_PARAM_CHILD7_M3,
	SWITCH_PARAM_CHILD7_L4,
	SWITCH_PARAM_CHILD7_M4,
	SWITCH_PARAM_CHILD7_L5,
	SWITCH_PARAM_CHILD7_M5,
	SWITCH_PARAM_CHILD7_L6,
	SWITCH_PARAM_CHILD7_M6,
	SWITCH_PARAM_CHILD7_L7,
	SWITCH_PARAM_CHILD7_M7,
	SWITCH_PARAM_CHILD7_L8,
	SWITCH_PARAM_CHILD7_M8,
	SWITCH_PARAM_CHILD8_L1,
	SWITCH_PARAM_CHILD8_M1,
	SWITCH_PARAM_CHILD8_L2,
	SWITCH_PARAM_CHILD8_M2,
	SWITCH_PARAM_CHILD8_L3,
	SWITCH_PARAM_CHILD8_M3,
	SWITCH_PARAM_CHILD8_L4,
	SWITCH_PARAM_CHILD8_M4,
	SWITCH_PARAM_CHILD8_L5,
	SWITCH_PARAM_CHILD8_M5,
	SWITCH_PARAM_CHILD8_L6,
	SWITCH_PARAM_CHILD8_M6,
	SWITCH_PARAM_CHILD8_L7,
	SWITCH_PARAM_CHILD8_M7,
	SWITCH_PARAM_CHILD8_L8,
	SWITCH_PARAM_CHILD8_M8,
};

/* chooseコマンドのパラメータ */
enum choose_command_param {
	CHOOSE_PARAM_LABEL1 = 1,
	CHOOSE_PARAM_TEXT1,
	CHOOSE_PARAM_LABEL2,
	CHOOSE_PARAM_TEXT2,
	CHOOSE_PARAM_LABEL3,
	CHOOSE_PARAM_TEXT3,
	CHOOSE_PARAM_LABEL4,
	CHOOSE_PARAM_TEXT4,
	CHOOSE_PARAM_LABEL5,
	CHOOSE_PARAM_TEXT5,
	CHOOSE_PARAM_LABEL6,
	CHOOSE_PARAM_TEXT6,
	CHOOSE_PARAM_LABEL7,
	CHOOSE_PARAM_TEXT7,
	CHOOSE_PARAM_LABEL8,
	CHOOSE_PARAM_TEXT8,
};

/* gosubコマンドのパラメータ */
enum gosub_command_param {
	GOSUB_PARAM_LABEL = 1,
};

/* chaコマンドのパラメータ */
enum cha_command_param {
	CHA_PARAM_POS = 1,
	CHA_PARAM_SPAN,
	CHA_PARAM_ACCEL,
	CHA_PARAM_OFFSET_X,
	CHA_PARAM_OFFSET_Y,
	CHA_PARAM_ALPHA,
};

/* shakeコマンドのパラメータ */
enum shake_command_param {
	SHAKE_PARAM_MOVE = 1,
	SHAKE_PARAM_SPAN,
	SHAKE_PARAM_TIMES,
	SHAKE_PARAM_AMOUNT,
};

/* setsaveコマンドのパラメータ */
enum setsave_command_param {
	SETSAVE_PARAM_MODE = 1,
};

/* chsコマンドのパラメータ */
enum chs_command_param {
	CHS_PARAM_CENTER = 1,
	CHS_PARAM_RIGHT,
	CHS_PARAM_LEFT,
	CHS_PARAM_BACK,
	CHS_PARAM_SPAN,
	CHS_PARAM_BG,
	CHS_PARAM_METHOD,
};

/* videoコマンドのパラメータ */
enum video_command_param {
	VIDEO_PARAM_FILE = 1,
	VIDEO_PARAM_OPTIONS,
};

/* skipコマンドのパラメータ */
enum skip_command_param {
	SKIP_PARAM_MODE = 1,
};

/* chapterコマンドのパラメータ */
enum chapter_command_param {
	CHAPTER_PARAM_NAME = 1,
};

/* guiコマンドのパラメータ */
enum gui_command_param {
	GUI_PARAM_FILE = 1,
	GUI_PARAM_OPTIONS,
};

/* wmsコマンドのパラメータ */
enum wms_command_param {
	WMS_PARAM_FILE = 1,
};

/* animeコマンドのパラメータ */
enum anime_command_param {
	ANIME_PARAM_FILE = 1,
	ANIME_PARAM_SPEC,
};

/* unlessコマンドのパラメータ */
enum unless_command_param {
	UNLESS_PARAM_LHS = 1,
	UNLESS_PARAM_OP,
	UNLESS_PARAM_RHS,
	UNLESS_PARAM_LABEL,
	UNLESS_PARAM_FINALLY,
};

/* labeledgotoコマンドのパラメータ */
enum labeledgoto_command_param {
	LABELEDGOTO_PARAM_LABEL = 1,
	LABELEDGOTO_PARAM_GOTO,
};

/* setconfigコマンドのパラメータ */
enum setconfig_command_param {
	SETCONFIG_PARAM_KEY = 1,
	SETCONFIG_PARAM_VALUE,
};

/* chsxコマンドのパラメータ */
enum chsx_command_param {
	CHSX_PARAM_CENTER = 1,
	CHSX_PARAM_CX,
	CHSX_PARAM_CY,
	CHSX_PARAM_CA,
	CHSX_PARAM_CD,
	CHSX_PARAM_RIGHT,
	CHSX_PARAM_RX,
	CHSX_PARAM_RY,
	CHSX_PARAM_RA,
	CHSX_PARAM_RD,
	CHSX_PARAM_RIGHT_CENTER,
	CHSX_PARAM_RCX,
	CHSX_PARAM_RCY,
	CHSX_PARAM_RCA,
	CHSX_PARAM_RCD,
	CHSX_PARAM_LEFT,
	CHSX_PARAM_LX,
	CHSX_PARAM_LY,
	CHSX_PARAM_LA,
	CHSX_PARAM_LD,
	CHSX_PARAM_LEFT_CENTER,
	CHSX_PARAM_LCX,
	CHSX_PARAM_LCY,
	CHSX_PARAM_LCA,
	CHSX_PARAM_LCD,
	CHSX_PARAM_BACK,
	CHSX_PARAM_BX,
	CHSX_PARAM_BY,
	CHSX_PARAM_BA,
	CHSX_PARAM_BD,
	CHSX_PARAM_BG,
	CHSX_PARAM_BGX,
	CHSX_PARAM_BGY,
	CHSX_PARAM_SPAN,
	CHSX_PARAM_METHOD,
};

/* pencilコマンドのパラメータ */
enum pencil_command_param {
	PENCIL_PARAM_LAYER = 1,
	PENCIL_PARAM_TEXT,
};

/* layerコマンドのパラメータ */
enum layer_command_param {
	LAYER_PARAM_NAME = 1,
	LAYER_PARAM_FILE,
	LAYER_PARAM_X,
	LAYER_PARAM_Y,
	LAYER_PARAM_A,
};

/*
 * 初期化
 */

/* 初期スクリプトを読み込む */
bool init_script(void);

/* コマンドを破棄する */
void cleanup_script(void);

/*
 * スクリプトとコマンドへの公開アクセス
 */

/* スクリプトをロードする */
bool load_script(const char *fname);

/* スクリプトファイル名を取得する */
const char *get_script_file_name(void);

/* 実行中のコマンドのインデックスを取得する(セーブ用) */
int get_command_index(void);

/* 実行中のコマンドのインデックスを設定する(gosub,ロード用) */
bool move_to_command_index(int index);

/* 次のコマンドに移動する */
bool move_to_next_command(void);

/* ラベルへ移動する */
bool move_to_label(const char *label);

/* ラベルへ移動する(なければfinallyラベルにジャンプする) */
bool move_to_label_finally(const char *label, const char *finally_label);

/* gosubによるリターンポイントを記録する(gosub用) */
void push_return_point(void);

/* gosubによるリターンポイントを記録する(カスタムSYSMENUのgosub用) */
void push_return_point_minus_one(void);

/* gosubによるリターンポイントを取得する(return用) */
int pop_return_point(void);

/* gosubによるリターンポイントの行番号を設定する(ロード用) */
bool set_return_point(int index);

/* gosubによるリターンポイントの行番号を取得する(セーブ用) */
int get_return_point(void);

/* 最後のコマンドであるかを取得する(@goto $SAVE用) */
bool is_final_command(void);

/* コマンドの行番号を取得する(ログ用) */
int get_line_num(void);

/* コマンドの行全体を取得する(ログ用) */
const char *get_line_string(void);

/* コマンドのタイプを取得する */
int get_command_type(void);

/* コマンドのロケール指定を取得する */
const char *get_command_locale(void);

/* 文字列のコマンドパラメータを取得する */
const char *get_string_param(int index);

/* 整数のコマンドパラメータを取得する */
int get_int_param(int index);

/* 浮動小数点数のコマンドパラメータを取得する */
float get_float_param(int index);

/* 行の数を取得する */
int get_line_count(void);

/* コマンドの数を取得する */
int get_command_count(void);

/*
 * For Suika2 Pro
 */
#if defined(USE_EDITOR) || defined(USE_DEBUGGER)

/* using展開後のコマンドの行番号を取得する(ログ用) */
int get_expanded_line_num(void);

/* 指定した行番号以降の最初のコマンドインデックスを取得する */
int get_command_index_from_line_num(int line);

/* 指定した行番号の行全体を取得する */
const char *get_line_string_at_line_num(int line);

/* エラー時に@コマンドを'!'で始まるメッセージに変換する */
void translate_command_to_message_for_runtime_error(int index);

/* デバッグ用の仮のスクリプトをロードする */
bool load_debug_script(void);

/* スクリプトの行を挿入する */
bool insert_script_line(int line, const char *text);

/* スクリプトの行を更新する */
bool update_script_line(int line, const char *text);

/* スクリプトの行を削除する */
bool delete_script_line(int line);

/* スクリプトモデルを元にスクリプトファイルを保存する */
bool save_script(void);

/* スクリプトの拡張構文を再度パースする */
bool reparse_script_for_structured_syntax(void);

/* コマンド名からコマンドタイプを返す */
int get_command_type_from_name(const char *name);

#endif /* defined(USE_EDITOR) || defined(USE_DEBUGGER) */

#endif
