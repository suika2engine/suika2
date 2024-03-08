/* -*- Coding: utf-8; tab-width: 8; indent-tabs-mode: t; -*- */

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

#include "suika.h"

#ifdef _MSC_VER
#define strcasecmp _stricmp
#endif

/* false assertion */
#define NEVER_COME_HERE	(0)

/* 1行の読み込みサイズ */
#define LINE_BUF_SIZE	(4096)

/* マクロインクルードのキーワード */
#define MACRO_INC	"using "

/*
 * コマンド配列
 */

/*
 * コマンドの引数の最大数
 *  - コマンド名も含める
 *  - TODO: @switchを廃止して、引数の最大値を大幅に小さくする
 */
#define PARAM_SIZE	(137)

/* コマンド配列 */
static struct command {
	/* ファイル名 */
	const char *file;

	/* 行番号(starts from 0) */
	int line;

	/* usingで拡張された行番号(starts from 0) */
	int expanded_line;

	/* コマンドタイプ */
	int type;

	/* 行の生テキスト */
	char *text;

	/* 引数 (@で始まるコマンドのとき、param[0]はコマンド名) */
	char *param[PARAM_SIZE];

	/* 実行条件のローケル指定子 */
	char locale[3];

} cmd[SCRIPT_CMD_SIZE];

/* 読み込み済みのコマンドの数 */
static int cmd_size;

/* コマンドの作成が1つ完成したときに呼ぶ */
#define COMMIT_CMD()	cmd_size++

/*
 * コマンド実行ポインタ
 */

/* 実行中のスクリプト名 */
static const char *cur_script;

/* 実行中のコマンド番号 */
static int cur_index;

/* 最後にgosubが実行されたコマンド番号 */
static int return_point;

/* 無効なreturn_pointの値 */
#define INVALID_RETURN_POINT	(-2)

/*
 * ファイル名
 */

#define FILE_NAME_TBL_ENTRIES	(32)

/* ファイル名一覧 */
static char *file_name_tbl[FILE_NAME_TBL_ENTRIES];

/* 使用済みのファイル名の数 */
static int used_file_names;

/*
 * パース中のファイル名と行番号
 */

/* パース中のファイル名 */
static const char *cur_parse_file;

/* パース中のファイルの行番号 */
static int cur_parse_line;

/* 入力行の処理が1つ完了したときに呼ぶ */
#define CONSUME_INPUT_LINE()	cur_parse_line++

/* usingで展開後の行番号 (ラベル作成に使用する) */
static int cur_expanded_line;

/* 行を1つ出力したときに呼ぶ */
#define INC_OUTPUT_LINE()	cur_expanded_line++

/*
 * For the main engine
 */
#if !defined(USE_EDITOR)
bool reparse_script_for_structured_syntax(void);
#endif

/*
 * For Suika2 Pro
 */
#ifdef USE_EDITOR

/* コメント行のテキスト */
#define SCRIPT_LINE_SIZE	(65536)

/* コメント行 */
static char *comment_text[SCRIPT_LINE_SIZE];

/* 前方参照 */
static bool replace_command_by_command(int index, const char *text);
static void replace_command_by_comment(int line, const char *text);
static bool replace_comment_by_command(int line, const char *text);
static void replace_comment_by_comment(int line, const char *text);
static void insert_comment(int line, const char *text);
static bool insert_command(int line, const char *text);

#endif /* USE_EDITOR */

/*
 * 命令の種類
 */

struct insn_item {
	const char *str;	/* 命令の文字列 */
	int type;		/* コマンドのタイプ */
	int min;		/* 最小のパラメータ数 */
	int max;		/* 最大のパラメータ数 */
} insn_tbl[] = {
	/* 背景変更 */
	{"@bg", COMMAND_BG, 1, 5},
	{U8("@背景"), COMMAND_BG, 1, 5},

	/* BGM再生 */
	{"@bgm", COMMAND_BGM, 1, 2},
	{U8("@音楽"), COMMAND_BGM, 1, 2},

	/* キャラ変更 */
	{"@ch", COMMAND_CH, 1, 7},
	{U8("@キャラ"), COMMAND_CH, 1, 7},

	/* クリック待ち */
	{"@click", COMMAND_CLICK, 0, 1},
	{U8("@クリック"), COMMAND_CLICK, 0, 1},

	/* 時間指定待ち */
	{"@wait", COMMAND_WAIT, 1, 2},
	{U8("@時間待ち"), COMMAND_WAIT, 1, 2},

	/* ラベルへジャンプ */
	{"@goto", COMMAND_GOTO, 1, 1},
	{U8("@ジャンプ"), COMMAND_GOTO, 1, 1},

	/* ラベルかつジャンプ */
	{"@labeledgoto", COMMAND_LABELEDGOTO, 2, 2},

	/* シナリオファイルのロード */
	{"@load", COMMAND_LOAD, 1, 2},
	{U8("@シナリオ"), COMMAND_LOAD, 1, 2},

	/* ボリューム設定 */
	{"@vol", COMMAND_VOL, 2, 3},
	{U8("@音量"), COMMAND_VOL, 2, 3},

	/* 変数設定 */
	{"@set", COMMAND_SET, 3, 3},
	{U8("@フラグをセット"), COMMAND_SET, 3, 3},

	/* 変数分岐 */
	{"@if", COMMAND_IF, 4, 4},
	{U8("@フラグでジャンプ"), COMMAND_IF, 4, 4},

	/* 変数分岐 */
	{"@unless", COMMAND_UNLESS, 4, 5},

	/* 効果音 */
	{"@se", COMMAND_SE, 1, 2},
	{U8("@効果音"), COMMAND_SE, 1, 2},

	/* キャラアニメ */
	{"@cha", COMMAND_CHA, 6, 6},
	{U8("@キャラ移動"), COMMAND_CHA, 6, 6},

	/* 画面を揺らす */
	{"@shake", COMMAND_SHAKE, 4, 4},
	{U8("@振動"), COMMAND_SHAKE, 4, 4},

	/* ステージの一括変更 */
	{"@chs", COMMAND_CHS, 4, 7},
	{U8("@場面転換"), COMMAND_CHS, 4, 7},

	/* ステージの一括変更(オフセット付き) */
	{"@chsx", COMMAND_CHSX, 0, 29},
	{U8("@場面転換X"), COMMAND_CHSX, 0, 21},

	/* ビデオ再生 */
	{"@video", COMMAND_VIDEO, 1, 2},
	{U8("@動画"), COMMAND_VIDEO, 1, 2},

	/* 選択肢 */
	{"@choose", COMMAND_CHOOSE, 2, 16},
	{U8("@選択肢"), COMMAND_CHOOSE, 2, 16},

	/* インライン選択肢 */
	{"@ichoose", COMMAND_ICHOOSE, 2, 16},
	{U8("@インライン選択肢"), COMMAND_ICHOOSE, 2, 16},

	/* 条件付き選択肢 */
	{"@mchoose", COMMAND_MCHOOSE, 3, 24},
	{U8("@条件付き選択肢"), COMMAND_MCHOOSE, 3, 24},

	/* 条件付きインライン選択肢 */
	{"@michoose", COMMAND_MICHOOSE, 3, 24},
	{U8("@条件付きインライン選択肢"), COMMAND_MICHOOSE, 3, 24},

	/* 章タイトル */
	{"@chapter", COMMAND_CHAPTER, 1, 1},
	{U8("@章"), COMMAND_CHAPTER, 1, 1},

	/* GUI */
	{"@gui", COMMAND_GUI, 1, 2},
	{U8("@メニュー"), COMMAND_GUI, 1, 2},

	/* WMS */
	{"@wms", COMMAND_WMS, 1, 1},
	{U8("@スクリプト"), COMMAND_WMS, 1, 1},

	/* スキップ設定 */
	{"@skip", COMMAND_SKIP, 1, 1},
	{U8("@スキップ"), COMMAND_SKIP, 1, 1},

	/* アニメ */
	{"@anime", COMMAND_ANIME, 1, 2},
	{U8("@アニメ"), COMMAND_ANIME, 1, 2},
	{"@layer", COMMAND_LAYER, 2, 5},
	{U8("@レイヤ"), COMMAND_LAYER, 2, 5},

	/* マクロ */
	{"@gosub", COMMAND_GOSUB, 1, 1},
	{"@return", COMMAND_RETURN, 0, 0},

	/* コンフィグ変更 */
	{"@setconfig", COMMAND_SETCONFIG, 1, 2},
	{U8("@設定"), COMMAND_SETCONFIG, 1, 2},

	/* テキスト描画 */
	{"@pencil", COMMAND_PENCIL, 1, 2},
	{U8("@鉛筆"), COMMAND_PENCIL, 1, 2},

	/* その他 */
	{"@setsave", COMMAND_SETSAVE, 1, 1},

	/* deprecated */
	{"@news", COMMAND_NEWS, 9, 136},
	{"@switch", COMMAND_SWITCH, 9, 136},
};

#define INSN_TBL_SIZE	(sizeof(insn_tbl) / sizeof(struct insn_item))

/*
 * パラメータの名前
 */

