/* -*- coding: utf-8-with-signature; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Suika 2
 * Copyright (C) 2001-2022, TABATA Keiichi. All rights reserved.
 */

/*
 * セーブ画面とセーブ・ロード実行
 *
 * [Changes]
 *  - 2016/06/30 作成
 *  - 2017/08/17 グローバル変数に対応
 *  - 2018/07/22 gosubに対応
 *  - 2021/06/05 マスターボリュームに対応
 *  - 2021/07/07 セーブ専用画面に対応
 *  - 2021/07/29 クイックセーブ・ロードに対応
 *  - 2022/06/09 デバッガに対応
 */

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "suika.h"

/* クイックセーブのファイル名 */
#define QUICK_SAVE_FILE_NAME	"q000.sav"

/* セーブデータ数 */
#define SAVE_SLOTS		(30)

/* クイックセーブデータのインデックス */
#define QUICK_SAVE_INDEX	(SAVE_SLOTS)

/* 1画面あたりのセーブデータ数 */
#define PAGE_SLOTS		(3)

/* セーブ画面のページ数 */
#define SAVE_PAGES		(SAVE_SLOTS / PAGE_SLOTS)

/* ボタンの数 */
#define BUTTON_COUNT		(12)

/* ボタンのインデックス */
#define BUTTON_ONE		(0)
#define BUTTON_TWO		(1)
#define BUTTON_THREE		(2)
#define BUTTON_PREV		(3)
#define BUTTON_NEXT		(4)
#define BUTTON_SAVE		(5)
#define BUTTON_LOAD		(6)
#define BUTTON_EXIT		(7)
#define BUTTON_TITLE		(8)

/* 章題*/
static char *chapter_name;

/* 最後のメッセージ */
static char *last_message;

/* セーブデータの日付 */
static time_t save_time[SAVE_SLOTS];

/* セーブデータの章タイトル */
static char *save_title[SAVE_SLOTS];

/* セーブデータのメッセージ */
static char *save_message[SAVE_SLOTS];

/* セーブデータのサムネイル */
static struct image *save_thumb[SAVE_SLOTS];

/* クイックセーブデータの日付 */
static time_t quick_save_time;

/* 文字列読み込み用バッファ */
static char tmp_str[4096];

/* サムネイル読み書き用バッファ */
static unsigned char *tmp_pixels;

/* ボタンの座標 */
static struct button {
	int x;
	int y;
	int w;
	int h;
} button[BUTTON_COUNT];

/* ロードが実行された直後であるか */
static bool load_flag;

/* セーブ画面から復帰した直後であるか */
static bool restore_flag;

/* セーブ画面が有効であるか */
static bool is_save_load_mode_enabled;

/* セーブモードであるか (falseならロードモード) */
static bool is_save_mode;

/* @gotoコマンドから呼ばれているか */
static bool is_goto;

/* 最初の描画であるか */
static bool is_first_frame;

/* ページ */
static int page;

/* ポイントされている項目のインデックス */
static int pointed_index;

/* 削除ボタンがポイントされているか */
static bool is_delete_button_pointed;

/* 前方参照 */
static void load_button_conf(void);
static void load_basic_save_data(void);
static void load_basic_save_data_file(struct rfile *rf, int index);
static void draw_page(int *x, int *y, int *w, int *h);
static void draw_page_keep(void);
static int get_pointed_index(void);
static void draw_all_thumbs(void);
static void draw_all_text_items(void);
static int draw_text_item(int x, int y, const char *text, int base_x);
static bool update_pointed_index(int *x, int *y, int *w, int *h);
static bool process_left_press(int new_pointed_index, int *x, int *y, int *w,
			       int *h);
static void process_left_press_save_button(int new_pointed_index, int *x,
					   int *y, int *w, int *h);
static void process_delete(int new_pointed_index);
static bool have_save_data(int new_pointed_index);
static void play_se(const char *file);
static bool process_save(int new_pointed_index);
static bool serialize_all(const char *fname, uint64_t *timestamp, int index);
static bool serialize_title(struct wfile *wf, int index);
static bool serialize_message(struct wfile *wf, int index);
static bool serialize_thumb(struct wfile *wf, int index);
static bool serialize_command(struct wfile *wf);
static bool serialize_stage(struct wfile *wf);
static bool serialize_bgm(struct wfile *wf);
static bool serialize_volumes(struct wfile *wf);
static bool serialize_vars(struct wfile *wf);
static bool process_load(int new_pointed_index);
static bool deserialize_all(const char *fname);
static bool deserialize_command(struct rfile *rf);
static bool deserialize_stage(struct rfile *rf);
static bool deserialize_bgm(struct rfile *rf);
static bool deserialize_volumes(struct rfile *rf);
static bool deserialize_vars(struct rfile *rf);
static void load_global_data(void);

/*
 * 初期化
 */

/*
 * セーブデータに関する初期化処理を行う
 */
bool init_save(void)
{
	int i;

	/* 再利用時のための初期化を行う */
	load_flag = false;
	restore_flag = false;
	is_save_load_mode_enabled = false;

	/* セーブスロットを初期化する */
	for (i = 0; i < SAVE_SLOTS; i++) {
		save_time[i] = 0;
		if (save_title[i] != NULL) {
			free(save_title[i]);
			save_title[i] = NULL;
		}
		if (save_message[i] != NULL) {
			free(save_message[i]);
			save_message[i] = NULL;
		}
		if (save_thumb[i] != NULL) {
			destroy_image(save_thumb[i]);
			save_thumb[i] = NULL;
		}
	}

	/* 文字列を初期化する */
	if (chapter_name != NULL)
		free(chapter_name);
	if (last_message != NULL)
		free(last_message);
	chapter_name = strdup("");
	last_message = strdup("");
	if (chapter_name == NULL || last_message == NULL) {
		log_memory();
		return false;
	}

	/* サムネイルの読み書きのためのヒープを確保する */
	if (tmp_pixels != NULL)
		free(tmp_pixels);
	tmp_pixels = malloc((size_t)(conf_save_data_thumb_width *
				     conf_save_data_thumb_height * 3));
	if (tmp_pixels == NULL) {
		log_memory();
		return false;
	}

	/* コンフィグからボタンの位置と大きさをロードする */
	load_button_conf();

	/* セーブデータから基本情報を取得する */
	load_basic_save_data();

	/* グローバルデータのロードを行う */
	load_global_data();

	return true;
}

