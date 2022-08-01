/* -*- coding: utf-8-with-signature; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Suika 2
 * Copyright (C) 2001-2022, TABATA Keiichi. All rights reserved.
 */

/*
 * GUI
 *
 * [Changes]
 *  - 2022/07/27 作成
 */

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "suika.h"

/* デフォルトのGUIファイル */
#define DEFAULT_FILE	"system.txt"

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

	/* コンフィグのキーを変更するボタン */
	TYPE_CONFIG,

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

	/* 別のGUIに移動するボタン */
	TYPE_GUI,

	/* タイトルに戻るボタン */
	TYPE_TITLE,

	/* GUIを終了するボタン */
	TYPE_CANCEL,

	/* アプリケーションを終了するボタン */
	TYPE_QUIT,
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

	/* TYPE_GOTO, TYPE_GALLERY */
	char *label;

	/* TYPE_BGMVOL, TYPE_VOICEVOL, TYPE_SEVOL, TYPE_GUI */
	char *file;

	/* TYPE_CONFIG */
	char *key;
	char *value;

	/* TYPE_CHARACTERVOL */
	int index;

	/* TYPE_PREVIEW */
	char *msg;

	/* TYPE_VOLUME以外 */
	char *clickse;

	/* すべて */
	char *pointse;

	/*
	 * 実行時の情報
	 */
	struct {
		/* TYPE_CONFIG, TYPE_FULLSCREEN, TYPE_WINDOWのときアクティブか */
		bool is_active;

		/* TYPE_GALLERYのときボタンが無効化されているか */
		bool is_disabled;

		/* 前のフレームでポイントされていたか */
		bool is_pointed;

		/* ドラッグ中か */
		bool is_dragging;

		/* ボリューム・スピードのスライダーの値 */
		float slider;

		/*
		 * テキストプレビューの情報
		 */

		/* メッセージ領域の画像(フォント描画用) */
		struct image *img;

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

/* ポイントされているボタンのインデックス */
static int pointed_index;

/* ポイントされているボタンが変化したか */
static bool is_pointed_changed;

/* 選択結果のボタンのインデックス */
static int result_index;

/*
 * 前方参照
 */
static bool add_button(int index);
static bool set_global_key_value(const char *key, const char *val);
static bool set_button_key_value(const int index, const char *key,
				 const char *val);
static void set_active_config_buttons(void);
static void process_button_point(int index);
static void process_button_drag(int index);
static void process_button_click(int index);
static void process_button_draw(int index);
static void process_button_draw_slider(int index);
static void process_button_draw_config(int index);
static void process_button_draw_gallery(int index);
static void process_play_se(void);
static void play_se(const char *file, bool is_voice);
static bool init_preview_buttons(void);
static void reset_preview_all_buttons(void);
static void reset_preview_button(int index);
static void process_button_draw_preview(int index);
static void draw_message(int index);
static int get_frame_chars(int index);
static void draw_char(int index, uint32_t wc, int *width, int *height);
static int get_en_word_width(const char *m);
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
	is_called_from_command = false;

	/* ボタンのメモリを解放する */
	for (i = 0; i < BUTTON_COUNT; i++) {
		if (button[i].label != NULL)
			free(button[i].label);
		if (button[i].file != NULL)
			free(button[i].file);
		if (button[i].key != NULL)
			free(button[i].key);
		if (button[i].value != NULL)
			free(button[i].value);
		if (button[i].clickse != NULL)
			free(button[i].clickse);
		if (button[i].pointse != NULL)
			free(button[i].pointse);
		if (button[i].rt.img != NULL)
			destroy_image(button[i].rt.img);
	}

	/* ボタンをゼロクリアする */
	memset(button, 0, sizeof(button));

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
bool prepare_gui_mode(const char *file, bool cancel)
{
	assert(!flag_gui_mode);

	/* ボタンをゼロクリアする */
	memset(button, 0, sizeof(button));

	/* GUI定義ファイルが指定されていない場合 */
	if (file == NULL) {
		/* メッセージかスイッチで右クリックされたと判断する */
		is_called_from_command = file == NULL;

		/* デフォルトのファイルを使う */
		gui_file = DEFAULT_FILE;
	} else {
		/* 指定されたファイルを使う */
		gui_file = file;
	}

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

	/* TYPE_CONFIGのボタンのアクティブ状態を設定する */
	set_active_config_buttons();

	/* TYPE_PREVIEWのボタンの初期化を行う */
	if (!init_preview_buttons()) {
		cleanup_gui();
		return false;
	}

	/* 右クリックでキャンセルするか */
	cancel_when_right_click = cancel;

	return true;
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

	if (strcmp("type", key) == 0) {
		if (strcmp("goto", val) == 0) {
			b->type = TYPE_GOTO;
			return true;
		}
		if (strcmp("gallery", val) == 0) {
			b->type = TYPE_GALLERY;
			return true;
		}
		if (strcmp("config", val) == 0) {
			b->type = TYPE_CONFIG;
			return true;
		}
		if (strcmp("bgmvol", val) == 0) {
			b->type = TYPE_BGMVOL;
			b->rt.slider = get_mixer_global_volume(BGM_STREAM);
			return true;
		}
		if (strcmp("voicevol", val) == 0) {
			b->type = TYPE_VOICEVOL;
			b->rt.slider = get_mixer_global_volume(VOICE_STREAM);
			return true;
		}
		if (strcmp("sevol", val) == 0) {
			b->type = TYPE_SEVOL;
			b->rt.slider = get_mixer_global_volume(SE_STREAM);
			return true;
		}
		if (strcmp("charactervol", val) == 0) {
			b->type = TYPE_CHARACTERVOL;
			return true;
		}
		if (strcmp("textspeed", val) == 0) {
			b->type = TYPE_TEXTSPEED;
			b->rt.slider = get_text_speed();
			return true;
		}
		if (strcmp("autospeed", val) == 0) {
			b->type = TYPE_AUTOSPEED;
			b->rt.slider = get_auto_speed();
			return true;
		}
		if (strcmp("preview", val) == 0) {
			b->type = TYPE_PREVIEW;
			return true;
		}
		if (strcmp("fullscreen", val) == 0) {
			b->type = TYPE_FULLSCREEN;
			return true;
		}
		if (strcmp("window", val) == 0) {
			b->type = TYPE_WINDOW;
			return true;
		}
		if (strcmp("gui", val) == 0) {
			b->type = TYPE_GUI;
			return true;
		}
		if (strcmp("title", val) == 0) {
			b->type = TYPE_TITLE;
			return true;
		}
		if (strcmp("cancel", val) == 0) {
			b->type = TYPE_CANCEL;
			return true;
		}
		if (strcmp("quit", val) == 0) {
			b->type = TYPE_QUIT;
			return true;
		}
		log_gui_unknown_button_type(val);
		return false;
	}
	if (strcmp("x", key) == 0) {
		b->x = atoi(val);
		return true;
	}
	if (strcmp("y", key) == 0) {
		b->y = atoi(val);
		return true;
	}
	if (strcmp("width", key) == 0) {
		b->width = atoi(val);
		return true;
	}
	if (strcmp("height", key) == 0) {
		b->height = atoi(val);
		return true;
	}
	if (strcmp("label", key) == 0) {
		b->label = strdup(val);
		if (b->label == NULL) {
			log_memory();
			return false;
		}
		return true;
	}
	if (strcmp("file", key) == 0) {
		b->file = strdup(val);
		if (b->file == NULL) {
			log_memory();
			return false;
		}
		return true;
	}
	if (strcmp("var", key) == 0) {
		if (!get_variable_by_string(val, &var_val))
			return false;
		if (var_val == 0)
			b->rt.is_disabled = true;
		return true;
	}
	if (strcmp("key", key) == 0) {
		b->key = strdup(val);
		if (b->key == NULL) {
			log_memory();
			return false;
		}
		return true;
	}
	if (strcmp("value", key) == 0) {
		b->value = strdup(val);
		if (b->value == NULL) {
			log_memory();
			return false;
		}
		return true;
	}
	if (strcmp("index", key) == 0) {
		b->index = atoi(val);
		if (b->type == TYPE_CHARACTERVOL) {
			if (b->index < 0)
				b->index = 0;
			if (b->index >= CH_VOL_SLOTS)
				b->index = CH_VOL_SLOTS - 1;
			b->rt.slider = get_character_volume(b->index);
		}
		return true;
	}
	if (strcmp("msg", key) == 0) {
		b->msg = strdup(val);
		if (b->msg == NULL) {
			log_memory();
			return false;
		}
		return true;
	}
	if (strcmp("clickse", key) == 0) {
		b->clickse = strdup(val);
		if (b->clickse == NULL) {
			log_memory();
			return false;
		}
		return true;
	}
	if (strcmp("pointse", key) == 0) {
		b->pointse = strdup(val);
		if (b->pointse == NULL) {
			log_memory();
			return false;
		}
		return true;
	}

	log_gui_unknown_button_property(key);
	return false;
}

/* TYPE_CONFIGのボタンのアクティブ状態を設定する */
static void set_active_config_buttons(void)
{
	int i;

	for (i = 0; i < BUTTON_COUNT; i++) {
		if (button[i].type == TYPE_CONFIG) {
			if (button[i].key == NULL)
				continue;
			if (button[i].value == NULL)
				continue;
			if (compare_config_key_value(button[i].key,
						     button[i].value))
				button[i].rt.is_active = true;
			else
				button[i].rt.is_active = false;
		} else if (button[i].type == TYPE_FULLSCREEN) {
			if (!conf_window_fullscreen_disable) {
				button[i].rt.is_active =
					!is_full_screen_mode();
			}
		} else if (button[i].type == TYPE_WINDOW) {
			button[i].rt.is_active = is_full_screen_mode();
		}
	}
}

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
	char *file;
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
	for (i = 0; i < BUTTON_COUNT; i++) {
		process_button_point(i);
		process_button_drag(i);
		process_button_click(i);
		process_button_draw(i);
	}

	/* 右クリックでキャンセル可能な場合 */
	if (cancel_when_right_click) {
		/* 右クリックされた場合 */
		if (is_right_button_pressed) {
			/* どのボタンも選ばれなかったことにする */
			result_index = -1;

			/* GUIモードを終了する */
			stop_gui_mode();

			is_right_button_pressed = false;
			is_left_button_pressed = false;
			return true;
		}			
	}

	/* SEを再生する */
	if (!is_first_frame)
		process_play_se();

	/* ボタンが決定された場合 */
	if (result_index != -1) {
		/* 他のGUIに移動する場合 */
		if (button[result_index].type == TYPE_GUI) {
			/* GUIをロードする */
			file = strdup(button[result_index].file);
			if (file == NULL) {
				log_memory();
				return false;
			}
			stop_gui_mode();
			cleanup_gui();
			if (!prepare_gui_mode(file, cancel_when_right_click)) {
				free(file);
				return false;
			}
			free(file);
			start_gui_mode();
			return true;
		}

		/* GUIモードを終了する */
		stop_gui_mode();
		return true;
	}

	is_first_frame = false;
	return true;
}

