/* -*- coding: utf-8; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Suika 2
 * Copyright (C) 2001-2023, TABATA Keiichi. All rights reserved.
 */

/*
 * GUI
 *
 * [Changes]
 *  - 2022/07/27 作成
 *  - 2023/06/11 名前変数の編集に対応
 */

#include "suika.h"

/* ボタンの最大数 */
#define BUTTON_COUNT	(128)

/* ボタンタイプ */
enum {
	/* 無効なボタン */
	TYPE_INVALID = 0,

	/* ラベルにジャンプするボタン */
	TYPE_GOTO,

	/* ギャラリーモードのボタン */
	TYPE_GALLERY,

	/* ボリュームを設定するボタン */
	TYPE_BGMVOL,
	TYPE_VOICEVOL,
	TYPE_SEVOL,
	TYPE_CHARACTERVOL,

	/* テキストスピードを設定するボタン */
	TYPE_TEXTSPEED,

	/* オートスピードを設定するボタン */
	TYPE_AUTOSPEED,

	/* テキストプレビューを表示するボタン */
	TYPE_PREVIEW,

	/* フルスクリーンモードにするボタン */
	TYPE_FULLSCREEN,

	/* ウィンドウモードにするボタン */
	TYPE_WINDOW,

	/* フォントを変更するボタン */
	TYPE_FONT,

	/* デフォルト値に戻すボタン */
	TYPE_DEFAULT,

	/* 別のGUIに移動するボタン */
	TYPE_GUI,

	/* セーブページボタン */
	TYPE_SAVEPAGE,

	/* セーブボタン */
	TYPE_SAVE,

	/* ロードボタン */
	TYPE_LOAD,

	/* ヒストリボタン */
	TYPE_HISTORY,

	/* ヒストリスクロールボタン */
	TYPE_HISTORYSCROLL,

	/* タイトルに戻るボタン */
	TYPE_TITLE,

	/* GUIを終了するボタン */
	TYPE_CANCEL,

	/* アプリケーションを終了するボタン */
	TYPE_QUIT,

	/* 名前変数のプレビューボタン */
	TYPE_NAMEVAR,

	/* 名前変数に文字列を追加するボタン */
	TYPE_CHAR,
};

/* ボタン */
static struct gui_button {
	/*
	 * GUIファイルから設定されるプロパティ
	 */

	/* ボタンタイプ */
	int type;

	/* 座標とサイズ */
	int x;
	int y;
	int width;
	int height;

	/* TYPE_SAVE/TYPE_LOAD */
	int margin;

	/* TYPE_GOTO, TYPE_GALLERY */
	char *label;

	/* TYPE_BGMVOL, TYPE_VOICEVOL, TYPE_SEVOL, TYPE_GUI, TYPE_FONT */
	char *file;

	/*
	 * TYPE_SAVEPAGE, TYPE_LOADPAGE, TYPE_SAVE, TYPE_LOAD,
	 * TYPE_CHARACTERVOL
	 */
	int index;

	/* TYPE_PREVIEW, TYPE_CHAR */
	char *msg;

	/* TYPE_NAMEVAR, TYPE_CHAR */
	int namevar;

	/* TYPE_CHAR */
	int max;

	/* TYPE_VOLUME以外 */
	char *clickse;

	/* すべて */
	char *pointse;

	/*
	 * 実行時の情報
	 */
	struct {
		/* TYPE_FONT, TYPE_FULLSCREEN, TYPE_WINDOWのときアクティブか */
		bool is_active;

		/* TYPE_GALLERYのときボタンが無効化されているか */
		bool is_disabled;

		/* 前のフレームでポイントされていたか */
		bool is_pointed;

		/* ドラッグ中か */
		bool is_dragging;

		/* ボリューム・スピードのスライダーの値 */
		float slider;

		/* サムネイル、フォント描画用 */
		struct image *img;

		/* ヒストリのオフセット */
		int history_offset;

		/*
		 * テキストプレビューの情報
		 */

		/* オートモードの待ち時間か */
		bool is_waiting;

		/* まだ描画されていないメッセージの先頭 */
		const char *top;

		/* 色 */
		pixel_t color, outline_color;

		/* 描画する文字の総数 */
		int total_chars;

		/* 前回までに描画した文字数 */
		int drawn_chars;

		/* 描画位置 */
		int pen_x, pen_y;
		int pen_ruby_x, pen_ruby_y;

		/* スペースの直後か(ワードラッピング用) */
		bool is_after_space;

		/* メッセージ表示用あるいはオートモード待機用の時計 */
		stop_watch_t sw;
	} rt;

} button[BUTTON_COUNT];

/* GUIモードであるか */
static bool flag_gui_mode;

/* 最初のフレームであるか */
static bool is_first_frame;

/* メッセージ・スイッチの最中に呼ばれたか */
static bool is_called_from_command;

/* 右クリックでキャンセルするか */
static bool cancel_when_right_click;

/* 処理中のGUIファイル */
static const char *gui_file;

/* キャンセル時のSE */
static char *cancel_se;

/* ポイントされているボタンのインデックス */
static int pointed_index;

/* ポイントされているボタンが変化したか */
static bool is_pointed_changed;

/* 選択結果のボタンのインデックス */
static int result_index;

/* ドラッグ中のテキストスピード */
static float transient_text_speed;

/* ドラッグ中のオートモードスピード */
static float transient_auto_speed;

/* セーブ・ロードページのページ番号 */
static int save_page;

/* セーブ・ロードページの1ページあたりのスロット数 */
static int save_slots;

/* このフレームでセーブされたか */
static bool is_saved_in_this_frame;

/* SEの再生を控えるフレームであるか */
static bool suppress_se;

/* ヒストリの1ページあたりのスロット数 */
static int history_slots;

/* 表示する先頭のヒストリ番号 */
static int history_top;

/* ドラッグ中のヒストリのスライダー値 */
static float transient_history_slider;

/* ヒストリボタンの更新が必要か */
static bool need_update_history_buttons;

/*
 * 前方参照
 */
static bool add_button(int index);
static bool set_global_key_value(const char *key, const char *val);
static bool set_button_key_value(const int index, const char *key,
				 const char *val);
static int get_type_for_name(const char *name);
static void update_runtime_props(bool is_first_time);
static bool move_to_other_gui(void);
static bool move_to_title(void);
static void process_button_point(int index);
static void process_button_drag(int index);
static float calc_slider_value(int index);
static void process_button_click(int index);
static void process_button_draw(int index);
static void process_button_draw_slider(int index);
static void process_button_draw_slider_vertical(int index);
static void process_button_draw_activatable(int index);
static void process_button_draw_gallery(int index);
static void process_button_draw_namevar(int index);
static void process_play_se(void);
static bool init_save_buttons(void);
static void update_save_buttons(void);
static void draw_save_button(int button_index);
static int draw_save_text_item(int button_index, int x, int y,
			       const char *text);
static void draw_save_char(int index, uint32_t wc, int *width, int *height);
static void process_save(int button_index);
static void process_load(int button_index);
static void process_button_draw_save(int button_index);
static bool init_history_buttons(void);
static void update_history_buttons(void);
static void draw_history_button(int button_index);
static void draw_history_text_item(int button_index);
static void draw_history_char(int index, uint32_t wc, int *width, int *height);
static void process_escape_sequence(int button_index, bool ignore_lf,
				    bool ignore_ruby, int ruby_size);
static bool process_escape_sequence_lf(int button_index, bool ignore_lf);
static bool process_escape_sequence_color(int button_index);
static bool process_escape_sequence_ruby(int button_index, bool ignore_ruby,
					 int ruby_size);
static void draw_ruby_char(int button_index, uint32_t wc, int *width,
			   int *height);
static bool process_escape_sequence_other(int button_index);
static void process_button_draw_history(int button_index);
static void process_history_scroll(int delta);
static void update_history_top(int button_index);
static void process_history_voice(int button_index);
static bool init_preview_buttons(void);
static void reset_preview_all_buttons(void);
static void reset_preview_button(int index);
static void process_button_draw_preview(int index);
static void draw_message(int index);
static void draw_message_char(int index, uint32_t wc, int *width, int *height);
static int get_frame_chars(int index);
static void word_wrapping(int index);
static int get_en_word_width(const char *m);
static int get_wait_time(int index);
static bool init_namevar_buttons(void);
static void update_namevar_buttons(void);
static void draw_name(int index);
static void process_char(int index);
static void play_se(const char *file, bool is_voice);
static bool load_gui_file(const char *file);

/*
 * GUIに関する初期化処理を行う
 */
bool init_gui(void)
{
	/* Android NDK用に再初期化する */
	cleanup_gui();
	return true;
}

/*
 * GUIに関する終了処理を行う
 */
void cleanup_gui(void)
{
	int i;

	/* フラグを再初期化する */
	flag_gui_mode = false;

	/* ボタンのメモリを解放する */
	for (i = 0; i < BUTTON_COUNT; i++) {
		if (button[i].label != NULL)
			free(button[i].label);
		if (button[i].file != NULL)
			free(button[i].file);
		if (button[i].msg != NULL)
			free(button[i].msg);
		if (button[i].clickse != NULL)
			free(button[i].clickse);
		if (button[i].pointse != NULL)
			free(button[i].pointse);
		if (button[i].rt.img != NULL)
			destroy_image(button[i].rt.img);
	}

	/* ボタンをゼロクリアする */
	memset(button, 0, sizeof(button));

	/* 文字列の解放を行う */
	if (cancel_se != NULL) {
		free(cancel_se);
		cancel_se = NULL;
	}
	
	/* ステージの後処理を行う */
	remove_gui_images();
}

/*
 * GUIから復帰した直後かどうかを確認する
 */
bool check_gui_flag(void)
{
	if (is_called_from_command) {
		is_called_from_command = false;
		return true;
	}
	return false;
}

/*
 * GUIを準備する
 */
