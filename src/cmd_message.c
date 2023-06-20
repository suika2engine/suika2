/* -*- coding: utf-8; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Suika 2
 * Copyright (C) 2001-2023, TABATA Keiichi. All rights reserved.
 */

/*
 * The implementation of the message and the line commands.
 *
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
 *  - 2023/06/19 リファクタリング
 */

/*
 * [Notice (注意)]
 *  - Decomposed Unicode characters are not supported, use precomposed ones.
 *    (Unicodeの合成はサポートしないので、合成済みの文字列を使用してください)
 *
 * [読み解くにあたって]
 *  - メッセージボックス内のボタンと、システムメニュー内のボタンがある
 *    - これらが呼び出す機能は同じ
 *    - メッセージボックスを隠すボタンは、メッセージボックス内のみにある
 *  - システムメニュー非表示時には、折りたたみシステムメニューが表示される
 */

#include "suika.h"

/* false assertion */
#define ASSERT_INVALID_BTN_INDEX (0)

/* メッセージボックス内のボタンのインデックス */
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

/*
 * オートモードでボイスありのときの待ち時間
 *  - 表示と再生の完了からどれくらい待つかの基本時間
 *  - 実際にはオートモードスピードの係数を乗算して使用する
 */
#define AUTO_MODE_VOICE_WAIT		(4000)

/*
 * オートモードでボイスなしのときの待ち時間係数のデフォルト値
 *  - automode.speed=0 のときに使用される
 *  - 文字数に乗算して使用する
 */
#define AUTO_MODE_TEXT_WAIT_SCALE	(0.15f)

/*
 * 文字列バッファ
 */

/* 描画する名前 */
static char *name_top;

/*
 * 描画する本文 (バッファの先頭)
 *  - 文字列の内容は変数の値を展開した後のもの
 *  - 行継続の場合、先頭の'\\'は含まれない
 *  - 行継続の場合、先頭の連続する"\\n"は含まれない
 */
static char *msg_top;

/*
 * 描画状態
 */

/*
 * 描画する本文の現在の先頭位置
 *  - msg_topから開始される
 *  - 描画(またはエスケープシーケンスとして解釈)された分だけポインタが進む
 */
static const char *msg_cur;

/*
 * 描画する文字の総数
 *  - エスケープシーケンスを除外した合計の表示文字数
 *  - Unicodeの合成はサポートしていない
 *    - 基底文字+結合文字はそれぞれ1文字としてカウントする
 */
static int total_chars;

/* すでに描画した文字数 (0 <= drawn_chars <= total_chars) */
static int drawn_chars;

/* スペースの直後であるか (日本語以外のワードラッピング用) */
static bool is_after_space;

/*
 * 描画位置
 */

/*
 * 現在の描画位置
 *  - 他のコマンドに移ったり、GUIから戻ってきた場合も、保持される
 *  - TODO: main.cに移動する
 */
static int pen_x;
static int pen_y;

/* 描画開始位置(重ね塗りのため) */
static int orig_pen_x;
static int orig_pen_y;

/* メッセージボックスの位置とサイズ */
static int msgbox_x;
static int msgbox_y;
static int msgbox_w;
static int msgbox_h;

/*
 * 色
 */

/* 文字の色 */
static pixel_t color;

/* 文字の縁取りの色 */
static pixel_t outline_color;

/*
 * 行継続モード
 */

/* 行継続モードであるか */
static bool is_continue_mode;

/* 重ね塗りしているか */
static bool is_overcoating;

/*
 * ボイス
 */

/* ボイスがあるか */
static bool have_voice;

/* ビープ音再生中であるか */
static bool is_beep;

/*
 * クリックアニメーション
 */

/* クリックアニメーションの初回描画か */
static bool is_click_first;

/* クリックアニメーションの表示状態 */
static bool is_click_visible;

/* コマンドの経過時刻を表すストップウォッチ */
static stop_watch_t click_sw;

/*
 * オートモード
 */

/* オートモードでメッセージ表示とボイス再生の完了後の待ち時間中か */
static bool is_auto_mode_wait;

/* オートモードの経過時刻を表すストップウォッチ */
static stop_watch_t auto_sw;

/*
 * コマンドが開始されたときの状態 
 */

/* ロードによって開始されたか */
static bool load_flag;

/* GUIコマンド終了後の最初のコマンド、あるいはシステムGUIから復帰したか */
static bool gui_flag;

/*
 * システム遷移フラグ
 */

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

/*
 * 非表示
 */

/* スペースキーによる非表示が実行中であるか */
static bool is_hidden;

/*
 * メッセージボックス内のボタン
 */

/* ポイント中のボタン */
static int pointed_index;

/*
 * システムメニュー
 */

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

/* 初期化 */
static bool init(int *x, int *y, int *w, int *h);
static void init_flags(void);
static void init_auto_mode(void);
static void init_skip_mode(void);
static bool init_name_top(void);
static bool init_msg_top(void);
static bool is_escape_sequence_char(char c);
static const char *skip_lf(const char *m, int *lf);
static void put_space(void);
static bool register_message_for_history(const char *msg);
static char *concat_serif(const char *name, const char *serif);
static int count_chars(const char *msg);
static void init_colors(pixel_t *color, pixel_t *outline_color);
static bool init_serif(int *x, int *y, int *w, int *h);
static bool check_play_voice(void);
static bool play_voice(void);
static void set_character_volume_by_name(const char *name);
static void draw_namebox(void);
static int get_namebox_width(void);
static void init_pen(void);
static void init_msgbox(int *x, int *y, int *w, int *h);
static void init_click(void);
static void init_first_draw_area(int *x, int *y, int *w, int *h);
static void init_pointed_index(void);
static int get_pointed_button(void);
static void get_button_rect(int btn, int *x, int *y, int *w, int *h);
static void init_repetition(void);

/* フレーム処理 */
static bool frame_auto_mode(int *x, int *y, int *w, int *h);
static void action_auto_start(int *x, int *y, int *w, int *h);
static void action_auto_end(int *x, int *y, int *w, int *h);
static bool check_auto_play_condition(void);
static int get_wait_time(void);
static bool frame_buttons(int *x, int *y, int *w, int *h);
static bool draw_buttons(int *x, int *y, int *w, int *h);
static bool process_button_click(int *x, int *y, int *w, int *h);
static bool process_qsave_click(void);
static void action_qsave(void);
static bool process_qload_click(void);
static bool action_qload(void);
static bool process_save_click(void);
static void action_save(void);
static bool process_load_click(void);
static void action_load(void);
static bool process_auto_click(int *x, int *y, int *w, int *h);
static bool process_skip_click(int *x, int *y, int *w, int *h);
static void action_skip(int *x, int *y, int *w, int *h);
static bool process_history_click(void);
static void action_history(void);
static bool process_config_click(void);
static void action_config(void);
static bool process_hide_click(int *x, int *y, int *w, int *h);
static bool frame_sysmenu(int *x, int *y, int *w, int *h);
static int get_sysmenu_pointed_button(void);
static void get_sysmenu_button_rect(int btn, int *x, int *y, int *w, int *h);

