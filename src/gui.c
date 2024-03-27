/* -*- coding: utf-8; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Suika2
 * Copyright (C) 2001-2023, Keiichi Tabata. All rights reserved.
 */

/*
 * GUI
 *
 * [Changes]
 *  - 2022/07/27 作成
 *  - 2023/06/11 名前変数の編集に対応
 */

#include "suika.h"
#include "wms.h"

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
	TYPE_MASTERVOL,
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

	/* 縦書き用ヒストリスクロールボタン */
	TYPE_HISTORYSCROLL_HORIZONTAL,

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

	/*WMSを実行するボタン*/
	TYPE_WMS,
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
	int margin, new_x, new_y;

	/* TYPE_GOTO, TYPE_GALLERY */
	char *label;

	/* TYPE_BGMVOL, TYPE_VOICEVOL, TYPE_SEVOL, TYPE_GUI, TYPE_FONT TYPE_WMS */
	char *file;

	/* すべてのボタン */
	char *alt;
	int order;

	/* TYPE_SAVEPAGE, TYPE_LOADPAGE, TYPE_SAVE, TYPE_LOAD, TYPE_CHARACTERVOL */
	int index;

	/* TYPE_PREVIEW, TYPE_CHAR */
	char *msg;

	/* TYPE_NAMEVAR, TYPE_CHAR */
	int namevar;

	/* TYPE_CHAR */
	int max;

	/* TYPE_*VOL以外 */
	char *clickse;

	/* すべて */
	char *pointse;

	/* TYPE_HISTORY, TYPE_SAVE, TYPE_LOAD */
	int clear_r, clear_g, clear_b;

	/* TYPE_GOTO */
	bool gosub;

	/*
	 * 実行時の情報
	 */
	struct {
		/* TYPE_FONT, TYPE_FULLSCREEN, TYPE_WINDOWのときアクティブか */
		bool is_active;

		/* TYPE_GALLERYのときボタンが無効化されているか */
		bool is_disabled;

		/* キー入力によりポイントされているか */
		bool is_selected_by_key;

		/* ドラッグ中か */
		bool is_dragging;

		/* ボリューム・スピード・スクロールバーのスライダーの値 */
		float slider;

		/* サムネイル、フォント描画用 */
		struct image *img;

		/* ヒストリのオフセット */
		int history_offset;

		/* TYPE_SAVE/TYPE_LOADのときNEWボタンを表示するか */
		bool is_new_enabled;

		/*
		 * テキストプレビューの情報
		 */

		/* オートモードの待ち時間か */
		bool is_waiting;

		/* 描画コンテキスト */
		struct draw_msg_context msg_context;

		/* 描画する文字の総数 */
		int total_chars;

		/* 前回までに描画した文字数 */
		int drawn_chars;

		/* メッセージ表示用あるいはオートモード待機用の時計 */
		uint64_t sw;
	} rt;

} button[BUTTON_COUNT];

/* GUIv2であるか */
static bool is_v2;

/* GUIのbaseイメージ */
static struct image *base_image;

/* GUIのidleイメージ */
static struct image *idle_image;

/* GUIのhoverイメージ */
static struct image *hover_image;

/* GUIのactiveイメージ */
static struct image *active_image;

/* GUIモードであるか */
static bool flag_gui_mode;

/* システムGUIであるか */
static bool is_sys_gui;

/* 右クリックでキャンセルするか */
static bool cancelable;

/* 処理中のGUIファイル */
static const char *gui_file;

/* キャンセル時のSE */
static char *cancel_se;

/* フェードイン時間 */
static float fade_in_time;

/* フェードアウト時間 */
static float fade_out_time;

/* 終了したか */
static bool is_finished;

/* ポイントされているボタンのインデックス */
static int pointed_index;

/* 前フレームでポイントされていたボタンのインデックス */
static int prev_pointed_index;

/* キー入力によりポイントされているか */
static bool is_pointed_by_key;

/* ドラッグが完了したか */
static bool is_drag_finished;

/* キー入力によりポイントされたときのマウス座標 */
static int save_mouse_pos_x, save_mouse_pos_y;

/* ドラッグが開始されたボタンのインデックス */
static int dragging_index;

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

/* SEが再生済みなのでこれ以上の再生を控えるか */
static bool suppress_se_forever;

/* ヒストリの1ページあたりのスロット数 */
static int history_slots;

/* 表示する先頭のヒストリ番号(index=0のヒストリオフセット番号) */
static int history_top;

/* ドラッグ中のヒストリのスライダー値 */
static float transient_history_slider;

/* ヒストリボタンの更新が必要か */
static bool need_update_history_buttons;

/* ヒストリボタンの更新が必要か */
static bool need_update_namevar_buttons;

/* フェードイン中であるか */
static bool is_fading_in;

/* フェードアウト中であるか */
static bool is_fading_out;

/* フェード用のストップウォッチ */
static uint64_t fade_sw;

/* 現在のフェードのアルファ */
static int cur_alpha;

/* 最初のフレームであるか */
static bool is_first_frame;

/* セーブを行ったか */
static bool did_save;

/* ロードを行ったか */
static bool did_load;

/* 制限時間 */
static float bomb_time;
static uint64_t bomb_sw;
static bool is_bombed;

/*
 * 前方参照
 */
static void process_input(void);
static void process_blit(void);
static void process_render(void);
static bool process_move(void);
static void process_se(void);
static void process_left_right_arrow_keys(void);
static bool add_button(int index);
static bool set_global_key_value(const char *key, const char *val);
static bool set_button_key_value(const int index, const char *key, const char *val);
static int get_type_for_name(const char *name);
static void update_runtime_props(bool is_first_time);
static bool move_to_other_gui(void);
static bool move_to_title(void);
static bool process_button_point(int index, bool key);
static void process_button_drag(int index);
static float calc_slider_value(int index);
static void process_button_click(int index);
static void process_button_render(int index);
static void process_button_render_slider(int index);
static void process_button_render_slider_vertical(int index);
static void process_button_render_activatable(int index);
static void process_button_render_generic(int index);
static void process_button_render_gallery(int index);
static void process_button_render_namevar(int index);
static void process_button_render_preview(int index);
static bool init_save_buttons(void);
static void update_save_buttons(void);
static void draw_save_button(int button_index);
static int draw_save_text_item(int button_index, int x, int y, const char *text, bool multiline);
static void process_save(int button_index);
static void process_load(int button_index);
static void process_button_render_save(int button_index);
static bool init_history_buttons(void);
static void draw_history_buttons(void);
static void draw_history_button(int button_index);
static void draw_history_text_item(int button_index);
static void process_button_render_history(int button_index);
static void process_history_scroll_up(void);
static void process_history_scroll_down(void);
static void process_history_scroll_at(float pos);
static void process_history_scroll_click(int index);
static void process_history_voice(int button_index);
static bool init_preview_buttons(void);
static void reset_preview_buttons(void);
static void reset_preview_button(int index);
static void draw_preview_buttons(void);
static void draw_preview_button(int index);
static void draw_preview_message(int index);
static int get_frame_chars(int index);
static int get_wait_time(int index);
static bool init_namevar_buttons(void);
static void draw_namevar_buttons(void);
static void draw_namevar_button(int index);
static void draw_name(int index);
static void process_char(int index);
static void play_se(const char *file, bool is_voice);
static void play_sys_se(const char *file);
static void speak(const char *text);
static bool run_wms(const char *file);
static bool load_gui_file(const char *file);
static void destroy_gui_images(void);
static bool load_base_image(const char *file);
static bool load_idle_image(const char *file);
static bool load_hover_image(const char *file);
static bool load_active_image(const char *file);

/* TODO: will be replaced by rendering target. */
static void process_button_draw(struct image *target, int index);
static void process_button_draw_history(struct image *target, int index);
static void process_button_draw_save(struct image *target, int index);
static void process_button_draw_slider(struct image *taget, int index);
static void process_button_draw_slider_vertical(struct image *target, int index);
static void process_button_draw_activatable(struct image *target, int index);
static void process_button_draw_preview(struct image *target, int index);
static void process_button_draw_generic(struct image *target, int index);
static void process_button_draw_gallery(struct image *target, int index);
static void process_button_draw_namevar(struct image *target, int index);

bool register_s2_functions(struct wms_runtime *rt);

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
	
	/* base, idle, hover, activeのイメージを破棄する */
	destroy_gui_images();
}

/*
 * GUIから復帰した直後かどうかを確認する
 */
bool check_gui_flag(void)
{
	if (is_sys_gui) {
		is_sys_gui = false;
		return true;
	}
	return false;
}

/*
 * GUIを準備する
 */
bool prepare_gui_mode(const char *file, bool sys)
{
	assert(!flag_gui_mode);

	/* プロパティを保存する */
	gui_file = file;
	is_sys_gui = sys;

	/* ボタンをゼロクリアする */
	memset(button, 0, sizeof(button));

	/* グローバルプロパティの初期値を設定する */
	save_page = conf_gui_save_last_page;
	save_slots = 0;
	history_slots = 0;
	history_top = -1;
	fade_in_time = 0;
	fade_out_time = 0;
	did_save = false;
	did_load = false;
	is_finished = false;
	dragging_index = -1;
	is_drag_finished = false;
	bomb_time = 0;

	/* GUIv2動作を無効にしておく(base:が指定されるとv2になる) */
	is_v2 = false;

	/* GUIファイルを開く */
	if (!load_gui_file(gui_file)) {
		cleanup_gui();
		return false;
	}

	/* イメージが揃っているか調べる */
	if (idle_image == NULL || hover_image == NULL || active_image == NULL) {
		log_gui_image_not_loaded();
		cleanup_gui();
		return false;
	}

	/* GUIv1の場合にidleイメージのアルファ値を255にする */
	if (!is_v2)
		fill_image_alpha(idle_image);

	/* TYPE_PREVIEWのボタンの初期化を行う */
	if (!init_preview_buttons()) {
		cleanup_gui();
		return false;
	}

	/* TYPE_SAVE/TYPE_LOADのボタンの初期化を行う */
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
	draw_history_buttons();

	/* TYPE_NAMEVARのボタンの初期化を行う */
	if (!init_namevar_buttons()) {
		cleanup_gui();
		return false;
	}
	draw_namevar_buttons();

	/* ボタンの状態を準備する */
	update_runtime_props(true);

	/* 右クリックでキャンセルするか */
	cancelable = false;

	/* フェードインを行うか */
	is_fading_in = fade_in_time > 0;
	is_fading_out = false;

	return true;
}

