/* -*- coding: utf-8-with-signature; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Suika 2
 * Copyright (C) 2001-2017, TABATA Keiichi. All rights reserved.
 */

/*
 * [Changes]
 *  - 2016/06/24 作成
 *  - 2017/01/30 "\n"対応
 *  - 2017/04/13 ワードラッピング対応
 */

#include "suika.h"
#include <ctype.h>

/* Unicodeコードポイント */
#define CHAR_SPACE	(0x0020)
#define CHAR_COMMA 	(0x002c)
#define CHAR_PERIOD	(0x002e)
#define CHAR_COLON	(0x003a)
#define CHAR_SEMICOLON  (0x003b)
#define CHAR_TOUTEN	(0x3001)
#define CHAR_KUTEN	(0x3002)
#define CHAR_BACKSLASH	(0x005c)
#define CHAR_YENSIGN	(0x00a5)
#define CHAR_SMALLN	(0x006e)

/* ビープ音ファイルあたりの文字数 */
#define BEEP_CHARS	(6)

/* コマンドの経過時刻を表すストップウォッチ */
static stop_watch_t sw;

/* 描画する文字の総数 */
static int total_chars;

/* 前回までに描画した文字数 */
static int drawn_chars;

/* スペースの直後であるか */
static bool is_after_space;

/* 描画位置 */
static int pen_x;
static int pen_y;

/* メッセージボックスの幅 */
static int msgbox_x;
static int msgbox_y;
static int msgbox_w;
static int msgbox_h;

/* 描画する矩形 */
static int draw_x;
static int draw_y;
static int draw_w;
static int draw_h;

/* 描画するメッセージ */
static char *msg_top;
static const char *msg;

/* 文字の色 */
static pixel_t color;

/* クリックアニメーションの初回描画を処理すべきか */
static bool process_click_first;

/* クリックアニメーションの表示状態 */
static bool is_click_visible;

/* スペースキーによる非表示が実行中であるか */
static bool is_hidden;

/* ヒストリ画面から戻ったばかりであるか */
static bool history_flag;

/* セーブ画面から戻ったばかりであるか */
static bool restore_flag;

/* 文字列がエスケープ中か */
static bool escaped;

/*
 * 前方参照
 */
static bool init(void);
static bool register_message_for_history(void);
static bool process_serif_command(void);
static void draw_namebox(void);
static int get_namebox_width(void);
static bool play_voice(void);
static void draw_msgbox(void);
static int get_frame_chars(void);
static void draw_click(void);
static int get_en_word_width(void);
static pixel_t get_message_color(void);
static bool cleanup(void);

/*
 * メッセージ・セリフコマンド
 */
bool message_command(int *x, int *y, int *w, int *h)
{
	draw_x = 0;
	draw_y = 0;
	draw_w = 0;
	draw_h = 0;

	/* 初期化処理を行う */
	if (!is_in_command_repetition())
		if (!init())
			return false;

	/* セーブ画面への遷移を処理する */
	if (is_right_button_pressed) {
		start_save_mode(false);
		stop_command_repetition();
		free(msg_top);
		show_click(false);
		return true;
	}

	/* ヒストリ画面への遷移を処理する */
	if (is_up_pressed) {
		start_history_mode();
		stop_command_repetition();
		free(msg_top);
		return true;
	}

	/* 繰り返し動作を行う */
	if (!is_hidden && is_space_pressed) {
		is_hidden = true;
		if (get_command_type() == COMMAND_SERIF)
			show_namebox(false);
		show_msgbox(false);
		show_click(false);
		draw_x = 0;
		draw_y = 0;
		draw_w = conf_window_width;
		draw_h = conf_window_height;
	} else if(is_hidden && !is_space_pressed) {
		is_hidden = false;
		if (get_command_type() == COMMAND_SERIF)
			show_namebox(true);
		show_msgbox(true);
		show_click(true);
		draw_x = 0;
		draw_y = 0;
		draw_w = conf_window_width;
		draw_h = conf_window_height;
	} else if (!is_hidden) {
		if (is_control_pressed)
			stop_sound(VOICE_STREAM);
		if (drawn_chars < total_chars)
			draw_msgbox();
		else
			draw_click();
	}

	/* 終了処理を行う */
	if (!is_in_command_repetition())
		if (!cleanup())
			return false;

	/* ステージを描画する */
	draw_stage_rect(draw_x, draw_y, draw_w, draw_h);

	/* 描画した範囲をウィンドウの更新範囲とする */
	*x = draw_x;
	*y = draw_y;
	*w = draw_w;
	*h = draw_h;

	return true;
}

