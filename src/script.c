/* -*- coding: utf-8; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Suika 2
 * Copyright (C) 2001-2023, TABATA Keiichi. All rights reserved.
 */

/*
 * [Changes]
 *  - 2016/06/01 作成
 *  - 2017/08/13 スイッチに対応
 *  - 2018/07/21 @gosubに対応
 *  - 2019/09/17 @newsに対応
 *  - 2021/06/05 @bgにエフェクトを追加
 *  - 2021/06/05 @volにマスターボリュームを追加
 *  - 2021/06/05 @menuのボタン数を増やした
 *  - 2021/06/06 @seにボイスストリーム出力を追加
 *  - 2021/06/10 @bgと@chにマスク描画を追加
 *  - 2021/06/10 @chにオフセットとアルファを追加
 *  - 2021/06/10 @chaに対応
 *  - 2021/06/12 @shakeに対応
 *  - 2021/06/15 @setsaveに対応
 *  - 2021/07/07 @goto $SAVEに対応
 *  - 2021/07/19 @chsに対応
 *  - 2022/05/11 @videoに対応
 *  - 2022/06/05 @skipに対応
 *  - 2022/06/06 デバッガに対応
 *  - 2022/06/17 @chooseに対応
 *  - 2022/07/29 @guiに対応
 *  - 2022/10/19 ローケルに対応
 *  - 2023/01/06 日本語コマンド名、パラメータ名指定、カギカッコに対応
 *  - 2023/01/14 スタートアップファイル/ラインに対応
 *  - 2023/08/14 usingに対応
 *  - 2023/08/14 @ichooseに対応
 *  - 2023/08/27 構造化構文に対応
 *  - 2023/08/27 @setconfigに対応
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

/* コマンドの引数の最大数(コマンド名も含める) */
#define PARAM_SIZE	(137)

/* コマンド配列 */
static struct command {
	int type;
	int line;
	char *text;
	char *param[PARAM_SIZE];
	char locale[3];
} cmd[SCRIPT_CMD_SIZE];

/* コマンドの数 */
static int cmd_size;

/*
 * パース中の行番号
 */
int script_lines;

#define INC_LINE()		script_lines++

#ifdef USE_DEBUGGER

/* コメント行のテキスト */
#define SCRIPT_LINE_SIZE	(65536)

/* コメント行 */
static char *comment_text[SCRIPT_LINE_SIZE];

/* エラーの数 */
static int error_count;

/* 現在パース中の行でエラーがあったか */
static bool is_parse_error;

#define INC_ERROR()		error_count++

#else

#define INC_ERROR()

#endif


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
	{"@bg", COMMAND_BG, 1, 3},
	{U8("@背景"), COMMAND_BG, 1, 3},

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
	{"@wait", COMMAND_WAIT, 1, 1},
	{U8("@時間待ち"), COMMAND_WAIT, 1, 1},

	/* ラベルへジャンプ */
	{"@goto", COMMAND_GOTO, 1, 1},
	{U8("@ジャンプ"), COMMAND_GOTO, 1, 1},

	/* ラベルかつジャンプ */
	{"@labeledgoto", COMMAND_LABELEDGOTO, 2, 2},

	/* シナリオファイルのロード */
	{"@load", COMMAND_LOAD, 1, 1},
	{U8("@シナリオ"), COMMAND_LOAD, 1, 1},

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

	/* ビデオ再生 */
	{"@video", COMMAND_VIDEO, 1, 2},
	{U8("@動画"), COMMAND_VIDEO, 1, 2},

	/* 選択肢 */
	{"@choose", COMMAND_CHOOSE, 2, 16},
	{U8("@選択肢"), COMMAND_CHOOSE, 2, 16},

	/* インライン選択肢 */
	{"@ichoose", COMMAND_ICHOOSE, 2, 16},
	{U8("@インライン選択肢"), COMMAND_ICHOOSE, 2, 16},

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

	/* マクロ */
	{"@gosub", COMMAND_GOSUB, 1, 1},
	{"@return", COMMAND_RETURN, 0, 0},

	/* コンフィグ変更 */
	{"@setconfig", COMMAND_SETCONFIG, 1, 2},
	{U8("@設定"), COMMAND_SETCONFIG, 1, 2},

	/* その他 */
	{"@setsave", COMMAND_SETSAVE, 1, 1},

	/* deprecated */
	{"@select", COMMAND_SELECT, 6, 6},
	{"@menu", COMMAND_MENU, 7, 83},
	{"@news", COMMAND_NEWS, 9, 136},
	{"@retrospect", COMMAND_RETROSPECT, 11, 55},
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

	/* @wms */
	{COMMAND_ANIME, ANIME_PARAM_FILE, "file="},
	{COMMAND_ANIME, ANIME_PARAM_FILE, U8("ファイル=")},

	/* @setconfig */
	{COMMAND_SETCONFIG, SETCONFIG_PARAM_KEY, "key="},
	{COMMAND_SETCONFIG, SETCONFIG_PARAM_KEY, "名前="},
	{COMMAND_SETCONFIG, SETCONFIG_PARAM_VALUE, "value="},
	{COMMAND_SETCONFIG, SETCONFIG_PARAM_VALUE, U8("値=")},
};

