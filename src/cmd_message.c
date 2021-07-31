/* -*- coding: utf-8-with-signature; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Suika 2
 * Copyright (C) 2001-2021, TABATA Keiichi. All rights reserved.
 */

/*
 * [Changes]
 *  - 2016/06/24 作成
 *  - 2017/01/30 "\n"対応
 *  - 2017/04/13 ワードラッピング対応
 *  - 2021/06/15 @setsave対応
 *  - 2021/07/29 メッセージボックスボタン対応
 *  - 2021/07/30 オートモード対応
 *  - 2021/07/31 スキップモード対応
 */

#include "suika.h"
#include <ctype.h>

#define ASSERT_INVALID_BTN_INDEX (0)

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

/* メッセージボックスボタンのインデックス */
#define BTN_NONE	(-1)
#define BTN_QSAVE	(0)
#define BTN_QLOAD	(1)
#define BTN_SAVE	(2)
#define BTN_LOAD	(3)
#define BTN_AUTO	(4)
#define BTN_SKIP	(5)
#define BTN_HISTORY	(6)

/* オートモードでボイスありのとき待ち時間 */
#define AUTO_MODE_VOICE_WAIT		(2000)

/* オートモードでボイスなしのとき待ち時間のスケール */
#define AUTO_MODE_TEXT_WAIT_SCALE	(0.15f)

/* コマンドの経過時刻を表すストップウォッチ */
static stop_watch_t click_sw;

/* 描画する文字の総数 */
static int total_chars;

/* 前回までに描画した文字数 */
static int drawn_chars;

/* スペースの直後であるか */
static bool is_after_space;

/* ビープ音再生中であるか */
static bool is_beep;

/* ボイスがあるか */
static bool have_voice;

/* オートモードでメッセージ表示とボイス再生の完了後の待ち時間中か */
static bool is_auto_mode_wait;

/* オートモードの経過時刻を表すストップウォッチ */
static stop_watch_t auto_sw;

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

/* 文字の縁取りの色 */
static pixel_t outline_color;

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

/* ポイント中のボタン */
static int pointed_index;

/*
 * 前方参照
 */

static bool init(void);
static void init_skip_mode(void);
static bool register_message_for_history(void);
static bool process_serif_command(void);
static void draw_namebox(void);
static int get_namebox_width(void);
static bool play_voice(void);
static void draw_msgbox(void);
static int get_frame_chars(void);
static void draw_click(void);
static int get_en_word_width(void);
static void get_message_color(pixel_t *color, pixel_t *outline_color);
static void init_pointed_index(void);
static void init_first_draw_area(void);
static void init_repetition(void);
static void frame_draw_buttons(bool se);
static int get_pointed_button(void);
static void get_button_rect(int btn, int *x, int *y, int *w, int *h);
static void frame_quick_save(void);
static bool frame_quick_load(void);
static bool frame_save(void);
static bool frame_load(void);
static bool frame_history(void);
static void frame_auto_mode(void);
static bool check_auto_play_condition(void);
static int get_wait_time(void);
static void frame_skip_mode(void);
static void frame_main(void);
static void play_se(const char *file);
static bool is_skippable(void);
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

	/* ボタンの描画を行う */
	frame_draw_buttons(true);

	/* クイックセーブを処理する */
	frame_quick_save();

	/* クイックロードを処理する */
	if (frame_quick_load())
		return true;

	/* セーブ画面への遷移を処理する */
	if (frame_save())
		return true;

	/* ロード画面への遷移を処理する */
	if (frame_load())
		return true;

	/* ヒストリ画面への遷移を処理する */
	if (frame_history())
		return true;

	/* オートモードを処理する */
	frame_auto_mode();
	
	/* スキップモードを処理する */
	frame_skip_mode();

	/* メイン表示処理を行う */
	frame_main();

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

	/* 初期化処理のスキップモードの部分を行う */
	init_skip_mode();

	/* メッセージを取得する */
	raw_msg = get_command_type() == COMMAND_MESSAGE ?
		get_line_string() : get_string_param(SERIF_PARAM_MESSAGE);
	msg_top = expand_variable(raw_msg);
	msg = msg_top;

	/* ヒストリ画面用にメッセージ履歴を登録する */
	if (!register_message_for_history())
		return false;

	/* 文字色を求める */
	get_message_color(&color, &outline_color);

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
	init_repetition();

	/* 初回に描画する矩形を求める */
	init_first_draw_area();

	/* オートモードの設定をする */
	is_auto_mode_wait = false;

	/* ボタンの選択状態を取得する */
	init_pointed_index();

	return true;
}

