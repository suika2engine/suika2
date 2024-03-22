/* -*- coding: utf-8; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Suika2
 * Copyright (C) 2001-2024, Keiichi Tabata. All rights reserved.
 */

/*
 * The Ciel Direction System
 *
 * [Changes]
 *  - 2024/03/17 Created.
 */

#include "suika.h"

/* False Assertions */
#define NOT_IMPLEMENTED		0
#define NEVER_COME_HERE		0

/* Parameter Check */
#define IS_EMPTY(s)	(strlen((s)) == 0)

/* Layers */
#define CL_CH0		CH_BACK
#define CL_CH1		CH_LEFT
#define CL_CH2		CH_LEFT_CENTER
#define CL_CH3		CH_RIGHT
#define CL_CH4		CH_RIGHT_CENTER
#define CL_CH5		CH_CENTER
#define CL_CHARACTERS	CH_CENTER
#define CL_BG		(CL_CHARACTERS + 1)
#define CL_LAYERS	(CL_BG + 1)

/* Anime Sequece Size */
#define CL_SEQ_SIZE	32

/* init func for each op. */
static bool init_cl_enter(bool *cont);
static bool init_cl_leave(bool *cont);
static bool init_cl_file(bool *cont);
static bool init_cl_pos(bool *cont);
static bool init_cl_alpha(bool *cont);
static bool init_cl_dim(bool *cont);
static bool init_cl_time(bool *cont);
static bool init_cl_effect(bool *cont);
static bool init_cl_move(bool *cont);
static bool init_cl_run(bool *cont);

/*
 * Map from op name to init func.
 */

struct op_func {
	const char *op;
	bool (*init)(bool *);
};

static struct op_func op_func_tbl[] = {
	{"@cl.enter", init_cl_enter},
	{"@cl.leave", init_cl_leave},
	{"@cl.file", init_cl_file},
	{"@cl.pos", init_cl_pos},
	{"@cl.alpha", init_cl_alpha},
	{"@cl.dim", init_cl_dim},
	{"@cl.time", init_cl_time},
	{"@cl.effect", init_cl_effect},
	{"@cl.move", init_cl_move},
	{"@cl.run", init_cl_run},
};

#define OP_COUNT	((int)(sizeof(op_func_tbl) / sizeof(struct op_func)))

/*
 * Temporary Stage
 */

struct temporary_stage {
	/* Character Count */
	int ch_count;

	/* Character Names */
	char *name[CL_CHARACTERS];

	/* Sanity Check */
	bool is_modified;

	/* Files */
	char *file[CL_LAYERS];
	bool is_file_changed[CL_LAYERS];
	struct image *img[CL_LAYERS];

	/* Layer Parameters */
	int x[CL_LAYERS];
	int y[CL_LAYERS];
	int a[CL_LAYERS];
	bool dim[CL_CHARACTERS];

	/* Fading Effect */
	int effect;
	struct image *rule_img;

	/* Duration */
	float time;

	/* Anime */
	bool use_anime;
	int anime_seq_count[CL_LAYERS];
	float anime_time[CL_LAYERS][CL_SEQ_SIZE];
	int anime_x_orig[CL_LAYERS];
	int anime_y_orig[CL_LAYERS];
	int anime_a_orig[CL_LAYERS];
	int anime_x[CL_LAYERS][CL_SEQ_SIZE];
	int anime_y[CL_LAYERS][CL_SEQ_SIZE];
	int anime_a[CL_LAYERS][CL_SEQ_SIZE];
};

static struct temporary_stage ts;

/*
 * Rendering Lap
 */
static uint64_t sw;

/*
 * Forward Declarations
 */
static bool init(bool *cont);
static bool init_cl_run_fade(void);
static bool init_cl_run_anime(void);
static int cl_layer_to_stage_layer(int cl_layer);
static void render(void);
static void render_fade_frame(void);
static void render_anime_frame(void);
static void process_anime_finish(void);
static int get_index_for_name(bool allow_bg);