#define PARAM_TBL_SIZE	(sizeof(param_tbl) / sizeof(struct param_item))

/*
 * コマンド実行ポインタ
 */

/* 実行中のスクリプト名 */
static char *cur_script;

/* 実行中のコマンド番号 */
static int cur_index;

/* 最後にgosubが実行されたコマンド番号 */
static int return_point;

/* 無効なreturn_pointの値 */
#define INVALID_RETURN_POINT	(-2)


#ifdef USE_DEBUGGER
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
 * 前方参照
 */
static bool read_script_from_file(const char *fname, bool is_included);
static bool check_size(void);
static bool process_include(char *raw_buf, bool is_included);
static bool process_smode(struct rfile *rf, const char *fname);
static bool process_smode_line(struct rfile *rf, const char *fname,
			       char *line_buf, int state, int *accepted);
static bool process_switch_block(struct rfile *rf, const char *fname,
				 const char *raw, char *params);
static bool process_case_block(struct rfile *rf, const char *fname,
			       const char *raw);
static bool process_break(const char *fname, const char *raw);
static bool process_switch_close(const char *fname, const char *raw);
static bool process_if_block(struct rfile *rf, const char *fname,
			     const char *raw, char *params);
static bool process_if_close(const char *fname, const char *raw);
static bool process_if_closecont(const char *fname, const char *raw);
static bool process_elseif(const char *fname, const char *raw,
			   const char *params);
static bool process_else(const char *raw);
static bool process_normal_line(const char *raw, const char *buf,
				const char *fname);
static bool parse_insn(int index, const char *fname, int line,
		       const char *raw, const char *buf, int locale_offset);
static char *strtok_escape(char *buf, bool *escaped);
static bool parse_serif(int index, const char *fname, int line,
			const char *raw, const char *buf, int locale_offset);
static bool parse_message(int index, const char *fname, int line,
			  const char *raw, const char *buf, int locale_offset);
static bool parse_label(int index, const char *fname, int line,
			const char *raw, const char *buf, int locale_offset);
static bool starts_with(const char *s, const char *prefix);

#ifdef USE_DEBUGGER
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
#ifndef USE_DEBUGGER
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

	for (i = 0; i < SCRIPT_CMD_SIZE; i++) {
		/* コマンドタイプをクリアする */
		cmd[i].type = COMMAND_MIN;

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

#ifdef USE_DEBUGGER
	for (i = 0; i < script_lines; i++) {
		if (comment_text[i] != NULL) {
			free(comment_text[i]);
			comment_text[i] = NULL;
		}
	}
#endif

	if (cur_script != NULL) {
		free(cur_script);
		cur_script = NULL;
	}
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

	/* スクリプト名を保存する */
	cur_index = 0;
	cur_script = strdup(fname);
	if (cur_script == NULL) {
		log_memory();
		return false;
	}

	/* コマンドサイズを初期化する */
	cmd_size = 0;

#ifdef USE_DEBUGGER
	/* 行番号情報を初期化する */
	script_lines = 0;

	/* エラーカウントを初期化する */
	error_count = 0;
#endif

	/* スクリプトファイルを読み込む */
	if (!read_script_from_file(fname, false))
		return false;

	/* コマンドが含まれない場合 */
	if (cmd_size == 0) {
		log_script_no_command(fname);
#ifdef USE_DEBUGGER
		/* デバッグ表示用のダミーのスクリプトを読み込む */
		return load_debug_script();
#else
		return false;
#endif
	}

	/* リターンポイントを無効にする */
	set_return_point(INVALID_RETURN_POINT);

#ifdef USE_DEBUGGER
	if (dbg_is_stop_requested())
		dbg_stop();
	update_debug_info(true);
#endif

	return true;
}

/*
 * スクリプトファイル名を取得する
 */
const char *get_script_file_name(void)
{
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

#ifdef USE_DEBUGGER
	if (dbg_is_stop_requested())
		dbg_stop();
	update_debug_info(false);
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
	if (++cur_index == cmd_size)
		return false;

#ifdef USE_DEBUGGER
	if (dbg_is_stop_requested())
		dbg_stop();
	update_debug_info(false);
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

#ifdef USE_DEBUGGER
	if (dbg_is_stop_requested())
		dbg_stop();
	update_debug_info(false);
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
		cur_index = i;
		if (c->type == COMMAND_LABELEDGOTO)
			cur_index++;
#ifdef USE_DEBUGGER
		if (dbg_is_stop_requested())
			dbg_stop();
		update_debug_info(false);
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
		    strcmp(c->param[LABEL_PARAM_LABEL], label) == 0)
			break;
		if (c->type == COMMAND_LABELEDGOTO &&
		    strcmp(c->param[LABELEDGOTO_PARAM_LABEL], label) == 0)
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

