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
 *  - 2023/08/14 文字色、サイズの変更を実装
 *  - 2023/08/19 インラインウェイトの実装
 *  - 2023/08/19 ペン移動の実装
 */

/*
 * [Notice (注意)]
 *  - Decomposed Unicode characters are not supported, use precomposed ones.
 *    (Unicodeの合成はサポートしないので、合成済みの文字列を使用してください)
 *
 * [Memo]
 *  - メッセージボックス内のボタンと、システムメニュー内のボタンがある
 *    - これらが呼び出す機能は同じ
 *    - メッセージボックスを隠すボタンは、メッセージボックス内のみにある
 *  - システムメニュー非表示時には、折りたたみシステムメニューが表示される
 *  - システムGUIから戻った場合は、すでに描画されているメッセージレイヤを
 *    そのまま流用する (gui_sys_flag)
 *
 * [TODO]
 *  - システムメニュー処理はcmd_switch.cと重複するので、sysmenu.cに分離する
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
#define MAX_BTN			(8)

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
#define SYSMENU_CUSTOM1		(8)
#define SYSMENU_CUSTOM2		(9)
#define SYSMENU_COUNT		(10)

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
 *  - メッセージボックス内の座標
 *  - 他のコマンドに移ったり、GUIから戻ってきた場合も、保持される
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

/* ルビ用のペン位置 */
static int pen_ruby_x;
static int pen_ruby_y;

/* 名前ボックスの描画中であるか */
static bool is_drawing_namebox;

/*
 * 色
 */

/* 本文の文字の色 */
static pixel_t body_color;

/* 本文の文字の縁取りの色 */
static pixel_t body_outline_color;

/* 名前の文字の色 */
static pixel_t name_color;

/* 名前の文字の縁取りの色 */
static pixel_t name_outline_color;

/*
 * 行継続モード
 */

/* 行継続モードであるか */
static bool is_continue_mode;

/* 重ね塗りしているか (将来、色のエスケープシーケンスを無視するため) */
static bool is_dimming;

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
 * インラインウェイト
 */

/* インラインウェイト中であるか */
static bool is_inline_wait;

/* インラインウェイトの待ち時間 */
static float inline_wait_time;

/* インラインウェイトで待ったトータルの時間 */
static float inline_wait_time_total;

/* インラインウェイトの経過時刻を表すストップウォッチ */
static stop_watch_t inline_sw;


/*
 * コマンドが開始されたときの状態 
 */

/* ロードによって開始されたか */
static bool load_flag;

/* GUIコマンド終了後の最初のコマンドか */
static bool gui_cmd_flag;

/* システムGUIから復帰したか */
static bool gui_sys_flag;

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
 *  - TODO: cmd_switch.cと共通化する
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
 * システム遷移フラグ
 *  - TODO: cmd_switch.cと共通化する
 */

/* クイックセーブを行うか */
static bool will_quick_save;

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

/* クイックロードに失敗したか */
static bool is_quick_load_failed;

/* カスタムルーチンに遷移するか */
static bool need_custom_gosub;

/* カスタムルーチン */
static const char *custom_gosub_label;

/*
 * 前方参照
 */

/* 主な処理 */
static bool preprocess(int *x, int *y, int *w, int *h);
static void main_process(int *x, int *y, int *w, int *h);
static bool postprocess(int *x, int *y, int *w, int *h);

/* 初期化 */
static bool init(int *x, int *y, int *w, int *h);
static void init_flags_and_vars(void);
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
static bool search_for_end_of_escape_sequence(const char **msg);
static void init_font_color(void);
static bool init_serif(int *x, int *y, int *w, int *h);
static bool check_play_voice(void);
static bool play_voice(void);
static void set_character_volume_by_name(const char *name);
static void draw_namebox(void);
static int get_namebox_width(void);
static void focus_character(void);
static void init_font_size(void);
static void init_pen(void);
static void init_msgbox(int *x, int *y, int *w, int *h);
static void init_click(void);
static void init_first_draw_area(int *x, int *y, int *w, int *h);
static void init_pointed_index(void);
static int get_pointed_button(void);
static void adjust_pointed_index(void);
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
static bool process_hide_click(int *x, int *y, int *w, int *h);
static void action_hide_start(int *x, int *y, int *w, int *h);
static void action_hide_end(int *x, int *y, int *w, int *h);
static void action_qsave(void);
static void action_qload(void);
static void action_save(void);
static void action_load(void);
static void action_skip(int *x, int *y, int *w, int *h);
static void action_history(void);
static void action_config(void);
static void action_custom(int index);
static bool frame_sysmenu(int *x, int *y, int *w, int *h);
static bool process_collapsed_sysmenu(int *x, int *y, int *w, int *h);
static void adjust_sysmenu_pointed_index(void);
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
static int calc_frame_chars_by_lap(void);
static void process_escape_sequence(const char **s);
static void process_escape_sequence_lf(const char **s);
static bool process_escape_sequence_color(const char **s);
static bool process_escape_sequence_size(const char **s);
static bool process_escape_sequence_wait(const char **s);
static bool process_escape_sequence_pen(const char **s);
static bool process_escape_sequence_ruby(const char **s);
static void do_word_wrapping(void);
static uint32_t convert_tategaki_char(uint32_t wc);
static bool is_tategaki_punctuation(uint32_t wc);
static int get_en_word_width(void);
static void process_lf(uint32_t c, int glyph_width, int glyph_height);
static bool is_small_kana(uint32_t wc);
static void draw_click(int *x, int *y, int *w, int *h);
static bool check_stop_click_animation(void);
static void draw_sysmenu(bool calc_only, int *x, int *y, int *w, int *h);
static void draw_collapsed_sysmenu(int *x, int *y, int *w, int *h);
static bool is_collapsed_sysmenu_pointed(void);
static void union_banners(int *x, int *y, int *w, int *h);
static void draw_dimming(int *x, int *y, int *w, int *h);

/* その他 */
static void play_se(const char *file);
static bool is_skippable(void);

/* 終了処理 */
static bool cleanup(void);

/*
 * メッセージ・セリフコマンド
 */
bool message_command(int *x, int *y, int *w, int *h, bool *cont)
{
	/* 初期化処理を行う */
	if (!is_in_command_repetition())
		if (!init(x, y, w, h))
			return false;

	/*
	 * フレームの前処理を行う
	 *  - オートモード時の時間経過に伴う処理
	 *  - メッセージボックス内のボタンへの入力の処理
	 *  - システムメニューへの入力の処理
	 */
	if (!preprocess(x, y, w, h))
		return false;

	/*
	 * アップデートを行う
	 *  - メッセージボックスレイヤに対する文字描画 (時間経過に伴う)
	 *  - 文字描画完了後のクリックアニメーションの制御
	 *  - 次のコマンドに進むためのクリックの処理
	 */
	main_process(x, y, w, h);

	/*
	 * フレームの後処理を行う
	 *  - ステージの描画を行う
	 *  - システムGUIの開始を行う
	 *  - 重ね塗り(dimming)の描画を行う
	 */
	if (!postprocess(x, y, w, h))
		return false;

	/* 終了処理を行う */
	if (!is_in_command_repetition())
		if (!cleanup())
			return false;

	/*
	 * クイックロードされた場合は、このフレームの描画を継続する
	 *  - でないとステージ描画されないのにフリップされるフレームが生じる
	 */
	*cont = did_quick_load;

	/* 次のフレームへ */
	return true;
}

/* 前処理を行う */
static bool preprocess(int *x, int *y, int *w, int *h)
{
	/* インラインウェイトのキャンセルを処理する */
	if (is_inline_wait) {
		if (is_left_button_pressed || is_right_button_pressed ||
		    is_control_pressed || is_space_pressed ||
		    is_return_pressed || is_up_pressed || is_down_pressed ||
		    is_page_up_pressed || is_page_down_pressed ||
		    is_escape_pressed)
			is_inline_wait = false;
	}

	/* オートモードを処理する */
	if (frame_auto_mode(x, y, w, h)) {
		/* 入力がキャプチャされたので、ここでリターンする */
		return true;
	}

	/* メッセージボックス内のボタンを処理する */
	if (frame_buttons(x, y, w, h)) {
		/* ロードに失敗した場合 */
		if (is_quick_load_failed) {
			/* 継続不能 */
			return false;
		}

		/* 入力がキャプチャされたので、ここでリターンする */
		return true;
	}

	/* システムメニューを処理する */
	if (frame_sysmenu(x, y, w, h)) {
		/* ロードに失敗した場合 */
		if (is_quick_load_failed) {
			/* 継続不能 */
			return false;
		}

		/* 入力がキャプチャされたので、ここでリターンする */
		return true;
	}

	/* 入力はキャプチャされなかったが、エラーはない */
	return true;
}

/* メッセージレイヤ描画と、クリック表示のアップデートを行う */
static void main_process(int *x, int *y, int *w, int *h)
{
	/* クイックロードされた場合は描画しない */
	if (did_quick_load) {
		/* 繰り返しを停止する */
		stop_command_repetition();

		/* 文字の描画を行わない */
		return;
	}

	/*
	 * 文字の描画/クリックアニメーションの制御を行う
	 *  - システムGUIに遷移する場合でもサムネイル描画のため処理する
	 */
	draw_frame(x, y, w, h);

	/* システムGUIへ遷移する場合 */
	if (need_save_mode || need_load_mode || need_history_mode ||
	    need_config_mode || need_custom_gosub) {
		/* 繰り返しを停止する */
		stop_command_repetition();
	}
}