/* 初期化処理のスキップモードの部分を行う */
static void init_skip_mode(void)
{
	/* スキップモードでなければ何もしない */
	if (!is_skip_mode())
		return;

	/* 未読に到達した場合、スキップモードを終了する */
	if (!get_seen()) {
		stop_skip_mode();
		return;
	}

	/* クリックされた場合 */
	if (is_right_button_pressed || is_left_button_pressed) {
		/* SEを再生する */
		play_se(conf_msgbox_skip_cancel_se);

		/* スキップモードを終了する */
		stop_skip_mode();

		/* 以降のクリック処理を行わない */
		is_right_button_pressed = false;
		is_left_button_pressed = false;
	}
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
		/* ボイスはない */
		have_voice = false;

		/* 名前ボックスを表示しない */
		show_namebox(false);
		is_beep = false;
		return true;
	}

	/* ボイスを再生する */
	if (!(is_skip_mode() && is_skippable()) && !is_control_pressed &&
	    !history_flag && (!restore_flag || !is_message_registered())) {
		/* いったんボイスなしにしておく */
		have_voice = false;

		/* ボイスを再生する */
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
		w = draw_char_on_namebox(x, conf_namebox_margin_top, c, color,
					 outline_color);

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

	is_beep = repeat;

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

	/* ボイスありにする */
	have_voice = true;

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
		h = draw_char_on_msgbox(pen_x, pen_y, c, color, outline_color);

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
	if (is_skippable() && (is_skip_mode() || is_control_pressed)) {
		/* 繰り返し動作を停止する */
		stop_command_repetition();

		/* 残りの文字をすべて描画する */
		return total_chars - drawn_chars;
	}
	if (is_return_pressed || is_down_pressed ||
	    (pointed_index != BTN_NONE && is_left_button_pressed)) {
		/* ビープの再生を止める */
		if (is_beep)
			set_mixer_input(VOICE_STREAM, NULL);

		/* 残りの文字をすべて描画する */
		return total_chars - drawn_chars;
	}

	/* 経過時間を取得する */
	lap = (float)get_stop_watch_lap(&click_sw) / 1000.0f;

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
	if (is_skippable() && (is_skip_mode() || is_control_pressed)) {
		stop_command_repetition();
		return;
	}
	if (!process_click_first &&
	    (is_return_pressed || is_down_pressed ||
	     (pointed_index == BTN_NONE && is_left_button_pressed))) {
		stop_command_repetition();
		return;
	}

	/* クリックアニメーションの初回表示のとき */
	if (process_click_first) {
		process_click_first = false;

		/* 時間計測を開始する */
		reset_stop_watch(&click_sw);
	}

	/* 経過時間を取得する */
	lap = get_stop_watch_lap(&click_sw);

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
	while (isgraph(*m))
		width += get_glyph_width((unsigned char)*m++);

	return width;
}

/* 文字色を求める */
static void get_message_color(pixel_t *color, pixel_t *outline_color)
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
				*color = make_pixel(
					0xff,
					(uint32_t)conf_serif_color_r[i],
					(uint32_t)conf_serif_color_g[i],
					(uint32_t)conf_serif_color_b[i]);
				*outline_color = make_pixel(
					0xff,
					(uint32_t)conf_serif_outline_color_r[i],
					(uint32_t)conf_serif_outline_color_g[i],
					(uint32_t)conf_serif_outline_color_b[i]);
				return;
			}
		}
	}

	/* セリフでないかコンフィグで名前が指定されていない場合 */
	*color =  make_pixel(0xff, (pixel_t)conf_font_color_r,
			     (pixel_t)conf_font_color_g,
			     (pixel_t)conf_font_color_b);
	*outline_color = make_pixel(0xff, (pixel_t)conf_font_outline_color_r,
				    (pixel_t)conf_font_outline_color_g,
				    (pixel_t)conf_font_outline_color_b);
}

