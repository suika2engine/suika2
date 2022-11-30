/* -*- coding: utf-8; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Suika 2
 * Copyright (C) 2001-2022, TABATA Keiichi. All rights reserved.
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
 */

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "suika.h"

/* 1行の読み込みサイズ */
#define LINE_BUF_SIZE	(65536)

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

/* 行数 */
int script_lines;

#ifdef USE_DEBUGGER
/* コメント行のテキスト */
#define SCRIPT_LINE_SIZE	(65536)
static char *comment_text[SCRIPT_LINE_SIZE];

/* エラーの数 */
static int error_count;
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
	{"@bg", COMMAND_BG, 1, 3},
	{"@bgm", COMMAND_BGM, 1, 2},
	{"@ch", COMMAND_CH, 1, 7},
	{"@click", COMMAND_CLICK, 0, 1},
	{"@wait", COMMAND_WAIT, 1, 1},
	{"@goto", COMMAND_GOTO, 1, 1},
	{"@load", COMMAND_LOAD, 1, 1},
	{"@vol", COMMAND_VOL, 2, 3},
	{"@set", COMMAND_SET, 3, 3},
	{"@if", COMMAND_IF, 4, 4},
	{"@select", COMMAND_SELECT, 6, 6},
	{"@se", COMMAND_SE, 1, 2},
	{"@menu", COMMAND_MENU, 7, 83},
	{"@news", COMMAND_NEWS, 9, 136},
	{"@retrospect", COMMAND_RETROSPECT, 11, 55},
	{"@switch", COMMAND_SWITCH, 9, 136},
	{"@gosub", COMMAND_GOSUB, 1, 1},
	{"@return", COMMAND_RETURN, 0, 0},
	{"@cha", COMMAND_CHA, 6, 6},
	{"@shake", COMMAND_SHAKE, 4, 4},
	{"@setsave", COMMAND_SETSAVE, 1, 1},
	{"@chs", COMMAND_CHS, 4, 7},
	{"@video", COMMAND_VIDEO, 1, 1},
	{"@skip", COMMAND_SKIP, 1, 1},
	{"@choose", COMMAND_CHOOSE, 2, 16},
	{"@chapter", COMMAND_CHAPTER, 1, 1},
	{"@gui", COMMAND_GUI, 1, 2},
};

#define INSN_TBL_SIZE	(sizeof(insn_tbl) / sizeof(struct insn_item))

/*
 * コマンド実行ポインタ
 */

/* 実行中のスクリプト名 */
static char *cur_script;

/* 実行中の行番号 */
static int cur_index;

/* 最後にgosubが実行された行番号 */
static int return_point;

#ifdef USE_DEBUGGER
/*
 * その他
 */

/* パースエラー状態 */
static bool is_parse_error;
#endif

/*
 * 前方参照
 */
static bool read_script_from_file(const char *fname);
static bool parse_insn(int index, const char *fname, int line,
		       const char *buf, int locale_offset);
static char *strtok_escape(char *buf);
static bool parse_serif(int index, const char *fname, int line,
			const char *buf, int locale_offset);
static bool parse_message(int index, const char *fname, int line,
			  const char *buf, int locale_offset);
static bool parse_label(int index, const char *fname, int line,
			const char *buf, int locale_offset);

/*
 * 初期化
 */

/*
 * 初期スクリプトを読み込む
 */
