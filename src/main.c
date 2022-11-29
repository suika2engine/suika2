/* -*- coding: utf-8; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Suika 2
 * Copyright (C) 2001-2022, TABATA Keiichi. All rights reserved.
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
bool is_return_pressed;
bool is_space_pressed;
bool is_escape_pressed;
bool is_up_pressed;
bool is_down_pressed;
bool is_page_up_pressed;
bool is_page_down_pressed;
bool is_control_pressed;
int mouse_pos_x;
int mouse_pos_y;
bool is_mouse_dragging;

/* 複数のイテレーションに渡るコマンドの実行中であるか */
static bool is_in_repetition;

/* 現在表示中のメッセージがヒストリに登録済みであるか */
static bool flag_message_registered;

/* menuコマンドが完了したばかりであるか */
static bool flag_menu_finished;

/* retrospectコマンドが完了したばかりであるか */
static bool flag_retrospect_finished;

/* オートモードが実行中であるか */
static bool flag_auto_mode;

/* スキップモードが実行中であるか */
static bool flag_skip_mode;

/* セーブ・ロード画面が許可されているか */
static bool flag_save_load_enabled = true;

/* 割り込み不可モードであるか */
static bool flag_non_interruptible;

/* 前方参照 */
static bool dispatch_command(int *x, int *y, int *w, int *h, bool *cont);

#ifdef USE_DEBUGGER
/* 実行中であるか */
static bool dbg_running;

/* 停止が要求されているか */
static bool dbg_request_stop;

/* エラー状態であるか */
static bool dbg_error_state;

/* 前方参照 */
static bool pre_dispatch(void);
#endif

/*
 * ゲームループの初期化処理を実行する
 */
void init_game_loop(void)
{
	/* Android NDK用に変数を初期化する */
	is_left_button_pressed = false;
	is_right_button_pressed = false;
	is_return_pressed = false;
	is_space_pressed = false;
	is_escape_pressed = false;
	is_up_pressed = false;
	is_down_pressed = false;
	is_page_up_pressed = false;
	is_page_down_pressed = false;
	is_control_pressed = false;
	mouse_pos_x = 0;
	mouse_pos_y = 0;
	is_mouse_dragging = false;
	is_in_repetition = false;
	flag_message_registered = false;
	flag_menu_finished = false;
	flag_retrospect_finished = false;
	flag_auto_mode = false;
	flag_save_load_enabled = true;
	flag_non_interruptible = false;

	/* Android NDK用に状態を初期化する */
	check_menu_finish_flag();
	check_retrospect_finish_flag();

#ifdef USE_DEBUGGER
	dbg_running = false;
	update_debug_info(true);
#endif
}

/*
 * ゲームループの中身を実行する
 */
bool game_loop_iter(int *x, int *y, int *w, int *h)
{
	bool cont;

	if (is_gui_mode()) {
		/* GUIモードを実行する */
		if (!run_gui_mode(x, y, w, h))
			return false; /* エラー */

		/* GUIモードが終了した場合 */
		if (!is_gui_mode()) {
			if (!is_in_repetition) {
				/* ロードされたときのために終了処理を行う */
				cleanup_gui();
			} else {
				/* @guiを終了する */
				if (!gui_command(x, y, w, h))
					return false; /* エラー */
			}
		}
	} else {
		/* コマンドを実行する */
		do {
#ifdef USE_DEBUGGER
			/* 実行中になるまでディスパッチに進めない */
			if (!pre_dispatch()) {
				draw_stage_keep();
				break;
			}
#endif

			if (!dispatch_command(x, y, w, h, &cont)) {
#ifdef USE_DEBUGGER
				if (dbg_error_state) {
					/* エラーによる終了をキャンセルする */
					dbg_error_state = false;
					dbg_stop();
					return true;
				} else {
					/* 最後まで実行した */
					if (!load_debug_script())
						return false;
					dbg_stop();
					return true;
				}
				return false;
#else
				return false;
#endif
			}
		} while (cont);
	}

	/* サウンドのフェード処理を実行する */
	process_sound_fading();

	/*
	 * 入力の状態をリセットする
	 *  - Controlキー押下とドラッグ状態以外は1フレームごとにリセットする
	 */
	is_left_button_pressed = false;
	is_right_button_pressed = false;
	is_space_pressed = false;
	is_return_pressed = false;
	is_escape_pressed = false;
	is_up_pressed = false;
	is_down_pressed = false;

	return true;
}