struct param_item {
	int type;
	int param_index;
	const char *name;
} param_tbl[] = {
	/* @bg */
	{COMMAND_BG, BG_PARAM_FILE, "file="},
	{COMMAND_BG, BG_PARAM_FILE, U8("ファイル=")},
	{COMMAND_BG, BG_PARAM_SPAN, "duration="},
	{COMMAND_BG, BG_PARAM_SPAN, U8("秒=")},
	{COMMAND_BG, BG_PARAM_METHOD, "effect="},
	{COMMAND_BG, BG_PARAM_METHOD, U8("エフェクト=")},
	{COMMAND_BG, BG_PARAM_X, "x="},
	{COMMAND_BG, BG_PARAM_Y, "y="},

	/* @bgm */
	{COMMAND_BGM, BG_PARAM_FILE, "file="},
	{COMMAND_BGM, BG_PARAM_FILE, U8("ファイル=")},

	/* @ch */
	{COMMAND_CH, CH_PARAM_POS, "position="},
	{COMMAND_CH, CH_PARAM_POS, U8("位置=")},
	{COMMAND_CH, CH_PARAM_FILE, "file="},
	{COMMAND_CH, CH_PARAM_FILE, U8("ファイル=")},
	{COMMAND_CH, CH_PARAM_SPAN, "duration="},
	{COMMAND_CH, CH_PARAM_SPAN, U8("秒=")},
	{COMMAND_CH, CH_PARAM_METHOD, "effect="},
	{COMMAND_CH, CH_PARAM_METHOD, U8("エフェクト=")},
	{COMMAND_CH, CH_PARAM_OFFSET_X, "right="},
	{COMMAND_CH, CH_PARAM_OFFSET_X, U8("右=")},
	{COMMAND_CH, CH_PARAM_OFFSET_Y, "down="},
	{COMMAND_CH, CH_PARAM_OFFSET_Y, U8("下=")},
	{COMMAND_CH, CH_PARAM_ALPHA, "alpha="},
	{COMMAND_CH, CH_PARAM_ALPHA, U8("アルファ=")},

	/* @wait */
	{COMMAND_WAIT, WAIT_PARAM_SPAN, "duration="},
	{COMMAND_WAIT, WAIT_PARAM_SPAN, U8("秒=")},

	/* @goto */
	{COMMAND_GOTO, GOTO_PARAM_LABEL, "destination="},
	{COMMAND_GOTO, GOTO_PARAM_LABEL, U8("行き先=")},

	/* @load */
	{COMMAND_LOAD, LOAD_PARAM_FILE, "file="},
	{COMMAND_LOAD, LOAD_PARAM_FILE, U8("ファイル=")},
	{COMMAND_LOAD, LOAD_PARAM_FILE, "label="},
	{COMMAND_LOAD, LOAD_PARAM_FILE, U8("ラベル=")},

	/* @vol */
	{COMMAND_VOL, VOL_PARAM_STREAM, "track="},
	{COMMAND_VOL, VOL_PARAM_STREAM, U8("トラック=")},
	{COMMAND_VOL, VOL_PARAM_VOL, "volume="},
	{COMMAND_VOL, VOL_PARAM_VOL, U8("音量=")},
	{COMMAND_VOL, VOL_PARAM_SPAN, "duration="},
	{COMMAND_VOL, VOL_PARAM_SPAN, U8("秒=")},

	/* @se */
	{COMMAND_SE, SE_PARAM_FILE, "file="},
	{COMMAND_SE, SE_PARAM_FILE, U8("ファイル=")},

	/* @choose */
	{COMMAND_CHOOSE, CHOOSE_PARAM_LABEL1, "destination1="},
	{COMMAND_CHOOSE, CHOOSE_PARAM_LABEL1, U8("行き先1=")},
	{COMMAND_CHOOSE, CHOOSE_PARAM_TEXT1, "option1="},
	{COMMAND_CHOOSE, CHOOSE_PARAM_TEXT1, U8("選択肢1=")},
	{COMMAND_CHOOSE, CHOOSE_PARAM_LABEL2, "destination2="},
	{COMMAND_CHOOSE, CHOOSE_PARAM_LABEL2, U8("行き先2=")},
	{COMMAND_CHOOSE, CHOOSE_PARAM_TEXT2, "option2="},
	{COMMAND_CHOOSE, CHOOSE_PARAM_TEXT2, U8("選択肢2=")},
	{COMMAND_CHOOSE, CHOOSE_PARAM_LABEL3, "destination3="},
	{COMMAND_CHOOSE, CHOOSE_PARAM_LABEL3, U8("行き先3=")},
	{COMMAND_CHOOSE, CHOOSE_PARAM_TEXT3, "option3="},
	{COMMAND_CHOOSE, CHOOSE_PARAM_TEXT3, U8("選択肢3=")},
	{COMMAND_CHOOSE, CHOOSE_PARAM_LABEL4, "destination4="},
	{COMMAND_CHOOSE, CHOOSE_PARAM_LABEL4, U8("行き先4=")},
	{COMMAND_CHOOSE, CHOOSE_PARAM_TEXT4, "option4="},
	{COMMAND_CHOOSE, CHOOSE_PARAM_TEXT4, U8("選択肢4=")},
	{COMMAND_CHOOSE, CHOOSE_PARAM_LABEL5, "destination5="},
	{COMMAND_CHOOSE, CHOOSE_PARAM_LABEL5, U8("行き先5=")},
	{COMMAND_CHOOSE, CHOOSE_PARAM_TEXT5, "option5="},
	{COMMAND_CHOOSE, CHOOSE_PARAM_TEXT5, U8("選択肢5=")},
	{COMMAND_CHOOSE, CHOOSE_PARAM_LABEL6, "destination6="},
	{COMMAND_CHOOSE, CHOOSE_PARAM_LABEL6, U8("行き先6=")},
	{COMMAND_CHOOSE, CHOOSE_PARAM_TEXT6, "option6="},
	{COMMAND_CHOOSE, CHOOSE_PARAM_TEXT6, U8("選択肢6=")},
	{COMMAND_CHOOSE, CHOOSE_PARAM_LABEL7, "destination7="},
	{COMMAND_CHOOSE, CHOOSE_PARAM_LABEL7, U8("行き先7=")},
	{COMMAND_CHOOSE, CHOOSE_PARAM_TEXT7, "option7="},
	{COMMAND_CHOOSE, CHOOSE_PARAM_TEXT7, U8("選択肢7=")},
	{COMMAND_CHOOSE, CHOOSE_PARAM_LABEL8, "destination8="},
	{COMMAND_CHOOSE, CHOOSE_PARAM_LABEL8, U8("行き先8=")},
	{COMMAND_CHOOSE, CHOOSE_PARAM_TEXT8, "option8="},
	{COMMAND_CHOOSE, CHOOSE_PARAM_TEXT8, U8("選択肢8=")},

	/* @ichoose */
	{COMMAND_ICHOOSE, CHOOSE_PARAM_LABEL1, "destination1="},
	{COMMAND_ICHOOSE, CHOOSE_PARAM_LABEL1, U8("行き先1=")},
	{COMMAND_ICHOOSE, CHOOSE_PARAM_TEXT1, "option1="},
	{COMMAND_ICHOOSE, CHOOSE_PARAM_TEXT1, U8("選択肢1=")},
	{COMMAND_ICHOOSE, CHOOSE_PARAM_LABEL2, "destination2="},
	{COMMAND_ICHOOSE, CHOOSE_PARAM_LABEL2, U8("行き先2=")},
	{COMMAND_ICHOOSE, CHOOSE_PARAM_TEXT2, "option2="},
	{COMMAND_ICHOOSE, CHOOSE_PARAM_TEXT2, U8("選択肢2=")},
	{COMMAND_ICHOOSE, CHOOSE_PARAM_LABEL3, "destination3="},
	{COMMAND_ICHOOSE, CHOOSE_PARAM_LABEL3, U8("行き先3=")},
	{COMMAND_ICHOOSE, CHOOSE_PARAM_TEXT3, "option3="},
	{COMMAND_ICHOOSE, CHOOSE_PARAM_TEXT3, U8("選択肢3=")},
	{COMMAND_ICHOOSE, CHOOSE_PARAM_LABEL4, "destination4="},
	{COMMAND_ICHOOSE, CHOOSE_PARAM_LABEL4, U8("行き先4=")},
	{COMMAND_ICHOOSE, CHOOSE_PARAM_TEXT4, "option4="},
	{COMMAND_ICHOOSE, CHOOSE_PARAM_TEXT4, U8("選択肢4=")},
	{COMMAND_ICHOOSE, CHOOSE_PARAM_LABEL5, "destination5="},
	{COMMAND_ICHOOSE, CHOOSE_PARAM_LABEL5, U8("行き先5=")},
	{COMMAND_ICHOOSE, CHOOSE_PARAM_TEXT5, "option5="},
	{COMMAND_ICHOOSE, CHOOSE_PARAM_TEXT5, U8("選択肢5=")},
	{COMMAND_ICHOOSE, CHOOSE_PARAM_LABEL6, "destination6="},
	{COMMAND_ICHOOSE, CHOOSE_PARAM_LABEL6, U8("行き先6=")},
	{COMMAND_ICHOOSE, CHOOSE_PARAM_TEXT6, "option6="},
	{COMMAND_ICHOOSE, CHOOSE_PARAM_TEXT6, U8("選択肢6=")},
	{COMMAND_ICHOOSE, CHOOSE_PARAM_LABEL7, "destination7="},
	{COMMAND_ICHOOSE, CHOOSE_PARAM_LABEL7, U8("行き先7=")},
	{COMMAND_ICHOOSE, CHOOSE_PARAM_TEXT7, "option7="},
	{COMMAND_ICHOOSE, CHOOSE_PARAM_TEXT7, U8("選択肢7=")},
	{COMMAND_ICHOOSE, CHOOSE_PARAM_LABEL8, "destination8="},
	{COMMAND_ICHOOSE, CHOOSE_PARAM_LABEL8, U8("行き先8=")},
	{COMMAND_ICHOOSE, CHOOSE_PARAM_TEXT8, "option8="},
	{COMMAND_ICHOOSE, CHOOSE_PARAM_TEXT8, U8("選択肢8=")},

	/* @cha */
	{COMMAND_CHA, CHA_PARAM_POS, "position="},
	{COMMAND_CHA, CHA_PARAM_POS, U8("位置=")},
	{COMMAND_CHA, CHA_PARAM_SPAN, "duration="},
	{COMMAND_CHA, CHA_PARAM_SPAN, U8("秒=")},
	{COMMAND_CHA, CHA_PARAM_ACCEL, "acceleration="},
	{COMMAND_CHA, CHA_PARAM_ACCEL, U8("加速=")},
	{COMMAND_CHA, CHA_PARAM_OFFSET_X, "x="},
	{COMMAND_CHA, CHA_PARAM_OFFSET_Y, "y="},
	{COMMAND_CHA, CHA_PARAM_ALPHA, "alpha"},
	{COMMAND_CHA, CHA_PARAM_ALPHA, U8("アルファ=")},

	/* @shake */
	{COMMAND_SHAKE, SHAKE_PARAM_MOVE, "direction="},
	{COMMAND_SHAKE, SHAKE_PARAM_MOVE, U8("方向=")},
	{COMMAND_SHAKE, SHAKE_PARAM_SPAN, "duration="},
	{COMMAND_SHAKE, SHAKE_PARAM_SPAN, U8("秒=")},
	{COMMAND_SHAKE, SHAKE_PARAM_TIMES, "times="},
	{COMMAND_SHAKE, SHAKE_PARAM_TIMES, U8("回数=")},
	{COMMAND_SHAKE, SHAKE_PARAM_AMOUNT, "amplitude="},
	{COMMAND_SHAKE, SHAKE_PARAM_AMOUNT, U8("大きさ=")},

	/* @chs */
	{COMMAND_CHS, CHS_PARAM_CENTER, "center="},
	{COMMAND_CHS, CHS_PARAM_CENTER, "centre="},
	{COMMAND_CHS, CHS_PARAM_CENTER, U8("中央=")},
	{COMMAND_CHS, CHS_PARAM_RIGHT, "right="},
	{COMMAND_CHS, CHS_PARAM_RIGHT, U8("右=")},
	{COMMAND_CHS, CHS_PARAM_LEFT, "left="},
	{COMMAND_CHS, CHS_PARAM_LEFT, U8("左=")},
	{COMMAND_CHS, CHS_PARAM_BACK, "back="},
	{COMMAND_CHS, CHS_PARAM_BACK, U8("背面=")},
	{COMMAND_CHS, CHS_PARAM_SPAN, "duration="},
	{COMMAND_CHS, CHS_PARAM_SPAN, U8("秒=")},
	{COMMAND_CHS, CHS_PARAM_BG, "background="},
	{COMMAND_CHS, CHS_PARAM_BG, U8("背景=")},
	{COMMAND_CHS, CHS_PARAM_METHOD, "effect="},
	{COMMAND_CHS, CHS_PARAM_METHOD, U8("エフェクト=")},

	/* @video */
	{COMMAND_VIDEO, VIDEO_PARAM_FILE, "file="},
	{COMMAND_VIDEO, VIDEO_PARAM_FILE, U8("ファイル=")},

	/* @chapter */
	{COMMAND_CHAPTER, CHAPTER_PARAM_NAME, "title="},
	{COMMAND_CHAPTER, CHAPTER_PARAM_NAME, U8("タイトル=")},

	/* @gui */
	{COMMAND_GUI, GUI_PARAM_FILE, "file="},
	{COMMAND_GUI, GUI_PARAM_FILE, U8("ファイル=")},

	/* @wms */
	{COMMAND_WMS, WMS_PARAM_FILE, "file="},
	{COMMAND_WMS, WMS_PARAM_FILE, U8("ファイル=")},

	/* @anime */
	{COMMAND_ANIME, ANIME_PARAM_FILE, "file="},
	{COMMAND_ANIME, ANIME_PARAM_FILE, U8("ファイル=")},

	/* @layer */
	{COMMAND_LAYER, LAYER_PARAM_NAME, "name="},
	{COMMAND_LAYER, LAYER_PARAM_NAME, U8("名前=")},
	{COMMAND_LAYER, LAYER_PARAM_FILE, "file="},
	{COMMAND_LAYER, LAYER_PARAM_FILE, U8("ファイル=")},
	{COMMAND_LAYER, LAYER_PARAM_X, "x="},
	{COMMAND_LAYER, LAYER_PARAM_Y, "y="},
	{COMMAND_LAYER, LAYER_PARAM_A, "a="},

	/* @setconfig */
	{COMMAND_SETCONFIG, SETCONFIG_PARAM_KEY, "key="},
	{COMMAND_SETCONFIG, SETCONFIG_PARAM_KEY, "名前="},
	{COMMAND_SETCONFIG, SETCONFIG_PARAM_VALUE, "value="},
	{COMMAND_SETCONFIG, SETCONFIG_PARAM_VALUE, U8("値=")},

	/* @chsx */
	{COMMAND_CHSX, CHSX_PARAM_CENTER, "c="},
	{COMMAND_CHSX, CHSX_PARAM_CENTER, "center="},
	{COMMAND_CHSX, CHSX_PARAM_CENTER, "centre="},
	{COMMAND_CHSX, CHSX_PARAM_CENTER, U8("中央=")},
	{COMMAND_CHSX, CHSX_PARAM_CX, "center-x="},
	{COMMAND_CHSX, CHSX_PARAM_CX, "centre-x="},
	{COMMAND_CHSX, CHSX_PARAM_CX, U8("中央X=")},
	{COMMAND_CHSX, CHSX_PARAM_CY, "center-y="},
	{COMMAND_CHSX, CHSX_PARAM_CY, "centre-y="},
	{COMMAND_CHSX, CHSX_PARAM_CY, U8("中央Y=")},
	{COMMAND_CHSX, CHSX_PARAM_CA, "center-a="},
	{COMMAND_CHSX, CHSX_PARAM_CA, "centre-a="},
	{COMMAND_CHSX, CHSX_PARAM_CA, U8("中央A=")},
	{COMMAND_CHSX, CHSX_PARAM_CD, "center-dim="},
	{COMMAND_CHSX, CHSX_PARAM_CD, "centre-dim="},
	{COMMAND_CHSX, CHSX_PARAM_CD, U8("中央の明暗=")},
	{COMMAND_CHSX, CHSX_PARAM_RIGHT, "r="},
	{COMMAND_CHSX, CHSX_PARAM_RIGHT, "right="},
	{COMMAND_CHSX, CHSX_PARAM_RIGHT, U8("右=")},
	{COMMAND_CHSX, CHSX_PARAM_RX, "right-x="},
	{COMMAND_CHSX, CHSX_PARAM_RX, U8("右X=")},
	{COMMAND_CHSX, CHSX_PARAM_RY, "right-y="},
	{COMMAND_CHSX, CHSX_PARAM_RY, "右Y="},
	{COMMAND_CHSX, CHSX_PARAM_RA, "right-a="},
	{COMMAND_CHSX, CHSX_PARAM_RA, "右A="},
	{COMMAND_CHSX, CHSX_PARAM_RD, "right-dim="},
	{COMMAND_CHSX, CHSX_PARAM_RD, U8("右の明暗=")},
	{COMMAND_CHSX, CHSX_PARAM_RIGHT_CENTER, "rc="},
	{COMMAND_CHSX, CHSX_PARAM_RIGHT_CENTER, "right-center="},
	{COMMAND_CHSX, CHSX_PARAM_RIGHT_CENTER, "right-centre="},
	{COMMAND_CHSX, CHSX_PARAM_RIGHT_CENTER, U8("右中=")},
	{COMMAND_CHSX, CHSX_PARAM_RCX, "right-center-x="},
	{COMMAND_CHSX, CHSX_PARAM_RCX, U8("右中X=")},
	{COMMAND_CHSX, CHSX_PARAM_RCY, "right-center-y="},
	{COMMAND_CHSX, CHSX_PARAM_RCY, "右中Y="},
	{COMMAND_CHSX, CHSX_PARAM_RCA, "right-center-a="},
	{COMMAND_CHSX, CHSX_PARAM_RCA, "右中A="},
	{COMMAND_CHSX, CHSX_PARAM_RCD, "right-center-dim="},
	{COMMAND_CHSX, CHSX_PARAM_RCD, "right-centre-dim="},
	{COMMAND_CHSX, CHSX_PARAM_RCD, U8("右中央の明暗=")},
	{COMMAND_CHSX, CHSX_PARAM_LEFT, "l="},
	{COMMAND_CHSX, CHSX_PARAM_LEFT, "left="},
	{COMMAND_CHSX, CHSX_PARAM_LEFT, U8("左=")},
	{COMMAND_CHSX, CHSX_PARAM_LX, "left-x="},
	{COMMAND_CHSX, CHSX_PARAM_LX, U8("左X=")},
	{COMMAND_CHSX, CHSX_PARAM_LY, "left-y="},
	{COMMAND_CHSX, CHSX_PARAM_LY, U8("左Y=")},
	{COMMAND_CHSX, CHSX_PARAM_LA, "left-a="},
	{COMMAND_CHSX, CHSX_PARAM_LA, U8("左A=")},
	{COMMAND_CHSX, CHSX_PARAM_LD, "left-dim="},
	{COMMAND_CHSX, CHSX_PARAM_LD, U8("左の明暗=")},
	{COMMAND_CHSX, CHSX_PARAM_LEFT_CENTER, "lc="},
	{COMMAND_CHSX, CHSX_PARAM_LEFT_CENTER, "left-center="},
	{COMMAND_CHSX, CHSX_PARAM_LEFT_CENTER, "left-centre="},
	{COMMAND_CHSX, CHSX_PARAM_LEFT_CENTER, U8("左中=")},
	{COMMAND_CHSX, CHSX_PARAM_LCX, "left-center-x="},
	{COMMAND_CHSX, CHSX_PARAM_LCX, U8("左中X=")},
	{COMMAND_CHSX, CHSX_PARAM_LCY, "left-center-y="},
	{COMMAND_CHSX, CHSX_PARAM_LCY, U8("左中Y=")},
	{COMMAND_CHSX, CHSX_PARAM_LCA, "left-center-a="},
	{COMMAND_CHSX, CHSX_PARAM_LCA, U8("左中A=")},
	{COMMAND_CHSX, CHSX_PARAM_LCD, "left-center-dim="},
	{COMMAND_CHSX, CHSX_PARAM_LCD, "left-centre-dim="},
	{COMMAND_CHSX, CHSX_PARAM_LCD, U8("左中央の明暗=")},
	{COMMAND_CHSX, CHSX_PARAM_BACK, "b="},
	{COMMAND_CHSX, CHSX_PARAM_BACK, "back="},
	{COMMAND_CHSX, CHSX_PARAM_BACK, U8("背面=")},
	{COMMAND_CHSX, CHSX_PARAM_BX, "back-x="},
	{COMMAND_CHSX, CHSX_PARAM_BX, U8("背面X=")},
	{COMMAND_CHSX, CHSX_PARAM_BY, "back-y="},
	{COMMAND_CHSX, CHSX_PARAM_BY, U8("背面Y=")},
	{COMMAND_CHSX, CHSX_PARAM_BA, "back-a="},
	{COMMAND_CHSX, CHSX_PARAM_BA, U8("背面A=")},
	{COMMAND_CHSX, CHSX_PARAM_BD, "back-dim="},
	{COMMAND_CHSX, CHSX_PARAM_BD, "背面の明暗="},
	{COMMAND_CHSX, CHSX_PARAM_BG, "bg="},
	{COMMAND_CHSX, CHSX_PARAM_BG, "background="},
	{COMMAND_CHSX, CHSX_PARAM_BG, U8("背景=")},
	{COMMAND_CHSX, CHSX_PARAM_BGX, "bg-x="},
	{COMMAND_CHSX, CHSX_PARAM_BGX, U8("背景X=")},
	{COMMAND_CHSX, CHSX_PARAM_BGY, "bg-y="},
	{COMMAND_CHSX, CHSX_PARAM_BGY, U8("背景Y=")},
	{COMMAND_CHSX, CHSX_PARAM_METHOD, "effect="},
	{COMMAND_CHSX, CHSX_PARAM_METHOD, U8("エフェクト=")},
	{COMMAND_CHSX, CHSX_PARAM_SPAN, "t="},
	{COMMAND_CHSX, CHSX_PARAM_SPAN, "duration="},
	{COMMAND_CHSX, CHSX_PARAM_SPAN, U8("秒=")},
};