bool prepare_gui_mode(const char *file, bool cancel, bool from_command,
		      bool is_thumb_prepared)
{
	assert(!flag_gui_mode);

	/* ボタンをゼロクリアする */
	memset(button, 0, sizeof(button));

	/* 初期値を設定する */
	save_page = 0;
	save_slots = 0;
	history_slots = 0;
	history_top = -1;
	is_saved_in_this_frame = false;
	suppress_se = false;

	/* プロパティを保存する */
	gui_file = file;
	is_called_from_command = from_command;

	/* GUIファイルを開く */
	if (!load_gui_file(gui_file)) {
		cleanup_gui();
		return false;
	}

	/* globalセクションのイメージがすべて揃っているか調べる */
	if (!check_stage_gui_images()) {
		log_gui_image_not_loaded();
		cleanup_gui();
		return false;
	}

	/* TYPE_PREVIEWのボタンの初期化を行う */
	if (!init_preview_buttons()) {
		cleanup_gui();
		return false;
	}

	/* TYPE_SAVE/TYP_LOADのボタンの初期化を行う */
	if (!init_save_buttons()) {
		cleanup_gui();
		return false;
	}
	update_save_buttons();

	/* TYPE_HISTORYのボタンの初期化を行う */
	if (!init_history_buttons()) {
		cleanup_gui();
		return false;
	}
	update_history_buttons();

	/* TYPE_NAMEVARのボタンの初期化を行う */
	if (!init_namevar_buttons()) {
		cleanup_gui();
		return false;
	}
	update_namevar_buttons();

	/* ボタンの状態を準備する */
	update_runtime_props(true);

	/*
	 * セーブされる場合に備えてサムネイルを描画する
	 *  - switch.cから呼ばれた場合はFOレイヤからサムネイルを作成済み
	 */
	if (!is_thumb_prepared)
		draw_stage_to_thumb();

	/* 右クリックでキャンセルするか */
	cancel_when_right_click = cancel;

	return true;
}

/*
 * GUIのロード時
 */

/* グローバルのキーと値を設定する */
static bool set_global_key_value(const char *key, const char *val)
{
	if (strcmp(key, "idle") == 0) {
		if (!load_gui_idle_image(val))
			return false;
		return true;
	} else if (strcmp(key, "hover") == 0) {
		if (!load_gui_hover_image(val))
			return false;
		return true;
	} else if (strcmp(key, "active") == 0) {
		if (!load_gui_active_image(val))
			return false;
		return true;
	} else if (strcmp(key, "saveslots") == 0) {
		save_slots = atoi(val);
		return true;
	} else if (strcmp(key, "historyslots") == 0) {
		history_slots = atoi(val);
		return true;
	} else if (strcmp(key, "cancelse") == 0) {
		cancel_se = strdup(val);
		if (cancel_se == NULL) {
			log_memory();
			return false;
		}
		return true;
	}

	log_gui_unknown_global_key(key);
	return false;
}

/* ボタンを追加する */
static bool add_button(int index)
{
	/* ボタン数の上限を越えている場合 */
	if (index >= BUTTON_COUNT) {
		log_gui_too_many_buttons();
		return false;
	}

	/* 特にボタン追加の処理はなく、数を確認しているだけ */
	return true;
}

/* ボタンのキーを設定する */
static bool set_button_key_value(const int index, const char *key,
				 const char *val)
{

	struct gui_button *b;
	int var_val;

	assert(index >= 0 && index < BUTTON_COUNT);

	b = &button[index];

	/* typeキーを処理する */
	if (strcmp("type", key) == 0) {
		b->type = get_type_for_name(val);
		if (b->type == TYPE_INVALID)
			return false;
		return true;
	}

	/* typeが指定されていない場合 */
	if (b->type == TYPE_INVALID) {
		log_gui_parse_property_before_type(key);
		return false;
	}

	/* type以外のキーを処理する */
	if (strcmp("x", key) == 0) {
		b->x = atoi(val);
	} else if (strcmp("y", key) == 0) {
		b->y = atoi(val);
	} else if (strcmp("width", key) == 0) {
		b->width = atoi(val);
		if (b->width <= 0)
			b->width = 1;
	} else if (strcmp("height", key) == 0) {
		b->height = atoi(val);
		if (b->height <= 0)
			b->height = 1;
	} else if (strcmp("margin", key) == 0) {
		b->margin = atoi(val);
	} else if (strcmp("label", key) == 0) {
		b->label = strdup(val);
		if (b->label == NULL) {
			log_memory();
			return false;
		}
	} else if (strcmp("file", key) == 0) {
		b->file = strdup(val);
		if (b->file == NULL) {
			log_memory();
			return false;
		}
	} else if (strcmp("var", key) == 0) {
		if (!get_variable_by_string(val, &var_val))
			return false;
		if (var_val == 0)
			b->rt.is_disabled = true;
	} else if (strcmp("index", key) == 0) {
		b->index = atoi(val);
		if (b->type == TYPE_CHARACTERVOL) {
			if (b->index < 0)
				b->index = 0;
			if (b->index >= CH_VOL_SLOTS)
				b->index = CH_VOL_SLOTS - 1;
		}
	} else if (strcmp("msg", key) == 0) {
		b->msg = strdup(val);
		if (b->msg == NULL) {
			log_memory();
			return false;
		}
	} else if (strcmp("clickse", key) == 0) {
		b->clickse = strdup(val);
		if (b->clickse == NULL) {
			log_memory();
			return false;
		}
	} else if (strcmp("pointse", key) == 0) {
		b->pointse = strdup(val);
		if (b->pointse == NULL) {
			log_memory();
			return false;
		}
	} else if (strcmp("namevar", key) == 0) {
		if (val[0] != '\0' &&
		    (val[0] >= 'a' && val[0] <= 'z') &&
		    val[1] == '\0')
			b->namevar = val[0] - 'a';
	} else if (strcmp("max", key) == 0) {
		b->max = abs(atoi(val));
	} else {
		log_gui_unknown_button_property(key);
		return false;
	}

	return true;
}

/* タイプ名に対応する整数値を取得する */
static int get_type_for_name(const char *name)
{
	struct {
		const char *name;
		int value;
	} type_array[] = {
		{"goto", TYPE_GOTO},
		{"gallery", TYPE_GALLERY},
		{"bgmvol", TYPE_BGMVOL},
		{"voicevol", TYPE_VOICEVOL},
		{"sevol", TYPE_SEVOL},
		{"charactervol", TYPE_CHARACTERVOL},
		{"textspeed", TYPE_TEXTSPEED},
		{"autospeed", TYPE_AUTOSPEED},
		{"preview", TYPE_PREVIEW},
		{"fullscreen", TYPE_FULLSCREEN},
		{"window", TYPE_WINDOW},
		{"font", TYPE_FONT},
		{"gui", TYPE_GUI},
		{"savepage", TYPE_SAVEPAGE},
		{"save", TYPE_SAVE},
		{"load", TYPE_LOAD},
		{"history", TYPE_HISTORY},
		{"historyscroll", TYPE_HISTORYSCROLL},
		{"default", TYPE_DEFAULT},
		{"title", TYPE_TITLE},
		{"cancel", TYPE_CANCEL},
		{"quit", TYPE_QUIT},
		{"QUIT", TYPE_QUIT}, /* typoだけど互換性のため残した */
		{"namevar", TYPE_NAMEVAR},
		{"char", TYPE_CHAR},
	};
	
	size_t i;

	/* タイプ名を検索する */
	for (i = 0; i < sizeof(type_array) / sizeof(type_array[0]); i++)
		if (strcmp(name, type_array[i].name) == 0)
			return type_array[i].value;

	/* みつからなかった場合 */
	log_gui_unknown_button_type(name);
	return TYPE_INVALID;
}

/*
 * 実行時プロパティの更新
 */

/* ボタンの状態を更新する */
static void update_runtime_props(bool is_first_time)
{
	int i;

	/* 初回呼び出しの場合 */
	if (is_first_time) {
		transient_text_speed = get_text_speed();
		transient_auto_speed = get_auto_speed();
	}

	for (i = 0; i < BUTTON_COUNT; i++) {
		switch (button[i].type) {
		case TYPE_BGMVOL:
			button[i].rt.slider =
				get_mixer_global_volume(BGM_STREAM);
			break;
		case TYPE_VOICEVOL:
			button[i].rt.slider =
				get_mixer_global_volume(VOICE_STREAM);
			break;
		case TYPE_SEVOL:
			button[i].rt.slider =
				get_mixer_global_volume(SE_STREAM);
			break;
		case TYPE_CHARACTERVOL:
			button[i].rt.slider =
				get_character_volume(button[i].index);
			break;
		case TYPE_TEXTSPEED:
			button[i].rt.slider = transient_text_speed;
			break;
		case TYPE_AUTOSPEED:
			button[i].rt.slider = transient_auto_speed;
			break;
		case TYPE_FONT:
			if (button[i].file == NULL)
				break;
			if (strcmp(button[i].file, get_font_file_name()) == 0)
				button[i].rt.is_active = true;
			else
				button[i].rt.is_active = false;
			break;
		case TYPE_FULLSCREEN:
			if (conf_window_fullscreen_disable)
				break;
			if (!is_full_screen_supported())
				break;
			button[i].rt.is_active = is_full_screen_mode();
			break;
		case TYPE_WINDOW:
			if (conf_window_fullscreen_disable)
				break;
			if (!is_full_screen_supported())
				break;
			button[i].rt.is_active = !is_full_screen_mode();
			break;
		case TYPE_SAVEPAGE:
			if (button[i].index == save_page)
				button[i].rt.is_active = true;
			else
				button[i].rt.is_active = false;
			break;
		case TYPE_HISTORYSCROLL:
			button[i].rt.slider = transient_history_slider;
			break;
		default:
			break;
		}
	}
}

/*
 * GUIの実行
 */

/*
 * GUIを開始する
 */
void start_gui_mode(void)
{
	assert(!flag_gui_mode);

	/* GUIモードを有効にする */
	flag_gui_mode = true;
	is_first_frame = true;
}

/*
 * GUIを停止する
 */
void stop_gui_mode(void)
{
	assert(flag_gui_mode);

	/* GUIモードを無効にする */
	flag_gui_mode = false;
}