/*
 * The implementation of the @cl.* commands.
 */
bool ciel_command(bool *cont)
{
	if (!is_in_command_repetition())
		if (!init(cont))
			return false;

	if (is_in_command_repetition())
		render();

	if (!is_in_command_repetition())
		if (!move_to_next_command())
			return false;

	return true;
}

/*
 * Command Initialization
 */

static bool init(bool *cont)
{
	const char *action;
	int i;

	action = get_string_param(CIEL_PARAM_ACTION);

	for (i = 0; i < OP_COUNT; i++) {
		if (strcmp(action, op_func_tbl[i].op) == 0) {
			if (!op_func_tbl[i].init(cont))
				return false;
			return true;
		}
	}

	log_error("%s is not supported.", action);
	log_script_exec_footer();

	return false;
}

static bool init_cl_enter(bool *cont)
{
	const char *name;
	int index;

	name = get_string_param(CIEL_PARAM_NAME);
	if (IS_EMPTY(name)) {
		log_error("name= is required");
		log_script_exec_footer();
		return false;
	}

	if (ts.ch_count >= CL_CHARACTERS) {
		log_error("Too many characters.");
		log_script_exec_footer();
		return false;
	}

	index = ts.ch_count++;
	ts.name[index] = strdup(name);
	if (ts.name[index] == NULL) {
		log_memory();
		return false;
	}
	ts.is_modified = true;
	ts.x[index] = 0;
	ts.y[index] = 0;
	ts.a[index] = 0;

	*cont = true;

	return true;
}

static bool init_cl_leave(bool *cont)
{
	int index;

	index = get_index_for_name(false);
	if (index == -1)
		return false;

	free(ts.name[index]);
	ts.name[index] = NULL;
	if (ts.file[index] != NULL) {
		free(ts.name[index]);
		ts.name[index] = NULL;
	}
	if (ts.img[index] != NULL) {
		destroy_image(ts.img[index]);
		ts.img[index] = NULL;
	}
	ts.is_modified = true;
	ts.is_file_changed[index] = true;
	ts.x[index] = 0;
	ts.y[index] = 0;
	ts.a[index] = 0;

	ts.ch_count--;
	assert(ts.ch_count >= 0);

	*cont = true;

	return true;
}

static bool init_cl_file(bool *cont)
{
	const char *file;
	int index;

	index = get_index_for_name(true);
	if (index == -1)
		return false;

	file = get_string_param(CIEL_PARAM_FILE);
	if (IS_EMPTY(file)) {
		log_error("file= is required");
		log_script_exec_footer();
		return false;
	}
	file = expand_variable(file);

	ts.is_modified = true;
	ts.is_file_changed[index] = true;
	if (ts.file[index] != NULL) {
		free(ts.file[index]);
		ts.file[index] = NULL;
	}
	ts.file[index] = strdup(file);
	if (IS_EMPTY(ts.file[index])) {
		log_memory();
		return false;
	}
	if (strcmp(file, "none") != 0) {
		ts.img[index] = create_image_from_file(index != CL_BG ? CH_DIR : BG_DIR, file);
		if (ts.img[index] == NULL)
			return false;
	}
	ts.a[index] = 255;

	*cont = true;

	return true;
}