/*
 * GUIのオプションを指定する
 */
void set_gui_options(bool cancel, bool nofadein, bool nofadeout)
{
	cancelable = cancel;
	if (nofadein) {
		fade_in_time = 0;
		is_fading_in = false;
	}
	if (nofadeout) {
		fade_out_time = 0;
		is_fading_out = false;
	}
}

/*
 * GUIのロード時
 */

/* グローバルのキーと値を設定する */
static bool set_global_key_value(const char *key, const char *val)
{
	if (strcmp(key, "base") == 0) {
		if (!load_base_image(val))
			return false;
		return true;
	} else if (strcmp(key, "overlay") == 0) {
		if (!load_base_image("none"))
			return false;
		return true;
	} else if (strcmp(key, "idle") == 0) {
		if (!load_idle_image(val))
			return false;
		return true;
	} else if (strcmp(key, "hover") == 0) {
		if (!load_hover_image(val))
			return false;
		return true;
	} else if (strcmp(key, "active") == 0) {
		if (!load_active_image(val))
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
	} else if (strcmp(key, "fadein") == 0) {
		fade_in_time = (float)atof(val);
		return true;
	} else if (strcmp(key, "fadeout") == 0) {
		fade_out_time = (float)atof(val);
		return true;
	} else if (strcmp(key, "timed") == 0) {
		bomb_time = (float)atof(val);
		reset_lap_timer(&bomb_sw);
		return true;
	} else if (strcmp(key, "alt") == 0) {
		if (conf_tts_enable == 1)
			speak(val);
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
	} else if (strcmp("alt", key) == 0) {
		b->alt = strdup(val);
		if (b->alt == NULL) {
			log_memory();
			return false;
		}
	} else if (strcmp("order", key) == 0) {
		b->order = atoi(val);
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
	} else if (strcmp("msg-en", key) == 0) {
		if (conf_locale == LOCALE_EN) {
			free(b->msg);
			b->msg = strdup(val);
			if (b->msg == NULL) {
				log_memory();
				return false;
			}
		}
	} else if (strcmp("msg-fr", key) == 0) {
		if (conf_locale == LOCALE_FR) {
			free(b->msg);
			b->msg = strdup(val);
			if (b->msg == NULL) {
				log_memory();
				return false;
			}
		}
	} else if (strcmp("msg-de", key) == 0) {
		if (conf_locale == LOCALE_DE) {
			free(b->msg);
			b->msg = strdup(val);
			if (b->msg == NULL) {
				log_memory();
				return false;
			}
		}
	} else if (strcmp("msg-es", key) == 0) {
		if (conf_locale == LOCALE_ES) {
			free(b->msg);
			b->msg = strdup(val);
			if (b->msg == NULL) {
				log_memory();
				return false;
			}
		}
	} else if (strcmp("msg-it", key) == 0) {
		if (conf_locale == LOCALE_IT) {
			free(b->msg);
			b->msg = strdup(val);
			if (b->msg == NULL) {
				log_memory();
				return false;
			}
		}
	} else if (strcmp("msg-el", key) == 0) {
		if (conf_locale == LOCALE_EL) {
			free(b->msg);
			b->msg = strdup(val);
			if (b->msg == NULL) {
				log_memory();
				return false;
			}
		}
	} else if (strcmp("msg-ru", key) == 0) {
		if (conf_locale == LOCALE_RU) {
			free(b->msg);
			b->msg = strdup(val);
			if (b->msg == NULL) {
				log_memory();
				return false;
			}
		}
	} else if (strcmp("msg-zh", key) == 0) {
		if (conf_locale == LOCALE_ZH) {
			free(b->msg);
			b->msg = strdup(val);
			if (b->msg == NULL) {
				log_memory();
				return false;
			}
		}
	} else if (strcmp("msg-tw", key) == 0) {
		if (conf_locale == LOCALE_TW) {
			free(b->msg);
			b->msg = strdup(val);
			if (b->msg == NULL) {
				log_memory();
				return false;
			}
		}
	} else if (strcmp("msg-ja", key) == 0) {
		if (conf_locale == LOCALE_JA) {
			free(b->msg);
			b->msg = strdup(val);
			if (b->msg == NULL) {
				log_memory();
				return false;
			}
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
	} else if (strcmp("clear-r", key) == 0) {
		b->clear_r = atoi(val);
	} else if (strcmp("clear-g", key) == 0) {
		b->clear_g = atoi(val);
	} else if (strcmp("clear-b", key) == 0) {
		b->clear_b = atoi(val);
	} else if (strcmp("new-x", key) == 0) {
		b->new_x = atoi(val);
		b->rt.is_new_enabled = true;
	} else if (strcmp("new-y", key) == 0) {
		b->new_y = atoi(val);
		b->rt.is_new_enabled = true;
	} else if (strcmp("gosub", key) == 0) {
		b->gosub = true;
	} else if (strcmp("usearrow", key) == 0) {
		/* removed */
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
		{"mastervol", TYPE_MASTERVOL},
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
		{"historyscroll-tategaki", TYPE_HISTORYSCROLL_HORIZONTAL},
		{"default", TYPE_DEFAULT},
		{"title", TYPE_TITLE},
		{"cancel", TYPE_CANCEL},
		{"quit", TYPE_QUIT},
		{"QUIT", TYPE_QUIT}, /* typoだけど互換性のため残した */
		{"namevar", TYPE_NAMEVAR},
		{"char", TYPE_CHAR},
		{"wms", TYPE_WMS},
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
		case TYPE_MASTERVOL:
			button[i].rt.slider = get_master_volume();
			break;
		case TYPE_BGMVOL:
			button[i].rt.slider = get_mixer_global_volume(BGM_STREAM);
			break;
		case TYPE_VOICEVOL:
			button[i].rt.slider = get_mixer_global_volume(VOICE_STREAM);
			break;
		case TYPE_SEVOL:
			button[i].rt.slider = get_mixer_global_volume(SE_STREAM);
			break;
		case TYPE_CHARACTERVOL:
			button[i].rt.slider = get_character_volume(button[i].index);
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
			if (strcmp(button[i].file, conf_font_global_file) == 0)
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
		case TYPE_HISTORYSCROLL_HORIZONTAL:
			button[i].rt.slider = transient_history_slider;
			process_history_scroll_at(button[i].rt.slider);
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
	pointed_index = -1;
	result_index = -1;
	is_pointed_by_key = false;
	if (is_fading_in)
		reset_lap_timer(&fade_sw);
	is_fading_out = false;
	is_saved_in_this_frame = false;
	suppress_se = false;
	suppress_se_forever = false;

	/* 連続スワイプによるスキップ動作を無効にする */
	set_continuous_swipe_enabled(false);
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
bool run_gui_mode(void)
{
	bool chain;

	/* 時間制限を処理する */
	if (bomb_time > 0) {
		if ((float)get_lap_timer_millisec(&bomb_sw) >= bomb_time * 1000.0f) {
			is_bombed = true;
			result_index = -1;
			if (fade_out_time > 0) {
				is_fading_out = true;
				reset_lap_timer(&fade_sw);
			} else {
				is_finished = true;
				clear_input_state();
			}
		}
	}

	/* 入力を処理する */
	process_input();

	/* タイトルへ戻る場合 */
	if (!did_load && !is_bombed) {
		if (result_index != -1 &&
		    button[result_index].type == TYPE_TITLE)
			return move_to_title();
	}

	/* 画像の更新を処理する */
	if (!did_load) {
		process_blit();
	}

	/*
	 * レンダリングを行う
	 *  - ただしGUIのチェインを行う場合を除く
	 *  - 理由は、チェインイ先のGUIの読み込みでblitが発生するから
	 *  - Metalではrender後にblitができない
	 */
	chain = false;
	if (!did_load) {
		if (result_index != -1 && button[result_index].type == TYPE_GUI) {
			chain = true;
			process_se();
		} else {
			process_render();
		}
	}

	/* コマンドやGUIの移動を処理する */
	if (!did_load) {
		if (!process_move())
			return false;
	}

	/* チェイン時のレンダリングを行う */
	if (chain)
		process_render();

	/* SEを再生する */
	process_se();

	is_first_frame = false;
	return true;
}

/* 入力を処理する */
static void process_input(void)
{
	int i;

	/* フェード中の場合は入力を受け付けない */
	if (is_fading_in || is_fading_out)
		return;

	/* 制限時間に達した場合は入力を受け付けない */
	if (is_bombed)
		return;

	result_index = -1;
	prev_pointed_index = pointed_index;
	need_update_history_buttons = false;

	/* 左右キーを処理する */
	process_left_right_arrow_keys();

	/* マウスホイールか上下キーかスワイプによるスクロールを処理する */
	if (is_down_pressed) {
		if (is_swiped)
			process_history_scroll_up();
		else
			process_history_scroll_down();
		update_runtime_props(false);
	} else if (is_up_pressed) {
		if (is_swiped)
			process_history_scroll_down();
		else
			process_history_scroll_up();
		update_runtime_props(false);
	}

	/* 右クリックでキャンセル可能な場合 */
	if (cancelable) {
		/* 右クリックされた場合か、エスケープキーが押下された場合 */
		if (is_right_clicked || is_escape_pressed) {
			/* どのボタンも選ばれなかったことにする */
			result_index = -1;

			/* フェードアウトか終了処理を行う */
			if (fade_out_time > 0) {
				is_fading_out = true;
				reset_lap_timer(&fade_sw);
			} else {
				is_finished = true;
				clear_input_state();
			}
		}
	}

	/* 各ボタンについて処理する */
	is_drag_finished = false;
	for (i = 0; i < BUTTON_COUNT; i++) {
		if (!is_fading_in && !is_fading_out) {
			/* ポイント状態を更新する */
			process_button_point(i, false);

			/* ドラッグ状態を更新する */
			process_button_drag(i);

			/* クリック結果を取得する */
			process_button_click(i);
		}
	}

	/* ドラッグ終了後の処理を行う */
	if (is_drag_finished) {
		for (i = 0; i < BUTTON_COUNT; i++)
			process_button_point(i, false);
		is_drag_finished = false;
	}

	/* ボタンが決定された場合は終了する */
	if (result_index != -1) {
		if (fade_out_time > 0 &&
		    (button[result_index].type != TYPE_GUI &&
		     button[result_index].type != TYPE_TITLE)) {
			is_fading_out = true;
			reset_lap_timer(&fade_sw);
		} else {
			is_finished = true;
		}
	}

	/* 続くコマンド実行に影響を与えないようにする */
	clear_input_state();
}

/* 画像の描画を処理する */
static void process_blit(void)
{
	bool make_bg;

	/* ヒストリボタンの更新が必要な場合 */
	if (need_update_history_buttons) {
		draw_history_buttons();
		need_update_history_buttons = false;
	}

	/* 名前変数ボタンの更新を行う */
	if (need_update_namevar_buttons) {
		draw_namevar_buttons();
		need_update_namevar_buttons = false;
	}

	/* プレビューボタンの更新を行う */
	draw_preview_buttons();

	/* 条件に該当するときは終了時に背景を作成する */
	make_bg = false;
	if (is_finished) {
		if (!is_v2) {
			if (!is_sys_gui) {
				if (fade_out_time == 0) {
					if (result_index != -1) {
						switch (button[result_index].type) {
						case TYPE_GUI:
						case TYPE_TITLE:
						case TYPE_LOAD:
							make_bg = false;
							break;
						default:
							make_bg = true;
							break;
						}
					}
				}
			}
		}
	}
	if (make_bg) {
		struct image *img;
		int i;

		/* 一時的な背景を作成する */
		img = create_initial_bg();
		if (img == NULL)
			return;
		clear_stage_basic();
		set_layer_image(LAYER_BG, img);
		set_layer_file_name(LAYER_BG, NULL);
		set_layer_position(LAYER_BG, 0, 0);
		set_layer_alpha(LAYER_BG, 255);

		/* idleをコピーする */
		draw_image_copy(img, 0, 0, idle_image, idle_image->width, idle_image->height, 0, 0);

		/* 各ボタンの状態に合わせて描画する */
		for (i = 0; i < BUTTON_COUNT; i++)
			process_button_draw(img, i);

		notify_image_update(img);
	}
}

/* 描画を処理する */
static void process_render(void)
{
	float progress;
	int i;

	/* フェードイン・フェードアウト中のアルファ値を計算する */
	cur_alpha = 255;
	if (is_fading_in) {
		/* フェードインを処理する */
		progress = (float)get_lap_timer_millisec(&fade_sw) / 1000.0f / fade_in_time;
		if (progress < 1.0f) {
			/* フェードインを継続する */
			cur_alpha = (int)(progress * 255.0f);
		} else {
			/* フェードインを終了する */
			is_fading_in = false;
			cur_alpha = 255;
		}
	} else if (is_fading_out) {
		/* フェードアウトを処理する */
		progress = (float)get_lap_timer_millisec(&fade_sw) / 1000.0f / fade_out_time;
		if (progress < 1.0f) {
			/* フェードアウトを継続する */
			cur_alpha = 255 - (int)(progress * 255.0f);
		} else {
			/* フェードアウトを終了する */
			is_fading_out = false;
			is_finished = true;
			cur_alpha = 0;
		}
	}

	/* ステージをレンダリングする */
	render_stage();

	/* baseをレンダリングする */
	if (base_image != NULL)
		render_image_normal(0, 0, -1, -1, base_image, 0, 0, -1, -1, cur_alpha);

	/* GUIv2でないなら、idleを全体にレンダリングする */
	if (!is_v2)
		render_image_normal(0, 0, -1, -1, idle_image, 0, 0, -1, -1, cur_alpha);

	/* 各ボタンの状態に合わせてレンダリングする */
	for (i = 0; i < BUTTON_COUNT; i++)
		process_button_render(i);
}

/* 他のコマンドやGUIへの遷移を処理する */
static bool process_move(void)
{
	if (!is_finished)
		return true;
	if (is_fading_in || is_fading_out)
		return true;

	/* 他のGUIに移動する場合 */
	if (result_index != -1 &&
	    button[result_index].type == TYPE_GUI)
		return move_to_other_gui();

	/* キャンセルの場合 */
	if (result_index != -1 &&
	    button[result_index].type == TYPE_CANCEL) {
		/* ここでは何もしないが、@guiコマンドの場合はcmd_gui.cで後処理を行う。 */
	}

	/*
	 * ラベルへジャンプする場合:
	 *  - @guiコマンドの場合はcmd_gui.cで処理する
	 *  - ただしシステムGUIの場合はここで処理する
	 *    - gosubオプションがついている場合はpushも行う
	 */
	if (result_index != -1 &&
	    button[result_index].type == TYPE_GOTO) {
		if (is_sys_gui) {
			is_sys_gui = false;
			if (button[result_index].gosub)
				push_return_point_minus_one();
			if (!move_to_label(button[result_index].label))
				return false;
		}
	}

	/* GUIモードを終了する */
	stop_gui_mode();

	return true;
}

/* SEの再生を処理する */
static void process_se(void)
{
	/* 初回フレームの場合は再生しない */
	if (is_first_frame)
		return;

	/* SEを抑止するフレームの場合は再生しない */
	if (suppress_se) {
		suppress_se = false;
		return;
	}

	/* セーブされたフレームの場合は再生しない */
	if (is_saved_in_this_frame) {
		suppress_se = true;
		is_saved_in_this_frame = false;
		return;
	}

	/* ボタンが選択された場合 */
	if (!suppress_se_forever) {
		if (result_index == -1 && (is_fading_out || is_finished)) {
			/* キャンセルの場合 */
			play_sys_se(cancel_se);

			/* これ以上SEを再生しない */
			suppress_se_forever = true;
			return;
		} else if (result_index != -1) {
			/* 音量系のボタンの場合、SEは再生しない */
			if (button[result_index].type == TYPE_BGMVOL ||
			    button[result_index].type == TYPE_VOICEVOL ||
			    button[result_index].type == TYPE_SEVOL ||
			    button[result_index].type == TYPE_CHARACTERVOL)
				return;

			/* クリック音を再生する */
			play_sys_se(button[result_index].clickse);

			/* これ以上SEを再生しない */
			if (is_fading_out || is_finished)
				suppress_se_forever = true;
			return;
		}
	}

	/* 前フレームとは異なるボタンがポイントされた場合 */
	if (pointed_index != prev_pointed_index && pointed_index != -1)
		play_sys_se(button[pointed_index].pointse);
}

/* 左右キーを処理する */
static void process_left_right_arrow_keys(void)
{
	int i, search_start;

	search_start = -1;

	/* 右キーを処理する */
	if (is_right_arrow_pressed) {
		/* 選択されているボタンを求める */
		search_start = pointed_index + 1;

		/* 選択するボタンを探す */
		for (i = search_start; i < BUTTON_COUNT; i++)
			if (process_button_point(i, true))
				return;

		/* 検索をラップする */
		if (search_start == 0)
			return;
		for (i = 0; i < search_start; i++)
			if (process_button_point(i, true))
				return;
		return;
	}

	/* 左キーを処理する */
	if (is_left_arrow_pressed) {
		/* 選択されているボタンを求める */
		if (pointed_index == -1 || pointed_index == 0)
			search_start = BUTTON_COUNT - 1;
		else
			search_start = pointed_index - 1;

		/* 選択するボタンを探す */
		for (i = search_start; i >= 0; i--)
			if (process_button_point(i, true))
				return;

		/* 検索をラップする */
		if (search_start == BUTTON_COUNT -1)
			return;
		for (i = BUTTON_COUNT - 1; i > search_start; i--)
			if (process_button_point(i, true))
				return;
	}
}

/* 他のGUIに移動する */
static bool move_to_other_gui(void)
{
	char *file;
	bool sys;
	bool cancel;
	bool nofadeout;

	sys = is_sys_gui;
	cancel = cancelable;
	nofadeout = fade_out_time == 0;

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
	if (!prepare_gui_mode(file, sys)) {
		free(file);
		return false;
	}
	free(file);
	set_gui_options(cancel,		/* cancalation */
			true,		/* don't fade in for GUI move */
			nofadeout);	/* chain the "nofadeout" option */

	/* GUIを開始する */
	start_gui_mode();

	return true;
}

/* タイトルに移動する */
static bool move_to_title(void)
{
	const char *file;
	int i;

	file = button[result_index].file;

	/* ファイルが指定されていない場合 */
	if (file == NULL)
		return true;

	/* 現在のスクリプトに対応する既読フラグをセーブする */
	if (!save_seen())
		return false;

	/* ステージをクリアする */
	clear_stage();

	/* ローカル変数をクリアする */
	for (i = 0; i < LOCAL_VAR_SIZE; i++)
		set_variable(i, 0);

	/* スクリプトをロードする */
	if (!load_script(file))
		return false;

	/* @guiコマンドを実行中の場合はキャンセルする */
	if (is_in_command_repetition())
		stop_command_repetition();

	/*
	 * メッセージか選択肢の最中に呼ばれた場合
	 *  - この場合はシステムGUIから戻ったという扱いにしない
	 */
	if (is_sys_gui)
		is_sys_gui = false;

	/* メッセージをアクティブでなくする */
	if (is_message_active())
		clear_message_active();

	/* GUIを終了する */
	stop_gui_mode();

	/* 現在のGUIを破棄する */
	cleanup_gui();

	/* ステージをクリアする */
	clear_stage();

	/* 音声を停止する */
	for (i = 0; i < MIXER_STREAMS; i++)
		set_mixer_input(i, NULL);

	did_load = true;

	return true;
}

/*
 * ボタンのポイント
 */

/* ボタンのポイント状態の変化を処理する */
static bool process_button_point(int index, bool key)
{
	struct gui_button *b;

	b = &button[index];

	/* TYPE_INVALIDのときポイントできない(ボタンがない) */
	if (b->type == TYPE_INVALID)
		return false;

	/* TYPE_LOADのときセーブデータがないとポイントできない */
	if (b->type == TYPE_LOAD && !b->rt.is_active)
		return false;

	/* TYPE_GALLERYのとき、ボタンが無効であればポイントできない */
	if (b->rt.is_disabled)
		return false;

	/* TYPE_FULLSCREENのとき、ボタンがアクティブならポイントできない */
	if (b->type == TYPE_FULLSCREEN && b->rt.is_active)
		return false;

	/* TYPE_WINDOWのとき、ボタンがアクティブならポイントできない */
	if (b->type == TYPE_WINDOW && b->rt.is_active)
		return false;

	/* TYPE_HISTORYのとき、ボタンが非アクティブならポイントできない */
	if (b->type == TYPE_HISTORY && !b->rt.is_active)
		return false;

	/* TYPE_PREVIEWのとき、ポイントできない */
	if (b->type == TYPE_PREVIEW)
		return false;

	/* TYPE_NAMEVARのとき、ポイントできない */
	if (b->type == TYPE_NAMEVAR)
		return false;

	/* 他の項目がドラッグ中の場合 */
	if (dragging_index != -1 && dragging_index != index)
		return false;

	/* キー操作の場合 */
	if (key) {
		/* ポイントされている状態にする */
		pointed_index = index;
		is_pointed_by_key = true;
		save_mouse_pos_x = mouse_pos_x;
		save_mouse_pos_y = mouse_pos_y;

		/* 読み上げる */
		if (conf_tts_enable == 1)
			speak(button[index].alt);
		return true;
	}

	/* マウスがボタン領域に入っている場合 */
	if (mouse_pos_x >= b->x && mouse_pos_x <= b->x + b->width &&
	    mouse_pos_y >= b->y && mouse_pos_y <= b->y + b->height) {
		/* すでにキーで選択済みの項目の場合 */
		if (is_pointed_by_key && index == pointed_index)
			return false;

		/* キーで選択済みの項目があり、マウスが移動していない場合 */
		if (is_pointed_by_key &&
		    mouse_pos_x == save_mouse_pos_x &&
		    mouse_pos_y == save_mouse_pos_y)
			return false;

		/* まだポイント済みになっていない場合 */
		if (index != prev_pointed_index) {
			/* ポイントされている状態にする */
			pointed_index = index;
			is_pointed_by_key = false;
		}
		return true;
	}

	/* ポイントから外れた場合(キー選択の場合を除く) */
	if (pointed_index == index && !is_pointed_by_key)
		pointed_index = -1;

	return false;
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
	    b->type != TYPE_HISTORYSCROLL &&
	    b->type != TYPE_HISTORYSCROLL_HORIZONTAL)
		return;

	/* ドラッグ中でない場合 */
	if (!b->rt.is_dragging) {
		/* ドラッグ中でない場合 */
		if (!is_mouse_dragging)
			return;

		/* ポイントされていない場合 */
		if (pointed_index != index)
			return;

		/* すでにドラッグされている場合 */
		if (dragging_index != -1)
			return;

		/* ドラッグを開始する */
		dragging_index = index;
		b->rt.is_dragging = true;
		b->rt.slider = calc_slider_value(index);
		if (b->type == TYPE_MASTERVOL)
			set_master_volume(b->rt.slider);
		if (b->type == TYPE_BGMVOL)
			set_mixer_global_volume(BGM_STREAM, b->rt.slider);
		return;
	}

	/*
	 * ドラッグ中の場合
	 */

	/* スライダの量を設定に反映する */
	b->rt.slider = calc_slider_value(index);
	switch (b->type) {
	case TYPE_MASTERVOL:
		set_master_volume(b->rt.slider);
		break;
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
	case TYPE_HISTORYSCROLL_HORIZONTAL:
		transient_history_slider = b->rt.slider;
		break;
	default:
		break;
	}

	/* ドラッグを終了する場合 */
	if (!is_mouse_dragging) {
		b->rt.is_dragging = false;
		dragging_index = -1;
		is_drag_finished = true;

		/* 調節完了後のアクションを実行する */
		switch (b->type) {
		case TYPE_MASTERVOL:
			break;
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
			reset_preview_buttons();
			break;
		case TYPE_AUTOSPEED:
			/* テキストを再表示する */
			set_auto_speed(1.0f - b->rt.slider);
			reset_preview_buttons();
			break;
		case TYPE_HISTORYSCROLL:
		case TYPE_HISTORYSCROLL_HORIZONTAL:
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
	    b->type == TYPE_PREVIEW)
		return;

	/* ボタンがポイントされていない場合 */
	if (index != pointed_index)
		return;

	/* ボタンがクリックされていない場合 */
	if (!is_left_clicked && !is_return_pressed)
		return;

	/* 他のボタンがドラッグ中の場合 */
	if (dragging_index != -1 && dragging_index != index)
		return;

	/* ボタンのタイプごとにクリックを処理する */
	switch (b->type) {
	case TYPE_FULLSCREEN:
		play_sys_se(b->clickse);
		if (!conf_window_fullscreen_disable)
			enter_full_screen_mode();
		update_runtime_props(false);
		break;
	case TYPE_WINDOW:
		play_sys_se(b->clickse);
		if (!conf_window_fullscreen_disable)
			leave_full_screen_mode();
		update_runtime_props(false);
		break;
	case TYPE_FONT:
		play_sys_se(b->clickse);
		if (b->file != NULL)
			overwrite_config("font.file", b->file);
		update_runtime_props(false);
		reset_preview_buttons();
		break;
	case TYPE_DEFAULT:
		play_sys_se(b->clickse);
		set_text_speed(0.5f);
		set_auto_speed(0.5f);
		apply_initial_values();
		update_runtime_props(true);
		reset_preview_buttons();
		break;
	case TYPE_SAVEPAGE:
		play_sys_se(b->clickse);
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
	case TYPE_HISTORYSCROLL:
	case TYPE_HISTORYSCROLL_HORIZONTAL:
		process_history_scroll_click(index);
		break;
	case TYPE_TITLE:
		result_index = index;
		break;
	case TYPE_CHAR:
		play_sys_se(b->clickse);
		process_char(index);
		need_update_namevar_buttons = true;
		break;
	case TYPE_WMS:
		run_wms(b->file);
		break;
	default:
		result_index = index;
		break;
	}
}

/*
 * ボタンのレンダリング
 */

/* ボタンを描画する */
static void process_button_render(int index)
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
		/* スライダをレンダリングする */
		process_button_render_slider(index);
		break;
	case TYPE_FONT:
	case TYPE_FULLSCREEN:
	case TYPE_WINDOW:
	case TYPE_SAVEPAGE:
		/* アクティブ化可能ボタンをレンダリングする */
		process_button_render_activatable(index);
		break;
	case TYPE_GALLERY:
		/* ギャラリーをレンダリングする */
		process_button_render_gallery(index);
		break;
	case TYPE_SAVE:
	case TYPE_LOAD:
		/* セーブ・ロードボタンを描画する */
		process_button_render_save(index);
		break;
	case TYPE_HISTORY:
		/* ヒストリボタンを描画する */
		process_button_render_history(index);
		break;
	case TYPE_HISTORYSCROLL:
		/* 垂直スライダを描画する */
		process_button_render_slider_vertical(index);
		break;
	case TYPE_HISTORYSCROLL_HORIZONTAL:
		/* 垂直スライダを描画する */
		process_button_render_slider(index);
		break;
	case TYPE_PREVIEW:
		/* プレビューを描画する */
		process_button_render_preview(index);
		break;
	case TYPE_NAMEVAR:
		/* 名前変数ボタンを描画する */
		process_button_render_namevar(index);
		break;
	default:
		/* 一般的なボタンを描画する */
		process_button_render_generic(index);
		break;
	}
}

/* スライダーボタンをレンダリングする */
static void process_button_render_slider(int index)
{
	struct gui_button *b;
	int x;

	b = &button[index];

	if (!is_v2) {
		/* ポイントされているとき、バー部分をhover画像で描画する */
		if (index == pointed_index && !is_fading_in && !is_fading_out) {
			render_image_normal(b->x,
					    b->y,
					    b->width,
					    b->height,
					    hover_image,
					    b->x,
					    b->y,
					    b->width,
					    b->height,
					    cur_alpha);
		}

		/* 描画位置を計算する */
		x = b->x + (int)((float)(b->width - b->height) * b->rt.slider);

		/* ツマミをactive画像で描画する */
		render_image_normal(x,
				    b->y,
				    b->height,
				    b->height,
				    active_image,
				    b->x,
				    b->y,
				    b->height,
				    b->height,
				    cur_alpha);
	} else {
		/* ポイントされていないときバー部分をidle画像で描画する */
		if (index != pointed_index || is_fading_in || is_fading_out)
			render_image_normal(b->x, b->y, b->width, b->height, idle_image, b->x, b->y, b->width, b->height, cur_alpha);

		/* ポイントされているときバー部分をhover画像で描画する */
		if (index == pointed_index && !is_fading_in && !is_fading_out)
			render_image_normal(b->x, b->y, b->width, b->height, hover_image, b->x, b->y, b->width, b->height, cur_alpha);

		/* 描画位置を計算する */
		x = b->x + (int)((float)(b->width - b->height) * b->rt.slider);

		/* ツマミをactive画像で描画する */
		render_image_normal(x, b->y, b->height, b->height, active_image, b->x, b->y, b->height, b->height, cur_alpha);
	}
}

/* 垂直スライダーボタンをレンダリングする */
static void process_button_render_slider_vertical(int index)
{
	struct gui_button *b;
	int y;

	b = &button[index];

	if (!is_v2) {
		/* ポイントされているとき、バー部分をhover画像で描画する */
		if (index == pointed_index && !is_fading_in && !is_fading_out)
			render_image_normal(b->x, b->y, b->width, b->height, hover_image, b->x, b->y, b->width, b->height, cur_alpha);

		/* 描画位置を計算する */
		y = b->y + (int)((float)(b->height - b->width) * b->rt.slider);

		/* ツマミをactive画像で描画する */
		render_image_normal(b->x, y, b->width, b->width, active_image, b->x, b->y, b->width, b->width, cur_alpha);
	} else {
		/* ポイントされていないときバー部分をidle画像で描画する */
		if (index != pointed_index || is_fading_in || is_fading_out)
			render_image_normal(b->x, b->y, b->width, b->height, idle_image, b->x, b->y, b->width, b->height, cur_alpha);

		/* ポイントされているときバー部分をhover画像で描画する */
		if (index == pointed_index && !is_fading_in && !is_fading_out)
			render_image_normal(b->x, b->y, b->width, b->height, hover_image, b->x, b->y, b->width, b->height, cur_alpha);

		/* 描画位置を計算する */
		y = b->y + (int)((float)(b->height - b->width) * b->rt.slider);

		/* ツマミをactive画像で描画する */
		render_image_normal(b->x, y, b->width, b->width, active_image, b->x, b->y, b->width, b->width, cur_alpha);
	}
}

/* アクティブ化可能ボタンをレンダリングする */
static void process_button_render_activatable(int index)
{
	struct gui_button *b;

	b = &button[index];
	assert(b->type == TYPE_FONT || b->type == TYPE_FULLSCREEN ||
	       b->type == TYPE_WINDOW || b->type == TYPE_SAVEPAGE);

	/* フルスクリーンにできない場合 */
	if (conf_window_fullscreen_disable)
		if (b->type == TYPE_FULLSCREEN || b->type == TYPE_WINDOW)
			return;

	if (b->rt.is_active) {
		/* アクティブなとき、active画像を描画する */
		render_image_normal(b->x, b->y, b->width, b->height, active_image, b->x, b->y, b->width, b->height, cur_alpha);
	} else if (pointed_index == index && !is_fading_in && !is_fading_out) {
		/* ポイントされているとき、hover画像を描画する */
		render_image_normal(b->x, b->y, b->width, b->height, hover_image, b->x, b->y, b->width, b->height, cur_alpha);
	} else {
		/* それ以外のとき、GUIv2ならidle画像を描画する */
		if (is_v2)
			render_image_normal(b->x, b->y, b->width, b->height, idle_image, b->x, b->y, b->width, b->height, cur_alpha);
	}
}

/* プレビューボタンをレンダリングする */
static void process_button_render_preview(int index)
{
	struct gui_button *b;

	b = &button[index];

	render_image_normal(b->x, b->y, b->width, b->height, b->rt.img, 0, 0, b->width, b->height, cur_alpha);
}

/* 一般のボタンをレンダリングする */
static void process_button_render_generic(int index)
{
	struct gui_button *b;

	b = &button[index];

	if (is_v2) {
		/* GUIv2/GUIv1オーバレイ */
		if (index != pointed_index)
			render_image_normal(b->x, b->y, b->width, b->height, idle_image, b->x, b->y, b->width, b->height, cur_alpha);
		else
			render_image_normal(b->x, b->y, b->width, b->height, hover_image, b->x, b->y, b->width, b->height, cur_alpha);
	} else {
		/* GUIv1 */
		if (index == pointed_index)
			render_image_normal(b->x, b->y, b->width, b->height, hover_image, b->x, b->y, b->width, b->height, cur_alpha);
	}
}

/* ギャラリーボタンを描画する */
static void process_button_render_gallery(int index)
{
	struct gui_button *b;

	b = &button[index];
	assert(b->type == TYPE_GALLERY);

	if (is_v2) {
		/* GUIv2/GUIv1オーバレイ */
		if (b->rt.is_disabled) {
			/* 指定された変数が0のときはidleを描画する */
			render_image_normal(b->x, b->y, b->width, b->height, idle_image, b->x, b->y, b->width, b->height, cur_alpha);
		} else if (index != pointed_index) {
			/* ポイントされていないとき、active画像を描画する */
			render_image_normal(b->x, b->y, b->width, b->height, active_image, b->x, b->y, b->width, b->height, cur_alpha);
		} else {
			/* ポイントされているとき、hover画像を描画する */
			render_image_normal(b->x, b->y, b->width, b->height, hover_image, b->x, b->y, b->width, b->height, cur_alpha);
		}
	} else {
		/* GUIv1 */
		if (b->rt.is_disabled) {
			/*
			 * 指定された変数が0のときは描画しない
			 *  - つまり解放されていないギャラリー
			 *  - idleに無効項目を書いておく
			 */
		} else if (index != pointed_index) {
			/* ポイントされていないとき、active画像を描画する */
			render_image_normal(b->x, b->y, b->width, b->height, active_image, b->x, b->y, b->width, b->height, cur_alpha);
		} else {
			/* ポイントされているとき、hover画像を描画する */
			render_image_normal(b->x, b->y, b->width, b->height, hover_image, b->x, b->y, b->width, b->height, cur_alpha);
		}
	}
}

/* 名前変数ボタンを描画する */
static void process_button_render_namevar(int index)
{
	struct gui_button *b;

	b = &button[index];
	assert(b->type == TYPE_NAMEVAR);

	/* スクリーンへの描画を行う */
	render_image_normal(button[index].x,
			    button[index].y,
			    button[index].width,
			    button[index].height,
			    button[index].rt.img,
			    0,
			    0,
			    button[index].width,
			    button[index].height,
			    cur_alpha);
}

/*
 * 一時的な背景へのボタンの描画
 *  - TODO: レンダリングターゲットを導入して、process_button_render()と統合する
 */

static void process_button_draw(struct image *target, int index)
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
		process_button_draw_slider(target, index);
		break;
	case TYPE_FONT:
	case TYPE_FULLSCREEN:
	case TYPE_WINDOW:
	case TYPE_SAVEPAGE:
		process_button_draw_activatable(target, index);
		break;
	case TYPE_GALLERY:
		process_button_draw_gallery(target, index);
		break;
	case TYPE_SAVE:
	case TYPE_LOAD:
		process_button_draw_save(target, index);
		break;
	case TYPE_HISTORY:
		process_button_draw_history(target, index);
		break;
	case TYPE_HISTORYSCROLL:
		process_button_draw_slider_vertical(target, index);
		break;
	case TYPE_HISTORYSCROLL_HORIZONTAL:
		process_button_draw_slider(target, index);
		break;
	case TYPE_PREVIEW:
		process_button_draw_preview(target, index);
		break;
	case TYPE_NAMEVAR:
		process_button_draw_namevar(target, index);
		break;
	default:
		process_button_draw_generic(target, index);
		break;
	}
}