/*
 * GUIが有効であるかを返す
 */
bool is_gui_mode(void)
{
	return flag_gui_mode;
}

/*
 * GUIを実行する
 */
bool run_gui_mode(int *x, int *y, int *w, int *h)
{
	int i;

	*x = 0;
	*y = 0;
	*w = conf_window_width;
	*h = conf_window_height;

	/* 背景を描画する */
	draw_stage_gui_idle();

	/* 各ボタンについて処理する */
	pointed_index = -1;
	result_index = -1;
	is_pointed_changed = false;
	need_update_history_buttons = false;
	for (i = 0; i < BUTTON_COUNT; i++) {
		/* ポイント状態を更新する */
		process_button_point(i);

		/* ドラッグ状態を更新する */
		process_button_drag(i);

		/* クリック結果を取得する */
		process_button_click(i);

		/* ボタンの状態に合わせて描画する */
		process_button_draw(i);
	}

	/* マウスホイールか上下キーによるスクロールを処理する */
	if (is_up_pressed) {
		process_history_scroll(-1);
		update_runtime_props(false);
	} else if (is_down_pressed) {
		process_history_scroll(1);
		update_runtime_props(false);
	}

	/* ヒストリボタンの更新が必要な場合 */
	if (need_update_history_buttons) {
		update_history_buttons();
		need_update_history_buttons = false;
	}

	/* 右クリックでキャンセル可能な場合 */
	if (cancel_when_right_click) {
		/* 右クリックされた場合か、エスケープキーが押下された場合 */
		if (is_right_clicked || is_escape_pressed) {
			/* どのボタンも選ばれなかったことにする */
			result_index = -1;

			/* SEを再生する */
			play_se(cancel_se, false);

			/* GUIモードを終了する */
			stop_gui_mode();

			/* 続くコマンド実行に影響を与えないようにする */
			clear_input_state();
			return true;
		}			
	}

	/* SEを再生する */
	if (!suppress_se) {
		if (!is_first_frame)
			process_play_se();
		if (is_saved_in_this_frame) {
			suppress_se = true;
			is_saved_in_this_frame = false;
		}
	} else {
		suppress_se = false;
	}

	/* ボタンが決定された場合 */
	if (result_index != -1) {
		/* 他のGUIに移動する場合 */
		if (button[result_index].type == TYPE_GUI)
			return move_to_other_gui();

		/* タイトルへ戻る場合 */
		if (button[result_index].type == TYPE_TITLE)
			return move_to_title();

		/* GUIモードを終了する */
		stop_gui_mode();
		return true;
	}

	is_first_frame = false;
	return true;
}

/* 他のGUIに移動する */
static bool move_to_other_gui(void)
{
	char *file;
	bool from_command;

	from_command = is_called_from_command;

	/* ファイル名をコピーする(cleanup_gui()によって参照不能となるため) */
	file = strdup(button[result_index].file);
	if (file == NULL) {
		log_memory();
		cleanup_gui();
		return false;
	}

	/* GUIを停止する */
	stop_gui_mode();

	/* 現在のGUIを破棄する */
	cleanup_gui();

	/* GUIをロードする */
	if (!prepare_gui_mode(file, cancel_when_right_click, from_command,
			      false)) {
		free(file);
		return false;
	}
	free(file);

	/* 終了後に表示されるBGレイヤを設定する */
	if (!is_called_from_command)
		if (!create_temporary_bg_for_gui())
			return false;

	/* GUIを開始する */
	start_gui_mode();

	return true;
}

/* タイトルに移動する */
static bool move_to_title(void)
{
	const char *file;

	file = button[result_index].file;

	/* ファイルが指定されていない場合 */
	if (file == NULL)
		return true;

	/* スクリプトをロードする */
	if (!load_script(file))
		return false;

	/* @guiコマンドを実行中の場合はキャンセルする */
	if (is_in_command_repetition())
		stop_command_repetition();

	/*
	 * メッセージ・スイッチの最中に呼ばれた場合
	 *  - この場合はシステムGUIから戻ったという扱いにしない
	 */
	if (is_called_from_command)
		is_called_from_command = false;

	/* メッセージをアクティブでなくする */
	if (is_message_active())
		clear_message_active();

	/* GUIを終了する */
	stop_gui_mode();
	cleanup_gui();
	
	return true;
}

/*
 * ボタンのポイント
 */

/* ボタンのポイント状態の変化を処理する */
static void process_button_point(int index)
{
	struct gui_button *b;
	bool prev_pointed_status;

	b = &button[index];
	prev_pointed_status = b->rt.is_pointed;

	/* TYPE_GALLERYのとき、ボタンが無効であればポイントできない */
	if (b->rt.is_disabled)
		return;

	/* TYPE_FULLSCREENのとき、ボタンがアクティブならポイントできない */
	if (b->type == TYPE_FULLSCREEN && b->rt.is_active) {
		b->rt.is_pointed = false;
		return;
	}

	/* TYPE_WINDOWのとき、ボタンがアクティブならポイントできない */
	if (b->type == TYPE_WINDOW && b->rt.is_active) {
		b->rt.is_pointed = false;
		return;
	}

	/* TYPE_HISTORYのとき、ボタンが非アクティブならポイントできない */
	if (b->type == TYPE_HISTORY && !b->rt.is_active) {
		b->rt.is_pointed = false;
		return;
	}

	/* TYPE_PREVIEWのとき、ポイントできない */
	if (b->type == TYPE_PREVIEW)
		return;

	/* TYPE_NAMEVARのとき、ポイントできない */
	if (b->type == TYPE_NAMEVAR)
		return;

	/* マウスがボタン領域に入っているかチェックする */
	if (mouse_pos_x >= b->x && mouse_pos_x <= b->x + b->width &&
	    mouse_pos_y >= b->y && mouse_pos_y <= b->y + b->height) {
		/* ポイントされている */
		b->rt.is_pointed = true;
		pointed_index = index;
	} else {
		/* ポイントされていない */
		b->rt.is_pointed = false;
	}

	/* ポイント状態が変化した場合 */
	if (prev_pointed_status != b->rt.is_pointed)
		is_pointed_changed = true;
}

/*
 * ボタンのドラッグ
 */

/* ボリュームボタンのドラッグを処理する */
static void process_button_drag(int index)
{
	struct gui_button *b;

	b = &button[index];

	/* ドラッグ可能なボタンではない場合 */
	if (b->type != TYPE_BGMVOL && b->type != TYPE_VOICEVOL &&
	    b->type != TYPE_SEVOL && b->type != TYPE_CHARACTERVOL &&
	    b->type != TYPE_TEXTSPEED && b->type != TYPE_AUTOSPEED &&
	    b->type != TYPE_HISTORYSCROLL)
		return;

	/* ドラッグ中でない場合 */
	if (!b->rt.is_dragging) {
		/* ポイントされていない場合 */
		if (!b->rt.is_pointed)
			return;

		/* マウスの左ボタンが押下されていない場合 */
		if (!is_mouse_dragging)
			return;

		/* ドラッグを開始する */
		b->rt.is_dragging = true;
		b->rt.slider = calc_slider_value(index);
		if (b->type == TYPE_BGMVOL)
			set_mixer_global_volume(BGM_STREAM, b->rt.slider);
		if (b->type == TYPE_HISTORYSCROLL) {
			transient_history_slider = b->rt.slider;
			update_history_top(index);
			b->rt.slider = transient_history_slider;
		}
		return;
	}

	/* ドラッグ中の場合 */
	b->rt.slider = calc_slider_value(index);

	/* スライダの量を設定に反映する */
	switch (b->type) {
	case TYPE_BGMVOL:
		set_mixer_global_volume(BGM_STREAM, b->rt.slider);
		break;
	case TYPE_VOICEVOL:
		set_mixer_global_volume(VOICE_STREAM, b->rt.slider);
		break;
	case TYPE_SEVOL:
		set_mixer_global_volume(SE_STREAM, b->rt.slider);
		break;
	case TYPE_CHARACTERVOL:
		set_character_volume(b->index, b->rt.slider);
		break;
	case TYPE_TEXTSPEED:
		transient_text_speed = b->rt.slider;
		break;
	case TYPE_AUTOSPEED:
		transient_auto_speed = b->rt.slider;
		break;
	case TYPE_HISTORYSCROLL:
		transient_history_slider = b->rt.slider;
		update_history_top(index);
		b->rt.slider = transient_history_slider;
		break;
	}

	/* ドラッグを終了する場合 */
	if (!is_mouse_dragging) {
		b->rt.is_dragging = false;

		/* 調節完了後のアクションを実行する */
		switch (b->type) {
		case TYPE_BGMVOL:
			break;
		case TYPE_VOICEVOL:
			/* デフォルトのキャラクター音量でフィードバックする */
			apply_character_volume(CH_VOL_SLOT_DEFAULT);
			play_se(b->file, true);
			break;
		case TYPE_SEVOL:
			/* フィードバックする */
			play_se(b->file, false);
			break;
		case TYPE_CHARACTERVOL:
			/* 指定されたキャラクター音量でフィードバックする */
			apply_character_volume(b->index);
			play_se(b->file, true);
			break;
		case TYPE_TEXTSPEED:
			/* テキストを再表示する */
			set_text_speed(transient_text_speed);
			reset_preview_all_buttons();
			break;
		case TYPE_AUTOSPEED:
			/* テキストを再表示する */
			set_auto_speed(b->rt.slider);
			reset_preview_all_buttons();
			break;
		default:
			break;
		}
	}

	/* 同じタイプのボタンが複数ある場合のために、他のボタンの更新を行う */
	update_runtime_props(false);
}