#define PARAM_TBL_SIZE	(sizeof(param_tbl) / sizeof(struct param_item))

#ifdef USE_EDITOR
/*
 * スタートアップ情報
 */
char *startup_file;
int startup_line;
#endif

/*
 * 構造化モード
 */

#define GEN_CMD_SIZE			(1024)

/* キーワード */
#define SMODE_START		"<<<"
#define SMODE_END		">>>"
#define SMODE_SWITCH		"switch "
#define SMODE_CASE		"case "
#define SMODE_BREAK		"break"
#define SMODE_IF		"if"
#define SMODE_ELSEIF		"else if "
#define SMODE_ELSE		"else "
#define SMODE_CLOSE		"}"
#define SMODE_CLOSECONT		"}-"

/* パース中に入力を受け付けるキーワードのビット組み合わせ */
#define SMODE_ACCEPT_NONE		(0)
#define SMODE_ACCEPT_SWITCH		(1)
#define SMODE_ACCEPT_CASE		(1 << 1)
#define SMODE_ACCEPT_SWITCHCLOSE	(1 << 2)
#define SMODE_ACCEPT_BREAK		(1 << 3)
#define SMODE_ACCEPT_IF			(1 << 4)
#define SMODE_ACCEPT_IFCLOSE		(1 << 5)
#define SMODE_ACCEPT_IFCLOSECONT	(1 << 6)
#define SMODE_ACCEPT_ELSEIF		(1 << 7)
#define SMODE_ACCEPT_ELSE		(1 << 8)
#define SMODE_ACCEPT_ELSECLOSE		(1 << 9)

static const char *smode_target_finally;
static const char *smode_target_case;
static char *smode_target_skip;

/*
 * Forward Declarations (main)
 */

/* The script loading function. */
static bool read_script_from_file(const char *fname, bool is_included);

/* File name table manipulation. */
static const char *add_file_name(const char *fname);
static const char *search_file_name_pointer(const char *fname);

/* Non-structured script line parsers. */
static bool process_include(char *raw_buf, bool is_included);
static bool process_normal_line(const char *raw, const char *buf);

/* The command string parsers. */
static bool parse_insn(const char *raw, const char *buf, int locale_offset, int index);
static bool parse_serif(const char *raw, const char *buf, int locale_offset, int index);
static bool parse_message(const char *raw, const char *buf, int locale_offset, int index);
static bool parse_label(const char *raw, const char *buf, int locale_offset, int index);

/* The structured mode reparser. */
static bool reparse_smode(int index, int *end_index);
static bool reparse_smode_line(int index, int state, int *accepted, int *end_index);
static bool reparse_switch_block(int index, char *params, int *end_index);
static bool reparse_case_block(int index, const char *raw, int *end_index);
static bool reparse_break(int index);
static bool reparse_switch_close(int index);
static bool reparse_if_block(int index, char *params, int *end_index);
static bool reparse_if_close(int index);
static bool reparse_if_closecont(int index);
static bool reparse_elseif(int index, const char *params);
static void reparse_else(int index);
static bool reparse_normal_line(int index, int spaces);
static void nullify_command(int index);

/* Helpers. */
static bool check_size(void);
static char *strtok_escape(char *buf, bool *escaped);
static bool check_param_name_order(int command_type, int param_index, int param_name_index);
static bool starts_with(const char *s, const char *prefix);
static void show_parse_error_footer(int cmd_index, const char *raw);

/*
 * Forward Declarations (dynamic script model manipulation)
 */
#ifdef USE_EDITOR
/* For parse error handling. */
static void recover_from_parse_error(int cmd_index, const char *raw);

/* Fpr fake comments that are not describes in scripts. */
static bool add_comment_line(const char *s, ...);
#endif

/*
 * 初期化
 */

/*
 * 初期スクリプトを読み込む
 */
bool init_script(void)
{
#ifdef SUIKA_TARGET_ANDROID
	/* NDKではプロセスが再利用されるため、解放を行う */
	cleanup_script();
#endif

#ifndef USE_EDITOR
	/* スクリプトをロードする */
	if (!load_script(INIT_FILE))
		return false;
#else
	int i;

	/*
	 * 読み込むスクリプトが指定されていればそれを使用し、
	 * そうでなければinit.txtを使用する
	 */
	if (!load_script(startup_file == NULL ? INIT_FILE : startup_file))
		return false;

	/* 開始行が指定されていれば移動する */
	if (startup_line > 0) {
		for (i = 0; i < cmd_size; i++) {
			if (cmd[i].line < startup_line)
				continue;
			if (cmd[i].line >= startup_line) {
				cur_index = i;
				break;
			}
		}
	}
#endif

	return true;
}

/*
 * コマンドを破棄する
 */
void cleanup_script(void)
{
	int i, j;

	/* コマンド配列を解放する */
	for (i = 0; i < SCRIPT_CMD_SIZE; i++) {
		/* コマンドタイプをクリアする */
		cmd[i].type = COMMAND_INVALID;

		/* 行の内容を解放する */
		if (cmd[i].text != NULL) {
			free(cmd[i].text);
			cmd[i].text = NULL;
		}

		/* 引数の本体を解放する */
		if (cmd[i].param[0] != NULL) {
			free(cmd[i].param[0]);
			cmd[i].param[0] = NULL;
		}

		/* 引数の参照をNULLで上書きする */
		for (j = 1; j < PARAM_SIZE; j++)
			cmd[i].param[j] = NULL;
	}

	/* ファイル名一覧を解放する */
	for (i = 0; i < FILE_NAME_TBL_ENTRIES; i++) {
		if (file_name_tbl[i] != NULL) {
			free(file_name_tbl[i]);
			file_name_tbl[i] = NULL;
		}
	}
	used_file_names = 0;

#ifdef USE_EDITOR
	/* コメント行の配列を解放する */
	for (i = 0; i < cur_expanded_line; i++) {
		if (comment_text[i] != NULL) {
			free(comment_text[i]);
			comment_text[i] = NULL;
		}
	}
#endif

	/* 実行位置情報をクリアする */
	cur_script = NULL;
	cur_index = 0;

	/* パース位置情報をクリアする */
	cur_parse_file = NULL;
	cur_parse_line = 0;
}

/*
 * スクリプトとコマンドへの公開アクセス
 */

/*
 * スクリプトをロードする
 */
bool load_script(const char *fname)
{
	/* 現在のスクリプトを破棄する */
	cleanup_script();

	/* コマンドサイズを初期化する */
	cmd_size = 0;

	/* 行番号情報を初期化する */
	cur_expanded_line = 0;

	/* スクリプトファイルを読み込む */
	if (!read_script_from_file(fname, false))
		return false;

	/* コマンドが含まれない場合 */
	if (cmd_size == 0) {
		log_script_no_command(fname);
#ifdef USE_EDITOR
		/* デバッグ表示用のダミーのスクリプトを読み込む */
		return load_debug_script();
#else
		return false;
#endif
	}

	/* パース位置情報をクリアする */
	cur_parse_file = NULL;
	cur_parse_line = 0;

	/* 構造化文法を再度パースする */
	if (!reparse_script_for_structured_syntax())
		return false;

	/* スクリプト実行位置を設定する */
	cur_index = 0;
	cur_script = search_file_name_pointer(fname);
	assert(cur_script != NULL);
	set_return_point(INVALID_RETURN_POINT);

#ifdef USE_EDITOR
	/* スクリプトロードのタイミングでは停止要求を処理する */
	if (dbg_is_stop_requested())
		dbg_stop();
	on_load_script();
	on_change_position();
#endif

	clear_last_en_command();

	return true;
}

/*
 * スクリプトファイル名を取得する
 */
const char *get_script_file_name(void)
{
	if (cur_script == NULL)
		return "";

	return cur_script;
}

/*
 * 実行中のコマンドのインデックスを取得する(セーブ用)
 */
int get_command_index(void)
{
	return cur_index;
}

/*
 * 実行中のコマンドのインデックスを設定する(ロード用)
 */
bool move_to_command_index(int index)
{
	if (index < 0 || index >= cmd_size)
		return false;

	cur_index = index;

#ifdef USE_EDITOR
	on_change_position();
#endif

	return true;
}

/*
 * 次のコマンドに移動する
 */
bool move_to_next_command(void)
{
	assert(cur_index < cmd_size);

	/* スクリプトの末尾に達した場合 */
	if (cur_index == cmd_size - 1)
		return false;

	cur_index++;

#ifdef USE_EDITOR
	/* コマンド移動のタイミングでは停止要求を処理する */
	if (dbg_is_stop_requested())
		dbg_stop();
	on_change_position();
#endif

	return true;
}

/*
 * ラベルへ移動する
 */
