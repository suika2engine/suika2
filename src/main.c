/* -*- coding: utf-8; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Suika2
 * Copyright (C) 2001-2022, Keiichi Tabata. All rights reserved.
 */

/*
 * ゲームループのメインモジュール
 *
 * [Changes]
 *  - 2016/05/27 作成
 *  - 2017/08/13 スイッチに対応
 *  - 2018/07/21 gosubに対応
 *  - 2019/09/17 newsに対応
 *  - 2021/06/10 chaに対応
 *  - 2021/06/12 shakeに対応
 *  - 2021/06/15 setsaveに対応
 *  - 2021/07/19 chsに対応
 *  - 2021/07/30 オートモードに対応
 *  - 2021/07/31 スキップモードに対応
 *  - 2022/05/11 動画再生に対応
 *  - 2022/06/06 デバッガに対応
 */

#include "suika.h"

/* false assertion */
#define COMMAND_DISPATCH_NOT_IMPLEMENTED	(0)

/*
 * 入力の状態
 *  - ControlキーとSpaceキーは、フレームをまたがって押下状態になる
 *  - その他のキー・マウスボタンはフレームごとに押下状態がクリアされる
 */

bool is_left_button_pressed;
bool is_right_button_pressed;
bool is_left_clicked;
bool is_right_clicked;
bool is_return_pressed;
bool is_space_pressed;
bool is_escape_pressed;
bool is_up_pressed;
bool is_down_pressed;
bool is_left_arrow_pressed;
bool is_right_arrow_pressed;
bool is_page_up_pressed;
bool is_page_down_pressed;
bool is_control_pressed;
bool is_s_pressed;
bool is_l_pressed;
bool is_h_pressed;
bool is_touch_canceled;
bool is_swiped;

int mouse_pos_x;
int mouse_pos_y;
bool is_mouse_dragging;

/* 複数のイテレーションに渡るコマンドの実行中であるか */
static bool is_in_repetition;

/* メッセージの表示中状態であるか */
static bool flag_message_active;

/* オートモードが実行中であるか */
static bool flag_auto_mode;

/* スキップモードが実行中であるか */
static bool flag_skip_mode;

/* セーブ・ロード画面が許可されているか */
static bool flag_save_load_enabled = true;

/* 割り込み不可モードであるか */
static bool flag_non_interruptible;

/* ペンの位置 */
static int saved_pen_x;
static int saved_pen_y;

/* 前方参照 */
static void post_process(void);
static bool dispatch_command(bool *cont);

#ifdef USE_EDITOR
/* 実行中であるか */
static bool dbg_running;

/* 停止が要求されているか */
static bool dbg_request_stop;

/* GUI直後の停止が要求されているか */
static bool dbg_request_stop_after_gui;

/* エラー状態であるか */
static bool dbg_runtime_error;

/* エラーカウント */
static int dbg_error_count;

/* 前方参照 */
static bool pre_dispatch(void);
#endif

/*
 * ゲームループの初期化処理を実行する
 */
void init_game_loop(void)
{
#ifdef SUIKA_TARGET_ANDROID
	/* Android NDK用に変数を初期化する */
	is_left_button_pressed = false;
	is_right_button_pressed = false;
	is_left_clicked = false;
	is_right_clicked = false;
	is_return_pressed = false;
	is_space_pressed = false;
	is_escape_pressed = false;
	is_up_pressed = false;
	is_down_pressed = false;
	is_page_up_pressed = false;
	is_page_down_pressed = false;
	is_control_pressed = false;
	is_s_pressed = false;
	is_l_pressed = false;
	is_h_pressed = false;
	mouse_pos_x = 0;
	mouse_pos_y = 0;
	is_mouse_dragging = false;
	is_in_repetition = false;
	flag_message_active = false;
	flag_auto_mode = false;
	flag_save_load_enabled = true;
	flag_non_interruptible = false;

#endif

	srand((unsigned int)time(NULL));

#ifdef USE_EDITOR
	dbg_running = false;
	on_change_position();
#endif
}

#if !defined(USE_EDITOR)
/*
 * ゲームループの中身を実行する
 */