/* スライダの値を計算する */
static float calc_slider_value(int index)
{
	float val;

	if (button[index].type != TYPE_HISTORYSCROLL) {
		float x1, x2;

		/* スライダの左端を求める */
		x1 = (float)(button[index].x + button[index].height / 2);

		/* スライダの右端を求める */
		x2 = (float)(button[index].x + button[index].width -
			     button[index].height / 2);

		/* ポイントされている座標における値を計算する */
		val = ((float)mouse_pos_x - x1) / (x2 - x1);
	} else {
		float y1, y2;

		/* スライダの上端を求める */
		y1 = (float)(button[index].y + button[index].width / 2);

		/* スライダの下端を求める */
		y2 = (float)(button[index].y + button[index].height -
			     button[index].width / 2);

		/* ポイントされている座標における値を計算する */
		val = ((float)mouse_pos_y - y1) / (y2 - y1);
	}

	/* 0未満のときを処理する */
	if (val < 0)
		val = 0;

	/* 1以上のときを処理する */
	if (val > 1.0f)
		val = 1.0f;

	return val;
}

/*
 * ボタンのクリック
 */

/* ボタンのクリックを処理する */
static void process_button_click(int index)
{
	struct gui_button *b;

	b = &button[index];

	/* クリックできないボタンの場合 */
	if (b->type == TYPE_BGMVOL || b->type == TYPE_VOICEVOL ||
	    b->type == TYPE_SEVOL || b->type == TYPE_CHARACTERVOL ||
	    b->type == TYPE_TEXTSPEED || b->type == TYPE_AUTOSPEED ||
	    b->type == TYPE_PREVIEW || b->type == TYPE_HISTORYSCROLL)
		return;

	/* ボタンがポイントされていない場合 */
	if (!b->rt.is_pointed)
		return;

	/* ボタンがクリックされていない場合 */
	if (!is_left_clicked)
		return;

	/* ボタンのタイプごとにクリックを処理する */
	switch (b->type) {
	case TYPE_FULLSCREEN:
		play_se(b->clickse, false);
		if (!conf_window_fullscreen_disable)
			enter_full_screen_mode();
		update_runtime_props(false);
		break;
	case TYPE_WINDOW:
		play_se(b->clickse, false);
		if (!conf_window_fullscreen_disable)
			leave_full_screen_mode();
		update_runtime_props(false);
		break;
	case TYPE_FONT:
		play_se(b->clickse, false);
		if (b->file != NULL) {
			set_global_font_file_name(b->file);
			if (!init_glyph())
				abort();
		}
		update_runtime_props(false);
		reset_preview_all_buttons();
		break;
	case TYPE_DEFAULT:
		if (default_dialog()) {
			play_se(b->clickse, false);
			set_text_speed(0.5f);
			set_auto_speed(0.5f);
			apply_initial_values();
			update_runtime_props(true);
			reset_preview_all_buttons();
		}
		break;
	case TYPE_SAVEPAGE:
		play_se(b->clickse, false);
		save_page = b->index;
		update_runtime_props(false);
		update_save_buttons();
		break;
	case TYPE_SAVE:
		process_save(index);
		update_save_buttons();
		break;
	case TYPE_LOAD:
		process_load(index);
		result_index = index;
		break;
	case TYPE_HISTORY:
		process_history_voice(index);
		break;
	case TYPE_TITLE:
		if (title_dialog())
			result_index = index;
		break;
	case TYPE_CHAR:
		play_se(b->clickse, false);
		process_char(index);
		update_namevar_buttons();
		break;
	default:
		result_index = index;
		break;
	}
}

/*
 * ボタンの描画
 */

/* ボタンを描画する */
static void process_button_draw(int index)
{
	struct gui_button *b;

	b = &button[index];

	switch (b->type) {
	case TYPE_BGMVOL:
	case TYPE_VOICEVOL:
	case TYPE_SEVOL:
	case TYPE_CHARACTERVOL:
	case TYPE_TEXTSPEED:
	case TYPE_AUTOSPEED:
		/* スライダを描画する */
		process_button_draw_slider(index);
		break;
	case TYPE_FONT:
	case TYPE_FULLSCREEN:
	case TYPE_WINDOW:
	case TYPE_SAVEPAGE:
		/* アクティブ化可能ボタンを描画する */
		process_button_draw_activatable(index);
		break;
	case TYPE_GALLERY:
		/* ギャラリーを描画する */
		process_button_draw_gallery(index);
		break;
	case TYPE_PREVIEW:
		/* プレビューを描画する */
		process_button_draw_preview(index);
		break;
	case TYPE_SAVE:
	case TYPE_LOAD:
		/* セーブ・ロードボタンを描画する */
		process_button_draw_save(index);
		break;
	case TYPE_HISTORY:
		/* ヒストリボタンを描画する */
		process_button_draw_history(index);
		break;
	case TYPE_HISTORYSCROLL:
		/* 垂直スライダを描画する */
		process_button_draw_slider_vertical(index);
		break;
	case TYPE_NAMEVAR:
		/* 名前変数のプレビューを描画する */
		process_button_draw_namevar(index);
		break;
	default:
		/* ボタンがポイントされているとき、hover画像を描画する */
		if (b->rt.is_pointed)
			draw_stage_gui_hover(b->x, b->y, b->width, b->height);
		break;
	}
}

/* スライダーボタンを描画する */
static void process_button_draw_slider(int index)
{
	struct gui_button *b;
	int x;

	b = &button[index];

	/* ポイントされているとき、バー部分をhover画像で描画する */
	if (b->rt.is_pointed)
		draw_stage_gui_hover(b->x, b->y, b->width, b->height);

	/* 描画位置を計算する */
	x = b->x + (int)((float)(b->width - b->height) * b->rt.slider);

	/* ツマミを描画する */
	draw_stage_gui_active(x, b->y, b->height, b->height, b->x, b->y);
}

/* 垂直スライダーボタンを描画する */
static void process_button_draw_slider_vertical(int index)
{
	struct gui_button *b;
	int y;

	b = &button[index];

	/* ポイントされているとき、バー部分をhover画像で描画する */
	if (b->rt.is_pointed)
		draw_stage_gui_hover(b->x, b->y, b->width, b->height);

	/* 描画位置を計算する */
	y = b->y + (int)((float)(b->height - b->width) * b->rt.slider);

	/* ツマミを描画する */
	draw_stage_gui_active(b->x, y, b->width, b->width, b->x, b->y);
}

/* アクティブ化可能ボタンを描画する */
static void process_button_draw_activatable(int index)
{
	struct gui_button *b;

	b = &button[index];
	assert(b->type == TYPE_FONT || b->type == TYPE_FULLSCREEN ||
	       b->type == TYPE_WINDOW || b->type == TYPE_SAVEPAGE);

	/* フルスクリーンにできない場合 */
	if (conf_window_fullscreen_disable)
		if (b->type == TYPE_FULLSCREEN || b->type == TYPE_WINDOW)
			return;

	/* ポイントされているとき、hover画像を描画する */
	if (b->rt.is_pointed) {
		draw_stage_gui_hover(b->x, b->y, b->width, b->height);
		return;
	}

	/* コンフィグが選択されていればactive画像を描画する */
	if (b->rt.is_active) {
		draw_stage_gui_active(b->x, b->y, b->width, b->height, b->x,
				      b->y);
	}
}

/* ギャラリーボタンを描画する */
static void process_button_draw_gallery(int index)
{
	struct gui_button *b;

	b = &button[index];
	assert(b->type == TYPE_GALLERY);

	/* 指定された変数が0のとき(解放されていないギャラリーの場合) */
	if (b->rt.is_disabled) {
		/* 描画しない */
		return;
	}

	/* ポイントされているとき、hover画像を描画する */
	if (b->rt.is_pointed) {
		draw_stage_gui_hover(b->x, b->y, b->width, b->height);
		return;
	}

	/* ポイントされていないとき、active画像を描画する */
	draw_stage_gui_active(b->x, b->y, b->width, b->height, b->x, b->y);
}

/* ギャラリーボタンを描画する */
static void process_button_draw_namevar(int index)
{
	struct gui_button *b;

	b = &button[index];
	assert(b->type == TYPE_NAMEVAR);

	/* スクリーンへの描画を行う */
	render_image(button[index].x, button[index].y,
		     button[index].rt.img, button[index].width,
		     button[index].height, 0, 0, 255, BLEND_FAST);
}

/* ボタンの状況に応じたSEを再生する */
static void process_play_se(void)
{
	/* 音量系のボタンの場合、SEは再生しない */
	if (button[result_index].type == TYPE_BGMVOL ||
	    button[result_index].type == TYPE_VOICEVOL ||
	    button[result_index].type == TYPE_SEVOL ||
	    button[result_index].type == TYPE_CHARACTERVOL)
		return;

	/* ボタンが選択された場合 */
	if (result_index != -1) {
		play_se(button[result_index].clickse, false);
		return;
	}

	/* 前フレームとは異なるボタンがポイントされた場合 */
	if (is_pointed_changed && pointed_index != -1) {
		play_se(button[pointed_index].pointse, false);
		return;
	}
}

/*
 * セーブ・ロード
 */

/* TYPE_SAVE/TYPE_LOADのボタンの初期化を行う */
static bool init_save_buttons(void)
{
	int i;

	for (i = 0; i < BUTTON_COUNT; i++) {
		if (button[i].type != TYPE_SAVE && button[i].type != TYPE_LOAD)
			continue;

		button[i].rt.img = create_image(button[i].width,
						button[i].height);
		if (button[i].rt.img == NULL)
			return false;

		button[i].rt.color =
			make_pixel_slow(0xff,
					(pixel_t)conf_font_color_r,
					(pixel_t)conf_font_color_g,
					(pixel_t)conf_font_color_b);
		button[i].rt.outline_color =
			make_pixel_slow(0xff,
					(pixel_t)conf_font_outline_color_r,
					(pixel_t)conf_font_outline_color_g,
					(pixel_t)conf_font_outline_color_b);
	}

	return true;
}

/* セーブ・ロードのスロットを描画する */
static void update_save_buttons(void)
{
	int i, base, save_index;

	base = save_slots * save_page;
	for (i = 0; i < BUTTON_COUNT; i++) {
		if (button[i].type != TYPE_SAVE && button[i].type != TYPE_LOAD)
			continue;

		save_index = base + button[i].index;

		if (get_save_date(save_index) != 0)
			button[i].rt.is_active = true;
		else
			button[i].rt.is_active = false;

		draw_save_button(i);
	}
}