/* コンフィグからボタンの位置と大きさをロードする */
static void load_button_conf(void)
{
	button[BUTTON_ONE].x = conf_save_data1_x;
	button[BUTTON_ONE].y = conf_save_data1_y;
	button[BUTTON_ONE].w = conf_save_data_width;
	button[BUTTON_ONE].h = conf_save_data_height;

	button[BUTTON_TWO].x = conf_save_data2_x;
	button[BUTTON_TWO].y = conf_save_data2_y;
	button[BUTTON_TWO].w = conf_save_data_width;
	button[BUTTON_TWO].h = conf_save_data_height;

	button[BUTTON_THREE].x = conf_save_data3_x;
	button[BUTTON_THREE].y = conf_save_data3_y;
	button[BUTTON_THREE].w = conf_save_data_width;
	button[BUTTON_THREE].h = conf_save_data_height;

	button[BUTTON_PREV].x = conf_save_prev_x;
	button[BUTTON_PREV].y = conf_save_prev_y;
	button[BUTTON_PREV].w = conf_save_prev_width;
	button[BUTTON_PREV].h = conf_save_prev_height;

	button[BUTTON_NEXT].x = conf_save_next_x;
	button[BUTTON_NEXT].y = conf_save_next_y;
	button[BUTTON_NEXT].w = conf_save_next_width;
	button[BUTTON_NEXT].h = conf_save_next_height;

	button[BUTTON_EXIT].x = conf_save_exit_x;
	button[BUTTON_EXIT].y = conf_save_exit_y;
	button[BUTTON_EXIT].w = conf_save_exit_width;
	button[BUTTON_EXIT].h = conf_save_exit_height;

	button[BUTTON_TITLE].x = conf_save_title_x;
	button[BUTTON_TITLE].y = conf_save_title_y;
	button[BUTTON_TITLE].w = conf_save_title_width;
	button[BUTTON_TITLE].h = conf_save_title_height;
}

/*
 * セーブデータに関する終了処理を行う
 */
void cleanup_save(void)
{
	int i;

	for (i = 0; i < SAVE_SLOTS; i++) {
		if (save_title[i] != NULL) {
			free(save_title[i]);
			save_title[i] = NULL;
		}
		if (save_message[i] != NULL) {
			free(save_message[i]);
			save_message[i] = NULL;
		}
		if (save_thumb[i] != NULL) {
			destroy_image(save_thumb[i]);
			save_thumb[i] = NULL;
		}
	}

	free(chapter_name);
	chapter_name = NULL;

	free(last_message);
	last_message = NULL;

	free(tmp_pixels);
	tmp_pixels = NULL;

	/* グローバル変数のセーブを行う */
	save_global_data();
}

/*
 * コマンドからの確認
 */

/*
 * コマンドがロードによって開始されたかを確認する
 */
bool check_load_flag(void)
{
	bool ret;

	ret = load_flag;
	load_flag = false;

	return ret;
}

/* コマンドがセーブ画面から復帰したかを確認する */
bool check_restore_flag(void)
{
	bool ret;

	ret = restore_flag;
	restore_flag = false;

	return ret;
}

/*
 * セーブ画面
 */

/*
 * セーブ画面を開始する
 */
void start_save_mode(bool is_goto_save)
{
	is_goto = is_goto_save;

	/* オートモードを解除する */
	if (is_auto_mode())
		stop_auto_mode();

	/* スキップモードを解除する */
	if (is_skip_mode())
		stop_skip_mode();

	/* セーブ画面を開始する */
	is_save_load_mode_enabled = true;
	is_save_mode = true;

	/* 最初のフレームである */
	is_first_frame = true;

	/* 選択項目を無効とする */
	pointed_index = -1;

	/* 0ページ目を表示する */
	page = 0;
}

/*
 * ロード画面を開始する
 */
void start_load_mode(bool is_goto_load)
{
	is_goto = is_goto_load;

	/* オートモードを解除する */
	if (is_auto_mode())
		stop_auto_mode();

	/* スキップモードを解除する */
	if (is_skip_mode())
		stop_skip_mode();

	/* セーブ画面を開始する */
	is_save_load_mode_enabled = true;
	is_save_mode = false;

	/* 最初のフレームである */
	is_first_frame = true;

	/* 選択項目を無効とする */
	pointed_index = -1;

	/* 0ページ目を表示する */
	page = 0;
}

/* セーブ画面を終了する */
static void stop_save_load_mode(int *x, int *y, int *w, int *h)
{
	/* セーブ画面を終了する */
	is_save_load_mode_enabled = false;

	/* ステージを再描画する */
	draw_stage_with_button(0, 0, 0, 0);

	/* ステージ全体をウィンドウに転送する */
	*x = 0;
	*y = 0;
	*w = conf_window_width;
	*h = conf_window_height;
}

/*
 * セーブ画面が有効であるかを返す
 */
bool is_save_load_mode(void)
{
	return is_save_load_mode_enabled;
}

/*
 * セーブ画面の1フレームを実行する
 */