/* 後処理を行う */
static bool postprocess(int *x, int *y, int *w, int *h)
{
	/*
	 * クイックロードされた場合は描画を行わない
	 *  - 同じフレームで、ロード後のコマンドが実行されるため
	 */
	if (did_quick_load)
		return true;

	/*
	 * ロードされて最初のフレームの場合、画面全体を描画する
	 *  - これはGPUを使わない場合の最適化で、GPUを使う場合は関係ない
	 */
	if (load_flag) {
		union_rect(x, y, w, h, *w, *y, *w, *h, 0, 0, conf_window_width,
			   conf_window_height);
		load_flag = false;
	}

	/*
	 * ステージの更新領域(x, y) (w, h)を描画する
	 *  - アニメ中、またはGPU利用時は、画面全体の描画になる
	 *  - アニメのアップデート処理はこの内部で行われる
	 */
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

	/* システムメニューを表示開始したフレームのフラグをクリアする */
	if (is_sysmenu_finished)
		is_sysmenu_finished = false;

	/* クイックセーブされる場合を処理する */
	if (will_quick_save) {
		/* クイックセーブを行う */
		quick_save();
		will_quick_save = false;
		return true;
	}

	/* システムGUIへの遷移を処理する */
	if (need_save_mode) {
		if (!prepare_gui_mode(SAVE_GUI_FILE, true, true, false))
			return false;
		start_gui_mode();
		return true;
	}
	if (need_load_mode) {
		if (!prepare_gui_mode(LOAD_GUI_FILE, true, true, false))
			return false;
		start_gui_mode();
		return true;
	}
	if (need_history_mode) {
		if (!prepare_gui_mode(HISTORY_GUI_FILE, true, true, false))
			return false;
		start_gui_mode();
		return true;
	}
	if (need_config_mode) {
		if (!prepare_gui_mode(CONFIG_GUI_FILE, true, true, false))
			return false;
		start_gui_mode();
		return true;
	}

	/*
	 * 重ね塗り(dimming)をする場合
	 *  - コマンドを終了するときに描画する
	 *  - ステージ描画後にメッセージレイヤを更新している
	 *  - つまり現在のフレームの見た目には影響しない
	 *  - 次のメッセージ/セリフの表示開始時に初めて見える
	 */
	if (conf_msgbox_dim && !is_in_command_repetition())
		draw_dimming(x, y, w, h);

	return true;
}

/*
 * 初期化
 */

/* 初期化処理を行う */
static bool init(int *x, int *y, int *w, int *h)
{
	/* フラグを初期化する */
	init_flags_and_vars();

	/* オートモードの場合の初期化を行う */
	init_auto_mode();

	/* スキップモードの場合の初期化を行う */
	init_skip_mode();

	/* 名前を取得する */
	if (!init_name_top())
		return false;

	/* 文字色の初期化を行う */
	init_font_color();

	/* メッセージを取得する */
	if (!init_msg_top())
		return false;

	/* セリフ固有の初期化を行う */
	if (!init_serif(x, y, w, h))
		return false;

	/*
	 * メッセージの表示中状態をセットする
	 *  - システムGUIに入っても保持される
	 *  - メッセージから次のコマンドに移行するときにクリアされる
	 *  - ロードされてもクリアされる
	 *  - タイトルに戻るときにもクリアされる
	 */
	if (!gui_sys_flag)
		set_message_active();

	/* メッセージボックスを初期化する */
	init_msgbox(x, y, w, h);

	/* フォントのサイズを初期化する */
	init_font_size();

	/* ペンの位置を初期化する */
	init_pen();

	/* クリックアニメーションを非表示の状態にする */
	init_click();

	/* 初回に描画する矩形を求める */
	init_first_draw_area(x, y, w, h);

	/* ボタンの選択状態を初期化する */
	init_pointed_index();

	/* 繰り返し動作を設定する */
	init_repetition();

	return true;
}

/* フラグを初期化する */
static void init_flags_and_vars(void)
{
	/* GUIから戻ったばかりかチェックする */
	if (check_gui_flag()) {
		if (is_message_active()) {
			/*
			 * メッセージがアクティブ状態のままシステムGUIに
			 * 移行して、その後戻ってきた場合
			 *  - 瞬間表示を行うためにフラグをセットする
			 */
			gui_sys_flag = true;
			gui_cmd_flag = false;
		} else {
			/*
			 * GUIコマンドの直後の場合
			 *  - 画面全体の更新を行うためにフラグをセットする
			 */
			gui_sys_flag = false;
			gui_cmd_flag = true;
		}
	} else {
		gui_sys_flag = false;
		gui_cmd_flag = false;
	}

	/* ロードされたばかりかチェックする */
	load_flag = check_load_flag();

	/* スペースキーによる非表示でない状態にする */
	is_hidden = false;

	/* オートモードの状態設定を行う */
	is_auto_mode_wait = false;

	/* セーブ・ロード・ヒストリ・コンフィグの状態設定を行う */
	did_quick_load = false;
	will_quick_save = false;
	need_save_mode = false;
	need_load_mode = false;
	need_history_mode = false;
	need_config_mode = false;
	need_custom_gosub = false;
	is_quick_load_failed = false;

	/* システムメニューの状態設定を行う */
	is_sysmenu = false;
	is_sysmenu_finished = false;
	is_collapsed_sysmenu_pointed_prev = false;

	/* 重ね塗り(dimming)でない状態にする */
	is_dimming = false;

	/* インラインウェイトでない状態にする */
	is_inline_wait = false;
	inline_wait_time_total = 0;
}

/* オートモードの場合の初期化処理を行う */
static void init_auto_mode(void)
{
	/* システムGUIから戻った場合は処理しない */
	if (gui_sys_flag)
		return;

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
	/* システムGUIから戻った場合は処理しない */
	if (gui_sys_flag)
		return;

	/* スキップモードの場合 */
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

	/* システムGUIから戻った場合 */
	if (gui_sys_flag)
		return true;

	/* ロード/タイトルへ戻った場合はname_topが残っているので解放する */
	if (name_top != NULL) {
		free(name_top);
		name_top = NULL;
	}

	/* 名前を取得する */
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

	/* システムGUIから戻った場合 */
	if (gui_sys_flag)
		return true;

	/* ロード/タイトルへ戻った場合はmsg_topが残っているので解放する */
	if (msg_top != NULL) {
		free(msg_top);
		msg_top = NULL;
	}

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
		if (conf_namebox_hidden || conf_msgbox_tategaki) {
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

	/* ロードで開始された場合は、行継続モードを解除する */
	if (load_flag && is_continue_mode)
		is_continue_mode = false;

	return true;
}

/* エスケープ文字かチェックする */
static bool is_escape_sequence_char(char c)
{
	switch (c) {
	case 'n': /* 改行 */
	case '#': /* 文字色 */
	case '@': /* 文字サイズ */
	case 'w': /* インラインウェイト */
	case 'p': /* ペン移動 */
		return true;
	default:
		break;
	}

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

			/* ロードで開始されたときは、先頭の改行を行わない */
			if (load_flag)
				continue;

			/* ペンを改行する */
			if (!conf_msgbox_tategaki) {
				pen_x = conf_msgbox_margin_left;
				pen_y += conf_msgbox_margin_line;
			} else {
				pen_x -= conf_msgbox_margin_line;
				pen_y = conf_msgbox_margin_top;
			}
		} else {
			m++;
		}
	}
	return m;
}

/* 空白文字の分だけカーソルを移動する */
static void put_space(void)
{
	int cw, ch;

	if (!conf_msgbox_tategaki) {
		cw = get_glyph_width(' ');
		if (pen_x + cw >= msgbox_w - conf_msgbox_margin_right) {
			pen_y += conf_msgbox_margin_line;
			pen_x = conf_msgbox_margin_left;
		} else {
			pen_x += cw;
		}
	} else {
		ch = get_glyph_height(' ');
		if (pen_y + ch >= msgbox_h - conf_msgbox_margin_bottom) {
			pen_y = conf_msgbox_margin_top;
			pen_x -= conf_msgbox_margin_line;
		} else {
			pen_y += ch;
		}
	}
}

