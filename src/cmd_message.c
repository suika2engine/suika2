/* -*- coding: utf-8-with-signature; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Suika 2
 * Copyright (C) 2001-2022, TABATA Keiichi. All rights reserved.
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
 *  - 2022/06/06 デバッガに対応
 *  - 2022/07/19 システムメニューに対応
 *  - 2022/07/28 コンフィグに対応
 *  - 2022/08/08 セーブ・ロード・ヒストリをGUIに変更
 */

#include "suika.h"
#include <ctype.h>

#define ASSERT_INVALID_BTN_INDEX (0)

/* メッセージボックスボタンのインデックス */
#define BTN_NONE		(-1)
#define BTN_QSAVE		(0)
#define BTN_QLOAD		(1)
#define BTN_SAVE		(2)
#define BTN_LOAD		(3)
#define BTN_AUTO		(4)
#define BTN_SKIP		(5)
#define BTN_HISTORY		(6)
#define BTN_CONFIG		(7)
#define BTN_HIDE		(8)

/* システムメニューのボタンのインデックス */
#define SYSMENU_NONE		(-1)
#define SYSMENU_QSAVE		(0)
#define SYSMENU_QLOAD		(1)
#define SYSMENU_SAVE		(2)
#define SYSMENU_LOAD		(3)
#define SYSMENU_AUTO		(4)
#define SYSMENU_SKIP		(5)
#define SYSMENU_HISTORY		(6)
#define SYSMENU_CONFIG		(7)

/* オートモードでボイスありのとき待ち時間 */
#define AUTO_MODE_VOICE_WAIT		(4000)

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

/* クイックロードを行ったか */
static bool did_quick_load;

/* セーブモードに遷移するか */
static bool need_save_mode;

/* ロードモードに遷移するか */
static bool need_load_mode;

/* ヒストリモードに遷移するか */
static bool need_history_mode;

/* コンフィグモードに遷移するか */
static bool need_config_mode;

/* オートモードでメッセージ表示とボイス再生の完了後の待ち時間中か */
static bool is_auto_mode_wait;

/* オートモードの経過時刻を表すストップウォッチ */
static stop_watch_t auto_sw;

/* 描画位置 */
static int pen_x;
static int pen_y;

/* メッセージボックスの位置とサイズ */
static int msgbox_x;
static int msgbox_y;
static int msgbox_w;
static int msgbox_h;

/* 描画するメッセージ */
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

/* ロード画面から戻ったばかりであるか */
static bool load_flag;

/* GUI画面から戻ったばかりであるか */
static bool gui_flag;

/* 文字列がエスケープ中か */
static bool escaped;

/* ポイント中のボタン */
static int pointed_index;

/* システムメニューを表示中か */
static bool is_sysmenu;

/* システムメニューの最初のフレームか */
static bool is_sysmenu_first_frame;

/* システムメニューのどのボタンがポイントされているか */
static int sysmenu_pointed_index;

/* システムメニューのどのボタンがポイントされていたか */
static int old_sysmenu_pointed_index;

/* システムメニューが終了した直後か */
static bool is_sysmenu_finished;

/* 折りたたみシステムメニューが前のフレームでポイントされていたか */
static bool is_collapsed_sysmenu_pointed_prev;

/*
 * 前方参照
 */

static bool init(int *x, int *y, int *w, int *h);
static void init_auto_mode(void);
static void init_skip_mode(void);
static bool register_message_for_history(void);
static bool process_serif_command(int *x, int *y, int *w, int *h);
static void draw_namebox(void);
static int get_namebox_width(void);
static bool play_voice(void);
static void set_character_volume_by_name(const char *name);
static void draw_msgbox(int *x, int *y, int *w, int *h);
static int get_frame_chars(void);
static void draw_click(int *x, int *y, int *w, int *h);
static void check_stop_click_animation(void);
static int get_en_word_width(void);
static void get_message_color(pixel_t *color, pixel_t *outline_color);
static void init_pointed_index(void);
static void init_first_draw_area(int *x, int *y, int *w, int *h);
static void init_repetition(void);
static void frame_draw_buttons(bool se, int *x, int *y, int *w, int *h);
static int get_pointed_button(void);
static void get_button_rect(int btn, int *x, int *y, int *w, int *h);
static int get_sysmenu_pointed_button(void);
static void get_sysmenu_button_rect(int btn, int *x, int *y, int *w, int *h);
static bool frame_quick_save(void);
static bool frame_quick_load(void);
static bool frame_save(void);
static bool frame_load(void);
static bool frame_history(void);
static bool frame_config(void);
static void frame_auto_mode(int *x, int *y, int *w, int *h);
static bool check_auto_play_condition(void);
static int get_wait_time(void);
static void frame_skip_mode(void);
static void frame_sysmenu(void);
static void frame_hide(int *x, int *y, int *w, int *h);
static void frame_main(int *x, int *y, int *w, int *h);
static void draw_sysmenu(bool calc_only, int *x, int *y, int *w, int *h);
static void draw_collapsed_sysmenu(int *x, int *y, int *w, int *h);
static bool is_collapsed_sysmenu_pointed(void);
static void draw_banners(int *x, int *y, int *w, int *h);
static void play_se(const char *file);
static bool is_skippable(void);
static bool cleanup(void);

/*
 * メッセージ・セリフコマンド
 */