#ifdef USE_DEBUGGER
/* デバッガ用のコマンドディスパッチの前処理 */
static bool pre_dispatch(void)
{
	char *scr;
	int line, cmd;

	/* 実行中の場合 */
	if (dbg_running) {
		/* 停止が押された場合 */
		if (is_pause_pushed()) {
			dbg_request_stop = true;
			set_running_state(true, true);
		}

		/* コマンドディスパッチへ進む */
		return true;
	}

	/*
	 * 停止中の場合
	 */

	/* 続けるが押された場合 */
	if (is_resume_pushed()) {
		dbg_running = true;
		set_running_state(true, false);

		/* コマンドディスパッチへ進む */
		return true;
	}

	/* 次へが押された場合 */
	if (is_next_pushed()) {
		dbg_running = true;
		dbg_request_stop = true;
		set_running_state(true, true);

		/* コマンドディスパッチへ進む */
		return true;
	}

	/* 実行するスクリプトが変更された場合 */
	if (is_script_changed()) {
		scr = strdup(get_changed_script());
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
	if (is_line_changed()) {
		int index = get_command_index_from_line_number(
			get_changed_line());
		if (index != -1)
			move_to_command_index(index);
	}

	/* コマンドが更新された場合 */
	if (is_command_updated()) {
		update_command(get_command_index(),
			       get_updated_command());
		update_debug_info(true);
		return false;
	}

	/* 実行中のスクリプトがリロードされば場合 */
	if (is_script_reloaded()) {
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
		cmd = get_command_index_from_line_number(line);
		if (cmd == -1) {
			/* 削除された末尾の場合、最終コマンドにする */
			cmd = get_command_count() - 1;
		}

		/* ジャンプする */
		move_to_command_index(cmd);
	}

	/* 続けるか次へが押されるまでコマンドディスパッチへ進まない */
	return false;
}
#endif

/* コマンドをディスパッチする */
static bool dispatch_command(int *x, int *y, int *w, int *h, bool *cont)
{
	const char *locale;

	/* 次のコマンドを同じフレーム内で実行するか */
	*cont = false;

	/* 国際化プレフィクスをチェックする */
	if (!is_in_command_repetition()) {
		/* ロケールが指定されている場合 */
		locale = get_command_locale();
		if (strcmp(locale, "") != 0) {
			/* ロケールが一致しない場合 */
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
	switch (get_command_type()) {
	case COMMAND_LABEL:
		*cont = true;
		if (!move_to_next_command())
			return false;
		break;
	case COMMAND_MESSAGE:
	case COMMAND_SERIF:
		if (!message_command(x, y, w, h))
			return false;
		break;
	case COMMAND_BG:
		if (!bg_command(x, y, w, h))
			return false;
		break;
	case COMMAND_BGM:
		if (!bgm_command())
			return false;
		*cont = true;
		break;
	case COMMAND_CH:
		if (!ch_command(x, y, w, h))
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
		if (!if_command())
			return false;
		*cont = true;
		break;
	case COMMAND_SE:
		if (!se_command())
			return false;
		*cont = true;
		break;
	case COMMAND_MENU:
		if (!menu_command(x, y, w, h))
			return false;
		break;
	case COMMAND_RETROSPECT:
		if (!retrospect_command(x, y, w, h))
			return false;
		break;
	case COMMAND_SWITCH:
	case COMMAND_NEWS:
	case COMMAND_CHOOSE:
	case COMMAND_SELECT:
		if (!switch_command(x, y, w, h))
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
		if (!cha_command(x, y, w, h))
			return false;
		break;
	case COMMAND_SHAKE:
		if (!shake_command(x, y, w, h))
			return false;
		break;
	case COMMAND_SETSAVE:
		if (!setsave_command())
			return false;
		*cont = true;
		break;
	case COMMAND_CHS:
		if (!chs_command(x, y, w, h))
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
		if (!gui_command(x, y, w, h))
			return false;
		break;
	default:
		/* コマンドに対応するcaseを追加し忘れている */
		assert(COMMAND_DISPATCH_NOT_IMPLEMENTED);
		break;
	}

#ifdef USE_DEBUGGER
	if (*cont) {
		draw_stage_keep();
		*cont = false;
	}
#endif

	return true;
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
 * 現在表示中のメッセージがヒストリに登録済みであることを設定する
 */
void set_message_registered(void)
{
	assert(!flag_message_registered);
	flag_message_registered = true;
}

/*
 * 表示中のメッセージがなくなったことを設定する
 */
void clear_message_registered(void)
{
	assert(flag_message_registered);
	flag_message_registered = false;
}

/*
 * 現在表示中のメッセージがヒストリに登録済みであるかを返す
 */
bool is_message_registered(void)
{
	return flag_message_registered;
}

/*
 * メニューコマンドが完了したばかりであることを記憶する
 */
void set_menu_finish_flag(void)
{
	flag_menu_finished = true;
}

/*
 * メニューコマンドが完了したばかりであるかをチェックする
 */
bool check_menu_finish_flag(void)
{
	bool ret;

	ret = flag_menu_finished;
	flag_menu_finished = false;

	return ret;
}

/*
 * 回想コマンドが完了したばかりであることを記憶する
 */
void set_retrospect_finish_flag(void)
{
	flag_retrospect_finished = true;
}

/*
 * 回想コマンドが完了したばかりであるかをチェックする
 */
bool check_retrospect_finish_flag(void)
{
	bool ret;

	ret = flag_retrospect_finished;
	flag_retrospect_finished = false;

	return ret;
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

#ifdef USE_DEBUGGER
/*
 * デバッガの実行状態を取得する
 */
bool dbg_is_running(void)
{
	return dbg_running;
}

/*
 * デバッガの実行状態を停止にする
 */
void dbg_stop(void)
{
	dbg_running = false;
	dbg_request_stop = false;

	/* デバッグウィンドウの状態を変更する */
	set_running_state(false, false);
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
void dbg_set_error_state(void)
{
	dbg_error_state = true;
}
#endif