bool init_script(void)
{
	if (!load_script(INIT_FILE))
		return false;

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

	/* スクリプトファイルを読み込む */
	if (!read_script_from_file(fname)) {
#ifdef USE_DEBUGGER
		/* 最後の行まで読み込まれる */
#else
		return false;
#endif
	}

	/* コマンドが含まれない場合 */
	if (cmd_size == 0) {
		log_script_no_command(fname);
#ifdef USE_DEBUGGER
		return load_debug_script();
#else
		return false;
#endif
	}

	/* リターンポイントを無効にする */
	set_return_point(-1);

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
	for (i = 0; i < cmd_size; i++) {
		/* ラベルでないコマンドをスキップする */
		c = &cmd[i];
		if (c->type != COMMAND_LABEL)
			continue;

		/* ラベルがみつかった場合 */
		if (strcmp(c->param[LABEL_PARAM_LABEL], label) == 0) {
			cur_index = i;

#ifdef USE_DEBUGGER
			if (dbg_is_stop_requested())
				dbg_stop();
			update_debug_info(false);
#endif

			return true;
		}
	}

	/* エラーを出力する */
	log_script_label_not_found(label);
	log_script_exec_footer();
	return false;
}

/*
 * gosubによるリターンポイントを記録する(gosub用)
 */
void push_return_point(void)
{
	return_point = cur_index;
}

/*
 * gosubによるリターンポイントを取得する(return用)
 */