static void process_button_draw_history(struct image *target, int index)
{
	struct gui_button *b;

	b = &button[index];

	/* ヒストリ項目がない場合(まだヒストリが少ない場合) */
	if (b->rt.img == NULL)
		return;

	/* ポイントされているときの描画を行う */
	if (b->rt.is_active && index == result_index)
		draw_image_normal(target, b->x, b->y, hover_image, b->width, b->height, b->x, b->y, 255);

	/* テキストの描画を行う */
	draw_image_normal(target, b->x, b->y, b->rt.img, b->width, b->height, 0, 0, 255);
}

static void process_button_draw_save(struct image *target, int index)
{
	struct gui_button *b;

	b = &button[index];

	/* ポイントされているときの描画を行う */
	if (index == result_index)
		draw_image_normal(target, b->x, b->y, hover_image, b->width, b->height, b->x, b->y, 255);

	/* サムネイルとテキストの描画を行う */
	draw_image_normal(target, b->x, b->y, b->rt.img, b->width, b->height, 0, 0, 255);
}

static void process_button_draw_slider(struct image *target, int index)
{
	struct gui_button *b;
	int x;

	b = &button[index];

	/* ポイントされているとき、バー部分をhover画像で描画する */
	if (index == result_index) {
		draw_image_normal(target,
				  b->x,
				  b->y,
				  hover_image,
				  b->width,
				  b->height,
				  b->x,
				  b->y,
				  255);
	}

	/* 描画位置を計算する */
	x = b->x + (int)((float)(b->width - b->height) * b->rt.slider);

	/* ツマミをactive画像で描画する */
	draw_image_normal(target,
			  x,
			  b->y,
			  active_image,
			  b->height,
			  b->height,
			  b->x,
			  b->y,
			  255);
}

