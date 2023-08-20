/* -*- Coding: utf-8; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Suika 2
 * Copyright (C) 2001-2023, TABATA Keiichi. All rights reserved.
 */

/*
 * Animation Subsystem
 *
 * [Changes]
 *  - 2023/08/20 作成
 */

#include "suika.h"

/* レイヤー名とレイヤーのインデックスのマップ */
struct layer_name_map {
	const char *name;
	int index;
};
static struct layer_name_map layer_name_map[] = {
	{"bg", ANIME_LAYER_BG}, {U8("背景"), ANIME_LAYER_BG},
	{"chb", ANIME_LAYER_CHB}, {U8("背面キャラ"), ANIME_LAYER_CHB},
	{"chl", ANIME_LAYER_CHL}, {U8("左キャラ"), ANIME_LAYER_CHL},
	{"chr", ANIME_LAYER_CHR}, {U8("右キャラ"), ANIME_LAYER_CHR},
	{"chc", ANIME_LAYER_CHC}, {U8("中央キャラ"), ANIME_LAYER_CHC},
	{"msg", ANIME_LAYER_MSG}, {U8("メッセージ"), ANIME_LAYER_MSG},
	{"name", ANIME_LAYER_NAME}, {U8("名前"), ANIME_LAYER_NAME},
	{"face", ANIME_LAYER_CHF}, {U8("顔"), ANIME_LAYER_CHF},
};

/* レイヤごとのアニメーションシーケンスの最大数 */
#define SEQUENCE_COUNT		(16)

/* アニメーションシーケンスの構造 */
struct sequence {
	int layer;
	bool clear;
	char *file;
	float start_time;
	float end_time;
	float from_x;
	float from_y;
	float from_a;
	float to_x;
	float to_y;
	float to_a;
};

/* アニメーションシーケンス(レイヤxシーケンス長) */
static struct sequence sequence[ANIME_LAYER_COUNT][SEQUENCE_COUNT];

/* レイヤごとのアニメーションの状況 */
struct layer_context {
	bool is_running;
	float start_time;
	int seq_count;
};
static struct layer_context context[ANIME_LAYER_COUNT];

/* ロード中の情報 */
static int cur_seq_layer;

/* 現在の時刻 */
static stop_watch_t cur_time;

/*
 * 前方参照
 */
static bool start_sequence(const char *name);
static bool on_key_value(const char *key, const char *val);
static int layer_name_to_index(const char *name);
static bool load_anime_file(const char *file);

/*
 * アニメーションサブシステムに関する初期化処理を行う
 */
bool init_anime(void)
{
#ifdef ANDROID
	/* Android NDK用に再初期化する */
	cleanup_anime();
#endif
	return true;
}

/*
 * アニメーションサブシステムに関する終了処理を行う
 */
void cleanup_anime(void)
{
	int i, j;

	for (i = 0; i < ANIME_LAYER_COUNT; i++) {
		for (j = 0 ; j < SEQUENCE_COUNT; j++) {
			if (sequence[i][j].file != NULL) {
				free(sequence[i][j].file);
				sequence[i][j].file = NULL;
			}
		}
	}
	memset(sequence, 0, sizeof(sequence));
	memset(context, 0, sizeof(context));
}

/*
 * アニメーションファイルを読み込む
 */
bool load_anime_from_file(const char *fname)
{
	if (!load_anime_file(fname))
		return false;
	return true;
}

/*
 * アニメーションシーケンスを開始する
 */
bool start_anime_sequence(void)
{
	if (!start_sequence(NULL))
		return false;
	return true;
}

/*
 * アニメーションシーケンスにプロパティを追加する
 */
bool add_anime_sequence_property(const char *key, float val)
{
	char s[128];

	snprintf(s, sizeof(s), "%f", val);

	if (!on_key_value(key, s))
		return false;

	return true;
}

/*
 * 指定したレイヤのアニメーションを開始する
 */
bool start_layer_anime(const char *layer)
{
	int layer_index;

	layer_index = layer_name_to_index(layer);
	assert(layer_index != -1);

	/* TODO */
	return true;
}

/*
 * 実行中のアニメーションがあるか調べる
 */
bool is_anime_running(void)
{
	int i;

	for (i = 0; i < ANIME_LAYER_COUNT; i++) {
		if (context[i].is_running)
			return true;
	}
	return false;
}

/*
 * アニメーションのフレームを更新する
 */
void update_anime_frame(void)
{
}

/*
 * レイヤのパラメータを取得する
 */