/* 初期化処理を行う */
static bool init(void)
{
	const char *raw_msg;

	/* メッセージを取得する */
	raw_msg = get_command_type() == COMMAND_MESSAGE ?
		get_line_string() : get_string_param(SERIF_PARAM_MESSAGE);
	msg_top = expand_variable(raw_msg);
	msg = msg_top;

	/* ヒストリ画面用にメッセージ履歴を登録する */
	if (!register_message_for_history())
		return false;

	/* 文字色を求める */
	color = get_message_color();

	/* セリフの場合を処理する */
	if (!process_serif_command())
		return false;

	/* セリフが登録・表示されたことを記録する */
	if (!is_message_registered())
		set_message_registered();

	/*メッセージの文字数を求める */
	total_chars = utf8_chars(msg);
	drawn_chars = 0;

	/* 先頭文字はスペースの直後とみなす */
	is_after_space = true;

	/* メッセージの描画位置を初期化する */
	pen_x = conf_msgbox_margin_left;
	pen_y = conf_msgbox_margin_top;

	/* メッセージボックスの矩形を取得する */
	get_msgbox_rect(&msgbox_x, &msgbox_y, &msgbox_w, &msgbox_h);

	/* メッセージボックスをクリアする */
	clear_msgbox();

	/* メッセージボックスを表示する */
	show_msgbox(true);

	/* クリックアニメーションを非表示の状態にする */
	show_click(false);
	process_click_first = true;
	is_click_visible = false;

	/* スペースキーによる非表示でない状態にする */
	is_hidden = false;
	
	/* 繰り返し動作を設定する */
	if (is_control_pressed) {
		/* 繰り返し動作せず、すぐに表示する */
	} else {
		/* コマンドが繰り返し呼び出されるようにする */
		start_command_repetition();

		/* 時間計測を開始する */
		reset_stop_watch(&sw);
	}

	/* 初回に描画する矩形を求める */
	if (check_menu_finish_flag() || check_retrospect_finish_flag()) {
		/* メニューコマンドが終了したばかりの場合 */
		draw_x = 0;
		draw_y = 0;
		draw_w = conf_window_width;
		draw_h = conf_window_height;
	} else {
		/* それ以外の場合 */
		union_rect(&draw_x, &draw_y, &draw_w, &draw_h, draw_x, draw_y,
			   draw_w, draw_h, msgbox_x, msgbox_y, msgbox_w,
			   msgbox_h);
	}
	return true;
}

/* ヒストリ画面用にメッセージ履歴を登録する */
static bool register_message_for_history(void)
{
	const char *name;
	const char *voice;

	/* ヒストリ画面から戻ったばかりの場合、2重登録を防ぐ */
	history_flag = check_history_flag();
	if (history_flag)
		return true;

	/* セーブ画面から戻ったばかりの場合、2重登録を防ぐ */
	restore_flag = check_restore_flag();
	if (restore_flag && is_message_registered())
		return true;

	/* 名前、ボイスファイル名、メッセージを取得する */
	if (get_command_type() == COMMAND_SERIF) {
		name = get_string_param(SERIF_PARAM_NAME);
		voice = get_string_param(SERIF_PARAM_VOICE);
	} else {
		name = NULL;
		voice = NULL;
	}

	/* ヒストリ画面用に登録する */
	if (!register_message(name, msg, voice))
		return false;

	return true;
}
		
/* セリフコマンドを処理する */
static bool process_serif_command(void)
{
	int namebox_x, namebox_y, namebox_w, namebox_h;

	/*
	 * 描画範囲を更新する
	 *  - セリフコマンド以外でも、名前ボックス領域を消すために描画する
	 */
	get_namebox_rect(&namebox_x, &namebox_y, &namebox_w, &namebox_h);
	union_rect(&draw_x, &draw_y, &draw_w, &draw_h, draw_x, draw_y, draw_w,
		   draw_h, namebox_x, namebox_y, namebox_w, namebox_h);

	/* セリフコマンドではない場合 */
	if (get_command_type() != COMMAND_SERIF) {
		/* 名前ボックスを表示しない */
		show_namebox(false);
		return true;
	}

	/* ボイスを再生する */
	if (!is_control_pressed && !history_flag &&
	    (!restore_flag || !is_message_registered())) {
		if (!play_voice())
			return false;
	}

	/* 名前を描画する */
	draw_namebox();

	/* 名前ボックスを表示する */
	show_namebox(true);

	return true;
}