bool run_save_load_mode(int *x, int *y, int *w, int *h)
{
	/* 最初のフレームを実行する */
	if (is_first_frame) {
		draw_page(x, y, w, h);
		is_first_frame = false;
		return true;
	}

	/* 右クリックされた場合 */
	if (is_right_button_pressed) {
		draw_page_keep();

		/* セーブ・ロードをキャンセルする */
		play_se(is_save_mode ? conf_save_cancel_save_se :
			conf_save_cancel_load_se);
		stop_save_load_mode(x, y, w, h);
		restore_flag = !is_goto;
		return true;
	}

	/* ポイントされている項目を更新する */
	if (!update_pointed_index(x, y, w, h)) {
		draw_page_keep();
		return false;	/* 終了ボタンが押下された */
	}

	/* セーブ画面を継続する */
	draw_page_keep();
	return true;
}

/* ページの描画を行う */
static void draw_page(int *x, int *y, int *w, int *h)
{
	/* FO/FIレイヤをロックする */
	lock_draw_char_on_fo_fi();

	/* ステージのレイヤの背景を描画する */
	if (is_save_mode)
		clear_save_stage();
	else
		clear_load_stage();

	/* サムネイルを描画する */
	draw_all_thumbs();

	/* セーブデータのテキストを描画する */
	draw_all_text_items();

	/* FO/FIレイヤをアンロックする */
	unlock_draw_char_on_fo_fi();

	/* 現在選択されている項目を取得する */
	pointed_index = get_pointed_index();
	if (pointed_index != -1) {
		/* 選択されているボタンがある場合 */
		draw_stage_with_button(button[pointed_index].x,
				       button[pointed_index].y,
				       button[pointed_index].w,
				       button[pointed_index].h);
	} else {
		/* 選択されているボタンがない場合 */
		draw_stage_with_button(0, 0, 0, 0);
	}

	*x = 0;
	*y = 0;
	*w = conf_window_width;
	*h = conf_window_height;
}

/* ページの描画を行う */
static void draw_page_keep(void)
{
	/* 現在選択されている項目を取得する */
	pointed_index = get_pointed_index();
	if (pointed_index != -1) {
		/* 選択されているボタンがある場合 */
		draw_stage_with_button_keep(button[pointed_index].x,
					    button[pointed_index].y,
					    button[pointed_index].w,
					    button[pointed_index].h);
	} else {
		/* 選択されているボタンがない場合 */
		draw_stage_with_button_keep(0, 0, 0, 0);
	}
}

/* ポイントされているボタンを返す */
static int get_pointed_index(void)
{
	int i, pointed, rel_x, rel_y;

	/* 領域を元にポイントされているボタンを求める */
	pointed = -1;
	for (i = 0; i < BUTTON_COUNT; i++) {
		/* ボタンがポイントされているかチェックする */
		if (mouse_pos_x >= button[i].x &&
		    mouse_pos_x < button[i].x + button[i].w &&
		    mouse_pos_y >= button[i].y &&
		    mouse_pos_y < button[i].y + button[i].h) {
			/* i番目のボタンがポイントされている */
			pointed = i;
			break;
		}
	}
	if (pointed == -1)
		return -1;

	/* セーブデータ項目が選択されている場合 */
	is_delete_button_pointed = false;
	if (pointed >= BUTTON_ONE && pointed <= BUTTON_THREE) {
		/* 削除ボタンがポイントされているかチェックする */
		rel_x = mouse_pos_x - button[pointed].x;
		rel_y = mouse_pos_y - button[pointed].y;
		if (rel_x >= conf_save_data_delete_x &&
		    rel_x <= conf_save_data_delete_x +
			     conf_save_data_delete_width &&
		    rel_y >= conf_save_data_delete_y &&
		    rel_y < conf_save_data_delete_y +
			    conf_save_data_delete_height)
			is_delete_button_pointed = true;
	}

	/* 最初のページの場合、BUTTON_PREVを無効にする */
	if (page == 0 && pointed == BUTTON_PREV)
		return -1;

	/* 最後のページの場合、BUTTON_NEXTを無効にする */
	if (page == SAVE_PAGES - 1 && pointed == BUTTON_NEXT)
		return -1;

	/* ポイントされたボタンを返す */
	return pointed;
}

/* セーブデータのサムネイルを描画する */
static void draw_all_thumbs(void)
{
	int i, base;

	/* 先頭のセーブデータの番号を求める */
	base = page * PAGE_SLOTS;

	/* 3つのセーブボタンについて描画する */
	for (i = BUTTON_ONE; i <= BUTTON_THREE; i++) {
		if (save_thumb[base + i] == NULL)
			continue;

		draw_image_on_fo_fi(button[i].x + conf_save_data_margin_left,
				    button[i].y + conf_save_data_margin_top,
				    save_thumb[base + i]);
	}
}

/* セーブデータのテキストを描画する */
static void draw_all_text_items(void)
{
	struct tm *timeptr;
	char text[128];
	int i, base, width;

	/* 先頭のセーブデータの番号を求める */
	base = page * PAGE_SLOTS;

	/* 3つのセーブボタンについて描画する */
	for (i = BUTTON_ONE; i <= BUTTON_THREE; i++) {
		/* 日時を描画する */
		if (save_time[base + i] == 0) {
			snprintf(text, sizeof(text), "[%02d] NO DATA",
				 base + i + 1);
		} else {
			timeptr = localtime(&save_time[base + i]);
			snprintf(text, sizeof(text), "[%02d] ", base + i + 1);
			strftime(&text[5], sizeof(text), "%m/%d %H:%M ",
				 timeptr);
		}
		width = draw_text_item(button[i].x +
				       conf_save_data_margin_left +
				       conf_save_data_thumb_width +
				       conf_save_data_margin_left,
				       button[i].y +
				       conf_save_data_margin_top,
				       text,
				       button[i].x);

		/* 章題を描画する */
		if (save_title[base + i] != NULL) {
			draw_text_item(button[i].x +
				       conf_save_data_margin_left +
				       conf_save_data_thumb_width +
				       conf_save_data_margin_left +
				       width,
				       button[i].y +
				       conf_save_data_margin_top,
				       save_title[base + i],
				       button[i].x);
		}

		/* メッセージを描画する */
		if (save_message[base + i] != NULL) {
			draw_text_item(button[i].x +
				       conf_save_data_margin_left +
				       conf_save_data_thumb_width +
				       conf_save_data_margin_left,
				       button[i].y +
				       conf_save_data_margin_top +
				       conf_font_size +
				       conf_save_data_margin_top,
				       save_message[base + i],
				       button[i].x);
			}
		}
		
}

