/* -*- coding: utf-8-with-signature; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Suika
 * Copyright (C) 2001-2016, TABATA Keiichi. All rights reserved.
 */

/*
 * @menuコマンド
 *
 * [Changes]
 *  - 2016/07/04 作成
 */

#include "suika.h"

/* false assertion */
#define NEVER_COME_HERE	(0)

/* ボタンの数 */
#define PARENT_COUNT	(8)
#define CHILD_COUNT	(8)

/* コマンドの引数 */
#define PARENT_MESSAGE(n)	(SWITCH_PARAM_PARENT_M1 + n)
#define CHILD_LABEL(p,c)	(SWITCH_PARAM_CHILD1_L1 + 16 * p + 2 * c)
#define CHILD_MESSAGE(p,c)	(SWITCH_PARAM_CHILD1_M1 + 16 * p + 2 * c)

/* 親選択肢のボタン */
static struct parent_button {
	const char *msg;
	const char *label;
	bool has_child;
	int child_count;
	int x;
	int y;
	int w;
	int h;
} parent_button[PARENT_COUNT];

int parent_button_count;

/* 子選択肢のボタン */
static struct child_button {
	const char *msg;
	const char *label;
	int x;
	int y;
	int w;
	int h;
} child_button[PARENT_COUNT][CHILD_COUNT];

/* 繰り返し動作中であるか */
static bool repeatedly;

/* 最初の描画であるか */
static bool is_first_frame;

/* ポイントされている親項目のインデックス */
static int pointed_parent_index;

/* 選択されているされている親項目のインデックス */
static int selected_parent_index;

/* ポイントされている子項目のインデックス */
static int pointed_child_index;

/* 前方参照 */
static bool init(void);
static bool get_parent_info(void);
static bool get_children_info(void);
static void draw_frame(int *x, int *y, int *w, int *h);
static void draw_frame_parent(int *x, int *y, int *w, int *h);
static void draw_frame_child(int *x, int *y, int *w, int *h);
static int get_pointed_parent_index(void);
static int get_pointed_child_index(void);
static void draw_switch_images(int *x, int *y, int *w, int *h);
static bool cleanup(void);

/*
 * switchコマンド
 */
bool switch_command(int *x, int *y, int *w, int *h)
{
	/* 初期化処理を行う */
	if (!repeatedly)
		if (!init())
			return false;

	/* 繰り返し動作を行う */
	draw_frame(x, y, w, h);

	/* 終了処理を行う */
	if (!repeatedly)
		if (!cleanup())
			return false;

	return true;
}

/* コマンドの初期化処理を行う */
bool init(void)
{
	repeatedly = true;
	is_first_frame = true;
	pointed_parent_index = -1;
	selected_parent_index = -1;
	pointed_child_index = -1;

	/* 親選択肢の情報を取得する */
	if (!get_parent_info())
		return false;

	/* 子選択肢の情報を取得する */
	if (!get_children_info())
		return false;

	return true;
}

/* 親選択肢の情報を取得する */
static bool get_parent_info(void)
{
	const char *p;
	int i;

	/* 親選択肢の情報を取得する */
	parent_button_count = 0;
	for (i = 0; i < PARENT_COUNT; i++) {
		/* 親選択肢のメッセージを取得し、"*"が現れたらスキップする */
		p = get_string_param(PARENT_MESSAGE(i));
		assert(strcmp(p, "") != 0);
		if (strcmp(p, "*") == 0)
			break;
		parent_button[i].msg = p;

		/* 子の最初のメッセージが"*"か省略なら、一階層のメニューと
		   判断してラベルを取得する */
		p = get_string_param(CHILD_MESSAGE(i, 0));
		if (strcmp(p, "*") == 0 || strcmp(p, "") == 0) {
			p = get_string_param(CHILD_LABEL(i, 0));

			/* ラベルは省略できない */
			if (strcmp(p, "*") == 0 || strcmp(p, "") == 0) {
				log_script_switch_no_item();
				log_script_exec_footer();
				return false;
			}

			parent_button[i].label = p;
			parent_button[i].has_child = false;
			parent_button[i].child_count = 0;
		} else {
			parent_button[i].label = NULL;
			parent_button[i].has_child = true;
			parent_button[i].child_count = 0;
		}

		/* 座標を計算する */
		get_switch_rect(i, &parent_button[i].x,
				&parent_button[i].y,
				&parent_button[i].w,
				&parent_button[i].h);

		/* 親選択肢のボタン数をカウントする */
		parent_button_count++;
	}
	if (parent_button_count == 0) {
		log_script_switch_no_item();
		log_script_exec_footer();
		return false;
	}
	for (i = parent_button_count; i < PARENT_COUNT; i++) {
		parent_button[i].msg = NULL;
		parent_button[i].label = NULL;
		parent_button[i].has_child = false;
		parent_button[i].child_count = 0;
		parent_button[i].x = 0;
		parent_button[i].y = 0;
		parent_button[i].w = 0;
		parent_button[i].h = 0;
	}
	return true;
}

