/* -*- Coding: utf-8; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Suika2
 * Copyright (C) 2001-2023, Keiichi Tabata. All rights reserved.
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
#define SEQUENCE_COUNT		(64)

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
	float from_scale_x;
	float from_scale_y;
	float to_x;
	float to_y;
	float to_a;
	float to_scale_x;
	float to_scale_y;
	float center_x;
	float center_y;
	float from_rotate;
	float to_rotate;
	int frame;	/* 目パチ用 */
	int blend;
	int accel;
	bool loop;
};

/* アニメーションシーケンス(レイヤxシーケンス長) */
static struct sequence sequence[STAGE_LAYERS][SEQUENCE_COUNT];

/* レイヤごとのアニメーションの状況 */
struct layer_context {
	int seq_count;
	bool is_running;
	bool is_finished;
	uint64_t sw;
	float cur_lap;
	uint64_t loop_len;
	uint64_t loop_ofs;
	char *file;
};
static struct layer_context context[STAGE_LAYERS];

/* レイヤー名とレイヤーのインデックスのマップ */
struct layer_name_map {
	const char *name;
	int index;
};
static struct layer_name_map layer_name_map[] = {
	{"bg", LAYER_BG},
	{U8("背景"), LAYER_BG},
	{"bg2", LAYER_BG2},
	{"effect5", LAYER_EFFECT5},
	{"effect6", LAYER_EFFECT6},
	{"effect7", LAYER_EFFECT7},
	{"effect8", LAYER_EFFECT8},
	{"chb", LAYER_CHB},
	{U8("背面キャラ"), LAYER_CHB},
	{"chl", LAYER_CHL},
	{U8("左キャラ"), LAYER_CHL},
	{"chlc", LAYER_CHL},
	{U8("左中キャラ"), LAYER_CHLC},
	{"chr", LAYER_CHR},
	{U8("右キャラ"), LAYER_CHR},
	{"chrc", LAYER_CHRC},
	{U8("右中キャラ"), LAYER_CHRC},
	{"chc", LAYER_CHC},
	{U8("中央キャラ"), LAYER_CHC},
	{"effect1", LAYER_EFFECT1},
	{"effect2", LAYER_EFFECT2},
	{"effect3", LAYER_EFFECT3},
	{"effect4", LAYER_EFFECT4},
	{"msg", LAYER_MSG},
	{U8("メッセージ"), LAYER_MSG},
	{"name", LAYER_NAME},
	{U8("名前"), LAYER_NAME},
	{"face", LAYER_CHF},
	{U8("顔"), LAYER_CHF},
	{"text1", LAYER_TEXT1},
	{"text2", LAYER_TEXT2},
	{"text3", LAYER_TEXT3},
	{"text4", LAYER_TEXT4},
	{"text5", LAYER_TEXT5},
	{"text6", LAYER_TEXT6},
	{"text7", LAYER_TEXT7},
	{"text8", LAYER_TEXT8},
};

/* Registered Anime Files */
static char *reg_anime_file[REG_ANIME_COUNT];

/* ロード中の情報 */
static int cur_seq_layer;
static uint64_t cur_sw;

/*
 * 前方参照
 */
static bool start_sequence(const char *name);
static bool on_key_value(const char *key, const char *val);
static int layer_name_to_index(const char *name);
static float calc_pos_x_from(int anime_layer, int index, const char *value);
static float calc_pos_x_to(int anime_layer, int index, const char *value);
static float calc_pos_y_from(int anime_layer, int index, const char *value);
static float calc_pos_y_to(int anime_layer, int index, const char *value);
static bool load_anime_file(const char *file);
static bool update_layer_params(int layer);
static bool load_anime_image(int layer);

/*
 * アニメーションサブシステムに関する初期化処理を行う
 */
