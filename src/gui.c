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
#define DEFAULT_FILE	"default.txt"

/* ボタンの最大数 */
#define BUTTON_COUNT	(128)

/* ボタンタイプ */
enum {
	/* 無効なボタン */
	TYPE_INVALID = 0,

	/* ラベルにジャンプするボタン */
	TYPE_LABEL,

	/* ファイルにジャンプするボタン */
	TYPE_FILE,

	/* ギャラリーモードのボタン */
	TYPE_GALLERY,

	/* コンフィグのキーを変更するボタン */
	TYPE_CONFIG,

	/* ボリュームを設定するボタン */
	TYPE_BGMVOL,
	TYPE_VOICEVOL,
	TYPE_SEVOL,

	/* 別のGUIに移動するボタン */
	TYPE_GUI,

	/* GUIを終了するボタン */
	TYPE_CANCEL,

	/* アプリケーションを終了するボタン */
	TYPE_QUIT,
};

/* ボタン */
struct button {
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

	/* TYPE_LABEL, TYPE_GALLERY */
	char *label;

	/* TYPE_FILE, TYPE_BGMVOL, TYPE_VOICEVOL, TYPE_SEVOL, TYPE_GUI */
	char *file;

	/* TYPE_GALLERY */
	char *var;

	/* TYPE_CONFIG */
	char *key;

	/* TYPE_CONFIG */
	char *value;

	/* TYPE_VOLUME以外 */
	char *clickse;
	
	/* すべて */
	char *pointse;

	/*
	 * 実行時の情報
	 */

	/* 前のフレームでポイントされていたか */
	bool is_pointed;

	/* ドラッグ中か */
	bool is_dragging;
};

/* GUIモードであるか */
static bool flag_gui_mode;

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

/* ボタン */
struct button button[BUTTON_COUNT];

/*
 * 前方参照
 */
static void process_button_point(int index);
static void process_button_drag(int index);
static void process_button_click(int index);
static void process_button_draw(int index);
static void process_button_draw_volume(int index);
static void process_button_draw_gallery(int index);
static void process_play_se(void);
static bool add_button(int index);
static bool set_global_key_value(const char *key, const char *val);
static bool set_button_key_value(const int index, const char *key,
				 const char *val);
static void play_se(const char *file);
static bool load_gui_file(const char *file);

/*
 * GUIに関する初期化処理を行う
 */
bool init_gui(void)
{
	/* Android NDK用に再初期化する */
	flag_gui_mode = false;
	is_called_from_command = false;

	return true;
}

/*
 * GUIに関する終了処理を行う
 */
