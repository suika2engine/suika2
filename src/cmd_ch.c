/* -*- coding: utf-8; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Suika2
 * Copyright (C) 2001-2023, Keiichi Tabata. All rights reserved.
 */

/*
 * [Changes]
 *  - 2016/06/09 作成
 *  - 2021/06/10 マスクつき描画の対応
 *  - 2023/01/06 日本語の位置名に対応
 */

#include "suika.h"

static uint64_t sw;
static float span;

static bool init(void);
static bool get_position(const char *pos, struct image *img, int ofs_x,
			 int ofs_y, int *chpos, int *xpos, int *ypos);
static int get_alpha(const char *alpha);
static void focus_character(int chpos, const char *fname);
static void draw(void);
static bool cleanup(void);

/*
 * キャラクタコマンド
 */
bool ch_command(void)
{
	if (!is_in_command_repetition())
		if (!init())
			return false;

	draw();

	if (!is_in_command_repetition())
		if (!cleanup())
			return false;

	return true;
}

/*
 * コマンドの初期化処理を行う
 */
static bool init(void)
{
	struct image *img, *rule_img;
	const char *fname;
	const char *pos;
	const char *method;
	const char *alpha_s;
	int xpos, ypos, chpos, ofs_x, ofs_y, alpha;
	int fade_method;

	/* パラメータを取得する */
	pos = get_string_param(CH_PARAM_POS);
	fname = get_string_param(CH_PARAM_FILE);
	span = get_float_param(CH_PARAM_SPAN);
	method = get_string_param(CH_PARAM_METHOD);
	ofs_x = get_int_param(CH_PARAM_OFFSET_X);
	ofs_y = get_int_param(CH_PARAM_OFFSET_Y);
	alpha_s = get_string_param(CH_PARAM_ALPHA);

	/* キャラの消去が指定されているかチェックする */
	if (strcmp(fname, "none") == 0 || strcmp(fname, U8("消去")) == 0)
		fname = NULL;

	/* イメージが指定された場合 */
	if (fname != NULL) {
		/* イメージを読み込む */
		img = create_image_from_file(CH_DIR, fname);
		if (img == NULL) {
			log_script_exec_footer();
			return false;
		}
	} else {
		/* イメージが指定されなかった場合(消す) */
		img = NULL;
	}

	/* キャラの位置と座標を取得する */
	if (!get_position(pos, img, ofs_x, ofs_y, &chpos, &xpos, &ypos))
		return false;

	/* フェードの種類を求める */
	fade_method = get_fade_method(method);
	if (fade_method == FADE_METHOD_INVALID) {
		/* スクリプト実行エラー */
		log_script_fade_method(method);
		log_script_exec_footer();
		return false;
	}

	/* ルールが使用される場合 */
	if (fade_method == FADE_METHOD_RULE ||
	    fade_method == FADE_METHOD_MELT) {
		/* ルールファイルが指定されていない場合 */
		if (strcmp(&method[5], "") == 0) {
			log_script_rule();
			log_script_exec_footer();
			return false;
		}

		/* イメージを読み込む */
		rule_img = create_image_from_file(RULE_DIR, &method[5]);
		if (rule_img == NULL) {
			log_script_exec_footer();
			return false;
		}
	} else {
		rule_img = NULL;
	}

	/* アルファ値を求める */
	alpha = get_alpha(alpha_s);

	/* 発話中以外のキャラを暗くする */
	if (chpos != CH_FACE)
		focus_character(chpos, fname);

	/* メッセージボックスを消す */
	if (!conf_msgbox_show_on_ch) {
		show_namebox(false);
		show_msgbox(false);
	}
	show_click(false);

	/* キャラフェードモードを有効にする */
	if (!start_fade_for_ch(chpos, fname, img, xpos, ypos, alpha,
			       fade_method, rule_img)) {
		log_script_exec_footer();
		return false;
	}

	/* 繰り返し動作を開始する */
	start_command_repetition();

	/* 時間計測を開始する */
	reset_lap_timer(&sw);

	return true;
}