#ifdef USE_DEBUGGER
	if (dbg_is_stop_requested())
		dbg_stop();
	update_debug_info(false);
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
	assert(c->type > COMMAND_MIN && c->type < COMMAND_MAX);

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
	bool result;

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

		/* 構造化モードの開始"<<<"を処理する */
		if (strcmp(line_buf, SMODE_START) == 0) {
			if (!process_smode(rf, fname)) {
				result = false;
				break;
			}
			continue;
		}

		/* 通常の行を処理する */
		if (!process_normal_line(line_buf,line_buf, fname)) {
			result = false;
			break;
		}
	}

	/* ファイルをクローズする */
	close_rfile(rf);

	return result;
}

/* スクリプトの保存先の容量をチェックする */
static bool check_size(void)
{
#ifdef USE_DEBUGGER
	/* これ以上スクリプト行を保存できない場合 */
	if (script_lines > SCRIPT_LINE_SIZE) {
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

#ifdef USE_DEBUGGER
		/* デバッガの場合、回復してコメント行扱いにする */
		if (!add_comment_line("!%s", raw_buf))
			return false;
		INC_ERROR();
		return true;
#else
		/* エンジン本体の動作の場合、エラーとする */
		return false;
#endif
	}

#ifdef USE_DEBUGGER
	/* デバッガの場合、開始コメント行を挿入する */
	if (!add_comment_line("<!-- // using %s",
			      &raw_buf[strlen(MACRO_INC)]))
		return false;
#endif

	/* 再帰呼び出しを行う */
	if (!read_script_from_file(&raw_buf[strlen(MACRO_INC)], true))
		return false;

#ifdef USE_DEBUGGER
	/* デバッガの場合、終了コメント行を挿入する */
	if (!add_comment_line("--> // using %s",
			      &raw_buf[strlen(MACRO_INC)]))
		return false;
#endif

	/* インクルード成功 */
	return true;
}

/* 構造化モードを処理する */
static bool process_smode(struct rfile *rf, const char *fname)
{
	char line_buf[LINE_BUF_SIZE];
	int state, accepted;

#ifdef USE_DEBUGGER
	/* デバッガの場合、コメント行を追加する */
	if (!add_comment_line("<<<"))
		return false;
#else
	INC_LINE();
#endif

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
		/* スクリプトの保存先の容量をチェックする */
		if (!check_size())
			return false;

		/* 行を読み込む */
		if (gets_rfile(rf, line_buf, sizeof(line_buf)) == NULL)
			break;

		/* ">>>"が現れたら終了する */
		if (strcmp(line_buf, SMODE_END) == 0)
			break;

		/* 構造化モードの行を処理する */
		if (!process_smode_line(rf, fname, line_buf, state, &accepted))
			return false;
	}

#ifdef USE_DEBUGGER
	/* デバッガの場合、コメント行">>>"を追加する */
	if (!add_comment_line(SMODE_END))
		return false;
#else
	INC_LINE();
#endif

	/* 構造化モード正常終了 */
	return true;
}

