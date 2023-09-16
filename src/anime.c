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

#define INVALID_ACCEL_TYPE	(0)

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
	int accel;
};

/* アニメーションシーケンス(レイヤxシーケンス長) */
static struct sequence sequence[ANIME_LAYER_COUNT][SEQUENCE_COUNT];

/* レイヤごとのアニメーションの状況 */
struct layer_context {
	int seq_count;
	bool is_running;
	bool is_finished;
	stop_watch_t sw;
	float cur_lap;
};
static struct layer_context context[ANIME_LAYER_COUNT];

/* レイヤー名とレイヤーのインデックスのマップ */
struct layer_name_map {
	const char *name;
	int index;
};
static struct layer_name_map layer_name_map[] = {
	{"bg", ANIME_LAYER_BG}, {U8("背景"), ANIME_LAYER_BG},
	{"bg2", ANIME_LAYER_BG2},
	{"chb", ANIME_LAYER_CHB}, {U8("背面キャラ"), ANIME_LAYER_CHB},
	{"chl", ANIME_LAYER_CHL}, {U8("左キャラ"), ANIME_LAYER_CHL},
	{"chr", ANIME_LAYER_CHR}, {U8("右キャラ"), ANIME_LAYER_CHR},
	{"chc", ANIME_LAYER_CHC}, {U8("中央キャラ"), ANIME_LAYER_CHC},
	{"msg", ANIME_LAYER_MSG}, {U8("メッセージ"), ANIME_LAYER_MSG},
	{"name", ANIME_LAYER_NAME}, {U8("名前"), ANIME_LAYER_NAME},
	{"face", ANIME_LAYER_CHF}, {U8("顔"), ANIME_LAYER_CHF},
	{"text1", ANIME_LAYER_TEXT1},
	{"text2", ANIME_LAYER_TEXT2},
	{"text3", ANIME_LAYER_TEXT3},
	{"text4", ANIME_LAYER_TEXT4},
	{"text5", ANIME_LAYER_TEXT5},
	{"text6", ANIME_LAYER_TEXT6},
	{"text7", ANIME_LAYER_TEXT7},
	{"text8", ANIME_LAYER_TEXT8},
	{"effect1", ANIME_LAYER_EFFECT1},
	{"effect2", ANIME_LAYER_EFFECT2},
	{"effect3", ANIME_LAYER_EFFECT3},
	{"effect4", ANIME_LAYER_EFFECT4},
};

/* ロード中の情報 */
static int cur_seq_layer;
static stop_watch_t cur_sw;

/* 座標 */
static float anime_layer_x[ANIME_LAYER_COUNT];
static float anime_layer_y[ANIME_LAYER_COUNT];

/*
 * 前方参照
 */
static bool start_sequence(const char *name);
static bool on_key_value(const char *key, const char *val);
static int layer_name_to_index(const char *name);
static float calc_pos_x(int anime_layer, int index, const char *value);
static float calc_pos_y(int anime_layer, int index, const char *value);
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
	cur_seq_layer = -1;

	reset_stop_watch(&cur_sw);

	if (!load_anime_file(fname))
		return false;

	return true;
}

/*
 * アニメーションシーケンスをクリアする
 */
void clear_anime_sequence(int layer)
{
	assert(layer >= 0 && layer < ANIME_LAYER_COUNT);

	context[layer].seq_count = 0;
	context[layer].is_running = false;
	context[layer].is_finished = false;
}

/*
 * アニメーションシーケンスを開始する
 */
bool new_anime_sequence(int layer)
{
	assert(layer >= 0 && layer < ANIME_LAYER_COUNT);

	cur_seq_layer = layer;
	context[layer].seq_count++;
	
	return true;
}

/*
 * アニメーションシーケンスにプロパティを追加する(float)
 */
bool add_anime_sequence_property_f(const char *key, float val)
{
	char s[128];

	snprintf(s, sizeof(s), "%f", val);

	if (!on_key_value(key, s))
		return false;

	return true;
}

/*
 * アニメーションシーケンスにプロパティを追加する(int)
 */
bool add_anime_sequence_property_i(const char *key, int val)
{
	char s[128];

	snprintf(s, sizeof(s), "%d", val);

	if (!on_key_value(key, s))
		return false;

	return true;
}

/*
 * 指定したレイヤのアニメーションを開始する
 */
bool start_layer_anime(int layer)
{
	assert(layer >= 0 && layer < ANIME_LAYER_COUNT);

	if (context[layer].seq_count == 0)
		return true;

	reset_stop_watch(&context[layer].sw);
	context[layer].is_running = true;
	context[layer].is_finished = false;
	context[layer].cur_lap = 0;
	return true;
}

/*
 * 指定したレイヤのアニメーションを完了する
 */
