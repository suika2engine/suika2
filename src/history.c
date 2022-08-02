/* -*- coding: utf-8-with-signature; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Suika 2
 * Copyright (C) 2001-2016, TABATA Keiichi. All rights reserved.
 */

/*
 * ヒストリ画面の実行
 *
 * [Changes]
 *  - 2016/07/09 作成
 */

#include "suika.h"

/* Unicodeコードポイント */
#define CHAR_SPACE	(0x0020)
#define CHAR_COMMA 	(0x002c)
#define CHAR_PERIOD	(0x002e)
#define CHAR_COLON	(0x003a)
#define CHAR_SEMICOLON  (0x003b)
#define CHAR_TOUTEN	(0x3001)
#define CHAR_KUTEN	(0x3002)
#define CHAR_BACKSLASH	(0x005c)
#define CHAR_YENSIGN	(0x00a5)
#define CHAR_SMALLN	(0x006e)

/* テキストのサイズ(名前とメッセージを連結するため) */
#define TEXT_SIZE	(1024)

/* 表示する履歴の数 */
#define HISTORY_SIZE	(100)

/* ヒストリ項目 */
static struct history {
	char *text;
	char *voice;
	int y_top;
	int y_bottom;
} history[HISTORY_SIZE];

/* ヒストリ項目の個数 */
static int history_count;

/* ヒストリ項目の先頭 */
static int history_index;

/* ヒストリ画面がキャンセルされた直後であるか */
static bool history_flag;

/* ヒストリ画面が有効であるか */
static bool is_history_mode_enabled;

/* 最初の描画であるか */
static bool is_first_frame;

/* 描画開始項目のオフセット(0 <= start_offset < HISTORY_COUNT) */
static int start_offset;

/* 描画された最初の項目 */
static int view_start;

/* 描画された最後の項目 */
static int view_end;

/* ポイントされているヒストリ項目 */
static int pointed_index;

/* 前方参照 */
static void draw_page(int *x, int *y, int *w, int *h);
static bool draw_message(int *pen_x, int *pen_y, int index);
static int get_en_word_width(const char *text);
static void update_pointed_index(void);
static void play_voice(void);
static void play_se(const char *file);

/*
 * 初期化
 */

/*
 * ヒストリに関する初期化処理を行う
 */
bool init_history(void)
{
	/* Android NDK用に初期化を行う */
	clear_history();
	history_flag = false;
	is_history_mode_enabled = false;

	return true;
}

/*
 * ヒストリに関する終了処理を行う
 */
void cleanup_history(void)
{
	clear_history();
}

/*
 * コマンドからのメッセージ登録
 */

/*
 * メッセージを登録する
 */
bool register_message(const char *name, const char *msg, const char *voice)
{
	char text[TEXT_SIZE];
	struct history *h;

	/* 格納位置を求める */
	h = &history[history_index];

	/* 以前の情報を消去する */
	if (h->text != NULL) {
		free(h->text);
		h->text = NULL;
	}
	if (h->voice != NULL) {
		free(h->voice);
		h->voice = NULL;
	}

	/* ボイスが指定されている場合 */
	if (voice != NULL && strcmp(voice, "") != 0) {
		h->voice = strdup(voice);
		if (h->voice == NULL) {
			log_memory();
			return false;
		}
	}

	/* 名前が指定されいる場合 */
	if (name != NULL) {
		/* "名前「メッセージ」"の形式に連結して保存する */
		if (!conf_i18n) {
			snprintf(text, TEXT_SIZE, "%s%c%c%c%s%c%c%c", name,
				 0xe3, 0x80, 0x8c, msg, 0xe3, 0x80, 0x8d);
		} else {
			snprintf(text, TEXT_SIZE, "%s: %s", name, msg);
		}
		h->text = strdup(text);
		if (h->text == NULL) {
			log_memory();
			return false;
		}
	} else {
		/* メッセージのみを保存する */
		h->text = strdup(msg);
		if (h->text == NULL) {
			log_memory();
			return false;
		}
	}

	/* 格納位置を更新する */
	history_index = (history_index + 1) % HISTORY_SIZE;
	history_count = (history_count + 1) >= HISTORY_SIZE ? HISTORY_SIZE :
			(history_count + 1);

	return true;
}

/*
 * ロード時のクリア
 */

/*
 * ヒストリをクリアする
 */
void clear_history(void)
{
	int i;

	for (i = 0; i < HISTORY_SIZE; i++) {
		if (history[i].text != NULL) {
			free(history[i].text);
			history[i].text = NULL;
		}

		if (history[i].voice != NULL) {
			free(history[i].voice);
			history[i].voice = NULL;
		}
	}

	history_count = 0;
	history_index = 0;
}

/*
 * コマンドからの確認
 */