void
get_anime_layer_params(
	int layer,
	struct image **image,
	char **file,
	int *x,
	int *y,
	int *alpha)
{
	struct sequence *s;
	float progress;
	int i;

	assert(layer >= 0 && layer < ANIME_LAYER_COUNT);

	/* TODO */
	UNUSED_PARAMETER(image);
	UNUSED_PARAMETER(file);

	/* シーケンスが定義されていない場合 */
	if (context[layer].seq_count == 0)
		return;

	/* 補間を行う */
	for (i = 0; i < context[layer].seq_count; i++) {
		s = &sequence[layer][i];
		if (s->start_time < cur_time)
			continue;
		if (s->end_time > cur_time)
			continue;

		progress = (float)cur_time / 1000.0f - s->start_time;
		*x = (int)(s->from_x + (s->to_x - s->from_x) * progress);
		*y = (int)(s->from_y + (s->to_y - s->from_y) * progress);
		*alpha = (int)(s->from_a + (s->to_a - s->from_a) * progress);
	}
}

/*
 * アニメーションファイルの読み込み
 */

/* シーケンス(ブロック)が開始されたときに呼び出される */
static bool start_sequence(const char *name)
{
	UNUSED_PARAMETER(name);
	cur_seq_layer = -1;
	return true;
}

/* キーバリューペアが出現したときに呼び出される */
static bool on_key_value(const char *key, const char *val)
{
	struct sequence *s;
	int *top;

	/* 最初にレイヤのキーが指定される必要がある */
	if (strcmp(key, "layer") == 0) {
		cur_seq_layer = layer_name_to_index(val);
		if (cur_seq_layer == -1)
			return false;
		return true;
	}
	if (cur_seq_layer == -1) {
		log_anime_layer_not_specified(key);
		return false;
	}

	/* レイヤのシーケンス長をチェックする */
	top = &context[cur_seq_layer].seq_count;
	if (*top == SEQUENCE_COUNT) {
		log_anime_long_sequence();
		return false;
	}

	/* その他のキーのとき */
	s = &sequence[cur_seq_layer][*top];
	if (strcmp(key, "start") == 0) {
		s->start_time = (float)atof(val);
	} else if (strcmp(key, "end") == 0) {
		s->end_time = (float)atof(val);
	} else if (strcmp(key, "from-x") == 0) {
		s->from_x = (float)atoi(val);
	} else if (strcmp(key, "from-y") == 0) {
		s->from_y = (float)atoi(val);
	} else if (strcmp(key, "from-a") == 0) {
		s->from_a = (float)atoi(val);
	} else if (strcmp(key, "to-x") == 0) {
		s->to_x = (float)atoi(val);
	} else if (strcmp(key, "to-y") == 0) {
		s->to_y = (float)atoi(val);
	} else if (strcmp(key, "to-a") == 0) {
		s->to_a = (float)atoi(val);
	} else if (strcmp(key, "file") == 0) {
		s->file = strdup(val);
		if (s->file == NULL) {
			log_memory();
			return false;
		}
	} else {
		log_anime_unknown_key(key);
		return false;
	}

	(*top)++;

	return true;
}

/* レイヤ名をレイヤインデックスに変換する */
static int layer_name_to_index(const char *name)
{
	int i;

	for (i = 0;
	     i < (int)(sizeof(layer_name_map) / sizeof(struct layer_name_map));
	     i++) {
		if (strcmp(layer_name_map[i].name, name) == 0)
			return i;
	}
	return -1;
}