static bool init_cl_pos(bool *cont)
{
	const char *align, *valign, *xequal, *xplus, *xminus, *yequal, *yplus, *yminus;
	int index;

	index = get_index_for_name(true);
	if (index == -1)
		return false;
	align = get_string_param(CIEL_PARAM_ALIGN);
	valign = get_string_param(CIEL_PARAM_VALIGN);
	xequal = get_string_param(CIEL_PARAM_XEQUAL);
	xplus = get_string_param(CIEL_PARAM_XPLUS);
	xminus = get_string_param(CIEL_PARAM_XMINUS);
	yequal = get_string_param(CIEL_PARAM_YEQUAL);
	yplus = get_string_param(CIEL_PARAM_YPLUS);
	yminus = get_string_param(CIEL_PARAM_YMINUS);

	ts.is_modified = true;
	if (!IS_EMPTY(align)) {
		if (strcmp(align, "left") == 0) {
			ts.x[index] = 0;
		} else if (strcmp(align, "right") == 0) {
			ts.x[index] = conf_window_width - ts.img[index]->width;
		} else if (strcmp(align, "center") == 0) {
			ts.x[index] = (conf_window_width - ts.img[index]->width) / 2;
		} else {
			log_error("align=%s is not supported.", align);
			return false;
		}
	}
	if (!IS_EMPTY(valign)) {
		if (strcmp(valign, "top") == 0) {
			ts.y[index] = 0;
		} else if (strcmp(align, "bottom") == 0) {
			ts.y[index] = conf_window_height - ts.img[index]->height;
		} else if (strcmp(align, "center") == 0) {
			ts.y[index] = (conf_window_height - ts.img[index]->height) / 2;
		} else {
			log_error("valign=%s is not supported.", valign);
			return false;
		}
	}
	if (!IS_EMPTY(xequal))
		ts.x[index] = atoi(xequal);
	if (!IS_EMPTY(xplus))
		ts.x[index] += atoi(xplus);
	if (!IS_EMPTY(xminus))
		ts.x[index] -= atoi(xminus);
	if (!IS_EMPTY(yequal))
		ts.y[index] = atoi(yequal);
	if (!IS_EMPTY(yplus))
		ts.y[index] += atoi(yplus);
	if (!IS_EMPTY(yminus))
		ts.y[index] -= atoi(yminus);

	*cont = true;

	return true;
}

static bool init_cl_alpha(bool *cont)
{
	const char *alpha;
	int index;

	index = get_index_for_name(true);
	if (index == -1)
		return false;
	alpha = get_string_param(CIEL_PARAM_ALPHA);

	ts.is_modified = true;
	ts.a[index] = atoi(alpha);
	if (ts.a[index] < 0)
		ts.a[index] = 0;
	if (ts.a[index] > 255)
		ts.a[index] = 255;

	*cont = true;

	return true;
}

static bool init_cl_dim(bool *cont)
{
	const char *dim;
	int index;

	index = get_index_for_name(false);
	if (index == -1)
		return false;
	dim = get_string_param(CIEL_PARAM_DIM);

	ts.is_modified = true;
	ts.dim[index] = strcmp(dim, "true") == 0;

	*cont = true;

	return true;
}

static bool init_cl_time(bool *cont)
{
	const char *time;

	time = get_string_param(CIEL_PARAM_TIME);

	ts.time = (float)atof(time);

	*cont = true;

	return true;
}

static bool init_cl_effect(bool *cont)
{
	const char *effect;

	effect = get_string_param(CIEL_PARAM_ALPHA);

	ts.effect = get_fade_method(effect);
	if (ts.effect == FADE_METHOD_INVALID) {
		log_error("effect=%s is invalid.", effect);
		return false;
	}
	if (ts.effect == FADE_METHOD_RULE || ts.effect == FADE_METHOD_MELT) {
		ts.rule_img = create_image_from_file(RULE_DIR, &effect[5]);
		if (ts.rule_img == NULL)
			return false;
	}

	*cont = true;

	return true;
}

