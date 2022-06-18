/* -*- coding: utf-8-with-signature; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Suika
 * Copyright (C) 2001-2019, TABATA Keiichi. All rights reserved.
 */

/*
 * @choose, @select, @switchコマンド
 *
 * [Changes]
 *  - 2017/08/14 作成
 *  - 2017/10/31 効果音に対応
 *  - 2019/09/17 NEWSに対応
 *  - 2022/06/17 @chooseに対応、@newsの設定項目を@switchに統合
 */

#include "suika.h"

/* 親ボタンの最大数 */
#define PARENT_COUNT		(8)

/* 親ボタン1つあたりの子ボタンの最大数 */
#define CHILD_COUNT		(8)

/* @selectの選択肢の数 */
#define SELECT_OPTION_COUNT	(3)

/* コマンドの引数 */
#define PARENT_MESSAGE(n)	(SWITCH_PARAM_PARENT_M1 + n)
#define CHILD_LABEL(p,c)	(SWITCH_PARAM_CHILD1_L1 + 16 * p + 2 * c)
#define CHILD_MESSAGE(p,c)	(SWITCH_PARAM_CHILD1_M1 + 16 * p + 2 * c)
#define CHOOSE_LABEL(n)		(CHOOSE_PARAM_LABEL1 + n * 2)
#define CHOOSE_MESSAGE(n)	(CHOOSE_PARAM_LABEL1 + n * 2 + 1)
#define SELECT_LABEL(n)		(SELECT_PARAM_LABEL1 + n)
#define SELECT_MESSAGE(n)	(SELECT_PARAM_TEXT1 + n)

/* NEWSコマンド時のswitchボックス開始オフセット */
#define SWITCH_BASE	(4)

/* 指定した親選択肢が無効であるか */
#define IS_PARENT_DISABLED(n)	(parent_button[n].msg == NULL)

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

/* 子選択肢のボタン */
static struct child_button {
	const char *msg;
	const char *label;
	int x;
	int y;
	int w;
	int h;
} child_button[PARENT_COUNT][CHILD_COUNT];

/* 最初の描画であるか */
static bool is_first_frame;

/* 子選択肢の最初の描画であるか */
static bool is_child_first_frame;

/* ポイントされている親項目のインデックス */
static int pointed_parent_index;

/* 選択されているされている親項目のインデックス */
static int selected_parent_index;

/* ポイントされている子項目のインデックス */
static int pointed_child_index;

/* 前方参照 */
static bool init(void);
static bool get_choose_info(void);
static void get_select_info(void);
static bool get_parents_info(void);
static bool get_children_info(void);
static void draw_frame(int *x, int *y, int *w, int *h);
static void draw_frame_parent(int *x, int *y, int *w, int *h);
static void draw_frame_child(int *x, int *y, int *w, int *h);
static int get_pointed_parent_index(void);
static int get_pointed_child_index(void);
static void draw_fo_fi_parent(void);
static void draw_switch_parent_images(void);
static void update_switch_parent(int *x, int *y, int *w, int *h);
static void draw_fo_fi_child(void);
static void draw_switch_child_images(void);
static void update_switch_child(int *x, int *y, int *w, int *h);
static void draw_text(int x, int y, int w, const char *t, bool is_news);
static void draw_keep(void);
static void play_se(const char *file);
static bool cleanup(void);

/*
 * switchコマンド
 */
bool switch_command(int *x, int *y, int *w, int *h)
{
	/* 初期化処理を行う */
	if (!is_in_command_repetition())
		if (!init())
			return false;

	/* セーブ画面への遷移を確認する */
	if (selected_parent_index == -1 && is_right_button_pressed &&
	    is_save_load_enabled()) {
		draw_keep();
		play_se(conf_msgbox_save_se);
		start_save_mode(false);
		stop_command_repetition();
		return true;
	}

	/* ヒストリ画面への遷移を確認する */
	if (is_up_pressed && !is_history_empty()) {
		draw_keep();
		play_se(conf_msgbox_history_se);
		start_history_mode();
		stop_command_repetition();
		return true;
	}

	/* 繰り返し動作を行う */
	draw_frame(x, y, w, h);

	/* 終了処理を行う */
	if (!is_in_command_repetition())
		if (!cleanup())
			return false;

	return true;
}