bool game_loop_iter(void)
{
	bool is_gui, cont;

	/* GUIを実行中であるかを取得する */
	is_gui = is_gui_mode();

	/* GUI実行中の場合を処理する */
	if (is_gui) {
		bool is_cmd_gui = get_command_type() == COMMAND_GUI;

		/* GUIのフレームを実行する */
		if (!run_gui_mode())
			return false; /* エラー */

		/* GUIが終了した場合 */
		if (!is_gui_mode()) {
			if (is_gui_loaded()) {
				/* ロードされたのでGUI終了処理を行う */
				cleanup_gui();
			} else if (is_cmd_gui) {
				/* @guiコマンド内なのでコマンド終了処理のためにgui_command()を呼び出す */
				is_gui = false;
			} else {
				/* システムGUIの中でgotoが選択された場合 */
				const char *label = get_gui_result_label();
				if (label != NULL) {
					if (!move_to_label(label))
						return false;
				}
			}
		}
	}

	/* GUI実行中でない場合 */
	if (!is_gui) {
		/* 連続するコマンドを実行する */
		do {
			/* ディスパッチする */
			if (!dispatch_command(&cont))
				return false;
		} while (cont);
	}

	post_process();

	return true;
}
#endif

#if defined(USE_EDITOR)
/*
 * ゲームループの中身を実行する
 */
bool game_loop_iter(void)
{
	bool need_dummy_render;

	need_dummy_render = false;

	/* 実行制御を行う */
	if (!pre_dispatch()) {
		/* 停止中で実行されない場合、ステージを描画してフレームを終了する */
		render_stage();
		if (conf_sysmenu_hidden != 0)
			render_collapsed_sysmenu(false);
		return true;
	}

	/* @guiコマンドに対して一行実行ボタンが押下されたとき */
	if (get_command_type() == COMMAND_GUI && !is_gui_mode() && dbg_request_stop) {
		dbg_request_stop = false;
		dbg_request_stop_after_gui = true;
	}

	/* フレームを実行する */
	if (!is_gui_mode()) {
		/* GUI実行中でない場合 */
		do {
			/* ディスパッチする */
			if (!dispatch_command(&need_dummy_render)) {
				/* 実行時エラーの場合、エラー終了をキャンセルする */
				if (dbg_runtime_error) {
					dbg_runtime_error = false;
					dbg_stop();
					on_change_position();
					break;
				} else {
					/* 最後まで実行した場合、最後のコマンドに留まる */
					dbg_stop();
					on_change_position();
					break;
				}
			}
		} while (0);
	} else {
		/* GUI実行中の場合 */
		do {
			bool is_cmd_gui = get_command_type() == COMMAND_GUI;

			/* GUIのフレームを実行する */
			if (!run_gui_mode()) {
				/* エラー */
				return false;
			}

			/* GUIが終了した場合 */
			if (!is_gui_mode()) {
				/* 一行実行で実行された@guiコマンドの直後で停止する */
				if (dbg_request_stop_after_gui) {
					dbg_request_stop = dbg_request_stop_after_gui;
					dbg_request_stop_after_gui = false;
				}

				/* ロードされた場合、GUI終了処理を行う */
				if (is_gui_loaded()) {
					cleanup_gui();
					break;
				}

				/* @guiコマンド内なので、終了処理を行う */
				if (is_cmd_gui) {
					if (!gui_command())
						return false;
					break;
				}

				break;
			}

			/* @guiコマンドの実行中に停止ボタンが押された場合 */
			if (get_command_type() == COMMAND_GUI && dbg_request_stop) {
				stop_gui_mode();
				cleanup_gui();
				is_in_repetition = false;
				dbg_stop();
				break;
			}
		} while(0);
	}

	/* 描画のないコマンドである場合 */
	if (need_dummy_render) {
		/* 実行中でないので画面を再描画する */
		render_stage();
		if (conf_sysmenu_hidden != 0)
			render_collapsed_sysmenu(false);
	}

	post_process();

	return true;
}
#endif

/* ゲームループのフレームの後処理を実行する */
static void post_process(void)
{
	/* サウンドのフェード処理を実行する */
	process_sound_fading();

	/*
	 * 入力の状態をリセットする
	 *  - マウスボタン押下、Controlキー押下、ドラッグ状態以外は1フレームごとにリセットする
	 */
	is_left_clicked = false;
	is_right_clicked = false;
	is_space_pressed = false;
	is_return_pressed = false;
	is_escape_pressed = false;
	is_up_pressed = false;
	is_down_pressed = false;
	is_left_arrow_pressed = false;
	is_right_arrow_pressed = false;
	is_s_pressed = false;
	is_l_pressed = false;
	is_h_pressed = false;
	is_touch_canceled = false;
	is_swiped = false;
}