static bool init_cl_move(bool *cont)
{
	const char *time, *xequal, *xplus, *xminus, *yequal, *yplus, *yminus, *alpha;
	int index, seq;

	index = get_index_for_name(true);
	if (index == -1)
		return false;
	time = get_string_param(CIEL_PARAM_TIME);
	if (IS_EMPTY(time)) {
		log_error("time= not specified.");
		log_script_exec_footer();
		return false;
	}
	xequal = get_string_param(CIEL_PARAM_XEQUAL);
	xplus = get_string_param(CIEL_PARAM_XPLUS);
	xminus = get_string_param(CIEL_PARAM_XMINUS);
	yequal = get_string_param(CIEL_PARAM_YEQUAL);
	yplus = get_string_param(CIEL_PARAM_YPLUS);
	yminus = get_string_param(CIEL_PARAM_YMINUS);
	alpha = get_string_param(CIEL_PARAM_ALPHA);
	if (IS_EMPTY(alpha))
		alpha = "255";

	seq = ts.anime_seq_count[index];
	if (seq >= CL_SEQ_SIZE) {
		log_error("Too many moves.");
		log_script_exec_footer();
		return false;
	}

	if (seq == 0) {
		ts.anime_x_orig[index] = ts.x[index];
		ts.anime_y_orig[index] = ts.y[index];
		ts.anime_a_orig[index] = ts.a[index];
		ts.anime_x[index][seq] = ts.x[index];
		ts.anime_y[index][seq] = ts.y[index];
		ts.anime_a[index][seq] = ts.a[index];
	} else {
		ts.anime_x[index][seq] = ts.anime_x[index][seq - 1];
		ts.anime_y[index][seq] = ts.anime_y[index][seq - 1];
		ts.anime_a[index][seq] = ts.anime_a[index][seq - 1];
	}

	ts.use_anime = true;
	ts.anime_time[index][seq] = (float)atof(time);
	if (!IS_EMPTY(xequal))
		ts.anime_x[index][seq] = atoi(xequal);
	if (!IS_EMPTY(xplus))
		ts.anime_x[index][seq] += atoi(xplus);
	if (!IS_EMPTY(xminus))
		ts.anime_x[index][seq] -= atoi(xminus);
	if (!IS_EMPTY(yequal))
		ts.anime_y[index][seq] = atoi(yequal);
	if (!IS_EMPTY(yplus))
		ts.anime_y[index][seq] += atoi(yplus);
	if (!IS_EMPTY(yminus))
		ts.anime_y[index][seq] -= atoi(yminus);
	if (!IS_EMPTY(alpha)) {
		ts.anime_a[index][seq] = atoi(alpha);
		if (ts.anime_a[index][seq] < 0)
			ts.anime_a[index][seq] = 0;
		if (ts.anime_a[index][seq] > 255)
			ts.anime_a[index][seq] = 255;
	}

	ts.anime_seq_count[index]++;

	*cont = true;

	return true;
}

static bool init_cl_run(bool *cont)
{
	if (!ts.use_anime) {
		if (!init_cl_run_fade())
			return false;
	} else {
		if (!init_cl_run_anime())
			return false;
	}

	*cont = false;

	return true;
}

static bool init_cl_run_fade(void)
{
	bool stay[CL_LAYERS];
	const char *fname[CL_LAYERS];
	int i;

	if (!ts.is_modified) {
		log_error("No action for @cl.run");
		log_script_exec_footer();
		return false;
	}

	for (i = 0; i < CL_LAYERS; i++) {
		if (ts.is_file_changed[i]) {
			stay[i] = false;
			fname[i] = ts.file[i];	
		} else {
			stay[i] = true;
			fname[i] = NULL;
		}
	}

	if (ts.effect == FADE_METHOD_INVALID)
		ts.effect = FADE_METHOD_NORMAL;

	reset_lap_timer(&sw);
	start_command_repetition();

	show_click(false);

	if (!start_fade_for_chs(stay, fname, ts.img, ts.x, ts.y, ts.a, ts.effect, ts.rule_img))	
		return false;

	/* メッセージボックスを消す (msgbox.show.on.bg=0, フェードアウト) */
	if (ts.is_file_changed[CL_BG] && conf_msgbox_show_on_bg == 0) {
		show_namebox(false);
		show_msgbox(false);
	}

	ts.is_modified = false;
	for (i = 0; i < CL_LAYERS; i++) {
		if (ts.file[i] != NULL) {
			free(ts.file[i]);
			ts.file[i] = NULL;
		}
		ts.is_file_changed[i] = false;
		ts.img[i] = NULL;
	}
	ts.rule_img = NULL;
	ts.use_anime = false;

	return true;
}