bool move_to_label(const char *label)
{
	struct command *c;
	int i;

	/* ラベルを探す */
	c = NULL;
	for (i = 0; i < cmd_size; i++) {
		/* ラベルでないコマンドをスキップする */
		c = &cmd[i];
		if (c->type != COMMAND_LABEL && c->type != COMMAND_LABELEDGOTO)
			continue;

		/* ラベルがみつかった場合 */
		if (c->type == COMMAND_LABEL &&
		    strcmp(c->param[LABEL_PARAM_LABEL], label) == 0)
			break;
		if (c->type == COMMAND_LABELEDGOTO &&
		    strcmp(c->param[LABELEDGOTO_PARAM_LABEL], label) == 0)
			break;
	}
	if (c == NULL || i == cmd_size) {
		/* エラーを出力する */
		log_script_label_not_found(label);
		log_script_exec_footer();
		return false;
	}

	cur_index = i;
	if (c->type == COMMAND_LABELEDGOTO)
		cur_index++;

#ifdef USE_EDITOR
	/* コマンド移動のタイミングでは停止要求を処理する */
	if (dbg_is_stop_requested())
		dbg_stop();
	on_change_position();
#endif

	return true;
}

/*
 * ラベルへ移動する(なければfinallyラベルにジャンプする)
 */
bool move_to_label_finally(const char *label, const char *finally_label)
{
	struct command *c;
	int i;

	/* 1つめのラベルを探す */
	c = NULL;
	for (i = 0; i < cmd_size; i++) {
		/* ラベルでないコマンドをスキップする */
		c = &cmd[i];
		if (c->type != COMMAND_LABEL && c->type != COMMAND_LABELEDGOTO)
			continue;

		/* ラベルがみつかった場合 */
		if (c->type == COMMAND_LABEL &&
		    strcmp(c->param[LABEL_PARAM_LABEL], label) == 0)
			break;
		if (c->type == COMMAND_LABELEDGOTO &&
		    strcmp(c->param[LABELEDGOTO_PARAM_LABEL], label) == 0)
			break;
	}
	if (i != cmd_size) {
		assert(c != NULL);
		cur_index = i;
		if (c->type == COMMAND_LABELEDGOTO)
			cur_index++;
#ifdef USE_EDITOR
		/* コマンド移動のタイミングでは停止要求を処理する */
		if (dbg_is_stop_requested())
			dbg_stop();
		on_change_position();
#endif
		return true;
	}

	/* 2つめのラベルを探す */
	c = NULL;
	for (i = 0; i < cmd_size; i++) {
		/* ラベルでないコマンドをスキップする */
		c = &cmd[i];
		if (c->type != COMMAND_LABEL && c->type != COMMAND_LABELEDGOTO)
			continue;

		/* ラベルがみつかった場合 */
		if (c->type == COMMAND_LABEL &&
		    strcmp(c->param[LABEL_PARAM_LABEL], finally_label) == 0)
			break;
		if (c->type == COMMAND_LABELEDGOTO &&
		    strcmp(c->param[LABELEDGOTO_PARAM_LABEL], finally_label) == 0)
			break;
	}
	if (c == NULL || i == cmd_size) {
		/* エラーを出力する */
		log_script_label_not_found(finally_label);
		log_script_exec_footer();
		return false;
	}

	cur_index = i;
	if (c->type == COMMAND_LABELEDGOTO)
		cur_index++;

#ifdef USE_EDITOR
	/* コマンド移動のタイミングでは停止要求を処理する */
	if (dbg_is_stop_requested())
		dbg_stop();
	on_change_position();
#endif

	return true;
}

/*
 * gosubによるリターンポイントを記録する(gosub用)
 */
void push_return_point(void)
{
	return_point = cur_index;
}

/*
 * gosubによるリターンポイントを記録する(カスタムSYSMENUのgosub用)
 */
void push_return_point_minus_one(void)
{
	return_point = cur_index - 1;
}

/*
 * gosubによるリターンポイントを取得する(return用)
 */
int pop_return_point(void)
{
	int rp;
	rp = return_point;
	return_point = INVALID_RETURN_POINT;
	return rp;
}

/*
 * gosubによるリターンポイントの行番号を設定する(ロード用)
 *  - indexが-1ならリターンポイントは無効
 */
bool set_return_point(int index)
{
	if (index >= cmd_size)
		return false;

	return_point = index;
	return true;
}

/*
 * gosubによるリターンポイントの行番号を取得する(return,セーブ用)
 *  - indexが-1ならリターンポイントは無効
 */
int get_return_point(void)
{
	return return_point;
}

/*
 * 最後のコマンドであるかを取得する(@goto $SAVE用)
 */
bool is_final_command(void)
{
	if (cur_index == cmd_size - 1)
		return true;

	return false;
}

/*
 * コマンドの行番号を取得する(ログ用)
 */
int get_line_num(void)
{
	return cmd[cur_index].line;
}

/*
 * コマンドの行番号を取得する(ログ用)
 */
const char *get_line_string(void)
{
	struct command *c;

	assert(cur_index < cmd_size);

	c = &cmd[cur_index];

	return c->text;
}

/*
 * コマンドのタイプを取得する
 */
int get_command_type(void)
{
	struct command *c;

	assert(cur_index < cmd_size);

	c = &cmd[cur_index];
	assert(c->type > 0 && c->type < COMMAND_MAX);

	return c->type;
}

/*
 * コマンドのロケール指定を取得する
 */
const char *get_command_locale(void)
{
	struct command *c;

	assert(cur_index < cmd_size);

	c = &cmd[cur_index];

	return c->locale;
}

/*
 * 文字列のコマンドパラメータを取得する
 */
const char *get_string_param(int index)
{
	struct command *c;

	assert(cur_index < cmd_size);
	assert(index < PARAM_SIZE);

	c = &cmd[cur_index];

	/* パラメータが省略された場合 */
	if (c->param[index] == NULL)
		return "";

	/* 文字列を返す */
	return c->param[index];
}

/*
 * 整数のコマンドパラメータを取得する
 */
int get_int_param(int index)
{
	struct command *c;

	assert(cur_index < cmd_size);
	assert(index < PARAM_SIZE);

	c = &cmd[cur_index];

	/* パラメータが省略された場合 */
	if (c->param[index] == NULL)
		return 0;

	/* 整数に変換して返す */
	return atoi(c->param[index]);
}

/*
 * 浮動小数点数のコマンドパラメータを取得する
 */
float get_float_param(int index)
{
	struct command *c;

	assert(cur_index < cmd_size);
	assert(index < PARAM_SIZE);

	c = &cmd[cur_index];

	/* パラメータが省略された場合 */
	if (c->param[index] == NULL)
		return 0.0f;

	/* 浮動小数点数に変換して返す */
	return (float)atof(c->param[index]);
}

/*
 * コマンドの数を取得する
 */
int get_command_count(void)
{
	return cmd_size;
}

/*
 * スクリプトファイルの読み込み
 */

/* ファイルを読み込む */
static bool read_script_from_file(const char *fname, bool is_included)
{
	static char line_buf[LINE_BUF_SIZE];
	struct rfile *rf;
	const char *saved_cur_parse_file;
	int saved_cur_parse_line;
	bool result;

	/* ファイル名と行番号をスタックに積む */
	saved_cur_parse_file = cur_parse_file;
	saved_cur_parse_line = cur_parse_line;
	cur_parse_file = add_file_name(fname);
	if (cur_parse_file == NULL) {
		cur_parse_file = saved_cur_parse_file;
		return false;
	}
	cur_parse_line = 0;

	/* ファイルをオープンする */
	rf = open_rfile(SCRIPT_DIR, fname, false);
	if (rf == NULL)
		return false;

	/* 行ごとに処理する */
	result = true;
	while (result) {
		/* スクリプトの保存先の容量をチェックする */
		if (!check_size()) {
			result = false;
			break;
		}

		/* 行を読み込む */
		if (gets_rfile(rf, line_buf, sizeof(line_buf)) == NULL)
			break;

		/* インクルード"using"を処理する */
		if (starts_with(line_buf, MACRO_INC)) {
			if (!process_include(line_buf, is_included)) {
				result = false;
				break;
			}
			continue;
		}

		/* 通常の行を処理する */
		if (!process_normal_line(line_buf, line_buf)) {
			result = false;
			break;
		}
	}

	/* ファイルをクローズする */
	close_rfile(rf);

	/* ファイル名と行番号をスタックから戻す */
	if (is_included) {
		cur_parse_file = saved_cur_parse_file;
		cur_parse_line = saved_cur_parse_line;
	}

	return result;
}

/* ファイル名テーブルにファイル名を追加する */
static const char *add_file_name(const char *fname)
{
	if (used_file_names == FILE_NAME_TBL_ENTRIES) {
		log_script_too_many_files();
		return false;
	}

	file_name_tbl[used_file_names] = strdup(fname);
	if (file_name_tbl[used_file_names] == NULL) {
		log_memory();
		return false;
	}

	return file_name_tbl[used_file_names++];
}

/* ファイル名テーブルからファイル名ポインタを探す */
static const char *search_file_name_pointer(const char *fname)
{
	int i;

	for (i = 0; i < FILE_NAME_TBL_ENTRIES; i++) {
		if (file_name_tbl[i] == NULL)
			break;
		if (strcmp(file_name_tbl[i], fname) == 0)
			return file_name_tbl[i];
	}
	return NULL;
}

/* スクリプトの保存先の容量をチェックする */
static bool check_size(void)
{
#ifdef USE_EDITOR
	/* これ以上スクリプト行を保存できない場合 */
	if (cur_expanded_line >= SCRIPT_LINE_SIZE) {
		log_script_line_size();
		return false;
	}
#endif

	/* これ以上コマンド行を保存できない場合 */
	if (cmd_size >= SCRIPT_CMD_SIZE) {
		log_script_size(SCRIPT_CMD_SIZE);
		return false;
	}

	return true;
}

/* インクルードを処理する */
static bool process_include(char *raw_buf, bool is_included)
{
	/* 現在のところ、二重のインクルードはできない */
	if (is_included) {
		log_script_deep_include(&raw_buf[strlen(MACRO_INC)]);

#ifdef USE_EDITOR
		/* デバッガの場合、回復してコメント行扱いにする */
		if (!add_comment_line("!%s", raw_buf))
			return false;
		if (dbg_get_parse_error_count())
			log_inform_translated_commands();
		dbg_increment_parse_error_count();
		return true;
#else
		/* エンジン本体の動作の場合、エラーとする */
		return false;
#endif
	}

#ifdef USE_EDITOR
	/* デバッガの場合、開始コメント行を挿入する */
	if (!add_comment_line("<!-- // begin using %s", &raw_buf[strlen(MACRO_INC)]))
		return false;
#endif

	/* 再帰呼び出しを行う */
	if (!read_script_from_file(&raw_buf[strlen(MACRO_INC)], true))
		return false;

#ifdef USE_EDITOR
	/* デバッガの場合、終了コメント行を挿入する */
	if (!add_comment_line("--> // end using"))
		return false;
#endif

	/* インクルード成功 */
	return true;
}

/* 通常の行を処理する */
static bool process_normal_line(const char *raw, const char *buf)
{
	struct command *c;
	int top;
	bool ret;

	const int LOCALE_OFS = 4;

	c = &cmd[cmd_size];

	/* ロケールを処理する */
	top = 0;
	if (strlen(buf) > 4 && buf[0] == '+' && buf[3] == '+') {
		c->locale[0] = buf[1];
		c->locale[1] = buf[2];
		c->locale[2] = '\0';
		top = LOCALE_OFS;
	} else {
		c->locale[0] = '\0';
	}

	/* 行頭の文字で仕分けする */
	ret = true;
	switch (buf[top]) {
	case '\0':
	case '#':
#ifdef USE_EDITOR
		/* デバッガならコメントを保存する */
		if (!add_comment_line("%s", raw))
			return false;
#endif
		CONSUME_INPUT_LINE();
		return true;
	case '@':
		/* 命令行をパースする */
		CONSUME_INPUT_LINE();
		if (!parse_insn(raw, buf, top, -1))
			ret = false;
		INC_OUTPUT_LINE();
		break;
	case '*':
		/* セリフ行をパースする */
		CONSUME_INPUT_LINE();
		if (!parse_serif(raw, buf, top, -1))
			ret = false;
		INC_OUTPUT_LINE();
		break;
	case ':':
		/* ラベル行をパースする */
		CONSUME_INPUT_LINE();
		if (!parse_label(raw, buf, top, -1))
			ret = false;
		INC_OUTPUT_LINE();
		break;
	default:
		/* メッセージ行をパースする */
		CONSUME_INPUT_LINE();
		if (!parse_message(raw, buf, top, -1))
			ret = false;
		INC_OUTPUT_LINE();
		break;
	}

#ifdef USE_EDITOR
	/* デバッガの場合、パースエラーから復旧する */
	if (!ret)
		ret = true;
#endif

	if (!ret)
		return false;

	return true;
}