#ifdef USE_EDITOR
/* デバッガ用のコマンドディスパッチの前処理 */
static bool pre_dispatch(void)
{
#ifdef USE_EDITOR
	/* コマンドがない場合 */
	if (get_command_count() == 0) {
		dbg_request_stop = true;
		on_change_running_state(false, false);

		/* コマンドディスパッチへ進めない */
		return false;
	}
#endif

	/* 実行中の場合 */
	if (dbg_running) {
		/* 停止が押された場合 */
		if (is_stop_pushed()) {
			dbg_request_stop = true;
			on_change_running_state(true, true);
		}

		/* コマンドディスパッチへ進む */
		return true;
	}

	/* 停止中で、実行するスクリプトが変更された場合 */
	if (is_script_opened()) {
		char *scr;
		scr = strdup(get_opened_script());
		if (scr == NULL) {
			log_memory();
			return false;
		}
		if (strcmp(scr, "DEBUG") == 0) {
			free(scr);
			return false;
		}
		if (!load_script(scr)) {
			free(scr);
			if (!load_debug_script())
				return false;
		} else {
			free(scr);
		}
	}

	/* 行番号が変更された場合 */
	if (is_exec_line_changed()) {
		int index = get_command_index_from_line_num(get_changed_exec_line());
		if (index != -1)
			move_to_command_index(index);
		else
			move_to_command_index(get_command_count() - 1);
	}

#ifndef USE_EDITOR
	/* 停止中で、コマンドが更新された場合 */
	if (is_command_updated()) {
		update_script_line(get_command_index(), get_updated_command());
		on_load_script();
		on_change_position();
	}

	/* 停止中で、実行中のスクリプトがリロードされた場合 */
	if (is_script_reloaded()) {
		char *scr;
		int line, cmd;
		scr = strdup(get_script_file_name());
		if (scr == NULL) {
			log_memory();
			return false;
		}
		if (strcmp(scr, "DEBUG") == 0) {
			free(scr);
			return false;
		}

		/* 現在実行中の行番号を取得する */
		line = get_line_num();

		/* 同じファイルを再度読み込みする */
		if (!load_script(scr)) {
			free(scr);
			/* エラー時の仮スクリプトを読み込む */
			if (!load_debug_script())
				return false;
		} else {
			free(scr);
		}

		/* 元の行番号の最寄りコマンドを取得する */
		cmd = get_command_index_from_line_num(line);
		if (cmd == -1) {
			/* 削除された末尾の場合、最終コマンドにする */
			cmd = get_command_count() - 1;
		}

		/* ジャンプする */
		move_to_command_index(cmd);
	}
#endif

	/* 停止中で、続けるが押された場合 */
	if (is_continue_pushed()) {
		dbg_running = true;
		on_change_running_state(true, false);

		/* コマンドディスパッチへ進む */
		return true;
	}

	/* 次停止中で、へが押された場合 */
	if (is_next_pushed()) {
		dbg_running = true;
		dbg_request_stop = true;
		on_change_running_state(true, true);

		/* コマンドディスパッチへ進む */
		return true;
	}

	/* 続けるか次へが押されるまでコマンドディスパッチへ進まない */
	return false;
}
#endif