/* セーブデータのテキストを描画する */
static int draw_text_item(int x, int y, const char *text, int base_x)
{
	uint32_t wc;
	int mblen, cw, result;

	/* 1文字ずつ描画する */
	result = 0;
	while (*text != '\0') {
		/* 描画する文字を取得する */
		mblen = utf8_to_utf32(text, &wc);
		if (mblen == -1)
			return 0;

		/* 文字の幅を取得する */
		cw = get_glyph_width(wc);
		if (x + cw >= base_x + conf_save_data_width)
			break;

		/* 描画する */
		cw = draw_char_on_fo_fi(x, y, wc);
		x += cw;
		result += cw;

		/* 次の文字へ移動する */
		text += mblen;
	}

	return result;
}

/*
 * ポイントされている項目を更新する
 */
bool update_pointed_index(int *x, int *y, int *w, int *h)
{
	int new_pointed_index;

	/* 現在ポイントされている項目を取得する */
	new_pointed_index = get_pointed_index();
	if (new_pointed_index != -1 && new_pointed_index != pointed_index)
		play_se(conf_save_change_se);

	/* 左クリックされた場合 */
	if (is_left_button_pressed) {
		if (!process_left_press(new_pointed_index, x, y, w, h))
			return false; /* 終了ボタンが押下された */
		return true;
	}

	/* 前回ポイントされていた項目と同じ場合は何もしない */
	if (new_pointed_index == pointed_index)
		return true;

	/* 選択されたボタンとポイントされたボタンをバックイメージに描画する */
	if (new_pointed_index != -1) {
		/* 選択中のボタンがない場合 */
		draw_stage_with_button(button[new_pointed_index].x,
				       button[new_pointed_index].y,
				       button[new_pointed_index].w,
				       button[new_pointed_index].h);
	} else {
		/* 選択中のボタンがない場合 */
		draw_stage_with_button(0, 0, 0, 0);
	}

	/* ウィンドウの更新領域を求める */
	if (new_pointed_index != -1) {
		union_rect(x, y, w, h,
			   button[new_pointed_index].x,
			   button[new_pointed_index].y,
			   button[new_pointed_index].w,
			   button[new_pointed_index].h,
			   button[pointed_index].x,
			   button[pointed_index].y,
			   button[pointed_index].w,
			   button[pointed_index].h);
	} else {
		union_rect(x, y, w, h, 0, 0, 0, 0,
			   button[pointed_index].x,
			   button[pointed_index].y,
			   button[pointed_index].w,
			   button[pointed_index].h);
	}

	/* ポイントされている項目を変更する */
	pointed_index = new_pointed_index;

	return true;
}

/* 左ボタンのクリックを処理する */
static bool process_left_press(int new_pointed_index, int *x, int *y, int *w,
			       int *h)
{
	/* ポイントされている項目がない場合 */
	if (new_pointed_index == -1)
		return true;

	/* セーブデータのボタンの場合 */
	if (new_pointed_index >= BUTTON_ONE &&
	    new_pointed_index <= BUTTON_THREE) {
		/* 削除ボタンの領域がポイントされている場合 */
		if (is_delete_button_pointed) {
			/* 削除を処理する */
			process_delete(new_pointed_index);
			draw_page(x, y, w, h);
			return true;
		}

		/* セーブかロードを実行する */
		process_left_press_save_button(new_pointed_index, x, y, w, h);
		if (is_save_mode) {
			/* セーブを行う */
			play_se(conf_save_data_save_se);
			process_save(new_pointed_index);
			draw_page(x, y, w, h);
		} else if (have_save_data(new_pointed_index)){
			/* ロードを行う */
			play_se(conf_save_data_load_se);
			process_load(new_pointed_index);
			stop_save_load_mode(x, y, w, h);
		}
		return true;
	}

	/* 前ページ、次ページボタンの場合 */
	if (new_pointed_index == BUTTON_PREV ||
	    new_pointed_index == BUTTON_NEXT) {
		if (new_pointed_index == BUTTON_PREV)
			play_se(conf_save_prev_se);
		else
			play_se(conf_save_next_se);
		page += new_pointed_index == BUTTON_PREV ? -1 : 1;
		draw_page(x, y, w, h);
		return true;
	}

	/* 戻るボタンの場合 */
	if (new_pointed_index == BUTTON_EXIT) {
		play_se(conf_save_exit_se);
		stop_save_load_mode(x, y, w, h);
		restore_flag = !is_goto;
		return true;
	}

	/* タイトルへ戻るボタンの場合 */
	if (new_pointed_index == BUTTON_TITLE) {
		play_se(conf_save_title_se);
		if (title_dialog()) {
			save_seen();
			if (!load_script(conf_save_title_txt))
				return false;
			set_mixer_input(BGM_STREAM, NULL);
			set_mixer_input(SE_STREAM, NULL);
			set_mixer_input(VOICE_STREAM, NULL);
			show_namebox(false);
			show_msgbox(false);
			stop_save_load_mode(x, y, w, h);
			return true;
		}
	}

	return true;
}