bool finish_layer_anime(int layer)
{
	assert(layer >= 0 && layer < ANIME_LAYER_COUNT);

	if (!context[layer].is_running)
		return true;
	if (context[layer].is_finished)
		return true;

	context[layer].is_running = false;
	context[layer].is_finished = true;
	context[layer].cur_lap = 0;
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
 * アニメーションが完了したか調べる
 */
bool is_anime_finished_for_layer(int layer)
{
	assert(layer >= 0 && layer < ANIME_LAYER_COUNT);

	if (context[layer].is_finished)
		return true;

	return false;
}

/*
 * レイヤでアニメーションが実行中であるか調べる
 */
bool is_anime_running_for_layer(int layer)
{
	assert(layer >= 0 && layer < ANIME_LAYER_COUNT);

	if (context[layer].is_running)
		return true;

	return false;
}

/*
 * アニメーションのフレーム時刻を更新し、完了していればフラグをセットする
 */
void update_anime_frame(void)
{
	int i, last_seq;

	for (i = 0; i < ANIME_LAYER_COUNT; i++) {
		if (!context[i].is_running)
			continue;
		if (context[i].is_finished)
			continue;

		context[i].cur_lap =
			(float)get_stop_watch_lap(&context[i].sw) / 1000.0f;

		last_seq = context[i].seq_count - 1;
		assert(last_seq >= 0);

		if (context[i].cur_lap > sequence[i][last_seq].end_time) {
			context[i].is_running = false;
			context[i].is_finished = true;
		}
	}
}

/*
 * レイヤのパラメータを取得する
 */
bool
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
	const char *dir;
	int i;

	assert(layer >= 0 && layer < ANIME_LAYER_COUNT);

	/* シーケンスが定義されていない場合 */
	if (context[layer].seq_count == 0)
		return true;

	/* すでに完了している場合 */
	if (context[layer].is_finished) {
		s = &sequence[layer][context[layer].seq_count - 1];
		*x = (int)s->to_x;
		*y = (int)s->to_y;
		*alpha = (int)s->to_a;
		return true;
	}

	/* 補間を行う */
	for (i = 0; i < context[layer].seq_count; i++) {
		s = &sequence[layer][i];
		if (context[layer].cur_lap < s->start_time)
			continue;
		if (i != context[layer].seq_count - 1 &&
		    context[layer].cur_lap > s->end_time)
			continue;

		/* ファイル読み込みを行う */
		if (s->file != NULL) {
			if (layer == ANIME_LAYER_BG ||
			    layer == ANIME_LAYER_BG2)
				dir = BG_DIR;
			else if (layer >= ANIME_LAYER_CHB &&
				 layer <= ANIME_LAYER_CHC)
				dir = CH_DIR;
			else if (layer == ANIME_LAYER_MSG ||
				 layer == ANIME_LAYER_NAME)
				dir = CG_DIR;
			else if (layer == ANIME_LAYER_CHF)
				dir = CH_DIR;
			else if (layer >= ANIME_LAYER_TEXT1 &&
				 layer <= ANIME_LAYER_TEXT8)
				dir = CG_DIR;
			else if (layer >= ANIME_LAYER_EFFECT1 &&
				 layer <= ANIME_LAYER_EFFECT4)
				dir = CG_DIR;
			else
				dir = "";
			if (image != NULL && *image != NULL) {
				destroy_image(*image);
				*image = NULL;
			}
			if (image != NULL) {
				*image = create_image_from_file(dir, s->file);
				if (*image == NULL)
					return false;
			}
			if (file != NULL) {
				free(*file);
				*file = s->file;
			}
			s->file = NULL;
		}

		/* 進捗率を計算する */
		progress = (context[layer].cur_lap - s->start_time) /
			(s->end_time - s->start_time);
		if (progress > 1.0f)
			progress = 1.0f;

		/* 加速を処理する */
		switch (s->accel) {
		case ANIME_ACCEL_UNIFORM:
			break;
		case ANIME_ACCEL_ACCEL:
			progress = progress * progress;
			break;
		case ANIME_ACCEL_DEACCEL:
			progress = sqrtf(progress);
			break;
		default:
			assert(INVALID_ACCEL_TYPE);
			break;
		}

		/* パラメータを計算する */
		*x = (int)(s->from_x + (s->to_x - s->from_x) * progress);
		*y = (int)(s->from_y + (s->to_y - s->from_y) * progress);
		*alpha = (int)(s->from_a + (s->to_a - s->from_a) * progress);

		anime_layer_x[i] = (float)*x;
		anime_layer_y[i] = (float)*y;

		break;
	}

	return true;
}

/*
 * アニメレイヤの座標を更新する
 *  - アニメ以外の@bg, @ch, @chsを使ったときに設定する
 */