/* コマンドをディスパッチする */
static bool dispatch_command(bool *cont)
{
	const char *locale;
	int command_type;

	/* 次のコマンドを同じフレーム内で実行するか */
	*cont = false;

	/* 国際化プレフィクスをチェックする */
	if (!is_in_command_repetition()) {
		/* ロケールが指定されている場合 */
		locale = get_command_locale();

		/* +en+コマンドの位置を記録する */
		if (strcmp(locale, "en") == 0)
			set_last_en_command();

		/* ロケールが一致しない場合は実行しない */
		if (strcmp(locale, "") != 0) {
			if (strcmp(locale, conf_locale_mapped) != 0) {
				/* 実行しない */
				*cont = true;
				if (!move_to_next_command())
					return false;
				return true;
			}
		}
	}

	/* コマンドをディスパッチする */
	command_type = get_command_type();
	switch (command_type) {
	case COMMAND_LABEL:
		if (!move_to_next_command())
			return false;
		*cont = true;
		break;
	case COMMAND_MESSAGE:
	case COMMAND_SERIF:
		if (!message_command(cont))
			return false;
		break;
	case COMMAND_BG:
		if (!bg_command())
			return false;
		break;
	case COMMAND_BGM:
		if (!bgm_command())
			return false;
		*cont = true;
		break;
	case COMMAND_CH:
		if (!ch_command())
			return false;
		break;
	case COMMAND_CLICK:
		if (!click_command())
			return false;
		break;
	case COMMAND_WAIT:
		if (!wait_command())
			return false;
		break;
	case COMMAND_GOTO:
	case COMMAND_LABELEDGOTO:
		if (!goto_command(cont))
			return false;
		break;
	case COMMAND_LOAD:
		if (!load_command())
			return false;
		*cont = true;
		break;
	case COMMAND_VOL:
		if (!vol_command())
			return false;
		*cont = true;
		break;
	case COMMAND_SET:
		if (!set_command())
			return false;
		*cont = true;
		break;
	case COMMAND_IF:
	case COMMAND_UNLESS:
		if (!if_command())
			return false;
		*cont = true;
		break;
	case COMMAND_SE:
		if (!se_command())
			return false;
		*cont = true;
		break;
	case COMMAND_CHOOSE:
	case COMMAND_ICHOOSE:
	case COMMAND_MCHOOSE:
	case COMMAND_MICHOOSE:
	case COMMAND_SWITCH:	/* deprecated */
	case COMMAND_NEWS:	/* deprecated */
		if (!switch_command())
			return false;
		break;
	case COMMAND_GOSUB:
		if (!gosub_command())
			return false;
		*cont = true;
		break;
	case COMMAND_RETURN:
		if (!return_command())
			return false;
		*cont = true;
		break;
	case COMMAND_CHA:
		if (!cha_command())
			return false;
		break;
	case COMMAND_SHAKE:
		if (!shake_command())
			return false;
		break;
	case COMMAND_SETSAVE:
		if (!setsave_command())
			return false;
		*cont = true;
		break;
	case COMMAND_CHS:
	case COMMAND_CHSX:
		if (!chs_command())
			return false;
		break;
	case COMMAND_VIDEO:
		if (!video_command())
			return false;
		break;
	case COMMAND_SKIP:
		if (!skip_command())
			return false;
		*cont = true;
		break;
	case COMMAND_CHAPTER:
		if (!chapter_command())
			return false;
		*cont = true;
		break;
	case COMMAND_GUI:
		if (!gui_command())
			return false;
		break;
	case COMMAND_WMS:
		if (!wms_command())
			return false;
		*cont = true;
		break;
	case COMMAND_ANIME:
		if (!anime_command())
			return false;
		break;
	case COMMAND_SETCONFIG:
		if (!setconfig_command())
			return false;
		*cont = true;
		break;
	case COMMAND_PENCIL:
		if (!pencil_command())
			return false;
		break;
	case COMMAND_LAYER:
		if (!layer_command())
			return false;
		*cont = true;
		break;
	case COMMAND_CIEL:
		if (!ciel_command(cont))
			return false;
		break;
	case COMMAND_NULL:
		if (!move_to_next_command())
			return false;
		*cont = true;
		break;
	default:
		/* コマンドに対応するcaseを追加し忘れている */
		assert(COMMAND_DISPATCH_NOT_IMPLEMENTED);
		break;
	}

	return true;
}

/*
 * 入力状態をクリアする
 */
void clear_input_state(void)
{
	is_left_button_pressed = false;
	is_right_button_pressed = false;
	is_left_clicked = false;
	is_right_clicked = false;
	is_return_pressed = false;
	is_up_pressed = false;
	is_down_pressed = false;
	is_left_arrow_pressed = false;
	is_right_arrow_pressed = false;
	is_escape_pressed = false;
}

/*
 * ゲームループの終了処理を実行する
 */
void cleanup_game_loop(void)
{
}

/*
 * 複数のイテレーションに渡るコマンドの実行を開始する
 */
void start_command_repetition(void)
{
	assert(!is_in_repetition);
	is_in_repetition = true;
}

/*
 * 複数のイテレーションに渡るコマンドの実行を終了する
 */
void stop_command_repetition(void)
{
	assert(is_in_repetition);
	is_in_repetition = false;
}

/*
 * 複数のイテレーションに渡るコマンドの実行中であるかを返す
 */
bool is_in_command_repetition(void)
{
	return is_in_repetition;
}

/*
 * メッセージの表示中状態を設定する
 *  - メッセージ表示中にシステムGUIに移行しても保持される
 *  - メッセージコマンドから次のコマンドに進むときにクリアされる
 *  - ロードされてもクリアされる
 */
void set_message_active(void)
{
	assert(!flag_message_active);
	flag_message_active = true;
}

/*
 * メッセージの表示中状態を解除する
 */
void clear_message_active(void)
{
	assert(flag_message_active);
	flag_message_active = false;
}