/* セーブボタン上での左クリックを処理する */
static void process_left_press_save_button(int new_pointed_index, int *x,
					   int *y, int *w, int *h)
{
	/* バックイメージに新しい選択ボタンを描画する */
	draw_stage_with_button(button[new_pointed_index].x,
			       button[new_pointed_index].y,
			       button[new_pointed_index].w,
			       button[new_pointed_index].h);

	/* ウィンドウの更新領域を求める */
	union_rect(x, y, w, h,
		   button[new_pointed_index].x,
		   button[new_pointed_index].y,
		   button[new_pointed_index].w,
		   button[new_pointed_index].h,
		   0, 0, 0, 0);

	/* ポイントされている項目を更新する */
	pointed_index = new_pointed_index;
}

/* セーブデータの削除を処理する */
static void process_delete(int new_pointed_index)
{
	char s[128];
	int index;

	/* セーブデータ番号を求める */
	index = page * PAGE_SLOTS + (new_pointed_index - BUTTON_ONE);

	/* セーブデータがない場合、何もしない */
	if (save_time[index] == 0)
		return;

	/* プロンプトを表示する */
	if (!delete_dialog())
		return;

	/* ファイル名を求める */
	snprintf(s, sizeof(s), "%03d.sav", index);

	/* セーブファイルを削除する */
	remove_file(SAVE_DIR, s);

	/* セーブデータを消去する */
	save_time[index] = 0;
	if (save_title[index] != NULL) {
		free(save_title[index]);
		save_title[index] = NULL;
	}
	if (save_message[index] != NULL) {
		free(save_message[index]);
		save_message[index] = NULL;
	}
	if (save_thumb[index] != NULL) {
		destroy_image(save_thumb[index]);
		save_thumb[index] = NULL;
	}
}