/* 命令行をパースする */
static bool parse_insn(const char *raw, const char *buf, int locale_offset,
		       int index)
{
	struct command *c;
	char *tp;
	int i, j, len, param_index, min = 0, max = 0;
	bool escaped;

	assert(buf[locale_offset] == '@');

	if (index == -1) {
		c = &cmd[cmd_size];
	} else {
		assert(index >= 0 && index < cmd_size);
		c = &cmd[index];
		if (cmd[index].text != NULL) {
			free(cmd[index].text);
			cmd[index].text = NULL;
		}
		if (cmd[index].param[0] != NULL) {
			free(cmd[index].param[0]);
			cmd[index].param[0] = NULL;
			for (i = 1; i < PARAM_SIZE; i++)
				cmd[index].param[i] = NULL;
		}
	}

	/* ファイル名、行番号、オリジナルの行内容を保存しておく */
	if (index == -1) {
		c->file = cur_parse_file;
		c->line = cur_parse_line;
		c->expanded_line = cur_expanded_line;
	}
	c->text = strdup(raw);
	if (c->text == NULL) {
		log_memory();
		return false;
	}

	/* トークン化する文字列を複製する */
	c->param[0] = strdup(buf + locale_offset);
	if (c->param[0] == NULL) {
		log_memory();
		return false;
	}

	/* 最初のトークンを切り出す */
	strtok_escape(c->param[0], &escaped);

	/* コマンドのタイプを取得する */
	for (i = 0; i < (int)INSN_TBL_SIZE; i++) {
		if (strcmp(c->param[0], insn_tbl[i].str) == 0) {
			c->type = insn_tbl[i].type;
			min = insn_tbl[i].min;
			max = insn_tbl[i].max;
			break;
		}
	}
	if (i == INSN_TBL_SIZE) {
		log_script_command_not_found(c->param[0]);
		show_parse_error_footer(index, raw);
		return false;
	}

	/* 2番目以降のトークンを取得する */
	i = 1;
	escaped = false;
	while ((tp = strtok_escape(NULL, &escaped)) != NULL &&
	       i < PARAM_SIZE) {
		if (strcmp(tp, "") == 0) {
			log_script_empty_string();
			show_parse_error_footer(index, raw);
			return false;
		}

		/* @set, @if, @unless, @pencilの=は引数名ではない (ex: @set $1 = 0) */
		if (c->type == COMMAND_SET ||
		    c->type == COMMAND_IF ||
		    c->type == COMMAND_UNLESS ||
		    c->type == COMMAND_PENCIL) {
			/* =も含んだまま、引数の位置にそのまま格納する */
			c->param[i] = tp;
			i++;
			continue;
		}

		/* それ以外のコマンドで引数名がない場合 */
		if (strstr(tp, "=") == NULL) {
			/*  @chsxだけは引数名が必須 */
			if (c->type == COMMAND_CHSX) {
				log_script_parameter_name_not_specified();
				show_parse_error_footer(index, raw);
				return false;
			}

			/* 引数の位置にそのまま格納する */
			c->param[i] = tp;
			i++;
			continue;
		}

		/* 引数名があるので、テーブルと一致するかチェックする */
		for (j = 0; j < (int)PARAM_TBL_SIZE; j++) {
			if (param_tbl[j].type != c->type ||
			    strncmp(param_tbl[j].name, tp,
				    strlen(param_tbl[j].name)) != 0)
				continue;

			/* 引数名の順番をチェックする */
			if (!check_param_name_order(c->type, i, j)) {
				log_script_param_order_mismatch();
				show_parse_error_footer(index, raw);
				return false;
			}

			/* 格納先引数インデックスを求める */
			if (c->type == COMMAND_CHSX)
				param_index = param_tbl[j].param_index;
			else
				param_index = i;

			/* 引数を保存する */
			c->param[param_index] = tp + strlen(param_tbl[j].name);

			/* エスケープする */
			len = (int)strlen(c->param[param_index]);
			if (c->param[param_index][0] == '\"' &&
			    c->param[param_index][len - 1] == '\"') {
				c->param[param_index][len - 1] = '\0';
				c->param[param_index]++;
			}
			i++;
			break;
		}
		if (j == PARAM_TBL_SIZE) {
			*strstr(tp, "=") = '\0';
			log_script_param_mismatch(tp);
			show_parse_error_footer(index, raw);
			return false;
		}
	}

	/* パラメータの数をチェックする */
	if (i - 1 < min) {
		log_script_too_few_param(min, i - 1);
		show_parse_error_footer(index, raw);
		return false;
	}
	if (i - 1 > max) {
		log_script_too_many_param(max, i - 1);
		show_parse_error_footer(index, raw);
		return false;
	}

	if (index == -1)
		COMMIT_CMD();
	
	/* 成功 */
	return true;
}

/* シングル/ダブルクォーテーションでエスケープ可能なトークナイズを実行する */
static char *strtok_escape(char *buf, bool *escaped)
{
	static char *top = NULL;
	char *result;

	/* 初回呼び出しの場合バッファを保存する */
	if (buf != NULL)
		top = buf;
	assert(top != NULL);

	/* すでにバッファの終端に達している場合NULLを返す */
	if (*top == '\0') {
		*escaped = false;
		return NULL;
	}

	/* 先頭のスペースをスキップする */
	for (; *top != '\0' && *top == ' '; top++)
		;
	if (*top == '\0') {
		*escaped = false;
		return NULL;
	}

	/* シングルクオーテーションでエスケープされている場合 */
	if (*top == '\'') {
		result = ++top;
		for (; *top != '\0' && *top != '\''; top++)
			;
		if (*top == '\'')
			*top++ = '\0';
		*escaped = true;
		return result;
	}

	/* ダブルクオーテーションでエスケープされている場合 */
	if (*top == '\"') {
		result = ++top;
		for (; *top != '\0' && *top != '\"'; top++)
			;
		if (*top == '\"')
			*top++ = '\0';
		*escaped = true;
		return result;
	}

	/* エスケープされていない場合 */
	result = top;
	for (; *top != '\0' && *top != ' '; top++)
		;
	if (*top == ' ')
		*top++ = '\0';
	*escaped = false;
	return result;
}

/* 引数名の順番をチェックする */
static bool check_param_name_order(int command_type, int param_index,
				   int param_name_index)
{
	/* @chsxコマンドでは引数名の順番は変更可能 */
	if (command_type == COMMAND_CHSX)
		return true;

	/* その他のコマンドでは引数の順番を変更できない */
	if (param_index != param_tbl[param_name_index].param_index)
		return false;

	return true;
}

/* '*'で始まるセリフ行をパースする */
static bool parse_serif(const char *raw, const char *buf, int locale_offset,
			int index)
{
	struct command *c;
	char *first, *second, *third;

	assert(buf[locale_offset] == '*');

	if (index == -1) {
		c = &cmd[cmd_size];
	} else {
		assert(index >= 0 && index < cmd_size);
		c = &cmd[index];
	}

	/* ファイル名、行番号、オリジナルの行内容を保存しておく */
	c->type = COMMAND_SERIF;
	if (index == -1) {
		c->file = cur_parse_file;
		c->line = cur_parse_line;
		c->expanded_line = cur_expanded_line;
	}
	c->text = strdup(raw);
	if (c->text == NULL) {
		log_memory();
		return false;
	}

	/* トークン化する文字列を複製する */
	c->param[0] = strdup(&buf[locale_offset + 1]);
	if (c->param[0] == NULL) {
		log_memory();
		return false;
	}

	/* トークンを取得する(ボイスなしなら2つ、ボイスありなら3つある) */
	first = strtok(c->param[0], "*");
	second = strtok(NULL, "*");
	third = strtok(NULL, "*");
	if (first == NULL || second == NULL) {
		log_script_empty_serif();
		show_parse_error_footer(index, raw);
		return false;
	}

	/* トークンの数で場合分けする */
	if (third != NULL) {
		/* ボイスあり */
		c->param[SERIF_PARAM_NAME] = first;
		c->param[SERIF_PARAM_VOICE] = second;
		c->param[SERIF_PARAM_MESSAGE] = third;
	} else {
		/* ボイスなし */
		c->param[SERIF_PARAM_NAME] = first;
		c->param[SERIF_PARAM_VOICE] = NULL;
		c->param[SERIF_PARAM_MESSAGE] = second;
	}

	if (index == -1)
		COMMIT_CMD();

	/* 成功 */
	return true;
}

/* メッセージ行をパースする (セリフに変換される場合がある) */
static bool parse_message(const char *raw, const char *buf, int locale_offset,
			  int index)
{
	struct command *c;
	char *lpar;
	size_t len;
	int i;

	assert(buf[locale_offset] != '@');
	assert(buf[locale_offset] != '*');
	assert(buf[locale_offset] != ':');

	if (index == -1) {
		c = &cmd[cmd_size];
	} else {
		assert(index >= 0 && index < cmd_size);
		c = &cmd[index];
		if (cmd[index].text != NULL) {
			free(cmd[index].text);
			cmd[index].text = NULL;
		}
		if (cmd[index].param[0] != NULL) {
			free(cmd[index].param[0]);
			cmd[index].param[0] = NULL;
			for (i = 1; i < PARAM_SIZE; i++)
				cmd[index].param[i] = NULL;
		}
	}

	/* 行番号とオリジナルの行(メッセージ全体)を保存しておく */
	c->type = COMMAND_MESSAGE;
	if (index == -1) {
		c->file = cur_parse_file;
		c->line = cur_parse_line;
		c->expanded_line = cur_expanded_line;
	}
	c->text = strdup(raw);
	if (c->text == NULL) {
		log_memory();
		return false;
	}

	/* メッセージを複製する (param[0]) */
	c->param[0] = strdup(buf + locale_offset);
	if (c->text == NULL) {
		log_memory();
		return false;
	}

	/*
	 * この段階でメッセージのcommandは完成済み
	 * 以下、"名前「メッセージ」"の形式の場合はセリフに変換する
	 */
	if (index == -1)
		COMMIT_CMD();

	/* メッセージ中の"「"を検索する */
	lpar = strstr(c->param[0], U8("「"));

	/* セリフに変換しない場合を除外する */
	if (lpar == NULL)
		return true;	/* "「"がないなら除外する */
	if (lpar == c->param[0])
		return true;	/* "「"が先頭文字なら除外する */
	if (strcmp(c->param[0] + strlen(c->param[0]) - 3, U8("」")) != 0)
		return true;	/* "」"が末尾文字でないなら除外する */
	if (*buf == '\\')
		return true;	/* 先頭が'\\'なら除外する(継続行は不可) */

	/* メッセージの長さを保存しておく */
	len = strlen(c->param[0]);

	/* 名前の直後の"「"の位置にNUL文字を入れる */
	*lpar = '\0';

	/* 末尾の"」"の位置にNUL文字を入れる */
	*(c->param[0] + len - 3) = '\0';

	/* セリフに変更する */
	c->type = COMMAND_SERIF;
	c->param[SERIF_PARAM_NAME] = c->param[0];
	c->param[SERIF_PARAM_VOICE] = NULL;
	c->param[SERIF_PARAM_MESSAGE] = lpar + 3;

	return true;
}

/* ラベル行をパースする */
static bool parse_label(const char *raw, const char *buf, int locale_offset,
			int index)
{
	struct command *c;
	int i;

	assert(buf[locale_offset] == ':');

	if (index == -1) {
		c = &cmd[cmd_size];
	} else {
		assert(index >= 0 && index < cmd_size);
		c = &cmd[index];
		if (cmd[index].text != NULL) {
			free(cmd[index].text);
			cmd[index].text = NULL;
		}
		if (cmd[index].param[0] != NULL) {
			free(cmd[index].param[0]);
			cmd[index].param[0] = NULL;
			for (i = 1; i < PARAM_SIZE; i++)
				cmd[index].param[i] = NULL;
		}
	}

	/* 行番号とオリジナルの行を保存しておく */
	c->type = COMMAND_LABEL;
	if (index == -1) {
		c->file = cur_parse_file;
		c->line = cur_parse_line;
		c->expanded_line = cur_expanded_line;
	}
	c->text = strdup(raw);
	if (c->text == NULL) {
		log_memory();
		return false;
	}

	/* ラベル名を保存する */
	c->param[0] = strdup(&buf[locale_offset + 1]);
	if (c->param[0] == NULL) {
		log_memory();
		return false;
	}

	if (index == -1)
		COMMIT_CMD();

	/* 成功 */
	return true;
}

/*
 * Reparse script for the structured syntax.
 */
bool reparse_script_for_structured_syntax(void)
{
	int i, ret_index;

	for (i = 0; i < cmd_size; i++) {
		assert(cmd[i].type != COMMAND_INVALID);

		if (cmd[i].type == COMMAND_MESSAGE && strcmp(cmd[i].text, SMODE_START) == 0) {
			/* Change the "<<<" message to a NULL command. */
			nullify_command(i);

			/* Reparse. */
			ret_index = i;
			if (!reparse_smode(i, &ret_index))
				return false;
			i = ret_index;

			/* Change the ">>>" message to a NULL command. */
			if (cmd[i].text != NULL && strcmp(cmd[i].text, SMODE_END) == 0)
				nullify_command(i);
		}
	}

	return true;
}

static bool reparse_smode(int index, int *end_index)
{
	int state, accepted, ret_index;

	/* ターゲットラベルを初期化する */
	if (smode_target_skip != NULL)
		free(smode_target_skip);
	smode_target_finally = NULL;
	smode_target_case = NULL;
	smode_target_skip = NULL;

	/* 受け付けるのはswitchかif */
	state = SMODE_ACCEPT_SWITCH | SMODE_ACCEPT_IF;

	/* 行ごとに処理する */
	while (true) {
		index++;
		if (index >= cmd_size)
			break;

		/* メッセージ行として処理された拡張構文の行以外はスキップする */
		if (cmd[index].type != COMMAND_MESSAGE)
			continue;

		/* ">>>"が現れたら終了する */
		if (strcmp(cmd[index].text, SMODE_END) == 0)
			break;

		/* 構造化モードの行を処理する */
		if (!reparse_smode_line(index, state, &accepted, &ret_index))
			return false;
		index = ret_index;
	}

	*end_index = index;
	return true;
}