/* コマンドの初期化処理を行う */
bool init(void)
{
	int type;

	start_command_repetition();

	is_first_frame = true;
	pointed_parent_index = -1;
	selected_parent_index = -1;
	pointed_child_index = -1;
	is_child_first_frame = false;

	type = get_command_type();
	if (type == COMMAND_CHOOSE) {
		/* @chooseコマンドの引数情報を取得する */
		if (!get_choose_info())
			return false;
	} else if (type == COMMAND_SELECT) {
		/* @selectコマンドの引数情報を取得する */
		get_select_info();
	} else {
		/* 親選択肢の情報を取得する */
		if (!get_parents_info())
			return false;

		/* 子選択肢の情報を取得する */
		if (!get_children_info())
			return false;
	}

	/* 名前ボックス、メッセージボックスを非表示にする */
	show_namebox(false);
	show_msgbox(false);

	/* オートモードを終了する */
	if (is_auto_mode())
		stop_auto_mode();

	/* スキップモードを終了する */
	if (is_skip_mode())
		stop_skip_mode();

	return true;
}

/* @chooseコマンドの引数情報を取得する */
static bool get_choose_info(void)
{
	const char *label, *msg;
	int i;

	memset(parent_button, 0, sizeof(parent_button));
	memset(child_button, 0, sizeof(child_button));

	/* 選択肢の情報を取得する */
	for (i = 0; i < PARENT_COUNT; i++) {
		/* ラベルを取得する */
		label = get_string_param(CHOOSE_LABEL(i));
		if (strcmp(label, "") == 0)
			break;

		/* メッセージを取得する */
		msg = get_string_param(CHOOSE_MESSAGE(i));
		if (strcmp(msg, "") == 0) {
			log_script_choose_no_message();
			log_script_exec_footer();
			return false;
		}

		/* ボタンの情報を保存する */
		parent_button[i].msg = msg;
		parent_button[i].label = label;
		parent_button[i].has_child = false;
		parent_button[i].child_count = 0;

		/* 座標を計算する */
		get_switch_rect(i, &parent_button[i].x,
				&parent_button[i].y,
				&parent_button[i].w,
				&parent_button[i].h);
	}

	return true;
}

/* @selectコマンドの引数情報を取得する */
static void get_select_info(void)
{
	const char *label, *msg;
	int i;

	memset(parent_button, 0, sizeof(parent_button));
	memset(child_button, 0, sizeof(child_button));

	/* 選択肢の情報を取得する */
	for (i = 0; i < SELECT_OPTION_COUNT; i++) {
		/* ラベルを取得する */
		label = get_string_param(SELECT_LABEL(i));

		/* メッセージを取得する */
		msg = get_string_param(SELECT_MESSAGE(i));

		/* ボタンの情報を保存する */
		parent_button[i].msg = msg;
		parent_button[i].label = label;
		parent_button[i].has_child = false;
		parent_button[i].child_count = 0;

		/* 座標を計算する */
		get_switch_rect(i, &parent_button[i].x,
				&parent_button[i].y,
				&parent_button[i].w,
				&parent_button[i].h);
	}
}