static void process_button_draw_slider_vertical(struct image *target, int index)
{
	struct gui_button *b;
	int y;

	b = &button[index];

	/* ポイントされているとき、バー部分をhover画像で描画する */
	if (index == result_index)
		draw_image_normal(target, b->x, b->y, hover_image, b->width, b->height, b->x, b->y, 255);

	/* 描画位置を計算する */
	y = b->y + (int)((float)(b->height - b->width) * b->rt.slider);

	/* ツマミをactive画像で描画する */
	draw_image_normal(target, b->x, y, active_image, b->width, b->width, b->x, b->y, 255);
}

static void process_button_draw_activatable(struct image *target, int index)
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
	if (index == result_index) {
		draw_image_normal(target, b->x, b->y, hover_image, b->width, b->height, b->x, b->y, 255);
		return;
	}

	/* コンフィグが選択されていればactive画像を描画する */
	if (b->rt.is_active)
		draw_image_normal(target, b->x, b->y, active_image, b->width, b->height, b->x, b->y, 255);
}

static void process_button_draw_preview(struct image *target, int index)
{
	struct gui_button *b;

	b = &button[index];

	draw_image_normal(target, b->x, b->y, b->rt.img, b->width, b->height, 0, 0, 255);
}

static void process_button_draw_generic(struct image *target, int index)
{
	struct gui_button *b;

	b = &button[index];

	if (index == result_index)
		draw_image_normal(target, b->x, b->y, hover_image, b->width, b->height, b->x, b->y, 255);
}