/* 初期化処理においてポイントされているボタンを求め描画する */
static void init_pointed_index(void)
{
	int x, y, w, h;

	/* ポイントされているボタンを求める */
	pointed_index = get_pointed_button();
	if (pointed_index == BTN_NONE)
		return;

	/* セーブロードが無効な場合にセーブロードのボタンを無効化する */
	if (!is_save_load_enabled() &&
	     (pointed_index == BTN_QSAVE || pointed_index == BTN_QLOAD ||
	      pointed_index == BTN_SAVE || pointed_index == BTN_LOAD)) {
		pointed_index = BTN_NONE;
		return;
	}

	/* クイックセーブデータがない場合にQLOADボタンを無効化する */
	if (!have_quick_save_data() && pointed_index == BTN_QLOAD) {
		pointed_index = BTN_NONE;
		return;
	}

	/* 未読の場合にSKIPボタンを無効化する */
	if (!get_seen() && pointed_index == BTN_SKIP) {
		pointed_index = BTN_NONE;
		return;
	}

	/* ボタンを描画する */
	get_button_rect(pointed_index, &x, &y, &w, &h);
	clear_msgbox_rect_with_fg(x, y, w, h);
}

/* 初期化処理において、繰り返し動作を設定する */
static void init_repetition(void)
{
	if (is_skippable() && (is_skip_mode() || is_control_pressed)) {
		/* 繰り返し動作せず、すぐに表示する */
	} else {
		/* コマンドが繰り返し呼び出されるようにする */
		start_command_repetition();

		/* 時間計測を開始する */
		reset_stop_watch(&click_sw);
	}
}

/* 初期化処理において、初回に描画する矩形を求める */
static void init_first_draw_area(void)
{
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
}

/* ボタンを描画する */
static void frame_draw_buttons(bool se)
{
	int last_pointed_index, bx, by, bw, bh;;

	/* 選択中のボタンを取得する */
	last_pointed_index = pointed_index;
	pointed_index = get_pointed_button();

	/* 選択状態に変更がない場合 */
	if (pointed_index == last_pointed_index)
		return;

	/* 非アクティブになるボタンを描画する */
	if (last_pointed_index != BTN_NONE) {
		/* ボタンを描画する */
		get_button_rect(last_pointed_index, &bx, &by, &bw, &bh);
		clear_msgbox_rect_with_bg(bx, by, bw, bh);
		union_rect(&draw_x, &draw_y, &draw_w, &draw_h,
			   draw_x, draw_y, draw_w, draw_h,
			   bx + conf_msgbox_x, by + conf_msgbox_y, bw, bh);
	}

	/* アクティブになるボタンを描画する */
	if (pointed_index != BTN_NONE) {
		/* セーブロード無効時のQSAVE/QLOAD/SAVE/LOADボタンの場合 */
		if (!is_save_load_enabled() &&
		    (pointed_index == BTN_SAVE || pointed_index == BTN_QSAVE ||
		     pointed_index == BTN_LOAD || pointed_index == BTN_QLOAD))
			return;

		/* クイックセーブデータがない時のQLOADボタンの場合 */
		if (!have_quick_save_data() && pointed_index == BTN_QLOAD)
			return;

		/* 未読の場合のSKIPボタンの場合 */
		if (!get_seen() && pointed_index == BTN_SKIP)
			return;

		/* ボタンを描画する */
		get_button_rect(pointed_index, &bx, &by, &bw, &bh);
		clear_msgbox_rect_with_fg(bx, by, bw, bh);
		union_rect(&draw_x, &draw_y, &draw_w, &draw_h,
			   draw_x, draw_y, draw_w, draw_h,
			   bx + conf_msgbox_x, by + conf_msgbox_y, bw, bh);

		/* SEを再生する */
		if (se)
			play_se(conf_msgbox_btn_change_se);
	}
}