/* ボタンのポイント状態の変化を処理する */
static void process_button_point(int index)
{
	struct gui_button *b;
	bool prev_pointed;

	b = &button[index];
	prev_pointed = b->rt.is_pointed;

	if (b->rt.is_disabled)
		return;
	if (b->type == TYPE_FULLSCREEN && !b->rt.is_active)
		return;
	if (b->type == TYPE_WINDOW && !b->rt.is_active)
		return;

	if (mouse_pos_x >= b->x && mouse_pos_x <= b->x + b->width &&
	    mouse_pos_y >= b->y && mouse_pos_y <= b->y + b->height) {
		b->rt.is_pointed = true;
		pointed_index = index;
	} else {
		b->rt.is_pointed = false;
	}

	if (prev_pointed != b->rt.is_pointed)
		is_pointed_changed = true;
}

/* ボリュームボタンのドラッグを処理する */
static void process_button_drag(int index)
{
	struct gui_button *b;

	b = &button[index];

	if (b->type != TYPE_BGMVOL && b->type != TYPE_VOICEVOL &&
	    b->type != TYPE_SEVOL && b->type != TYPE_CHARACTERVOL &&
	    b->type != TYPE_TEXTSPEED && b->type != TYPE_AUTOSPEED)
		return;

	if (!b->rt.is_dragging) {
		if (!b->rt.is_pointed)
			return;
		if (is_left_button_pressed) {
			b->rt.is_dragging = true;
			b->rt.slider = (float)(mouse_pos_x - b->x) /
				       (float)b->width;
			b->rt.slider = b->rt.slider < 0 ? 0 : b->rt.slider;
			b->rt.slider = b->rt.slider > 1.0f ? 1.0f :
				       b->rt.slider;
			if (b->type == TYPE_BGMVOL) {
				set_mixer_global_volume(BGM_STREAM,
							b->rt.slider);
			}
			return;
		}
	} else {
		b->rt.slider = (float)(mouse_pos_x - b->x) / (float)b->width;
		b->rt.slider = b->rt.slider < 0 ? 0 : b->rt.slider;
		b->rt.slider = b->rt.slider > 1.0f ? 1.0f : b->rt.slider;
		if (!is_mouse_dragging) {
			b->rt.is_dragging = false;
			if (b->type == TYPE_BGMVOL) {
				set_mixer_global_volume(BGM_STREAM,
							b->rt.slider);
			} else if (b->type == TYPE_VOICEVOL) {
				set_mixer_global_volume(VOICE_STREAM,
							b->rt.slider);
				apply_character_volume(-1);
				play_se(b->file, true);
			} else if (b->type == TYPE_SEVOL) {
				set_mixer_global_volume(SE_STREAM,
							b->rt.slider);
				play_se(b->file, false);
			} else if (b->type == TYPE_CHARACTERVOL) {
				set_character_volume(b->index, b->rt.slider);
				apply_character_volume(b->index);
				play_se(b->file, true);
			} else if (b->type == TYPE_TEXTSPEED) {
				set_text_speed(b->rt.slider);
				reset_preview_all_buttons();
			} else if (b->type == TYPE_AUTOSPEED) {
				set_auto_speed(b->rt.slider);
				reset_preview_all_buttons();
			}
			return;
		}
		if (b->type == TYPE_BGMVOL)
			set_mixer_global_volume(BGM_STREAM, b->rt.slider);
	}
}