static bool reparse_smode_line(int index, int state, int *accepted, int *end_index)
{
	int spaces, ret_index;

	/* 先頭の空白をスキップする */
	spaces = 0;
	while (cmd[index].text[spaces] != '\0' &&
	       (cmd[index].text[spaces] == ' ' || cmd[index].text[spaces] == '\t'))
		spaces++;

	/* switch文を処理する */
	if ((state & SMODE_ACCEPT_SWITCH) != 0) {
		if (starts_with(&cmd[index].text[spaces], SMODE_SWITCH)) {
			char *p = cmd[index].text + spaces + strlen(SMODE_SWITCH);
			if (!reparse_switch_block(index, p, &ret_index))
				return false;
			*accepted = SMODE_ACCEPT_SWITCH;
			*end_index = ret_index;
			return true;
		}
	}

	/* case文を処理する */
	if ((state & SMODE_ACCEPT_CASE) != 0) {
		if (starts_with(&cmd[index].text[spaces], SMODE_CASE)) {
			if (!reparse_case_block(index, &cmd[index].text[spaces], &ret_index))
				return false;
			*accepted = SMODE_ACCEPT_CASE;
			*end_index = ret_index;
			return true;
		}
	}

	/* break文を処理する */
	if ((state & SMODE_ACCEPT_BREAK) != 0) {
		if (starts_with(&cmd[index].text[spaces], SMODE_BREAK)) {
			if (!reparse_break(index))
				return false;
			*accepted = SMODE_ACCEPT_BREAK;
			*end_index = index;
			return true;
		}

		/* break前の不正な}への対策 */
		if (starts_with(&cmd[index].text[spaces], "}")) {
			log_script_close_before_break();
			*end_index = index + 1;
			return true;
		}
	}

	/* switchの'}'文を処理する */
	if ((state & SMODE_ACCEPT_SWITCHCLOSE) != 0) {
		if (strcmp(&cmd[index].text[spaces], SMODE_CLOSE) == 0) {
			if (!reparse_switch_close(index))
				return false;
			*accepted = SMODE_ACCEPT_SWITCHCLOSE;
			*end_index = index;
			return true;
		}
	}

	/* ifとそれに続くブロックを処理する(else if, elseまで再帰的に処理される) */
	if ((state & SMODE_ACCEPT_IF) != 0) {
		if (starts_with(&cmd[index].text[spaces], SMODE_IF)) {
			char *p = cmd[index].text + spaces + strlen(SMODE_IF);
			if (!reparse_if_block(index, p, &ret_index))
				return false;
			*accepted = SMODE_ACCEPT_IF;
			*end_index = ret_index;
			return true;
		}
	}

	/* ifとelse ifの"}"を処理する */
	if ((state & SMODE_ACCEPT_IFCLOSE) != 0) {
		if (strcmp(&cmd[index].text[spaces], SMODE_CLOSE) == 0) {
			if (!reparse_if_close(index))
				return false;
			*accepted = SMODE_ACCEPT_IFCLOSE;
			*end_index = index;
			return true;
		}
	}

	/* ifとelse ifの"}-"を処理する */
	if ((state & SMODE_ACCEPT_IFCLOSECONT) != 0) {
		if (strcmp(&cmd[index].text[spaces], SMODE_CLOSECONT) == 0) {
			if (!reparse_if_closecont(index))
				return false;
			*accepted = SMODE_ACCEPT_IFCLOSECONT;
			*end_index = index;
			return true;
		}
	}

	/* "else if {"文を処理する */
	if ((state & SMODE_ACCEPT_ELSEIF) != 0) {
		if (starts_with(&cmd[index].text[spaces], SMODE_ELSEIF)) {
			const char *p = cmd[index].text + spaces + strlen(SMODE_ELSEIF);
			char *stop = strstr(p, "{");
			if (stop != NULL)
				*stop = '\0';
			if (!reparse_elseif(index, p))
				return false;
			*accepted = SMODE_ACCEPT_ELSEIF;
			*end_index = index;
			return true;
		}
	}

	/* "else {"文を処理する */
	if ((state & SMODE_ACCEPT_ELSE) != 0) {
		if (starts_with(&cmd[index].text[spaces], SMODE_ELSE)) {
			reparse_else(index);
			*accepted = SMODE_ACCEPT_ELSE;
			*end_index = index;
			return true;
		}
	}

	/* else文の"}"を処理する */
	if ((state & SMODE_ACCEPT_ELSECLOSE) != 0) {
		if (starts_with(&cmd[index].text[spaces], SMODE_CLOSE)) {
			if (!reparse_if_close(index))
				return false;
			*accepted = SMODE_ACCEPT_ELSECLOSE;
			*end_index = index;
			return true;
		}
	}

	/* 字下げされた行はメッセージになっているので、本来のコマンドに変換する */
	int orig = cmd_size;
	if (spaces > 0) {
		if (!reparse_normal_line(index, spaces))
			return false;
	}
	*accepted = SMODE_ACCEPT_NONE;
	assert(cmd_size == orig);

	*end_index = index;
	return true;
}

/* 構造化switchブロックを処理する */
static bool reparse_switch_block(int index, char *params, int *end_index)
{
	char tmp_command[LINE_BUF_SIZE];
	char finally_label[GEN_CMD_SIZE];
	char *raw_save;
	char *stop;
	char *opt[8];
	char label[8][256];
	const char *save_smode_target_finally;
	const char *save_smode_target_case;
	int opt_count, i, state, accepted, cur_opt, ret_index;
	bool escaped;

	memset(opt, 0, sizeof(opt));
	memset(label, 0, sizeof(label));

	/* 文字列を保存する */
	raw_save = strdup(cmd[index].text);
	if (raw_save == NULL) {
		log_memory();
		return false;
	}

	/* paramsを"{"の位置で止める */
	stop = strstr(params, "{");
	if (stop != NULL)
		*stop = '\0';

	/* 最初のトークンを切り出す */
	opt[0] = strtok_escape(params, &escaped);
	if (opt[0] == NULL) {
		free(raw_save);
		log_script_too_few_param(2, 0);
		show_parse_error_footer(index, cmd[index].text);
		return false;
	}

	/* ２番目以降のトークンを取り出す */
	opt_count = 1;
	while ((opt[opt_count] = strtok_escape(NULL, &escaped)) != NULL &&
	       opt_count < 8) {
		if (strcmp(opt[opt_count], "{") == 0)
			break;
		opt_count++;

		/* FIXME: とりあえず空白も許可しておく */
	}
	for (i = opt_count; i < 8; i++)
		opt[i] = "";

	/* ラベル名を生成する */
	for (i = 0; i < 8; i++) {
		if (i >= opt_count) {
			label[i][0] = '\0';
		} else {
			snprintf(label[i],
				 sizeof(label[i]),
				 "CHOOSE_%d_%d",
				 cmd[index].expanded_line,
				 i);
		}
	}
	snprintf(finally_label,
		 sizeof(finally_label),
		 "CHOOSE_%d_FINALLY",
		 cmd[index].expanded_line);

	/* @chooseコマンドを生成して格納する */
	snprintf(tmp_command, sizeof(tmp_command),
		 "@choose %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s",
		 label[0], opt[0],
		 label[1], opt[1],
		 label[2], opt[2],
		 label[3], opt[3],
		 label[4], opt[4],
		 label[5], opt[5],
		 label[6], opt[6],
		 label[7], opt[7]);
	if (!parse_insn(raw_save, tmp_command, 0, index)) {
		free(raw_save);
		return false;
	}
	free(raw_save);
	raw_save = NULL;

	/* 現在のターゲットをスタックに詰む */
	save_smode_target_finally = smode_target_finally;
	save_smode_target_case = smode_target_case;

	/* ターゲットをセットする */
#if defined(__GNUC__) && __GNUC__ >= 13 && !defined(__llvm__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdangling-pointer"
#endif
	smode_target_finally = finally_label;
#if defined(__GNUC__) && __GNUC__ >= 13 && !defined(__llvm__)
#pragma GCC diagnostic pop
#endif
	smode_target_case = NULL;

	/* switchブロックが終了するまで読み込む */
	cur_opt = 0;
	while (true) {
		index++;
		if (index >= cmd_size)
			break;

		/* ステートと移動先ラベルを設定する */
		if (cur_opt < opt_count) {
			state = SMODE_ACCEPT_CASE;
#if defined(__GNUC__) && __GNUC__ >= 13 && !defined(__llvm__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdangling-pointer"
#endif
			smode_target_case = label[cur_opt];
#if defined(__GNUC__) && __GNUC__ >= 13 && !defined(__llvm__)
#pragma GCC diagnostic pop
#endif
		} else {
			state = SMODE_ACCEPT_SWITCHCLOSE;
		}

		/* 行を処理する */
		if (!reparse_smode_line(index, state, &accepted, &ret_index))
			return false;
		index = ret_index;
		if (accepted == SMODE_ACCEPT_NONE)
			continue;
		if (accepted == SMODE_ACCEPT_CASE) {
			cur_opt++;
			continue;
		}
		if (accepted == SMODE_ACCEPT_SWITCHCLOSE)
			break;
		assert(NEVER_COME_HERE);
	}

	/* ターゲットを戻す */
	smode_target_finally = save_smode_target_finally;
	smode_target_case = save_smode_target_case;

	*end_index = index;
	return true;
}

/* caseブロックを処理する */
static bool reparse_case_block(int index, const char *raw, int *end_index)
{
	char tmp_command[GEN_CMD_SIZE];
	int state, accepted, ret_index;
	char *raw_copy;

	raw_copy = strdup(raw);
	if (raw_copy == NULL) {
		log_memory();
		return false;
	}

	/* ラベルコマンドを生成して格納する */
	snprintf(tmp_command,
		 sizeof(tmp_command),
		 ":%s",
		 smode_target_case);
	if (!parse_label(raw_copy, tmp_command, 0, index)) {
		free(raw_copy);
		return false;
	}
	free(raw_copy);

	/* breakが現れるまで読み込む */
	state = SMODE_ACCEPT_BREAK | SMODE_ACCEPT_IF | SMODE_ACCEPT_SWITCH;
	while (true) {
		ret_index = index; /* avoid warning */
		index++;
		if (index >= cmd_size)
			break;

		/* 行を処理する */
		accepted = SMODE_ACCEPT_NONE; /* avoid warning */
		if (!reparse_smode_line(index, state, &accepted, &ret_index))
			return false;
		index = ret_index;
		if (accepted == SMODE_ACCEPT_NONE)
			continue;
		if (accepted == SMODE_ACCEPT_BREAK)
			break;
		if (accepted == SMODE_ACCEPT_IF)
			continue;
		if (accepted == SMODE_ACCEPT_SWITCH)
			continue;
		assert(NEVER_COME_HERE);
	}

	*end_index = index;
	return true;
}

/* breakを処理する */
static bool reparse_break(int index)
{
	char tmp_command[GEN_CMD_SIZE];
	char *raw_copy;

	raw_copy = strdup(cmd[index].text);
	if (raw_copy == NULL) {
		log_memory();
		return false;
	}

	/* @gotoコマンドを生成して格納する */
	snprintf(tmp_command,
		 sizeof(tmp_command),
		 "@goto %s",
		 smode_target_finally);
	if (!parse_insn(raw_copy, tmp_command, 0, index)) {
		free(raw_copy);
		return false;
	}

	free(raw_copy);

	return true;
}

/* switchの"}"を処理する */
static bool reparse_switch_close(int index)
{
	char tmp_command[GEN_CMD_SIZE];
	char *raw_copy;

	raw_copy = strdup(cmd[index].text);
	if (raw_copy == NULL) {
		log_memory();
		return false;
	}

	/* ラベルコマンドを生成して格納する */
	snprintf(tmp_command,
		 sizeof(tmp_command),
		 ":%s",
		 smode_target_finally);
	if (!parse_label(raw_copy, tmp_command, 0, index)) {
		free(raw_copy);
		return false;
	}
	free(raw_copy);
	
	return true;
}