int pop_return_point(void)
{
	int rp;
	rp = return_point;
	return_point = -1;
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
 * 行の数を取得する
 */
int get_line_count(void)
{
	return script_lines;
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
static bool read_script_from_file(const char *fname)
{
	const int BUF_OFS = 4;
	char buf[LINE_BUF_SIZE];
	struct rfile *rf;
	int line;
	int top;
	bool result;

#ifdef USE_DEBUGGER
	error_count = 0;
#endif

	/* ファイルをオープンする */
	rf = open_rfile(SCRIPT_DIR, fname, false);
	if (rf == NULL)
		return false;

	/* 行ごとに処理する */
	cmd_size = 0;
	line = 0;
	result = true;
	while (result) {
#ifdef USE_DEBUGGER
		if (line > SCRIPT_LINE_SIZE) {
			log_script_line_size();
			result = false;
			break;
		}
#endif

		/* 行を読み込む */
		if (gets_rfile(rf, buf, sizeof(buf)) == NULL)
			break;

		/* 最大コマンド数をチェックする */
		if (line >= SCRIPT_CMD_SIZE) {
			log_script_size(SCRIPT_CMD_SIZE);
			result = false;
			break;
		}

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
		switch (buf[top]) {
		case '\0':
		case '#':
#ifdef USE_DEBUGGER
			/* コメントを保存する */
			comment_text[line] = strdup(buf);
			if (comment_text[line] == NULL) {
				log_memory();
				return false;
			}
#else
			/* 空行とコメント行を読み飛ばす */
#endif
			break;
		case '@':
			/* 命令行をパースする */
			if (!parse_insn(cmd_size, fname, line, buf, top)) {
#ifdef USE_DEBUGGER
				if (is_parse_error) {
					cmd_size++;
					is_parse_error = false;
				} else {
					result = false;
				}
#else
				result = false;
#endif
			} else {
				cmd_size++;
			}
			break;
		case '*':
			/* セリフ行をパースする */
			if (!parse_serif(cmd_size, fname, line, buf, top)) {
#ifdef USE_DEBUGGER
				if (is_parse_error) {
					cmd_size++;
					is_parse_error = false;
				} else {
					result = false;
				}
#else
				result = false;
#endif
			} else {
				cmd_size++;
			}
			break;
		case ':':
			/* ラベル行をパースする */
			if (!parse_label(cmd_size, fname, line, buf, top)) {
#ifdef USE_DEBUGGER
				if (is_parse_error) {
					cmd_size++;
					is_parse_error = false;
				} else {
					result = false;
				}
#else
				result = false;
#endif
			} else {
				cmd_size++;
			}
			break;
		default:
			/* メッセージ行をパースする */
			if (!parse_message(cmd_size, fname, line, buf, top)) {
#ifdef USE_DEBUGGER
				if (is_parse_error) {
					cmd_size++;
					is_parse_error = false;
				} else {
					result = false;
				}
#else
				result = false;
#endif
			} else {
				cmd_size++;
			}
			break;
		}
		line++;
	}

	script_lines = line;

	close_rfile(rf);

	return result;
}

/* 命令行をパースする */
static bool parse_insn(int index, const char *file, int line, const char *buf,
		       int locale_offset)
{
	struct command *c;
	char *tp;
	int i, min = 0, max = 0;

#ifdef USE_DEBUGGER
	UNUSED_PARAMETER(file);
#endif

	c = &cmd[index];

	/* 行番号とオリジナルの行を保存しておく */
	c->line = line;
	c->text = strdup(buf);
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
	strtok_escape(c->param[0]);

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

	/* 2番目以降のトークンを取得する */
	i = 1;
	while ((tp = strtok_escape(NULL))  != NULL && i < PARAM_SIZE) {
		if (strcmp(tp, "") == 0) {
			log_script_empty_string();
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
		c->param[i] = tp;
		i++;
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

	return true;
}

/* ダブルクォーテーションでエスケープ可能なトークナイズを実行する */
static char *strtok_escape(char *buf)
{
	static char *top = NULL;
	char *result;

	/* 初回呼び出しの場合バッファを保存する */
	if (buf != NULL)
		top = buf;
	assert(top != NULL);

	/* すでにバッファの終端に達している場合NULLを返す */
	if (*top == '\0')
		return NULL;

	/* 先頭のスペースをスキップする */
	for (; *top != '\0' && *top == ' '; top++)
		;
	if (*top == '\0')
		return NULL;

	/* エスケープされている場合 */
	if (*top == '\"') {
		result = ++top;
		for (; *top != '\0' && *top != '\"'; top++)
			;
		if (*top == '\"')
			*top++ = '\0';
		return result;
	}
	
	/* エスケープされていない場合 */
	result = top;
	for (; *top != '\0' && *top != ' '; top++)
		;
	if (*top == ' ')
		*top++ = '\0';
	return result;
}

/* セリフ行をパースする */
static bool parse_serif(int index, const char *file, int line, const char *buf,
			int locale_offset)
{
	char *first, *second, *third;

	assert(buf[locale_offset] == '*');

#ifdef USE_DEBUGGER
	UNUSED_PARAMETER(file);
#endif

	/* 行番号とオリジナルの行を保存しておく */
	cmd[index].type = COMMAND_SERIF;
	cmd[index].line = line;
	cmd[index].text = strdup(buf);
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
		if(error_count++ == 0)
			log_command_update_error();
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

	/* 成功 */
	return true;
}

/* メッセージ行をパースする */
static bool parse_message(int index, const char *file, int line,
			  const char *buf, int locale_offset)
{
	UNUSED_PARAMETER(file);

	/* 行番号とオリジナルの行(メッセージ全体)を保存しておく */
	cmd[index].type = COMMAND_MESSAGE;
	cmd[index].line = line;
	cmd[index].text = strdup(buf);
	if (cmd[index].text == NULL) {
		log_memory();
		return false;
	}
	cmd[index].param[MESSAGE_PARAM_MESSAGE] = strdup(buf + locale_offset);
	if (cmd[index].text == NULL) {
		log_memory();
		return false;
	}

	/* 成功 */
	return true;
}

/* ラベル行をパースする */
		static bool parse_label(int index, const char *file, int line, const char *buf,
					int locale_offset)
{
	UNUSED_PARAMETER(file);

	/* 行番号とオリジナルの行(メッセージ全体)を保存しておく */
	cmd[index].type = COMMAND_LABEL;
	cmd[index].line = line;
	cmd[index].text = strdup(buf);
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

	/* 成功 */
	return true;
}

#ifdef USE_DEBUGGER
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
		if (!parse_insn(index, cur_script, line, cmd_str, top))
			return false;
		return true;
	case '*':
		if (!parse_serif(index, cur_script, line, cmd_str, top))
			return false;
		return true;
	case ':':
		if (!parse_label(index, cur_script, line, cmd_str, top))
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
		if (!parse_message(index, cur_script, line, cmd_str, top))
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