/* 選択された項目にセーブデータがあるか調べる */
static bool have_save_data(int new_pointed_index)
{
	int index;

	index = PAGE_SLOTS * page + new_pointed_index;
	if (save_time[index] == 0)
		return false;

	return true;
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

/*
 * セーブの実際の処理
 */

/*
 * クイックセーブを行う
 */
bool quick_save(void)
{
	uint64_t timestamp;

	/* ローカルデータのシリアライズを行う */
	if (!serialize_all(QUICK_SAVE_FILE_NAME, &timestamp, -1))
		return false;

	/* 既読フラグのセーブを行う */
	save_seen();

	/* グローバル変数のセーブを行う */
	save_global_data();

	/* クイックセーブの時刻を更新する */
	quick_save_time = (time_t)timestamp;

	return true;
}

/* セーブを処理する */
static bool process_save(int new_pointed_index)
{
	char s[128];
	uint64_t timestamp;
	int index;

	/* ファイル名を求める */
	index = page * PAGE_SLOTS + (new_pointed_index - BUTTON_ONE);
	snprintf(s, sizeof(s), "%03d.sav", index);

	/* ローカルデータのシリアライズを行う */
	if (!serialize_all(s, &timestamp, index))
		return false;

	/* 既読フラグのセーブを行う */
	save_seen();

	/* グローバル変数のセーブを行う */
	save_global_data();

	/* 時刻を保存する */
	save_time[index] = (time_t)timestamp;

	return true;
}

/* すべてのシリアライズを行う */
static bool serialize_all(const char *fname, uint64_t *timestamp, int index)
{
	struct wfile *wf;
	uint64_t t;
	bool success;

	/* セーブディレクトリを作成する */
	make_sav_dir();

	/* ファイルを開く */
	wf = open_wfile(SAVE_DIR, fname);
	if (wf == NULL)
		return false;

	success = false;
	do {
		/* 日付を書き込む */
		t = (uint64_t)time(NULL);
		if (write_wfile(wf, &t, sizeof(t)) < sizeof(t))
			break;

		/* 章題のシリアライズを行う */
		if (!serialize_title(wf, index))
			break;

		/* メッセージのシリアライズを行う */
		if (!serialize_message(wf, index))
			break;

		/* サムネイルのシリアライズを行う */
		if (!serialize_thumb(wf, index))
			break;

		/* コマンド位置のシリアライズを行う */
		if (!serialize_command(wf))
			break;

		/* ステージのシリアライズを行う */
		if (!serialize_stage(wf))
			break;

		/* BGMのシリアライズを行う */
		if (!serialize_bgm(wf))
			break;

		/* ボリュームのシリアライズを行う */
		if (!serialize_volumes(wf))
			break;

		/* 変数のシリアライズを行う */
		if (!serialize_vars(wf))
			break;

		/* 成功 */
		success = true;
	} while (0);

	/* ファイルをクローズする */
	close_wfile(wf);

	/* 時刻を保存する */
	*timestamp = t;

	if (!success)
		log_file_write(fname);

	return success;
}

/* 章題のシリアライズを行う */
static bool serialize_title(struct wfile *wf, int index)
{
	size_t len;

	/* 文字列を準備する */
	strncpy(tmp_str, chapter_name, sizeof(tmp_str));
	tmp_str[sizeof(tmp_str) - 1] = '\0';

	/* 書き出す */
	len = strlen(tmp_str) + 1;
	if (write_wfile(wf, tmp_str, len) < len)
		return false;

	/* 章題を保存する */
	if (save_title[index] != NULL)
		free(save_title[index]);
	save_title[index] = strdup(tmp_str);
	if (save_title[index] == NULL)
		return false;

	return true;
}

/* メッセージのシリアライズを行う */
static bool serialize_message(struct wfile *wf, int index)
{
	size_t len;

	/* 文字列を準備する */
	strncpy(tmp_str, last_message, sizeof(tmp_str));
	tmp_str[sizeof(tmp_str) - 1] = '\0';

	/* 書き出す */
	len = strlen(tmp_str) + 1;
	if (write_wfile(wf, tmp_str, len) < len)
		return false;

	/* メッセージを保存する */
	if (save_message[index] != NULL)
		free(save_message[index]);
	save_message[index] = strdup(tmp_str);
	if (save_message[index] == NULL)
		return false;

	return true;
}

/* サムネイルのシリアライズを行う */
static bool serialize_thumb(struct wfile *wf, int index)
{
	pixel_t *src, pix;
	unsigned char *dst;
	size_t len;
	int x, y;

	/* クイックセーブの場合 */
	if (index == -1) {
		/* 内容は気にせず書き出す */
		len = (size_t)(conf_save_data_thumb_width *
			       conf_save_data_thumb_height * 3);
		if (write_wfile(wf, tmp_pixels, len) < len)
			return false;
		return true;
	}

	/* stage.cのサムネイルをsave.cのイメージにコピーする */
	if (save_thumb[index] == NULL) {
		save_thumb[index] = create_image(conf_save_data_thumb_width,
						 conf_save_data_thumb_height);
		if (save_thumb[index] == NULL)
			return false;
	}
	lock_image(save_thumb[index]);
	draw_image(save_thumb[index], 0, 0, get_thumb_image(),
		   conf_save_data_thumb_width, conf_save_data_thumb_height, 0, 0,
		   255, BLEND_NONE);
	unlock_image(save_thumb[index]);

	/* ピクセル列を準備する */
	src = get_image_pixels(get_thumb_image());
	dst = tmp_pixels;
	for (y = 0; y < conf_save_data_thumb_height; y++) {
		for (x = 0; x < conf_save_data_thumb_width; x++) {
			pix = *src++;
			*dst++ = (unsigned char)get_pixel_r_slow(pix);
			*dst++ = (unsigned char)get_pixel_g_slow(pix);
			*dst++ = (unsigned char)get_pixel_b_slow(pix);
		}
	}

	/* 書き出す */
	len = (size_t)(conf_save_data_thumb_width *
		       conf_save_data_thumb_height * 3);
	if (write_wfile(wf, tmp_pixels, len) < len)
		return false;

	return true;
}

/* コマンド位置をシリアライズする */
static bool serialize_command(struct wfile *wf)
{
	const char *s;
	int n, m;

	/* スクリプトファイル名を取得してシリアライズする */
	s = get_script_file_name();
	if (write_wfile(wf, s, strlen(s) + 1) < strlen(s) + 1)
		return false;

	/* コマンドインデックスを取得してシリアライズする */
	n = get_command_index();
	if (write_wfile(wf, &n, sizeof(n)) < sizeof(n))
		return false;

	/* '@gosub'のリターンポイントを取得してシリアライズする */
	m = get_return_point();
	if (write_wfile(wf, &m, sizeof(m)) < sizeof(m))
		return false;

	return true;
}

/* ステージをシリアライズする */
static bool serialize_stage(struct wfile *wf)
{
	const char *s;
	int i, m, n, o;

	s = get_bg_file_name();
	if (s == NULL)
		s = "none";
	if (write_wfile(wf, s, strlen(s) + 1) < strlen(s) + 1)
		return false;

	for (i = CH_BACK; i <= CH_CENTER; i++) {
		get_ch_position(i, &m, &n);
		o = get_ch_alpha(i);
		if (write_wfile(wf, &m, sizeof(m)) < sizeof(m))
			return false;
		if (write_wfile(wf, &n, sizeof(n)) < sizeof(n))
			return false;
		if (write_wfile(wf, &o, sizeof(o)) < sizeof(o))
			return false;

		s = get_ch_file_name(i);
		if (s == NULL)
			s = "none";
		if (write_wfile(wf, s, strlen(s) + 1) < strlen(s) + 1)
			return false;
	}

	return true;
}

/* BGMをシリアライズする */
static bool serialize_bgm(struct wfile *wf)
{
	const char *s;

	s = get_bgm_file_name();
	if (s == NULL)
		s = "none";
	if (write_wfile(wf, s, strlen(s) + 1) < strlen(s) + 1)
		return false;

	return true;
}

/* ボリュームをシリアライズする */
static bool serialize_volumes(struct wfile *wf)
{
	float vol;
	int n;

	for (n = 0; n < MIXER_STREAMS; n++) {
		vol = get_mixer_volume(n);
		if (write_wfile(wf, &vol, sizeof(vol)) < sizeof(vol))
			return false;
	}

	return true;
}

/* ローカル変数をシリアライズする */
static bool serialize_vars(struct wfile *wf)
{
	size_t len;

	len = LOCAL_VAR_SIZE * sizeof(int32_t);
	if (write_wfile(wf, get_local_variables_pointer(), len) < len)
		return false;

	return true;
}

/*
 * ロードの実際の処理
 */

/*
 * クイックセーブデータがあるか
 */
bool have_quick_save_data(void)
{
	if (quick_save_time == 0)
		return false;

	return true;
}

/*
 * クイックロードを行う Do quick load
 */
bool quick_load(void)
{
	/* 既読フラグのセーブを行う */
	save_seen();

	/* グローバル変数のセーブを行う */
	save_global_data();

	/* ローカルデータのデシリアライズを行う */
	if (!deserialize_all(QUICK_SAVE_FILE_NAME))
		return false;

	/* 名前ボックス、メッセージボックス、選択ボックスを非表示とする */
	show_namebox(false);
	show_msgbox(false);

	/* ウィンドウタイトルをアップデートする */
	update_window_title();

	/* オートモードを解除する */
	if (is_auto_mode())
		stop_auto_mode();

	/* スキップモードを解除する */
	if (is_skip_mode())
		stop_skip_mode();

#ifdef USE_DEBUGGER
	clear_variable_changed();
	update_debug_info(true);
#endif
	return true;
}

/* ロードを処理する */
static bool process_load(int new_pointed_index)
{
	char s[128];
	int index;

	/* ファイル名を求める */
	index = page * PAGE_SLOTS + (new_pointed_index - BUTTON_ONE);
	snprintf(s, sizeof(s), "%03d.sav", index);

	/* 既読フラグのセーブを行う */
	save_seen();

	/* グローバル変数のセーブを行う */
	save_global_data();

	/* ローカルデータのデシリアライズを行う */
	if (!deserialize_all(s))
		return false;

	/* 名前ボックス、メッセージボックス、選択ボックスを非表示とする */
	show_namebox(false);
	show_msgbox(false);

	/* ウィンドウタイトルをアップデートする */
	update_window_title();
	
#ifdef USE_DEBUGGER
	clear_variable_changed();
	update_debug_info(true);
#endif
	return true;
}

/* すべてをデシリアライズする */
static bool deserialize_all(const char *fname)
{
	struct rfile *rf;
	uint64_t t;
	size_t img_size;
	bool success;

	/* ファイルを開く */
	rf = open_rfile(SAVE_DIR, fname, false);
	if (rf == NULL)
		return false;

	success = false;
	do {
		/* 日付を読み込む (読み飛ばす) */
		if (read_rfile(rf, &t, sizeof(t)) < sizeof(t))
			break;

		/* 章題を読み込む */
		if (gets_rfile(rf, tmp_str, sizeof(tmp_str)) == NULL)
			break;
		if (!set_chapter_name(tmp_str))
			break;

		/* メッセージを読み込む (読み飛ばす) */
		if (gets_rfile(rf, tmp_str, sizeof(tmp_str)) == NULL)
			break;

		/* サムネイルを読み込む (読み飛ばす) */
		img_size = (size_t)(conf_save_data_thumb_width *
				    conf_save_data_thumb_height * 3);
		if (read_rfile(rf, tmp_pixels, img_size) < img_size)
			break;

		/* コマンド位置のデシリアライズを行う */
		if (!deserialize_command(rf))
			break;

		/* ステージのデシリアライズを行う */
		if (!deserialize_stage(rf))
			break;

		/* BGMのデシリアライズを行う */
		if (!deserialize_bgm(rf))
			break;

		/* ボリュームのデシリアライズを行う */
		if (!deserialize_volumes(rf))
			break;

		/* 変数のデシリアライズを行う */
		if (!deserialize_vars(rf))
			break;
		
		/* ヒストリをクリアする */
		clear_history();

		/* ロードが実行された直後であることをマークする */
		load_flag = true;

		/* 成功 */
		success = true;
	} while (0);

	/* ファイルをクローズする */
	close_rfile(rf);

	return success;
}

/* コマンド位置のデシリアライズを行う */
static bool deserialize_command(struct rfile *rf)
{
	char s[1024];
	int n, m;

	if (gets_rfile(rf, s, sizeof(s)) == NULL)
		return false;

	if (!load_script(s))
		return false;

	if (read_rfile(rf, &n, sizeof(n)) < sizeof(n))
		return false;

	if (read_rfile(rf, &m, sizeof(m)) < sizeof(m))
		return false;

	if (!move_to_command_index(n))
		return false;

	if (!set_return_point(m))
		return false;

	return true;
}

/* ステージのデシリアライズを行う */
static bool deserialize_stage(struct rfile *rf)
{
	char s[1024];
	struct image *img;
	int m, n, o, i;

	if (gets_rfile(rf, s, sizeof(s)) == NULL)
		return false;

	if (strcmp(s, "none") == 0) {
		set_bg_file_name(NULL);
		img = create_initial_bg();
		if (img == NULL)
			return false;;
	} else if (s[0] == '#') {
		set_bg_file_name(s);
		img = create_image_from_color_string(conf_window_width,
						     conf_window_height,
						     &s[1]);
		if (img == NULL)
			return false;
	} else {
		set_bg_file_name(s);
		img = create_image_from_file(BG_DIR, s);
		if (img == NULL)
			return false;
	}

	change_bg_immediately(img);

	for (i = CH_BACK; i <= CH_CENTER; i++) {
		if (read_rfile(rf, &m, sizeof(m)) < sizeof(n))
			return false;
		if (read_rfile(rf, &n, sizeof(n)) < sizeof(m))
			return false;
		if (read_rfile(rf, &o, sizeof(o)) < sizeof(o))
			return false;
		if (gets_rfile(rf, s, sizeof(s)) == NULL)
			return false;

		assert(strcmp(s, "") != 0);
		if (strcmp(s, "none") == 0) {
			set_ch_file_name(i, NULL);
			img = NULL;
		} else {
			set_ch_file_name(i, s);
			img = create_image_from_file(CH_DIR, s);
			if (img == NULL)
				return false;
		}

		change_ch_immediately(i, img, m, n, o);
	}

	return true;
}

/* BGMをデシリアライズする */
static bool deserialize_bgm(struct rfile *rf)
{
	char s[1024];
	struct wave *w;

	if (gets_rfile(rf, s, sizeof(s)) == NULL)
		return false;

	if (strcmp(s, "none") == 0) {
		set_bgm_file_name(NULL);
		w = NULL;
	} else {
		set_bgm_file_name(s);
		w = create_wave_from_file(BGM_DIR, s, true);
		if (w == NULL)
			return false;
	}

	set_mixer_input(BGM_STREAM, w);

	return true;
}

/* ボリュームをデシリアライズする */
static bool deserialize_volumes(struct rfile *rf)
{
	float vol;
	int n;

	for (n = 0; n < MIXER_STREAMS; n++) {
		if (read_rfile(rf, &vol, sizeof(vol)) < sizeof(vol))
			return false;
		set_mixer_volume(n, vol, 0);
	}

	return true;
}

/* ローカル変数をデシリアライズする */
static bool deserialize_vars(struct rfile *rf)
{
	size_t len;

	len = LOCAL_VAR_SIZE * sizeof(int32_t);
	if (read_rfile(rf, get_local_variables_pointer(), len) < len)
		return false;

	return true;
}

/* セーブデータから基本情報を読み込む */
static void load_basic_save_data(void)
{
	struct rfile *rf;
	char buf[128];
	uint64_t t;
	int i;

	/* セーブスロットごとに読み込む */
	for (i = 0; i < SAVE_SLOTS; i++) {
		/* セーブデータファイルを開く */
		snprintf(buf, sizeof(buf), "%03d.sav", i);
		rf = open_rfile(SAVE_DIR, buf, true);
		if (rf != NULL) {
			/* 読み込む */
			load_basic_save_data_file(rf, i);
			close_rfile(rf);
		}
	}

	/* セーブデータファイルを開く */
	rf = open_rfile(SAVE_DIR, QUICK_SAVE_FILE_NAME, true);
	if (rf != NULL) {
		/* セーブ時刻を取得する */
		if (read_rfile(rf, &t, sizeof(t)) == sizeof(t))
			quick_save_time = (time_t)t;
		close_rfile(rf);
	}
}

/* セーブデータファイルから基本情報を読み込む */
static void load_basic_save_data_file(struct rfile *rf, int index)
{
	uint64_t t;
	size_t img_size;
	pixel_t *dst;
	const unsigned char *src;
	uint32_t r, g, b;
	int x, y;

	/* セーブ時刻を取得する */
	if (read_rfile(rf, &t, sizeof(t)) < sizeof(t))
		return;
	save_time[index] = (time_t)t;

	/* 章題を取得する */
	if (gets_rfile(rf, tmp_str, sizeof(tmp_str)) == NULL)
		return;
	save_title[index] = strdup(tmp_str);
	if (save_title[index] == NULL) {
		log_memory();
		return;
	}

	/* メッセージを取得する */
	if (gets_rfile(rf, tmp_str, sizeof(tmp_str)) == NULL)
		return;
	save_message[index] = strdup(tmp_str);
	if (save_message[index] == NULL) {
		log_memory();
		return;
	}

	/* サムネイルを取得する */
	img_size = (size_t)(conf_save_data_thumb_width *
			    conf_save_data_thumb_height * 3);
	if (read_rfile(rf, tmp_pixels, img_size) < img_size)
		return;

	/* サムネイルの画像を生成する */
	save_thumb[index] = create_image(conf_save_data_thumb_width,
					 conf_save_data_thumb_height);
	if (save_thumb[index] == NULL)
		return;
	lock_image(save_thumb[index]);
	dst = get_image_pixels(save_thumb[index]);
	src = tmp_pixels;
	for (y = 0; y < conf_save_data_thumb_height; y++) {
		for (x = 0; x < conf_save_data_thumb_width; x++) {
			r = *src++;
			g = *src++;
			b = *src++;
			*dst++ = make_pixel_slow(0xff, r, g, b);
		}
	}
	unlock_image(save_thumb[index]);
}

/*
 * グローバル変数
 */

/* グローバルデータのロードを行う */
static void load_global_data(void)
{
	struct rfile *rf;
	float f;
	int i;

	/* ファイルを開く */
	rf = open_rfile(SAVE_DIR, GLOBAL_VARS_FILE, true);
	if (rf == NULL)
		return;

	/* グローバル変数をデシリアライズする */
	read_rfile(rf, get_global_variables_pointer(),
		   GLOBAL_VAR_SIZE * sizeof(int32_t));

	/* マスターボリュームをデシリアライズする */
	for (i = 0; i < MIXER_STREAMS; i++) {
		if (read_rfile(rf, &f, sizeof(f)) < sizeof(f))
			break;
		set_mixer_master_volume(i, f);
		/*
		 * load_global_data()はinit_mixer()より後に呼ばれるので、
		 * ここでset_mixer_master_volume()を呼んでも、
		 * init_mixer()に上書きされることはない。
		 */
	}

	/* ファイルを閉じる */
	close_rfile(rf);
}

/*
 * グローバルデータのセーブを行う
 */
void save_global_data(void)
{
	struct wfile *wf;
	float f;
	int i;

	/* セーブディレクトリを作成する */
	make_sav_dir();

	/* ファイルを開く */
	wf = open_wfile(SAVE_DIR, GLOBAL_VARS_FILE);
	if (wf == NULL)
		return;

	/* グローバル変数をシリアライズする */
	write_wfile(wf, get_global_variables_pointer(),
		    GLOBAL_VAR_SIZE * sizeof(int32_t));

	/* マスターボリュームをシリアライズする */
	for (i = 0; i < MIXER_STREAMS; i++) {
		f = get_mixer_master_volume(i);
		if (write_wfile(wf, &f, sizeof(f)) < sizeof(f))
			break;
	}

	/* ファイルを閉じる */
	close_wfile(wf);
}

/*
 * 章題と最後のメッセージ
 */

/*
 * 章題を設定する
 */
bool set_chapter_name(const char *name)
{
	free(chapter_name);

	chapter_name = strdup(name);
	if (chapter_name == NULL) {
		log_memory();
		return false;
	}

	return true;
}

/*
 * 章題を取得する
 */
const char *get_chapter_name(void)
{
	if (chapter_name == NULL)
		return "";

	return chapter_name;
}

/*
 * 最後のメッセージを設定する
 */
bool set_last_message(const char *msg)
{
	free(last_message);

	last_message = strdup(msg);
	if (last_message == NULL) {
		log_memory();
		return false;
	}

	return true;
}