/* 選択中のボタンを取得する */
static int get_pointed_button(void)
{
	int rx, ry, btn_x, btn_y, btn_w, btn_h, i;

	assert(!(is_auto_mode() && is_skip_mode()));

	/* オートモード時はAUTOボタンを選択中とする */
	if (is_auto_mode())
		return BTN_AUTO;

	/* スキップモード時はSKIPボタンを選択中とする */
	if (is_skip_mode())
		return BTN_SKIP;

	/* マウス座標からメッセージボックス内座標に変換する */
	rx = mouse_pos_x - conf_msgbox_x;
	ry = mouse_pos_y - conf_msgbox_y;

	/* ボタンを順番に見ていく */
	for (i = BTN_QSAVE; i <= BTN_HISTORY; i++) {
		/* ボタンの座標を取得する */
		get_button_rect(i, &btn_x, &btn_y, &btn_w, &btn_h);

		/* マウスがボタンの中にあればボタンの番号を返す */
		if ((rx >= btn_x && rx < btn_x + btn_w) &&
		    (ry >= btn_y && ry < btn_y + btn_h))
			return i;
	}

	/* ボタンがポイントされていない */
	return BTN_NONE;
}

/* ボタンの座標を取得する */
static void get_button_rect(int btn, int *x, int *y, int *w, int *h)
{
	switch (btn) {
	case BTN_QSAVE:
		*x = conf_msgbox_btn_qsave_x;
		*y = conf_msgbox_btn_qsave_y;
		*w = conf_msgbox_btn_qsave_width;
		*h = conf_msgbox_btn_qsave_height;
		break;
	case BTN_QLOAD:
		*x = conf_msgbox_btn_qload_x;
		*y = conf_msgbox_btn_qload_y;
		*w = conf_msgbox_btn_qload_width;
		*h = conf_msgbox_btn_qload_height;
		break;
	case BTN_SAVE:
		*x = conf_msgbox_btn_save_x;
		*y = conf_msgbox_btn_save_y;
		*w = conf_msgbox_btn_save_width;
		*h = conf_msgbox_btn_save_height;
		break;
	case BTN_LOAD:
		*x = conf_msgbox_btn_load_x;
		*y = conf_msgbox_btn_load_y;
		*w = conf_msgbox_btn_load_width;
		*h = conf_msgbox_btn_load_height;
		break;
	case BTN_AUTO:
		*x = conf_msgbox_btn_auto_x;
		*y = conf_msgbox_btn_auto_y;
		*w = conf_msgbox_btn_auto_width;
		*h = conf_msgbox_btn_auto_height;
		break;
	case BTN_SKIP:
		*x = conf_msgbox_btn_skip_x;
		*y = conf_msgbox_btn_skip_y;
		*w = conf_msgbox_btn_skip_width;
		*h = conf_msgbox_btn_skip_height;
		break;
	case BTN_HISTORY:
		*x = conf_msgbox_btn_history_x;
		*y = conf_msgbox_btn_history_y;
		*w = conf_msgbox_btn_history_width;
		*h = conf_msgbox_btn_history_height;
		break;
	default:
		assert(ASSERT_INVALID_BTN_INDEX);
		break;
	}
}

/* フレーム描画中のクイックセーブボタン押下を処理する */
static void frame_quick_save(void)
{
	/* セーブロード無効時は処理しない */
	if (!is_save_load_enabled())
		return;

	/* QSAVEボタンが押下されたとき */
	if (is_left_button_pressed && pointed_index == BTN_QSAVE) {
		/* クイックセーブを行う */
		quick_save();

		/* SEを再生する */
		play_se(conf_msgbox_btn_qsave_se);

		/* 以降のクリック処理を行わない */
		is_left_button_pressed = false;
	}
}