bool message_command(int *x, int *y, int *w, int *h)
{
	/* 初期化処理を行う */
	if (!is_in_command_repetition())
		if (!init(x, y, w, h))
			return false;

	/* ボタンの描画を行う */
	frame_draw_buttons(true, x, y, w, h);

	/* 各種操作を処理する */
	do {
		/* クイックセーブを処理する */
		if (frame_quick_save())
			break;

		/* クイックロードを処理する */
		if (frame_quick_load())
			break;

		/* セーブ画面への遷移を処理する */
		if (frame_save())
			break;

		/* ロード画面への遷移を処理する */
		if (frame_load())
			break;

		/* ヒストリ画面への遷移を処理する */
		if (frame_history())
			break;

		/* コンフィグ画面への遷移を処理する */
		if (frame_config())
			break;

		/* オートモードを処理する */
		frame_auto_mode(x, y, w, h);

		/* スキップモードを処理する */
		frame_skip_mode();

		/* メッセージボックスの非表示/表示切り替えを処理する */
		frame_hide(x, y, w, h);

		/* システムメニューを処理する */
		frame_sysmenu();
	} while (0);

	/*
	 * メイン表示処理を行う
	 *  - クイックロードされた場合は処理しない
	 *  - セーブ・ロード画面に遷移する場合はサムネイル描画のため処理する
	 */
	if (!did_quick_load)
		frame_main(x, y, w, h);

	/* クイックロード・セーブ・ロード・ヒストリモードが選択された場合 */
	if (did_quick_load || need_save_mode || need_load_mode ||
	    need_history_mode || need_config_mode)
		stop_command_repetition();

	/* 終了処理を行う */
	if (!is_in_command_repetition())
		if (!cleanup())
			return false;

	/* ロードされて最初のフレームの場合、画面全体を描画する */
	if (load_flag) {
		union_rect(x, y, w, h, *w, *y, *w, *h, 0, 0, conf_window_width,
			   conf_window_height);
		load_flag = false;
	}

	/* ステージを描画する */
	draw_stage_rect(*x, *y, *w, *h);

	/* システムメニューを描画する */
	if (!conf_sysmenu_hidden) {
		if (is_sysmenu)
			draw_sysmenu(false, x, y, w, h);
		else if (!is_auto_mode() && !is_skip_mode())
			draw_collapsed_sysmenu(x, y, w, h);
	}
	is_sysmenu_finished = false;

	/* セーブ・ロード・ヒストリ・コンフィグモードへ遷移する */
	if (need_save_mode) {
		if (!prepare_gui_mode(SAVE_GUI_FILE, true, true))
			return false;
		start_gui_mode();
	}
	if (need_load_mode) {
		if (!prepare_gui_mode(LOAD_GUI_FILE, true, true))
			return false;
		start_gui_mode();
	}
	if (need_history_mode) {
		if (!prepare_gui_mode(HISTORY_GUI_FILE, true, true))
			return false;
		start_gui_mode();
	}
	if (need_config_mode) {
		if (!prepare_gui_mode(CONFIG_GUI_FILE, true, true))
			return false;
		start_gui_mode();
	}

	return true;
}

/* 初期化処理を行う */
static bool init(int *x, int *y, int *w, int *h)
{
	const char *raw_msg;

	/* 初期化処理のスキップモードの部分を行う */
	init_auto_mode();

	/* 初期化処理のスキップモードの部分を行う */
	init_skip_mode();

	/* メッセージを取得する */
	raw_msg = get_command_type() == COMMAND_MESSAGE ?
		get_string_param(MESSAGE_PARAM_MESSAGE) :
		get_string_param(SERIF_PARAM_MESSAGE);
	msg = expand_variable(raw_msg);

	/* セーブ用にメッセージを保存する */
	if (!set_last_message(msg))
		return false;
	
	/* ヒストリ画面用にメッセージ履歴を登録する */
	if (!register_message_for_history())
		return false;

	/* 文字色を求める */
	get_message_color(&color, &outline_color);

	/* セリフの場合を処理する */
	if (!process_serif_command(x, y, w, h))
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
	union_rect(x, y, w, h,
		   *x, *y, *w, *h,
		   msgbox_x, msgbox_y, msgbox_w, msgbox_h);

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
	init_first_draw_area(x, y, w, h);

	/* オートモードの設定をする */
	is_auto_mode_wait = false;

	/* セーブ・ロード・ヒストリ・コンフィグの設定を行う */
	did_quick_load = false;
	need_save_mode = false;
	need_load_mode = false;
	need_history_mode = false;
	need_config_mode = false;

	/* システムメニューの設定を行う */
	is_sysmenu = false;
	is_sysmenu_finished = false;
	is_collapsed_sysmenu_pointed_prev = false;

	/* ボタンの選択状態を取得する */
	init_pointed_index();

	return true;
}

/* 初期化処理のスキップモードの部分を行う */
static void init_auto_mode(void)
{
	/* オートモードの場合 */
	if (is_auto_mode()) {
		/* リターンキー、下キーの入力を無効にする */
		is_return_pressed = false;
		is_down_pressed = false;
	}
}