/* ifブロックを処理する */
static bool reparse_if_block(int index, char *params, int *end_index)
{
	char unless_command[LINE_BUF_SIZE];
	char skip_label[GEN_CMD_SIZE];
	char finally_label[GEN_CMD_SIZE];
	const char *save_smode_target_finally;
	char *save_smode_target_skip;
	char *stop;
	char *raw_copy;
	int state, accepted, ret_index;

	raw_copy = strdup(cmd[index].text);
	if (raw_copy == NULL) {
		log_memory();
		*end_index = index;
		return false;
	}

	/* paramsを"{"の位置で止める */
	stop = strstr(params, "{");
	if (stop != NULL)
		*stop = '\0';

	/* ラベルを生成する */
	snprintf(skip_label,
		 sizeof(skip_label),
		 "IF_%d_SKIP",
		 cmd[index].expanded_line);
	snprintf(finally_label,
		 sizeof(finally_label),
		 "IF_%d_FINALLY",
		 cmd[index].expanded_line);

	/* @unlessコマンドを生成する */
	snprintf(unless_command,
		 sizeof(unless_command),
		 "@unless %s %s %s",
		 params,
		 skip_label,
		 finally_label);
	if (!parse_insn(raw_copy, unless_command, 0, index)) {
		free(raw_copy);
		return false;
	}
	free(raw_copy);

	/* 現在のターゲットをスタックに詰む */
	save_smode_target_finally = smode_target_finally;
	save_smode_target_skip = smode_target_skip;

	/* finallyターゲットを設定する */
	smode_target_finally = finally_label;

	/*
	 * skipターゲットを設定する
	 *  - else ifで付け替えられるのでstrdup()する
	 */
	smode_target_skip = strdup(skip_label);
	if (smode_target_skip == NULL) {
		log_memory();
		*end_index = index;
		return false;
	}

	/* 受け付ける文を設定する */
	state = SMODE_ACCEPT_IFCLOSE |
		SMODE_ACCEPT_IFCLOSECONT |
		SMODE_ACCEPT_SWITCH |
		SMODE_ACCEPT_IF;

	/* ifブロックが終了するまで読み込む */
	while (true) {
		index++;
		if (index >= cmd_size)
			break;

		/* 行を処理する */
		if (!reparse_smode_line(index, state, &accepted, &ret_index))
			return false;
		index = ret_index;

		/* 通常行のとき */
		if (accepted == SMODE_ACCEPT_NONE)
			continue;

		/* switchブロックのとき */
		if (accepted == SMODE_ACCEPT_SWITCH)
			continue;

		/* ifが"}"で閉じられたとき */
		if (accepted == SMODE_ACCEPT_IFCLOSE)
			break;

		/* ifが"}-"で閉じられたとき */
		if (accepted == SMODE_ACCEPT_IFCLOSECONT) {
			state = SMODE_ACCEPT_ELSEIF |
				SMODE_ACCEPT_ELSE;
			continue;
		}

		/* else ifを処理したとき */
		if (accepted == SMODE_ACCEPT_ELSEIF) {
			state = SMODE_ACCEPT_IFCLOSE |
				SMODE_ACCEPT_IFCLOSECONT |
				SMODE_ACCEPT_SWITCH |
				SMODE_ACCEPT_IF;
			continue;
		}

		/* elseを処理したとき */
		if (accepted == SMODE_ACCEPT_ELSE) {
			state = SMODE_ACCEPT_ELSECLOSE |
				SMODE_ACCEPT_SWITCH |
				SMODE_ACCEPT_IF;
			continue;
		}

		/* elseが"}"で閉じられたとき */
		if (accepted == SMODE_ACCEPT_ELSECLOSE)
			break;

		assert(NEVER_COME_HERE);
	}

	/* skipターゲットを解放する */
	if (smode_target_skip != NULL) {
		free(smode_target_skip);
		smode_target_skip = NULL;
	}

	/* ターゲットを元に戻す */
	smode_target_finally = save_smode_target_finally;
	smode_target_skip = save_smode_target_skip;

	*end_index = index;
	return true;
}

/* ifを閉じる"}"を処理する */
static bool reparse_if_close(int index)
{
	char tmp_command[GEN_CMD_SIZE];
	char *raw_copy;

	raw_copy = strdup(cmd[index].text);
	if (raw_copy == NULL) {
		log_memory();
		return false;
	}

	/* ラベルコマンドを生成する */
	snprintf(tmp_command,
		 sizeof(tmp_command),
		 ":%s",
		 smode_target_finally);
	if (!parse_label(raw_copy, tmp_command, 0, index)) {
		free(raw_copy);
		return false;
	}
	free(raw_copy);

	return true;
}

/* if/else ifを閉じる"}-"を処理する */
static bool reparse_if_closecont(int index)
{
	char tmp_command[GEN_CMD_SIZE];
	char *raw_copy;

	raw_copy = strdup(cmd[index].text);
	if (raw_copy == NULL) {
		log_memory();
		return false;
	}

	/* ラベルコマンドを生成する */
	snprintf(tmp_command,
		 sizeof(tmp_command),
		 "@labeledgoto %s %s",
		 smode_target_skip,
		 smode_target_finally);
	if (!parse_insn(raw_copy, tmp_command, 0, index)) {
		free(raw_copy);
		return false;
	}
	free(raw_copy);

	return true;
}

/* else ifを処理する */
static bool reparse_elseif(int index, const char *params)
{
	char skip_label[GEN_CMD_SIZE];
	char unless_command[LINE_BUF_SIZE];
	char *raw_copy;

	raw_copy = strdup(cmd[index].text);
	if (raw_copy == NULL) {
		log_memory();
		return false;
	}

	/* ラベルを生成する */
	snprintf(skip_label,
		 sizeof(skip_label),
		 "ELIF_%d_SKIP",
		 cmd[index].expanded_line);

	/* targetラベルを付け替える */
	if (smode_target_skip != NULL)
		free(smode_target_skip);
	smode_target_skip = strdup(skip_label);
	if (smode_target_skip == NULL) {
		log_memory();
		return false;
	}

	/* @unlessコマンドを生成する */
	snprintf(unless_command,
		 sizeof(unless_command),
		 "@unless %s %s %s",
		 params,
		 skip_label,
		 smode_target_finally);
	if (!parse_insn(raw_copy, unless_command, 0, index)) {
		free(raw_copy);
		return false;
	}
	free(raw_copy);

	return true;
}

/* elseを処理する */
static void reparse_else(int index)
{
	nullify_command(index);
}

/* 通常の行を処理する */
static bool reparse_normal_line(int index, int spaces)
{
	struct command *c;
	char *raw_copy;
	const char *save_parse_file;
	int save_parse_line;
	int save_expanded_line;
	int top;
	bool ret;

	const int LOCALE_OFS = 4;

	raw_copy = strdup(cmd[index].text);
	if (raw_copy == NULL) {
		log_memory();
		return false;
	}

	c = &cmd[cmd_size];

	/* ロケールを処理する */
	top = 0;
	if (strlen(raw_copy + spaces) > 4 && raw_copy[spaces] == '+' && raw_copy[spaces + 3] == '+') {
		c->locale[0] = raw_copy[spaces + 1];
		c->locale[1] = raw_copy[spaces + 2];
		c->locale[2] = '\0';
		top = LOCALE_OFS;
	} else {
		c->locale[0] = '\0';
	}

	save_parse_file = cur_parse_file;
	save_parse_line = cur_parse_line;
	save_expanded_line = cur_expanded_line;
	cur_parse_file = c->file;
	cur_parse_line = c->line;
	cur_expanded_line = c->expanded_line;

	/* 行頭の文字で仕分けする */
	ret = true;
	switch (raw_copy[spaces + top]) {
	case '\0':
	case '#':
		nullify_command(index);
		break;
	case '@':
		/* 命令行をパースする */
		if (!parse_insn(raw_copy, raw_copy + spaces, top, index))
			ret = false;
		break;
	case '*':
		/* セリフ行をパースする */
		if (!parse_serif(raw_copy, raw_copy + spaces, top, index))
			ret = false;
		break;
	case ':':
		/* ラベル行をパースする */
		if (!parse_label(raw_copy, raw_copy + spaces, top, index))
			ret = false;
		break;
	default:
		/* メッセージ行をパースする */
		if (!parse_message(raw_copy, raw_copy + spaces, top, index))
			ret = false;
		break;
	}

#ifdef USE_EDITOR
	/* デバッガの場合、パースエラーから復旧する */
	if (!ret)
		ret = true;
#endif

	cur_parse_file = save_parse_file;
	cur_parse_line = save_parse_line;
	cur_expanded_line = save_expanded_line;
	free(raw_copy);

	if (!ret)
		return false;

	return true;
}

/* NULL-ify the command. */
static void nullify_command(int index)
{
	int i;

	cmd[index].type = COMMAND_NULL;
	if (cmd[index].param[0] != NULL) {
		free(cmd[index].param[0]);
		cmd[index].param[0] = NULL;
	}
	for (i = 1; i < PARAM_SIZE; i++)
		cmd[index].param[i] = NULL;
}

/* 文字列sがprefixで始まるかをチェックする */
static bool starts_with(const char *s, const char *prefix)
{
	if (strncmp(s, prefix, strlen(prefix)) == 0)
		return true;
	return false;
}

/* パースエラーのフッタを表示する */
static void show_parse_error_footer(int index, const char *raw)
{
#ifndef USE_EDITOR
	/* フッタを表示する */
	log_script_parse_footer(cur_parse_file, cur_parse_line, raw);

	UNUSED_PARAMETER(index);
#else
	/* デバッガ動作の場合、パースエラーから回復する */
	recover_from_parse_error(index, raw);
#endif
}

/*
 * For Suika2 Pro
 */
#ifdef USE_EDITOR

/* パースエラーから回復する */
static void recover_from_parse_error(int cmd_index, const char *raw)
{
	struct command *c;

	if (cmd_index == -1)
		c = &cmd[cmd_size];
	else
		c = &cmd[cmd_index];

	/* コマンドの種類、ファイル、行番号を設定する */
	c->type = COMMAND_MESSAGE;
	c->file = cur_parse_file;
	c->line = cur_parse_line;
	c->expanded_line = cur_expanded_line;

	/* rawテキストを複製する */
	if (c->text != NULL) {
		free(c->text);
		c->text = NULL;
	}
	c->text = strdup(raw);
	if (c->text == NULL) {
		log_memory();
		abort();
	}

	/* rawテキストの先頭文字を'@'から'!'に変更する */
	c->text[0] = '!';

	/* メッセージとしてparam[0]に複製する */
	if (c->param[0] != NULL) {
		free(c->param[0]);
		c->param[0] = NULL;
	}
	c->param[0] = strdup(c->text);
	if (c->param[0] == NULL) {
		log_memory();
		abort();
	}

	/* on-the-flyの更新でなければ処理済みコマンドの数を増やす */
	if (cmd_index == -1)
		COMMIT_CMD();

	/* 最初のパースエラーであれば、メッセージに変換された旨を表示する */
	if(dbg_get_parse_error_count() == 0)
		log_inform_translated_commands();
	dbg_increment_parse_error_count();
}

/* コメント行を追加する */
static bool add_comment_line(const char *s, ...)
{
	char buf[1024];
	va_list ap;

	va_start(ap, s);
	vsnprintf(buf, sizeof(buf), s, ap);
	va_end(ap);

	comment_text[cur_expanded_line] = strdup(buf);
	if (comment_text[cur_expanded_line] == NULL) {
		log_memory();
		return false;
	}

	INC_OUTPUT_LINE();
	return true;
}

/*
 * スタートアップファイル/ラインを指定する
 */
bool set_startup_file_and_line(const char *file, int line)
{
	startup_file = strdup(file);
	if (startup_file == NULL) {
		log_memory();
		return false;
	}
	startup_line = line;
	return true;
}

/*
 * スタートアップファイルが指定されたか
 */
bool has_startup_file(void)
{
	if (startup_file != NULL)
		return true;

	return false;
}

/*
 * using展開後のコマンドの行番号を取得する(ログ用)
 */
int get_expanded_line_num(void)
{
	return cmd[cur_index].expanded_line;
}

/*
 * 行の数を取得する
 */
int get_line_count(void)
{
	return cur_expanded_line;
}

/*
 * 指定した行番号以降の最初のコマンドインデックスを取得する
 */
int get_command_index_from_line_num(int line)
{
	int i;

	for (i = 0; i < cmd_size; i++)
		if (cmd[i].expanded_line >= line)
			return i;

	return -1;
}

/*
 *  指定した行番号の行全体を取得する
 */
const char *get_line_string_at_line_num(int line)
{
	int i;

	/* コメント行の場合 */
	if (comment_text[line] != NULL)
		return comment_text[line];

	/* コマンドを探す */
	for (i = 0; i < cmd_size; i++) {
		if (cmd[i].expanded_line == line) {
			assert(cmd[i].text != NULL);
			return cmd[i].text;
		}
		if (cmd[i].expanded_line > line)
			break;
	}

	/* 空行の場合 */
	return "";
}

/*
 * 実行エラー時に@コマンドを'!'で始まるメッセージに変換する
 */
void translate_command_to_message_for_runtime_error(int index)
{
	struct command *c;
	int i;

	assert(index >= 0 && index < cmd_size);

	c = &cmd[index];

	/* コマンドの種類をメッセージに変更する */
	c->type = COMMAND_MESSAGE;

	/* rawテキストの先頭文字を'@'から'!'に変更する */
	c->text[0] = '!';

	/* メッセージとしてparam[0]に複製する */
	if (c->param[0] != NULL) {
		free(c->param[0]);
		c->param[0] = NULL;
	}
	c->param[0] = strdup(c->text);
	if (c->param[0] == NULL) {
		log_memory();
		abort();
	}
	for (i = 1; i < PARAM_SIZE; i++)
		c->param[i] = NULL;

	/* 実行時エラー状態を設定する */
	dbg_raise_runtime_error();
}

/*
 * デバッグ用の仮のスクリプトをロードする
 */
bool load_debug_script(void)
{
	cleanup_script();

	cur_script = add_file_name("DEBUG");
	if (cur_script == NULL) {
		log_memory();
		cleanup_script();
		return false;
	}

	cur_index = 0;
	cmd_size = 1;
	cur_expanded_line = 1;

	cmd[0].type = COMMAND_MESSAGE;
	cmd[0].line = 0;
	cmd[0].expanded_line = 0;
	cmd[0].text = strdup(conf_locale == LOCALE_JA ?
			     U8("実行を終了しました。") :
			     "Execution finished.");
	if (cmd[0].text == NULL) {
		log_memory();
		cleanup_script();
		return false;
	}
	cmd[0].param[0] = strdup(cmd[0].text);
	if (cmd[0].text == NULL) {
		log_memory();
		cleanup_script();
		return false;
	}

	return true;
}