/* 名前ボックスを描画する */
static void draw_namebox(void)
{
	uint32_t c;
	int char_count, mblen, i, x, w;
	const char *name;

	/* 名前の文字列を取得する */
	name = get_string_param(SERIF_PARAM_NAME);

	/* 名前の文字数を取得する */
	char_count = utf8_chars(name);
	if (char_count == 0)
		return;

	/* 描画位置を決める */
	x = (get_namebox_width() - get_utf8_width(name)) / 2;

	/* 名前ボックスをクリアする */
	clear_namebox();

	/* 1文字ずつ描画する */
	for (i = 0; i < char_count; i++) {
		/* 描画する文字を取得する */
		mblen = utf8_to_utf32(name, &c);
		if (mblen == -1)
			return;

		/* 描画する */
		w = draw_char_on_namebox(x, conf_namebox_margin_top, c, color);

		/* 次の文字へ移動する */
		x += w;
		name += mblen;
	}
}

/* 名前ボックスの幅を取得する */
static int get_namebox_width(void)
{
	int x, y, w, h;

	get_namebox_rect(&x, &y, &w, &h);

	return w;
}

/* ボイスを再生する */
static bool play_voice(void)
{
	struct wave *w;
	const char *voice;
	int times;
	bool repeat;

	/* ボイスのファイル名を取得する */
	voice = get_string_param(SERIF_PARAM_VOICE);
	repeat = voice[0] == '@';
	voice = repeat ? &voice[1] : voice;
	if (strcmp(voice, "") == 0)
		return true;

	/* PCMストリームを開く */
	w = create_wave_from_file(CV_DIR, voice, repeat);
	if (w == NULL) {
		log_script_exec_footer();
		return false;
	}

	/* ビープ音用のリピート回数をセットする */
	if (repeat) {
		times = utf8_chars(get_string_param(SERIF_PARAM_MESSAGE));
		times /= BEEP_CHARS;
		times = times == 0 ? 1 : times;
		set_wave_repeat_times(w, times);
	}

	/* PCMストリームを再生する */
	set_mixer_input(VOICE_STREAM, w);

	return true;
}

/* メッセージボックスの描画を行う */
static void draw_msgbox(void)
{
	uint32_t c;
	int char_count, mblen, w, h, i;

	/* 今回のフレームで描画する文字数を取得する */
	char_count = get_frame_chars();
	if (char_count == 0)
		return;

	/* 1文字ずつ描画する */
	for (i = 0; i < char_count; i++) {
		/* ワードラッピングを処理する */
		if (is_after_space) {
			if (pen_x + get_en_word_width() >= msgbox_w -
			    conf_msgbox_margin_right) {
				pen_y += conf_msgbox_margin_line;
				pen_x = conf_msgbox_margin_left;
			}
		}
		is_after_space = *msg == ' ';

		/* 描画する文字を取得する */
		mblen = utf8_to_utf32(msg, &c);
		if (mblen == -1) {
			drawn_chars = total_chars;
			return;
		}

		/* エスケープの処理 */
		if (!escaped) {
			/* エスケープ文字であるとき */
			if (c == CHAR_BACKSLASH || c == CHAR_YENSIGN) {
				escaped = true;
				msg += mblen;
				drawn_chars++;
				continue;
			}
		} else if (escaped) {
			/* エスケープされた文字であるとき */
			if (c == CHAR_SMALLN) {
				pen_y += conf_msgbox_margin_line;
				pen_x = conf_msgbox_margin_left;
				escaped = false;
				msg += mblen;
				drawn_chars++;
				continue;
			}

			/* 不明なエスケープシーケンスの場合 */
			escaped = false;
		}

		/* 描画する文字の幅を取得する */
		w = get_glyph_width(c);

		/*
		 * メッセージボックスの幅を超える場合、改行する。
		 * ただし行頭禁則文字の場合は改行しない。
		 */
		if ((pen_x + w >= msgbox_w - conf_msgbox_margin_right) &&
		    (c != CHAR_SPACE && c != CHAR_COMMA && c != CHAR_PERIOD &&
		     c != CHAR_COLON && c != CHAR_SEMICOLON &&
		     c != CHAR_TOUTEN && c != CHAR_KUTEN)) {
			pen_y += conf_msgbox_margin_line;
			pen_x = conf_msgbox_margin_left;
		}

		/* 描画する */
		h = draw_char_on_msgbox(pen_x, pen_y, c, color);

		/* 更新領域を求める */
		union_rect(&draw_x, &draw_y, &draw_w, &draw_h, draw_x, draw_y,
			   draw_w, draw_h, msgbox_x + pen_x, msgbox_y + pen_y,
			   w, h);

		/* 次の文字へ移動する */
		pen_x += w;
		msg += mblen;
		drawn_chars++;
	}
}