/* ボタンのクリックを処理する */
static void process_button_click(int index)
{
	struct gui_button *b;

	b = &button[index];

	if (b->type == TYPE_BGMVOL || b->type == TYPE_VOICEVOL ||
	    b->type == TYPE_SEVOL || b->type == TYPE_CHARACTERVOL ||
	    b->type == TYPE_TEXTSPEED || b->type == TYPE_AUTOSPEED ||
	    b->type == TYPE_PREVIEW)
		return;

	if (b->rt.is_pointed && is_left_button_pressed) {
		if (b->type == TYPE_CONFIG) {
			play_se(b->clickse, false);
			set_config_key_value(b->key, b->value);
			set_active_config_buttons();
			reset_preview_all_buttons();
		} else if (b->type == TYPE_FULLSCREEN) {
			enter_full_screen_mode();
			set_active_config_buttons();
		} else if (b->type == TYPE_WINDOW) {
			leave_full_screen_mode();
			set_active_config_buttons();
		} else if (b->type == TYPE_TITLE) {
			if (title_dialog())
				result_index = index;
		} else {
			result_index = index;
		}
	}
}

/* ボタンを描画する */
static void process_button_draw(int index)
{
	struct gui_button *b;

	b = &button[index];

	if (b->type == TYPE_BGMVOL || b->type == TYPE_VOICEVOL ||
	    b->type == TYPE_SEVOL || b->type == TYPE_CHARACTERVOL ||
	    b->type == TYPE_TEXTSPEED || b->type == TYPE_AUTOSPEED)
		process_button_draw_slider(index);
	else if (b->type == TYPE_CONFIG)
		process_button_draw_config(index);
	else if (b->type == TYPE_GALLERY)
		process_button_draw_gallery(index);
	else if (b->type == TYPE_PREVIEW)
		process_button_draw_preview(index);
	else if (b->rt.is_pointed)
		draw_stage_gui_hover(b->x, b->y, b->width, b->height);
}