/*
 * ヒストリ画面から復帰したかばかりかを確認する
 */
bool check_history_flag(void)
{
	bool ret;

	ret = history_flag;
	history_flag = false;

	return ret;
}

/*
 * ヒストリが空であるかを返す
 */
bool is_history_empty(void)
{
	if (history_count == 0)
		return true;

	return false;
}

/*
 * ヒストリ画面
 */

/*
 * ヒストリ画面を開始する
 */
void start_history_mode(void)
{
	/* オートモードを解除する */
	if (is_auto_mode())
		stop_auto_mode();

	/* ヒストリ画面を開始する */
	is_history_mode_enabled = true;

	/* 最初のフレームである */
	is_first_frame = true;

	/* 選択項目を無効とする */
	pointed_index = -1;

	/* 表示位置をクリアする */
	start_offset = 0;

	/* FOレイヤにステージを描画する */
	draw_history_fo();
}

/* セーブ画面を終了する */
static void stop_history_mode(int *x, int *y, int *w, int *h)
{
	/* SEを再生する */
	play_se(conf_history_cancel_se);

	/* セーブ画面を終了する */
	is_history_mode_enabled = false;

	/* ヒストリ画面を終了した直後であることを記録する */
	history_flag = true;

	/* ステージを再描画する */
	draw_stage();

	/* ステージ全体をウィンドウに転送する */
	*x = 0;
	*y = 0;
	*w = conf_window_width;
	*h = conf_window_height;
}

/*
 * ヒストリ画面が有効であるかを返す
 */
bool is_history_mode(void)
{
	return is_history_mode_enabled;
}

/*
 * ヒストリ画面を実行する
 */
void run_history_mode(int *x, int *y, int *w, int *h)
{
	int ofs;

	/* 最初のフレームを実行する */
	if (is_first_frame) {
		draw_page(x, y, w, h);
		is_first_frame = false;
		return;
	}

	/* 右クリックされた場合、ヒストリ画面を終了する */
	if (is_right_button_pressed) {
		/* 描画を行う(GPU用) */
		draw_stage_history_keep();

		stop_history_mode(x, y, w, h);
		return;
	}

	/* 上キーが押された場合、描画開始項目のオフセットを1つ上にする */
	if (is_up_pressed) {
		/* オフセットを1つ上にする */
		ofs = start_offset >= (history_count - 1) ?
			(history_count - 1) : start_offset + 1;
		if (ofs != start_offset) {
			start_offset = ofs;
			draw_page(x, y, w, h);
		} else {
			/* 描画を行う(GPU用) */
			draw_stage_history_keep();
		}
		return;
	}

	/* 下キーが押された場合、描画開始項目のオフセットを1つ下にする */
	if (is_down_pressed) {
#if 0
		/* ただしオフセットが0の場合はヒストリ画面を終了する */
		if (start_offset == 0) {
			stop_history_mode(x, y, w, h);
			return;
		}
#endif
		/* オフセットを1つ下にする */
		ofs = start_offset == 0 ? 0 : start_offset - 1;
		if (ofs != start_offset) {
			start_offset = ofs;
			draw_page(x, y, w, h);
		} else {
			/* 描画を行う(GPU用) */
			draw_stage_history_keep();
		}
		return;
	}

	/* ポイントされているテキスト項目を更新する */
	update_pointed_index();

	/* 描画を行う(GPU用) */
	draw_stage_history_keep();

	/* ポイントされているテキスト項目があり、左ボタンが押された場合 */
	if (pointed_index != -1 && is_left_button_pressed) {
		play_voice();
	}
}

/* 描画を行う */
static void draw_page(int *x, int *y, int *w, int *h)
{
	int index, pen_x, pen_y;

	/* FIレイヤをロックする */
	lock_fi_layer_for_history();

	/* FIレイヤを色で塗り潰す */
	draw_history_fi(make_pixel_slow((uint8_t)conf_history_color_a,
					(uint8_t)conf_history_color_r,
					(uint8_t)conf_history_color_g,
					(uint8_t)conf_history_color_b));

	/* テキストを描画する */
	index = (history_index - start_offset + HISTORY_SIZE - 1) %
		HISTORY_SIZE;
	view_start = index;
	pen_x = conf_history_margin_left;
	pen_y = conf_history_margin_top;
	while (true) {
		assert(index >= 0);
		assert(index < history_count);

		/* 描画終了項目を更新する */
		view_end = index;

		/* メッセージを１つ描画する */
		if (!draw_message(&pen_x, &pen_y, index))
			break;	/* 画面の高さを超えた */

		/* 次に描画する項目を求める */
		index = (index + 1) % HISTORY_SIZE;

		/* 最新のメッセージまで書き終わった場合 */
		if (index == history_index)
			break;
	}

	/* FIレイヤをアンロックする */
	unlock_fi_layer_for_history();

	/* ステージを描画する */
	draw_stage_history();
	
	/* ステージ全体をウィンドウに転送する */
	*x = 0;
	*y = 0;
	*w = conf_window_width;
	*h = conf_window_height;
}