/*
 * スクリプトの行を挿入する
 */
bool insert_script_line(int line, const char *text)
{
	assert(line <= cur_expanded_line);
	assert(text != NULL);

	if (text[0] != '#' && text[0] != '\0') {
		/* コマンドを挿入する */
		if (!insert_command(line, text))
			return false;
	} else {
		/* コメントを挿入する */
		insert_comment(line, text);
	}

	return true;
}

/*
 * スクリプトの行をアップデートする
 */
bool update_script_line(int line, const char *text)
{
	int cmd_index;

	assert(line < cur_expanded_line);
	assert(text != NULL);

	/* 行番号line以降の最初のコマンドを探す */
	cmd_index = get_command_index_from_line_num(line);

	/* 行番号line以降にコマンドがあるか */
	if (cmd_index != -1) {
		/* 行番号lineのちょうどその位置にコマンドがあるか */
		if (cmd[cmd_index].expanded_line == line) {
			/* あるので、そのコマンドをアップデートする */
			if (cmd[cmd_index].type == COMMAND_NULL ||
			    strcmp(cmd[cmd_index].text, text) != 0) {
				if (text[0] != '#' && text[0] != '\0') {
					if (!replace_command_by_command(cmd_index, text))
						return false;
				} else {
					replace_command_by_comment(cmd_index, text);
				}
			}
		} else {
			/* ないが、行番号line+1以降にコマンドがある */
			if (text[0] != '#' && text[0] != '\0') {
				if (!replace_comment_by_command(line, text))
					return false;
			} else {
				replace_comment_by_comment(line, text);
			}
		}
	} else {
		/* 行番号line以降にコマンドがない場合 */
		if (text[0] != '#' && text[0] != '\0') {
			/* コマンドを挿入する */
			if (!insert_command(line, text))
				return false;
		} else {
			/* 行番号lineにコメントがあるか */
			if (comment_text[line] != NULL) {
				/* コメントを置き換える */
				replace_comment_by_comment(line, text);
			} else {
				/* コメントを挿入する */
				insert_comment(line, text);
			}
		}
	}

	return true;
}

/*
 * コマンドを書き換える
 */
static bool replace_command_by_command(int index, const char *text)
{
	struct command *c;
	const char *save_parse_file;
	int save_parse_line;
	int save_expanded_line;
	int top;
	bool ret;

	assert(index >= 0 && index < cmd_size);
	assert(text != NULL);
	assert(text[0] != '#');
	assert(text[0] != '\0');
	assert(comment_text[cmd[index].expanded_line] == NULL);

	c = &cmd[index];

	/* コマンドの文字列を解放する */
	if (c->text != NULL) {
		assert(text != c->text);
		free(c->text);
		c->text = NULL;
	}
	if (c->param[0] != NULL) {
		free(c->param[0]);
		c->param[0] = NULL;
	}

	/* ロケールを処理する */
	top = 0;
	if (strlen(text) > 4 && text[0] == '+' && text[3] == '+') {
		c->locale[0] = text[1];
		c->locale[1] = text[2];
		c->locale[2] = '\0';
		top = 4;
	} else {
		c->locale[0] = '\0';
	}

	/* パース位置の情報を設定する */
	save_parse_file = cur_parse_file;
	save_parse_line = cur_parse_line;
	save_expanded_line = cur_expanded_line;
	cur_parse_file = c->file;
	cur_parse_line = c->line;
	cur_expanded_line = c->expanded_line;

	/* 行頭の文字で仕分けする */
	ret = true;
	switch (text[top]) {
	case '@':
		if (!parse_insn(text, text, top, index))
			ret = false;
		break;
	case '*':
		if (!parse_serif(text, text, top, index))
			ret = false;
		break;
	case ':':
		if (!parse_label(text, text, top, index))
			ret = false;
		break;
	default:
		if (!parse_message(text, text, top, index))
			ret = false;
		break;
	}

	/* on-the-flyのパースを終了する */
	cur_parse_file = save_parse_file;
	cur_parse_line = save_parse_line;
	cur_expanded_line = save_expanded_line;

	return ret;
}

/* コマンド行をコメント行に置き換える */
static void replace_command_by_comment(int cmd_index, const char *text)
{
	int i, line;

	assert(text != NULL);

	/* 行番号を求める */
	line = cmd[cmd_index].expanded_line;

	/* コメントを保存する */
	assert(comment_text[line] == NULL);
	if (text[0] != '\0') {
		comment_text[line] = strdup(text);
		if (comment_text[line] == NULL)
			log_memory();
	}

	/* コマンドを解放する */
	if (cmd[cmd_index].text != NULL) {
		free(cmd[cmd_index].text);
		cmd[cmd_index].text = NULL;
	}
	if (cmd[cmd_index].param[0] != NULL) {
		free(cmd[cmd_index].param[0]);
		cmd[cmd_index].param[0] = NULL;
	}
	memset(&cmd[cmd_index], 0, sizeof(struct command));

	/* cmd_index+1以降のコマンドを1つずつ手前にずらす */
	for (i = cmd_index; i < SCRIPT_CMD_SIZE - 1; i++)
		cmd[i] = cmd[i + 1];
	memset(&cmd[SCRIPT_CMD_SIZE - 1], 0, sizeof(struct command));
	cmd_size--;
}

/* コメント行をコマンド行に置き換える */
static bool replace_comment_by_command(int line, const char *text)
{
	int i, cmd_index;

	assert(text != NULL);

	/* コメント行が存在することをチェックする */
	assert(line >= 0);
	assert(line < cur_expanded_line);

	/* コメント行のテキストを解放する */
	if (comment_text[line] != NULL) {
		free(comment_text[line]);
		comment_text[line] = NULL;
	}

	/* 行番号line以降の最初のコマンドを探す */
	cmd_index = get_command_index_from_line_num(line);

	/* 行番号line以降にコマンドがある場合 (末尾のコメントでない場合) */
	if (cmd_index != -1) {
		/* cmd_index以降のコマンドを1つずつ後ろにずらす */
		for (i = cmd_size; i > cmd_index; i--)
			cmd[i] = cmd[i - 1];
		memset(&cmd[cmd_index], 0, sizeof(struct command));
	}

	/* コマンドをパースする */
	cmd_size++;
	cmd[cmd_index].file = cur_script;
	cmd[cmd_index].line = line;
	cmd[cmd_index].expanded_line = line;
	if (!replace_command_by_command(cmd_index, text))
		return false;

	return true;
}

/* コメント行を置き換える */
static void replace_comment_by_comment(int line, const char *text)
{
	/* コメント行のテキストを置き換える */
	if (comment_text[line] != NULL) {
		free(comment_text[line]);
		comment_text[line] = NULL;
	}
	if (text[0] != '\0') {
		comment_text[line] = strdup(text);
		if (comment_text[line] == NULL)
			log_memory();
	}
}

/* コメントを挿入する */
static void insert_comment(int line, const char *text)
{
	int i, cmd_index;

	assert(text != NULL);

	/* コメントテーブルがいっぱいでないことを確認する */
	assert(line < SCRIPT_LINE_SIZE - 1);
	assert(text != NULL);
	assert(text[0] == '#' || text[0] == '\0');
	assert(cmd_size < SCRIPT_CMD_SIZE - 1);

	/* 行番号line以降のコメントについて、1つずつ後ろにずらす */
	for (i = SCRIPT_LINE_SIZE - 1; i > line; i--)
		comment_text[i] = comment_text[i - 1];
	comment_text[line] = NULL;

	/* 空けた行にコメントを設定する */
	if (text[0] != '\0') {
		comment_text[line] = strdup(text);
		if (comment_text[line] == NULL)
			log_memory();
	}

	/* 行番号line以降のコマンドについて、行番号を1加算する */
	cmd_index = get_command_index_from_line_num(line);
	if (cmd_index != -1) {
		for (i = cmd_index; i < cmd_size; i++) {
			cmd[i].line++;
			cmd[i].expanded_line++;
		}
	}

	if (line < cur_expanded_line)
		cur_expanded_line++;
	else
		cur_expanded_line = line + 1;
}

/* コマンドを挿入する */
static bool insert_command(int line, const char *text)
{
	int i, cmd_index;

	assert(line < SCRIPT_LINE_SIZE - 1);
	assert(text != NULL);
	assert(text[0] != '#' && text[0] != '\0');
	assert(cmd_size < SCRIPT_CMD_SIZE - 1);

	/* 行番号lineにすでにコメントがあれば削除する */
	if (comment_text[line] != NULL) {
		free(comment_text[line]);
		comment_text[line] = NULL;
	}

	/* 行番号line+1以降のコメントについて、1つずつ後ろにずらす */
	for (i = SCRIPT_LINE_SIZE - 1; i > line; i--)
		comment_text[i] = comment_text[i - 1];

	/* 行番号line以降の最初のコマンドを探す */
	cmd_index = get_command_index_from_line_num(line);
	if (cmd_index != -1) {
		/* コマンドがある場合、cmd_index以降のコマンドを1つずつ後ろにずらす */
		for (i = cmd_size; i > cmd_index; i--) {
			cmd[i] = cmd[i - 1];
			cmd[i].line++;
			cmd[i].expanded_line++;
		}
		memset(&cmd[cmd_index], 0, sizeof(struct command));
		cmd_size++;
	} else {
		/* コマンドがない場合、末尾に追加する */
		cmd_index = cmd_size;
		cmd_size++;
	}

	cur_expanded_line++;

	/* 追加するコマンドをパースする */
	cmd[cmd_index].file = cur_script;
	cmd[cmd_index].line = line;
	cmd[cmd_index].expanded_line = line;
	if (!replace_command_by_command(cmd_index, text))
		return false;

	return true;
}

/*
 * スクリプトの行を削除する
 *  - return: 実行行が変更された場合true
 */
bool delete_script_line(int line)
{
	int i, cmd_index;

	assert(line < cur_expanded_line);

	/* コメント行であればテキストを解放する */
	if (comment_text[line] != NULL) {
		free(comment_text[line]);
		comment_text[line] = NULL;
	}

	/* line以降のコメントを1つずつ前の要素にずらす */
	for (i = line; i < SCRIPT_LINE_SIZE - 1; i++)
		comment_text[i] = comment_text[i + 1];
	comment_text[SCRIPT_LINE_SIZE - 1] = NULL;

	/* コマンド行であれば解放する */
	cmd_index = get_command_index_from_line_num(line);
	if (cmd_index != -1 && cmd[cmd_index].expanded_line == line) {
		/* コマンドを解放する */
		if (cmd[cmd_index].text != NULL) {
			free(cmd[cmd_index].text);
			cmd[cmd_index].text = NULL;
		}
		if (cmd[cmd_index].param[0] != NULL) {
			free(cmd[cmd_index].param[0]);
			cmd[cmd_index].param[0] = NULL;
		}
		memset(&cmd[cmd_index], 0, sizeof(struct command));

		/* cmd_index+1以降のコマンドを1つずつ手前にずらす */
		for (i = cmd_index; i < cmd_size - 1; i++)
			cmd[i] = cmd[i + 1];
		memset(&cmd[cmd_size - 1], 0, sizeof(struct command));
		cmd_size--;
	}

	/* cmd_index以降のコマンドの行番号を1つ減らす */
	if (cmd_index != -1) {
		for (i = cmd_index; i < cmd_size; i++) {
			cmd[i].line--;
			cmd[i].expanded_line--;
		}
	}

	/* 実行行を補正する */
	if (cur_index >= cmd_size) {
		cur_index = cmd_size - 1;
		return true;
	}
	return false;
}

/*
 * スクリプトを保存する
 */
bool save_script(void)
{
	FILE *fp;
	char *path;
	int i;
	bool in_using;

	const char USING_BEGIN_PREFIX[] = "<!-- // begin using ";
	const char USING_END[] = "--> // end using";

	/* DEBUG画面なら保存しない */
	if (strcmp(cur_script, "DEBUG") == 0)
		return false;

	/* パスを生成する */
	path = make_valid_path(SCRIPT_DIR, cur_script);

	/* ファイルをオープンする */
	fp = fopen(path, "wb");
	if (fp == NULL) {
		free(path);
		return false;
	}
	free(path);

	/* 行ごとに処理する */
	in_using = false;
	for (i = 0; i < get_line_count(); i++) {
		const char *line;
		int out, lf;

		/* 行の文字列を取得する */
		line = get_line_string_at_line_num(i);

		/* 行を出力する */
		if (!in_using) {
			if (strncmp(line, USING_BEGIN_PREFIX, strlen(USING_BEGIN_PREFIX)) == 0) {
				in_using = true;
				out = fputs(strstr(line, "using"), fp);
			} else {
				out = fputs(line, fp);
			}
		} else {
            if (strcmp(line, USING_END) == 0) {
                in_using = false;
                out = 1;
            } else {
                out = fputs(line, fp);
            }
		}
		lf = fputs("\n", fp);
		if(out < 0 || lf < 0) {
			fclose(fp);
			return false;
		}			
	}
	fclose(fp);

	return true;
}

/*
 * コマンド名からコマンドタイプを返す
 */
int get_command_type_from_name(const char *name)
{
	int i;

	for (i = 0; i < (int)INSN_TBL_SIZE; i++)
		if (strcmp(name, insn_tbl[i].str) == 0)
			return insn_tbl[i].type;

	return -1;
}

#endif /* USE_EDITOR */