/* 子選択肢の情報を取得する */
static bool get_children_info(void)
{
	const char *p;
	int i, j;

	/* 子選択肢の情報を取得する */
	for (i = 0; i < parent_button_count; i++) {
		/* 親選択肢が子選択肢を持たない場合、スキップする */
		if (!parent_button[i].has_child) {
			for (j = 0; j < CHILD_COUNT; j++) {
				child_button[i][j].msg = NULL;
				child_button[i][j].label = NULL;
				child_button[i][j].x = 0;
				child_button[i][j].y = 0;
				child_button[i][j].w = 0;
				child_button[i][j].h = 0;
			}
			continue;
		}

		/* 子選択肢の情報を取得する */
		for (j = 0; j < CHILD_COUNT; j++) {
			/* ラベルを取得し、"*"か省略が現れたらスキップする */
			p = get_string_param(CHILD_LABEL(i, j));
			if (strcmp(p, "*") == 0 || strcmp(p, "") == 0)
				break;
			child_button[i][j].label = p;

			/* メッセージを取得する */
			p = get_string_param(CHILD_MESSAGE(i, j));
			if (strcmp(p, "*") == 0 || strcmp(p, "") == 0) {
				log_script_switch_no_item();
				log_script_exec_footer();
				return false;
			}
			child_button[i][j].msg = p;

			/* 座標を計算する */
			get_switch_rect(j, &child_button[i][j].x,
					&child_button[i][j].y,
					&child_button[i][j].w,
					&child_button[i][j].h);
		}
		parent_button[i].child_count = j;
		for (; j < CHILD_COUNT; j++) {
			child_button[i][j].msg = NULL;
			child_button[i][j].label = NULL;
			child_button[i][j].x = 0;
			child_button[i][j].y = 0;
			child_button[i][j].w = 0;
			child_button[i][j].h = 0;
		}
	}

	return true;
}

/* フレームを描画する */
static void draw_frame(int *x, int *y, int *w, int *h)
{
	*x = 0;
	*y = 0;
	*w = 0;
	*h = 0;

	/* 初回描画の場合 */
	if (is_first_frame) {
		draw_switch_images(x, y, w, h);
		is_first_frame = false;
		return;
	}

	/* 親選択肢を選んでいる最中の場合 */
	if (selected_parent_index == -1) {
		draw_frame_parent(x, y, w, h);
		return;
	} else {
		/* 子選択肢を選んでいる最中の場合 */
		draw_frame_child(x, y, w, h);
	}
}

/* 親選択肢の描画を行う */
static void draw_frame_parent(int *x, int *y, int *w, int *h)
{
	int new_pointed_index;

	new_pointed_index = get_pointed_parent_index();

	if (new_pointed_index == -1 && pointed_parent_index == -1) {
		/* 何もしない */
	} else if (new_pointed_index == pointed_parent_index) {
		/* 何もしない */
	} else {
		/* ボタンを描画する */
		pointed_parent_index = new_pointed_index;
		draw_switch_images(x, y, w, h);
	}

	if (new_pointed_index != -1 && is_left_button_pressed) {
		selected_parent_index = new_pointed_index;

		/* ステージをボタンなしで描画しなおす */
		draw_stage();
		*x = 0;
		*y = 0;
		*w = conf_window_width;
		*h = conf_window_height;

		/* 子選択肢の描画を行う */
		draw_switch_images(x, y, w, h);
	}
}

