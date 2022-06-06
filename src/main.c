/* -*- coding: utf-8-with-signature; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Suika 2
 * Copyright (C) 2001-2021, TABATA Keiichi. All rights reserved.
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
	check_load_flag();
	check_restore_flag();

#ifdef USE_DEBUGGER
	dbg_running = false;
	update_debug_info();
#endif
}

/*
 * ゲームループの中身を実行する
 */
bool game_loop_iter(int *x, int *y, int *w, int *h)
{
	bool cont;

	if (is_save_load_mode()) {
		/* セーブ画面を実行する */
		if (!run_save_load_mode(x, y, w, h))
			return false; /* 終了ボタンが押下された */
	} else if (is_history_mode()) {
		/* ヒストリ画面を実行する */
		run_history_mode(x, y, w, h);
	} else {
		/* コマンドを実行する */
		do {
#ifdef USE_DEBUGGER
			/* 実行中になるまでディスパッチに進めない */
			if (!pre_dispatch())
				break;
#endif
			if (!dispatch_command(x, y, w, h, &cont))
				return false;
		} while (cont);
	}

	/* サウンドのフェード処理を実行する */
	process_sound_fading();

	/*
	 * 入力の状態をリセットする
	 *  - Control, Space以外は1フレームごとにリセットする
	 */
	is_left_button_pressed = false;
	is_right_button_pressed = false;
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
	if (!dbg_running) {
		/* 停止中の場合 */
		if (is_resume_pushed()) {
			/* 続けるが押された場合 */
			dbg_running = true;
			set_running_state(true, false);

			/* コマンドディスパッチへ進む */
			return true;
		} else if (is_next_pushed()) {
			/* 次へが押された場合 */
			dbg_running = true;
			dbg_request_stop = true;
			set_running_state(true, true);

			/* コマンドディスパッチへ進む */
			return true;
		} else if (is_script_changed()) {
			/* 実行するスクリプトが変更された場合 */
			const char *script = get_changed_script();
			struct rfile *rf = open_rfile(SCRIPT_DIR, script, false);
			if (rf == NULL)
				return false;
			else if (!load_script(script))
				exit(1); /* 救済措置はない */
			close_rfile(rf);
		} else if (is_line_changed()) {
			/* 行番号が変更された場合 */
			int index = get_command_index_from_line_number(
				get_changed_line());
			if (index != -1)
				move_to_command_index(index);
		}

		/* 続けるか次へが押されるまでコマンドディスパッチへ進まない */
		return false;
	} else {
		/* 実行中の場合 */
		if (is_pause_pushed()) {
			/* 停止が押された場合 */
			dbg_request_stop = true;
			set_running_state(false, true);
		}
	}

	/* コマンドディスパッチへ進む */
	return true;
}
#endif

/* コマンドをディスパッチする */
static bool dispatch_command(int *x, int *y, int *w, int *h, bool *cont)
{
	/* 次のコマンドを同じフレーム内で実行するか */
	*cont = false;

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
	case COMMAND_SELECT:
		if (!select_command(x, y, w, h))
			return false;
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
	default:
		/* コマンドに対応するcaseを追加し忘れている */
		assert(COMMAND_DISPATCH_NOT_IMPLEMENTED);
		break;
	}

#ifdef USE_DEBUGGER
	*cont = false;
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
#endif