static bool init_cl_run_anime(void)
{
	float t;
	int i, j, prev_x, prev_y, prev_a;
	int stage_layer;

	for (i = 0; i < CL_LAYERS; i++) {
		if (ts.anime_seq_count[i] == 0)
			continue;

		stage_layer = cl_layer_to_stage_layer(i);

		clear_layer_anime_sequence(stage_layer);

		t = 0;
		prev_x = ts.anime_x_orig[i];
		prev_y = ts.anime_y_orig[i];
		prev_a = ts.anime_a_orig[i];

		for (j = 0; j < ts.anime_seq_count[i]; j++) {
			new_anime_sequence(stage_layer);
			add_anime_sequence_property_f("start", t);
			add_anime_sequence_property_f("end", t + ts.anime_time[i][j]);
			add_anime_sequence_property_i("from-x", prev_x);
			add_anime_sequence_property_i("from-y", prev_y);
			add_anime_sequence_property_i("from-a", prev_a);
			add_anime_sequence_property_i("to-x", ts.anime_x[i][j]);
			add_anime_sequence_property_i("to-y", ts.anime_y[i][j]);
			add_anime_sequence_property_i("to-a", ts.anime_a[i][j]);

			t += ts.anime_time[i][j];
			prev_x = ts.anime_x[i][j];
			prev_y = ts.anime_y[i][j];
			prev_a = ts.anime_a[i][j];
		}

		ts.anime_seq_count[i] = 0;
		ts.x[i] = prev_x;
		ts.y[i] = prev_y;
		ts.a[i] = prev_a;

		start_layer_anime(stage_layer);
	}

	start_command_repetition();

	ts.is_modified = false;
	for (i = 0; i < CL_LAYERS; i++) {
		if (ts.file[i] != NULL) {
			free(ts.file[i]);
			ts.file[i] = NULL;
		}
		ts.is_file_changed[i] = false;
		ts.img[i] = NULL;
	}
	ts.rule_img = NULL;

	return true;
}

static int cl_layer_to_stage_layer(int cl_layer)
{
	switch (cl_layer) {
	case CL_CH0:
		return LAYER_CHB;
	case CL_CH1:
		return LAYER_CHL;
	case CL_CH2:
		return LAYER_CHLC;
	case CL_CH3:
		return LAYER_CHR;
	case CL_CH4:
		return LAYER_CHRC;
	case CL_CH5:
		return LAYER_CHC;
	default:
		assert(NEVER_COME_HERE);
		break;
	}
	return -1;
}

static void render(void)
{
	if (!ts.use_anime)
		render_fade_frame();
	else
		render_anime_frame();
}

static void render_fade_frame(void)
{
	float lap;
	bool is_finished;

	/* 経過時間を取得する */
	lap = (float)get_lap_timer_millisec(&sw) / 1000.0f;
	if (lap >= ts.time)
		lap = ts.time;

	/* 入力に反応する */
	is_finished = false;
	if (is_auto_mode() &&
	    (is_control_pressed || is_return_pressed ||
	     is_left_clicked || is_down_pressed)) {
		/* 入力によりオートモードを終了する */
		stop_auto_mode();
		show_automode_banner(false);

		/* 繰り返し動作を終了する */
		stop_command_repetition();

		/* フェードを終了する */
		is_finished = true;
	} else if (is_skip_mode() &&
		   (is_control_pressed || is_return_pressed ||
		    is_left_clicked || is_down_pressed)) {
		/* 入力によりスキップモードを終了する */
		stop_skip_mode();
		show_skipmode_banner(false);

		/* 繰り返し動作を終了する */
		stop_command_repetition();

		/* フェードを終了する */
		is_finished = true;
	} else if ((lap >= ts.time)
		   ||
		   is_skip_mode()
		   ||
		   (!is_non_interruptible() &&
		    (is_control_pressed || is_return_pressed ||
		     is_left_clicked || is_down_pressed))) {
		/*
		 * 経過時間が一定値を超えた場合と、
		 * スキップモードの場合と、
		 * 入力により省略された場合
		 */

		/* 繰り返し動作を終了する */
		stop_command_repetition();

		/* フェードを終了する */
		is_finished = true;

		/* 入力ならスキップモードとオートモードを終了する */
		if (is_control_pressed || is_return_pressed ||
		    is_left_clicked || is_down_pressed) {
			if (is_skip_mode()) {
				stop_skip_mode();
				show_skipmode_banner(false);
			}
			if (is_auto_mode()) {
				stop_auto_mode();
				show_automode_banner(false);
			}
		}
	} else {
		/* 進捗を設定する */
		set_fade_progress(lap / ts.time);
	}

	/* ステージを描画する */
	render_fade();
	if (is_finished)
		finish_fade();

	/* 折りたたみシステムメニューを描画する */
	if (conf_sysmenu_transition && !is_non_interruptible())
		render_collapsed_sysmenu(false);
}