/* セーブ・ロードボタンを描画する */
static void draw_save_button(int button_index)
{
	char text[128];
	struct gui_button *b;
	struct image *thumb;
	struct tm *timeptr;
	const char *chapter, *msg;
	time_t save_time;
	int save_index;
	int width;

	b = &button[button_index];
	if (b->rt.img == NULL)
		return;

	/* セーブデータ番号を求める */
	save_index = save_slots * save_page + b->index;

	/* セーブ時刻を求める */
	save_time = get_save_date(save_index);

	/* イメージをロックする */
	lock_image(b->rt.img);

	/* イメージをクリアする */
	clear_image_color(b->rt.img, make_pixel_slow(0, 0, 0, 0));

	/* サムネイルを描画する */
	thumb = get_save_thumbnail(save_index);
	if (thumb != NULL) {
		draw_image(b->rt.img, b->margin, b->margin, thumb,
			   conf_save_data_thumb_width,
			   conf_save_data_thumb_height, 0, 0, 255, BLEND_NONE);
	}

	/* 日時を描画する */
	if (get_save_date(save_index) == 0) {
		snprintf(text, sizeof(text), "[%02d] NO DATA", save_index);
	} else {
		timeptr = localtime(&save_time);
		snprintf(text, sizeof(text), "[%02d] ", save_index);
		strftime(&text[5], sizeof(text) - 5, "%y/%m/%d %H:%M ",
			 timeptr);
	}
	width = draw_save_text_item(button_index,
				    conf_save_data_thumb_width + b->margin * 2,
				    b->margin, text);

	/* 章タイトルを描画する */
	chapter = get_save_chapter_name(save_index);
	if (chapter != NULL) {
		draw_save_text_item(button_index,
				    conf_save_data_thumb_width +
				    b->margin * 2 + width,
				    b->margin, chapter);
	}

	/* 最後のメッセージを描画する */
	msg = get_save_last_message(save_index);
	if (msg) {
		draw_save_text_item(button_index,
				    conf_save_data_thumb_width + b->margin * 2,
				    b->margin + conf_msgbox_margin_line,
				    msg);
	}

	/* イメージをアンロックする */
	unlock_image(b->rt.img);
}

/* セーブデータのテキストを描画する */
static int draw_save_text_item(int button_index, int x, int y,
			       const char *text)
{
	struct gui_button *b;
	uint32_t wc;
	int ruby_size, mblen, cw, ch, result;

	b = &button[button_index];

	/* ルビのフォントサイズを求める */
	ruby_size = conf_gui_save_font_ruby_size > 0 ?
		conf_gui_save_font_ruby_size :
		conf_font_ruby_size > 0 ?
		conf_font_ruby_size :
		conf_font_size / 5;

	/* 描画情報を初期化する */
	b->rt.pen_x = x;
	b->rt.pen_y = y;
	b->rt.top = text;
	b->rt.pen_ruby_y = y - ruby_size;

	/* フォントサイズを設定する */
	set_font_size(conf_gui_save_font_size > 0 ?
		      conf_gui_save_font_size : conf_font_size);

	/* 1文字ずつ描画する */
	result = 0;
	while (*b->rt.top) {
		/* 先頭のエスケープシーケンスをスキップする */
		process_escape_sequence(button_index, true, !conf_gui_ruby,
					ruby_size);
		if (*b->rt.top == '\0')
			break;

		/* 描画する文字を取得する */
		mblen = utf8_to_utf32(b->rt.top, &wc);
		if (mblen == -1)
			return 0;

		/* 文字の幅を取得する */
		cw = get_glyph_width(wc);
		if (button[button_index].rt.pen_x + cw +
		    button[button_index].margin >= button[button_index].width)
			break;

		/* 描画する */
		draw_save_char(button_index, wc, &cw, &ch);
		b->rt.pen_ruby_x = b->rt.pen_x;
		b->rt.pen_x += cw + conf_msgbox_margin_char;

		result += cw;

		/* 次の文字へ移動する */
		b->rt.top += mblen;
	}

	return result;
}

/* 文字を描画する */
static void draw_save_char(int index, uint32_t wc, int *width, int *height)
{
	draw_glyph(button[index].rt.img,
		   button[index].rt.pen_x,
		   button[index].rt.pen_y,
		   button[index].rt.color,
		   button[index].rt.outline_color,
		   wc,
		   width,
		   height,
		   get_font_size());
}

/* セーブを行う */
static void process_save(int button_index)
{
	int data_index;

	/* ボタン番号からセーブデータ番号に変換する */
	data_index = save_page * save_slots + button[button_index].index;

	/* プロンプトを表示する */
	if (get_save_date(data_index) != 0)
		if (!overwrite_dialog())
			return;

	/* SEを再生する */
	play_se(button[button_index].clickse, false);

	/* セーブを実行する */
	execute_save(data_index);

	/*
	 * セーブ直後に上書きダイアログのボタンをクリックするためにマウスが
	 * 移動され、ポイント項目が変わってしまい、セーブボタンのSEがかき消さ
	 * れることを回避する。
	 */
	is_saved_in_this_frame = true;
}

/* ロードを行う */
static void process_load(int button_index)
{
	int data_index;

	/* ボタン番号からセーブデータ番号に変換する */
	data_index = save_page * save_slots + button[button_index].index;

	/* セーブデータがない場合 */
	if (get_save_date(data_index) == 0)
		return;

	/* ロードを実行する */
	execute_load(data_index);
}

/* セーブ・ロードボタンの描画を行う */
static void process_button_draw_save(int button_index)
{
	/* ポイントされているときの描画を行う */
	if (button[button_index].rt.is_pointed) {
		draw_stage_gui_hover(button[button_index].x,
				     button[button_index].y,
				     button[button_index].width,
				     button[button_index].height);
	}

	/* サムネイルとテキストの描画を行う */
	render_image(button[button_index].x, button[button_index].y,
		     button[button_index].rt.img, button[button_index].width,
		     button[button_index].height, 0, 0, 255, BLEND_FAST);
}

/*
 * ヒストリ
 */

/* TYPE_HISTORYのボタンの初期化を行う */
static bool init_history_buttons(void)
{
	int i, history_count;

	/* ヒストリの数を取得する */
	history_count = get_history_count();
	if (history_count == 0)
		return true;

	/* ヒストリの先頭を計算する */
	if (history_count <= history_slots) {
		history_top = history_count - 1;
		transient_history_slider = 0;
	} else {
		history_top = history_slots - 1;
		transient_history_slider = 1.0f;
	}

	/* ボタンごとにイメージを作成する */
	for (i = 0; i < BUTTON_COUNT; i++) {
		if (button[i].type != TYPE_HISTORY)
			continue;

		button[i].rt.img = create_image(button[i].width,
						button[i].height);
		if (button[i].rt.img == NULL)
			return false;
	}

	return true;
}

/* ヒストリのスロットを描画する */
static void update_history_buttons(void)
{
	int i, history_count;

	history_count = get_history_count();
	for (i = 0; i < BUTTON_COUNT; i++) {
		if (button[i].type != TYPE_HISTORY)
			continue;
		if (button[i].rt.img == NULL)
			continue;

		/* ヒストリのオフセットを計算する */
		if (history_count < history_slots) {
			button[i].rt.history_offset =
				history_count - button[i].index - 1;
		} else {
			button[i].rt.history_offset =
				history_top - button[i].index;
		}
		if (button[i].rt.history_offset < 0)
			button[i].rt.history_offset = -1;

		/* スロットのアクティブ状態を求める */
		if (button[i].rt.history_offset >= 0 &&
		    button[i].rt.history_offset < get_history_count()) {
			if (get_history_voice(button[i].rt.history_offset) !=
			    NULL)
				button[i].rt.is_active = true;
			else
				button[i].rt.is_active = false;
		} else {
			button[i].rt.is_active = false;
		}

		/* ボタンのイメージを描画する */
		draw_history_button(i);
	}
}

/* ヒストリボタンを描画する */
static void draw_history_button(int button_index)
{
	struct gui_button *b;

	b = &button[button_index];
	if (b->rt.img == NULL)
		return;

	/* イメージをロックする */
	lock_image(b->rt.img);

	/* イメージをクリアする */
	clear_image_color(b->rt.img, make_pixel_slow(0, 0, 0, 0));

	/* メッセージを描画する */
	if (button[button_index].rt.history_offset != -1)
		draw_history_text_item(button_index);

	/* イメージをアンロックする */
	unlock_image(b->rt.img);
}