static void process_button_draw_gallery(struct image *target, int index)
{
	struct gui_button *b;

	b = &button[index];
	assert(b->type == TYPE_GALLERY);

	if (b->rt.is_disabled) {
		/*
		 * 指定された変数が0のときは描画しない
		 *  - つまり解放されていないギャラリー
		 *  - idleに無効項目を書いておく
		 */
	} else if (index != result_index) {
		/* ポイントされていないとき、active画像を描画する */
		draw_image_normal(target, b->x, b->y, active_image, b->width, b->height, b->x, b->y, 255);
	} else {
		/* ポイントされているとき、hover画像を描画する */
		draw_image_normal(target, b->x, b->y, hover_image, b->width, b->height, b->x, b->y, 255);
	}
}

static void process_button_draw_namevar(struct image *target, int index)
{
	struct gui_button *b;

	b = &button[index];
	assert(b->type == TYPE_NAMEVAR);

	draw_image_normal(target,
			  button[index].x,
			  button[index].y,
			  button[index].rt.img,
			  button[index].width,
			  button[index].height,
			  0,
			  0,
			  255);
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

	/* イメージをクリアする */
	clear_image_color(b->rt.img,
			  make_pixel(0,
				     (uint32_t)b->clear_r,
				     (uint32_t)b->clear_g,
				     (uint32_t)b->clear_b));

	/* サムネイルを描画する */
	thumb = get_save_thumbnail(save_index);
	if (thumb != NULL) {
		draw_image_copy(b->rt.img, b->margin, b->margin, thumb,
			   conf_save_data_thumb_width,
			   conf_save_data_thumb_height, 0, 0);
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
				    b->margin, text, false);

	/* 章タイトルを描画する */
	chapter = get_save_chapter_name(save_index);
	if (chapter != NULL) {
		draw_save_text_item(button_index,
				    conf_save_data_thumb_width +
				    b->margin * 2 + width,
				    b->margin,
				    chapter,
				    false);
	}

	/* 最後のメッセージを描画する */
	msg = get_save_last_message(save_index);
	if (msg) {
		draw_save_text_item(button_index,
				    conf_save_data_thumb_width + b->margin * 2,
				    b->margin + conf_msgbox_margin_line,
				    msg,
				    true);
	}

	notify_image_update(b->rt.img);
}