/* メイン描画処理 */
static void draw_frame(int *x, int *y, int *w, int *h);
static bool is_end_of_msg(void);
static void set_end_of_msg(void);
static void draw_msgbox(int *x, int *y, int *w, int *h);
static int get_frame_chars(void);
static bool is_canceled_by_skip(void);
static bool is_fast_forward_by_click(void);
static bool calc_frame_chars_by_lap(void);
static void process_escape_sequence(void);
static void process_escape_sequence_lf(void);
static void do_word_wrapping(void);
static int get_en_word_width(void);
static void process_lf(uint32_t c, int glyph_width);
static void draw_click(int *x, int *y, int *w, int *h);
static void check_stop_click_animation(void);
static void draw_sysmenu(bool calc_only, int *x, int *y, int *w, int *h);
static void draw_collapsed_sysmenu(int *x, int *y, int *w, int *h);
static bool is_collapsed_sysmenu_pointed(void);
static void union_banners(int *x, int *y, int *w, int *h);

/* その他 */
static void play_se(const char *file);
static bool is_skippable(void);

/* 終了処理 */
static bool cleanup(int *x, int *y, int *w, int *h);

/*
 * メッセージ・セリフコマンド
 */
bool message_command(int *x, int *y, int *w, int *h)
{
	/* 初期化処理を行う */
	if (!is_in_command_repetition())
		if (!init(x, y, w, h))
			return false;

	/* 各種操作を処理する */
	do {
		/* オートモードを処理する */
		if (frame_auto_mode(x, y, w, h))
			break;

		/* メッセージボックス内のボタンを処理する */
		if (frame_buttons(x, y, w, h))
			break;

		/* システムメニューを処理する */
		if (frame_sysmenu(x, y, w, h))
			break;
	} while (0);

	/*
	 * メイン表示処理を行う
	 *  - クイックロードされた場合は処理しない
	 *  - セーブ・ロード画面に遷移する場合はサムネイル描画のため処理する
	 */
	if (!did_quick_load)
		draw_frame(x, y, w, h);

	/* クイックロード・セーブ・ロード・ヒストリモードが選択された場合 */
	if (did_quick_load || need_save_mode || need_load_mode ||
	    need_history_mode || need_config_mode)
		stop_command_repetition();

	/* 終了処理を行う */
	if (!is_in_command_repetition())
		if (!cleanup(x, y, w, h))
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
	if (!conf_sysmenu_hidden && !is_hidden) {
		if (is_sysmenu)
			draw_sysmenu(false, x, y, w, h);
		else if (conf_sysmenu_transition)
			draw_collapsed_sysmenu(x, y, w, h);
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

/*
 * 初期化
 */

/* 初期化処理を行う */
static bool init(int *x, int *y, int *w, int *h)
{
	/* フラグを初期化する */
	init_flags();

	/* オートモードの場合の初期化を行う */
	init_auto_mode();

	/* スキップモードの場合の初期化を行う */
	init_skip_mode();

	/* 名前を取得する */
	if (!init_name_top())
		return false;

	/* メッセージを取得する */
	if (!init_msg_top())
		return false;

	/* 文字色の初期化を行う */
	init_colors(&color, &outline_color);

	/* セリフ固有の初期化を行う */
	if (!init_serif(x, y, w, h))
		return false;

	/*
	 * メッセージの表示中状態をセットする
	 *  - システムGUIに入っても保持される
	 *  - メッセージから次のコマンドに移行するときにクリアされる
	 */
	if (!is_message_active())
		set_message_active();

	/* ペンの位置を初期化する */
	init_pen();

	/* メッセージボックスを初期化する */
	init_msgbox(x, y, w, h);

	/* クリックアニメーションを非表示の状態にする */
	init_click();

	/* 初回に描画する矩形を求める */
	init_first_draw_area(x, y, w, h);

	/* ボタンの選択状態を取得する */
	init_pointed_index();

	/* 繰り返し動作を設定する */
	init_repetition();

	return true;
}

/* フラグを初期化する */
static void init_flags(void)
{
	/* GUIから戻ったばかりかチェックする */
	gui_flag = check_gui_flag();

	/* ロードされたばかりかチェックする */
	load_flag = check_load_flag();

	/* スペースキーによる非表示でない状態にする */
	is_hidden = false;

	/* オートモードの状態設定を行う */
	is_auto_mode_wait = false;

	/* セーブ・ロード・ヒストリ・コンフィグの状態設定を行う */
	did_quick_load = false;
	need_save_mode = false;
	need_load_mode = false;
	need_history_mode = false;
	need_config_mode = false;

	/* システムメニューの状態設定を行う */
	is_sysmenu = false;
	is_sysmenu_finished = false;
	is_collapsed_sysmenu_pointed_prev = false;

	/* 重ね塗りでない状態にする */
	is_overcoating = false;
}

/* オートモードの場合の初期化処理を行う */
static void init_auto_mode(void)
{
	/* オートモードの場合 */
	if (is_auto_mode()) {
		/* リターンキー、下キーの入力を無効にする */
		is_return_pressed = false;
		is_down_pressed = false;
	}
}

/* スキップモードの場合の初期化処理を行う */
static void init_skip_mode(void)
{
	if (is_skip_mode()) {
		/* 未読に到達した場合、スキップモードを終了する */
		if (!is_skippable()) {
			stop_skip_mode();
			show_skipmode_banner(false);
			return;
		}

		/* クリックされた場合 */
		if (is_right_clicked || is_left_clicked ||
		    is_escape_pressed) {
			/* SEを再生する */
			play_se(conf_msgbox_skip_cancel_se);

			/* スキップモードを終了する */
			stop_skip_mode();
			show_skipmode_banner(false);

			/* 以降のクリック処理を行わない */
			clear_input_state();
		}
	}
}

/* 名前を取得する */
static bool init_name_top(void)
{
	const char *raw, *exp;

	if (get_command_type() == COMMAND_SERIF) {
		raw = get_string_param(SERIF_PARAM_NAME);
		exp = expand_variable(raw);
		name_top = strdup(exp);
		if (name_top == NULL) {
			log_memory();
			return false;
		}
	} else {
		name_top = NULL;
	}

	return true;
}

/* メッセージを取得する */
static bool init_msg_top(void)
{
	const char *raw_msg;
	char *exp_msg;
	int lf;
	bool is_serif;

	/* 引数を取得する */
	is_serif = get_command_type() == COMMAND_SERIF;
	if (is_serif)
		raw_msg = get_string_param(SERIF_PARAM_MESSAGE);
	else
		raw_msg = get_string_param(MESSAGE_PARAM_MESSAGE);

	/* 継続行かチェックする */
	if (*raw_msg == '\\' && !is_escape_sequence_char(*(raw_msg + 1))) {
		is_continue_mode = true;

		/* 先頭の改行をスキップする */
		raw_msg = skip_lf(raw_msg + 1, &lf);

		/* 日本語以外のロケールで、改行がない場合 */
		if (conf_locale != LOCALE_JA && lf == 0)
			put_space();
	} else {
		is_continue_mode = false;
	}

	/* 変数を展開する */
	exp_msg = strdup(expand_variable(raw_msg));
	if (exp_msg == NULL) {
		log_memory();
		return false;
	}

	/* ヒストリ画面用にメッセージ履歴を登録する */
	if (!register_message_for_history(exp_msg)) {
		free(exp_msg);
		return false;
	}

	/* セーブ用にメッセージを保存する */
	if (!set_last_message(exp_msg)) {
		free(exp_msg);
		return false;
	}

	/* セリフの場合、実際に表示するメッセージを修飾する */
	if (is_serif) {
		if (conf_namebox_hidden) {
			/* 名前とカギカッコを付加する */
			msg_top = concat_serif(name_top, exp_msg);
			if (msg_top == NULL) {
				log_memory();
				free(exp_msg);
				return false;
			}
			free(exp_msg);
		} else if (conf_serif_quote && !is_quoted_serif(exp_msg)) {
			/* カギカッコを付加する */
			msg_top = concat_serif("", exp_msg);
			if (msg_top == NULL) {
				log_memory();
				free(exp_msg);
				return false;
			}
			free(exp_msg);
		} else {
			/* 何も付加しない場合 */
			msg_top = exp_msg;
		}
	} else {
		/* メッセージの場合はそのまま表示する */
		msg_top = exp_msg;
	}

	/* 表示位置を保存する */
	msg_cur = msg_top;

	/* メッセージの文字数を求める */
	total_chars = count_chars(msg_cur);
	drawn_chars = 0;

	/* 先頭文字はスペースの直後とみなす */
	is_after_space = true;

	return true;
}

/* エスケープ文字かチェックする */
static bool is_escape_sequence_char(char c)
{
	if (c == 'n')
		return true;

	/* TODO: 文字色などの文字を追加 */

	return false;
}

/* 継続行の先頭の改行をスキップする */
static const char *skip_lf(const char *m, int *lf)
{
	assert(is_continue_mode);

	*lf = 0;
	while (*m == '\\') {
		if (*(m + 1) == 'n') {
			(*lf)++;
			m += 2;
			if (!gui_flag) {
				pen_x = conf_msgbox_margin_left;
				pen_y += conf_msgbox_margin_line;
			}
		}
	}
	return m;
}

/* 空白文字の分だけカーソルを移動する */
static void put_space(void)
{
	int cw;

	cw = get_glyph_width(' ');
	if (pen_x + cw >= msgbox_w - conf_msgbox_margin_right) {
		pen_y += conf_msgbox_margin_line;
		pen_x = conf_msgbox_margin_left;
	} else {
		pen_x += cw;
	}
}

/* ヒストリ画面用にメッセージ履歴を登録する */
static bool register_message_for_history(const char *msg)
{
	const char *voice;
	bool reg;

	assert(msg != NULL);

	/* 二重登録を防ぐ */
	if (load_flag) {
		/* ロード直後は必ず登録する */
		reg = true;
	} if (gui_flag) {
		/* GUIの直後のとき */
		if (is_message_active()) {
			/* システムGUIから戻った場合は登録しない */
			reg = false;
		} else {
			/* GUIコマンドの直後の場合は登録する */
			reg = true;
		}
	} else {
		/* その他の場合は登録する */
		reg = true;
	}
	if (!reg)
		return true;

	/* メッセージ履歴を登録する */
	if (get_command_type() == COMMAND_SERIF) {
		assert(name_top != NULL);

		/* ボイスファイルを取得する */
		voice = get_string_param(SERIF_PARAM_VOICE);

		/* ビープ音は履歴画面で再生しない */
		if (voice[0] == '@')
			voice = NULL;

		/* セリフをヒストリ画面用に登録する */
		if (!register_message(name_top, msg, voice))
			return false;
	} else {
		/* メッセージをヒストリ画面用に登録する */
		if (!register_message(NULL, msg, NULL))
			return false;
	}

	/* 成功 */
	return true;
}

/* 名前とメッセージを連結する */
static char *concat_serif(const char *name, const char *serif)
{
	char *ret;
	size_t len, lf, i;
	const char *prefix;
	const char *suffix;

	assert(name != NULL);
	assert(serif != NULL);

	/* 日本語ロケールかどうかでセリフの囲いを分ける */
	if (conf_locale == LOCALE_JA || conf_serif_quote) {
		prefix = U8("「");
		suffix = U8("」");
	} else {
		prefix = ": ";
		suffix = "";
	}

	/* 先頭の'\\' 'n'をカウントする */
	lf = 0;
	while (*serif == '\\') {
		if (*(serif + 1) == 'n') {
			lf++;
			serif += 2;
		}
	}

	/* 先頭の改行の先の文字列を作る */
	if (is_quoted_serif(serif)) {
		/* クオート済みの文字列の場合 */
		len = lf * 2 + strlen(name) + strlen(serif) + 1;
		ret = malloc(len);
		if (ret == NULL) {
			log_memory();
			return NULL;
		}
		snprintf(ret + lf * 2, len, "%s%s", name, serif);
	} else {
		/* クオートされていない文字列の場合 */
		len = lf * 2 + strlen(name) + strlen(prefix) +
			strlen(serif) + strlen(suffix) + 1;
		ret = malloc(len);
		if (ret == NULL) {
			log_memory();
			return NULL;
		}
		snprintf(ret + lf * 2, len, "%s%s%s%s", name, prefix, serif,
			 suffix);
	}

	/* 先頭の改行を埋める */
	for (i = 0; i < lf; i++) {
		ret[i * 2] = '\\';
		ret[i * 2 + 1] = 'n';
	}

	return ret;
}

/*
 * エスケープシーケンスを除いた文字数を取得する
 *  - Unicodeの合成はサポートしていない
 *  - 基底文字+結合文字はそれぞれ1文字としてカウントする
 */
static int count_chars(const char *msg)
{
	uint32_t wc;
	int count, mblen;

	count = 0;
	while (*msg) {
		/* 先頭のエスケープシーケンスを読み飛ばす */
		while (*msg == '\\') {
			if (*(msg + 1) == 'n') {
				/* 改行 */
				msg += 2;
			} else {
				/*
				 * 不正なエスケープシーケンス
				 *  - 読み飛ばさない
				 */
				break;
			}
		}
		if (*msg == '\0')
			break;

		/* 次の1文字を取得する */
		mblen = utf8_to_utf32(msg, &wc);
		if (mblen == -1)
			break;

		msg += mblen;
		count++;
	}

	return count;
}

/* 文字色を求める */
static void init_colors(pixel_t *color, pixel_t *outline_color)
{
	int i;

	/* セリフの場合 */
	if (get_command_type() == COMMAND_SERIF) {
		/* コンフィグでnameの指す名前が指定されているか */
		for (i = 0; i < SERIF_COLOR_COUNT; i++) {
			if (conf_serif_color_name[i] == NULL)
				continue;
			if (strcmp(name_top, conf_serif_color_name[i]) == 0) {
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

/* セリフコマンドを処理する */
static bool init_serif(int *x, int *y, int *w, int *h)
{
	int namebox_x, namebox_y, namebox_w, namebox_h;

	/*
	 * 描画範囲を更新する
	 *  - セリフコマンドだけでなく、メッセージコマンドでも、
	 *    名前ボックス領域を消すために描画範囲を設定する
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
	if (check_play_voice()) {
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

/* ボイスを再生するかを判断する */
static bool check_play_voice(void)
{
	/* XXX:
	  下記の式から変換した
	    if (is_non_interruptible() &&
	        (!gui_flag && !is_message_registered()))
	        ||
	        (!is_non_interruptible() &&
	         !(is_skip_mode() && is_skippable()) &&
	         ((is_auto_mode() || (!is_control_pressed || !is_skippable())) &&
	         !gui_flag &&
	         !is_message_registered())))
	*/

	/* システムGUIから戻った場合は再生しない */
	if (gui_flag && is_message_active())
		return false;

	/* 割り込み不可モードの場合は他の条件を考慮せず再生する */
	if (is_non_interruptible())
		return true;

	/* スキップモードで、かつ、既読の場合は再生しない */
	if (is_skip_mode() && is_skippable())
		return false;

	/* オートモードの場合は再生する */
	if (is_auto_mode())
		return true;

	/* 未読の場合は再生する */
	if (!is_skippable())
		return true;

	/* コントロールキーが押下されている場合は再生しない */
	if (is_control_pressed)
		return false;

	/* その他の場合は再生する */
	return true;
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

/* 名前ボックスを描画する */
static void draw_namebox(void)
{
	uint32_t c;
	int char_count, mblen, i, x, w;
	const char *name;

	/* 名前の文字列を取得する */
	name = name_top;

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

/* ペンの位置を初期化する */
static void init_pen(void)
{
	/* 継続行でなければ、メッセージの描画位置を初期化する */
	if (!is_continue_mode) {
		pen_x = conf_msgbox_margin_left;
		pen_y = conf_msgbox_margin_top;
	}

	/* 重ね塗りをする場合 */
	if (conf_msgbox_dim) {
		/*
		 * 描画開始位置を保存する
		 *  - XXX: GUIから戻った場合に更新すると、どうなる？
		 */
		if (!gui_flag) {
			orig_pen_x = pen_x;
			orig_pen_y = pen_y;
		}
	}
}

/* メッセージボックスを初期化する */
static void init_msgbox(int *x, int *y, int *w, int *h)
{
	/* メッセージボックスの矩形を取得する */
	get_msgbox_rect(&msgbox_x, &msgbox_y, &msgbox_w, &msgbox_h);

	/* 更新領域に含める */
	union_rect(x, y, w, h,
		   *x, *y, *w, *h,
		   msgbox_x, msgbox_y, msgbox_w, msgbox_h);

	/* 行継続でなければ、メッセージレイヤをクリアする */
	if (!is_continue_mode)
		clear_msgbox();

	/* メッセージレイヤを可視にする */
	show_msgbox(true);
}

/* クリックアニメーションを初期化する */
static void init_click(void)
{
	/* クリックレイヤを不可視にする */
	show_click(false);

	/* クリックアニメーションの初回表示フラグをセットする */
	is_click_first = true;

	/* クリックアニメーションの表示状態を保存する */
	is_click_visible = false;
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

/* 初期化処理において、繰り返し動作を設定する */
static void init_repetition(void)
{
	if (is_skippable() && !is_non_interruptible() &&
	    (is_skip_mode() || (!is_auto_mode() && is_control_pressed))) {
		/* 繰り返し動作せず、すぐに表示する */
	} else {
		/* コマンドが繰り返し呼び出されるようにする */
		start_command_repetition();

		/* 時間計測を開始する */
		reset_stop_watch(&click_sw);
	}
}

/*
 * フレームの処理
 */

/* オートモード制御を処理する */
static bool frame_auto_mode(int *x, int *y, int *w, int *h)
{
	int lap;

	/* オートモードでない場合、何もしない */
	if (!is_auto_mode())
		return false;

	/* オートモード中はシステムメニューを表示できない */
	assert(!is_sysmenu);

	/* オートモード中はスキップモードにできない */
	assert(!is_skip_mode());

	/* オートモード中はメッセージボックスを非表示にできない */
	assert(!is_hidden);

	/* クリックされた場合 */
	if (is_left_clicked || is_right_clicked || is_escape_pressed ||
	    is_return_pressed || is_down_pressed) {
		/* SEを再生する */
		play_se(conf_msgbox_auto_cancel_se);

		/* 以降のクリック処理を行わない */
		clear_input_state();

		/* オートモード終了アクションを処理する */
		action_auto_end(x, y, w, h);

		/* メッセージボックス内のボタンを再描画する */
		draw_buttons(x, y, w, h);

		/* クリックされた */
		return true;
	} else if (!is_auto_mode_wait) {
		/* メッセージ表示とボイス再生が未完了の場合 */

		/* すでに表示が完了していれば */
		if (check_auto_play_condition()) {
			/* 待ち時間に入る */
			is_auto_mode_wait = true;

			/* 時間計測を開始する */
			reset_stop_watch(&auto_sw);
		}
	} else {
		/* 待ち時間の場合 */

		/* 時間を計測する */
		lap = get_stop_watch_lap(&auto_sw);

		/* 時間が経過していれば、コマンドの終了処理に移る */
		if (lap >= get_wait_time()) {
			stop_command_repetition();

			/* コマンドを終了する */
			return true;
		}
	}

	/* クリックされず、コマンドも終了しない */
	return false;
}

/* オートモード開始アクションを処理する */
static void action_auto_start(int *x, int *y, int *w, int *h)
{
	/* オートモードを開始する */
	start_auto_mode();

	/* オートモードバナーを表示する */
	show_automode_banner(true);

	/* メッセージ表示とボイス再生を未完了にする */
	is_auto_mode_wait = false;

	/* バナーを再描画する */
	*x = 0;
	*y = 0;
	*w = conf_window_width;
	*h = conf_window_height;
}

/* オートモード終了アクションを処理する */
static void action_auto_end(int *x, int *y, int *w, int *h)
{
	/* オートモードを終了する */
	stop_auto_mode();

	/* オートモードバナーを非表示にする */
	show_automode_banner(false);

	/* メッセージ表示とボイス再生を未完了にする */
	is_auto_mode_wait = false;

	/* バナーを再描画する */
	*x = 0;
	*y = 0;
	*w = conf_window_width;
	*h = conf_window_height;
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
		/* 表示完了かつ再生完了しているか */
		if (is_mixer_sound_finished(VOICE_STREAM) && is_end_of_msg())
			return true;

		return false;
	}

	/*
	 * ボイスなしの場合
	 *  - 文字の表示完了をチェックする
	 */
	if (is_end_of_msg())
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

/* メッセージボックス内のボタンを処理する */
static bool frame_buttons(int *x, int *y, int *w, int *h)
{
	/* ボタンを描画する */
	if (draw_buttons(x, y, w, h)) {
		/* アクティブ項目が変わったのでSEを再生する */
		play_se(conf_msgbox_btn_change_se);
	}

	/* オートモードの場合、クリックを処理しない */
	if (is_auto_mode())
		return false;

	/* ボタンのクリックを処理する */
	if (process_button_click(x, y, w, h))
		return true;	/* クリックされた */

	/* クリックされなかった */
	return false;
}

/* メッセージボックス内のボタンを描画する */
static bool draw_buttons(int *x, int *y, int *w, int *h)
{
	int last_pointed_index, bx, by, bw, bh;

	/* システムメニュー表示中は処理しない */
	if (is_sysmenu)
		return false;

	/* メッセージボックス非表示中は処理しない */
	if (is_hidden)
		return false;

	/* 選択中のボタンを取得する */
	last_pointed_index = pointed_index;
	pointed_index = get_pointed_button();

	/* 選択状態に変更がない場合 */
	if (pointed_index == last_pointed_index)
		return false;

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
			return false;

		/* クイックセーブデータがない時のQLOADボタンの場合 */
		if (!have_quick_save_data() && pointed_index == BTN_QLOAD)
			return false;

		/* 未読の場合のSKIPボタンの場合 */
		if (!is_skippable() && pointed_index == BTN_SKIP)
			return false;

		/* ボタンを描画する */
		get_button_rect(pointed_index, &bx, &by, &bw, &bh);
		clear_msgbox_rect_with_fg(bx, by, bw, bh);
		union_rect(x, y, w, h,
			   *x, *y, *w, *h,
			   bx + conf_msgbox_x, by + conf_msgbox_y, bw, bh);

		/* ボタンが変更された */
		return true;
	}

	return false;
}

/* メッセージボックス内のボタンのクリックを処理する */
static bool process_button_click(int *x, int *y, int *w, int *h)
{
	/* システムメニュー表示中は処理しない */
	if (is_sysmenu)
		return false;

	/* 隠すボタンを処理する */
	if (process_hide_click(x, y, w, h))
		return true;

	/* メッセージボックス非表示中は以降処理しない */
	if (is_hidden)
		return false;

	/* クイックセーブボタンを処理する */
	if (process_qsave_click())
		return true;

	/* クイックロードボタンを処理する */
	if (process_qload_click())
		return true;

	/* セーブボタンを処理する */
	if (process_save_click())
		return true;

	/* ロードボタンを処理する */
	if (process_load_click())
		return true;

	/* オートボタンを処理する */
	if (process_auto_click(x, y, w, h))
		return true;

	/* スキップボタンを処理する */
	if (process_skip_click(x, y, w, h))
		return true;

	/* ヒストリボタンを処理する */
	if (process_history_click())
		return true;

	/* コンフィグボタンを処理する */
	if (process_config_click())
		return true;

	/* ボタンがクリックされなかった */
	return false;
}

/* メッセージボックス内のクイックセーブボタン押下を処理する */
static bool process_qsave_click(void)
{
#ifdef USE_DEBUGGER
	/* シングルステップか停止要求中はセーブしない */
	if (dbg_is_stop_requested())
		return false;
#endif

	/* セーブロード無効時は処理しない */
	if (!is_save_load_enabled())
		return false;

	/* クイックセーブボタンが押下されたとき */
	if (is_left_clicked && pointed_index == BTN_QSAVE) {
		/* SEを再生する */
		play_se(conf_msgbox_btn_qsave_se);

		/* 以降のクリック処理を行わない */
		clear_input_state();

		/* クイックセーブを処理する */
		action_qsave();

		return true;
	}

	return false;
}

/* クイックセーブを処理する */
static void action_qsave(void)
{
	/* サムネイルを作成する(GUIを経由しないのでここで作成する) */
	draw_stage_to_thumb();

	/* クイックセーブを行う */
	quick_save();
}

/* メッセージボックス内のクイックロードボタン押下を処理する */
static bool process_qload_click(void)
{
#ifdef USE_DEBUGGER
	/* シングルステップか停止要求中はロードしない */
	if (dbg_is_stop_requested())
		return false;
#endif

	/* セーブロード無効時は処理しない */
	if (!is_save_load_enabled())
		return false;

	/* クイックセーブデータがない場合は処理しない */
	if (!have_quick_save_data())
		return false;

	/* クイックロードボタンが押下されたとき */
	if (is_left_clicked && pointed_index == BTN_QLOAD) {
		/* SEを再生する */
		play_se(conf_msgbox_btn_qload_se);

		/* 以降のクリック処理を行わない */
		clear_input_state();

		/* クイックロードを行う */
		action_qload();
		if (quick_load()) {
			/* 後処理を行う */
			did_quick_load = true;

			return true;
		}
	}

	return false;
}

/* クイックロードを行う */
static bool action_qload(void)
{
	/* クイックロードを行う */
	if (!quick_load())
		return false;
	
	/* 後処理を行う */
	did_quick_load = true;
	return true;
}

/* メッセージボックス内のセーブボタン押下を処理する */
static bool process_save_click(void)
{
#ifdef USE_DEBUGGER
	/* シングルステップか停止要求中はセーブしない */
	if (dbg_is_stop_requested())
		return false;
#endif

	/* セーブロード無効時は処理しない */
	if (!is_save_load_enabled())
		return false;

	/* セーブボタンが押下されたとき */
	if (is_left_clicked && pointed_index == BTN_SAVE) {
		/* SEを再生する */
		play_se(conf_msgbox_btn_save_se);

		/* ボイスを停止する */
		set_mixer_input(VOICE_STREAM, NULL);

		/* セーブ画面への遷移を処理する */
		action_save();
		return true;
	}

	return false;
}

/* セーブ画面への遷移を処理する */
static void action_save(void)
{
	need_save_mode = true;
}

/* メッセージボックス内のロードボタン押下を処理する */
static bool process_load_click(void)
{
#ifdef USE_DEBUGGER
	/* シングルステップか停止要求中はロードしない */
	if (dbg_is_stop_requested())
		return false;
#endif

	/* セーブロード無効時は処理しない */
	if (!is_save_load_enabled())
		return false;

	/* ロードボタンが押下されたとき */
	if (is_left_clicked && pointed_index == BTN_LOAD) {
		/* SEを再生する */
		play_se(conf_msgbox_btn_load_se);

		/* ボイスを停止する */
		set_mixer_input(VOICE_STREAM, NULL);

		/* ロード画面への遷移を処理する */
		action_load();
		return true;
	}

	return false;
}

/* ロード画面への遷移を処理する */
static void action_load(void)
{
	need_load_mode = true;
}

/* メッセージボックス内のオートボタン押下を処理する */
static bool process_auto_click(int *x, int *y, int *w, int *h)
{
	assert(!is_auto_mode());
	assert(!is_skip_mode());

	/* オートボタンが押下された場合 */
	if (is_left_clicked && pointed_index == BTN_AUTO) {
		/* SEを再生する */
		play_se(conf_msgbox_btn_auto_se);

		/* 以降のクリック処理を行わない */
		clear_input_state();

		/* オートモード開始アクションを行う */
		action_auto_start(x, y, w, h);
		return true;
	}

	return false;
}

/* メッセージボックス内のスキップボタン押下を処理する */
static bool process_skip_click(int *x, int *y, int *w, int *h)
{
	/* 未読の場合はクリックを処理しない */
	if (!is_skippable())
		return false;

	/* スキップボタンが押下された場合 */
	if (is_left_clicked && pointed_index == BTN_SKIP) {
		/* SEを再生する */
		play_se(conf_msgbox_btn_skip_se);

		/* 以降のクリック処理を行わない */
		clear_input_state();

		/* スキップモードを開始する */
		action_skip(x, y, w, h);
		return true;
	}

	return false;
}

/* スキップモードを開始する */
static void action_skip(int *x, int *y, int *w, int *h)
{
	/* スキップモードを開始する */
	start_skip_mode();

	/* スキップモードバナーを表示する */
	show_skipmode_banner(true);

	/* バナーを再描画する */
	*x = 0;
	*y = 0;
	*w = conf_window_width;
	*h = conf_window_height;
}

/*
 * メッセージボックス内のヒストリボタン押下を処理する
 * (上キー押下とマウスホイール上ボタン押下もここで処理する)
 */
static bool process_history_click(void)
{
#ifdef USE_DEBUGGER
	/* シングルステップか停止要求中はヒストリモードに遷移しない */
	if (dbg_is_stop_requested())
		return false;
#endif

	/* 上キーかヒストリボタンが押された場合 */
	if (is_up_pressed ||
	    (is_left_clicked && pointed_index == BTN_HISTORY)) {
		/* SEを再生する */
		if (is_up_pressed)
			play_se(conf_msgbox_history_se);
		else
			play_se(conf_msgbox_btn_history_se);

		/* ボイスを停止する */
		set_mixer_input(VOICE_STREAM, NULL);

		/* ヒストリ画面への遷移を処理する */
		action_history();
		return true;
	}

	return false;
}

/* ヒストリ画面への遷移を処理する */
static void action_history(void)
{
	need_history_mode = true;
}

/* メッセージボックス内のコンフィグボタン押下を処理する */
static bool process_config_click(void)
{
#ifdef USE_DEBUGGER
	/* シングルステップか停止要求中はセーブしない */
	if (dbg_is_stop_requested())
		return false;
#endif

	/* コンフィグボタンが押された場合 */
	if (is_left_clicked && pointed_index == BTN_CONFIG) {
		/* SEを再生する */
		play_se(conf_msgbox_btn_config_se);

		/* ボイスを停止する */
		set_mixer_input(VOICE_STREAM, NULL);

		/* コンフィグ画面への遷移を処理する */
		action_config();
		return true;
	}

	return false;
}

/* コンフィグ画面への遷移を処理する */
static void action_config(void)
{
	need_config_mode = true;
}

/*
 * メッセージボックス内の消すボタン押下を処理する
 * (スペースキーもここで処理する)
 */
static bool process_hide_click(int *x, int *y, int *w, int *h)
{
	/* メッセージボックスを表示中の場合 */
	if (!is_hidden) {
		/* スペースキーの押下と、消すボタンのクリックを処理する */
		if (is_space_pressed ||
		    (is_left_clicked && pointed_index == BTN_HIDE)) {
			/* SEを再生する */
			play_se(conf_msgbox_hide_se);

			/* 以降の左クリックを処理しない */
			clear_input_state();

			/* メッセージボックスを非表示にする */
			is_hidden = true;
			if (get_command_type() == COMMAND_SERIF)
				show_namebox(false);
			show_msgbox(false);
			show_click(false);

			/* 画面全体を再描画する */
			*x = 0;
			*y = 0;
			*w = conf_window_width;
			*h = conf_window_height;
			return true;
		}
		return false;
	}

	/* メッセージボックスを非表示中の場合、マウスとキーの押下を処理する */
	if(is_space_pressed || is_return_pressed || is_down_pressed ||
	   is_left_clicked || is_right_clicked) {
		/* SEを再生する */
		play_se(conf_msgbox_show_se);

		/* 以降のクリックを処理しない */
		clear_input_state();

		/* メッセージボックスを表示する */
		is_hidden = false;
		if (get_command_type() == COMMAND_SERIF)
			show_namebox(true);
		show_msgbox(true);
		show_click(is_click_visible);

		/* 画面全体を再描画する */
		*x = 0;
		*y = 0;
		*w = conf_window_width;
		*h = conf_window_height;
		return true;
	}

	return false;
}

/* フレーム描画中の右クリック押下を処理する */
static bool frame_sysmenu(int *x, int *y, int *w, int *h)
{
	bool enter_sysmenu;

#ifdef USE_DEBUGGER
	/* シングルステップか停止要求中はシステムメニューに入らない */
	if (dbg_is_stop_requested())
		return false;
#endif

	/* システムメニューを表示中の場合 */
	if (is_sysmenu) {
		/* 右クリックされた場合か、エスケープキーが押下された場合 */
		if (is_right_clicked || is_escape_pressed) {
			/* SEを再生する */
			play_se(conf_sysmenu_leave_se);

			/* システムメニューを終了する */
			is_sysmenu = false;
			is_sysmenu_finished = true;

			/* 以降のクリック処理を行わない */
			clear_input_state();
			return true;
		}

		/* ポイントされているシステムメニューのボタンを求める */
		old_sysmenu_pointed_index = sysmenu_pointed_index;
		sysmenu_pointed_index = get_sysmenu_pointed_button();

		/* ボタンのないところを左クリックされた場合 */
		if (sysmenu_pointed_index == SYSMENU_NONE &&
		    is_left_clicked) {
			/* SEを再生する */
			play_se(conf_sysmenu_leave_se);

			/* システムメニューを終了する */
			is_sysmenu = false;
			is_sysmenu_finished = true;

			/* 以降のクリック処理を行わない */
			clear_input_state();
			return true;
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
		if (!is_left_clicked)
			return false;
		clear_input_state();

		/* クイックセーブが左クリックされた場合 */
		if (sysmenu_pointed_index == SYSMENU_QSAVE) {
			/* SEを再生する */
			play_se(conf_sysmenu_qsave_se);

			/* システムメニューを終了する */
			is_sysmenu = false;
			is_sysmenu_finished = true;

			/* クイックセーブを行う */
			action_qsave();
			return true;
		}

		/* クイックロードが左クリックされた場合 */
		if (sysmenu_pointed_index == SYSMENU_QLOAD) {
			/* SEを再生する */
			play_se(conf_sysmenu_qload_se);

			/* システムメニューを終了する */
			is_sysmenu = false;
			is_sysmenu_finished = true;

			/* クイックロードを行う */
			action_qload();
			return true;
		}

		/* セーブが左クリックされた場合 */
		if (sysmenu_pointed_index == SYSMENU_SAVE) {
			/* SEを再生する */
			play_se(conf_sysmenu_save_se);

			/* システムメニューを終了する */
			is_sysmenu = false;
			is_sysmenu_finished = true;

			/* セーブ画面への移行を処理する */
			action_save();
			return true;
		}

		/* ロードが左クリックされた場合 */
		if (sysmenu_pointed_index == SYSMENU_LOAD) {
			/* SEを再生する */
			play_se(conf_sysmenu_load_se);

			/* システムメニューを終了する */
			is_sysmenu = false;
			is_sysmenu_finished = true;

			/* ロード画面への移行を処理する */
			action_load();
			return true;
		}

		/* オートが左クリックされた場合 */
		if (sysmenu_pointed_index == SYSMENU_AUTO) {
			/* SEを再生する */
			play_se(conf_sysmenu_auto_se);

			/* システムメニューを終了する */
			is_sysmenu = false;
			is_sysmenu_finished = true;

			/* オートモードを開始する */
			action_auto_start(x, y, w, h);

			/* 以降のクリック処理を行わない */
			clear_input_state();
			return true;
		}

		/* スキップが左クリックされた場合 */
		if (sysmenu_pointed_index == SYSMENU_SKIP) {
			/* SEを再生する */
			play_se(conf_sysmenu_skip_se);

			/* システムメニューを終了する */
			is_sysmenu = false;
			is_sysmenu_finished = true;

			/* スキップモードを開始する */
			action_skip(x, y, w, h);
			return true;
		}

		/* ヒストリが左クリックされた場合 */
		if (sysmenu_pointed_index == SYSMENU_HISTORY) {
			/* SEを再生する */
			play_se(conf_sysmenu_history_se);

			/* システムメニューを終了する */
			is_sysmenu = false;
			is_sysmenu_finished = true;

			/* ヒストリモードを開始する */
			action_history();
			return true;
		}

		/* コンフィグが左クリックされた場合 */
		if (sysmenu_pointed_index == SYSMENU_CONFIG) {
			/* SEを再生する */
			play_se(conf_sysmenu_config_se);

			/* システムメニューを終了する */
			is_sysmenu = false;
			is_sysmenu_finished = true;

			/* コンフィグモードを開始する */
			action_config();
			return true;
		}

		return false;
	}

	/* コンフィグでシステムメニューを表示しない場合 */
	if (conf_sysmenu_hidden)
		return false;

	/* メッセージボックス非表示中は処理しない */
	if (is_hidden)
		return false;

	/* オートモード中は処理しない */
	if (is_auto_mode())
		return false;

	/* スキップモード中は処理しない */
	if (is_skip_mode())
		return false;

	enter_sysmenu = false;

	/* 右クリックされた場合と、エスケープキーが押下された場合 */
	if (is_right_clicked || is_escape_pressed) {
		enter_sysmenu = true;
		clear_input_state();
	}

	/* 折りたたみシステムメニューがクリックされたとき */
	if (is_left_clicked && is_collapsed_sysmenu_pointed()) {
		enter_sysmenu = true;
		clear_input_state();
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
		return true;
	}

	return false;
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

/*
 * メイン描画処理
 */

/* フレーム描画を行う */
static void draw_frame(int *x, int *y, int *w, int *h)
{
	/* メッセージボックス非表示中は処理しない */
	if (is_hidden)
		return;

	/* システムメニューを表示中の場合 */
	if (is_sysmenu) {
		/* ステージを再描画するか求める(計算だけで描画しない) */
		draw_sysmenu(true, x, y, w, h);
		return;
	}

	/* 以下、メインの表示処理を行う */

	/* 入力があったらボイスを止める */
	if (is_canceled_by_skip())
		set_mixer_input(VOICE_STREAM, NULL);

	/* 本文の描画中であるか */
	if (!is_end_of_msg()) {
		/* 本文を描画する */
		draw_msgbox(x, y, w, h);
	} else {
		/*
		 * 本文の描画が完了してさらに描画しようとしているので、
		 * クリックアニメーションを描画する
		 *  - ただしシステムメニューが終了したフレームでは描画しない
		 */
		if (!is_sysmenu_finished)
			draw_click(x, y, w, h);
	}

	/* システムメニューが終了したフレームの場合 */
	if (is_sysmenu_finished) {
		/* 画面全体を再描画する */
		*x = 0;
		*y = 0;
		*w = conf_window_width;
		*h = conf_window_height;
	}

	/* オートモードとスキップモードのバナーの描画領域を取得する */
	union_banners(x, y, w, h);
}

/* メッセージ本文の描画が完了しているか */
static bool is_end_of_msg(void)
{
	/* 完了している場合 */
	if (*msg_cur == '\0')
		return true;

	/* 完了していない場合 */
	return false;
}

/* メッセージ本文の描画を完了したことにする */
static void set_end_of_msg(void)
{
	msg_cur = msg_top + strlen(msg_top);
	assert(*msg_cur == '\0');
}

/* メッセージボックスの描画を行う */
static void draw_msgbox(int *x, int *y, int *w, int *h)
{
	uint32_t wc;
	int char_count, i, mblen, glyph_width, ret_width, ret_height;

	/* 今回のフレームで描画する文字数を取得する */
	char_count = get_frame_chars();
	if (char_count == 0)
		return;

	/* 1文字ずつ描画する */
	for (i = 0; i < char_count; i++) {
		assert(*msg_cur);

		/* 途中のエスケープシーケンスを処理する */
		process_escape_sequence();

		/* ワードラッピングを処理する */
		do_word_wrapping();

		/* 描画する文字を取得する */
		mblen = utf8_to_utf32(msg_cur, &wc);
		if (mblen == -1) {
			/* 不正なUnicode: 本文の描画を終了したことにする */
			set_end_of_msg();
			return;
		}

		/* 描画する文字の幅を取得する */
		glyph_width = get_glyph_width(wc);

		/* 右側の幅が足りなければ改行する */
		process_lf(wc, glyph_width);

		/* 描画する */
		draw_char_on_msgbox(pen_x, pen_y, wc, color, outline_color,
				    &ret_width, &ret_height);

		/* 更新領域を求める */
		union_rect(x, y, w, h,
			   *x, *y, *w, *h,
			   msgbox_x + pen_x, msgbox_y + pen_y, ret_width,
			   ret_height);

		/* 次の文字へ移動する */
		msg_cur += mblen;
		pen_x += glyph_width;
		drawn_chars++;
	}

	/* 末尾のエスケープシーケンスを処理する */
	process_escape_sequence();
}

/*
 * 今回のフレームで描画する文字数を取得する
 *  - クリックやキー入力、システムGUIへの移行などすべてを加味する
 */
static int get_frame_chars(void)
{
	/* 繰り返し動作しない場合 */
	if (!is_in_command_repetition()) {
		/* すべての文字を描画する */
		return total_chars;
	}

	/* システムGUIからの復帰直後の場合 */
	if (gui_flag && is_message_active()) {
		/* 行継続の場合はすでに描画されている */
		if (is_continue_mode) {
			/* 描画を完了したことにする */
			set_end_of_msg();

			/* 描画しない */
			return 0;
		}

		/* すべての文字を描画する */
		return total_chars;
	}

	/* 重ね塗りする場合 */
	if (is_overcoating) {
		/* すべての文字を描画する */
		return total_chars;
	}

	/* セーブ・ロードのサムネイルを作成するために全文字描画する場合 */
	if (need_save_mode || need_load_mode) {
		/* 残りの文字をすべて描画する */
		return total_chars - drawn_chars;
	}

	/* スキップ処理する場合 */
	if (is_canceled_by_skip()) {
		/* 繰り返し動作を停止する */
		stop_command_repetition();

		/* 残りの文字をすべて描画する */
		return total_chars - drawn_chars;
	}

	/* 全部描画してクリック待ちに移行する場合 */
	if (is_fast_forward_by_click()) {
		/* ビープの再生を止める */
		if (is_beep)
			set_mixer_input(VOICE_STREAM, NULL);

#ifdef USE_DEBUGGER
		/*
		 * デバッガの停止ボタンが押されている場合は、
		 * クリック待ちに移行せずにコマンドを終了する流れにする
		 */
		if (dbg_is_stop_requested())
			stop_command_repetition();
#endif

		/* 残りの文字をすべて描画する */
		return total_chars - drawn_chars;
	}

	/* 経過時間を元に今回描画する文字数を計算する */
	return calc_frame_chars_by_lap();
}

/* スキップによりキャンセルされたかチェックする */
static bool is_canceled_by_skip(void)
{
	/* Translated from this code:
	if(is_skippable() && !is_non_interruptible() &&
	    (is_skip_mode() || (!is_auto_mode() && is_control_pressed)))
	*/

	/* 未読ならスキップしない */
	if (!is_skippable())
		return false;

	/* 割り込み不可ならスキップしない */
	if (is_non_interruptible())
		return false;

	/* スキップモードならスキップする */
	if (is_skip_mode())
		return true;

	/* Controlキーが押下されたらスキップする */
	if (is_control_pressed) {
		/* ただしオートモードならスキップしない */
		if (is_auto_mode())
			return false;
		else
			return true;
	}

	/* スキップしない */
	return false;
}

/* 全部描画してクリック待ちに移行する場合 */
static bool is_fast_forward_by_click(void)
{
	/* Translated from this code:
	if (!is_non_interruptible() &&
	    (is_return_pressed || is_down_pressed ||
	     (pointed_index == BTN_NONE && is_left_clicked)))
	*/

	/* 割り込み不可ならクリック待ちに移行しない */
	if (is_non_interruptible())
		return false;

	/* Returnキーが押下されたらクリック待ちに移行する */
	if (is_return_pressed)
		return true;

	/* 下キーが押下されたらクリック待ちに移行する */
	if (is_down_pressed)
		return true;

	/* クリックされたらクリック待ちに移行する */
	if (is_left_clicked) {
		/* ただしメッセージボックス内のボタンの位置なら無視する */
		if (pointed_index != BTN_NONE)
			return false;
		else
			return true;
	}

	/* クリック待ちに移行しない */
	return false;
}

/* 経過時間を元に今回描画する文字数を計算する */
static bool calc_frame_chars_by_lap(void)
{
	float lap;
	float progress;
	int char_count;

	/* 経過時間(秒)を取得する */
	lap = (float)get_stop_watch_lap(&click_sw) / 1000.0f;

	/* 進捗(文字数)を求める */
	progress = conf_msgbox_speed * lap;

	/* ユーザ設定のテキストスピードを乗算する (0にならないよう0.1足す) */
	progress *= get_text_speed() + 0.1f;

	/* 整数にする */
	char_count = (int)ceil(progress);

	/* 残りの文字数にする */
	char_count -= drawn_chars;

	/* 残りの文字数を越えた場合は、残りの文字数にする */
	if (char_count > total_chars - drawn_chars)
		char_count = total_chars - drawn_chars;

	return char_count;
}

/* 先頭のエスケープシーケンスを処理する */
static void process_escape_sequence(void)
{
	/* エスケープシーケンスが続く限り処理する */
	while (*msg_cur == '\\') {
		if (*(msg_cur + 1) == 'n') {
			/* 改行 */
			process_escape_sequence_lf();
		} else {
			/*
			 * 不正なエスケープシーケンス
			 *  - 読み飛ばさない
			 */
			break;
		}
	}
}

/* 改行("\\n")を処理する */
static void process_escape_sequence_lf(void)
{
	pen_y += conf_msgbox_margin_line;
	pen_x = conf_msgbox_margin_left;
	msg_cur += 2;
}

/* ワードラッピングを処理する */
static void do_word_wrapping(void)
{
	if (is_after_space) {
		if (pen_x + get_en_word_width() >=
		    msgbox_w - conf_msgbox_margin_right) {
			pen_y += conf_msgbox_margin_line;
			pen_x = conf_msgbox_margin_left;
		}
	}

	is_after_space = *msg_cur == ' ';
}

/* msgが英単語の先頭であれば、その単語の描画幅、それ以外の場合0を返す */
static int get_en_word_width(void)
{
	const char *m;
	uint32_t wc;
	int width;

	m = msg_cur;
	width = 0;
	while (isgraph_extended(&m, &wc))
		width += get_glyph_width(wc);

	return width;
}

/* 右側の幅が足りなければ改行する */
static void process_lf(uint32_t c, int glyph_width)
{
	/* 右側の幅が足りる場合、改行しない */
	if (pen_x + glyph_width < msgbox_w - conf_msgbox_margin_right)
		return;

	/* 禁則文字の場合、改行しない */
	if (c == ' ' || c == ',' || c == '.' || c == ':' || c == ';' ||
	    c == CHAR_TOUTEN || c == CHAR_KUTEN)
		return;

	/* 改行する */
	pen_y += conf_msgbox_margin_line;
	pen_x = conf_msgbox_margin_left;
}

/* クリックアニメーションを描画する */
static void draw_click(int *x, int *y, int *w, int *h)
{
	int click_x, click_y, click_w, click_h;
	int lap, index;

	/* 入力があったら繰り返しを終了する */
	check_stop_click_animation();

	/* クリックアニメーションの初回表示のとき */
	if (is_click_first) {
		is_click_first = false;

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
	if (!is_click_first && dbg_is_stop_requested()) {
		if (!have_voice &&
		    gui_flag &&
		    (is_return_pressed || is_down_pressed ||
		     (pointed_index == BTN_NONE && is_left_clicked)))
			stop_command_repetition();
		else if (!have_voice)
			stop_command_repetition();
		else if (have_voice &&
			 (is_mixer_sound_finished(VOICE_STREAM) || gui_flag))
			stop_command_repetition();
		else if (have_voice &&
			 (is_left_clicked || is_down_pressed ||
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
		   !is_click_first &&
		   (is_return_pressed || is_down_pressed ||
		    (pointed_index == BTN_NONE && is_left_clicked))) {
		stop_command_repetition();
	} else if (!is_click_first &&
		   (is_return_pressed || is_down_pressed ||
		    (pointed_index == BTN_NONE && is_left_clicked))) {
		if (!is_non_interruptible()) {
			stop_command_repetition();
		} else {
			if (!have_voice || is_mixer_sound_finished(VOICE_STREAM))
				stop_command_repetition();
		}
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

	if (is_auto_mode() || is_skip_mode())
		return false;

	get_collapsed_sysmenu_rect(&bx, &by, &bw, &bh);
	if (mouse_pos_x >= bx && mouse_pos_x < bx + bw &&
	    mouse_pos_y >= by && mouse_pos_y < by + bh)
		return true;

	return false;
}

/* バナーの描画領域を取得する */
static void union_banners(int *x, int *y, int *w, int *h)
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

/*
 * その他
 */

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

/*
 * 終了処理
 */

/* 終了処理を行う */
static bool cleanup(int *x, int *y, int *w, int *h)
{
	/* PCMストリームの再生を終了する */
	if (!conf_voice_stop_off)
		set_mixer_input(VOICE_STREAM, NULL);

	/* クリックアニメーションを非表示にする */
	show_click(false);

	/* 表示中のメッセージをなしとする */
	if (!need_save_mode && !need_load_mode && !need_history_mode &&
	    !need_config_mode)
		clear_message_active();

	/* 既読にする */
	set_seen();

	/* 重ね塗りをする場合 */
	if (conf_msgbox_dim &&
	    (!did_quick_load && !need_save_mode && !need_load_mode &&
	     !need_history_mode && !need_config_mode)) {
		is_overcoating = true;
		msg_cur = msg_top;
		drawn_chars = 0;
		pen_x = orig_pen_x;
		pen_y = orig_pen_y;
		color = make_pixel_slow(0xff,
					(uint32_t)conf_msgbox_dim_color_r,
					(uint32_t)conf_msgbox_dim_color_g,
					(uint32_t)conf_msgbox_dim_color_b);
		outline_color = make_pixel_slow(0xff,
						(uint32_t)conf_msgbox_dim_color_outline_r,
						(uint32_t)conf_msgbox_dim_color_outline_g,
						(uint32_t)conf_msgbox_dim_color_outline_b);
		draw_msgbox(x, y, w, h);
	}

	/* メッセージを解放する */
	if (msg_top != NULL) {
		free(msg_top);
		msg_top = NULL;
	}

	/* 次のコマンドに移動する */
	if (!did_quick_load && !need_save_mode && !need_load_mode &&
	    !need_history_mode && !need_config_mode) {
		if (!move_to_next_command())
			return false;
	}

	return true;
}