/* フレーム描画中のクイックロードボタン押下を処理する */
static bool frame_quick_load(void)
{
	/* セーブロード無効時は処理しない */
	if (!is_save_load_enabled())
		return false;

	/* QLOADボタンが押下されたとき */
	if (is_left_button_pressed && pointed_index == BTN_QLOAD) {
		/* クイックロードを行う */
		if (quick_load()) {
			/* SEを再生する */
			play_se(conf_msgbox_btn_qload_se);

			/* 後処理を行う */
			stop_command_repetition();
			free(msg_top);
			show_click(false);
			draw_stage();
			return true;
		}

		/* 以降のクリック処理を行わない */
		is_left_button_pressed = false;
	}

	return false;
}

/* フレーム描画中のセーブボタン押下および右クリックを処理する */
static bool frame_save(void)
{
	/* セーブロード無効時は処理しない */
	if (!is_save_load_enabled())
		return false;

	/* 右クリックかQLOADボタンが押下されたとき */
	if (is_right_button_pressed ||
	    (is_left_button_pressed && pointed_index == BTN_SAVE)) {
		/* SEを再生する */
		if (is_right_button_pressed)
			play_se(conf_msgbox_save_se);
		else
			play_se(conf_msgbox_btn_save_se);

		/* ボイスを停止する */
		set_mixer_input(VOICE_STREAM, NULL);

		/* セーブモードを開始する */
		start_save_mode(false);

		/* 後処理をする */
		stop_command_repetition();
		free(msg_top);
		show_click(false);
		return true;
	}

	return false;
}

/* フレーム描画中のロードボタン押下を処理する */
static bool frame_load(void)
{
	/* セーブロード無効時は処理しない */
	if (!is_save_load_enabled())
		return false;

	/* LOADボタンが押下されたとき */
	if (is_left_button_pressed && pointed_index == BTN_LOAD) {
		/* SEを再生する */
		play_se(conf_msgbox_btn_load_se);

		/* ボイスを停止する */
		set_mixer_input(VOICE_STREAM, NULL);

		/* ロードモードを開始する */
		start_load_mode(false);

		/* 後処理をする */
		stop_command_repetition();
		free(msg_top);
		show_click(false);
		return true;
	}

	return false;
}

/* フレーム描画中のヒストリボタン押下および上キー押下および
 * マウスホイール上押下を処理する */
static bool frame_history(void)
{
	/* 上キーかHISTORYボタンが押された場合 */
	if (is_up_pressed ||
	    (is_left_button_pressed && pointed_index == BTN_HISTORY)) {
		/* SEを再生する */
		if (is_up_pressed)
			play_se(conf_msgbox_history_se);
		else
			play_se(conf_msgbox_btn_history_se);

		/* ボイスを停止する */
		set_mixer_input(VOICE_STREAM, NULL);

		/* ヒストリーモードを開始する */
		start_history_mode();

		/* 後処理をする */
		stop_command_repetition();
		free(msg_top);
		return true;
	}

	return false;
}

/* フレーム描画中のオートボタン押下およびオートモード制御を処理する */
static void frame_auto_mode(void)
{
	int lap;

	/* スキップモードの場合、何もしない */
	if (is_skip_mode())
		return;

	/* オートモードでない場合 */
	if (!is_auto_mode()) {
		/* AUTOボタンが押下された場合 */
		if (is_left_button_pressed && pointed_index == BTN_AUTO) {
			/* SEを再生する */
			play_se(conf_msgbox_btn_auto_se);

			/* オートモードを開始する */
			start_auto_mode();

			/* メッセージ表示とボイス再生を未完了にする */
			is_auto_mode_wait = false;

			/* 以降のクリック処理を行わない */
			is_left_button_pressed = false;
		}
	}

	/* オートモードの場合 */
	if (is_auto_mode()) {
		/* 左クリックされた場合 */
		if (is_left_button_pressed) {
			/* SEを再生する */
			play_se(conf_msgbox_auto_cancel_se);

			/* オートモードを終了する */
			stop_auto_mode();

			/* メッセージ表示とボイス再生を未完了にする */
			is_auto_mode_wait = false;

			/* ボタンを再描画する */
			frame_draw_buttons(false);

			/* 以降のクリック処理を行わない */
			is_left_button_pressed = false;
			return;
		}

		/* メッセージ表示とボイス再生が未完了の場合 */
		if (!is_auto_mode_wait) {
			/* すでに表示が完了していれば */
			if (check_auto_play_condition()) {
				/* 待ち時間に入る */
				is_auto_mode_wait = true;

				/* 時間計測を開始する */
				reset_stop_watch(&auto_sw);
			}
			return;
		}

		/* 待ち時間なので、時間を計測する */
		lap = get_stop_watch_lap(&auto_sw);

		/* 時間が経過していれば、コマンドの終了処理に移る */
		if (lap >= get_wait_time())
			stop_command_repetition();
	}
}