/* 親選択肢の情報を取得する */
static bool get_parents_info(void)
{
	const char *p;
	int i, parent_button_count = 0;

	memset(parent_button, 0, sizeof(parent_button));

	/* 親選択肢の情報を取得する */
	for (i = 0; i < PARENT_COUNT; i++) {
		/* 親選択肢のメッセージを取得する */
		p = get_string_param(PARENT_MESSAGE(i));
		assert(strcmp(p, "") != 0);

		/* @switchの場合、"*"が現れたら親選択肢の読み込みを停止する */
		if (get_command_type() == COMMAND_SWITCH) {
			if (strcmp(p, "*") == 0)
				break;
		} else {
			/* @newsの場合、"*"が現れたら選択肢をスキップする */
			if (strcmp(p, "*") == 0)
				continue;
		}

		/* メッセージを保存する */
		parent_button[i].msg = p;

		/* ラベルがなければならない */
		p = get_string_param(CHILD_LABEL(i, 0));
		if (strcmp(p, "*") == 0 || strcmp(p, "") == 0) {
			log_script_switch_no_label();
			log_script_exec_footer();
			return false;
		}

		/* 子の最初のメッセージが"*"か省略なら、一階層のメニューと
		   判断してラベルを取得する */
		p = get_string_param(CHILD_MESSAGE(i, 0));
		if (strcmp(p, "*") == 0 || strcmp(p, "") == 0) {
			p = get_string_param(CHILD_LABEL(i, 0));
			parent_button[i].label = p;
			parent_button[i].has_child = false;
			parent_button[i].child_count = 0;
		} else {
			parent_button[i].label = NULL;
			parent_button[i].has_child = true;
			parent_button[i].child_count = 0;
		}

		/* 座標を計算する */
		if (get_command_type() == COMMAND_SWITCH) {
			get_switch_rect(i, &parent_button[i].x,
					&parent_button[i].y,
					&parent_button[i].w,
					&parent_button[i].h);
		} else {
			get_news_rect(i, &parent_button[i].x,
				      &parent_button[i].y,
				      &parent_button[i].w,
				      &parent_button[i].h);
		}

		parent_button_count++;
	}
	if (parent_button_count == 0) {
		log_script_switch_no_item();
		log_script_exec_footer();
		return false;
	}
	return true;
}