/* スライダーボタンを描画する */
static void process_button_draw_slider(int index)
{
	struct gui_button *b;
	int x;

	b = &button[index];

	if (b->rt.is_pointed)
		draw_stage_gui_hover(b->x, b->y, b->width, b->height);

	x = b->x + (int)((float)(b->width - b->height) * b->rt.slider);

	draw_stage_gui_active(x, b->y, b->height, b->height, b->x, b->y);
}

/* コンフィグボタンを描画する */
static void process_button_draw_config(int index)
{
	struct gui_button *b;

	b = &button[index];
	assert(b->type == TYPE_CONFIG);

	if (b->rt.is_disabled)
		return;

	if (!b->rt.is_pointed) {
		if (b->rt.is_active) {
			draw_stage_gui_active(b->x, b->y, b->width, b->height,
					      b->x, b->y);
		}
	} else {
		draw_stage_gui_hover(b->x, b->y, b->width, b->height);
	}
}

/* ギャラリーボタンを描画する */
static void process_button_draw_gallery(int index)
{
	struct gui_button *b;

	b = &button[index];
	assert(b->type == TYPE_GALLERY);

	if (b->rt.is_disabled)
		return;

	if (!b->rt.is_pointed) {
		draw_stage_gui_active(b->x, b->y, b->width, b->height,
				      b->x, b->y);
	} else {
		draw_stage_gui_hover(b->x, b->y, b->width, b->height);
	}
}