/* 構造化モードの行を処理する */
static bool process_smode_line(struct rfile *rf, const char *fname,
			       char *line_buf, int state, int *accepted)
{
	int spaces;

	/* 先頭の空白をスキップする */
	spaces = 0;
	while (line_buf[spaces] != '\0' &&
	       (line_buf[spaces] == ' ' || line_buf[spaces] == '\t'))
		spaces++;

	/* 空行だった場合 */
	if (line_buf[spaces] == '\0') {
#ifdef USE_DEBUGGER
		if (!add_comment_line(""))
			return false;
#else
		INC_LINE();
#endif
		*accepted = SMODE_ACCEPT_NONE;
		return true;
	}

	/* コメントを処理する */
	if (line_buf[spaces] == '#') {
#ifdef USE_DEBUGGER
		if (!add_comment_line("%s", line_buf))
			return false;
#else
		INC_LINE();
#endif
		*accepted = SMODE_ACCEPT_NONE;
		return true;
	}

	/* switch文を処理する */
	if ((state & SMODE_ACCEPT_SWITCH) != 0) {
		if (starts_with(&line_buf[spaces], SMODE_SWITCH)) {
			char *p = line_buf + spaces + strlen(SMODE_SWITCH);
			if (!process_switch_block(rf, fname, line_buf, p))
				return false;
			*accepted = SMODE_ACCEPT_SWITCH;
			return true;
		}
	}

	/* case文を処理する */
	if ((state & SMODE_ACCEPT_CASE) != 0) {
		if (starts_with(&line_buf[spaces], SMODE_CASE)) {
			if (!process_case_block(rf, fname, line_buf))
				return false;
			*accepted = SMODE_ACCEPT_CASE;
			return true;
		}
	}

	/* break文を処理する */
	if ((state & SMODE_ACCEPT_BREAK) != 0) {
		if (starts_with(&line_buf[spaces], SMODE_BREAK)) {
			if (!process_break(fname, line_buf))
				return false;
			*accepted = SMODE_ACCEPT_BREAK;
			return true;
		}
	}

	/* switchの'}'文を処理する */
	if ((state & SMODE_ACCEPT_SWITCHCLOSE) != 0) {
		if (strcmp(&line_buf[spaces], SMODE_CLOSE) == 0) {
			if (!process_switch_close(fname, line_buf))
				return false;
			*accepted = SMODE_ACCEPT_SWITCHCLOSE;
			return true;
		}
	}

	/* ifブロックを処理する(else if, elseまで再帰的に処理される) */
	if ((state & SMODE_ACCEPT_IF) != 0) {
		if (starts_with(&line_buf[spaces], SMODE_IF)) {
			char *p = line_buf + spaces + strlen(SMODE_IF);
			if (!process_if_block(rf, fname, line_buf, p))
				return false;
			*accepted = SMODE_ACCEPT_IF;
			return true;
		}
	}

	/* if/else if文の"}"を処理する */
	if ((state & SMODE_ACCEPT_IFCLOSE) != 0) {
		if (strcmp(&line_buf[spaces], SMODE_CLOSE) == 0) {
			if (!process_if_close(fname, line_buf))
				return false;
			*accepted = SMODE_ACCEPT_IFCLOSE;
			return true;
		}
	}

	/* if/else if文の"}-"を処理する */
	if ((state & SMODE_ACCEPT_IFCLOSECONT) != 0) {
		if (strcmp(&line_buf[spaces], SMODE_CLOSECONT) == 0) {
			if (!process_if_closecont(fname, line_buf))
				return false;
			*accepted = SMODE_ACCEPT_IFCLOSECONT;
			return true;
		}
	}

	/* else if文を処理する */
	if ((state & SMODE_ACCEPT_ELSEIF) != 0) {
		if (starts_with(&line_buf[spaces], SMODE_ELSEIF)) {
			const char *p = line_buf + spaces + strlen(SMODE_ELSEIF);
			char *stop = strstr(p, "{");
			if (stop != NULL)
				*stop = '\0';
			if (!process_elseif(fname, line_buf, p))
				return false;
			*accepted = SMODE_ACCEPT_ELSEIF;
			return true;
		}
	}

	/* else文を処理する */
	if ((state & SMODE_ACCEPT_ELSE) != 0) {
		if (starts_with(&line_buf[spaces], SMODE_ELSE)) {
			if (!process_else(line_buf))
				return false;
			*accepted = SMODE_ACCEPT_ELSE;
			return true;
		}
	}

	/* else文の"}"を処理する */
	if ((state & SMODE_ACCEPT_ELSECLOSE) != 0) {
		if (starts_with(&line_buf[spaces], SMODE_CLOSE)) {
			if (!process_if_close(fname, line_buf))
				return false;
			*accepted = SMODE_ACCEPT_ELSECLOSE;
			return true;
		}
	}

	/* 構造化されていない通常の行としてを処理する */
	if (!process_normal_line(line_buf, &line_buf[spaces], fname))
		return false;
	*accepted = SMODE_ACCEPT_NONE;
	return true;
}

/* 構造化switchブロックを処理する */
static bool process_switch_block(struct rfile *rf, const char *fname,
				 const char *raw, char *params)
{
	char line_buf[LINE_BUF_SIZE];
	char tmp_command[LINE_BUF_SIZE];
	char finally_label[GEN_CMD_SIZE];
	char *raw_save;
	char *stop;
	char *opt[8];
	char label[8][256];
	const char *save_smode_target_finally;
	const char *save_smode_target_case;
	int opt_count, i, state, accepted, cur_opt;
	bool escaped;

	/* rawを保存する */
	raw_save = strdup(raw);
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
		log_script_parse_footer(fname, script_lines, raw);
		return false;
	}

	/* ２番目以降のトークンを取り出す */
	opt_count = 1;
	while ((opt[opt_count] = strtok_escape(NULL, &escaped)) != NULL &&
	       opt_count < 8) {
		if (strcmp(opt[opt_count], "}") == 0)
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
			snprintf(label[i], sizeof(label[i]), "CHOOSE_%d_%d",
				 script_lines, i);
		}
	}
	snprintf(finally_label, sizeof(finally_label),
		 "CHOOSE_%d_FINALLYd", script_lines);

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
	if (!parse_insn(cmd_size, fname, script_lines, raw_save, tmp_command,
			0)) {
		free(raw_save);
		return false;
	}
	INC_LINE();
	free(raw_save);
	raw_save = NULL;

	/* 現在のターゲットをスタックに詰む */
	save_smode_target_finally = smode_target_finally;
	save_smode_target_case = smode_target_case;

	/* ターゲットをセットする */
	smode_target_finally = finally_label;
	smode_target_case = NULL;

	/* switchブロックが終了するまで読み込む */
	cur_opt = 0;
	while (true) {
		/* スクリプトの保存先の容量をチェックする */
		if (!check_size())
			return false;

		/* 行を読み込む */
		if (gets_rfile(rf, line_buf, sizeof(line_buf)) == NULL)
			break;

		/* ステートと移動先ラベルを設定する */
		if (cur_opt < opt_count) {
			state = SMODE_ACCEPT_CASE;
			smode_target_case = label[cur_opt];
		} else {
			state = SMODE_ACCEPT_SWITCHCLOSE;
		}

		/* 行を処理する */
		if (!process_smode_line(rf, fname, line_buf, state, &accepted))
			return false;
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

	return true;
}