/* アニメーションファイルをロードする */
static bool load_anime_file(const char *file)
{
	enum {
		ST_SCOPE,
		ST_OPEN,
		ST_KEY,
		ST_COLON,
		ST_VALUE,
		ST_VALUE_DQ,
		ST_SEMICOLON,
		ST_ERROR
	};

	char word[256], key[256];
	struct rfile *rf;
	char *buf;
	size_t fsize, pos;
	int st, len, line, seq;
	char c;
	bool is_comment;

	assert(file != NULL);

	/* ファイルをオープンする */
	rf = open_rfile(ANIME_DIR, file, false);
	if (rf == NULL)
		return false;

	/* ファイルサイズを取得する */
	fsize = get_rfile_size(rf);

	/* メモリを確保する */
	buf = malloc(fsize);
	if (buf == NULL) {
		log_memory();
		close_rfile(rf);
		return false;
	}

	/* ファイルを読み込む */
	if (read_rfile(rf, buf, fsize) < fsize) {
		log_file_read(GUI_DIR, file);
		close_rfile(rf);
		free(buf);
		return false;
	}

	/* コメントをスペースに変換する */
	is_comment = false;
	for (pos = 0; pos < fsize; pos++) {
		if (!is_comment) {
			if (buf[pos] == '#') {
				buf[pos] = ' ';
				is_comment = true;
			}
		} else {
			if (buf[pos] == '\n')
				is_comment = false;
			else
				buf[pos] = ' ';
		}
	}

	/* ファイルをパースする */
	st = ST_SCOPE;
	line = 0;
	len = 0;
	seq = -1;
	pos = 0;
	while (pos < fsize) {
		/* 1文字読み込む */
		c = buf[pos++];

		/* ステートに応じて解釈する */
		switch (st) {
		case ST_SCOPE:
			if (len == 0) {
				if (c == ' ' || c == '\t' || c == '\r' ||
				    c == '\n') {
					st = ST_SCOPE;
					break;
				}
				if (c == ':' || c == '{' || c == '}') {
					log_gui_parse_char(c);
					st = ST_ERROR;
					break;
				}
			}
			if (c == '}' || c == ':') {
				log_gui_parse_char(c);
				st = ST_ERROR;
				break;
			}
			if (c == ' ' || c == '\t' || c == '\r' || c == '\n' ||
			    c == '{') {
				assert(len > 0);
				word[len] = '\0';
				seq++;
				if (!start_sequence(word)) {
					st = ST_ERROR;
					break;
				}
				if (c == '{')
					st = ST_KEY;
				else
					st = ST_OPEN;
				len = 0;
				break;
			}
			if (len == sizeof(word) - 1) {
				log_gui_parse_long_word();
				st = ST_ERROR;
				break;
			}
			word[len++] = c;
			st = ST_SCOPE;
			break;
		case ST_OPEN:
			if (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
				st = ST_OPEN;
				break;
			}
			if (c == '{') {
				st = ST_KEY;
				len = 0;
				break;
			}
			log_gui_parse_char(c);
			st = ST_ERROR;
			break;
		case ST_KEY:
			if (len == 0) {
				if (c == ' ' || c == '\t' || c == '\r' ||
				    c == '\n') {
					st = ST_KEY;
					break;
				}
				if (c == ':') {
					log_gui_parse_char(c);
					st = ST_ERROR;
					break;
				}
				if (c == '}') {
					st = ST_SCOPE;
					break;
				}
			}
			if (c == '{' || c == '}') {
				log_gui_parse_char(c);
				st = ST_ERROR;
				break;
			}
			if (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
				word[len] = '\0';
				strcpy(key, word);
				st = ST_COLON;
				len = 0;
				break;
			}
			if (c == ':') {
				word[len] = '\0';
				strcpy(key, word);
				st = ST_VALUE;
				len = 0;
				break;
			}
			if (len == sizeof(word) - 1) {
				log_gui_parse_long_word();
				st = ST_ERROR;
				break;
			}
			word[len++] = c;
			st = ST_KEY;
			break;
		case ST_COLON:
			if (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
				st = ST_COLON;
				break;
			}
			if (c == ':') {
				st = ST_VALUE;
				len = 0;
				break;
			}
			log_gui_parse_char(c);
			st = ST_ERROR;
			break;
		case ST_VALUE:
			if (len == 0) {
				if (c == ' ' || c == '\t' || c == '\r' ||
				    c == '\n') {
					st = ST_VALUE;
					break;
				}
				if (c == '\"') {
					st = ST_VALUE_DQ;
					break;
				}
			}
			if (c == ':' || c == '{') {
				log_gui_parse_char(c);
				st = ST_ERROR;
				break;
			}
			if (c == ' ' || c == '\t' || c == '\r' || c == '\n' ||
			    c == ';' || c == '}') {
				word[len] = '\0';
				if (!on_key_value(key, word)) {
					st = ST_ERROR;
					break;
				}
				if (c == ';')
					st = ST_KEY;
				else if (c == '}')
					st = ST_SCOPE;
				else
					st = ST_SEMICOLON;
				len = 0;
				break;
			}
			if (len == sizeof(word) - 1) {
				log_gui_parse_long_word();
				st = ST_ERROR;
				break;
			}
			word[len++] = c;
			st = ST_VALUE;
			break;
		case ST_VALUE_DQ:
			if (c == '\"') {
				word[len] = '\0';
				if (!on_key_value(key, word)) {
					st = ST_ERROR;
					break;
				}
				st = ST_SEMICOLON;
				len = 0;
				break;
			}
			if (c == '\r' || c == '\n') {
				log_gui_parse_char(c);
				st = ST_ERROR;
				break;
			}
			if (len == sizeof(word) - 1) {
				log_gui_parse_long_word();
				st = ST_ERROR;
				break;
			}
			word[len++] = c;
			st = ST_VALUE_DQ;
			break;
		case ST_SEMICOLON:
			if (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
				st = ST_SEMICOLON;
				break;
			}
			if (c == ';') {
				st = ST_KEY;
				len = 0;
				break;
			}
			if (c == '}') {
				st = ST_SCOPE;
				len = 0;
				break;
			}
			log_gui_parse_char(c);
			st = ST_ERROR;
			break;
		}

		/* エラー時 */
		if (st == ST_ERROR)
			break;

		/* FIXME: '\r'のみの改行を処理する */
		if (c == '\n')
			line++;
	}

	/* エラーが発生した場合 */
	if (st == ST_ERROR) {
		log_gui_parse_footer(file, line);
	} else if (st != ST_SCOPE || len > 0) {
		log_gui_parse_invalid_eof();
	}

	/* バッファを解放する */
	free(buf);

	/* ファイルをクローズする */
	close_rfile(rf);

	return st != ST_ERROR;
}