/* ヒストリのテキストを描画する */
static void draw_history_text_item(int button_index)
{
	const char *text;
	struct gui_button *b;
	uint32_t c;
	int margin_line, ruby_size, mblen, width, height;

	b = &button[button_index];

	/* メッセージを取得する */
	text = get_history_message(b->rt.history_offset);

	/* 描画情報を初期化する */
	b->rt.top = text;
	b->rt.pen_x = b->margin;
	b->rt.pen_y = b->margin;
	b->rt.color = make_pixel_slow(0xff,
				      (pixel_t)conf_font_color_r,
				      (pixel_t)conf_font_color_g,
				      (pixel_t)conf_font_color_b);
	b->rt.outline_color =
		make_pixel_slow(0xff,
				(pixel_t)conf_font_outline_color_r,
				(pixel_t)conf_font_outline_color_g,
				(pixel_t)conf_font_outline_color_b);

	/* 行間マージンを求める */
	margin_line = conf_gui_history_margin_line > 0 ?
		conf_gui_history_margin_line : conf_msgbox_margin_line;

	/* フォントサイズを設定する */
	set_font_size(conf_gui_history_font_size > 0 ?
		      conf_gui_history_font_size : conf_font_size);

	/* ルビのフォントサイズを求める */
	ruby_size = conf_gui_history_font_ruby_size > 0 ?
		conf_gui_history_font_ruby_size :
		conf_font_ruby_size > 0 ?
		conf_font_ruby_size :
		conf_font_size / 5;

	/* 1文字ずつ描画する */
	while (*b->rt.top != '\0') {
		/* 先頭のエスケープシーケンスを解釈する */
		process_escape_sequence(button_index, false, !conf_gui_ruby,
					ruby_size);
		if (*b->rt.top == '\0')
			break;

		/* ワードラッピングを処理する */
		word_wrapping(button_index);

		/* 描画する文字を取得する */
		mblen = utf8_to_utf32(b->rt.top, &c);
		if (mblen == -1)
			return;

		/* 描画する文字の幅を取得する */
		width = get_glyph_width(c);

		/* メッセージボックスの幅を超える場合、改行する */
		if ((b->rt.pen_x + width + conf_msgbox_margin_char +
		     b->margin >= b->width)
		    &&
		    (c != ' ' && c != ',' && c != '.' && c != ':' &&
		     c != ';' && c != CHAR_TOUTEN && c != CHAR_KUTEN)) {
			b->rt.pen_y += margin_line;
			b->rt.pen_x = b->margin;
		}

		/* 描画する */
		draw_history_char(button_index, c, &width, &height);

		/* 次の文字へ移動する */
		b->rt.pen_ruby_x = b->rt.pen_x;
		b->rt.pen_ruby_y = b->rt.pen_y - ruby_size;
		b->rt.pen_x += width + conf_msgbox_margin_char;
		b->rt.top += mblen;
	}
}

/* 文字を描画する */
static void draw_history_char(int index, uint32_t wc, int *width, int *height)
{
	draw_glyph(button[index].rt.img,
		   button[index].rt.pen_x,
		   button[index].rt.pen_y,
		   button[index].rt.color,
		   button[index].rt.outline_color,
		   wc,
		   width,
		   height,
		   get_font_size());
}

/* エスケープシーケンスを処理する */
static void process_escape_sequence(int button_index, bool ignore_lf,
				    bool ignore_ruby, int ruby_size)
{
	struct gui_button *b;

	b = &button[button_index];

	/* エスケープシーケンスが続く限り処理する */
	while (*b->rt.top == '\\') {
		switch (*(b->rt.top + 1)) {
		case 'n':
			/* 改行 */
			process_escape_sequence_lf(button_index, ignore_lf);
			break;
		case '#':
			/* 色指定 */
			if (!process_escape_sequence_color(button_index))
				return; /* 不正: 読み飛ばさない */
			break;
		case '^':
			/* ルビ */
			if (!process_escape_sequence_ruby(button_index,
							  ignore_ruby,
							  ruby_size))
				return; /* 不正: 読み飛ばさない */
			break;
		case '@':
		case 'w':
		case 'p':
			/* サイズ、ウェイト、ペン位置は読み飛ばす */
			if (!process_escape_sequence_other(button_index))
				return;
			break;
		default:
			/* 不正なエスケープシーケンスなので読み飛ばさない */
			return;
		}
	}
}

/* 改行エスケープシーケンスを処理する */
static bool process_escape_sequence_lf(int button_index, bool ignore_lf)
{
	const char *c;

	c = button[button_index].rt.top;
	assert(*c == '\0');
	assert(*(c + 1) == 'n');

	/* 改行する */
	if (!ignore_lf) {
		button[button_index].rt.pen_y += conf_msgbox_margin_line;
		button[button_index].rt.pen_x = button[button_index].margin;
	}

	/* メッセージを進める */
	button[button_index].rt.top += 2;

	return true;
}

/* 色指定エスケープシーケンスを処理する */
static bool process_escape_sequence_color(int button_index)
{
	char color_code[7];
	uint32_t r, g, b;
	const char *c;
	int rgb;

	c = button[button_index].rt.top;
	assert(*c == '\\');
	assert(*(c + 1) == '#');

	/* 始まりのカッコ'{'をチェックする */
	if (*(c + 2) != '{')
		return false;

	/* カラーコードの長さが足りない場合 */
	if (strlen(c + 3) < 6)
		return false;

	/* 終わりのカッコ'}'をチェックする */
	if (*(c + 9) != '}')
		return false;

	/* メッセージを進める \#{RRGGBB} */
	button[button_index].rt.top += 10;
	if (conf_gui_history_disable_color)
		return true;

	/* カラーコードを読む */
	rgb = 0;
	memcpy(color_code, c + 3, 6);
	color_code[6] = '\0';
	sscanf(color_code, "%x", &rgb);
	r = (rgb >> 16) & 0xff;
	g = (rgb >> 8) & 0xff;
	b = rgb & 0xff;
	button[button_index].rt.color = make_pixel_slow(0xff, r, g, b);

	return true;
}

/* ルビのエスケープシーケンスを処理する */
static bool process_escape_sequence_ruby(int button_index, bool ignore_ruby,
					 int ruby_size)
{
	char ruby[64];
	const char *c, *s;
	struct gui_button *b;
	uint32_t wc;
	int i, font_size, mblen, ret_width, ret_height;

	b = &button[button_index];
	c = b->rt.top;
	assert(*c == '\\');
	assert(*(c + 1) == '^');

	/* '{'をチェックする */
	if (*(c + 2) != '{')
		return false;

	/* ルビを読む */
	for (i = 0; i < (int)sizeof(ruby) - 1; i++) {
		if (*(c + 3 + i) == '\0')
			return false;
		if (*(c + 3 + i) == '}')
			break;
		ruby[i] = *(c + 3 + i);
	}
	ruby[i] = '\0';

	/* \^{ + ruby[] + } */
	b->rt.top += 3 + i + 1;
	if (ignore_ruby)
		return true;

	/* フォントサイズを退避して、ルビ用に設定する */
	font_size = get_font_size();
	set_font_size(ruby_size);

	/* 描画する */
	s = ruby;
	while (*s) {
		mblen = utf8_to_utf32(s, &wc);
		if (mblen == -1)
			break;

		draw_ruby_char(button_index, wc, &ret_width, &ret_height);

		b->rt.pen_ruby_x += ret_width;
		s += mblen;
	}

	/* フォントサイズを復元する */
	set_font_size(font_size);

	return true;
}

/* ルビの文字を描画する */
static void draw_ruby_char(int button_index, uint32_t wc, int *width,
			   int *height)
{
	draw_glyph(button[button_index].rt.img,
		   button[button_index].rt.pen_ruby_x,
		   button[button_index].rt.pen_ruby_y,
		   button[button_index].rt.color,
		   button[button_index].rt.outline_color,
		   wc,
		   width,
		   height,
		   get_font_size());
}

/* エスケープシーケンスをスキップする */
static bool process_escape_sequence_other(int button_index)
{
	const char *c;
	int i;

	c = button[button_index].rt.top;
	assert(*c == '\\');
	assert(*(c + 1) == '@' || *(c + 1) == 'w' || *(c + 1) == 'p');

	/* 先頭をチェックする */
	if (*(c + 2) != '{')
		return false;
	c += 3;

	/* 終端文字'}'を探す */
	i = 0;
	while (*c != '\0' && *c != '}')
		c++, i++;
	if (*c != '}')
		return false;

	/* メッセージを進める \X{ + i + } */
	button[button_index].rt.top += 3 + i + 1;

	return true;
}

/* ヒストリボタンの描画を行う */
static void process_button_draw_history(int button_index)
{
	/* ヒストリがない場合 */
	if (button[button_index].rt.img == NULL)
		return;

	/* ポイントされているときの描画を行う */
	if (button[button_index].rt.is_active &&
	    button[button_index].rt.is_pointed) {
		draw_stage_gui_hover(button[button_index].x,
				     button[button_index].y,
				     button[button_index].width,
				     button[button_index].height);
	}

	/* テキストの描画を行う */
	render_image(button[button_index].x, button[button_index].y,
		     button[button_index].rt.img, button[button_index].width,
		     button[button_index].height, 0, 0, 255, BLEND_FAST);
}

/* history_topを更新する */
static void update_history_top(int button_index)
{
	struct gui_button *b;
	int history_count;
	int old_history_top;

	b = &button[button_index];
	old_history_top = history_top;

	/* ヒストリの数がスロットの数より小さい場合 */
	history_count = get_history_count();
	if (history_count <= history_slots) {
		history_top = history_count - 1;
		b->rt.slider = 0;
		transient_history_slider = 0;
		return;
	}

	/* ヒストリのトップを更新する */
	history_top = (int)((float)((history_count - 1) -
				    (history_slots - 1)) *
			    (1.0f - transient_history_slider)) +
			   (history_slots - 1);
	assert(history_top >= history_slots - 1);
	assert(history_top <= history_count - 1);

	/* スライダの位置を補正する */
	transient_history_slider = 1.0f -
		(float)(history_top - (history_slots - 1)) /
		(float)((history_count - 1) - (history_slots - 1));

	/* 再描画が必要な場合 */
	if (history_top != old_history_top)
		need_update_history_buttons = true;
}

/* マウスホイールおよび上下キーによるスクロールを処理する */
static void process_history_scroll(int delta)
{
	int history_count, old_history_top;

	old_history_top = history_top;

	/* ヒストリの数がスロットの数より小さい場合 */
	history_count = get_history_count();
	if (history_count <= history_slots)
		return;

	/* スクロールする行数を減算する */
	history_top -= delta;

	/* 上限と下限でカットする */
	if (history_top < history_slots)
		history_top = history_slots - 1;
	if (history_top >= history_count)
		history_top = history_count - 1;

	/* スライダの位置を補正する */
	transient_history_slider = 1.0f -
		(float)(history_top - (history_slots - 1)) /
		(float)((history_count - 1) - (history_slots - 1));

	/* 再描画が必要な場合 */
	if (history_top != old_history_top)
		need_update_history_buttons = true;
}

/* ヒストリのボイスを再生する */
static void process_history_voice(int button_index)
{
	if (button[button_index].rt.history_offset != -1) {
		/* その他のキャラクタのボリュームを適用する */
		apply_character_volume(CH_VOL_SLOT_DEFAULT);

		/* ボイスを再生する */
		play_se(get_history_voice(
				button[button_index].rt.history_offset),
			true);
	}
}