/*
 * メッセージが表示中状態であるかを返す
 */
bool is_message_active(void)
{
	return flag_message_active;
}

/*
 * オートモードを開始する
 */
void start_auto_mode(void)
{
	assert(!flag_auto_mode);
	assert(!flag_skip_mode);
	flag_auto_mode = true;
}

/*
 * オートモードを終了する
 */
void stop_auto_mode(void)
{
	assert(flag_auto_mode);
	assert(!flag_skip_mode);
	flag_auto_mode = false;
}

/*
 * オートモードであるか調べる
 */
bool is_auto_mode(void)
{
	return flag_auto_mode;
}

/*
 * スキップモードを開始する
 */
void start_skip_mode(void)
{
	assert(!flag_skip_mode);
	assert(!flag_auto_mode);
	flag_skip_mode = true;
}

/*
 * スキップモードを終了する
 */
void stop_skip_mode(void)
{
	assert(flag_skip_mode);
	assert(!flag_auto_mode);
	flag_skip_mode = false;
}

/*
 * スキップモードであるか調べる
 */
bool is_skip_mode(void)
{
	return flag_skip_mode;
}

/*
 * セーブ・ロード画面の許可を設定する
 */
void set_save_load(bool enable)
{
	flag_save_load_enabled = enable;
}

/*
 * セーブ・ロード画面の許可を取得する
 */
bool is_save_load_enabled(void)
{
	return flag_save_load_enabled;
}

/*
 * 割り込み不可モードを設定する
 */
void set_non_interruptible(bool mode)
{
	flag_non_interruptible = mode;
}

/*
 * 割り込み不可モードを取得する
 */
bool is_non_interruptible(void)
{
	return flag_non_interruptible;
}

/*
 * ペンの位置を設定する
 */
void set_pen_position(int x, int y)
{
	saved_pen_x = x;
	saved_pen_y = y;
}

/*
 * ペンのX座標を取得する
 */
int get_pen_position_x(void)
{
	return saved_pen_x;
}

/*
 * ペンのX座標を取得する
 */
int get_pen_position_y(void)
{
	return saved_pen_y;
}

/*
 * For Suika2 Pro
 */
#ifdef USE_EDITOR

/*
 * デバッガの実行状態を停止にする
 */
void dbg_stop(void)
{
	dbg_running = false;
	dbg_request_stop = false;

	/* デバッグウィンドウの状態を変更する */
	on_change_running_state(false, false);
}

/*
 * デバッガで停止がリクエストされたか取得する
 */
bool dbg_is_stop_requested(void)
{
	return dbg_request_stop;
}

/*
 * エラー状態を設定する
 */
void dbg_raise_runtime_error(void)
{
	dbg_runtime_error = true;
}

/*
 * エラーカウントをリセットする
 */
void dbg_reset_parse_error_count(void)
{
	dbg_error_count = 0;
}

/*
 * エラーカウントをインクリメントする
 */
void dbg_increment_parse_error_count(void)
{
	dbg_error_count++;
}

/*
 * エラーカウントを取得する
 */
int dbg_get_parse_error_count(void)
{
	return dbg_error_count;
}

#endif

/*
 * License String
 */

const char license_info[]
#ifdef __GNUC__
 /* Don't remove this string even if it's not referenced. */
 __attribute__((used))
#endif
 = "Suika2: Copyright (C) 2001-2023, Keiichi Tabata. All rights reserved.\n"
   "zlib: Copyright (C) 1995-2013 Jean-loup Gailly and Mark Adler. All rights reserved.\n"
   "libpng: Copyright (C) 2000-2002, 2004, 2006-2016, Glenn Randers-Pehrson and the original authors. All rights reserved.\n"
   "jpeg: copyright (C) 1991-2022, Thomas G. Lane, Guido Vollbeding. All rights reserved.\n"
   "bzip2: Copyright (C) 1996-2010 Julian R Seward. All rights reserved.\n"
   "libwebp: Copyright (C) 2010, Google Inc. All rights reserved.\n"
   "libogg: Copyright (C) 2002, Xiph.org Foundation. All rights reserved.\n"
   "libvorbis: Copyright (C) 2002-2015, Xiph.org Foundation. All rights reserved.\n"
   "brotli: Copyright (c) 2009, 2010, 2013-2016 by the Brotli Authors. All rights reserved.\n"
   "FreeType2: Copyright (C) 1996-2002, 2006 by David Turner, Robert Wilhelm, and Werner Lemberg. All rights reserved.\n";