/* 子選択肢の描画を行う */
static void draw_frame_child(int *x, int *y, int *w, int *h)
{
	int new_pointed_index;

	new_pointed_index = get_pointed_child_index();

	if (new_pointed_index == -1 && pointed_child_index == -1) {
		/* 何もしない */
	} else if (new_pointed_index == pointed_child_index) {
		/* 何もしない */
	} else {
		/* ボタンを描画する */
		pointed_child_index = new_pointed_index;
		draw_switch_images(x, y, w, h);
	}

	if (new_pointed_index != -1 && is_left_button_pressed) {
		/* ステージをボタンなしで描画しなおす */
		draw_stage();
		*x = 0;
		*y = 0;
		*w = conf_window_width;
		*h = conf_window_height;
	}
}

/* 親選択肢でポイントされているものを取得する */
static int get_pointed_parent_index(void)
{
	int i;

	for (i = 0; i < parent_button_count; i++) {
		if (mouse_pos_x >= parent_button[i].x &&
		    mouse_pos_x < parent_button[i].x + parent_button[i].w &&
		    mouse_pos_y >= parent_button[i].y &&
		    mouse_pos_y < parent_button[i].y + parent_button[i].h)
			return i;
	}

	return -1;
}

/* 子選択肢でポイントされているものを取得する */
static int get_pointed_child_index(void)
{
	int i, n;

	n = selected_parent_index;
	for (i = 0; i < parent_button[n].child_count; i++) {
		if (mouse_pos_x >= child_button[n][i].x &&
		    mouse_pos_x < child_button[n][i].x +
		    child_button[n][i].w &&
		    mouse_pos_y >= child_button[n][i].y &&
		    mouse_pos_y < child_button[n][i].y +
		    child_button[n][i].h)
			return i;
	}

	return -1;
}

/* FIレイヤにスイッチのイメージを描画する */
static void draw_switch_images(int *x, int *y, int *w, int *h)
{
	int i, n;

	if (selected_parent_index == -1) {
		for (i = 0; i < parent_button_count; i++) {
			/* FIレイヤにスイッチを描画する */
			if (i == pointed_parent_index) {
				draw_switch_fg_image(parent_button[i].x,
						     parent_button[i].y);
			} else {
				draw_switch_bg_image(parent_button[i].x,
						     parent_button[i].y);
			}

			/* FIレイヤを含めてステージを更新する */
			draw_stage_rect_with_switch(parent_button[i].x,
						    parent_button[i].y,
						    parent_button[i].w,
						    parent_button[i].h);

			/* 更新範囲を求める */
			union_rect(x, y, w, h, *x, *y, *w, *h,
				   parent_button[i].x, parent_button[i].y,
				   parent_button[i].w, parent_button[i].h);
		}
	} else {
		assert(parent_button[selected_parent_index].child_count > 0);
		n = selected_parent_index;
		for (i = 0; i < parent_button[n].child_count; i++) {
			/* FIレイヤにスイッチを描画する */
			if (i == pointed_child_index) {
				draw_switch_fg_image(child_button[n][i].x,
						     child_button[n][i].y);
			} else {
				draw_switch_bg_image(child_button[n][i].x,
						     child_button[n][i].y);
			}

			/* FIレイヤを含めてステージを更新する */
			draw_stage_rect_with_switch(child_button[n][i].x,
						    child_button[n][i].y,
						    child_button[n][i].w,
						    child_button[n][i].h);

			/* 更新範囲を求める */
			union_rect(x, y, w, h, *x, *y, *w, *h,
				   child_button[n][i].x,
				   child_button[n][i].y,
				   child_button[n][i].w,
				   child_button[n][i].h);
		}
	}
}

/* コマンドを終了する */
static bool cleanup(void)
{
	/* ラベルにジャンプする */
	/*return move_to_label(button[pointed_index].label);*/

	/* stub */
	return move_to_next_command();
}