/* キャラの横方向の位置を取得する */
static bool get_position(const char *pos,	/* IN: character position name */
			 struct image *img,	/* IN: character image (or NULL)*/
			 int ofs_x,		/* IN: offset x */
			 int ofs_y,		/* IN: offset y */
			 int *chpos,		/* OUT: character position index */
			 int *xpos,		/* OUT: character x */
			 int *ypos)		/* OUT: character y */
{
	*xpos = 0;

	if (strcmp(pos, "back") == 0 ||
	    strcmp(pos, "b") == 0 ||
	    strcmp(pos, U8("背面")) == 0) {
		/* 中央背面に配置する */
		*chpos = CH_BACK;
		if (img != NULL)
			*xpos = (conf_window_width - img->width) / 2 + ofs_x;
		else
			*xpos = 0;
	} else if (strcmp(pos, "left") == 0 ||
		   strcmp(pos, "l") == 0 ||
		   strcmp(pos, U8("左")) == 0) {
		/* 左に配置する */
		*chpos = CH_LEFT;
		if (img != NULL)
			*xpos = conf_stage_ch_margin_left + ofs_x;
		else
			*xpos = 0;
	} else if (strcmp(pos, "left-center") == 0 ||
		   strcmp(pos, "left-centre") == 0 ||
		   strcmp(pos, "lc") == 0 ||
		   strcmp(pos, U8("左中")) == 0) {
		/* 左中に配置する */
		*chpos = CH_LEFT_CENTER;
		if (img != NULL)
			*xpos = (conf_window_width - img->width) / 4 + ofs_x;
		else
			*xpos = 0;
	} else if (strcmp(pos, "right") == 0 ||
		   strcmp(pos, "r") == 0 ||
		   strcmp(pos, U8("右")) == 0) {
		/* 右に配置する */
		*chpos = CH_RIGHT;
		if (img != NULL)
			*xpos = conf_window_width - img->width - conf_stage_ch_margin_right + ofs_x;
		else
			*xpos = 0;
	} else if (strcmp(pos, "right-center") == 0 ||
		   strcmp(pos, "right-centre") == 0 ||
		   strcmp(pos, "rc") == 0 ||
		   strcmp(pos, U8("右中")) == 0) {
		/* 右中に配置する */
		*chpos = CH_RIGHT_CENTER;
		if (img != NULL)
			*xpos = (conf_window_width - img->width) - img->width / 2 + ofs_x;
		else
			*xpos = 0;
	} else if (strcmp(pos, "center") == 0 ||
		   strcmp(pos, "centre") == 0 ||
		   strcmp(pos, "c") == 0 ||
		   strcmp(pos, U8("中央")) == 0) {
		/* 中央に配置する */
		*chpos = CH_CENTER;
		if (img != NULL)
			*xpos = (conf_window_width - img->width) / 2 + ofs_x;
		else
			*xpos = 0;
	} else if (strcmp(pos, "face") == 0 ||
		   strcmp(pos, "f") == 0 ||
		   strcmp(pos, U8("顔")) == 0) {
		/* 顔に配置する */
		*chpos = CH_FACE;
		if (img != NULL)
			*xpos = conf_stage_ch_margin_left + ofs_x;
		else
			*xpos = 0;
	} else {
		/* スクリプト実行エラー */
		log_script_ch_position(pos);
		log_script_exec_footer();
		return false;
	}

	/* 縦方向の位置を求める */
	if (img != NULL) {
		*ypos = conf_window_height -
			img->height -
			conf_stage_ch_margin_bottom +
			ofs_y;
	} else {
		*ypos = 0;
	}

	return true;
}

/* アルファ値を取得する */
static int get_alpha(const char *alpha)
{
	if (strcmp(alpha, "show") == 0 ||
	    strcmp(alpha, "") == 0)
		return 255;
	else if (strcmp(alpha, "hide") == 0)
		return 0;
	else
		return atoi(alpha);
}

/* キャラクタのフォーカスを行う */
static void focus_character(int chpos, const char *fname)
{
	int i;

	/* 名前が登録されているキャラクタであるかチェックする */
	for (i = 0; i < CHARACTER_MAP_COUNT; i++) {
		if (conf_character_name[i] == NULL)
			continue;
		if (conf_character_file[i] == NULL)
			continue;
		if (fname == NULL)
			continue;
		if (strncmp(conf_character_file[i], fname, strlen(conf_character_file[i])) == 0)
			break;
	}
	if (i == CHARACTER_MAP_COUNT)
		i = -1;

	/* キャラ位置chposのキャラ番号を設定する */
	set_ch_name_mapping(chpos, i);

	/* 自動明暗を使い(!=0)、かつ、登場時に明るくする(==1)、かつ、未登録キャラの場合(-1) */
	if (conf_character_focus == 1 && i == -1)
		set_ch_talking(-1);

	/* 自動明暗を使う(!=0)場合、明暗を更新する */
	if (conf_character_focus != 0)
		update_ch_dim_by_talking_ch();
}

/* 描画を行う */
static void draw(void)
{
	float lap;

	/* 経過時間を取得する */
	lap = (float)get_lap_timer_millisec(&sw) / 1000.0f;
	if (lap >= span)
		lap = span;

	/* 入力に反応する */
	if (is_auto_mode() &&
	    (is_control_pressed || is_return_pressed ||
	     is_left_clicked || is_down_pressed)) {
		/* 入力によりオートモードを終了する */
		stop_auto_mode();
		show_automode_banner(false);

		/* 繰り返し動作を終了する */
		stop_command_repetition();

		/* フェードを終了する */
		finish_fade();
	} else if (is_skip_mode() &&
		   (is_control_pressed || is_return_pressed ||
		    is_left_clicked || is_down_pressed)) {
		/* 入力によりスキップモードを終了する */
		stop_skip_mode();
		show_skipmode_banner(false);

		/* 繰り返し動作を終了する */
		stop_command_repetition();

		/* フェードを終了する */
		finish_fade();
	} else if ((lap >= span)
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
		finish_fade();

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
		set_fade_progress(lap / span);
	}

	/* ステージを描画する */
	if (is_in_command_repetition())
		render_fade();
	else
		render_stage();

	/* 折りたたみシステムメニューを描画する */
	if (conf_sysmenu_transition && !is_non_interruptible())
		render_collapsed_sysmenu(false);
}

/* 終了処理を行う */
static bool cleanup(void)
{
	/* 次のコマンドに移動する */
	if (!move_to_next_command())
		return false;

	return true;
}