/* 子選択肢の情報を取得する */
static bool get_children_info(void)
{
	const char *p;
	int i, j;

	memset(child_button, 0, sizeof(child_button));

	/* 子選択肢の情報を取得する */
	for (i = 0; i < PARENT_COUNT; i++) {
		/* 親選択肢が無効の場合、スキップする */
		if (IS_PARENT_DISABLED(i))
			continue;

		/* 親選択肢が子選択肢を持たない場合、スキップする */
		if (!parent_button[i].has_child)
			continue;

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
		assert(j > 0);
		parent_button[i].child_count = j;
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

	/* セーブ画面かヒストリ画面から復帰した場合のフラグをクリアする */
	check_restore_flag();
	check_history_flag();

	/* 初回描画の場合 */
	if (is_first_frame) {
		pointed_parent_index = get_pointed_parent_index();

		/* 親選択肢の描画を行う */
		draw_fo_fi_parent();
		update_switch_parent(x, y, w, h);

		/* 名前ボックス、メッセージボックスを消すため再描画する */
		*x = 0;
		*y = 0;
		*w = conf_window_width;
		*h = conf_window_height;

		is_first_frame = false;
		return;
	}

	/* 親選択肢を選んでいる最中の場合 */
	if (selected_parent_index == -1) {
		draw_frame_parent(x, y, w, h);
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
		draw_keep();
	} else if (new_pointed_index == pointed_parent_index) {
		draw_keep();
	} else {
		/* ボタンを描画する */
		pointed_parent_index = new_pointed_index;
		update_switch_parent(x, y, w, h);

		/* SEを再生する */
		if (new_pointed_index != -1 && !is_left_button_pressed) {
			play_se(get_command_type() == COMMAND_NEWS ?
				conf_news_change_se : conf_switch_change_se);
		}
	}

	/* マウスの左ボタンでクリックされた場合 */
	if (new_pointed_index != -1 && is_left_button_pressed) {
		selected_parent_index = new_pointed_index;
		pointed_child_index = -1;
		is_child_first_frame = true;

		if (parent_button[new_pointed_index].has_child) {
			/* 子選択肢の描画を行う */
			draw_fo_fi_child();
			update_switch_child(x, y, w, h);

			/* SEを鳴らす */
			play_se(conf_switch_parent_click_se_file);
		} else {
			/* ステージをボタンなしで描画しなおす */
			draw_stage();

			/* SEを鳴らす */
			play_se(conf_switch_child_click_se_file);

			/* 繰り返し動作を終了する */
			stop_command_repetition();
		}

		/* ステージ全体を再描画する */
		*x = 0;
		*y = 0;
		*w = conf_window_width;
		*h = conf_window_height;
	}
}

/* 子選択肢の描画を行う */
static void draw_frame_child(int *x, int *y, int *w, int *h)
{
	int new_pointed_index;

	if (is_right_button_pressed) {
		selected_parent_index = -1;

		/* 親選択肢の描画を行う */
		draw_fo_fi_parent();
		update_switch_parent(x, y, w, h);
		return;
	}

	new_pointed_index = get_pointed_child_index();

	if (is_child_first_frame) {
		is_child_first_frame = false;

		/* ボタンを描画する */
		pointed_child_index = new_pointed_index;
		update_switch_child(x, y, w, h);
	} else if (new_pointed_index == -1 && pointed_child_index == -1) {
		draw_keep();
	} else if (new_pointed_index == pointed_child_index) {
		draw_keep();
	} else {
		/* ボタンを描画する */
		pointed_child_index = new_pointed_index;
		update_switch_child(x, y, w, h);

		/* SEを再生する */
		if (new_pointed_index != -1 && !is_left_button_pressed)
			play_se(conf_switch_change_se);
	}

	/* マウスの左ボタンでクリックされた場合 */
	if (new_pointed_index != -1 && is_left_button_pressed) {
		/* SEを鳴らす */
		play_se(conf_switch_child_click_se_file);

		/* ステージをボタンなしで描画しなおす */
		draw_stage();
		*x = 0;
		*y = 0;
		*w = conf_window_width;
		*h = conf_window_height;

		/* 繰り返し動作を終了する */
		stop_command_repetition();
	}
}

/* 親選択肢でポイントされているものを取得する */
static int get_pointed_parent_index(void)
{
	int i;

	for (i = 0; i < PARENT_COUNT; i++) {
		if (IS_PARENT_DISABLED(i))
			continue;

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

/* 親選択肢のFO/FIレイヤを描画する */
static void draw_fo_fi_parent(void)
{
	lock_draw_char_on_fo_fi();
	draw_stage_fo_fi();
	draw_switch_parent_images();
	unlock_draw_char_on_fo_fi();
}

/* 親選択肢のイメージを描画する */
void draw_switch_parent_images(void)
{
	int i;
	bool is_news;

	assert(selected_parent_index == -1);

	for (i = 0; i < PARENT_COUNT; i++) {
		if (IS_PARENT_DISABLED(i))
			continue;

		/* NEWSの項目であるか調べる */
		is_news = get_command_type() == COMMAND_NEWS &&
			i < SWITCH_BASE;

		/* FO/FIレイヤにスイッチを描画する */
		if (!is_news) {
			draw_switch_fg_image(parent_button[i].x,
					     parent_button[i].y);
			draw_switch_bg_image(parent_button[i].x,
					     parent_button[i].y);
		} else {
			draw_news_fg_image(parent_button[i].x,
					   parent_button[i].y);
			draw_news_bg_image(parent_button[i].x,
					   parent_button[i].y);
		}

		/* テキストを描画する */
		draw_text(parent_button[i].x, parent_button[i].y,
			  parent_button[i].w, parent_button[i].msg, is_news);
	}
}

/* 親選択肢を画面に描画する */
void update_switch_parent(int *x, int *y, int *w, int *h)
{
	int i, bx, by, bw, bh;

	assert(selected_parent_index == -1);

	i = pointed_parent_index;

	/* 描画するFIレイヤの矩形を求める */
	bx = by = bw = bh = 0;
	if (i != -1) {
		bx = parent_button[i].x;
		by = parent_button[i].y;
		bw = parent_button[i].w;
		bh = parent_button[i].h;
	}

	/* FOレイヤ全体とFIレイヤの矩形を画面に描画する */
	draw_stage_with_buttons(bx, by, bw, bh, 0, 0, 0, 0);

	/* 更新範囲を設定する */
	*x = 0;
	*y = 0;
	*w = conf_window_width;
	*h = conf_window_height;
}

/* 子選択肢のFO/FIレイヤを描画する */
static void draw_fo_fi_child(void)
{
	lock_draw_char_on_fo_fi();
	draw_stage_fo_fi();
	draw_switch_child_images();
	unlock_draw_char_on_fo_fi();
}

/* 子選択肢のイメージを描画する */
void draw_switch_child_images(void)
{
	int i, j;

	assert(selected_parent_index != -1);
	assert(parent_button[selected_parent_index].child_count > 0);

	i = selected_parent_index;
	for (j = 0; j < parent_button[i].child_count; j++) {
		/* FO/FIレイヤにスイッチを描画する */
		draw_switch_fg_image(child_button[i][j].x,
				     child_button[i][j].y);
		draw_switch_bg_image(child_button[i][j].x,
				     child_button[i][j].y);

		/* テキストを描画する */
		draw_text(child_button[i][j].x, child_button[i][j].y,
			  child_button[i][j].w, child_button[i][j].msg, false);
	}
}

/* 親選択肢を画面に描画する */
void update_switch_child(int *x, int *y, int *w, int *h)
{
	int i, j, bx, by, bw, bh;

	assert(selected_parent_index != -1);

	i = selected_parent_index;
	j = pointed_child_index;

	/* 描画するFIレイヤの矩形を求める */
	bx = by = bw = bh = 0;
	if (j != -1) {
		bx = child_button[i][j].x;
		by = child_button[i][j].y;
		bw = child_button[i][j].w;
		bh = child_button[i][j].h;
	}

	/* FO全体とFIの1矩形を描画する(GPU用) */
	draw_stage_with_buttons(bx, by, bw, bh, 0, 0, 0, 0);

	/* 更新範囲を設定する */
	*x = 0;
	*y = 0;
	*w = conf_window_width;
	*h = conf_window_height;
}

/* 選択肢のテキストを描画する */
static void draw_text(int x, int y, int w, const char *t, bool is_news)
{
	uint32_t c;
	int mblen, xx;

	/* 描画位置を決める */
	xx = x + (w - get_utf8_width(t)) / 2;
	y += is_news ? conf_news_text_margin_y : conf_switch_text_margin_y;

	/* 1文字ずつ描画する */
	while (*t != '\0') {
		/* 描画する文字を取得する */
		mblen = utf8_to_utf32(t, &c);
		if (mblen == -1)
			return;

		/* 描画する */
		xx += draw_char_on_fo_fi(xx, y, c);

		/* 次の文字へ移動する */
		t += mblen;
	}
}

/* 描画する(GPU用) */
static void draw_keep(void)
{
	int x, y, w, h, i, j;

	x = y = w = h = 0;
	if (selected_parent_index == -1) {
		i = pointed_parent_index;
		if (i != -1) {
			x = parent_button[i].x;
			y = parent_button[i].y;
			w = parent_button[i].w;
			h = parent_button[i].h;
		}
	} else {
		i = selected_parent_index;
		j = pointed_child_index;
		if (j != -1) {
			x = child_button[i][j].x;
			y = child_button[i][j].y;
			w = child_button[i][j].w;
			h = child_button[i][j].h;
		}
	}

	/* FO全体とFIの1矩形を描画する(GPU用) */
	draw_stage_with_buttons_keep(x, y, w, h, 0, 0, 0, 0);
}

/* SEを再生する */
static void play_se(const char *file)
{
	struct wave *w;

	if (file == NULL || strcmp(file, "") == 0)
		return;

	w = create_wave_from_file(SE_DIR, file, false);
	if (w == NULL)
		return;

	set_mixer_input(SE_STREAM, w);
}

/* コマンドを終了する */
static bool cleanup(void)
{
	int n, m;

	n = selected_parent_index;

	/* 子選択肢が選択された場合 */
	if (parent_button[n].has_child) {
		m = pointed_child_index;
		return move_to_label(child_button[n][m].label);
	}

	/* 親選択肢が選択された場合 */
	return move_to_label(parent_button[n].label);
}