/* ヒストリ画面用にメッセージ履歴を登録する */
static bool register_message_for_history(const char *msg)
{
	const char *voice;

	assert(msg != NULL);
	assert(!gui_sys_flag);

	/* メッセージ履歴を登録する */
	if (get_command_type() == COMMAND_SERIF) {
		assert(name_top != NULL);

		/* ボイスファイルを取得する */
		voice = get_string_param(SERIF_PARAM_VOICE);

		/* ビープ音は履歴画面で再生しない */
		if (voice[0] == '@')
			voice = NULL;

		/* セリフをヒストリ画面用に登録する */
		if (!register_message(name_top,
				      msg,
				      voice,
				      body_color,
				      body_outline_color,
				      name_color,
				      name_outline_color))
			return false;
	} else {
		/* メッセージをヒストリ画面用に登録する */
		if (!register_message(NULL,
				      msg,
				      NULL,
				      body_color,
				      body_outline_color,
				      0,
				      0))
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
		if (!conf_msgbox_tategaki) {
			prefix = U8("「");
			suffix = U8("」");
		} else {
			prefix = U8("﹁");
			suffix = U8("﹂");
		}
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
			switch (*(msg + 1)) {
			case 'n':	/* 改行 */
				msg += 2;
				break;
			case '#':	/* 色指定 */
			case '@':	/* サイズ指定 */
			case 'w':	/* インラインウェイト */
			case 'p':	/* ペン移動 */
			case '^':	/* ルビ */
				if (!search_for_end_of_escape_sequence(&msg))
					return count;
				break;
			default:
				/*
				 * 不正なエスケープシーケンス
				 *  - 読み飛ばさない
				 */
				return count;
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

/* エスケープシーケンスの終了位置を探す */
static bool search_for_end_of_escape_sequence(const char **msg)
{
	const char *s;
	int len;

	s = *msg;
	len = 0;
	while (*s != '\0' && *s != '}') {
		s++;
		len++;
	}
	if (*s == '\0')
		return false;

	*msg += len + 1;
	return true;
}

/* 文字色を求める */
static void init_font_color(void)
{
	int i;

	/* システムGUIから戻った場合 */
	if (gui_sys_flag)
		return;

	/* 色は、まずデフォルトの色をロードする */
	body_color = make_pixel_slow(0xff,
				     (pixel_t)conf_font_color_r,
				     (pixel_t)conf_font_color_g,
				     (pixel_t)conf_font_color_b);
	body_outline_color =
		make_pixel_slow(0xff,
				(pixel_t)conf_font_outline_color_r,
				(pixel_t)conf_font_outline_color_g,
				(pixel_t)conf_font_outline_color_b);
	name_color = body_color;
	name_outline_color = body_outline_color;

	/* セリフの場合は、名前リストにあれば色を変更する */
	if (get_command_type() == COMMAND_SERIF) {
		/* コンフィグでnameの指す名前が指定されているか */
		for (i = 0; i < SERIF_COLOR_COUNT; i++) {
			if (conf_serif_color_name[i] == NULL)
				continue;
			if (strcmp(name_top, conf_serif_color_name[i]) == 0) {
				/* コンフィグで指定された色にする */
				name_color = make_pixel_slow(
					0xff,
					(uint32_t)conf_serif_color_r[i],
					(uint32_t)conf_serif_color_g[i],
					(uint32_t)conf_serif_color_b[i]);
				name_outline_color = make_pixel_slow(
					0xff,
					(uint32_t)conf_serif_outline_color_r[i],
					(uint32_t)conf_serif_outline_color_g[i],
					(uint32_t)conf_serif_outline_color_b[i]);
				if (!conf_serif_color_name_only) {
					body_color = name_color;
					body_outline_color = name_outline_color;
				}
				return;
			}
		}
	}
}

/* セリフコマンドを処理する */
static bool init_serif(int *x, int *y, int *w, int *h)
{
	int namebox_x, namebox_y, namebox_w, namebox_h;

	/* システムGUIから戻った場合 */
	if (gui_sys_flag)
		return true;

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
	if (!conf_namebox_hidden && !conf_msgbox_tategaki)
		draw_namebox();

	/* 名前ボックスを表示する */
	show_namebox(true);

	/* キャラクタのフォーカスが有効なとき */
	if (conf_character_focus) {
		/* フォーカスを設定する */
		focus_character();

		/* 再描画する */
		*x = 0;
		*y = 0;
		*w = conf_window_width;
		*h = conf_window_height;
	}

	return true;
}

/* ボイスを再生するかを判断する */
static bool check_play_voice(void)
{
	/* システムGUIから戻った場合は再生しない */
	if (gui_sys_flag)
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
		times = (int)((float)count_chars(msg_cur) /
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
	uint32_t wc;
	int char_count, mblen, i, ret_width, ret_height;
	const char *name;

	/* フォントを設定する */
	select_font(conf_namebox_font_select);
	set_font_size(conf_namebox_font_size > 0 ?
		      conf_namebox_font_size : conf_font_size);
	switch (conf_namebox_font_outline) {
	case 0: set_font_outline(!conf_font_outline_remove); break;
	case 1: set_font_outline(true); break;
	case 2: set_font_outline(false); break;
	default: break;
	}

	/* 名前の文字列を取得する */
	name = name_top;

	/* 名前の文字数を取得する */
	char_count = count_chars(name);
	if (char_count == 0)
		return;

	/* 描画位置を決める */
	if (!conf_namebox_centering_no)
		pen_x = (get_namebox_width() - get_utf8_width(name)) / 2;
	else
		pen_x = conf_namebox_margin_left;
	pen_y = conf_namebox_margin_top;
	pen_ruby_y = pen_y - conf_font_ruby_size;

	/* 名前ボックスをクリアする */
	clear_namebox();

	/* 1文字ずつ描画する */
	is_drawing_namebox = true;
	for (i = 0; i < char_count; i++) {
		assert(*name != '\0');

		/* 途中のエスケープシーケンスを処理する */
		process_escape_sequence(&name);

		/* 描画する文字を取得する */
		mblen = utf8_to_utf32(name, &wc);
		if (mblen == -1)
			return;

		/* 描画する */
		draw_char_on_namebox(pen_x, pen_y, wc, name_color,
				     name_outline_color, &ret_width,
				     &ret_height, conf_font_size, false);

		/* ペン位置を更新する */
		pen_ruby_x = pen_x;
		pen_x += ret_width;

		/* 次の文字へ移動する */
		name += mblen;
	}

	/* 末尾のエスケープシーケンスを処理する */
	process_escape_sequence(&name);

	is_drawing_namebox = false;
}

/* 名前ボックスの幅を取得する */
static int get_namebox_width(void)
{
	int x, y, w, h;

	get_namebox_rect(&x, &y, &w, &h);

	return w;
}

/* キャラクタのフォーカスを行う */
static void focus_character(void)
{
	const char *fname;
	int i, j;

	/* 名前が登録されているキャラクタであるかチェックする */
	for (i = 0; i < CHARACTER_MAP_COUNT; i++) {
		if (conf_character_name[i] == NULL)
			continue;
		if (conf_character_file[i] == NULL)
			continue;
		if (strcmp(conf_character_name[i], name_top) == 0)
			break;
	}
	if (i == CHARACTER_MAP_COUNT) {
		/* 名前が登録されていなかったときは全キャラを暗くしない */
		for (j = 0; j < CH_BASIC_LAYERS; j++)
			set_ch_dim(j, false);
		return;
	}

	/* すべてのキャラクタについて設定する */
	for (j = 0; j < CH_BASIC_LAYERS; j++) {
		/* キャラがロードされていない位置なら飛ばす */
		fname = get_ch_file_name(j);
		if (fname == NULL)
			continue;

		/* キャラのファイル名がマッチするか調べる */
		if (strncmp(fname, conf_character_file[i],
			    strlen(conf_character_file[i])) == 0) {
			/* マッチしたので暗くしない */
			set_ch_dim(j, false);
		} else {
			/* マッチしなかったので暗くする */
			set_ch_dim(j, true);
		}
	}
}

/* フォントサイズを初期化する */
static void init_font_size(void)
{
	select_font(conf_font_select);
	set_font_size(conf_font_size);
	set_font_outline(!conf_font_outline_remove);
}

/* ペンの位置を初期化する */
static void init_pen(void)
{
	/* システムGUIから戻った場合 */
	if (gui_sys_flag)
		return;

	/* 継続行でなければ、メッセージの描画位置を初期化する */
	if (!is_continue_mode) {
		if (!conf_msgbox_tategaki) {
			pen_x = conf_msgbox_margin_left;
			pen_y = conf_msgbox_margin_top;
		} else {
			pen_x = msgbox_w - conf_msgbox_margin_right -
				conf_font_size;
			pen_y = conf_msgbox_margin_top;
		}
	}

	/* 重ね塗りをする場合 */
	if (conf_msgbox_dim) {
		/* 描画開始位置を保存する */
		orig_pen_x = pen_x;
		orig_pen_y = pen_y;
	}
}

/* メッセージボックスを初期化する */
static void init_msgbox(int *x, int *y, int *w, int *h)
{
	/* システムGUIから戻った場合 */
	if (gui_sys_flag)
		return;

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

/* 初期化処理において、初回に更新する矩形を求める */
static void init_first_draw_area(int *x, int *y, int *w, int *h)
{
	/*
	 * ここで求めた更新矩形は、GPUを利用しないときに適用される
	 *  - GPUを利用するときは、毎フレーム画面全体を更新する
	 *  - See also draw_stage() in stage.c
	 */

	/* deprecatedなメニュー系コマンドが終了した直後の場合 */
	if (check_menu_finish_flag() || check_retrospect_finish_flag()) {
		/* 画面全体を更新する */
		*x = 0;
		*y = 0;
		*w = conf_window_width;
		*h = conf_window_height;
		return;
	}

	/* GUIが終了した直後の場合 */
	if (gui_cmd_flag || gui_sys_flag) {
		/* 画面全体を更新する */
		*x = 0;
		*y = 0;
		*w = conf_window_width;
		*h = conf_window_height;
		return;
	}

	/* それ以外の場合、メッセージボックスの領域を更新する */
	union_rect(x, y, w, h,
		   *x, *y, *w, *h,
		   msgbox_x, msgbox_y, msgbox_w, msgbox_h);
}

/* 初期化処理においてポイントされているボタンを求め描画する */
static void init_pointed_index(void)
{
	int i, btn_x, btn_y, btn_w, btn_h;

	/* ポイントされているボタンを求める */
	pointed_index = get_pointed_button();
	adjust_pointed_index();

	/* ボタンがポイントされていなければ、すべてのボタンをbgで塗りつぶす */
	if (pointed_index == BTN_NONE) {
		for (i = 0; i <= MAX_BTN; i++) {
			get_button_rect(i, &btn_x, &btn_y, &btn_w, &btn_h);
			clear_msgbox_rect_with_bg(btn_x, btn_y, btn_w, btn_h);
		}
		return;
	}

	/* ポイントされているボタンのfgを描画する */
	get_button_rect(pointed_index, &btn_x, &btn_y, &btn_w, &btn_h);
	clear_msgbox_rect_with_fg(btn_x, btn_y, btn_w, btn_h);
}

/* 選択中のボタンを取得する */
static int get_pointed_button(void)
{
	int rx, ry, btn_x, btn_y, btn_w, btn_h, i;

#ifdef USE_DEBUGGER
	/* シングルステップか停止要求中の場合、ボタンを選択できなくする */
	if (dbg_is_stop_requested())
		return BTN_NONE;
#endif

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

/* 状況に応じてメッセージボックス内のボタンのポイントを無効化する */
static void adjust_pointed_index(void)
{
	/* システムメニューを表示中の間はボタンを選択しない */
	if (is_sysmenu) {
		pointed_index = BTN_NONE;
		return;
	}

	/* メッセージボックスを隠している間はボタンを選択しない */
	if (is_hidden) {
		pointed_index = BTN_NONE;
		return;
	}

	/* オートモード中とスキップモード中はボタンを選択しない */
	if (is_auto_mode() || is_skip_mode()) {
		pointed_index = BTN_NONE;
		return;
	}

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

#ifdef USE_DEBUGGER
	/* オートモード中に停止要求があった場合 */
	if (dbg_is_stop_requested()) {
		/* オートモード終了アクションを処理する */
		action_auto_end(x, y, w, h);

		/* クリックされたものとして処理する */
		return true;
	}
#endif

	/* クリックされた場合 */
	if (is_left_clicked || is_right_clicked || is_escape_pressed ||
	    is_return_pressed || is_down_pressed) {
		/* SEを再生する */
		play_se(conf_msgbox_auto_cancel_se);

		/* 以降のクリック処理を行わない */
		clear_input_state();

		/* オートモード終了アクションを処理する */
		action_auto_end(x, y, w, h);

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
	 * システムGUIから戻ったコマンドである場合
	 *  - すでに表示完了している
	 */
	if (gui_sys_flag)
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

/* メッセージボックス内のボタンを処理する (非表示中も呼ばれる) */
static bool frame_buttons(int *x, int *y, int *w, int *h)
{
	/* ボタンを描画する */
	if (draw_buttons(x, y, w, h)) {
		/* アクティブなボタンが変わったのでSEを再生する */
		play_se(conf_msgbox_btn_change_se);
	}

#ifdef USE_DEBUGGER
	/* シングルステップか停止要求中はボタンのクリックを処理しない */
	if (dbg_is_stop_requested())
		return false;
#endif

	/* システムメニューを表示中の場合、クリックを処理しない */
	if (is_sysmenu)
		return false;

	/* オートモードの場合、クリックを処理しない */
	if (is_auto_mode())
		return false;

	/* ボタンのクリックを処理する (非表示のキャンセルもここで処理する) */
	if (process_button_click(x, y, w, h))
		return true;	/* クリックされた */

	/* クリックされなかった */
	return false;
}

/* メッセージボックス内のボタンを描画する */
static bool draw_buttons(int *x, int *y, int *w, int *h)
{
	int last_pointed_index, btn_x, btn_y, btn_w, btn_h;

	/* メッセージボックス非表示中は処理しない */
	if (is_hidden)
		return false;

	/* ポイントされているボタンを取得する */
	last_pointed_index = pointed_index;
	pointed_index = get_pointed_button();
	adjust_pointed_index();

	/* ポイント状態に変更がない場合 */
	if (pointed_index == last_pointed_index)
		return false;

	/* 非アクティブになるボタンのbgを描画する */
	if (last_pointed_index != BTN_NONE) {
		get_button_rect(last_pointed_index, &btn_x, &btn_y, &btn_w,
				&btn_h);
		clear_msgbox_rect_with_bg(btn_x, btn_y, btn_w, btn_h);
		union_rect(x, y, w, h,
			   *x, *y, *w, *h,
			   btn_x + conf_msgbox_x, btn_y + conf_msgbox_y,
			   btn_w, btn_h);
	}

	/* アクティブになるボタンのfgを描画する */
	if (pointed_index != BTN_NONE) {
		get_button_rect(pointed_index, &btn_x, &btn_y, &btn_w, &btn_h);
		clear_msgbox_rect_with_fg(btn_x, btn_y, btn_w, btn_h);
		union_rect(x, y, w, h,
			   *x, *y, *w, *h,
			   btn_x + conf_msgbox_x, btn_y + conf_msgbox_y,
			   btn_w, btn_h);
	}

	/* ポイントが解除された場合 */
	if (pointed_index == BTN_NONE)
		return false;

	/* SEを再生する */
	return true;
}

/* メッセージボックス内のボタンのクリックを処理する */
static bool process_button_click(int *x, int *y, int *w, int *h)
{
	/*
	 * システムメニュー表示中は処理しない
	 *  - ボタンは選択できなくなっている (pointed_index == BTN_NONE)
	 *  - 加えて、システムメニュー表示中のスペースキーをここでは無視したい
	 *  - そこで、ここでリターンする
	 */
	if (is_sysmenu)
		return false;

	/* 隠すボタンを処理する (非表示の解除もここで行う) */
	if (process_hide_click(x, y, w, h)) {
		/* 入力は処理済みなので、以降は入力を処理しない */
		clear_input_state();
		return true;
	}

	/* メッセージボックス非表示中は以降処理しない */
	if (is_hidden)
		return false;

	/*
	 * 上キーの押下をヒストリボタンのクリックに変換する
	 *  - NOTE: マウスホイールの上は、上キーに変換されている
	 */
	if (!conf_msgbox_history_disable && is_up_pressed) {
		pointed_index = BTN_HISTORY;
		is_left_clicked = true;
	}

	/* ボタンがポイントされていない場合は以降処理しない */
	if (pointed_index == BTN_NONE)
		return false;

	/* ボタンがクリックされていない場合は以降処理しない */
	if (!is_left_clicked)
		return false;

	/* ボタンの押下を処理する (キー押下から変換された場合を含めて) */
	switch (pointed_index) {
	case BTN_QSAVE:
		play_se(conf_msgbox_btn_qsave_se);
		action_qsave();
		break;
	case BTN_QLOAD:
		play_se(conf_msgbox_btn_qload_se);
		action_qload();
		break;
	case BTN_SAVE:
		play_se(conf_msgbox_btn_save_se);
		action_save();
		break;
	case BTN_LOAD:
		play_se(conf_msgbox_btn_load_se);
		action_load();
		break;
	case BTN_AUTO:
		play_se(conf_msgbox_btn_auto_se);
		action_auto_start(x, y, w, h);
		break;
	case BTN_SKIP:
		play_se(conf_msgbox_btn_skip_se);
		action_skip(x, y, w, h);
		break;
	case BTN_HISTORY:
		play_se(is_up_pressed ?
			conf_msgbox_history_se : conf_msgbox_btn_history_se);
		action_history();
		break;
	case BTN_CONFIG:
		play_se(conf_msgbox_btn_config_se);
		action_config();
		break;
	default:
		assert(0);
		break;
	}

	/* 入力は処理済みなので、以降は入力を処理しない */
	clear_input_state();

	/* ボタンがクリックされた */
	return true;
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

			/* メッセージボックスの非表示を開始する */
			action_hide_start(x, y, w, h);

			/* 入力が処理された */
			return true;
		}

		/* 入力が処理されなかった */
		return false;
	}

	/* メッセージボックスを非表示中の場合、マウスとキーの押下を処理する */
	if(is_space_pressed || is_return_pressed || is_down_pressed ||
	   is_left_clicked || is_right_clicked) {
		/* SEを再生する */
		play_se(conf_msgbox_show_se);

		/* メッセージボックスの非表示を終了する */
		action_hide_end(x, y, w, h);

		/* 入力が処理された */
		return true;
	}

	/* 入力が処理されなかった */
	return false;
}

/* メッセージボックスの非表示を開始する */
static void action_hide_start(int *x, int *y, int *w, int *h)
{
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
}

/* メッセージボックスの非表示を終了する */
static void action_hide_end(int *x, int *y, int *w, int *h)
{
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
}

/* クイックセーブを処理する */
static void action_qsave(void)
{
	will_quick_save = true;
}

/* クイックロードを行う */
static void action_qload(void)
{
	/* クイックロードを行う */
	if (!quick_load()) {
		is_quick_load_failed = true;
		return;
	}

	/* 後処理を行う */
	did_quick_load = true;
}

/* セーブ画面への遷移を処理する */
static void action_save(void)
{
	/* ボイスを停止する */
	set_mixer_input(VOICE_STREAM, NULL);

	need_save_mode = true;
}

/* ロード画面への遷移を処理する */
static void action_load(void)
{
	/* ボイスを停止する */
	set_mixer_input(VOICE_STREAM, NULL);

	need_load_mode = true;
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

/* ヒストリ画面への遷移を処理する */
static void action_history(void)
{
	/* ボイスを停止する */
	set_mixer_input(VOICE_STREAM, NULL);

	need_history_mode = true;
}

/* コンフィグ画面への遷移を処理する */
static void action_config(void)
{
	/* ボイスを停止する */
	set_mixer_input(VOICE_STREAM, NULL);

	need_config_mode = true;
}

/* カスタムシステムメニューのgosubを処理する */
static void action_custom(int index)
{
	/* ボイスを停止する */
	set_mixer_input(VOICE_STREAM, NULL);

	switch (index) {
	case 0:
		if (conf_sysmenu_custom1_gosub == NULL)
			break;
		custom_gosub_label = conf_sysmenu_custom1_gosub;
		need_custom_gosub = true;
		break;
	case 1:
		if (conf_sysmenu_custom2_gosub == NULL)
			break;
		custom_gosub_label = conf_sysmenu_custom2_gosub;
		need_custom_gosub = true;
		break;
	default:
		assert(0);
		break;
	}
}

/* システムメニューの処理を行う */
static bool frame_sysmenu(int *x, int *y, int *w, int *h)
{
#ifdef USE_DEBUGGER
	/* シングルステップか停止要求中の場合 */
	if (dbg_is_stop_requested()) {
		/* システムメニュー表示中でない場合 */
		if (!is_sysmenu) {
			/*
			 * システムメニューには入らない
			 *  - 停止要求中はis_collapsed_sysmenu_pointed()が
			 *    falseになる
			 *  - しかし、エスケープキーの操作も防ぐ必要がある
			 *  - なので、ここでリターンする
			 */
			return false;
		}

		/*
		 * システムメニュー表示中の停止要求に対しては、
		 * システムメニューを終了する
		 */
		is_sysmenu = false;
		is_sysmenu_finished = true;
		return false;
	}
#endif

	/* システムメニューが表示中でないとき */
	if (!is_sysmenu) {
		/* 折りたたみシステムメニューの処理を行う */
		return process_collapsed_sysmenu(x, y, w, h);
	}

	/* 以下、システムメニューを表示中の場合 */

	/* ポイントされているシステムメニューのボタンを求める */
	old_sysmenu_pointed_index = sysmenu_pointed_index;
	sysmenu_pointed_index = get_sysmenu_pointed_button();

	/*
	 * キャンセルの処理を行う
	 *  - 右クリックされた場合
	 *  - エスケープキーが押下された場合
	 *  - ボタンのないところを左クリックされた場合
	 */
	if (is_right_clicked ||
	    is_escape_pressed ||
	    (is_left_clicked && sysmenu_pointed_index == SYSMENU_NONE)) {
		/* SEを再生する */
		play_se(conf_sysmenu_leave_se);

		/* システムメニューを終了する */
		is_sysmenu = false;
		is_sysmenu_finished = true;

		/* 以降のクリック処理を行わない */
		clear_input_state();
		return true;
	}

	/* 状態に応じて、ポイント中の項目を無効にする */
	adjust_sysmenu_pointed_index();

	/* 左クリックされていない場合、何もしない */
	if (!is_left_clicked)
		return false;

	/* システムメニューの項目がポイントされていない場合、何もしない */
	if (sysmenu_pointed_index == SYSMENU_NONE)
		return false;

	/* ポイントされている項目に応じて処理する */
	switch (sysmenu_pointed_index) {
	case SYSMENU_QSAVE:
		play_se(conf_sysmenu_qsave_se);
		action_qsave();
		break;
	case SYSMENU_QLOAD:
		play_se(conf_sysmenu_qload_se);
		action_qload();
		break;
	case SYSMENU_SAVE:
		play_se(conf_sysmenu_save_se);
		action_save();
		break;
	case SYSMENU_LOAD:
		play_se(conf_sysmenu_load_se);
		action_load();
		break;
	case SYSMENU_AUTO:
		play_se(conf_sysmenu_auto_se);
		action_auto_start(x, y, w, h);
		break;
	case SYSMENU_SKIP:
		play_se(conf_sysmenu_skip_se);
		action_skip(x, y, w, h);
		break;
	case SYSMENU_HISTORY:
		play_se(conf_sysmenu_history_se);
		action_history();
		break;
	case SYSMENU_CONFIG:
		play_se(conf_sysmenu_config_se);
		action_config();
		break;
	case SYSMENU_CUSTOM1:
		action_custom(0);
		break;
	case SYSMENU_CUSTOM2:
		action_custom(1);
		break;
	default:
		assert(0);
		break;
	}

	/* システムメニューを終了する */
	is_sysmenu = false;
	is_sysmenu_finished = true;

	/* 以降のクリック処理を行わない */
	clear_input_state();
	return true;
}

/* 折りたてみシステムメニューの処理を行う */
static bool process_collapsed_sysmenu(int *x, int *y, int *w, int *h)
{
	bool enter_sysmenu;

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

	/* システムメニューに入るかを求める */
	enter_sysmenu = false;
	if (is_right_clicked || is_escape_pressed) {
		/* 右クリックされた場合と、エスケープキーが押下された場合 */
		enter_sysmenu = true;
		clear_input_state();
	} else if (is_left_clicked && is_collapsed_sysmenu_pointed()) {
		/* 折りたたみシステムメニューがクリックされたとき */
		enter_sysmenu = true;
		clear_input_state();
	}

	/* システムメニューに入らないとき */
	if (!enter_sysmenu)
		return false;

	/* システムメニューに入る */

	/* SEを再生する */
	play_se(conf_sysmenu_enter_se);

	/* システムメニューを表示する */
	is_sysmenu = true;
	is_sysmenu_first_frame = true;
	is_sysmenu_finished = false;
	sysmenu_pointed_index = get_sysmenu_pointed_button();
	old_sysmenu_pointed_index = SYSMENU_NONE;
	adjust_sysmenu_pointed_index();

	/* メッセージボックス内のボタンをクリアする */
	draw_buttons(x, y, w, h);

	return true;
}

/* 状態に応じて、ポイント中の項目を無効にする */
static void adjust_sysmenu_pointed_index(void)
{
	/* セーブロードが無効な場合、セーブロードのポイントを無効にする */
	if (!is_save_load_enabled() &&
	    (sysmenu_pointed_index == SYSMENU_QSAVE ||
	     sysmenu_pointed_index == SYSMENU_QLOAD ||
	     sysmenu_pointed_index == SYSMENU_SAVE ||
	     sysmenu_pointed_index == SYSMENU_LOAD))
		sysmenu_pointed_index = SYSMENU_NONE;

	/* クイックセーブデータがない場合、QLOADのポイントを無効にする */
	if (!have_quick_save_data() &&
	    sysmenu_pointed_index == SYSMENU_QLOAD)
		sysmenu_pointed_index = SYSMENU_NONE;

	/* 未読の場合、スキップのポイントを無効にする */
	if (!is_skippable() && sysmenu_pointed_index == SYSMENU_SKIP)
		sysmenu_pointed_index = SYSMENU_NONE;
}

/* 選択中のシステムメニューのボタンを取得する */
static int get_sysmenu_pointed_button(void)
{
	int rx, ry, btn_x, btn_y, btn_w, btn_h, i;

#ifdef USE_DEBUGGER
	/* シングルステップか停止要求中の場合、ボタンを選択できなくする */
	if (dbg_is_stop_requested())
		return SYSMENU_NONE;
#endif

	/* システムメニューを表示中でない場合は非選択とする */
	if (!is_sysmenu)
		return SYSMENU_NONE;

	/* マウス座標からシステムメニュー画像内座標に変換する */
	rx = mouse_pos_x - conf_sysmenu_x;
	ry = mouse_pos_y - conf_sysmenu_y;

	/* ボタンを順番に見ていく */
	for (i = SYSMENU_QSAVE; i <= SYSMENU_CUSTOM2; i++) {
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
	case SYSMENU_CUSTOM1:
		*x = conf_sysmenu_custom1_x;
		*y = conf_sysmenu_custom1_y;
		*w = conf_sysmenu_custom1_width;
		*h = conf_sysmenu_custom1_height;
		break;
	case SYSMENU_CUSTOM2:
		*x = conf_sysmenu_custom2_x;
		*y = conf_sysmenu_custom2_y;
		*w = conf_sysmenu_custom2_width;
		*h = conf_sysmenu_custom2_height;
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
		/* インラインウェイトを処理する */
		if (is_inline_wait) {
			if ((float)get_stop_watch_lap(&inline_sw) / 1000.0f >
			    inline_wait_time)
				is_inline_wait = false;
		}
		if (!is_inline_wait) {
			/* 本文を描画する */
			draw_msgbox(x, y, w, h);
		}
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
	/* システムGUIから戻った場合 */
	if (gui_sys_flag)
		return true;

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
	int char_count, i, mblen;
	int glyph_width, glyph_height, ret_width, ret_height, ofs_x, ofs_y;

	/*
	 * システムGUIから戻った場合の描画ではないはず
	 *  - ただしディミングの場合はシステムGUIから戻った場合でも描画する
	 */
	if (!is_dimming)
		assert(!gui_sys_flag);

	/* 今回のフレームで描画する文字数を取得する */
	char_count = get_frame_chars();
	if (char_count == 0)
		return;

	/* 1文字ずつ描画する */
	for (i = 0; i < char_count; i++) {
		assert(*msg_cur);

		/* 途中のエスケープシーケンスを処理する */
		process_escape_sequence(&msg_cur);
		if (is_inline_wait)
			return;

		/* ワードラッピングを処理する */
		do_word_wrapping();

		/* 描画する文字を取得する */
		mblen = utf8_to_utf32(msg_cur, &wc);
		if (mblen == -1) {
			/* 不正なUnicode: 本文の描画を終了したことにする */
			set_end_of_msg();
			return;
		}

		/* 縦書きの句読点変換を行う */
		if (conf_msgbox_tategaki)
			wc = convert_tategaki_char(wc);

		/* 描画する文字の幅と高さを取得する */
		glyph_width = get_glyph_width(wc);
		glyph_height = get_glyph_height(wc);

		/* 右側の幅が足りなければ改行する */
		process_lf(wc, glyph_width, glyph_height);

		/* 小さいひらがな/カタカタのオフセットを計算する */
		if (conf_msgbox_tategaki && is_small_kana(wc)) {
			/* FIXME: 何らかの調整を加える */
			ofs_x = 0;
			ofs_y = 0;
		} else {
			ofs_x = 0;
			ofs_y = 0;
		}

		/* 描画する */
		draw_char_on_msgbox(pen_x + ofs_x, pen_y + ofs_y, wc,
				    body_color, body_outline_color,
				    &ret_width, &ret_height, conf_font_size,
				    is_dimming);

		/* 更新領域を求める */
		union_rect(x, y, w, h,
			   *x, *y, *w, *h,
			   msgbox_x + pen_x, msgbox_y + pen_y,
			   ret_width, ret_height);
		*x = *x < 0 ? 0 : *x;

		/* ルビ用のペン位置を更新する */
		if (!conf_msgbox_tategaki) {
			pen_ruby_x = pen_x;
			pen_ruby_y = pen_y - conf_font_ruby_size;
		} else {
			pen_ruby_x = pen_x + ret_width;
			pen_ruby_y = pen_y;
		}

		/* 次の文字へ移動する */
		msg_cur += mblen;
		if (!conf_msgbox_tategaki) {
			pen_x += glyph_width + conf_msgbox_margin_char;
		} else {
			if (is_tategaki_punctuation(wc))
				pen_y += conf_font_size;
			else
				pen_y += glyph_height;
			pen_y += conf_msgbox_margin_char;
		}
		drawn_chars++;
	}

	/* 末尾のエスケープシーケンスを処理する */
	process_escape_sequence(&msg_cur);
}

/*
 * 今回のフレームで描画する文字数を取得する
 *  - クリックやキー入力、システムGUIへの移行などすべてを加味する
 */
static int get_frame_chars(void)
{
	if (!is_dimming)
		assert(!gui_sys_flag);

	/* 繰り返し動作しない場合 (dimmingを含む) */
	if (!is_in_command_repetition()) {
		/* すべての文字を描画する */
		return total_chars;
	}

	/*
	 * セーブのサムネイルを作成するために全文字描画する場合
	 *  - クイックセーブされる場合 (現状ではQSのサムネイルは未使用)
	 *  - システムGUIに遷移する場合 (原理上どのGUIからもセーブできるので)
	 */
	if (will_quick_save || need_save_mode || need_load_mode ||
	    need_history_mode || need_config_mode) {
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
	/* 未読ならそもそもスキップできない */
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
static int calc_frame_chars_by_lap(void)
{
	float lap;
	float progress;
	int char_count;

	/*
	 * システムGUIのコンフィグでテキストスピードが最大のときは、
	 * ノーウェイトで全部描画する。
	 */
	if (get_text_speed() == 1.0f)
		return total_chars - drawn_chars;

	/*
	 * 経過時間(秒)を取得する
	 *  - インラインウェイトの分を差し引く
	 */
	lap = (float)get_stop_watch_lap(&click_sw) / 1000.0f -
		inline_wait_time_total;

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
static void process_escape_sequence(const char **s)
{
	const char *p;

	/* エスケープシーケンスが続く限り処理する */
	p = *s;
	while (*p == '\\') {
		switch (*(p + 1)) {
		case 'n':
			/* 改行 */
			process_escape_sequence_lf(s);
			break;
		case '#':
			/* 色指定 */
			if (!process_escape_sequence_color(s))
				return; /* 不正: 読み飛ばさない */
			break;
		case '@':
			/* サイズ指定 */
			if (!process_escape_sequence_size(s))
				return; /* 不正: 読み飛ばさない */
			break;
		case 'w':
			/* インラインウェイト */
			if (!process_escape_sequence_wait(s))
				return; /* 不正: 読み飛ばさない */
			break;
		case 'p':
			/* ペン移動 */
			if (!process_escape_sequence_pen(s))
				return; /* 不正: 読み飛ばさない */
			break;
		case '^':
			/* ルビ */
			if (!process_escape_sequence_ruby(s))
				return; /* 不正: 読み飛ばさない */
			break;
		default:
			/* 不正なエスケープシーケンスなので読み飛ばさない */
			return;
		}
		p = *s;
	}
}

/* 改行("\\n")を処理する */
static void process_escape_sequence_lf(const char **s)
{
	if (!conf_msgbox_tategaki) {
		pen_y += conf_msgbox_margin_line;
		pen_x = conf_msgbox_margin_left;
	} else {
		pen_x -= conf_msgbox_margin_line;
		pen_y = conf_msgbox_margin_top;
	}
	*s += 2;
}

/* 色指定("\\#{RRGGBB}")を処理する */
static bool process_escape_sequence_color(const char **s)
{
	char color_code[7];
	const char *p;
	uint32_t r, g, b;
	int rgb;

	p = *s;
	assert(*p == '\\');
	assert(*(p + 1) == '#');

	/* '{'をチェックする */
	if (*(p + 2) != '{')
		return false;

	/* 長さが足りない場合 */
	if (strlen(p + 3) < 6)
		return false;

	/* '}'をチェックする */
	if (*(p + 9) != '}')
		return false;

	if (!is_dimming) {
		/* カラーコードを読む */
		memcpy(color_code, p + 3, 6);
		color_code[6] = '\0';
		rgb = 0;
		sscanf(color_code, "%x", &rgb);
		r = (rgb >> 16) & 0xff;
		g = (rgb >> 8) & 0xff;
		b = rgb & 0xff;
		body_color = make_pixel_slow(0xff, r, g, b);
	}

	/* "\\#{" + "xxxxxx" + "}" */
	*s += 3 + 6 + 1;
	return true;
}

/* サイズ指定("\\@{xxx}")を処理する */
static bool process_escape_sequence_size(const char **s)
{
	char size_spec[8];
	const char *p;
	int i, size;

	p = *s;
	assert(*p == '\\');
	assert(*(p + 1) == '@');

	/* '{'をチェックする */
	if (*(p + 2) != '{')
		return false;

	/* サイズ文字列を読む */
	for (i = 0; i < (int)sizeof(size_spec) - 1; i++) {
		if (*(p + 3 + i) == '\0')
			return false;
		if (*(p + 3 + i) == '}')
			break;
		size_spec[i] = *(p + 3 + i);
	}
	size_spec[i] = '\0';

	/* サイズ文字列を整数に変換する */
	size = 0;
	sscanf(size_spec, "%d", &size);

	/* フォントサイズを変更する */
	set_font_size(size);

	/* "\\@{" + "xxx" + "}" */
	*s += 3 + i + 1;
	return true;
}

/* インラインウェイト("\\w{f.f}")を処理する */
static bool process_escape_sequence_wait(const char **s)
{
	char time_spec[16];
	const char *p;
	int i;

	p = *s;
	assert(*p == '\\');
	assert(*(p + 1) == 'w');

	/* '{'をチェックする */
	if (*(p + 2) != '{')
		return false;

	/* 時間文字列を読む */
	for (i = 0; i < (int)sizeof(time_spec) - 1; i++) {
		if (*(p + 3 + i) == '\0')
			return false;
		if (*(p + 3 + i) == '}')
			break;
		time_spec[i] = *(p + 3 + i);
	}
	time_spec[i] = '\0';

	/* 時間文字列を浮動小数点数に変換する */
	inline_wait_time = 0;
	sscanf(time_spec, "%f", &inline_wait_time);

	if (!is_dimming) {
		/* インラインウェイトを開始する */
		if (inline_wait_time > 0) {
			is_inline_wait = true;
			inline_wait_time_total += inline_wait_time;
			reset_stop_watch(&inline_sw);
		}
	}

	/* "\\w{" + "f.f" + "}" */
	*s += 3 + i + 1;
	return true;
}

/* ペン移動("\\p{x,y}")を処理する */
static bool process_escape_sequence_pen(const char **s)
{
	char pos_spec[32];
	const char *p;
	int i;
	bool separator_found;

	p = *s;
	assert(*p == '\\');
	assert(*(p + 1) == 'p');

	/* '{'をチェックする */
	if (*(p + 2) != '{')
		return false;

	/* 座標文字列を読む */
	separator_found = false;
	for (i = 0; i < (int)sizeof(pos_spec) - 1; i++) {
		if (*(p + 3 + i) == '\0')
			return false;
		if (*(p + 3 + i) == '}')
			break;
		if (*(p + 3 + i) == ',')
			separator_found = true;
		pos_spec[i] = *(p + 3 + i);
	}
	pos_spec[i] = '\0';
	if (!separator_found)
		return false;

	/* 座標文字列を浮動小数点数に変換する */
	sscanf(pos_spec, "%d,%d", &pen_x, &pen_y);

	/* "\\w{" + "x,y" + "}" */
	*s += 3 + i + 1;
	return true;
}

/* ルビ("\\^{ルビ}")を処理する */
static bool process_escape_sequence_ruby(const char **s)
{
	char ruby[64];
	const char *p;
	uint32_t wc;
	int i, font_size,mblen, ret_width, ret_height;

	p = *s;
	assert(*p == '\\');
	assert(*(p + 1) == '^');

	/* '{'をチェックする */
	if (*(p + 2) != '{')
		return false;

	/* ルビを読む */
	for (i = 0; i < (int)sizeof(ruby) - 1; i++) {
		if (*(p + 3 + i) == '\0')
			return false;
		if (*(p + 3 + i) == '}')
			break;
		ruby[i] = *(p + 3 + i);
	}
	ruby[i] = '\0';

	/* \^{ + ruby[] + } */
	*s += 3 + i + 1;

	/* フォントサイズを退避して、ルビ用に設定する */
	font_size = get_font_size();
	set_font_size(conf_font_ruby_size > 0 ?
		      conf_font_ruby_size : conf_font_size / 5);

	/* 描画する */
	p = ruby;
	while (*p) {
		mblen = utf8_to_utf32(p, &wc);
		if (mblen == -1)
			break;

		if (!is_drawing_namebox) {
			draw_char_on_msgbox(pen_ruby_x, pen_ruby_y, wc,
					    body_color, body_outline_color,
					    &ret_width, &ret_height,
					    conf_font_ruby_size, is_dimming);
		} else {
			draw_char_on_namebox(pen_ruby_x, pen_ruby_y, wc,
					     name_color, name_outline_color,
					     &ret_width, &ret_height,
					     conf_font_ruby_size, is_dimming);
		}

		if (!conf_msgbox_tategaki)
			pen_ruby_x += ret_width;
		else
			pen_ruby_y += ret_height;

		p += mblen;
	}

	/* フォントサイズを復元する */
	set_font_size(font_size);

	return true;
}

/* ワードラッピングを処理する */
static void do_word_wrapping(void)
{
	if (conf_msgbox_tategaki)
		return;

	if (is_after_space) {
		if (pen_x + get_en_word_width() >=
		    msgbox_w - conf_msgbox_margin_right) {
			pen_y += conf_msgbox_margin_line;
			pen_x = conf_msgbox_margin_left;
		}
	}

	is_after_space = *msg_cur == ' ';
}

/* 縦書きの句読点変換を行う */
static uint32_t convert_tategaki_char(uint32_t wc)
{
	switch (wc) {
	case U32_C('、'): return U32_C('︑');
	case U32_C('，'): return U32_C('︐');
	case U32_C('。'): return U32_C('︒');
	case U32_C('（'): return U32_C('︵');
	case U32_C('）'): return U32_C('︶');
	case U32_C('｛'): return U32_C('︷');
	case U32_C('｝'): return U32_C('︸');
	case U32_C('「'): return U32_C('﹁');
	case U32_C('」'): return U32_C('﹂');
	case U32_C('『'): return U32_C('﹃');
	case U32_C('』'): return U32_C('﹄');
	case U32_C('【'): return U32_C('︻');
	case U32_C('】'): return U32_C('︼');
	case U32_C('［'): return U32_C('﹇');
	case U32_C('］'): return U32_C('﹈');
	case U32_C('〔'): return U32_C('︹');
	case U32_C('〕'): return U32_C('︺');
	case U32_C('…'): return U32_C('︙');
	case U32_C('‥'): return U32_C('︰');
	case U32_C('ー'): return U32_C('丨');
	default:
		break;
	}
	return wc;
}

/* 縦書きの句読点かどうか調べる */
static bool is_tategaki_punctuation(uint32_t wc)
{
	switch (wc) {
	case U32_C('︑'): return true;
	case U32_C('︐'): return true;
	case U32_C('︒'): return true;
	case U32_C('︵'): return true;
	case U32_C('︶'): return true;
	case U32_C('︷'): return true;
	case U32_C('︸'): return true;
	case U32_C('﹁'): return true;
	case U32_C('﹂'): return true;
	case U32_C('﹃'): return true;
	case U32_C('﹄'): return true;
	case U32_C('︻'): return true;
	case U32_C('︼'): return true;
	case U32_C('﹇'): return true;
	case U32_C('﹈'): return true;
	case U32_C('︹'): return true;
	case U32_C('︺'): return true;
	case U32_C('︙'): return true;
	case U32_C('︰'): return true;
	case U32_C('丨'): return true;
	default:
		break;
	}
	return false;
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
static void process_lf(uint32_t c, int glyph_width, int glyph_height)
{
	if (!conf_msgbox_tategaki) {
		/* 右側の幅が足りる場合、改行しない */
		if (pen_x + glyph_width + conf_msgbox_margin_char <
		    msgbox_w - conf_msgbox_margin_right)
			return;
	} else {
		/* 下側の幅が足りる場合、改行しない */
		if (pen_y + glyph_height + conf_msgbox_margin_char <
		    msgbox_h - conf_msgbox_margin_bottom)
			return;
	}

	/* 禁則文字の場合、改行しない */
	if (c == ' ' || c == ',' || c == '.' || c == ':' || c == ';' ||
	    c == CHAR_TOUTEN || c == CHAR_KUTEN)
		return;

	/* 改行する */
	if (!conf_msgbox_tategaki) {
		pen_y += conf_msgbox_margin_line;
		pen_x = conf_msgbox_margin_left;
	} else {
		pen_x -= conf_msgbox_margin_line;
		pen_y = conf_msgbox_margin_top;
	}
}

/* 小さい仮名文字であるか調べる */
static bool is_small_kana(uint32_t wc)
{
	switch (wc) {
	case U32_C('ぁ'): return true;
	case U32_C('ぃ'): return true;
	case U32_C('ぅ'): return true;
	case U32_C('ぇ'): return true;
	case U32_C('ぉ'): return true;
	case U32_C('っ'): return true;
	case U32_C('ゃ'): return true;
	case U32_C('ゅ'): return true;
	case U32_C('ょ'): return true;
	case U32_C('ゎ'): return true;
	case U32_C('ゕ'): return true;
	case U32_C('ゖ'): return true;
	case U32_C('ァ'): return true;
	case U32_C('ィ'): return true;
	case U32_C('ゥ'): return true;
	case U32_C('ェ'): return true;
	case U32_C('ォ'): return true;
	case U32_C('ッ'): return true;
	case U32_C('ャ'): return true;
	case U32_C('ュ'): return true;
	case U32_C('ョ'): return true;
	case U32_C('ヮ'): return true;
	case U32_C('ヵ'): return true;
	case U32_C('ヶ'): return true;
	default: break;
	}
	return false;
}

/* クリックアニメーションを描画する */
static void draw_click(int *x, int *y, int *w, int *h)
{
	int click_x, click_y, click_w, click_h;
	int lap, index;

	assert(!is_sysmenu);

	/* 継続行で、改行のみの場合、クリック待ちを行わない */
	if (is_continue_mode && total_chars == 0) {
		if (is_in_command_repetition())
			stop_command_repetition();
		return;
	}

	/* 入力があったら繰り返しを終了する */
	if (check_stop_click_animation())
		stop_command_repetition();

	/* クリックアニメーションの初回表示のとき */
	if (is_click_first) {
		is_click_first = false;

		/* 表示位置を設定する */
		if (conf_click_move) {
			set_click_index(0);
			get_click_rect(&click_x, &click_y, &click_w, &click_h);
			if (!conf_msgbox_tategaki) {
				if (pen_x + click_w  >= msgbox_w -
				    conf_msgbox_margin_right) {
					pen_y += conf_msgbox_margin_line;
					pen_x = conf_msgbox_margin_left;
				}
			} else {
				if (pen_y + click_h  >= msgbox_h -
				    conf_msgbox_margin_bottom) {
					pen_x -= conf_msgbox_margin_line;
					pen_y = conf_msgbox_margin_top;
				}
			}
			set_click_position(pen_x + conf_msgbox_x,
					   pen_y + conf_msgbox_y);
		} else {
			set_click_position(conf_click_x, conf_click_y);
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
			((int)(conf_click_interval * 1000) / click_frames) %
			click_frames;
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
static bool check_stop_click_animation(void)
{
	/* システムメニュー表示中はここに来ない */
	assert(!is_sysmenu);

	/* 本文の描画が完了していなければここに来ない */
	assert(is_end_of_msg());

#ifdef USE_DEBUGGER
	/* デバッガから停止要求がある場合 */
	if (dbg_is_stop_requested()) {
		/* ボイスがなければ停止する */
		if (!have_voice)
			return true;

		/* ボイスが再生完了したフレームで停止する */
		if (is_mixer_sound_finished(VOICE_STREAM))
			return true;

		/* 何もないところをクリックされた場合は停止する */
		if (is_left_clicked &&
		    (pointed_index == BTN_NONE) &&
		    is_collapsed_sysmenu_pointed() &&
		    !is_sysmenu)
			return true;

		/*
		 * キーが押下された場合
		 *  - システムメニュー表示中は除く
		 */
		if ((is_down_pressed || is_return_pressed) &&
		    !is_sysmenu)
			return true;
	}
#endif

	/* スキップモードの場合 */
	if (is_skip_mode()) {
		assert(!is_auto_mode());

		/* 既読でない場合はスキップできないので停止しない */
		if (!is_skippable())
			return false;

		/* 割り込み不可ではない場合はスキップにより停止する */
		if (!is_non_interruptible())
			return true;

		/* ボイスがない場合はスキップにより停止する */
		if (!have_voice)
			return true;

		/* ボイスが再生完了している場合はスキップにより停止する */
		if (is_mixer_sound_finished(VOICE_STREAM))
			return true;

		/* 停止しない */
		return false;
	}

	/* コントロールキーが押下されている場合 */
	if (is_control_pressed) {
		/* オートモードの場合はコントロールキーを無視する */
		if (is_auto_mode())
			return false;

		/* 既読でない場合はスキップできないので停止しない */
		if (!is_skippable())
			return false;

		/* 割り込み不可でない場合はスキップにより停止する */
		if (!is_non_interruptible())
			return true;

		/* ボイスがない場合はスキップにより停止する */
		if (!have_voice)
			return true;

		/* ボイスが再生完了している場合はスキップにより停止する */
		if (is_mixer_sound_finished(VOICE_STREAM))
			return true;

		/* 停止しない */
		return false;
	}

	/* システムGUIから戻ってコマンドが開始されている場合 */
	if (gui_sys_flag) {
		/* FYI: ボイスは再生されていない */

		/* 何もないところをクリックされた場合は停止する */
		if (is_left_clicked &&
		    (pointed_index == BTN_NONE) &&
		    !is_collapsed_sysmenu_pointed())
			return true;

		/* キーが押下された場合は停止する */
		if (is_return_pressed || is_down_pressed)
			return true;

		/* 停止しない */
		return false;
	}

	/* 何もないところをクリックされた場合と、キーが押下された場合 */
	if ((is_left_clicked && (pointed_index == BTN_NONE) &&
	     !is_collapsed_sysmenu_pointed()) ||
	    (is_return_pressed || is_down_pressed)) {
		/*
		 * 割り込み不可モードでない場合は停止する
		 *  - 再生中のボイスがあればcleanup()で停止される
		 */
		if (!is_non_interruptible())
			return true;

		/*
		 * 割り込み不可モードでも、ボイスがないか、再生完了後の場合は
		 * 停止する
		 */
		if (!have_voice || is_mixer_sound_finished(VOICE_STREAM))
			return true;

		/* 停止しない */
		return false;
	}

	/* 停止しない */
	return false;
}

/* システムメニューを描画する */
static void draw_sysmenu(bool calc_only, int *x, int *y, int *w, int *h)
{
	int i, bx, by, bw, bh;
	bool redraw;
	bool sel[SYSMENU_COUNT];

	/* 描画するかの判定状態を初期化する */
	redraw = false;

	/* システムメニューの最初のフレームの場合、描画する */
	if (is_sysmenu_first_frame)
		redraw = true;

	/* クイックセーブボタンがポイントされているかを取得する */
	for (i = 0; i < SYSMENU_COUNT; i++) {
		if (sysmenu_pointed_index == i) {
			sel[i] = true;
			if (old_sysmenu_pointed_index != i &&
			    !is_sysmenu_first_frame) {
				play_se(conf_sysmenu_change_se);
				redraw = true;
			}
		} else {
			sel[i] = false;
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
					   sel[SYSMENU_QSAVE],
					   sel[SYSMENU_QLOAD],
					   sel[SYSMENU_SAVE],
					   sel[SYSMENU_LOAD],
					   sel[SYSMENU_AUTO],
					   sel[SYSMENU_SKIP],
					   sel[SYSMENU_HISTORY],
					   sel[SYSMENU_CONFIG],
					   sel[SYSMENU_CUSTOM1],
					   sel[SYSMENU_CUSTOM2],
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

#ifdef USE_DEBUGGER
	/* シングルステップか停止要求中の場合 */
	if (dbg_is_stop_requested())
		return false;
#endif

	/* システムメニューを表示中の場合 */
	if (is_sysmenu)
		return false;

	/* オートモードかスキップモードの場合 */
	if (is_auto_mode() || is_skip_mode())
		return false;

	/* マウスカーソル座標をチェックする */
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
 * 重ね塗りを行う (dimming)
 *  - 全画面スタイルで、すでに読んだ部分を暗くするための文字描画
 */
static void draw_dimming(int *x, int *y, int *w, int *h)
{
	/* コンフィグでdimmingが有効 */
	assert(conf_msgbox_dim);

	/* 次のコマンドに移るときだけdimmingする */
	assert(!is_in_command_repetition());

	/* システムGUIに移行するときにはdimmingしない */
	assert(!did_quick_load);
	assert(!need_save_mode);
	assert(!need_load_mode);
	assert(!need_history_mode);
	assert(!need_config_mode);

	/*
	 * 重ね塗りを有効にする
	 *  - 現状では参照されていない
	 *  - TODO: 将来、色のエスケープシーケンスを無視するのに使う
	 */
	is_dimming = true;

	/* 描画する本文の先頭を巻き戻す */
	msg_cur = msg_top;
	drawn_chars = 0;

	/* 描画位置を巻き戻す */
	pen_x = orig_pen_x;
	pen_y = orig_pen_y;

	/* dimming用の文字色を求める */
	body_color = make_pixel_slow(0xff,
				     (uint32_t)conf_msgbox_dim_color_r,
				     (uint32_t)conf_msgbox_dim_color_g,
				     (uint32_t)conf_msgbox_dim_color_b);
	body_outline_color =
			make_pixel_slow(0xff,
				(uint32_t)conf_msgbox_dim_color_outline_r,
				(uint32_t)conf_msgbox_dim_color_outline_g,
				(uint32_t)conf_msgbox_dim_color_outline_b);
	if (conf_font_outline_remove)
		body_outline_color = body_color;

	/*
	 * 本文を描画する
	 *  - (!is_in_command_repetition())なので全文字が描画される
	 */
	draw_msgbox(x, y, w, h);
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

	set_mixer_input(SYS_STREAM, w);
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
static bool cleanup(void)
{
	/* ペンの位置を保存する */
	set_pen_position(pen_x, pen_y);

	/* PCMストリームの再生を終了する */
	if (!conf_voice_stop_off)
		set_mixer_input(VOICE_STREAM, NULL);

	/* クリックアニメーションを非表示にする */
#ifdef USE_DEBUGGER
	if (dbg_is_stop_requested())
		show_click(true);
#else
	show_click(false);
#endif

	/*
	 * 次のコマンドに移動するときは、表示中のメッセージをなしとする
	 *  - システムGUIに移行する場合は、アクティブなままにしておく
	 *  - クイックロードされた場合はquick_load()ですでにクリアされている
	 */
	if (!did_quick_load && !need_save_mode && !need_load_mode &&
	    !need_history_mode && !need_config_mode)
		clear_message_active();

	/* 既読にする */
	set_seen();

	/* 名前と本文を解放する */
	if (!need_save_mode && !need_load_mode && !need_history_mode &&
	    !need_config_mode) {
		if (name_top != NULL) {
			free(name_top);
			name_top = NULL;
		}
		if (msg_top != NULL) {
			free(msg_top);
			msg_top = NULL;
			msg_cur = NULL;
		}
	}

	/* カスタムシステムメニューのgosubを処理する */
	if (need_custom_gosub && custom_gosub_label != NULL) {
		push_return_point_minus_one();
		if (!move_to_label(custom_gosub_label))
			return false;
	}

	/* 次のコマンドに移動する */
	if (!did_quick_load && !need_save_mode && !need_load_mode &&
	    !need_history_mode && !need_config_mode && !need_custom_gosub) {
		if (!move_to_next_command())
			return false;
	}

	return true;
}