/*
 * テキストプレビュー
 */

/* TYPE_PREVIEWのボタンの初期化を行う */
static bool init_preview_buttons(void)
{
	int i;

	for (i = 0; i < BUTTON_COUNT; i++) {
		if (button[i].type != TYPE_PREVIEW)
			continue;
		if (button[i].msg == NULL) {
			button[i].msg = strdup("");
			if (button[i].msg == NULL) {
				log_memory();
				return false;
			}
		}
		if (button[i].width <= 0)
			button[i].width = 1;
		if (button[i].height <= 0)
			button[i].height = 1;

		button[i].rt.img = create_image(button[i].width,
						button[i].height);
		if (button[i].rt.img == NULL)
			return false;
		lock_image(button[i].rt.img);
		clear_image_color(button[i].rt.img,
				  make_pixel_slow(0, 0, 0, 0));
		unlock_image(button[i].rt.img);

		button[i].rt.is_waiting = false;
		button[i].rt.top = button[i].msg;
		button[i].rt.total_chars = utf8_chars(button[i].msg);
		button[i].rt.drawn_chars = 0;
		button[i].rt.pen_x = 0;
		button[i].rt.pen_y = 0;
		button[i].rt.color =
			make_pixel_slow(0xff,
					(pixel_t)conf_font_color_r,
					(pixel_t)conf_font_color_g,
					(pixel_t)conf_font_color_b);
		button[i].rt.outline_color =
			make_pixel_slow(0xff,
					(pixel_t)conf_font_outline_color_r,
					(pixel_t)conf_font_outline_color_g,
					(pixel_t)conf_font_outline_color_b);
		button[i].rt.is_after_space = false;
		reset_stop_watch(&button[i].rt.sw);
	}

	return true;
}

/* テキストプレビューをリセットする */
static void reset_preview_all_buttons(void)
{
	int i;

	for (i = 0; i < BUTTON_COUNT; i++) {
		if (button[i].type != TYPE_PREVIEW)
			continue;

		assert(button[i].msg != NULL);

		reset_preview_button(i);
	}
}

/* テキストプレビューをリセットする */
static void reset_preview_button(int index)
{
	assert(button[index].type == TYPE_PREVIEW);

	lock_image(button[index].rt.img);
	clear_image_color(button[index].rt.img, make_pixel_slow(0, 0, 0, 0));
	unlock_image(button[index].rt.img);

	button[index].rt.is_waiting = false;
	button[index].rt.top = button[index].msg;
	button[index].rt.drawn_chars = 0;
	button[index].rt.pen_x = 0;
	button[index].rt.pen_y = 0;
	button[index].rt.is_after_space = false;
	reset_stop_watch(&button[index].rt.sw);
}

/* テキストプレビューのボタンの描画を行う */
static void process_button_draw_preview(int index)
{
	int lap;

	/* メッセージの途中の場合 */
	if (!button[index].rt.is_waiting) {
		/* メインメモリ上のイメージの描画を行う */
		draw_message(index);

		/* すべての文字を描画し終わった場合 */
		if (button[index].rt.drawn_chars ==
		    button[index].rt.total_chars) {
			/* ストップウォッチを初期化する */
			reset_stop_watch(&button[index].rt.sw);

			/* オートモードの待ち時間に入る */
			button[index].rt.is_waiting = true;
		}
	} else {
		/* オートモードの待ち時間の場合 */
		lap = get_stop_watch_lap(&button[index].rt.sw);
		if (lap >= get_wait_time(index)) {
			/* 待ちを終了する */
			reset_preview_button(index);
		}
	}

	/* スクリーンへの描画を行う */
	render_image(button[index].x, button[index].y,
		     button[index].rt.img, button[index].width,
		     button[index].height, 0, 0, 255, BLEND_FAST);
}

/* メッセージの描画を行う */
static void draw_message(int index)
{
	uint32_t c;
	int char_count, mblen, cw, ch, i;

	/* 今回のフレームで描画する文字数を取得する */
	char_count = get_frame_chars(index);
	if (char_count == 0)
		return;

	/* 1文字ずつ描画する */
	lock_image(button[index].rt.img);
	for (i = 0; i < char_count; i++) {
		/* ワードラッピングを処理する */
		word_wrapping(index);

		/* 描画する文字を取得する */
		mblen = utf8_to_utf32(button[index].rt.top, &c);
		if (mblen == -1) {
			button[index].rt.drawn_chars =
				button[index].rt.total_chars;
			unlock_image(button[index].rt.img);
			return;
		}

		/* 描画する文字の幅を取得する */
		cw = get_glyph_width(c);

		/* ボタン領域の幅を超える場合、改行する */
		if (button[index].rt.pen_x + cw >= button[index].width &&
		    (c != ' ' && c != ',' && c != '.' && c != ':' &&
		     c != ';' && c != CHAR_TOUTEN && c != CHAR_KUTEN)) {
			button[index].rt.pen_y += conf_msgbox_margin_line;
			button[index].rt.pen_x = 0;
			if (*button[index].rt.top == ' ') {
				button[index].rt.top++;
				button[index].rt.drawn_chars++;
				continue;
			}
		}

		/* 描画する */
		draw_message_char(index, c, &cw, &ch);

		/* 次の文字へ移動する */
		button[index].rt.pen_x += cw;
		button[index].rt.top += mblen;
		button[index].rt.drawn_chars++;
	}
	unlock_image(button[index].rt.img);
}

/* 文字を描画する */
static void draw_message_char(int index, uint32_t wc, int *width, int *height)
{
	draw_glyph(button[index].rt.img,
		   button[index].rt.pen_x,
		   button[index].rt.pen_y,
		   button[index].rt.color,
		   button[index].rt.outline_color,
		   wc,
		   width,
		   height,
		   conf_font_size);
}

/* 今回のフレームで描画する文字数を取得する */
static int get_frame_chars(int index)
{
	float lap;
	int char_count;

	/* テキストスピードが最大のときはノーウェイトで全部描画する */
	if (get_text_speed() == 1.0f) {
		return button[index].rt.total_chars -
			button[index].rt.drawn_chars;
	}

	/* 経過時間を取得する */
	lap = (float)get_stop_watch_lap(&button[index].rt.sw) / 1000.0f;

	/* 今回描画する文字数を取得する */
	char_count = (int)ceil(conf_msgbox_speed * (get_text_speed() + 0.1) *
			       lap) - button[index].rt.drawn_chars;
	if (char_count >
	    button[index].rt.total_chars - button[index].rt.drawn_chars) {
		char_count = button[index].rt.total_chars -
			     button[index].rt.drawn_chars;
	}

	return char_count;
}

/* ワードラッピングを行う */
static void word_wrapping(int index)
{
	if (button[index].rt.is_after_space) {
		if (button[index].rt.pen_x +
		    get_en_word_width(button[index].rt.top) >=
		    button[index].width - button[index].margin) {
			button[index].rt.pen_y += conf_msgbox_margin_line;
			button[index].rt.pen_x = button[index].margin;
		}
	}

	button[index].rt.is_after_space = *button[index].rt.top == ' ';
}

/* msgが英単語の先頭であれば、その単語の描画幅、それ以外の場合0を返す */
static int get_en_word_width(const char *m)
{
	uint32_t wc;
	int width;

	width = 0;
	while (isgraph_extended(&m, &wc))
		width += get_glyph_width(wc);

	return width;
}

/* オートモードの待ち時間を計算する */
static int get_wait_time(int index)
{
	const float AUTO_MODE_TEXT_WAIT_SCALE = 0.15f;
	float scale;

	scale = conf_automode_speed;
	if (scale == 0)
		conf_automode_speed = AUTO_MODE_TEXT_WAIT_SCALE;

	return (int)((float)button[index].rt.total_chars * scale *
		     get_auto_speed() * 1000.0f);
}

/*
 * 名前文字列の表示
 */

/* TYPE_NAMEVARのボタンの初期化を行う */
static bool init_namevar_buttons(void)
{
	int i;

	for (i = 0; i < BUTTON_COUNT; i++) {
		if (button[i].type != TYPE_NAMEVAR)
			continue;
		if (button[i].width <= 0)
			button[i].width = 1;
		if (button[i].height <= 0)
			button[i].height = 1;

		button[i].rt.img = create_image(button[i].width,
						button[i].height);
		if (button[i].rt.img == NULL)
			return false;
		lock_image(button[i].rt.img);
		clear_image_color(button[i].rt.img,
				  make_pixel_slow(0, 0, 0, 0));
		unlock_image(button[i].rt.img);

		button[i].rt.color =
			make_pixel_slow(0xff,
					(pixel_t)conf_font_color_r,
					(pixel_t)conf_font_color_g,
					(pixel_t)conf_font_color_b);
		button[i].rt.outline_color =
			make_pixel_slow(0xff,
					(pixel_t)conf_font_outline_color_r,
					(pixel_t)conf_font_outline_color_g,
					(pixel_t)conf_font_outline_color_b);
	}

	return true;
}

/* 名前変数のボタンを描画する */
static void update_namevar_buttons(void)
{
	int i;

	for (i = 0; i < BUTTON_COUNT; i++) {
		if (button[i].type != TYPE_NAMEVAR)
			continue;

		lock_image(button[i].rt.img);
		clear_image_color(button[i].rt.img,
				  make_pixel_slow(0, 0, 0, 0));
		draw_name(i);
		unlock_image(button[i].rt.img);
	}
}

/* 名前の描画を行う */
static void draw_name(int index)
{
	const char *name;
	uint32_t c;
	int char_count, mblen, i, pen_x, cw, w, h;

	/* 描画する文字列を取得する */
	name = get_name_variable(button[index].namevar);
	assert(name != NULL);

	/* 描画する文字数を取得する */
	char_count = utf8_chars(name);
	if (char_count == 0)
		return;

	/* 1文字ずつ描画する */
	pen_x = 2; /* アウトラインの2px分 */
	for (i = 0; i < char_count; i++) {
		/* 描画する文字を取得する */
		mblen = utf8_to_utf32(name, &c);
		if (mblen == -1)
			break;

		/* 描画する文字の幅を取得する */
		cw = get_glyph_width(c);

		/* 描画する */
		draw_glyph(button[index].rt.img, pen_x, 0,
			   button[index].rt.color,
			   button[index].rt.outline_color, c, &w, &h,
			   conf_font_size);

		/* 次の文字へ移動する */
		name += mblen;
		pen_x += cw;
	}
}