static void render_anime_frame(void)
{
	/* アニメ終了の場合を処理する */
	process_anime_finish();

	/* ステージを描画する */
	render_stage();

	/* 折りたたみシステムメニューを描画する */
	if (conf_sysmenu_transition && !is_non_interruptible())
		render_collapsed_sysmenu(false);
}

static void process_anime_finish(void)
{
	int i;
	bool has_running_anime;

	/* 繰り返し動作をしていない場合 */
	if (!is_in_command_repetition())
		return;

	/* アニメが終了した場合 */
	has_running_anime = false;
	for (i = 0; i < CL_LAYERS; i++) {
		if (ts.anime_seq_count[i] == 0)
			continue;
		if (!is_anime_finished_for_layer(cl_layer_to_stage_layer(i)))
			has_running_anime = true;
	}
	if (!has_running_anime) {
		/* 繰り返し動作を終了する */
		stop_command_repetition();
		ts.use_anime = false;
		return;
	}

	/* 入力によりオートモードが終了される場合 */
	if (is_auto_mode()) {
		if (is_control_pressed || is_return_pressed ||
		    is_left_clicked || is_down_pressed) {
			/* オートモードを終了する */
			stop_auto_mode();
			show_automode_banner(false);
			return;
		}
		return;
	}

	/* 入力によりスキップモードが終了される場合 */
	if (is_skip_mode()) {
		if (is_control_pressed || is_return_pressed ||
		    is_left_clicked || is_down_pressed) {
			/* スキップモードを終了する */
			stop_skip_mode();
			show_skipmode_banner(false);
			return;
		} else {
			/* アニメを終了する */
			for (i = 0; i < CL_LAYERS; i++) {
				if (ts.anime_seq_count[i] > 0)
					finish_layer_anime(i);
			}

			/* 繰り返し動作を終了する */
			stop_command_repetition();
			ts.use_anime = false;
		}
	}

	/* 入力で省略される場合 */
	if (!is_non_interruptible() &&
	     (is_control_pressed || is_return_pressed ||
	      is_left_clicked || is_down_pressed)) {
		/* アニメを終了する */
		for (i = 0; i < CL_LAYERS; i++) {
			if (ts.anime_seq_count[i] > 0)
				finish_layer_anime(i);
		}

		/* 繰り返し動作を終了する */
		stop_command_repetition();
		ts.use_anime = false;
		return;
	}
}

static int get_index_for_name(bool allow_bg)
{
	const char *name;
	int i;

	name = get_string_param(CIEL_PARAM_NAME);
	if (IS_EMPTY(name)) {
		log_error("name= is required");
		log_script_exec_footer();
		return false;
	}

	if (strcmp(name, "bg") == 0) {
		if (allow_bg)
			return CL_BG;
		log_error("name=bg is not allowed.");
		return false;
	}

	for (i = 0; i < CL_CHARACTERS; i++) {
		if (ts.name[i] != NULL && strcmp(name, ts.name[i]) == 0)
			return i;
	}

	log_error("No character named %d", name);
	log_script_exec_footer();

	return false;
}