/* 初期化処理のスキップモードの部分を行う */
static void init_skip_mode(void)
{
	/* スキップモードでなければ何もしない */
	if (!is_skip_mode())
		return;

	/* 未読に到達した場合、スキップモードを終了する */
	if (!is_skippable()) {
		stop_skip_mode();
		show_skipmode_banner(false);
		return;
	}

	/* クリックされた場合 */
	if (is_right_button_pressed || is_left_button_pressed) {
		/* SEを再生する */
		play_se(conf_msgbox_skip_cancel_se);

		/* スキップモードを終了する */
		stop_skip_mode();
		show_skipmode_banner(false);

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

	/* GUI画面から戻ったばかりの場合、2重登録を防ぐ */
	gui_flag = check_gui_flag();
	load_flag = check_load_flag();
	if (!load_flag && (gui_flag || is_message_registered()))
		return true;

	/* 名前、ボイスファイル名、メッセージを取得する */
	if (get_command_type() == COMMAND_SERIF) {
		name = get_string_param(SERIF_PARAM_NAME);
		voice = get_string_param(SERIF_PARAM_VOICE);

		/* ビープ音は履歴画面で再生しない */
		if (voice[0] == '@')
			voice = NULL;
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
static bool process_serif_command(int *x, int *y, int *w, int *h)
{
	int namebox_x, namebox_y, namebox_w, namebox_h;

	/*
	 * 描画範囲を更新する
	 *  - セリフコマンド以外でも、名前ボックス領域を消すために描画する
	 */
	get_namebox_rect(&namebox_x, &namebox_y, &namebox_w, &namebox_h);
	union_rect(x, y, w, h,
		   *x, *y, *w, *h,
		   namebox_x, namebox_y, namebox_w, namebox_h);

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
	if ((is_non_interruptible() &&
	     (!gui_flag && !is_message_registered()))
	    ||
	    (!is_non_interruptible() &&
	     !(is_skip_mode() && is_skippable()) &&
	     ((!is_control_pressed || !is_skippable()) &&
	      !gui_flag &&
	      !is_message_registered()))) {
		/* いったんボイスなしの判断にしておく(あとで変更する) */
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
	if (!conf_namebox_centering_no)
		x = (get_namebox_width() - get_utf8_width(name)) / 2;
	else
		x = conf_namebox_margin_left;

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
	float beep_factor;
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
		beep_factor = conf_beep_adjustment == 0 ?
			      1 : conf_beep_adjustment;
		times = (int)((float)utf8_chars(get_string_param(
							SERIF_PARAM_MESSAGE)) /
			      conf_msgbox_speed * beep_factor /
			      (get_text_speed() + 0.1));
		times = times == 0 ? 1 : times;
		set_wave_repeat_times(w, times);
	}

	/* キャラクタ音量を設定する */
	set_character_volume_by_name(get_string_param(SERIF_PARAM_NAME));

	/* PCMストリームを再生する */
	set_mixer_input(VOICE_STREAM, w);

	/* ボイスありの判断にする */
	have_voice = true;

	return true;
}

/* キャラクタ音量を設定する */
static void set_character_volume_by_name(const char *name)
{
	int i;

	for (i = 1; i < CH_VOL_SLOTS; i++) {
		/* キャラクタ名を探す */
		if (conf_sound_character_name[i] == NULL)
			continue;
		if (strcmp(conf_sound_character_name[i], name) == 0) {
			/* みつかった場合 */
			apply_character_volume(i);
			return;
		}
	}

	/* その他のキャラクタのボリュームを適用する */
	apply_character_volume(CH_VOL_SLOT_DEFAULT);
}

/* メッセージボックスの描画を行う */
static void draw_msgbox(int *x, int *y, int *w, int *h)
{
	uint32_t c;
	int char_count, mblen, cw, ch, i;

	/* 今回のフレームで描画する文字数を取得する */
	char_count = get_frame_chars();
	if (char_count == 0)
		return;

	/* 1文字ずつ描画する */
	for (i = 0; i < char_count; i++) {
		/* ワードラッピングを処理する */
		if (is_after_space) {
			if (pen_x + get_en_word_width() >=
			    msgbox_w - conf_msgbox_margin_right) {
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
		cw = get_glyph_width(c);

		/*
		 * メッセージボックスの幅を超える場合、改行する。
		 * ただし行頭禁則文字の場合は改行しない。
		 */
		if ((pen_x + cw >= msgbox_w - conf_msgbox_margin_right) &&
		    (c != CHAR_SPACE && c != CHAR_COMMA && c != CHAR_PERIOD &&
		     c != CHAR_COLON && c != CHAR_SEMICOLON &&
		     c != CHAR_TOUTEN && c != CHAR_KUTEN)) {
			pen_y += conf_msgbox_margin_line;
			pen_x = conf_msgbox_margin_left;
		}

		/* 描画する */
		draw_char_on_msgbox(pen_x, pen_y, c, color, outline_color, &cw,
				    &ch);

		/* 更新領域を求める */
		union_rect(x, y, w, h,
			   *x, *y, *w, *h,
			   msgbox_x + pen_x, msgbox_y + pen_y, cw, ch);

		/* 次の文字へ移動する */
		pen_x += cw;
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

	/* セーブ画面かヒストリ画面かコンフィグ画面から復帰した場合 */
	if (gui_flag) {
		/* すべての文字を描画する */
		return total_chars;
	}

	/* セーブ・ロードのサムネイルを作成するために全文字描画する場合 */
	if (need_save_mode || need_load_mode) {
		/* 残りの文字をすべて描画する */
		return total_chars - drawn_chars;
	}

	/* 入力によりスキップされた場合 */
	if (is_skippable() && !is_non_interruptible() &&
	    (is_skip_mode() || is_control_pressed)) {
		/* 繰り返し動作を停止する */
		stop_command_repetition();

		/* 残りの文字をすべて描画する */
		return total_chars - drawn_chars;
	}
	if (!is_non_interruptible() &&
	    (is_return_pressed || is_down_pressed ||
	     (pointed_index == BTN_NONE && is_left_button_pressed))) {
		/* ビープの再生を止める */
		if (is_beep)
			set_mixer_input(VOICE_STREAM, NULL);

#ifdef USE_DEBUGGER
		if (dbg_is_stop_requested())
			stop_command_repetition();
#endif

		/* 残りの文字をすべて描画する */
		return total_chars - drawn_chars;
	}

	/* 経過時間を取得する */
	lap = (float)get_stop_watch_lap(&click_sw) / 1000.0f;

	/* 今回描画する文字数を取得する */
	char_count = (int)ceil(conf_msgbox_speed * (get_text_speed() + 0.1) *
			       lap) - drawn_chars;
	if (char_count > total_chars - drawn_chars)
		char_count = total_chars - drawn_chars;

	return char_count;
}

/* クリックアニメーションを描画する */
static void draw_click(int *x, int *y, int *w, int *h)
{
	int click_x, click_y, click_w, click_h;
	int lap, index;

	/* 入力があったら繰り返しを終了する */
	check_stop_click_animation();

	/* クリックアニメーションの初回表示のとき */
	if (process_click_first) {
		process_click_first = false;

		/* 表示位置を設定する */
		if (conf_click_move) {
			set_click_index(0);
			get_click_rect(&click_x, &click_y, &click_w, &click_h);
			if (pen_x + click_w  >= msgbox_w -
			    conf_msgbox_margin_right) {
				pen_y += conf_msgbox_margin_line;
				pen_x = conf_msgbox_margin_left;
			}
			set_click_position(pen_x + conf_msgbox_x,
					   pen_y + conf_msgbox_y);
		}

		/* 時間計測を開始する */
		reset_stop_watch(&click_sw);
	}

	/* 経過時間を取得する */
	lap = get_stop_watch_lap(&click_sw);

	/* クリックアニメーションの表示を行う */
	if (conf_click_disable) {
		set_click_index(0);
		show_click(true);
		is_click_visible = true;
	} else {
		index = (lap % (int)(conf_click_interval * 1000)) /
			((int)(conf_click_interval * 1000) / CLICK_FRAMES) %
			CLICK_FRAMES;
		set_click_index(index);
		show_click(true);
		is_click_visible = true;
	}

	/* 描画範囲を求める */
	get_click_rect(&click_x, &click_y, &click_w, &click_h);
	union_rect(x, y, w, h,
		   *x, *y, *w, *h,
		   click_x, click_y, click_w, click_h);
}

/* クリックアニメーションで入力があったら繰り返しを終了する */
static void check_stop_click_animation(void)
{
#ifdef USE_DEBUGGER
	if (!process_click_first && dbg_is_stop_requested()) {
		if (!have_voice &&
		    gui_flag &&
		    (is_return_pressed || is_down_pressed ||
		     (pointed_index == BTN_NONE && is_left_button_pressed)))
			stop_command_repetition();
		else if (!have_voice)
			stop_command_repetition();
		else if (have_voice &&
			 (is_mixer_sound_finished(VOICE_STREAM) || gui_flag))
			stop_command_repetition();
		else if (have_voice &&
			 (is_left_button_pressed || is_down_pressed ||
			  is_return_pressed))
			stop_command_repetition();
	} else
#endif
	if (is_skippable() && !is_auto_mode() &&
	    (is_skip_mode() || is_control_pressed)) {
		if (!is_non_interruptible()) {
			stop_command_repetition();
		} else {
			if (!have_voice || is_mixer_sound_finished(VOICE_STREAM))
				stop_command_repetition();
		}
	} else if (gui_flag &&
		   !process_click_first &&
		   (is_return_pressed || is_down_pressed ||
		    (pointed_index == BTN_NONE && is_left_button_pressed))) {
		stop_command_repetition();
	} else if (!process_click_first &&
		   (is_return_pressed || is_down_pressed ||
		    (pointed_index == BTN_NONE && is_left_button_pressed))) {
		if (!is_non_interruptible()) {
			stop_command_repetition();
		} else {
			if (!have_voice || is_mixer_sound_finished(VOICE_STREAM))
				stop_command_repetition();
		}
	}
}

/* msgが英単語の先頭であれば、その単語の描画幅、それ以外の場合0を返す */
static int get_en_word_width(void)
{
	const char *m;
	uint32_t wc;
	int width;

	m = msg;
	width = 0;
	while (isgraph_extended(&m, &wc))
		width += get_glyph_width(wc);

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
				*color = make_pixel_slow(
					0xff,
					(uint32_t)conf_serif_color_r[i],
					(uint32_t)conf_serif_color_g[i],
					(uint32_t)conf_serif_color_b[i]);
				*outline_color = make_pixel_slow(
					0xff,
					(uint32_t)conf_serif_outline_color_r[i],
					(uint32_t)conf_serif_outline_color_g[i],
					(uint32_t)conf_serif_outline_color_b[i]);
				return;
			}
		}
	}

	/* セリフでないかコンフィグで名前が指定されていない場合 */
	*color = make_pixel_slow(0xff,
				 (pixel_t)conf_font_color_r,
				 (pixel_t)conf_font_color_g,
				 (pixel_t)conf_font_color_b);
	*outline_color = make_pixel_slow(0xff,
					 (pixel_t)conf_font_outline_color_r,
					 (pixel_t)conf_font_outline_color_g,
					 (pixel_t)conf_font_outline_color_b);
}

/* 初期化処理においてポイントされているボタンを求め描画する */
static void init_pointed_index(void)
{
	int bx, by, bw, bh;

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
	if (!is_skippable() && pointed_index == BTN_SKIP) {
		pointed_index = BTN_NONE;
		return;
	}

	/* ボタンを描画する */
	get_button_rect(pointed_index, &bx, &by, &bw, &bh);
	clear_msgbox_rect_with_fg(bx, by, bw, bh);
}

/* 初期化処理において、繰り返し動作を設定する */
static void init_repetition(void)
{
	if (is_skippable() && !is_non_interruptible() &&
	    (is_skip_mode() || is_control_pressed)) {
		/* 繰り返し動作せず、すぐに表示する */
	} else {
		/* コマンドが繰り返し呼び出されるようにする */
		start_command_repetition();

		/* 時間計測を開始する */
		reset_stop_watch(&click_sw);
	}
}

/* 初期化処理において、初回に描画する矩形を求める */
static void init_first_draw_area(int *x, int *y, int *w, int *h)
{
	/* 初回に描画する矩形を求める */
	if (check_menu_finish_flag() || check_retrospect_finish_flag() ||
	    gui_flag) {
		/* メニューコマンドが終了したばかりの場合 */
		*x = 0;
		*y = 0;
		*w = conf_window_width;
		*h = conf_window_height;
	} else {
		/* それ以外の場合 */
		union_rect(x, y, w, h,
			   *x, *y, *w, *h,
			   msgbox_x, msgbox_y, msgbox_w, msgbox_h);
	}
}

/* ボタンを描画する */
static void frame_draw_buttons(bool se, int *x, int *y, int *w, int *h)
{
	int last_pointed_index, bx, by, bw, bh;;

	/* システムメニュー表示中は処理しない */
	if (is_sysmenu)
		return;

	/* メッセージボックス非表示中は処理しない */
	if (is_hidden)
		return;

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
		union_rect(x, y, w, h,
			   *x, *y, *w, *h,
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
		if (!is_skippable() && pointed_index == BTN_SKIP)
			return;

		/* ボタンを描画する */
		get_button_rect(pointed_index, &bx, &by, &bw, &bh);
		clear_msgbox_rect_with_fg(bx, by, bw, bh);
		union_rect(x, y, w, h,
			   *x, *y, *w, *h,
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

	/* メッセージボックスを隠している間はボタンを選択しない */
	if (is_hidden)
		return BTN_NONE;

	/* マウス座標からメッセージボックス内座標に変換する */
	rx = mouse_pos_x - conf_msgbox_x;
	ry = mouse_pos_y - conf_msgbox_y;

	/* ボタンを順番に見ていく */
	for (i = BTN_QSAVE; i <= BTN_HIDE; i++) {
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
	case BTN_CONFIG:
		*x = conf_msgbox_btn_config_x;
		*y = conf_msgbox_btn_config_y;
		*w = conf_msgbox_btn_config_width;
		*h = conf_msgbox_btn_config_height;
		break;
	case BTN_HIDE:
		*x = conf_msgbox_btn_hide_x;
		*y = conf_msgbox_btn_hide_y;
		*w = conf_msgbox_btn_hide_width;
		*h = conf_msgbox_btn_hide_height;
		break;
	default:
		assert(ASSERT_INVALID_BTN_INDEX);
		break;
	}
}

/* 選択中のシステムメニューのボタンを取得する */
static int get_sysmenu_pointed_button(void)
{
	int rx, ry, btn_x, btn_y, btn_w, btn_h, i;

	/* システムメニューを表示中でない場合は非選択とする */
	if (!is_sysmenu)
		return SYSMENU_NONE;

	/* マウス座標からシステムメニュー画像内座標に変換する */
	rx = mouse_pos_x - conf_sysmenu_x;
	ry = mouse_pos_y - conf_sysmenu_y;

	/* ボタンを順番に見ていく */
	for (i = SYSMENU_QSAVE; i <= SYSMENU_CONFIG; i++) {
		/* ボタンの座標を取得する */
		get_sysmenu_button_rect(i, &btn_x, &btn_y, &btn_w, &btn_h);

		/* マウスがボタンの中にあればボタンの番号を返す */
		if ((rx >= btn_x && rx < btn_x + btn_w) &&
		    (ry >= btn_y && ry < btn_y + btn_h))
			return i;
	}

	/* ボタンがポイントされていない */
	return SYSMENU_NONE;
}

/* システムメニューのボタンの座標を取得する */
static void get_sysmenu_button_rect(int btn, int *x, int *y, int *w, int *h)
{
	switch (btn) {
	case SYSMENU_QSAVE:
		*x = conf_sysmenu_qsave_x;
		*y = conf_sysmenu_qsave_y;
		*w = conf_sysmenu_qsave_width;
		*h = conf_sysmenu_qsave_height;
		break;
	case SYSMENU_QLOAD:
		*x = conf_sysmenu_qload_x;
		*y = conf_sysmenu_qload_y;
		*w = conf_sysmenu_qload_width;
		*h = conf_sysmenu_qload_height;
		break;
	case SYSMENU_SAVE:
		*x = conf_sysmenu_save_x;
		*y = conf_sysmenu_save_y;
		*w = conf_sysmenu_save_width;
		*h = conf_sysmenu_save_height;
		break;
	case SYSMENU_LOAD:
		*x = conf_sysmenu_load_x;
		*y = conf_sysmenu_load_y;
		*w = conf_sysmenu_load_width;
		*h = conf_sysmenu_load_height;
		break;
	case SYSMENU_AUTO:
		*x = conf_sysmenu_auto_x;
		*y = conf_sysmenu_auto_y;
		*w = conf_sysmenu_auto_width;
		*h = conf_sysmenu_auto_height;
		break;
	case SYSMENU_SKIP:
		*x = conf_sysmenu_skip_x;
		*y = conf_sysmenu_skip_y;
		*w = conf_sysmenu_skip_width;
		*h = conf_sysmenu_skip_height;
		break;
	case SYSMENU_HISTORY:
		*x = conf_sysmenu_history_x;
		*y = conf_sysmenu_history_y;
		*w = conf_sysmenu_history_width;
		*h = conf_sysmenu_history_height;
		break;
	case SYSMENU_CONFIG:
		*x = conf_sysmenu_config_x;
		*y = conf_sysmenu_config_y;
		*w = conf_sysmenu_config_width;
		*h = conf_sysmenu_config_height;
		break;
	default:
		assert(ASSERT_INVALID_BTN_INDEX);
		break;
	}
}

/* フレーム描画中のクイックセーブボタン押下を処理する */
static bool frame_quick_save(void)
{
#ifdef USE_DEBUGGER
	/* シングルステップか停止要求中はセーブしない */
	if (dbg_is_stop_requested())
		return false;
#endif

	/* システムメニュー表示中は処理しない */
	if (is_sysmenu)
		return false;

	/* セーブロード無効時は処理しない */
	if (!is_save_load_enabled())
		return false;

	/* メッセージボックス非表示中は処理しない */
	if (is_hidden)
		return false;

	/* QSAVEボタンが押下されたとき */
	if (is_left_button_pressed && pointed_index == BTN_QSAVE) {
		/* サムネイルを作成する */
		draw_stage_to_thumb();

		/* クイックセーブを行う */
		quick_save();

		/* SEを再生する */
		play_se(conf_msgbox_btn_qsave_se);

		/* 以降のクリック処理を行わない */
		is_left_button_pressed = false;

		return true;
	}

	return false;
}

/* フレーム描画中のクイックロードボタン押下を処理する */
static bool frame_quick_load(void)
{
#ifdef USE_DEBUGGER
	/* シングルステップか停止要求中はロードしない */
	if (dbg_is_stop_requested())
		return false;
#endif

	/* システムメニュー表示中は処理しない */
	if (is_sysmenu)
		return false;

	/* セーブロード無効時は処理しない */
	if (!is_save_load_enabled())
		return false;

	/* メッセージボックス非表示中は処理しない */
	if (is_hidden)
		return false;

	/* クイックセーブデータがない場合は処理しない */
	if (!have_quick_save_data())
		return false;

	/* QLOADボタンが押下されたとき */
	if (is_left_button_pressed && pointed_index == BTN_QLOAD) {
		/* クイックロードを行う */
		if (quick_load()) {
			/* SEを再生する */
			play_se(conf_msgbox_btn_qload_se);

			/* 後処理を行う */
			did_quick_load = true;

			return true;
		}

		/* 以降のクリック処理を行わない */
		is_left_button_pressed = false;
	}

	return false;
}

/* フレーム描画中のセーブボタン押下を処理する */
static bool frame_save(void)
{
#ifdef USE_DEBUGGER
	/* シングルステップか停止要求中はセーブしない */
	if (dbg_is_stop_requested())
		return false;
#endif

	/* システムメニュー表示中は処理しない */
	if (is_sysmenu)
		return false;

	/* セーブロード無効時は処理しない */
	if (!is_save_load_enabled())
		return false;

	/* メッセージボックス非表示中は処理しない */
	if (is_hidden)
		return false;

	/* SAVEボタンが押下されたとき */
	if (is_left_button_pressed && pointed_index == BTN_SAVE) {
		/* SEを再生する */
		play_se(conf_msgbox_btn_save_se);

		/* ボイスを停止する */
		set_mixer_input(VOICE_STREAM, NULL);

		/* セーブモードを開始する */
		need_save_mode =  true;

		return true;
	}

	return false;
}

/* フレーム描画中のロードボタン押下を処理する */
static bool frame_load(void)
{
#ifdef USE_DEBUGGER
	/* シングルステップか停止要求中はロードしない */
	if (dbg_is_stop_requested())
		return false;
#endif

	/* システムメニュー表示中は処理しない */
	if (is_sysmenu)
		return false;

	/* セーブロード無効時は処理しない */
	if (!is_save_load_enabled())
		return false;

	/* メッセージボックス非表示中は処理しない */
	if (is_hidden)
		return false;

	/* LOADボタンが押下されたとき */
	if (is_left_button_pressed && pointed_index == BTN_LOAD) {
		/* SEを再生する */
		play_se(conf_msgbox_btn_load_se);

		/* ボイスを停止する */
		set_mixer_input(VOICE_STREAM, NULL);

		/* ロードモードを開始する */
		need_load_mode = true;

		return true;
	}

	return false;
}

/* フレーム描画中のヒストリボタン押下および上キー押下および
 * マウスホイール上押下を処理する */
static bool frame_history(void)
{
#ifdef USE_DEBUGGER
	/* シングルステップか停止要求中はヒストリモードに遷移しない */
	if (dbg_is_stop_requested())
		return false;
#endif

	/* システムメニュー表示中は処理しない */
	if (is_sysmenu)
		return false;

	/* メッセージボックス非表示中は処理しない */
	if (is_hidden)
		return false;

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

		/* ヒストリモードを開始する */
		need_history_mode = true;

		return true;
	}

	return false;
}

/* コンフィグの表示を処理する */
static bool frame_config(void)
{
#ifdef USE_DEBUGGER
	/* シングルステップか停止要求中はセーブしない */
	if (dbg_is_stop_requested())
		return false;
#endif

	/* システムメニュー表示中は処理しない */
	if (is_sysmenu)
		return false;

	/* メッセージボックス非表示中は処理しない */
	if (is_hidden)
		return false;

	/* ESCキーが押下されたとき */
	if (is_escape_pressed) {
		/* SEを再生する */
		play_se(conf_msgbox_config_se);

		/* ボイスを停止する */
		set_mixer_input(VOICE_STREAM, NULL);

		/* コンフィグモードを開始する */
		need_config_mode = true;

		return true;
	}

	/* CONFIGボタンが押された場合 */
	if (is_left_button_pressed && pointed_index == BTN_CONFIG) {
		/* SEを再生する */
		play_se(conf_msgbox_btn_config_se);

		/* ボイスを停止する */
		set_mixer_input(VOICE_STREAM, NULL);

		/* コンフィグモードを開始する */
		need_config_mode = true;

		return true;
	}

	return false;
}

/* フレーム描画中のオートボタン押下およびオートモード制御を処理する */
static void frame_auto_mode(int *x, int *y, int *w, int *h)
{
	int lap;

	/* システムメニュー表示中は処理しない */
	if (is_sysmenu)
		return;

	/* スキップモードの場合、何もしない */
	if (is_skip_mode())
		return;

	/* メッセージボックス非表示中は処理しない */
	if (is_hidden)
		return;

	/* オートモードでない場合 */
	if (!is_auto_mode()) {
		/* AUTOボタンが押下された場合 */
		if (is_left_button_pressed && pointed_index == BTN_AUTO) {
			/* SEを再生する */
			play_se(conf_msgbox_btn_auto_se);

			/* オートモードを開始する */
			start_auto_mode();

			/* オートモードバナーを表示する */
			show_automode_banner(true);

			/* メッセージ表示とボイス再生を未完了にする */
			is_auto_mode_wait = false;

			/* 以降のクリック処理を行わない */
			is_left_button_pressed = false;
		}
	}

	/* オートモードの場合 */
	if (is_auto_mode()) {
		/* リターンキーと下キーの入力を処理しない */
		is_return_pressed = false;
		is_down_pressed = false;

		/* クリックされた場合 */
		if (is_left_button_pressed || is_right_button_pressed) {
			/* SEを再生する */
			play_se(conf_msgbox_auto_cancel_se);

			/* オートモードを終了する */
			stop_auto_mode();

			/* オートモードバナーを非表示にする */
			show_automode_banner(false);

			/* メッセージ表示とボイス再生を未完了にする */
			is_auto_mode_wait = false;

			/* ボタンを再描画する */
			frame_draw_buttons(false, x, y, w, h);

			/* バナーを消すために再描画する */
			*x = 0;
			*y = 0;
			*w = conf_window_width;
			*h = conf_window_height;

			/* 以降のクリック処理を行わない */
			is_left_button_pressed = false;
			is_right_button_pressed = false;
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
	 * セーブ画面かヒストリ画面かコンフィグ画面から戻った場合
	 *  - 表示は瞬時に終わり、ボイスも再生されていない
	 *  - すでに表示完了しているとみなす
	 */
	if (gui_flag)
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
		return (int)(AUTO_MODE_VOICE_WAIT * get_auto_speed());

	/* ボイスなしのとき、スケールを求める */
	scale = conf_automode_speed;
	if (scale == 0)
		scale = AUTO_MODE_TEXT_WAIT_SCALE;

	return (int)((float)total_chars * scale * get_auto_speed() * 1000.0f);
}

/* フレーム描画中のスキップモードの処理を行う */
static void frame_skip_mode(void)
{
	/* システムメニュー表示中は処理しない */
	if (is_sysmenu)
		return;

	/* オートモードの場合、何もしない */
	if (is_auto_mode())
		return;

	/* スキップモードの場合、何もしない */
	if (is_skip_mode())
		return;

	/* 未読の場合 */
	if (!is_skippable())
		return;

	/* メッセージボックス非表示中は処理しない */
	if (is_hidden)
		return;

	/* SKIPボタンが押下された場合 */
	if (is_left_button_pressed && pointed_index == BTN_SKIP) {
		/* SEを再生する */
		play_se(conf_msgbox_btn_skip_se);

		/* スキップモードを開始する */
		start_skip_mode();

		/* スキップモードバナーを表示する */
		show_skipmode_banner(true);

		/* 以降のクリック処理を行わない */
		is_left_button_pressed = false;
	}
}

/* フレーム描画中の右クリック押下を処理する */
static void frame_sysmenu(void)
{
	bool enter_sysmenu;

#ifdef USE_DEBUGGER
	/* シングルステップか停止要求中はシステムメニューに入らない */
	if (dbg_is_stop_requested())
		return;
#endif

	/* システムメニューを表示中の場合 */
	if (is_sysmenu) {
		/* 右クリックされた場合 */
		if (is_right_button_pressed) {
			/* SEを再生する */
			play_se(conf_sysmenu_leave_se);

			/* システムメニューを終了する */
			is_sysmenu = false;
			is_sysmenu_finished = true;

			/* 以降のクリック処理を行わない */
			is_right_button_pressed = false;
			return;
		}

		/* ポイントされているシステムメニューのボタンを求める */
		old_sysmenu_pointed_index = sysmenu_pointed_index;
		sysmenu_pointed_index = get_sysmenu_pointed_button();

		/* ボタンのないところを左クリックされた場合 */
		if (sysmenu_pointed_index == SYSMENU_NONE &&
		    is_left_button_pressed) {
			/* SEを再生する */
			play_se(conf_sysmenu_leave_se);

			/* システムメニューを終了する */
			is_sysmenu = false;
			is_sysmenu_finished = true;

			/* 以降のクリック処理を行わない */
			is_left_button_pressed = false;
			return;
		}

		/* セーブロードが無効な場合 */
		if (!is_save_load_enabled() &&
		    (sysmenu_pointed_index == SYSMENU_QSAVE ||
		     sysmenu_pointed_index == SYSMENU_QLOAD ||
		     sysmenu_pointed_index == SYSMENU_SAVE ||
		     sysmenu_pointed_index == SYSMENU_LOAD))
			sysmenu_pointed_index = SYSMENU_NONE;

		/* クイックセーブデータがない場合 */
		if (!have_quick_save_data() &&
		    sysmenu_pointed_index == SYSMENU_QLOAD)
			sysmenu_pointed_index = SYSMENU_NONE;
			
		/* スキップできない場合 */
		if (!is_skippable() && sysmenu_pointed_index == SYSMENU_SKIP)
			sysmenu_pointed_index = SYSMENU_NONE;

		/* 左クリックされていない場合、何もしない */
		if (!is_left_button_pressed)
			return;
		is_left_button_pressed = false;

		/* クイックセーブが左クリックされた場合 */
		if (sysmenu_pointed_index == SYSMENU_QSAVE) {
			/* SEを再生する */
			play_se(conf_sysmenu_qsave_se);

			/* システムメニューを終了する */
			is_sysmenu = false;
			is_sysmenu_finished = true;

			/* クイックセーブを行う */
			quick_save();
			return;
		}

		/* クイックロードが左クリックされた場合 */
		if (sysmenu_pointed_index == SYSMENU_QLOAD) {
			/* クイックロードを行う */
			if (quick_load()) {
				/* SEを再生する */
				play_se(conf_sysmenu_qload_se);

				/* システムメニューを終了する */
				is_sysmenu = false;
				is_sysmenu_finished = true;

				/* 後処理を行う */
				did_quick_load = true;
			}
			return;
		}

		/* セーブが左クリックされた場合 */
		if (sysmenu_pointed_index == SYSMENU_SAVE) {
			/* SEを再生する */
			play_se(conf_sysmenu_save_se);

			/* システムメニューを終了する */
			is_sysmenu = false;
			is_sysmenu_finished = true;

			/* セーブモードに移行する */
			need_save_mode = true;
			return;
		}

		/* ロードが左クリックされた場合 */
		if (sysmenu_pointed_index == SYSMENU_LOAD) {
			/* SEを再生する */
			play_se(conf_sysmenu_load_se);

			/* システムメニューを終了する */
			is_sysmenu = false;
			is_sysmenu_finished = true;

			/* ロードモードに移行する */
			need_load_mode = true;
			return;
		}

		/* オートが左クリックされた場合 */
		if (sysmenu_pointed_index == SYSMENU_AUTO) {
			/* SEを再生する */
			play_se(conf_sysmenu_auto_se);

			/* システムメニューを終了する */
			is_sysmenu = false;
			is_sysmenu_finished = true;

			/* オートモードを開始する */
			start_auto_mode();

			/* オートモードバナーを表示する */
			show_automode_banner(true);

			/* メッセージ表示とボイス再生を未完了にする */
			is_auto_mode_wait = false;

			/* 以降のクリック処理を行わない */
			is_left_button_pressed = false;
			return;
		}

		/* スキップが左クリックされた場合 */
		if (sysmenu_pointed_index == SYSMENU_SKIP) {
			/* SEを再生する */
			play_se(conf_sysmenu_skip_se);

			/* システムメニューを終了する */
			is_sysmenu = false;
			is_sysmenu_finished = true;

			/* スキップモードを開始する */
			start_skip_mode();

			/* スキップモードバナーを表示する */
			show_skipmode_banner(true);
			return;
		}

		/* ヒストリが左クリックされた場合 */
		if (sysmenu_pointed_index == SYSMENU_HISTORY) {
			/* SEを再生する */
			play_se(conf_sysmenu_history_se);

			/* システムメニューを終了する */
			is_sysmenu = false;
			is_sysmenu_finished = true;

			/* ヒストリモードを開始する */
			need_history_mode = true;
			return;
		}

		/* コンフィグが左クリックされた場合 */
		if (sysmenu_pointed_index == SYSMENU_CONFIG) {
			/* SEを再生する */
			play_se(conf_sysmenu_config_se);

			/* システムメニューを終了する */
			is_sysmenu = false;
			is_sysmenu_finished = true;

			/* コンフィグモードを開始する */
			need_config_mode = true;
			return;
		}

		return;
	}

	/* コンフィグでシステムメニューを表示しない場合 */
	if (conf_sysmenu_hidden)
		return;

	/* メッセージボックス非表示中は処理しない */
	if (is_hidden)
		return;

	/* オートモード中は処理しない */
	if (is_auto_mode())
		return;

	/* スキップモード中は処理しない */
	if (is_skip_mode())
		return;

	enter_sysmenu = false;

	/* 右クリックされたとき */
	if (is_right_button_pressed) {
		enter_sysmenu = true;
		is_right_button_pressed = false;
	}

	/* 折りたたみシステムメニューがクリックされたとき */
	if (is_left_button_pressed && is_collapsed_sysmenu_pointed()) {
		enter_sysmenu = true;
		is_left_button_pressed = false;
	}

	/* システムメニューに入るとき */
	if (enter_sysmenu) {
		/* SEを再生する */
		play_se(conf_sysmenu_enter_se);

		/* システムメニューを表示する */
		is_sysmenu = true;
		is_sysmenu_first_frame = true;
		is_sysmenu_finished = false;
		sysmenu_pointed_index = get_sysmenu_pointed_button();
		old_sysmenu_pointed_index = SYSMENU_NONE;
	}
}

/* メッセージボックスの非表示/表示切り替えを処理する */
static void frame_hide(int *x, int *y, int *w, int *h)
{
	/* メッセージボックスを表示中の場合 */
	if (!is_hidden) {
		/*
		 * スペースキーの押下と、メッセージボックスを消すボタンの
		 * クリックを処理する
		 */
		if (is_space_pressed ||
		    (is_left_button_pressed && pointed_index == BTN_HIDE)) {
			/* メッセージボックスを非表示にする */
			is_hidden = true;
			if (get_command_type() == COMMAND_SERIF)
				show_namebox(false);
			show_msgbox(false);
			show_click(false);

			/* SEを再生する */
			play_se(conf_msgbox_hide_se);

			/* 画面全体を再描画する */
			*x = 0;
			*y = 0;
			*w = conf_window_width;
			*h = conf_window_height;

			/* 以降の左クリックを処理しない */
			is_left_button_pressed = false;
		}
		return;
	}

	/* メッセージボックスを非表示中の場合、マウスとキーの押下を処理する */
	if(is_space_pressed || is_return_pressed || is_down_pressed ||
	   is_left_button_pressed || is_right_button_pressed) {
		/* メッセージボックスを表示する */
		is_hidden = false;
		if (get_command_type() == COMMAND_SERIF)
			show_namebox(true);
		show_msgbox(true);
		show_click(true);

		/* SEを再生する */
		play_se(conf_msgbox_show_se);

		/* 画面全体を再描画する */
		*x = 0;
		*y = 0;
		*w = conf_window_width;
		*h = conf_window_height;

		/* 以降のクリックを処理しない */
		is_right_button_pressed = false;
		is_left_button_pressed = false;
		is_return_pressed = false;
		is_down_pressed = false;
	}
}

/* フレーム描画中のメイン処理を行う */
static void frame_main(int *x, int *y, int *w, int *h)
{
	/* メッセージボックス非表示中は処理しない */
	if (is_hidden)
		return;

	/* メインの表示処理を行う */
	if (!is_sysmenu) {
		/* 入力があったらボイスを止める */
		if (!conf_voice_stop_off &&
		    (is_skippable() && !is_non_interruptible() &&
		     (is_skip_mode() || is_control_pressed)))
			set_mixer_input(VOICE_STREAM, NULL);

		/* 文字かクリックアニメーションを描画する */
		if (drawn_chars < total_chars)
			draw_msgbox(x, y, w, h);
		else if (!is_sysmenu_finished)
			draw_click(x, y, w, h);

		/* システムメニューが終了された直後の場合 */
		if (is_sysmenu_finished) {
			/* 画面全体を再描画する */
			*x = 0;
			*y = 0;
			*w = conf_window_width;
			*h = conf_window_height;
		}

		/* オートモードかスキップモードのバナーの描画領域を取得する */
		draw_banners(x, y, w, h);
	} else {
		/* ステージを再描画するか求める(計算だけで描画しない) */
		draw_sysmenu(true, x, y, w, h);
	}
}

/* システムメニューを描画する */
static void draw_sysmenu(bool calc_only, int *x, int *y, int *w, int *h)
{
	int bx, by, bw, bh;
	bool qsave_sel, qload_sel, save_sel, load_sel, auto_sel, skip_sel;
	bool history_sel, config_sel, redraw;

	/* 描画するかの判定状態を初期化する */
	qsave_sel = false;
	qload_sel = false;
	save_sel = false;
	load_sel = false;
	auto_sel = false;
	skip_sel = false;
	history_sel = false;
	config_sel = false;
	redraw = false;

	/* システムメニューの最初のフレームの場合、描画する */
	if (is_sysmenu_first_frame)
		redraw = true;

	/* クイックセーブボタンがポイントされているかを取得する */
	if (sysmenu_pointed_index == SYSMENU_QSAVE) {
		qsave_sel = true;
		if (old_sysmenu_pointed_index != SYSMENU_QSAVE &&
		    !is_sysmenu_first_frame) {
			play_se(conf_sysmenu_change_se);
			redraw = true;
		}
	}

	/* クイックロードボタンがポイントされているかを取得する */
	if (sysmenu_pointed_index == SYSMENU_QLOAD) {
		qload_sel = true;
		if (old_sysmenu_pointed_index != SYSMENU_QLOAD &&
		    !is_sysmenu_first_frame) {
			play_se(conf_sysmenu_change_se);
			redraw = true;
		}
	}

	/* セーブボタンがポイントされているかを取得する */
	if (sysmenu_pointed_index == SYSMENU_SAVE) {
		save_sel = true;
		if (old_sysmenu_pointed_index != SYSMENU_SAVE &&
		    !is_sysmenu_first_frame) {
			play_se(conf_sysmenu_change_se);
			redraw = true;
		}
	}

	/* ロードボタンがポイントされているかを取得する */
	if (sysmenu_pointed_index == SYSMENU_LOAD) {
		load_sel = true;
		if (old_sysmenu_pointed_index != SYSMENU_LOAD &&
		    !is_sysmenu_first_frame) {
			play_se(conf_sysmenu_change_se);
			redraw = true;
		}
	}

	/* オートがポイントされているかを取得する */
	if (sysmenu_pointed_index == SYSMENU_AUTO) {
		auto_sel = true;
		if (old_sysmenu_pointed_index != SYSMENU_AUTO &&
		    !is_sysmenu_first_frame) {
			play_se(conf_sysmenu_change_se);
			redraw = true;
		}
	}

	/* スキップがポイントされているかを取得する */
	if (sysmenu_pointed_index == SYSMENU_SKIP) {
		skip_sel = true;
		if (old_sysmenu_pointed_index != SYSMENU_SKIP &&
		    !is_sysmenu_first_frame) {
			play_se(conf_sysmenu_change_se);
			redraw = true;
		}
	}

	/* ヒストリがポイントされているかを取得する */
	if (sysmenu_pointed_index == SYSMENU_HISTORY) {
		history_sel = true;
		if (old_sysmenu_pointed_index != SYSMENU_HISTORY &&
		    !is_sysmenu_first_frame) {
			play_se(conf_sysmenu_change_se);
			redraw = true;
		}
	}

	/* コンフィグがポイントされているかを取得する */
	if (sysmenu_pointed_index == SYSMENU_CONFIG) {
		config_sel = true;
		if (old_sysmenu_pointed_index != SYSMENU_CONFIG &&
		    !is_sysmenu_first_frame) {
			play_se(conf_sysmenu_change_se);
			redraw = true;
		}
	}

	/* ポイント項目がなくなった場合 */
	if (sysmenu_pointed_index == SYSMENU_NONE) {
		if (old_sysmenu_pointed_index != SYSMENU_NONE)
			redraw = true;
	}

	/* GPUを利用している場合 */
	if (is_gpu_accelerated())
		redraw = true;
		
	/* 描画する */
	if (redraw) {
		if (!calc_only) {
			draw_stage_sysmenu(true,
					   is_skippable(),
					   is_save_load_enabled(),
					   is_save_load_enabled() &&
					   have_quick_save_data(),
					   qsave_sel,
					   qload_sel,
					   save_sel,
					   load_sel,
					   auto_sel,
					   skip_sel,
					   history_sel,
					   config_sel,
					   x, y, w, h);
			is_sysmenu_first_frame = false;
		} else {
			get_sysmenu_rect(&bx, &by, &bw, &bh);
			union_rect(x, y, w, h, *x, *y, *w, *h, bx, by, bw, bh);
		}
	}
}

/* 折りたたみシステムメニューを描画する */
static void draw_collapsed_sysmenu(int *x, int *y, int *w, int *h)
{
	bool is_pointed;

	/* 折りたたみシステムメニューがポイントされているか調べる */
	is_pointed = is_collapsed_sysmenu_pointed();

	/* 描画する */
	draw_stage_collapsed_sysmenu(is_pointed, x, y, w, h);

	/* SEを再生する */
	if (!is_sysmenu_finished &&
	    (is_collapsed_sysmenu_pointed_prev != is_pointed))
		play_se(conf_sysmenu_collapsed_se);

	/* 折りたたみシステムメニューのポイント状態を保持する */
	is_collapsed_sysmenu_pointed_prev = is_pointed;
}

/* 折りたたみシステムメニューがポイントされているか調べる */
static bool is_collapsed_sysmenu_pointed(void)
{
	int bx, by, bw, bh;

	get_collapsed_sysmenu_rect(&bx, &by, &bw, &bh);
	if (mouse_pos_x >= bx && mouse_pos_x < bx + bw &&
	    mouse_pos_y >= by && mouse_pos_y < by + bh)
		return true;

	return false;
}

/* バナーを描画する */
static void draw_banners(int *x, int *y, int *w, int *h)
{
	int bx, by,bw, bh;

	if (is_auto_mode()) {
		get_automode_banner_rect(&bx, &by, &bw, &bh);
		union_rect(x, y, w, h, *x, *y, *w, *h, bx, by, bw, bh);
	} else if (is_skip_mode()) {
		get_skipmode_banner_rect(&bx, &by, &bw, &bh);
		union_rect(x, y, w, h, *x, *y, *w, *h, bx, by, bw, bh);
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
	/* コンフィグで未読スキップが指定されている場合 */
	if (conf_msgbox_skip_unseen)
		return true;

	/* 既読である場合 */
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

	/* クリックアニメーションを非表示にする */
	show_click(false);

	/* 表示中のメッセージをなしとする */
	if (!need_save_mode && !need_load_mode && !need_history_mode &&
	    !need_config_mode)
		clear_message_registered();

	/* 既読にする */
	set_seen();

	/* 次のコマンドに移動する */
	if (!did_quick_load && !need_save_mode && !need_load_mode &&
	    !need_history_mode && !need_config_mode) {
		if (!move_to_next_command())
			return false;
	}

	return true;
}