/* オートプレイ用の表示完了チェックを行う */
static bool check_auto_play_condition(void)
{
	/*
	 * セーブ画面およびヒストリ画面から戻った場合
	 *  - 表示は瞬時に終わり、ボイスも再生されていない
	 *  - すでに表示完了しているとみなす
	 */
	if (restore_flag || history_flag)
		return true;

	/*
	 * ボイスありの場合
	 *  - ボイスの再生完了と文字の表示完了をチェックする
	 */
	if (have_voice) {
		if (is_mixer_sound_finished(VOICE_STREAM) &&
		    drawn_chars == total_chars)
			return true;
		return false;
	}

	/*
	 * ボイスなしの場合
	 *  - 文字の表示完了をチェックする
	 */
	if (drawn_chars == total_chars)
		return true;

	return false;
}

/* オートモードの待ち時間を取得する */
static int get_wait_time(void)
{
	float scale;

	/* ボイスありのとき */
	if (have_voice)
		return AUTO_MODE_VOICE_WAIT;

	/* ボイスなしのとき、スケールを求める */
	scale = conf_msgbox_auto_speed;
	if (scale == 0)
		scale = AUTO_MODE_TEXT_WAIT_SCALE;

	return (int)((float)total_chars * scale * 1000.0f);
}

/* フレーム描画中のスキップモードの処理を行う */
static void frame_skip_mode(void)
{
	/* オートモードの場合、何もしない */
	if (is_auto_mode())
		return;

	/* スキップモードの場合、何もしない */
	if (is_skip_mode())
		return;

	/* 未読の場合 */
	if (!get_seen())
		return;

	/* SKIPボタンが押下された場合 */
	if (is_left_button_pressed && pointed_index == BTN_SKIP) {
		/* SEを再生する */
		play_se(conf_msgbox_btn_skip_se);

		/* オートモードを開始する */
		start_skip_mode();

		/* 以降のクリック処理を行わない */
		is_left_button_pressed = false;
	}
}

/* フレーム描画中のメイン処理を行う */
static void frame_main(void)
{
	/* スペースキーの押下を処理する */
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
		return;
	}

	/* スペースキーの解放を処理する */
	if(is_hidden && !is_space_pressed) {
		is_hidden = false;
		if (get_command_type() == COMMAND_SERIF)
			show_namebox(true);
		show_msgbox(true);
		show_click(true);
		draw_x = 0;
		draw_y = 0;
		draw_w = conf_window_width;
		draw_h = conf_window_height;
		return;
	}

	/* メインの表示処理を行う */
	if (!is_hidden) {
		/* 入力があったらボイスを止める */
		if (!conf_voice_stop_off &&
		    (is_skippable() && (is_skip_mode() || is_control_pressed)))
			set_mixer_input(VOICE_STREAM, NULL);

		/* 文字かクリックアニメーションを描画する */
		if (drawn_chars < total_chars)
			draw_msgbox();
		else
			draw_click();
		
	}
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

/* 既読であるか調べる */
static bool is_skippable(void)
{
	if (get_seen())
		return true;

	return false;
}

/* 終了処理を行う */
static bool cleanup(void)
{
	/* PCMストリームの再生を終了する */
	if (!conf_voice_stop_off)
		set_mixer_input(VOICE_STREAM, NULL);

	/* メッセージを破棄する */
	free(msg_top);

	/* クリックアニメーションを非表示にする */
	show_click(false);

	/* 表示中のメッセージをなしとする */
	clear_message_registered();

	/* 既読にする */
	set_seen();

	/* 次のコマンドに移動する */
	if (!move_to_next_command())
		return false;

	return true;
}