bool init_anime(void)
{
#ifdef SUIKA_TARGET_ANDROID
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

	for (i = 0; i < STAGE_LAYERS; i++) {
		for (j = 0 ; j < SEQUENCE_COUNT; j++) {
			if (sequence[i][j].file != NULL) {
				free(sequence[i][j].file);
				sequence[i][j].file = NULL;
			}
		}
		memset(sequence, 0, sizeof(sequence));
		for (j = 0 ; j < SEQUENCE_COUNT; j++) {
			sequence[i][j].from_scale_x = 1.0f;
			sequence[i][j].from_scale_y = 1.0f;
			sequence[i][j].to_scale_x = 1.0f;
			sequence[i][j].to_scale_y = 1.0f;
		}
	}
	memset(context, 0, sizeof(context));

	for (i = 0; i < REG_ANIME_COUNT; i++) {
		if (reg_anime_file[i] != NULL) {
			free(reg_anime_file[i]);
			reg_anime_file[i] = NULL;
		}
	}
}

/*
 * アニメーションファイルを読み込む
 */
bool load_anime_from_file(const char *fname, int reg_index)
{
	/* Load the anime file. */
	cur_seq_layer = -1;
	reset_lap_timer(&cur_sw);
	if (!load_anime_file(fname))
		return false;

	/* Register an anime file for looping. */
	if (reg_index != -1) {
		if (reg_anime_file[reg_index] != NULL) {
			free(reg_anime_file[reg_index]);
			reg_anime_file[reg_index] = NULL;
		}
		reg_anime_file[reg_index] = strdup(fname);
		if (reg_anime_file[reg_index] == NULL) {
			log_memory();
			return false;
		}
	}

	return true;
}

/*
 * アニメーションシーケンスをクリアする
 */
void clear_layer_anime_sequence(int layer)
{
	int i;

	assert(layer >= 0 && layer < STAGE_LAYERS);

	context[layer].seq_count = 0;
	context[layer].is_running = false;
	context[layer].is_finished = false;
	context[layer].loop_len = 0;
	context[layer].loop_ofs = 0;
	if (context[layer].file != NULL) {
		free(context[layer].file);
		context[layer].file = NULL;
	}

	for (i = 0 ; i < SEQUENCE_COUNT; i++) {
		if (sequence[layer][i].file != NULL) {
			free(sequence[layer][i].file);
			sequence[layer][i].file = NULL;
		}
		memset(&sequence[layer][i], 0, sizeof(struct sequence));
		sequence[layer][i].from_scale_x = 1.0f;
		sequence[layer][i].from_scale_y = 1.0f;
		sequence[layer][i].to_scale_x = 1.0f;
		sequence[layer][i].to_scale_y = 1.0f;
	}
}

/*
 * アニメーションシーケンスをクリアする
 */
void clear_all_anime_sequence(void)
{
	int i, j;

	for (i = 0; i < STAGE_LAYERS; i++) {
		context[i].seq_count = 0;
		context[i].is_running = false;
		context[i].is_finished = false;
		context[i].loop_len = 0;
		context[i].loop_ofs = 0;
		if (context[i].file != NULL) {
			free(context[i].file);
			context[i].file = NULL;
		}

		for (j = 0 ; j < SEQUENCE_COUNT; j++) {
			if (sequence[i][j].file != NULL) {
				free(sequence[i][j].file);
				sequence[i][j].file = NULL;
			}
			memset(&sequence[i][j], 0, sizeof(struct sequence));
			sequence[i][j].from_scale_x = 1.0f;
			sequence[i][j].from_scale_y = 1.0f;
			sequence[i][j].to_scale_x = 1.0f;
			sequence[i][j].to_scale_y = 1.0f;
		}
	}
}

/*
 * アニメーションシーケンスを開始する
 */
bool new_anime_sequence(int layer)
{
	struct sequence *s;

	assert(layer >= 0 && layer < STAGE_LAYERS);

	cur_seq_layer = layer;

	s = &sequence[layer][0];
	s->from_scale_x = 1.0f;
	s->from_scale_y = 1.0f;
	s->to_scale_x = 1.0f;
	s->to_scale_y = 1.0f;

	context[layer].seq_count++;
	context[layer].is_running = false;
	context[layer].is_finished = false;

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
	assert(layer >= 0 && layer < STAGE_LAYERS);

	if (context[layer].seq_count == 0)
		return true;

	reset_lap_timer(&context[layer].sw);
	context[layer].is_running = true;
	context[layer].is_finished = false;
	context[layer].cur_lap = 0;
	context[layer].loop_ofs = 0;
	context[layer].loop_len =
		sequence[layer][context[layer].seq_count - 1].loop ?
		(uint64_t)(sequence[layer][context[layer].seq_count - 1].end_time * 1000.0f) :
		0;

	return true;
}