/* ボタンの状況に応じたSEを再生する */
static void process_play_se(void)
{
	if (button[result_index].type == TYPE_BGMVOL ||
	    button[result_index].type == TYPE_VOICEVOL ||
	    button[result_index].type == TYPE_SEVOL ||
	    button[result_index].type == TYPE_CHARACTERVOL)
		return;
	if (result_index != -1) {
		play_se(button[result_index].clickse, false);
		return;
	}
	if (is_pointed_changed && pointed_index != -1) {
		play_se(button[pointed_index].pointse, false);
		return;
	}
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
 * GUIの実行結果がタイトルへ戻るであるかを調べる
 */
bool is_gui_result_title(void)
{
	struct gui_button *b;

	if (result_index == -1)
		return false;

	b = &button[result_index];

	if (b->type != TYPE_TITLE)
		return false;

	return true;
}

/*
 * GUIの実行結果が終了であるかを取得する
 */
bool is_gui_result_exit(void)
{
	struct gui_button *b;

	if (result_index == -1)
		return NULL;

	b = &button[result_index];

	if (b->type != TYPE_QUIT)
		return false;

	return true;
}

/*
 * グローバル設定
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
	} else if (strcmp(key, "startse") == 0) {
		play_se(val, false);
		return true;
	}

	log_gui_unknown_global_key(key);
	return false;
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
	int wait_time;

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
		wait_time = (int)((float)button[index].rt.total_chars *
				  conf_automode_speed * get_auto_speed() *
				  1000.0f);
		if (lap >= wait_time) {
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
	for (i = 0; i < char_count; i++) {
		/* ワードラッピングを処理する */
		if (button[index].rt.is_after_space) {
			if (button[index].rt.pen_x +
			    get_en_word_width(button[index].rt.top) >=
			    button[index].width) {
				button[index].rt.pen_y +=
					conf_msgbox_margin_line;
				button[index].rt.pen_x = 0;
			}
		}
		button[index].rt.is_after_space = *button[index].rt.top == ' ';

		/* 描画する文字を取得する */
		mblen = utf8_to_utf32(button[index].rt.top, &c);
		if (mblen == -1) {
			button[index].rt.drawn_chars =
				button[index].rt.total_chars;
			return;
		}

		/* 描画する文字の幅を取得する */
		cw = get_glyph_width(c);

		/* メッセージボックスの幅を超える場合、改行する */
		if (button[index].rt.pen_x + cw >= button[index].width) {
			button[index].rt.pen_y += conf_msgbox_margin_line;
			button[index].rt.pen_x = 0;
		}

		/* 描画する */
		draw_char(index, c, &cw, &ch);

		/* 次の文字へ移動する */
		button[index].rt.pen_x += cw;
		button[index].rt.top += mblen;
		button[index].rt.drawn_chars++;
	}
}

/* 今回のフレームで描画する文字数を取得する */
static int get_frame_chars(int index)
{
	float lap;
	int char_count;

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

/* 文字を描画する */
static void draw_char(int index, uint32_t wc, int *width, int *height)
{
	lock_image(button[index].rt.img);
	draw_glyph(button[index].rt.img,
		   button[index].rt.pen_x,
		   button[index].rt.pen_y,
		   button[index].rt.color,
		   button[index].rt.outline_color,
		   wc,
		   width,
		   height);
	unlock_image(button[index].rt.img);
}

/* msgが英単語の先頭であれば、その単語の描画幅、それ以外の場合0を返す */
static int get_en_word_width(const char *m)
{
	int width;

	width = 0;
	while (isgraph((unsigned char)(*m)))
		width += get_glyph_width((unsigned char)*m++);

	return width;
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
		ST_SEMICOLON,
		ST_ERROR
	};

	char word[128], key[128];
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
			if (len == sizeof(word) - 1) {
				log_gui_parse_long_word();
				st = ST_ERROR;
				break;
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
			word[len++] = c;
			st = ST_VALUE;
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
	} else if (st != ST_SCOPE || (st == ST_SCOPE && len > 0)) {
		log_gui_parse_invalid_eof();
	}

	/* バッファを解放する */
	free(buf);

	/* ファイルをクローズする */
	close_rfile(rf);

	return st != ST_ERROR;
}