void set_anime_layer_position(int anime_layer, int x, int y)
{
	assert(anime_layer >= 0 && anime_layer < ANIME_LAYER_COUNT);

	anime_layer_x[anime_layer] = (float)x;
	anime_layer_y[anime_layer] = (float)y;
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
	int top;

	/* 最初にレイヤのキーが指定される必要がある */
	if (strcmp(key, "layer") == 0) {
		cur_seq_layer = layer_name_to_index(val);
		if (cur_seq_layer == -1)
			return false;
		context[cur_seq_layer].seq_count++;
		context[cur_seq_layer].sw = cur_sw;
		context[cur_seq_layer].is_running = true;
		context[cur_seq_layer].is_finished = false;
		return true;
	}
	if (cur_seq_layer == -1) {
		log_anime_layer_not_specified(key);
		return false;
	}

	/* レイヤのシーケンス長をチェックする */
	top = context[cur_seq_layer].seq_count - 1;
	if (top == SEQUENCE_COUNT) {
		log_anime_long_sequence();
		return false;
	}

	/* その他のキーのとき */
	s = &sequence[cur_seq_layer][top];
	if (strcmp(key, "clear") == 0) {
		context[cur_seq_layer].seq_count = 1;
	} else if (strcmp(key, "file") == 0) {
		s->file = strdup(val);
		if (s->file == NULL) {
			log_memory();
			return false;
		}
	} else if (strcmp(key, "start") == 0) {
		s->start_time = (float)atof(val);
	} else if (strcmp(key, "end") == 0) {
		s->end_time = (float)atof(val);
	} else if (strcmp(key, "from-x") == 0) {
		s->from_x = calc_pos_x(cur_seq_layer, top, val);
	} else if (strcmp(key, "from-y") == 0) {
		s->from_y = calc_pos_y(cur_seq_layer, top, val);
	} else if (strcmp(key, "from-a") == 0) {
		s->from_a = (float)atoi(val);
	} else if (strcmp(key, "to-x") == 0) {
		s->to_x = calc_pos_x(cur_seq_layer, top, val);
	} else if (strcmp(key, "to-y") == 0) {
		s->to_y = calc_pos_y(cur_seq_layer, top, val);
	} else if (strcmp(key, "to-a") == 0) {
		s->to_a = (float)atoi(val);
	} else if (strcmp(key, "accel") == 0) {
		s->accel = atoi(val);
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
			return layer_name_map[i].index;
	}
	return -1;
}

/* 座標を計算する */
static float calc_pos_x(int anime_layer, int index, const char *value)
{
	float ret;

	assert(value != NULL);

	if (value[0] == '+') {
		if (index == 0)
			ret = anime_layer_x[anime_layer];
		else
			ret = sequence[cur_seq_layer][index - 1].to_x;
		ret += (float)atoi(value + 1);
	} else if (value[0] == '-') {
		if (index == 0)
			ret = anime_layer_x[anime_layer];
		else
			ret = sequence[cur_seq_layer][index - 1].to_x;
		ret += (float)atoi(value);
	} else {
		ret = (float)atoi(value);
	}

	return ret;
}

/* 座標を計算する */
static float calc_pos_y(int anime_layer, int index, const char *value)
{
	float ret;

	assert(value != NULL);

	if (value[0] == '+') {
		if (index == 0)
			ret = anime_layer_y[anime_layer];
		else
			ret = sequence[cur_seq_layer][index - 1].to_y;
		ret += (float)atoi(value + 1);
	} else if (value[0] == '-') {
		if (index == 0)
			ret = anime_layer_y[anime_layer];
		else
			ret = sequence[cur_seq_layer][index - 1].to_y;
		ret += (float)atoi(value);
	} else {
		ret = (float)atoi(value);
	}

	return ret;
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
	int st, len, line;
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
					log_anime_parse_char(c);
					st = ST_ERROR;
					break;
				}
			}
			if (c == '}' || c == ':') {
				log_anime_parse_char(c);
				st = ST_ERROR;
				break;
			}
			if (c == ' ' || c == '\t' || c == '\r' || c == '\n' ||
			    c == '{') {
				assert(len > 0);
				word[len] = '\0';
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
				log_anime_parse_long_word();
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
			log_anime_parse_char(c);
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
					log_anime_parse_char(c);
					st = ST_ERROR;
					break;
				}
				if (c == '}') {
					st = ST_SCOPE;
					break;
				}
			}
			if (c == '{' || c == '}') {
				log_anime_parse_char(c);
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
				log_anime_parse_long_word();
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
			log_anime_parse_char(c);
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
				log_anime_parse_char(c);
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
				log_anime_parse_long_word();
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
				log_anime_parse_char(c);
				st = ST_ERROR;
				break;
			}
			if (len == sizeof(word) - 1) {
				log_anime_parse_long_word();
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
			log_anime_parse_char(c);
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
		log_anime_parse_footer(file, line);
	} else if (st != ST_SCOPE || len > 0) {
		log_anime_parse_invalid_eof();
	}

	/* バッファを解放する */
	free(buf);

	/* ファイルをクローズする */
	close_rfile(rf);

	return st != ST_ERROR;
}