/* 今回のフレームで描画する文字数を取得する */
static int get_frame_chars(void)
{
	float lap;
	int char_count;

	/* 繰り返し動作しない場合 */
	if (!is_in_command_repetition()) {
		/* すべての文字を描画する */
		return total_chars;
	}

	/* セーブ画面かヒストリ画面から復帰した場合 */
	if (restore_flag || history_flag) {
		/* すべての文字を描画する */
		return total_chars;
	}

	/* 入力によりスキップされた場合 */
	if (is_control_pressed) {
		/* 繰り返し動作を停止する */
		stop_command_repetition();

		/* 残りの文字をすべて描画する */
		return total_chars - drawn_chars;
	}
	if (is_return_pressed || is_down_pressed || is_left_button_pressed) {
		/* 残りの文字をすべて描画する */
		return total_chars - drawn_chars;
	}

	/* 経過時間を取得する */
	lap = (float)get_stop_watch_lap(&sw) / 1000.0f;

	/* 今回描画する文字数を取得する */
	char_count = (int)ceil(conf_msgbox_speed * lap) - drawn_chars;
	if (char_count > total_chars - drawn_chars)
		char_count = total_chars - drawn_chars;

	return char_count;
}

/* クリックアニメーションを描画する */
static void draw_click(void)
{
	int click_x, click_y, click_w, click_h;
	int lap;

	/* 入力があったら終了する */
	if (is_control_pressed) {
		stop_command_repetition();
		return;
	}
	if (!process_click_first &&
	    (is_return_pressed || is_down_pressed || is_left_button_pressed)) {
		stop_command_repetition();
		return;
	}

	/* クリックアニメーションの初回表示のとき */
	if (process_click_first) {
		process_click_first = false;

		/* 時間計測を開始する */
		reset_stop_watch(&sw);
	}

	/* 経過時間を取得する */
	lap = get_stop_watch_lap(&sw);

	/* クリックアニメーションの点滅を行う */
	if (lap % (int)(conf_click_interval * 2 * 1000) <
	    (int)(conf_click_interval * 1000)) {
		if (!is_click_visible) {
			show_click(true);
			is_click_visible = true;
		} else {
			return;
		}
	} else {
		if (is_click_visible) {
			show_click(false);
			is_click_visible = false;
		} else {
			return;
		}
	}

	/* 描画範囲を求める */
	get_click_rect(&click_x, &click_y, &click_w, &click_h);
	union_rect(&draw_x, &draw_y, &draw_w, &draw_h, draw_x, draw_y, draw_w,
		   draw_h, click_x, click_y, click_w, click_h);
}

/* msgが英単語の先頭であれば、その単語の描画幅、それ以外の場合0を返す */
static int get_en_word_width(void)
{
	const char *m;
	int width;

	m = msg;
	width = 0;
	while (isalnum(*m))
		width += get_glyph_width((unsigned char)*m++);

	return width;
}

/* 文字色を求める */
static pixel_t get_message_color(void)
{
	int i;
	const char *name;

	/* セリフの場合 */
	if (get_command_type() == COMMAND_SERIF) {
		name = get_string_param(SERIF_PARAM_NAME);

		/* コンフィグでnameの指す名前が指定されているか */
		for (i = 0; i < SERIF_COLOR_COUNT; i++) {
			if (conf_serif_color_name[i] == NULL)
				continue;
			if (strcmp(name, conf_serif_color_name[i]) == 0) {
				/* コンフィグで指定された色にする */
				return make_pixel(
					0xff,
					(uint32_t)conf_serif_color_r[i],
					(uint32_t)conf_serif_color_g[i],
					(uint32_t)conf_serif_color_b[i]);
			}
		}
	}

	/* セリフでないかコンフィグで名前が指定されていない場合 */
	return make_pixel(0xff, (pixel_t)conf_font_color_r,
			  (pixel_t)conf_font_color_g,
			  (pixel_t)conf_font_color_b);
}

/* 終了処理を行う */
static bool cleanup(void)
{
	/* PCMストリームの再生を終了する */
	set_mixer_input(VOICE_STREAM, NULL);

	/* メッセージを破棄する */
	free(msg_top);

	/* クリックアニメーションを非表示にする */
	show_click(false);

	/* 表示中のメッセージをなしとする */
	clear_message_registered();

	/* 次のコマンドに移動する */
	if (!move_to_next_command())
		return false;

	return true;
}