/*
 * 名前文字列の編集
 */

/* 名前文字列の編集ボタンの押下を処理する */
static void process_char(int index)
{
	char buf[1204];
	struct gui_button *b;
	const char *orig;

	b = &button[index];
	if (b->msg == NULL)
		return;

	/* クリアボタンの場合 */
	if (strcmp(b->msg, "[clear]") == 0) {
		set_name_variable(b->namevar, "");
		return;
	}

	/* 1文字消去ボタンの場合 */
	if (strcmp(b->msg, "[backspace]") == 0) {
		truncate_name_variable(b->namevar);
		return;
	}

	/* 決定ボタンの場合 */
	if (strcmp(b->msg, "[ok]") == 0) {
		/* 名前変数が空白なら決定できない */
		if (strcmp(get_name_variable(b->namevar), "") != 0)
			result_index = index;
		return;
	}

	/* 現在の名前変数の値を取得する */
	orig = get_name_variable(b->namevar);

	/* すでに最大文字数まで到達している場合 */
	if (utf8_chars(orig) >= b->max)
		return;

	/* 末尾に追加した文字列を作成する */
	snprintf(buf, sizeof(buf), "%s%s", orig, b->msg);

	/* 名前変数の値を更新する */
	set_name_variable(b->namevar, buf);
}

/*
 * 結果の取得
 */

/*
 * GUIの実行結果のジャンプ先ラベルを取得する
 */
const char *get_gui_result_label(void)
{
	struct gui_button *b;

	if (result_index == -1)
		return NULL;

	b = &button[result_index];

	if (b->type != TYPE_GOTO && b->type != TYPE_GALLERY)
		return NULL;

	return b->label;
}

/*
 * GUIの実行結果が終了であるかを取得する
 */
bool is_gui_result_exit(void)
{
	struct gui_button *b;

	if (result_index == -1)
		return false;

	b = &button[result_index];

	if (b->type != TYPE_QUIT)
		return false;

	return true;
}

/*
 * SEの再生
 */

/* SEを再生する */
static void play_se(const char *file, bool is_voice)
{
	struct wave *w;

	if (file == NULL || strcmp(file, "") == 0)
		return;

	w = create_wave_from_file(is_voice ? CV_DIR : SE_DIR, file, false);
	if (w == NULL)
		return;

	set_mixer_input(is_voice ? VOICE_STREAM : SE_STREAM, w);
}

/*
 * GUIファイルのロード
 */

/* GUIファイルをロードする */
static bool load_gui_file(const char *file)
{
	enum {
		ST_SCOPE,
		ST_OPEN,
		ST_KEY,
		ST_COLON,
		ST_VALUE,
		ST_VALUE_DQ,
		ST_SEMICOLON,
		ST_ERROR
	};

	char word[256], key[256];
	struct rfile *rf;
	char *buf;
	size_t fsize, pos;
	int st, len, line, btn;
	char c;
	bool is_global, is_comment;

	assert(file != NULL);

	/* ファイルをオープンする */
	rf = open_rfile(GUI_DIR, file, false);
	if (rf == NULL)
		return false;

	/* ファイルサイズを取得する */
	fsize = get_rfile_size(rf);

	/* メモリを確保する */
	buf = malloc(fsize);
	if (buf == NULL) {
		log_memory();
		close_rfile(rf);
		return false;
	}

	/* ファイルを読み込む */
	if (read_rfile(rf, buf, fsize) < fsize) {
		log_file_read(GUI_DIR, file);
		close_rfile(rf);
		free(buf);
		return false;
	}

	/* コメントをスペースに変換する */
	is_comment = false;
	for (pos = 0; pos < fsize; pos++) {
		if (!is_comment) {
			if (buf[pos] == '#') {
				buf[pos] = ' ';
				is_comment = true;
			}
		} else {
			if (buf[pos] == '\n')
				is_comment = false;
			else
				buf[pos] = ' ';
		}
	}

	/* ファイルをパースする */
	st = ST_SCOPE;
	line = 0;
	len = 0;
	btn = -1;
	pos = 0;
	is_global = false;
	while (pos < fsize) {
		/* 1文字読み込む */
		c = buf[pos++];

		/* ステートに応じて解釈する */
		switch (st) {
		case ST_SCOPE:
			if (len == 0) {
				if (c == ' ' || c == '\t' || c == '\r' ||
				    c == '\n') {
					st = ST_SCOPE;
					break;
				}
				if (c == ':' || c == '{' || c == '}') {
					log_gui_parse_char(c);
					st = ST_ERROR;
					break;
				}
			}
			if (c == '}' || c == ':') {
				log_gui_parse_char(c);
				st = ST_ERROR;
				break;
			}
			if (c == ' ' || c == '\t' || c == '\r' || c == '\n' ||
			    c == '{') {
				assert(len > 0);
				word[len] = '\0';
				if (strcmp(word, "global") == 0) {
					is_global = true;
				} else {
					is_global = false;
					if (!add_button(++btn)) {
						st = ST_ERROR;
						break;
					}
				}
				if (c == '{')
					st = ST_KEY;
				else
					st = ST_OPEN;
				len = 0;
				break;
			}
			if (len == sizeof(word) - 1) {
				log_gui_parse_long_word();
				st = ST_ERROR;
				break;
			}
			word[len++] = c;
			st = ST_SCOPE;
			break;
		case ST_OPEN:
			if (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
				st = ST_OPEN;
				break;
			}
			if (c == '{') {
				st = ST_KEY;
				len = 0;
				break;
			}
			log_gui_parse_char(c);
			st = ST_ERROR;
			break;
		case ST_KEY:
			if (len == 0) {
				if (c == ' ' || c == '\t' || c == '\r' ||
				    c == '\n') {
					st = ST_KEY;
					break;
				}
				if (c == ':') {
					log_gui_parse_char(c);
					st = ST_ERROR;
					break;
				}
				if (c == '}') {
					st = ST_SCOPE;
					break;
				}
			}
			if (c == '{' || c == '}') {
				log_gui_parse_char(c);
				st = ST_ERROR;
				break;
			}
			if (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
				word[len] = '\0';
				strcpy(key, word);
				st = ST_COLON;
				len = 0;
				break;
			}
			if (c == ':') {
				word[len] = '\0';
				strcpy(key, word);
				st = ST_VALUE;
				len = 0;
				break;
			}
			if (len == sizeof(word) - 1) {
				log_gui_parse_long_word();
				st = ST_ERROR;
				break;
			}
			word[len++] = c;
			st = ST_KEY;
			break;
		case ST_COLON:
			if (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
				st = ST_COLON;
				break;
			}
			if (c == ':') {
				st = ST_VALUE;
				len = 0;
				break;
			}
			log_gui_parse_char(c);
			st = ST_ERROR;
			break;
		case ST_VALUE:
			if (len == 0) {
				if (c == ' ' || c == '\t' || c == '\r' ||
				    c == '\n') {
					st = ST_VALUE;
					break;
				}
				if (c == '\"') {
					st = ST_VALUE_DQ;
					break;
				}
			}
			if (c == ':' || c == '{') {
				log_gui_parse_char(c);
				st = ST_ERROR;
				break;
			}
			if (c == ' ' || c == '\t' || c == '\r' || c == '\n' ||
			    c == ';' || c == '}') {
				word[len] = '\0';
				if (is_global) {
					if (!set_global_key_value(key, word)) {
						st = ST_ERROR;
						break;
					}
				} else {
					if (!set_button_key_value(btn, key,
								  word)) {
						st = ST_ERROR;
						break;
					}
				}
				if (c == ';')
					st = ST_KEY;
				else if (c == '}')
					st = ST_SCOPE;
				else
					st = ST_SEMICOLON;
				len = 0;
				break;
			}
			if (len == sizeof(word) - 1) {
				log_gui_parse_long_word();
				st = ST_ERROR;
				break;
			}
			word[len++] = c;
			st = ST_VALUE;
			break;
		case ST_VALUE_DQ:
			if (c == '\"') {
				word[len] = '\0';
				if (is_global) {
					if (!set_global_key_value(key, word)) {
						st = ST_ERROR;
						break;
					}
				} else {
					if (!set_button_key_value(btn, key,
								  word)) {
						st = ST_ERROR;
						break;
					}
				}
				st = ST_SEMICOLON;
				len = 0;
				break;
			}
			if (c == '\r' || c == '\n') {
				log_gui_parse_char(c);
				st = ST_ERROR;
				break;
			}
			if (len == sizeof(word) - 1) {
				log_gui_parse_long_word();
				st = ST_ERROR;
				break;
			}
			word[len++] = c;
			st = ST_VALUE_DQ;
			break;
		case ST_SEMICOLON:
			if (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
				st = ST_SEMICOLON;
				break;
			}
			if (c == ';') {
				st = ST_KEY;
				len = 0;
				break;
			}
			if (c == '}') {
				st = ST_SCOPE;
				len = 0;
				break;
			}
			log_gui_parse_char(c);
			st = ST_ERROR;
			break;
		}

		/* エラー時 */
		if (st == ST_ERROR)
			break;

		/* FIXME: '\r'のみの改行を処理する */
		if (c == '\n')
			line++;
	}

	/* エラーが発生した場合 */
	if (st == ST_ERROR) {
		log_gui_parse_footer(file, line);
	} else if (st != ST_SCOPE || len > 0) {
		log_gui_parse_invalid_eof();
	}

	/* バッファを解放する */
	free(buf);

	/* ファイルをクローズする */
	close_rfile(rf);

	return st != ST_ERROR;
}