/*
 * 指定したレイヤのアニメーションを完了する
 */
bool finish_layer_anime(int layer)
{
	struct sequence *s;

	assert(layer >= 0 && layer < STAGE_LAYERS);

	if (!context[layer].is_running)
		return true;
	if (context[layer].is_finished)
		return true;
	if (context[layer].seq_count <= 0)
		return true;

	s = &sequence[layer][context[layer].seq_count - 1];
	set_layer_position(layer, (int)s->to_x, (int)s->to_y);
	set_layer_scale(layer, s->to_scale_x, s->to_scale_y);
	set_layer_alpha(layer, (int)s->to_a);

	context[layer].is_running = false;
	context[layer].is_finished = true;
	context[layer].cur_lap = 0;
	context[layer].loop_ofs = 0;
	context[layer].loop_len = 0;
	if (context[layer].file != NULL) {
		free(context[layer].file);
		context[layer].file = NULL;
	}

	return true;
}

/*
 * 実行中のアニメーションがあるか調べる
 */
bool is_anime_running(void)
{
	int i;

	for (i = 0; i < STAGE_LAYERS; i++) {
		if (context[i].is_running)
			return true;
	}
	return false;
}

/*
 * 指定したレイヤーのアニメーションが実行中であるか調べる
 */
bool is_anime_finished_for_layer(int layer)
{
	return context[layer].is_finished;
}

/*
 * アニメーションのフレーム時刻を更新し、完了したレイヤにフラグをセットする
 */
void update_anime_frame(void)
{
	int i, last_seq;
	uint64_t lap;

	/* 経過時刻を更新するとともに、アニメの完了を検出する */
	for (i = 0; i < STAGE_LAYERS; i++) {
		if (!context[i].is_running)
			continue;
		if (context[i].is_finished)
			continue;

		lap = get_lap_timer_millisec(&context[i].sw);
		if (context[i].loop_len > 0 &&
		    lap > context[i].loop_ofs + context[i].loop_len) {
			lap -= context[i].loop_ofs;
			lap %= context[i].loop_len;
			lap += context[i].loop_ofs;
		}
		context[i].cur_lap = (float)lap / 1000.0f;

		last_seq = context[i].seq_count - 1;
		assert(last_seq >= 0);

		if (context[i].loop_len == 0 &&
		    context[i].cur_lap >= sequence[i][last_seq].end_time) {
			context[i].is_running = false;
			context[i].is_finished = true;
		}
	}

	/*
	 * 各レイヤの描画パラメータを更新する
	 *  - アニメシーケンスの"file:"指定により画像とファイル名も変更される
	 */
	for (i = 0; i < STAGE_LAYERS; i++) {
		/* 下記3つのレイヤはアニメレイヤがない */
		if (i == LAYER_CLICK || i == LAYER_AUTO || i == LAYER_SKIP)
			continue;

		/* 更新の必要がない場合 */
		if (!context[i].is_finished && !context[i].is_running)
			continue;

		/* レイヤの情報を更新する */
		update_layer_params(i);
	}
}

/*
 * レイヤのパラメータを取得する
 */
static bool update_layer_params(int layer)
{
	struct sequence *s;
	float progress;
	int i, x, y, alpha;
	float scale_x, scale_y, rotate;

	assert(layer >= 0 && layer < STAGE_LAYERS);

	/* シーケンスが定義されていない場合 */
	if (context[layer].seq_count == 0)
		return true;

	/* 画像ファイル読み込みを行う */
	if (!load_anime_image(layer))
		return false;

	/* すでに完了している場合 */
	if (context[layer].is_finished &&
	    context[layer].seq_count > 0) {
		s = &sequence[layer][context[layer].seq_count - 1];
		x = (int)s->to_x;
		y = (int)s->to_y;
		alpha = (int)s->to_a;
		rotate = s->to_rotate;
		set_layer_position(layer, (int)s->to_x, (int)s->to_y);
		set_layer_scale(layer, s->to_scale_x, s->to_scale_y);
		set_layer_alpha(layer, alpha);
		set_layer_center(layer, (int)s->center_x, (int)s->center_y);
		set_layer_rotate(layer, rotate * (3.14159265f / 180.0f));
		set_layer_blend(layer, s->blend);
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
		x = (int)(s->from_x + (s->to_x - s->from_x) * progress);
		y = (int)(s->from_y + (s->to_y - s->from_y) * progress);
		scale_x = s->from_scale_x + (s->to_scale_x - s->from_scale_x) * progress;
		scale_y = s->from_scale_y + (s->to_scale_y - s->from_scale_y) * progress;
		rotate = s->from_rotate + (s->to_rotate - s->from_rotate) * progress;
		alpha = (int)(s->from_a + (s->to_a - s->from_a) * progress);
		set_layer_position(layer, x, y);
		set_layer_center(layer, (int)s->center_x, (int)s->center_y);
		set_layer_alpha(layer, alpha);
		set_layer_scale(layer, scale_x, scale_y);
		set_layer_rotate(layer, rotate * (3.14159265f / 180.0f));
		set_layer_blend(layer, s->blend);
		set_layer_frame(layer, s->frame);
		break;
	}

	return true;
}