/* セーブデータのテキストを描画する */
static int draw_save_text_item(int button_index, int x, int y,
			       const char *text, bool multiline)
{
	struct draw_msg_context context;
	struct gui_button *b;
	pixel_t color, outline_color;
	int font_size, ruby_size, total_chars;
	int width;
	int outline_width;
	bool use_outline;

	b = &button[button_index];

	/* フォントサイズを設定する */
	font_size = conf_gui_save_font_size > 0 ?
		conf_gui_save_font_size : conf_font_size;

	/* ふちどりを選択する */
	switch (conf_gui_save_font_outline) {
	case 0: use_outline = !conf_font_outline_remove; outline_width = 2 + conf_font_outline_add; break;
	case 1: use_outline = true; outline_width = 2 + conf_font_outline_add; break;
	case 2: use_outline = false; outline_width = 1; break;
	case 3: use_outline = true; outline_width = 1; break;
	case 4: use_outline = true; outline_width = 2; break;
	case 5: use_outline = true; outline_width = 3; break;
	case 6: use_outline = true; outline_width = 4; break;
	default: use_outline = false; outline_width = 4; break;
	}

	/* ルビのフォントサイズを求める */
	ruby_size = conf_gui_save_font_ruby_size > 0 ?
		conf_gui_save_font_ruby_size :
		conf_font_ruby_size > 0 ?
		conf_font_ruby_size :
		conf_font_size / 5;

	/* 色を決定する */
	color = make_pixel(0xff,
			   (uint32_t)conf_font_color_r,
			   (uint32_t)conf_font_color_g,
			   (uint32_t)conf_font_color_b);
	outline_color = make_pixel(0xff,
				   (uint32_t)conf_font_outline_color_r,
				   (uint32_t)conf_font_outline_color_g,
				   (uint32_t)conf_font_outline_color_b);

	/* 描画する */
	construct_draw_msg_context(
		&context,
		-1,		/* Not for a layer: use an alternative image. */
		text,
		conf_gui_save_font_select,
		font_size,
		font_size,
		ruby_size,
		use_outline,
		outline_width,
		x,
		y,
		b->width,
		b->height,
		x,		/* left_margin */
		b->margin,	/* right_margin */
		0,		/* top_margin */
		0,		/* bottom_margin */
		multiline ? conf_msgbox_margin_line : 0,
		conf_msgbox_margin_char,
		color,
		outline_color,
		false,		/* is_dimming */
		true,		/* ignore_linefeed */
		true,		/* ignore_font */
		true,		/* ignore_outline */
		true,		/* ignore_color */
		true,		/* ignore_size */
		true,		/* ignore_position */
		!conf_gui_ruby,
		true,		/* ignore_wait */
		false,		/* fill_bg */
		NULL,		/* inline_wait_hook */
		conf_gui_save_tategaki);
	set_alternative_target_image(&context, b->rt.img);
	total_chars = count_chars_common(&context);
	draw_msg_common(&context, total_chars);

	width = get_string_width(context.font, context.font_size, text);
	return width;
}

/* セーブを行う */
static void process_save(int button_index)
{
	int data_index;

	/* ボタン番号からセーブデータ番号に変換する */
	data_index = save_page * save_slots + button[button_index].index;
	if (get_save_date(data_index) != 0)
		return;

	/* SEを再生する */
	play_sys_se(button[button_index].clickse);

	/* "@gui save.txt"の場合は、次のコマンドに進めておく */
	if (!is_sys_gui && !did_save) {
		if (!move_to_next_command())
			return;
		did_save = true;
	}

	/* セーブを実行する */
	execute_save(data_index);

	/* 最後にセーブしたページを保存する */
	conf_gui_save_last_page = save_page;

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

	/* 最後にロードしたページを保存する */
	conf_gui_save_last_page = save_page;

	did_load = true;
	if (is_in_command_repetition())
		stop_command_repetition();
	stop_gui_mode();
}

