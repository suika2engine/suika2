/* -*- coding: utf-8-with-signature; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Suika 2
 * Copyright (C) 2001-2019, TABATA Keiichi. All rights reserved.
 */

/*
 * ゲームループのメインモジュール
 *
 * [Changes]
 *  - 2016/05/27 作成
 *  - 2017/08/13 スイッチに対応
 *  - 2018/07/21 gosubに対応
 *  - 2019/09/17 NEWSに対応
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

/*
 * 複数のイテレーションに渡るコマンドの実行中であるか
 */
static bool is_in_repetition;

/*
 * 現在表示中のメッセージがヒストリに登録済みであるか
 */
static bool flag_message_registered;

/*
 * 前方参照
 */
static bool dispatch_command(int *x, int *y, int *w, int *h);

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

	/* Android NDK用に状態を初期化する */
	check_menu_finish_flag();
	check_retrospect_finish_flag();
	check_load_flag();
	check_restore_flag();
}

/*
 * ゲームループの中身を実行する
 */
bool game_loop_iter(int *x, int *y, int *w, int *h)
{
	if (is_save_mode()) {
		/* セーブ画面を実行する */
		if (!run_save_mode(x, y, w, h))
			return false; /* 終了ボタンが押下された */
	} else if (is_history_mode()) {
		/* ヒストリ画面を実行する */
		run_history_mode(x, y, w, h);
	} else {
		/* コマンドを実行する */
		if (!dispatch_command(x, y, w, h))
			return false;
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

/*
 * コマンドをディスパッチする
 */
static bool dispatch_command(int *x, int *y, int *w, int *h)
{
	/* ラベルをスキップする */
	while (get_command_type() == COMMAND_LABEL) {
		if (!move_to_next_command())
			return false;
	}
	
	/* コマンドをディスパッチする */
	switch (get_command_type()) {
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
		if (!goto_command())
			return false;
		break;
	case COMMAND_LOAD:
		if (!load_command())
			return false;
		break;
	case COMMAND_VOL:
		if (!vol_command())
			return false;
		break;
	case COMMAND_SET:
		if (!set_command())
			return false;
		break;
	case COMMAND_IF:
		if (!if_command())
			return false;
		break;
	case COMMAND_SELECT:
		if (!select_command(x, y, w, h))
			return false;
		break;
	case COMMAND_SE:
		if (!se_command())
			return false;
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
		break;
	case COMMAND_RETURN:
		if (!return_command())
			return false;
		break;
	default:
		/* コマンドに対応するcaseを追加し忘れている */
		assert(COMMAND_DISPATCH_NOT_IMPLEMENTED);
		break;
	}

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