/* メッセージを描画する */
static bool draw_message(int *pen_x, int *pen_y, int index)
{
	const char *text;
	uint32_t c;
	int mblen, width, height;
	bool is_after_space = true, escaped = false;

	/* テキストの描画開始Y座標を記録する */
	history[index].y_top = *pen_y;

	/* 1文字ずつ描画する */
	text = history[index].text;
	while (*text != '\0') {
		/* ワードラッピングを処理する */
		if (is_after_space) {
			if (*pen_x + get_en_word_width(text) >=
			    conf_window_width - conf_history_margin_right) {
				*pen_y += conf_history_margin_line;
				*pen_x = conf_history_margin_left;
			}
		}
		is_after_space = *text == ' ';

		/* 描画する文字を取得する */
		mblen = utf8_to_utf32(text, &c);
		if (mblen == -1)
			return false;

		/* エスケープの処理 */
		if (!escaped) {
			/* エスケープ文字であるとき */
			if (c == CHAR_BACKSLASH || c == CHAR_YENSIGN) {
				escaped = true;
				text += mblen;
				continue;
			}
		} else if (escaped) {
			/* エスケープされた文字であるとき */
			if (c == CHAR_SMALLN) {
				*pen_y += conf_history_margin_line;
				*pen_x = conf_history_margin_left;
				escaped = false;
				text += mblen;
				continue;
			}

			/* 不明なエスケープシーケンスの場合 */
			escaped = false;
		}

		/* 描画する文字の幅を取得する */
		width = get_glyph_width(c);

		/* メッセージボックスの幅を超える場合、改行する */
		if ((*pen_x + width + conf_history_margin_right >=
		     conf_window_width) &&
		    (c != CHAR_SPACE && c != CHAR_COMMA && c != CHAR_PERIOD &&
		     c != CHAR_COLON && c != CHAR_SEMICOLON &&
		     c != CHAR_TOUTEN && c != CHAR_KUTEN)) {
			*pen_y += conf_history_margin_line;
			*pen_x = conf_history_margin_left;
		}

		/* 画面の高さを超える場合 */
		if (*pen_y + conf_font_size + conf_history_margin_bottom >
		    conf_window_height)
			return false;

		/* 描画する */
		draw_char_on_fi(*pen_x, *pen_y, c, &width, &height);

		/* 次の文字へ移動する */
		*pen_x += width;
		text += mblen;
	}

	/* 改行する */
	*pen_x = conf_history_margin_left;
	*pen_y += conf_history_margin_line;

	/* テキストの描画終了Y座標を記録する */
	history[index].y_bottom = *pen_y - 1;

	return true;
}

/* textが英単語であればその描画幅、それ以外の場合0を返す */
static int get_en_word_width(const char *text)
{
	const char *m;
	int width;

	m = text;
	width = 0;
	while (isgraph((unsigned char)*m))
		width += get_glyph_width((unsigned char)*m++);

	return width;
}

/* ポイントされている項目を更新する */
static void update_pointed_index(void)
{
	int i;

	if (mouse_pos_x < conf_history_margin_left)
		return;
	if (mouse_pos_x >= conf_window_width - conf_history_margin_right)
		return;

	/* 表示されている最初の項目から順に検索する */
	i = view_start;
	while (true) {
		/* マウスが項目の範囲内にある場合 */
		if (mouse_pos_y >= history[i].y_top &&
		    mouse_pos_y < history[i].y_bottom) {
			/* ボイスがある場合、ポイントされている項目とする */
			if (history[i].voice != NULL) {
				pointed_index = i;
				return;
			}
			break;
		}

		/* 表示されている最後の項目まで検索した場合 */
		if (i == view_end)
			break;

		/* 次の項目へ進む */
		i = (i + 1) % HISTORY_SIZE;
	}

	/* ポイントされている項目をなしとする */
	pointed_index = -1;
}

/* ボイスを再生する */
static void play_voice(void)
{
	struct wave *w;
	const char *voice;

	/* ボイスのファイル名を取得する */
	voice = history[pointed_index].voice;
	if (strcmp(voice, "") == 0)
		return;
	if (voice[0] == '@')
		return;

	/* PCMストリームを開く */
	w = create_wave_from_file(CV_DIR, voice, false);
	if (w == NULL)
		return;

	/* PCMストリームを再生する */
	set_mixer_input(VOICE_STREAM, w);
}

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