/* セーブ・ロードボタンの描画を行う */
static void process_button_render_save(int index)
{
	struct gui_button *b;

	b = &button[index];

	if (is_v2) {
		if (index != pointed_index)
			render_image_normal(b->x, b->y, b->width, b->height, idle_image, b->x, b->y, b->width, b->height, cur_alpha);
		else
			render_image_normal(b->x, b->y, b->width, b->height, hover_image, b->x, b->y, b->width, b->height, cur_alpha);
	} else {
		/* ポイントされているときの描画を行う */
		if (index == pointed_index)
			render_image_normal(b->x, b->y, b->width, b->height, hover_image, b->x, b->y, b->width, b->height, cur_alpha);
	}

	/* サムネイルとテキストの描画を行う */
	render_image_normal(b->x, b->y, b->width, b->height, b->rt.img, 0, 0, b->width, b->height, cur_alpha);

	/* NEW画像を表示する */
	if (b->rt.is_new_enabled &&
	    get_latest_save_index() == save_page * save_slots + b->index)
		render_savenew(b->new_x, b->new_y, cur_alpha);
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

/* ヒストリボタンの描画を行う */
static void process_button_render_history(int index)
{
	struct gui_button *b;

	b = &button[index];

	/* ヒストリ項目がない場合(まだヒストリが少ない場合) */
	if (b->rt.img == NULL)
		return;

	if (is_v2) {
		if (b->rt.is_active && index == pointed_index)
			render_image_normal(b->x, b->y, b->width, b->height, hover_image, b->x, b->y, b->width, b->height, cur_alpha);
		else
			render_image_normal(b->x, b->y, b->width, b->height, idle_image, b->x, b->y, b->width, b->height, cur_alpha);
	} else {
		/* ポイントされているときの描画を行う */
		if (b->rt.is_active && index == pointed_index)
			render_image_normal(b->x, b->y, b->width, b->height, hover_image, b->x, b->y, b->width, b->height, cur_alpha);
	}

	/* テキストの描画を行う */
	render_image_normal(b->x, b->y, b->width, b->height, b->rt.img, 0, 0, b->width, b->height, cur_alpha);
}

/* すべてのヒストリのスロットを描画する */
static void draw_history_buttons(void)
{
	int i;

	for (i = 0; i < BUTTON_COUNT; i++) {
		if (button[i].type != TYPE_HISTORY)
			continue;
		if (button[i].rt.img == NULL)
			continue;

		draw_history_button(i);
	}
}

/* ヒストリのスロットを描画する */
static void draw_history_button(int index)
{
	struct gui_button *b;
	int history_count;

	b = &button[index];

#if defined(SUIKA_TARGET_WASM)
	/* フォント描画に時間がかかる場合のために、サウンドバッファのフィルを行う */
	void fill_sound_buffer(void);
	fill_sound_buffer();
#endif

	/* ヒストリのオフセットを計算する */
	history_count = get_history_count();
	if (history_count < history_slots)
		b->rt.history_offset = history_count - b->index - 1;
	else
		b->rt.history_offset = history_top - b->index;
	if (b->rt.history_offset < 0)
		b->rt.history_offset = -1;

	/* スロットのアクティブ状態を求める */
	if (b->rt.history_offset >= 0 &&
	    b->rt.history_offset < get_history_count()) {
		if (get_history_voice(b->rt.history_offset) != NULL)
			b->rt.is_active = true;
		else if (conf_tts_enable)
			b->rt.is_active = true;
		else
			b->rt.is_active = false;
	} else {
		b->rt.is_active = false;
	}

	/* イメージをクリアする */
	clear_image_color(b->rt.img,
			  make_pixel(0,
				     (uint32_t)b->clear_r,
				     (uint32_t)b->clear_g,
				     (uint32_t)b->clear_b));

	/* メッセージを描画する */
	if (b->rt.history_offset != -1)
		draw_history_text_item(index);

	notify_image_update(b->rt.img);
}

/* ヒストリのテキストを描画する */
static void draw_history_text_item(int button_index)
{
	struct draw_msg_context context;
	struct gui_button *b;
	const char *text;
	pixel_t color, outline_color;
	int font_size, ruby_size, margin_line, total_chars;
	int pen_x, pen_y;
	int outline_width;
	bool use_outline, ignore_color;

	b = &button[button_index];

	/* メッセージを取得する */
	text = get_history_message(b->rt.history_offset);

	/* フォントサイズを取得する */
	font_size = conf_gui_history_font_size > 0 ?
		conf_gui_history_font_size : conf_font_size;

	/* ふちどりを選択する */
	switch (conf_gui_history_font_outline) {
	case 0: use_outline = !conf_font_outline_remove; outline_width = 2 + conf_font_outline_add; break;
	case 1: use_outline = true; outline_width = 2 + conf_font_outline_add; break;
	case 2: use_outline = false; outline_width = 1; break;
	case 3: use_outline = true; outline_width = 1; break;
	case 4: use_outline = true; outline_width = 2; break;
	case 5: use_outline = true; outline_width = 3; break;
	case 6: use_outline = true; outline_width = 4; break;
	default: use_outline = false; outline_width = 4; break;
	}

	/* 色を選択する */
	switch (conf_gui_history_disable_color) {
	case 0:
		/*
		 * Enable colors: a initial color is inserted to the
		 * string head as an escape sequece.
		 */
		ignore_color = false;
		color = make_pixel(0xff,
				   (pixel_t)conf_font_color_r,
				   (pixel_t)conf_font_color_g,
				   (pixel_t)conf_font_color_b);
		outline_color = make_pixel(0xff,
					   (pixel_t)conf_font_outline_color_r,
					   (pixel_t)conf_font_outline_color_g,
					   (pixel_t)conf_font_outline_color_b);
		break;
	case 1:
		/* Use font.color */
		ignore_color = true;
		color = make_pixel(0xff,
				   (pixel_t)conf_font_color_r,
				   (pixel_t)conf_font_color_g,
				   (pixel_t)conf_font_color_b);
		outline_color = make_pixel(0xff,
					   (pixel_t)conf_font_outline_color_r,
					   (pixel_t)conf_font_outline_color_g,
					   (pixel_t)conf_font_outline_color_b);
		break;
	case 2:
		/* Use gui.history.font.color */
		ignore_color = true;
		color = make_pixel(0xff,
				   (pixel_t)conf_gui_history_font_color_r,
				   (pixel_t)conf_gui_history_font_color_g,
				   (pixel_t)conf_gui_history_font_color_b);
		outline_color = make_pixel(0xff,
					   (pixel_t)conf_gui_history_font_outline_color_r,
					   (pixel_t)conf_gui_history_font_outline_color_g,
					   (pixel_t)conf_gui_history_font_outline_color_b);
		break;
	default:
		/* A strage value, but ignore for now. */
		ignore_color = false;
		color = outline_color = 0;
		break;
	}

	/* ルビのフォントサイズを求める */
	ruby_size = conf_gui_history_font_ruby_size > 0 ?
		conf_gui_history_font_ruby_size :
		conf_font_ruby_size > 0 ?
		conf_font_ruby_size : conf_font_size / 5;

	/* 行間マージンを求める */
	margin_line = conf_gui_history_margin_line > 0 ?
		conf_gui_history_margin_line : conf_msgbox_margin_line;

	/* ペン位置を計算する */
	if (!conf_gui_history_tategaki) {
		pen_x = b->margin;
		pen_y = b->margin;
	} else {
		pen_x = b->width - b->margin - font_size;
		pen_y = b->margin;
	}

	/* 描画する */
	construct_draw_msg_context(
		&context,
		-1,		/* Not for a layer: use an alternative image. */
		text,
		conf_gui_history_font_select,
		font_size,
		font_size,
		ruby_size,
		use_outline,
		outline_width,
		pen_x,
		pen_y,
		b->width,	/* area_width */
		b->height,	/* area_height */
		b->margin,	/* left_margin */
		b->margin,	/* right_margin */
		b->margin,	/* top_margin */
		b->margin,	/* bottom_margin */
		margin_line,	/* line_margin */
		conf_msgbox_margin_char,
		color,
		outline_color,
		false,		/* is_dimming */
		conf_gui_history_oneline,	/* ignore_linefeed */
		false,		/* ignore_font */
		false,		/* ignore_outline */
		ignore_color,	/* ignore_color */
		true,		/* ignore_size */
		true,		/* ignore_position */
		!conf_gui_ruby,
		true,		/* ignore_wait */
		false,		/* fill_bg */
		NULL,		/* inline_wait_hook */
		conf_gui_history_tategaki);
	set_alternative_target_image(&context, b->rt.img);
	total_chars = count_chars_common(&context);
	draw_msg_common(&context, total_chars);
}

/* 上スクロールを処理する */
static void process_history_scroll_up(void)
{
	int history_count, old_history_top;
	float pos;

	old_history_top = history_top;

	/* ヒストリの先頭位置を計算する */
	history_count = get_history_count();
	history_top++;
	if (history_top >= history_count)
		history_top = history_count - 1;

	/* スクロール位置を計算する */
	pos = 1.0f - (float)(history_top - (history_slots - 1)) / (float)((history_count - 1) - (history_slots - 1));
	if (pos < 0)
		pos = 0;
	if (pos > 1.0f)
		pos = 1.0f;

	/* スライダの位置を補正する */
	transient_history_slider = pos;

	/* 再描画が必要な場合 */
	if (history_top != old_history_top)
		need_update_history_buttons = true;
}

/* 下スクロールを処理する */
static void process_history_scroll_down(void)
{
	int history_count, old_history_top;
	float pos;

	old_history_top = history_top;

	/* ヒストリの先頭位置を計算する */
	history_count = get_history_count();
	history_top--;
	if (history_top < 0)
		history_top = 0;

	/* スクロール位置を計算する */
	pos = 1.0f - (float)(history_top - (history_slots - 1)) / (float)((history_count - 1) - (history_slots - 1));
	if (pos < 0)
		pos = 0;
	if (pos > 1.0f)
		pos = 1.0f;

	/* スライダの位置を補正する */
	transient_history_slider = pos;

	/* 再描画が必要な場合 */
	if (history_top != old_history_top)
		need_update_history_buttons = true;
}

/* ヒストリーバーのクリックによるスクロールを処理する */
static void process_history_scroll_at(float pos)
{
	int history_count, old_history_top;

	old_history_top = history_top;

	/* ヒストリの数がスロットの数より小さい場合 */
	history_count = get_history_count();
	if (history_count <= history_slots)
		return;

	/* スクロール位置を計算する */
	if (history_count <= history_slots)
		history_top = history_count - 1;
	else
		history_top = (int)(((float)((history_count - 1) - (history_slots - 1)) * (1.0f - pos)) + (float)(history_slots - 1) + 0.5f);

	/* スライダの位置を補正する */
	transient_history_slider = pos;

	/* 再描画が必要な場合 */
	if (history_top != old_history_top)
		need_update_history_buttons = true;
}

/* ヒストリースクロールバーのクリックを処理する */
static void process_history_scroll_click(int index)
{
	struct gui_button *b;

	b = &button[index];
	b->rt.slider = calc_slider_value(index);
	process_history_scroll_at(b->rt.slider);
}

/* ヒストリのボイスを再生する */
static void process_history_voice(int button_index)
{
	const char *voice;

	if (button[button_index].rt.history_offset == -1)
		return;

	/* その他のキャラクタのボリュームを適用する */
	apply_character_volume(CH_VOL_SLOT_DEFAULT);

	/* ボイスを再生する */
	voice = get_history_voice(button[button_index].rt.history_offset);
	if (voice != NULL)
		play_se(voice, true);
	else if (conf_tts_enable)
		speak(get_history_message(button[button_index].rt.history_offset));
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
	}

	reset_preview_buttons();

	return true;
}