void cleanup_gui(void)
{
	int i;

	/* ボタンの文字列を解放する */
	for (i = 0; i < BUTTON_COUNT; i++) {
		if (button[i].label != NULL)
			free(button[i].label);
		if (button[i].file != NULL)
			free(button[i].file);
		if (button[i].var != NULL)
			free(button[i].var);
		if (button[i].key != NULL)
			free(button[i].key);
		if (button[i].value != NULL)
			free(button[i].value);
		if (button[i].clickse != NULL)
			free(button[i].clickse);
		if (button[i].pointse != NULL)
			free(button[i].pointse);
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
	int i;

	assert(!flag_gui_mode);

	/* すべてのボタンを無効にしておく */
	for (i = 0; i < BUTTON_COUNT; i++)
		button[i].type = TYPE_INVALID;

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

	/* 右クリックでキャンセルするか */
	cancel_when_right_click = cancel;

	return true;
}

/*
 * GUIを開始する
 */
void start_gui_mode(void)
{
	assert(!flag_gui_mode);

	/* GUIモードを有効にする */
	flag_gui_mode = true;
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
	for (i = 0; i < BUTTON_COUNT; i++) {
		process_button_point(i);
		process_button_drag(i);
		process_button_click(i);
		process_button_draw(i);
	}

	/* SEを再生する */
	process_play_se();

	/* ボタンが決定された場合 */
	if (result_index != -1) {
		/* GUIモードを終了する */
		stop_gui_mode();
		return true;
	}

	return true;
}

/* ボタンのポイント状態の変化を処理する */
static void process_button_point(int index)
{
	struct button *b;
	bool prev_pointed;

	b = &button[index];
	prev_pointed = b->is_pointed;

	if (mouse_pos_x >= b->x && mouse_pos_x <= b->x + b->width &&
	    mouse_pos_y >= b->y && mouse_pos_y <= b->y + b->height) {
		b->is_pointed = true;
		pointed_index = index;
	} else {
		b->is_pointed = false;
	}

	if (prev_pointed != b->is_pointed)
		is_pointed_changed = true;
}

/* ボリュームボタンのドラッグを処理する */
static void process_button_drag(int index)
{
	struct button *b;

	b = &button[index];

	if (b->type != TYPE_BGMVOL && b->type != TYPE_VOICEVOL &&
	    b->type != TYPE_SEVOL)
		return;

	/* TODO: implement volume bar, set volume, play voice, play se */
}

/* ボタンのクリックを処理する */
static void process_button_click(int index)
{
	struct button *b;

	b = &button[index];

	if (b->type == TYPE_BGMVOL || b->type == TYPE_VOICEVOL ||
	    b->type == TYPE_SEVOL)
		return;

	if (b->is_pointed && is_left_button_pressed)
		result_index = index;
}

/* ボタンを描画する */
static void process_button_draw(int index)
{
	struct button *b;

	b = &button[index];

	if (b->type == TYPE_BGMVOL || b->type == TYPE_VOICEVOL ||
	    b->type == TYPE_SEVOL)
		process_button_draw_volume(index);
	else if (b->type == TYPE_GALLERY)
		process_button_draw_gallery(index);

	if (b->is_pointed)
		draw_stage_gui_hover(b->x, b->y, b->width, b->height);
}

/* ボリュームボタンを描画する */
static void process_button_draw_volume(int index)
{
	/* TODO */
	UNUSED_PARAMETER(index);
}

/* ギャラリーボタンを描画する */
static void process_button_draw_gallery(int index)
{
	/* TODO */
	UNUSED_PARAMETER(index);
}

/* ボタンの状況に応じたSEを再生する */
static void process_play_se(void)
{
	if (result_index != -1) {
		play_se(button[result_index].clickse);
		return;
	}
	if (is_pointed_changed) {
		play_se(button[pointed_index].pointse);
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
	struct button *b;

	if (result_index == -1)
		return NULL;

	b = &button[result_index];

	if (b->type != TYPE_LABEL)
		return NULL;

	return b->label;
}

/*
 * GUIの実行結果のジャンプ先ファイルを取得する
 */
const char *get_gui_result_file(void)
{
	struct button *b;

	if (result_index == -1)
		return NULL;

	b = &button[result_index];

	if (b->type != TYPE_FILE)
		return NULL;

	return b->file;
}

/*
 * GUIの実行結果が終了であるかを取得する
 */
bool is_gui_result_exit(void)
{
	struct button *b;

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
	}

	log_gui_unknown_global_key(gui_file, key);
	return false;
}

/*
 * ボタンの作成
 */

/* ボタンを追加する */
static bool add_button(int index)
{
	/* ボタン数の上限を越えている場合 */
	if (index >= BUTTON_COUNT) {
		log_gui_too_many_buttons(gui_file);
		return false;
	}

	/* 特にボタン追加の処理はなく、数を確認しているだけ */
	return true;
}

/* ボタンのキーを設定する */
static bool set_button_key_value(const int index, const char *key,
				 const char *val)
{
	struct button *b;

	assert(index >= 0 && index < BUTTON_COUNT);

	b = &button[index];

	if (strcmp("type", key) == 0) {
		if (strcmp("label", val) == 0) {
			b->type = TYPE_LABEL;
			return true;
		}
		if (strcmp("file", val) == 0) {
			b->type = TYPE_FILE;
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
			return true;
		}
		if (strcmp("voicevol", val) == 0) {
			b->type = TYPE_VOICEVOL;
			return true;
		}
		if (strcmp("sevol", val) == 0) {
			b->type = TYPE_SEVOL;
			return true;
		}
		if (strcmp("gui", val) == 0) {
			b->type = TYPE_GUI;
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
		log_gui_unknown_button_type(gui_file, val);
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
		b->var = strdup(val);
		if (b->var == NULL) {
			log_memory();
			return false;
		}
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

	log_gui_unknown_button_property(gui_file, key);
	return false;
}

/*
 * SEの再生
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
					log_gui_parse_char(file, line, c);
					st = ST_ERROR;
					break;
				}
			}
			if (len == sizeof(word) - 1) {
				log_gui_parse_long_word(file, line);
				st = ST_ERROR;
				break;
			}
			if (c == '}' || c == ':') {
				log_gui_parse_char(file, line, c);
				st = ST_ERROR;
				break;
			}
			if (c == ' ' || c == '\t' || c == '\r' || c == '\r' ||
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
			log_gui_parse_char(file, line, c);
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
					log_gui_parse_char(file, line, c);
					st = ST_ERROR;
					break;
				}
				if (c == '}') {
					st = ST_SCOPE;
					break;
				}
			}
			if (c == '{' || c == '}') {
				log_gui_parse_char(file, line, c);
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
			log_gui_parse_char(file, line, c);
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
				log_gui_parse_char(file, line, c);
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
			log_gui_parse_char(file, line, c);
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

	/* 中途半端に終わっていた場合 */
	if (st != ST_SCOPE || (st == ST_SCOPE && len > 0)) {
		log_gui_parse_invalid_eof(file, line);
		st = ST_ERROR;
	}

	/* バッファを解放する */
	free(buf);

	/* ファイルをクローズする */
	close_rfile(rf);

	return st != ST_ERROR;
}