/* ファイル読み込みを行う */
static bool load_anime_image(int layer)
{
	struct sequence *s = &sequence[layer][0];
	struct image *img;
	const char *dir;

	/* msgとnameはfile:指定で画像を変更できない */
	if (layer == LAYER_MSG || layer == LAYER_NAME)
		return true;

	/* 画像をロードする場合 */
	if (s->file != NULL && strcmp(s->file, "unload") != 0) {
		if (layer == LAYER_BG || layer == LAYER_BG2)
			dir = BG_DIR;
		else if (layer >= LAYER_CHB && layer <= LAYER_CHC)
			dir = CH_DIR;
		else if (layer == LAYER_CHF)
			dir = CH_DIR;
		else if (layer == LAYER_MSG || layer == LAYER_NAME)
			dir = CG_DIR;
		else if (layer >= LAYER_TEXT1 && layer <= LAYER_TEXT8)
			dir = CG_DIR;
		else if (layer >= LAYER_EFFECT1 && layer <= LAYER_EFFECT4)
			dir = CG_DIR;
		else if (layer >= LAYER_EFFECT5 && layer <= LAYER_EFFECT8)
			dir = CG_DIR;
		else
			dir = "";

		img = create_image_from_file(dir, s->file);
		if (img == NULL)
			return false;

		set_layer_image(layer, img);
		set_layer_file_name(layer, s->file);

		free(s->file);
		s->file = NULL;
		return true;
	}

	/* 画像をアンロードする場合 */
	if (s->file != NULL && strcmp(s->file, "unload") == 0) {
		set_layer_image(layer, NULL);
		set_layer_file_name(layer, NULL);
		free(s->file);
		s->file = NULL;
	}

	return true;
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
	int i;

	/* 最初にレイヤのキーが指定される必要がある */
	if (strcmp(key, "layer") == 0) {
		cur_seq_layer = layer_name_to_index(val);
		if (cur_seq_layer == -1) {
			log_invalid_layer_name(val);
			return false;
		}

		s = &sequence[cur_seq_layer][context[cur_seq_layer].seq_count];
		s->from_scale_x = 1.0f;
		s->from_scale_y = 1.0f;
		s->to_scale_x = 1.0f;
		s->to_scale_y = 1.0f;

		context[cur_seq_layer].seq_count++;
		context[cur_seq_layer].sw = cur_sw;
		context[cur_seq_layer].is_running = true;
		context[cur_seq_layer].is_finished = false;
		context[cur_seq_layer].loop_len = 0;
		context[cur_seq_layer].loop_ofs = 0;

		return true;
	}
	if (cur_seq_layer == -1) {
		log_anime_layer_not_specified(key);
		return false;
	}

	/* クリアが指定された場合 */
	if (strcmp(key, "clear") == 0) {
		context[cur_seq_layer].seq_count = 1;
		for (i = 0; i < SEQUENCE_COUNT; i++) {
			if (sequence[cur_seq_layer][i].file != NULL) {
				free(sequence[cur_seq_layer][i].file);
				sequence[cur_seq_layer][i].file = NULL;
			}
		}
		memset(&sequence[cur_seq_layer], 0, sizeof(struct sequence) * SEQUENCE_COUNT);
		for (i = 0; i < SEQUENCE_COUNT; i++) {
			sequence[cur_seq_layer][i].from_scale_x = 1.0f;
			sequence[cur_seq_layer][i].from_scale_y = 1.0f;
			sequence[cur_seq_layer][i].to_scale_x = 1.0f;
			sequence[cur_seq_layer][i].to_scale_y = 1.0f;
		}

		return true;
	}

	/* レイヤのシーケンス長をチェックする */
	top = context[cur_seq_layer].seq_count - 1;
	if (top == SEQUENCE_COUNT) {
		log_anime_long_sequence();
		return false;
	}
	assert(top >= 0);
	assert(top < SEQUENCE_COUNT);

	/* その他のキーのとき */
	s = &sequence[cur_seq_layer][top];
	if (strcmp(key, "file") == 0) {
		log_warn("\"file:\" is deprecated. Use @layer.");
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
		s->from_x = calc_pos_x_from(cur_seq_layer, top, val);
	} else if (strcmp(key, "from-y") == 0) {
		s->from_y = calc_pos_y_from(cur_seq_layer, top, val);
	} else if (strcmp(key, "from-a") == 0) {
		s->from_a = (float)atoi(val);
	} else if (strcmp(key, "from-scale-x") == 0) {
		s->from_scale_x = (float)atof(val);
	} else if (strcmp(key, "from-scale-y") == 0) {
		s->from_scale_y = (float)atof(val);
	} else if (strcmp(key, "to-x") == 0) {
		s->to_x = calc_pos_x_to(cur_seq_layer, top, val);
	} else if (strcmp(key, "to-y") == 0) {
		s->to_y = calc_pos_y_to(cur_seq_layer, top, val);
	} else if (strcmp(key, "to-a") == 0) {
		s->to_a = (float)atoi(val);
	} else if (strcmp(key, "to-scale-x") == 0) {
		s->to_scale_x = (float)atof(val);
	} else if (strcmp(key, "to-scale-y") == 0) {
		s->to_scale_y = (float)atof(val);
	} else if (strcmp(key, "from-rotate") == 0) {
		s->from_rotate = (float)atof(val);
	} else if (strcmp(key, "to-rotate") == 0) {
		s->to_rotate = (float)atof(val);
	} else if (strcmp(key, "center-x") == 0) {
		s->center_x = (float)atof(val);
	} else if (strcmp(key, "center-y") == 0) {
		s->center_y = (float)atof(val);
	} else if (strcmp(key, "blend") == 0) {
		s->blend = atoi(val);
	} else if (strcmp(key, "accel") == 0) {
		s->accel = atoi(val);
	} else if (strcmp(key, "loop") == 0) {
		s->loop = true;
		context[cur_seq_layer].loop_ofs = (uint64_t)(atof(val) * 1000.0f);
		context[cur_seq_layer].loop_len = (uint64_t)(s->end_time * 1000.0f) - context[cur_seq_layer].loop_ofs;
	} else if (strcmp(key, "file") == 0) {
		s->file = strdup(val);
		if (s->file == NULL) {
			log_memory();
			return false;
		}
	} else if (strcmp(key, "frame") == 0) {
		s->frame = atoi(val);
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
static float calc_pos_x_from(int layer, int index, const char *value)
{
	float ret;

	assert(value != NULL);

	if (value[0] == '+') {
		if (index == 0)
			ret = (float)get_layer_x(layer);
		else
			ret = sequence[layer][index - 1].to_x;
		ret += (float)atoi(value + 1);
	} else {
		ret = (float)atoi(value);
	}

	return ret;
}

/* 座標を計算する */
static float calc_pos_x_to(int layer, int index, const char *value)
{
	float ret;

	assert(value != NULL);

	if (value[0] == '+')
		ret = sequence[layer][index].from_x + (float)atoi(value + 1);
	else
		ret = (float)atoi(value);

	return ret;
}

/* 座標を計算する */
static float calc_pos_y_from(int anime_layer, int index, const char *value)
{
	float ret;

	assert(value != NULL);

	if (value[0] == '+') {
		if (index == 0)
			ret = (float)get_layer_y(anime_layer);
		else
			ret = sequence[anime_layer][index - 1].to_y;
		ret += (float)atoi(value + 1);
	} else {
		ret = (float)atoi(value);
	}

	return ret;
}

/* 座標を計算する */
static float calc_pos_y_to(int anime_layer, int index, const char *value)
{
	float ret;

	assert(value != NULL);

	if (value[0] == '+')
		ret = sequence[anime_layer][index].from_y + (float)atoi(value + 1);
	else
		ret = (float)atoi(value);

	return ret;
}

/*
 * ループアニメの登録を解除する
 */
void unregister_anime(int reg_index)
{
	free(reg_anime_file[reg_index]);
	reg_anime_file[reg_index] = NULL;
}

/*
 * ループアニメファイル名を取得する
 */
const char *get_reg_anime_file_name(int reg_index)
{
	return reg_anime_file[reg_index];
}

/*
 * 目パチ
 */

/*
 * 目パチ画像をロードする
 */
bool load_eye_image_if_exists(int chpos, const char *fname)
{
	char eye_fname[1024], ext[128], *dot;
	struct image *eye_img;
	float base_time;
	int base_layer, eye_layer, x, y, frame_count, i;

	/* まず目のレイヤを無効にする */
	eye_layer = chpos_to_eye_layer(chpos);
	set_layer_file_name(eye_layer, NULL);
	set_layer_image(eye_layer, NULL);

	/* キャラがない場合 */
	if (fname == NULL || strcmp(fname, "none") == 0 || strcmp(fname, U8("消去")) == 0) {
		/* 既存の目パチアニメを終了する */
		clear_layer_anime_sequence(eye_layer);
		return true;
	}

	/* 目パチファイル名の文字列"filename_eye.ext"を作る */
	strcpy(eye_fname, fname);
	dot = strstr(eye_fname, ".");
	if (dot != NULL)
		strcpy(ext, dot);
	else
		strcpy(ext, "");
	strcpy(dot, "_eye");
	strcat(eye_fname, ext);

	/* ファイルがない場合 */
	if (!check_file_exist(CH_DIR, eye_fname))
		return true;

	/* イメージを読み込む */
	eye_img = create_image_from_file(CH_DIR, eye_fname);
	if (eye_img == NULL) {
		log_script_exec_footer();
		return false;
	}

	/* キャラの座標を取得する */
	base_layer = chpos_to_layer(chpos);
	x = get_layer_x(base_layer);
	y = get_layer_y(base_layer);

	/* レイヤを設定する */
	set_layer_file_name(eye_layer, eye_fname);
	set_layer_image(eye_layer, eye_img);
	set_layer_position(eye_layer, x, y);
	set_layer_alpha(eye_layer, 0);
	set_layer_scale(eye_layer, 1.0f, 1.0f);

	/* 目パチのアニメを開始する */
	frame_count = get_layer_image(eye_layer)->width / get_layer_image(base_layer)->width;
	base_time = conf_character_eyeblink_interval + conf_character_eyeblink_interval * 0.3f * (2.0f * (float)rand() / (float)RAND_MAX - 1.0f);
	clear_layer_anime_sequence(eye_layer);
	new_anime_sequence(eye_layer);
	add_anime_sequence_property_f("start",	0);
	add_anime_sequence_property_f("end",	base_time);
	add_anime_sequence_property_i("from-x",	x);
	add_anime_sequence_property_i("from-y",	y);
	add_anime_sequence_property_i("to-x",	x);
	add_anime_sequence_property_i("to-y",	y);
	for (i = 0; i < frame_count; i++) {
		new_anime_sequence(eye_layer);
		add_anime_sequence_property_f("start",	base_time + (conf_character_eyeblink_frame * (float)i));
		add_anime_sequence_property_f("end",	base_time + (conf_character_eyeblink_frame * (float)(i + 1)));
		add_anime_sequence_property_i("from-x",	x);
		add_anime_sequence_property_i("from-y",	y);
		add_anime_sequence_property_i("from-a",	255);
		add_anime_sequence_property_i("to-x",	x);
		add_anime_sequence_property_i("to-y",	y);
		add_anime_sequence_property_i("to-a",	255);
		add_anime_sequence_property_i("frame",	i);
	}
	add_anime_sequence_property_i("loop", 0);

	/* アニメを開始する */
	start_layer_anime(eye_layer);

	return true;
}

/*
 * アニメファイルのパース
 */

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