/* caseブロックを処理する */
static bool process_case_block(struct rfile *rf, const char *fname,
			       const char *raw)
{
	char line_buf[LINE_BUF_SIZE];
	char tmp_command[GEN_CMD_SIZE];
	int state, accepted;

	/* ラベルコマンドを生成して格納する */
	snprintf(tmp_command, sizeof(tmp_command), ":%s", smode_target_case);
	if (!parse_label(cmd_size, fname, script_lines, raw, tmp_command, 0))
		return false;
	INC_LINE();

	/* breakが現れるまで読み込む */
	state = SMODE_ACCEPT_BREAK | SMODE_ACCEPT_IF | SMODE_ACCEPT_SWITCH;
	while (true) {
		/* スクリプトの保存先の容量をチェックする */
		if (!check_size())
			return false;

		/* 行を読み込む */
		if (gets_rfile(rf, line_buf, sizeof(line_buf)) == NULL)
			break;

		/* 行を処理する */
		if (!process_smode_line(rf, fname, line_buf, state, &accepted))
			return false;
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

	return true;
}

/* breakを処理する */
static bool process_break(const char *fname, const char *raw)
{
	char tmp_command[GEN_CMD_SIZE];

	/* @gotoコマンドを生成して格納する */
	snprintf(tmp_command, sizeof(tmp_command), "@goto %s",
		 smode_target_finally);
	if (!parse_insn(cmd_size, fname, script_lines, raw, tmp_command, 0))
		return false;
	INC_LINE();

	return true;
}

/* switchの"}"を処理する */
static bool process_switch_close(const char *fname, const char *raw)
{
	char tmp_command[GEN_CMD_SIZE];

	/* ラベルコマンドを生成して格納する */
	snprintf(tmp_command, sizeof(tmp_command), ":%s", smode_target_finally);
	if (!parse_label(cmd_size, fname, script_lines, raw, tmp_command, 0))
		return false;
	INC_LINE();

	return true;
}

/* ifブロックを処理する */
static bool process_if_block(struct rfile *rf, const char *fname,
			     const char *raw, char *params)
{
	char line_buf[LINE_BUF_SIZE];
	char unless_command[LINE_BUF_SIZE];
	char skip_label[GEN_CMD_SIZE];
	char finally_label[GEN_CMD_SIZE];
	const char *save_smode_target_finally;
	char *save_smode_target_skip;
	char *stop;
	int state, accepted;

	/* paramsを"{"の位置で止める */
	stop = strstr(params, "{");
	if (stop != NULL)
		*stop = '\0';

	/* ラベルを生成する */
	snprintf(skip_label, sizeof(skip_label), "IF_%d_SKIP", script_lines);
	snprintf(finally_label, sizeof(finally_label), "IF_%d_FINALLY", script_lines);

	/* @unlessコマンドを生成する */
	snprintf(unless_command, sizeof(unless_command), "@unless %s %s %s",
		 params, skip_label, finally_label);
	if (!parse_insn(cmd_size, fname, script_lines, raw, unless_command, 0))
		return false;
	INC_LINE();

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
		return false;
	}

	/* 受け付ける文を設定する */
	state = SMODE_ACCEPT_IFCLOSE |
		SMODE_ACCEPT_IFCLOSECONT |
		SMODE_ACCEPT_SWITCH |
		SMODE_ACCEPT_IF;

	/* ifブロックが終了するまで読み込む */
	while (true) {
		/* スクリプトの保存先の容量をチェックする */
		if (!check_size())
			return false;

		/* 行を読み込む */
		if (gets_rfile(rf, line_buf, sizeof(line_buf)) == NULL)
			break;

		/* 行を処理する */
		if (!process_smode_line(rf, fname, line_buf, state, &accepted))
			return false;

		/* 空行のとき */
		if (accepted == SMODE_ACCEPT_NONE)
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
		if (accepted == SMODE_ACCEPT_ELSEIF) {
			state = SMODE_ACCEPT_ELSECLOSE |
				SMODE_ACCEPT_SWITCH |
				SMODE_ACCEPT_IF;
			continue;
		}

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

	return true;
}

/* ifを閉じる"}"を処理する */
static bool process_if_close(const char *fname, const char *raw)
{
	char tmp_command[GEN_CMD_SIZE];

	/* ラベルコマンドを生成する */
	snprintf(tmp_command, sizeof(tmp_command), ":%s",
		 smode_target_finally);
	if (!parse_label(cmd_size, fname, script_lines, raw, tmp_command, 0))
		return false;
	INC_LINE();
	return true;
}

/* if/else ifを閉じる"}-"を処理する */
static bool process_if_closecont(const char *fname, const char *raw)
{
	char tmp_command[GEN_CMD_SIZE];

	/* ラベルコマンドを生成する */
	snprintf(tmp_command, sizeof(tmp_command), "@labeledgoto %s %s",
		 smode_target_skip, smode_target_finally);
	if (!parse_insn(cmd_size, fname, script_lines, raw, tmp_command, 0))
		return false;
	INC_LINE();
	return true;
}

/* else ifを処理する */
static bool process_elseif(const char *fname, const char *raw,
			   const char *params)
{
	char skip_label[GEN_CMD_SIZE];
	char unless_command[LINE_BUF_SIZE];

	/* ラベルを生成する */
	snprintf(skip_label, sizeof(skip_label), "ELIF_%d_SKIP", script_lines);

	/* targetラベルを付け替える */
	if (smode_target_skip != NULL)
		free(smode_target_skip);
	smode_target_skip = strdup(skip_label);
	if (smode_target_skip == NULL) {
		log_memory();
		return false;
	}

	/* @unlessコマンドを生成する */
	snprintf(unless_command, sizeof(unless_command), "@unless %s %s %s",
		 params, skip_label, smode_target_finally);
	if (!parse_insn(cmd_size, fname, script_lines, raw, unless_command, 0))
		return false;
	INC_LINE();

	return true;
}

/* elseを処理する */
static bool process_else(const char *raw)
{
#ifdef USE_DEBUGGER
	if (!add_comment_line("%s", raw))
		return false;
#else
	UNUSED_PARAMETER(raw);
#endif
	INC_LINE();
	return true;
}

/* 通常の行を処理する */
static bool process_normal_line(const char *raw, const char *buf,
				const char *fname)
{
	int top;
	bool ret;

	const int BUF_OFS = 4;

	/* ロケールを処理する */
	top = 0;
	if (strlen(buf) > 4 && buf[0] == '+' && buf[3] == '+') {
		cmd[cmd_size].locale[0] = buf[1];
		cmd[cmd_size].locale[1] = buf[2];
		cmd[cmd_size].locale[2] = '\0';
		top = BUF_OFS;
	} else {
		cmd[cmd_size].locale[0] = '\0';
	}

	/* 行頭の文字で仕分けする */
	ret = true;
	switch (buf[top]) {
	case '\0':
	case '#':
#ifdef USE_DEBUGGER
		/* デバッガならコメントを保存する */
		if (!add_comment_line("%s", buf))
			return false;
#else
		INC_LINE();
#endif
		return true;
	case '@':
		/* 命令行をパースする */
		if (!parse_insn(cmd_size, fname, script_lines, raw, buf, top))
			ret = false;
		break;
	case '*':
		/* セリフ行をパースする */
		if (!parse_serif(cmd_size, fname, script_lines, raw, buf, top))
			ret = false;
		break;
	case ':':
		/* ラベル行をパースする */
		if (!parse_label(cmd_size, fname, script_lines, raw, buf, top))
			ret = false;
		break;
	default:
		/* メッセージ行をパースする */
		if (!parse_message(cmd_size, fname, script_lines, raw, buf, top))
			ret = false;
		break;
	}

#ifdef USE_DEBUGGER
	/* デバッガの場合、パースエラーから復旧する */
	if (!ret && is_parse_error) {
		is_parse_error = false;
		ret = true;
	}
#endif

	if (!ret)
		return false;

	INC_LINE();

	return true;
}

/* 命令行をパースする */
static bool parse_insn(int index, const char *file, int line, const char *raw,
		       const char *buf, int locale_offset)
{
	struct command *c;
	char *tp;
	int i, j, len, min = 0, max = 0;
	bool escaped;

#ifdef USE_DEBUGGER
	UNUSED_PARAMETER(file);
#endif

	c = &cmd[index];

	/* 行番号とオリジナルの行を保存しておく */
	c->line = line;
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
		if (strcasecmp(c->param[0], insn_tbl[i].str) == 0) {
			c->type = insn_tbl[i].type;
			min = insn_tbl[i].min;
			max = insn_tbl[i].max;
			break;
		}
	}
	if (i == INSN_TBL_SIZE) {
		log_script_command_not_found(c->param[0]);
#ifdef USE_DEBUGGER
		is_parse_error = true;
		cmd[index].text[0] = '!';
		set_error_command(index, cmd[index].text);
		if(error_count == 0)
			log_command_update_error();
		error_count++;
#else
		log_script_parse_footer(file, line, buf);
#endif
		return false;
	}

	/* 2番目以降のトークンを取得する */
	i = 1;
	escaped = false;
	while ((tp = strtok_escape(NULL, &escaped)) != NULL &&
	       i < PARAM_SIZE) {
		if (strcmp(tp, "") == 0) {
			log_script_empty_string();
#ifdef USE_DEBUGGER
			is_parse_error = true;
			cmd[index].text[0] = '!';
			set_error_command(index, cmd[index].text);
			if(error_count == 0)
				log_command_update_error();
			error_count++;
#else
			log_script_parse_footer(file, line, buf);
#endif
			return false;
		}

		/* パラメータ名がない場合 */
		if (escaped ||
		    c->type == COMMAND_SET || c->type == COMMAND_IF ||
		    c->type == COMMAND_UNLESS ||
		    strstr(tp, "=") == NULL) {
			c->param[i] = tp;
			i++;
			continue;
		}

		/* パラメータ名をチェックする */
		for (j = 0; j < (int)PARAM_TBL_SIZE; j++) {
			if (strncmp(param_tbl[j].name, tp,
				    strlen(param_tbl[j].name)) == 0) {
				/* 引数を保存する */
				c->param[i] = tp + strlen(param_tbl[j].name);

				/* エスケープする */
				len = (int)strlen(c->param[i]);
				if (c->param[i][0] == '\"' &&
				    c->param[i][len - 1] == '\"') {
					c->param[i][len - 1] = '\0';
					c->param[i]++;
				}
				i++;
				break;
			}
		}
		if (j == PARAM_TBL_SIZE) {
			*strstr(tp, "=") = '\0';
			log_script_param_mismatch(tp);
#ifdef USE_DEBUGGER
			is_parse_error = true;
			cmd[index].text[0] = '!';
			set_error_command(index, cmd[index].text);
			if(error_count == 0)
				log_command_update_error();
			error_count++;
#else
			log_script_parse_footer(file, line, buf);
#endif
			return false;
		}
	}

	/* パラメータの数をチェックする */
	if (i - 1 < min) {
		log_script_too_few_param(min, i - 1);
#ifdef USE_DEBUGGER
		is_parse_error = true;
		cmd[index].text[0] = '!';
		set_error_command(index, cmd[index].text);
		if(error_count++ == 0)
			log_command_update_error();
#else
		log_script_parse_footer(file, line, buf);
#endif
		return false;
	}
	if (i - 1 > max) {
		log_script_too_many_param(max, i - 1);
#ifdef USE_DEBUGGER
		is_parse_error = true;
		cmd[index].text[0] = '!';
		set_error_command(index, cmd[index].text);
		if(error_count++ == 0)
			log_command_update_error();
#else
		log_script_parse_footer(file, line, buf);
#endif
		return false;
	}

	cmd_size++;

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

/* セリフ行をパースする */
static bool parse_serif(int index, const char *file, int line, const char *raw,
			const char *buf, int locale_offset)
{
	char *first, *second, *third;

	assert(buf[locale_offset] == '*');

#ifdef USE_DEBUGGER
	UNUSED_PARAMETER(file);
#endif

	/* 行番号とオリジナルの行を保存しておく */
	cmd[index].type = COMMAND_SERIF;
	cmd[index].line = line;
	cmd[index].text = strdup(raw);
	if (cmd[index].text == NULL) {
		log_memory();
		return false;
	}

	/* トークン化する文字列を複製する */
	cmd[index].param[0] = strdup(&buf[locale_offset + 1]);
	if (cmd[index].param[0] == NULL) {
		log_memory();
		return false;
	}

	/* トークンを取得する(2つか3つある) */
	first = strtok(cmd[index].param[0], "*");
	second = strtok(NULL, "*");
	third = strtok(NULL, "*");
	if (first == NULL || second == NULL) {
		log_script_empty_serif();
#ifdef USE_DEBUGGER
		is_parse_error = true;
		cmd[index].text[0] = '!';
		set_error_command(index, cmd[index].text);
		if(error_count == 0)
			log_command_update_error();
		error_count++;
#else
		log_script_parse_footer(file, line, buf);
#endif
		return false;
	}

	/* トークンの数で場合分けする */
	if (third != NULL) {
		cmd[index].param[SERIF_PARAM_NAME] = first;
		cmd[index].param[SERIF_PARAM_VOICE] = second;
		cmd[index].param[SERIF_PARAM_MESSAGE] = third;
	} else {
		cmd[index].param[SERIF_PARAM_NAME] = first;
		cmd[index].param[SERIF_PARAM_VOICE] = NULL;
		cmd[index].param[SERIF_PARAM_MESSAGE] = second;
	}

	cmd_size++;

	/* 成功 */
	return true;
}

/* メッセージ行をパースする */
static bool parse_message(int index, const char *file, int line,
			  const char *raw, const char *buf, int locale_offset)
{
	char *lpar, *p;

	UNUSED_PARAMETER(file);

	/* 行番号とオリジナルの行(メッセージ全体)を保存しておく */
	cmd[index].type = COMMAND_MESSAGE;
	cmd[index].line = line;
	cmd[index].text = strdup(raw);
	if (cmd[index].text == NULL) {
		log_memory();
		return false;
	}

	/* メッセージ(0番目のパラメータ)を複製する */
	cmd[index].param[MESSAGE_PARAM_MESSAGE] = strdup(buf + locale_offset);
	if (cmd[index].text == NULL) {
		log_memory();
		return false;
	}

	/* 名前「メッセージ」の形式の場合はセリフとする(継続行を除く) */
	p = cmd[index].param[MESSAGE_PARAM_MESSAGE];
	lpar = strstr(p, U8("「"));
	if (lpar != NULL &&
	    lpar != p &&
	    strcmp(p + strlen(p) - 3, U8("」")) == 0 &&
	    *buf != '\\') {
		/* セリフに変更する */
		cmd[index].type = COMMAND_SERIF;

		/* トークン化する */
		*(p + strlen(p) - 3) = '\0';
		*lpar = '\0';
		cmd[index].param[SERIF_PARAM_NAME] = p;
		cmd[index].param[SERIF_PARAM_VOICE] = NULL;
		cmd[index].param[SERIF_PARAM_MESSAGE] = lpar + 3;
	}

	cmd_size++;

	/* 成功 */
	return true;
}

/* ラベル行をパースする */
static bool parse_label(int index, const char *file, int line, const char *raw,
			const char *buf, int locale_offset)
{
	UNUSED_PARAMETER(file);

	/* 行番号とオリジナルの行(メッセージ全体)を保存しておく */
	cmd[index].type = COMMAND_LABEL;
	cmd[index].line = line;
	cmd[index].text = strdup(raw);
	if (cmd[index].text == NULL) {
		log_memory();
		return false;
	}

	/* ラベルを保存する */
	cmd[index].param[LABEL_PARAM_LABEL] = strdup(&buf[locale_offset + 1]);
	if (cmd[index].param[LABEL_PARAM_LABEL] == NULL) {
		log_memory();
		return false;
	}

	cmd_size++;

	/* 成功 */
	return true;
}

static bool starts_with(const char *s, const char *prefix)
{
	if (strncmp(s, prefix, strlen(prefix)) == 0)
		return true;
	return false;
}

#ifdef USE_DEBUGGER
/* コメント行を追加する */
static bool add_comment_line(const char *s, ...)
{
	char buf[1024];
	va_list ap;

	va_start(ap, s);
	vsnprintf(buf, sizeof(buf), s, ap);
	va_end(ap);

	comment_text[script_lines] = strdup(buf);
	if (comment_text[script_lines] == NULL) {
		log_memory();
		return false;
	}

	INC_LINE();
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
 * 行の数を取得する
 */
int get_line_count(void)
{
	return script_lines;
}

/*
 * 指定した行番号以降の最初のコマンドインデックスを取得する
 */
int get_command_index_from_line_number(int line)
{
	int i;

	for (i = 0; i < cmd_size; i++)
		if (cmd[i].line >= line)
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
		if (cmd[i].line == line)
			return cmd[i].text;
		if (cmd[i].line > line)
			break;
	}

	/* 空行の場合 */
	return "";
}

/*
 * デバッグ用に1コマンドだけ書き換える
 */
bool update_command(int index, const char *cmd_str)
{
	int line;
	int top;

	/* メッセージに変換されるメッセージボックスを表示するようにする */
	error_count = 0;

	/* コマンドのメモリを解放する */
	if (cmd[index].text != NULL) {
		free(cmd[index].text);
		cmd[index].text = NULL;
	}
	if (cmd[index].param[0] != NULL) {
		free(cmd[index].param[0]);
		cmd[index].param[0] = NULL;
	}

	/* ロケールを処理する */
	top = 0;
	if (strlen(cmd_str) > 4 && cmd_str[0] == '+' && cmd_str[3] == '+') {
		cmd[index].locale[0] = cmd_str[1];
		cmd[index].locale[1] = cmd_str[2];
		cmd[index].locale[2] = '\0';
		top = 4;
	} else {
		cmd[index].locale[0] = '\0';
	}

	/* 行頭の文字で仕分けする */
	line = cmd[index].line;
	switch (cmd_str[4]) {
	case '@':
		if (!parse_insn(index, cur_script, line, cmd_str, cmd_str, top))
			return false;
		return true;
	case '*':
		if (!parse_serif(index, cur_script, line, cmd_str, cmd_str, top))
			return false;
		return true;
	case ':':
		if (!parse_label(index, cur_script, line, cmd_str, cmd_str, top))
			return false;
		return true;
	case '\0':
		/* 空行は空白1つに変換する */
		cmd_str = " ";
		/* fall-thru */
	case '#':
		/* コメントもメッセージにする */
		/* fall-thru */
	default:
		if (!parse_message(index, cur_script, line, cmd_str, cmd_str, top))
			return false;
		return true;
	}
	return true;
}

/*
 * エラー時のコマンドを設定する
 */
void set_error_command(int index, char *text)
{
	if (cmd[index].text != text)
		if (cmd[index].text != NULL)
			free(cmd[index].text);
	if (cmd[index].param[0] != NULL)
		free(cmd[index].param[0]);

	cmd[index].type = COMMAND_MESSAGE;
	cmd[index].text = text;
	cmd[index].param[0] = NULL;
}

/*
 * デバッグ用の仮のスクリプトをロードする
 */
bool load_debug_script(void)
{
	cleanup_script();

	cur_script = strdup("DEBUG");
	if (cur_script == NULL) {
		log_memory();
		cleanup_script();
		return false;
	}

	cur_index = 0;
	cmd_size = 1;
	script_lines = 1;

	cmd[0].type = COMMAND_MESSAGE;
	cmd[0].line = 0;
	cmd[0].text = strdup(conf_locale == LOCALE_JA ?
			     /* "実行を終了しました" (utf-8) */
			     "\xe5\xae\x9f\xe8\xa1\x8c\xe3\x82\x92\xe7\xb5\x82"
			     "\xe4\xba\x86\xe3\x81\x97\xe3\x81\xbe\xe3\x81\x97"
			     "\xe3\x81\x9f" :
			     "Execution finished.");
	if (cmd[0].text == NULL) {
		log_memory();
		cleanup_script();
		return false;
	}

	update_debug_info(true);
	return true;
}
#endif