/* テキストプレビューをリセットする */
static void reset_preview_buttons(void)
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
	struct gui_button *b;
	pixel_t color, outline_color;
	int pen_x, pen_y;

	assert(button[index].type == TYPE_PREVIEW);

	b = &button[index];

	clear_image_color(b->rt.img, make_pixel(0, 0, 0, 0));
	notify_image_update(b->rt.img);

	color = make_pixel(0xff,
			   (pixel_t)conf_font_color_r,
			   (pixel_t)conf_font_color_g,
			   (pixel_t)conf_font_color_b);
	outline_color = make_pixel(0xff,
				   (pixel_t)conf_font_outline_color_r,
				   (pixel_t)conf_font_outline_color_g,
				   (pixel_t)conf_font_outline_color_b);

	/* ペン位置を計算する */
	if (!conf_gui_preview_tategaki) {
		pen_x = 0;
		pen_y = 0;
	} else {
		pen_x = b->width - conf_font_size;
		pen_y = 0;
	}

	construct_draw_msg_context(
		&b->rt.msg_context,
		-1,		/* Not for a layer: use an alternative image. */
		b->msg,
		conf_font_select,
		conf_font_size,
		conf_font_size,
		conf_font_ruby_size,
		!conf_font_outline_remove,
		2 + conf_font_outline_add,
		pen_x,
		pen_y,
		b->width,
		b->height,
		0,		/* left_margin */
		0,		/* right_margin */
		0,		/* top_margin */
		0,		/* bottom_margin */
		conf_msgbox_margin_line,
		conf_msgbox_margin_char,
		color,
		outline_color,
		false,		/* is_dimming */
		false,		/* ignore_linefeed */
		false,		/* ignore_font */
		false,		/* ignore_outline */
		false,		/* ignore_color */
		false,		/* ignore_size */
		false,		/* ignore_position */
		false,		/* ignore_ruby */
		true,		/* ignore_wait */
		false,		/* fill_bg */
		NULL,		/* inline_wait_hook */
		conf_gui_preview_tategaki);
	set_alternative_target_image(&b->rt.msg_context, b->rt.img);

	b->rt.is_waiting = false;
	b->rt.total_chars = count_chars_common(&b->rt.msg_context);
	b->rt.drawn_chars = 0;
	reset_lap_timer(&b->rt.sw);
}

/* テキストプレビューの描画(更新)を行う */
static void draw_preview_buttons(void)
{
	int i;

	for (i = 0; i < BUTTON_COUNT; i++) {
		if (button[i].type != TYPE_PREVIEW)
			continue;

		assert(button[i].msg != NULL);

		draw_preview_button(i);
	}
}

/* テキストプレビューのボタンの描画を行う */
static void draw_preview_button(int index)
{
	struct gui_button *b;
	uint64_t lap;

	b = &button[index];

	/* メッセージの途中の場合 */
	if (!b->rt.is_waiting) {
		/* メインメモリ上のイメージの描画を行う */
		draw_preview_message(index);

		/* すべての文字を描画し終わった場合 */
		if (b->rt.drawn_chars == b->rt.total_chars) {
			/* ストップウォッチを初期化する */
			reset_lap_timer(&b->rt.sw);

			/* オートモードの待ち時間に入る */
			b->rt.is_waiting = true;
		}
	} else {
		/* オートモードの待ち時間の場合 */
		lap = get_lap_timer_millisec(&b->rt.sw);
		if (lap >= (uint64_t)get_wait_time(index)) {
			/* 待ちを終了する */
			reset_preview_button(index);
		}
	}
}

/* メッセージの描画を行う */
static void draw_preview_message(int index)
{
	struct gui_button *b;
	int char_count;
	int ret_chars;

	b = &button[index];

	/* 今回のフレームで描画する文字数を取得する */
	char_count = get_frame_chars(index);
	if (char_count == 0)
		return;

	/* 描画する */
	ret_chars = draw_msg_common(&b->rt.msg_context, char_count);
	notify_image_update(b->rt.img);

	/* 描画した文字数を加算する */
	b->rt.drawn_chars += ret_chars;
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
	lap = (float)get_lap_timer_millisec(&button[index].rt.sw) / 1000.0f;

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

		clear_image_color(button[i].rt.img, make_pixel(0, 0, 0, 0));
		notify_image_update(button[i].rt.img);
	}

	return true;
}

/* すべての名前変数のボタンを描画する */
static void draw_namevar_buttons(void)
{
	int i;

	for (i = 0; i < BUTTON_COUNT; i++)
		if (button[i].type == TYPE_NAMEVAR)
			draw_namevar_button(i);
}

/* 名前変数のボタンを描画する */
static void draw_namevar_button(int index)
{
	clear_image_color(button[index].rt.img, make_pixel(0, 0, 0, 0));
	draw_name(index);
}

/* 名前の描画を行う */
static void draw_name(int index)
{
	struct draw_msg_context context;
	struct gui_button *b;
	const char *name;
	pixel_t color, outline_color;
	int char_count;

	b = &button[index];

	/* 色を取得する */
	color = make_pixel(0xff,
			   (pixel_t)conf_font_color_r,
			   (pixel_t)conf_font_color_g,
			   (pixel_t)conf_font_color_b);
	outline_color = make_pixel(0xff,
				   (pixel_t)conf_font_outline_color_r,
				   (pixel_t)conf_font_outline_color_g,
				   (pixel_t)conf_font_outline_color_b);

	/* 描画する文字列を取得する */
	name = get_name_variable(b->namevar);
	assert(name != NULL);

	/* 描画する */
	construct_draw_msg_context(
		&context,
		-1,		/* Not for a layer: use an alternative image. */
		name,
		conf_font_select,
		conf_font_size,
		conf_font_size,
		conf_font_ruby_size,
		!conf_font_outline_remove,
		2 + conf_font_outline_add,
		0,		/* pen_x */
		0,		/* pen_y */
		b->width,
		b->height,
		0,		/* left_margin */
		0,		/* right_margin */
		0,		/* top_margin */
		0,		/* bottom_margin */
		0,		/* line_margin */
		conf_msgbox_margin_char,
		color,
		outline_color,
		false,		/* is_dimming */
		false,		/* ignore_linefeed */
		false,		/* ignore_font */
		false,		/* ignore_outline */
		false,		/* ignore_color */
		false,		/* ignore_size */
		false,		/* ignore_position */
		false,		/* ignore_ruby */
		true,		/* ignore_wait */
		false,		/* fill_bg */
		NULL,		/* inline_wait_hook */
		false);		/* use_tategaki */
	set_alternative_target_image(&context, b->rt.img);
	char_count = count_chars_common(&context);
	draw_msg_common(&context, char_count);
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
	if (count_utf8_chars(orig) >= b->max)
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

	if (is_fading_in)
		return NULL;

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

	if (is_fading_in)
		return false;

	if (result_index == -1)
		return false;

	b = &button[result_index];

	if (b->type != TYPE_QUIT)
		return false;

	return true;
}

/*
 * GUIでセーブされたか
 */
bool is_gui_saved(void)
{
	if (did_save)
		return true;

	return false;
}

/*
 * GUIでロードされたか
 */
bool is_gui_loaded(void)
{
	if (did_load)
		return true;

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

/* システムSEを再生する */
static void play_sys_se(const char *file)
{
	struct wave *w;

	if (file == NULL || strcmp(file, "") == 0)
		return;

	w = create_wave_from_file(SE_DIR, file, false);
	if (w == NULL)
		return;

	set_mixer_input(SYS_STREAM, w);
}

/* ボタンの代替テキストを読み上げる */
static void speak(const char *text)
{
	if (text != NULL)
		speak_text(text);
}

/* WMSを実行する */
static bool run_wms(const char *file)
{
	struct rfile *rf;
	struct wms_runtime *rt;
	size_t len;
	char *script;

	/* スクリプトファイルを開いてすべて読み込む */
	rf = open_rfile(WMS_DIR, file, false);
	if (rf == NULL)
		return false;
	len = get_rfile_size(rf);
	script = malloc(len + 1);
	if (script == NULL) {
		log_memory();
		return false;
	}
	if (read_rfile(rf, script, len) != len) {
		log_file_read(WMS_DIR, file);
		return false;
	}
	close_rfile(rf);
	script[len] = '\0';

	/* パースしてランタイムを作成する */
	rt = wms_make_runtime(script);
	if (rt == NULL) {
		log_wms_syntax_error(file, wms_get_parse_error_line(),
				     wms_get_parse_error_column());
		return false;
	}

	/* ランタイムにFFI関数を登録する */
	if (!register_s2_functions(rt))
		return false;

		/* WMSを実行する */
	if (!wms_run(rt)) {
		log_wms_runtime_error(get_string_param(WMS_PARAM_FILE),
				      wms_get_runtime_error_line(rt),
				      wms_get_runtime_error_message(rt));
		return false;
		}

	return true;
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

/*
 * moved from stage.c
 */

/* GUIの画像を削除する */
static void destroy_gui_images(void)
{
	if (base_image != NULL) {
		destroy_image(base_image);
		base_image = NULL;
	}
	if (idle_image != NULL) {
		destroy_image(idle_image);
		idle_image = NULL;
	}
	if (hover_image != NULL) {
		destroy_image(hover_image);
		hover_image = NULL;
	}
	if (active_image != NULL) {
		destroy_image(active_image);
		active_image = NULL;
	}
}

/* GUIのbase画像を読み込む */
static bool load_base_image(const char *file)
{
	if (base_image != NULL) {
		destroy_image(base_image);
		base_image = NULL;
	}

	is_v2 = true;

	if (strcmp(file, "none") == 0)
		return true;

	base_image = create_image_from_file(CG_DIR, file);
	if (base_image == NULL)
		return false;

	return true;
}

/* GUIのidle画像を読み込む */
static bool load_idle_image(const char *file)
{
	if (idle_image != NULL) {
		destroy_image(idle_image);
		idle_image = NULL;
	}

	idle_image = create_image_from_file(CG_DIR, file);
	if (idle_image == NULL)
		return false;

	return true;
}

/* GUIのhover画像を読み込む */
static bool load_hover_image(const char *file)
{
	if (hover_image != NULL) {
		destroy_image(hover_image);
		hover_image = NULL;
	}

	hover_image = create_image_from_file(CG_DIR, file);
	if (hover_image == NULL)
		return false;

	return true;
}

/* GUIのactive画像を読み込む */
static bool load_active_image(const char *file)
{
	if (active_image != NULL) {
		destroy_image(active_image);
		active_image = NULL;
	}

	active_image = create_image_from_file(CG_DIR, file);
	if (active_image == NULL)
		return false;

	return true;
}
