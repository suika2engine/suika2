/* -*- coding: utf-8; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Suika 2
 * Copyright (C) 2001-2023, TABATA Keiichi. All rights reserved.
 */

/*
 * セーブ画面とセーブ・ロード実行
 *
 * [Changes]
 *  - 2016/06/30 作成
 *  - 2017/08/17 グローバル変数に対応
 *  - 2018/07/22 gosubに対応
 *  - 2021/06/05 マスターボリュームに対応
 *  - 2021/07/07 セーブ専用画面に対応
 *  - 2021/07/29 クイックセーブ・ロードに対応
 *  - 2022/06/09 デバッガに対応
 *  - 2022/08/07 GUIに機能を移管
 *  - 2023/06/11 名前変数に対応
 */

#include "suika.h"

#ifdef EM
#include <emscripten/emscripten.h>
#endif

/* False assertion */
#define CONFIG_TYPE_ERROR	(0)

/* クイックセーブのファイル名 */
#define QUICK_SAVE_FILE_NAME	"q000.sav"

/* セーブデータ数 */
#define SAVE_SLOTS		(100)

/* クイックセーブデータのインデックス */
#define QUICK_SAVE_INDEX	(SAVE_SLOTS)

/* コンフィグの終端記号 */
#define END_OF_CONFIG		"eoc"

/* ロードされた直後であるかのフラグ */
static bool load_flag;

/*
 * このモジュールで保持する設定値
 */

/* 章題*/
static char *chapter_name;

/* 最後のメッセージ */
static char *last_message;

/* テキストの表示スピード */
static float msg_text_speed;

/* オートモードの待ち時間の長さ */
static float msg_auto_speed;

/*
 * メモリ上に読み込んだセーブデータ
 */

/* セーブデータの日付 (0のときデータなし) */
static time_t save_time[SAVE_SLOTS];

/* セーブデータの章タイトル */
static char *save_title[SAVE_SLOTS];

/* セーブデータのメッセージ */
static char *save_message[SAVE_SLOTS];

/* セーブデータのサムネイル */
static struct image *save_thumb[SAVE_SLOTS];

/* クイックセーブデータの日付 */
static time_t quick_save_time;

/*
 * 作業用バッファ
 */

/* 文字列読み込み用バッファ */
static char tmp_str[4096];

/* サムネイル読み書き用バッファ */
static unsigned char *tmp_pixels;

/* 前方参照 */
static void load_basic_save_data(void);
static void load_basic_save_data_file(struct rfile *rf, int index);
static bool serialize_all(const char *fname, uint64_t *timestamp, int index);
static bool serialize_title(struct wfile *wf, int index);
static bool serialize_message(struct wfile *wf, int index);
static bool serialize_thumb(struct wfile *wf, int index);
static bool serialize_command(struct wfile *wf);
static bool serialize_stage(struct wfile *wf);
static bool serialize_bgm(struct wfile *wf);
static bool serialize_volumes(struct wfile *wf);
static bool serialize_vars(struct wfile *wf);
static bool serialize_name_vars(struct wfile *wf);
static bool serialize_local_config(struct wfile *wf);
static bool serialize_config_helper(struct wfile *wf, bool is_global);
static bool deserialize_all(const char *fname);
static bool deserialize_command(struct rfile *rf);
static bool deserialize_stage(struct rfile *rf);
static bool deserialize_bgm(struct rfile *rf);
static bool deserialize_volumes(struct rfile *rf);
static bool deserialize_vars(struct rfile *rf);
static bool deserialize_name_vars(struct rfile *rf);
static bool deserialize_config_common(struct rfile *rf);
static void load_global_data(void);

/*
 * 初期化
 */

/*
 * セーブデータに関する初期化処理を行う
 */
bool init_save(void)
{
	int i;

	/* 再利用時のための初期化を行う */
	msg_text_speed = 0.5f;
	msg_auto_speed = 0.5f;

	/* セーブスロットを初期化する */
	for (i = 0; i < SAVE_SLOTS; i++) {
		save_time[i] = 0;
		if (save_title[i] != NULL) {
			free(save_title[i]);
			save_title[i] = NULL;
		}
		if (save_message[i] != NULL) {
			free(save_message[i]);
			save_message[i] = NULL;
		}
		if (save_thumb[i] != NULL) {
			destroy_image(save_thumb[i]);
			save_thumb[i] = NULL;
		}
	}

	/* 文字列を初期化する */
	if (chapter_name != NULL)
		free(chapter_name);
	if (last_message != NULL)
		free(last_message);
	chapter_name = strdup("");
	last_message = strdup("");
	if (chapter_name == NULL || last_message == NULL) {
		log_memory();
		return false;
	}

	/* サムネイルの読み書きのためのヒープを確保する */
	if (tmp_pixels != NULL)
		free(tmp_pixels);
	tmp_pixels = malloc((size_t)(conf_save_data_thumb_width *
				     conf_save_data_thumb_height * 3));
	if (tmp_pixels == NULL) {
		log_memory();
		return false;
	}

	/* セーブデータから基本情報を取得する */
	load_basic_save_data();

	/* グローバルデータのロードを行う */
	load_global_data();

	return true;
}

/*
 * セーブデータに関する終了処理を行う
 */
void cleanup_save(void)
{
	int i;

	for (i = 0; i < SAVE_SLOTS; i++) {
		if (save_title[i] != NULL) {
			free(save_title[i]);
			save_title[i] = NULL;
		}
		if (save_message[i] != NULL) {
			free(save_message[i]);
			save_message[i] = NULL;
		}
		if (save_thumb[i] != NULL) {
			destroy_image(save_thumb[i]);
			save_thumb[i] = NULL;
		}
	}

	free(chapter_name);
	chapter_name = NULL;

	free(last_message);
	last_message = NULL;

	free(tmp_pixels);
	tmp_pixels = NULL;

	/* グローバル変数のセーブを行う */
	save_global_data();
}

/*
 * ロードが終了した直後であるかを調べる
 */
bool check_load_flag(void)
{
	if (load_flag) {
		load_flag = false;
		return true;
	}
	return false;
}

/*
 * GUIからの問い合わせ
 */

/*
 * セーブデータの日付を取得する
 */
time_t get_save_date(int index)
{
	assert(index >= 0);
	if (index >= SAVE_SLOTS)
		return 0;

	return save_time[index];
}

/*
 * セーブデータの章タイトルを取得する
 */
const char *get_save_chapter_name(int index)
{
	assert(index >= 0);
	if (index >= SAVE_SLOTS)
		return 0;

	return save_title[index];
}

/*
 * セーブデータの最後のメッセージを取得する
 */
const char *get_save_last_message(int index)
{
	assert(index >= 0);
	if (index >= SAVE_SLOTS)
		return 0;

	return save_message[index];
}

/*
 * セーブデータのサムネイルを取得する
 */
struct image *get_save_thumbnail(int index)
{
	assert(index >= 0);
	if (index >= SAVE_SLOTS)
		return NULL;

	return save_thumb[index];
}

/*
 * セーブの実際の処理
 */

/*
 * クイックセーブを行う
 */
bool quick_save(void)
{
	uint64_t timestamp;

	/*
	 * サムネイルを作成する
	 *  - GUIを経由しないのでここで作成する
	 *  - ただし、現状ではクイックセーブデータのサムネイルは未使用
	 */
	/*
	  In message command, use draw_stage_to_thumb().
	  In switch command, use draw_stage_fo_thumb().
	*/

	/* ローカルデータのシリアライズを行う */
	if (!serialize_all(QUICK_SAVE_FILE_NAME, &timestamp, -1))
		return false;

	/* 既読フラグのセーブを行う */
	save_seen();

	/* グローバル変数のセーブを行う */
	save_global_data();

	/* クイックセーブの時刻を更新する */
	quick_save_time = (time_t)timestamp;

#ifdef EM
	EM_ASM_(
		FS.syncfs(function (err) { alert('Saved to the browser!'); });
	);
#endif
	return true;
}

/*
 * セーブを実行する
 */
bool execute_save(int index)
{
	char s[128];
	uint64_t timestamp;

	/* ファイル名を求める */
	snprintf(s, sizeof(s), "%03d.sav", index);

	/* ローカルデータのシリアライズを行う */
	if (!serialize_all(s, &timestamp, index))
		return false;

	/* 既読フラグのセーブを行う */
	save_seen();

	/* グローバル変数のセーブを行う */
	save_global_data();

	/* 時刻を保存する */
	save_time[index] = (time_t)timestamp;

#ifdef EM
	EM_ASM_(
		FS.syncfs(function (err) { alert('Saved to the browser!'); });
	);
#endif
	return true;
}

/* すべてのシリアライズを行う */
static bool serialize_all(const char *fname, uint64_t *timestamp, int index)
{
	struct wfile *wf;
	uint64_t t;
	bool success;

	/* セーブディレクトリを作成する */
	make_sav_dir();

	/* ファイルを開く */
	wf = open_wfile(SAVE_DIR, fname);
	if (wf == NULL)
		return false;

	success = false;
	do {
		/* 日付を書き込む */
		t = (uint64_t)time(NULL);
		if (write_wfile(wf, &t, sizeof(t)) < sizeof(t))
			break;

		/* 章題のシリアライズを行う */
		if (!serialize_title(wf, index))
			break;

		/* メッセージのシリアライズを行う */
		if (!serialize_message(wf, index))
			break;

		/* サムネイルのシリアライズを行う */
		if (!serialize_thumb(wf, index))
			break;

		/* コマンド位置のシリアライズを行う */
		if (!serialize_command(wf))
			break;

		/* ステージのシリアライズを行う */
		if (!serialize_stage(wf))
			break;

		/* BGMのシリアライズを行う */
		if (!serialize_bgm(wf))
			break;

		/* ボリュームのシリアライズを行う */
		if (!serialize_volumes(wf))
			break;

		/* 変数のシリアライズを行う */
		if (!serialize_vars(wf))
			break;

		/* 名前変数のシリアライズを行う */
		if (!serialize_name_vars(wf))
			break;

		/* コンフィグのシリアライズを行う */
		if (!serialize_local_config(wf))
			break;

		/* 成功 */
		success = true;
	} while (0);

	/* ファイルをクローズする */
	close_wfile(wf);

	/* 時刻を保存する */
	*timestamp = t;

	if (!success)
		log_file_write(fname);

	return success;
}

/* 章題のシリアライズを行う */
static bool serialize_title(struct wfile *wf, int index)
{
	size_t len;

	/* 文字列を準備する */
	strncpy(tmp_str, chapter_name, sizeof(tmp_str));
	tmp_str[sizeof(tmp_str) - 1] = '\0';

	/* 書き出す */
	len = strlen(tmp_str) + 1;
	if (write_wfile(wf, tmp_str, len) < len)
		return false;

	/* 章題を保存する */
	if (index != -1) {
		if (save_title[index] != NULL)
			free(save_title[index]);
		save_title[index] = strdup(tmp_str);
		if (save_title[index] == NULL) {
			log_memory();
			return false;
		}
	}

	return true;
}

/* メッセージのシリアライズを行う */
static bool serialize_message(struct wfile *wf, int index)
{
	size_t len;

	/* 文字列を準備する */
	strncpy(tmp_str, last_message, sizeof(tmp_str));
	tmp_str[sizeof(tmp_str) - 1] = '\0';

	/* 書き出す */
	len = strlen(tmp_str) + 1;
	if (write_wfile(wf, tmp_str, len) < len)
		return false;

	/* メッセージを保存する */
	if (index != -1) {
		if (save_message[index] != NULL)
			free(save_message[index]);
		save_message[index] = strdup(tmp_str);
		if (save_message[index] == NULL)
			return false;
	}

	return true;
}

/* サムネイルのシリアライズを行う */
static bool serialize_thumb(struct wfile *wf, int index)
{
	pixel_t *src, pix;
	unsigned char *dst;
	size_t len;
	int x, y;

	/* クイックセーブの場合 */
	if (index == -1) {
		/* 内容は気にせず書き出す */
		len = (size_t)(conf_save_data_thumb_width *
			       conf_save_data_thumb_height * 3);
		if (write_wfile(wf, tmp_pixels, len) < len)
			return false;
		return true;
	}

	/* stage.cのサムネイルをsave.cのイメージにコピーする */
	if (save_thumb[index] == NULL) {
		save_thumb[index] = create_image(conf_save_data_thumb_width,
						 conf_save_data_thumb_height);
		if (save_thumb[index] == NULL)
			return false;
	}
	lock_image(save_thumb[index]);
	draw_image(save_thumb[index], 0, 0, get_thumb_image(),
		   conf_save_data_thumb_width, conf_save_data_thumb_height, 0, 0,
		   255, BLEND_NONE);
	unlock_image(save_thumb[index]);

	/* ピクセル列を準備する */
	src = get_image_pixels(get_thumb_image());
	dst = tmp_pixels;
	for (y = 0; y < conf_save_data_thumb_height; y++) {
		for (x = 0; x < conf_save_data_thumb_width; x++) {
			pix = *src++;
			*dst++ = (unsigned char)get_pixel_r_slow(pix);
			*dst++ = (unsigned char)get_pixel_g_slow(pix);
			*dst++ = (unsigned char)get_pixel_b_slow(pix);
		}
	}

	/* 書き出す */
	len = (size_t)(conf_save_data_thumb_width *
		       conf_save_data_thumb_height * 3);
	if (write_wfile(wf, tmp_pixels, len) < len)
		return false;

	return true;
}

/* コマンド位置をシリアライズする */
static bool serialize_command(struct wfile *wf)
{
	const char *s;
	int n, m;

	/* スクリプトファイル名を取得してシリアライズする */
	s = get_script_file_name();
	if (write_wfile(wf, s, strlen(s) + 1) < strlen(s) + 1)
		return false;

	/* コマンドインデックスを取得してシリアライズする */
	n = get_command_index();
	if (write_wfile(wf, &n, sizeof(n)) < sizeof(n))
		return false;

	/* '@gosub'のリターンポイントを取得してシリアライズする */
	m = get_return_point();
	if (write_wfile(wf, &m, sizeof(m)) < sizeof(m))
		return false;

	return true;
}

/* ステージをシリアライズする */
static bool serialize_stage(struct wfile *wf)
{
	const char *s;
	int i, m, n, o;

	s = get_bg_file_name();
	if (s == NULL)
		s = "none";
	if (write_wfile(wf, s, strlen(s) + 1) < strlen(s) + 1)
		return false;

	for (i = 0; i < CH_ALL_LAYERS; i++) {
		get_ch_position(i, &m, &n);
		o = get_ch_alpha(i);
		if (write_wfile(wf, &m, sizeof(m)) < sizeof(m))
			return false;
		if (write_wfile(wf, &n, sizeof(n)) < sizeof(n))
			return false;
		if (write_wfile(wf, &o, sizeof(o)) < sizeof(o))
			return false;

		s = get_ch_file_name(i);
		if (s == NULL)
			s = "none";
		if (write_wfile(wf, s, strlen(s) + 1) < strlen(s) + 1)
			return false;
	}

	return true;
}

/* BGMをシリアライズする */
static bool serialize_bgm(struct wfile *wf)
{
	const char *s;

	s = get_bgm_file_name();
	if (s == NULL)
		s = "none";
	if (write_wfile(wf, s, strlen(s) + 1) < strlen(s) + 1)
		return false;

	return true;
}

/* ボリュームをシリアライズする */
static bool serialize_volumes(struct wfile *wf)
{
	float vol;
	int n;

	for (n = 0; n < MIXER_STREAMS; n++) {
		vol = get_mixer_volume(n);
		if (write_wfile(wf, &vol, sizeof(vol)) < sizeof(vol))
			return false;
	}

	return true;
}

/* ローカル変数をシリアライズする */
static bool serialize_vars(struct wfile *wf)
{
	size_t len;

	len = LOCAL_VAR_SIZE * sizeof(int32_t);
	if (write_wfile(wf, get_local_variables_pointer(), len) < len)
		return false;

	return true;
}

/* 名前変数をシリアライズする */
static bool serialize_name_vars(struct wfile *wf)
{
	size_t len;
	const char *name;
	int i;

	for (i = 0; i < NAME_VAR_SIZE; i++) {
		name = get_name_variable(i);
		assert(name != NULL);

		len = strlen(name) + 1;
		if (write_wfile(wf, name, len) < len)
			return false;
	}

	return true;
}

/* ローカルコンフィグをシリアライズする */
static bool serialize_local_config(struct wfile *wf)
{
	if (!serialize_config_helper(wf, false))
		return false;

	return true;
}

/* コンフィグをシリアライズする */
static bool serialize_config_helper(struct wfile *wf, bool is_global)
{
	char val[1024];
	const char *key, *val_s;
	size_t len;
	int key_index;

	/* 保存可能なキーを列挙してループする */
	key_index = 0;
	while (1) {
		/* セーブするキーを取得する */
		key = get_config_key_for_save_data(key_index++);
		if (key == NULL) {
			/* キー列挙が終了した */
			break;
		}

		/* グローバル/ローカルの対象をチェックする */
		if (!is_global) {
			if (is_config_key_global(key))
				continue;
		} else {
			if (!is_config_key_global(key))
				continue;
		}

		/* キーを出力する */
		len = strlen(key) + 1;
		if (write_wfile(wf, key, len) < len)
			return false;

		/* 型ごとに値を出力する */
		switch (get_config_type_for_key(key)) {
		case 's':
			val_s = get_string_config_value_for_key(key);
			if (val_s == NULL)
				val_s = "";
			len = strlen(val_s) + 1;
			if (write_wfile(wf, val_s, len) < len)
				return false;
			break;
		case 'i':
			snprintf(val, sizeof(val), "%d",
				 get_int_config_value_for_key(key));
			len = strlen(val) + 1;
			if (write_wfile(wf, &val, len) < len)
				return false;
			break;
		case 'f':
			snprintf(val, sizeof(val), "%f",
				 get_float_config_value_for_key(key));
			len = strlen(val) + 1;
			if (write_wfile(wf, &val, len) < len)
				return false;
			break;
		default:
			assert(CONFIG_TYPE_ERROR);
			break;
		}
	}

	/* 終端記号を出力する */
	len = strlen(END_OF_CONFIG);
	if (write_wfile(wf, END_OF_CONFIG, len) < len)
		return false;

	return true;
}

/*
 * ロードの実際の処理
 */

/*
 * クイックセーブデータがあるか
 */
bool have_quick_save_data(void)
{
	if (quick_save_time == 0)
		return false;

	return true;
}

/*
 * クイックロードを行う Do quick load
 */
bool quick_load(void)
{
	/* 既読フラグのセーブを行う */
	save_seen();

	/* グローバル変数のセーブを行う */
	save_global_data();

	/* ローカルデータのデシリアライズを行う */
	if (!deserialize_all(QUICK_SAVE_FILE_NAME))
		return false;

	/* ステージを初期化する */
	if (!reload_stage())
		abort();

	/* SEを停止する */
	set_mixer_input(SE_STREAM, NULL);

	/* 名前ボックス、メッセージボックス、選択ボックスを非表示とする */
	show_namebox(false);
	show_msgbox(false);

	/* ウィンドウタイトルをアップデートする */
	if (!conf_window_title_chapter_disable)
		update_window_title();

	/* オートモードを解除する */
	if (is_auto_mode())
		stop_auto_mode();

	/* スキップモードを解除する */
	if (is_skip_mode())
		stop_skip_mode();

#ifdef USE_DEBUGGER
	clear_variable_changed();
	update_debug_info(true);
#endif

	load_flag = true;

	if (is_message_active())
		clear_message_active();

	return true;
}

/*
 * ロードを処理する
 */
bool execute_load(int index)
{
	char s[128];

	/* ファイル名を求める */
	snprintf(s, sizeof(s), "%03d.sav", index);

	/* 既読フラグのセーブを行う */
	save_seen();

	/* グローバル変数のセーブを行う */
	save_global_data();

	/* ローカルデータのデシリアライズを行う */
	if (!deserialize_all(s))
		return false;

	/* ステージを初期化する */
	if (!reload_stage())
		abort();

	/* SEを停止する */
	set_mixer_input(SE_STREAM, NULL);

	/* 名前ボックス、メッセージボックス、選択ボックスを非表示とする */
	show_namebox(false);
	show_msgbox(false);

	/* ウィンドウタイトルをアップデートする */
	if (!conf_window_title_chapter_disable)
		update_window_title();
	
#ifdef USE_DEBUGGER
	clear_variable_changed();
	update_debug_info(true);
#endif

	load_flag = true;

	if (is_message_active())
		clear_message_active();

	return true;
}

/* すべてをデシリアライズする */
static bool deserialize_all(const char *fname)
{
	struct rfile *rf;
	uint64_t t;
	size_t img_size;
	bool success;

	/* ファイルを開く */
	rf = open_rfile(SAVE_DIR, fname, true);
	if (rf == NULL)
		return false;

	success = false;
	do {
		/* 日付を読み込む (読み飛ばす) */
		if (read_rfile(rf, &t, sizeof(t)) < sizeof(t))
			break;

		/* 章題を読み込む */
		if (gets_rfile(rf, tmp_str, sizeof(tmp_str)) == NULL)
			break;
		if (!set_chapter_name(tmp_str))
			break;

		/* メッセージを読み込む (読み飛ばす) */
		if (gets_rfile(rf, tmp_str, sizeof(tmp_str)) == NULL)
			break;

		/* サムネイルを読み込む (読み飛ばす) */
		img_size = (size_t)(conf_save_data_thumb_width *
				    conf_save_data_thumb_height * 3);
		if (read_rfile(rf, tmp_pixels, img_size) < img_size)
			break;

		/* コマンド位置のデシリアライズを行う */
		if (!deserialize_command(rf))
			break;

		/* ステージのデシリアライズを行う */
		if (!deserialize_stage(rf))
			break;

		/* BGMのデシリアライズを行う */
		if (!deserialize_bgm(rf))
			break;

		/* ボリュームのデシリアライズを行う */
		if (!deserialize_volumes(rf))
			break;

		/* 変数のデシリアライズを行う */
		if (!deserialize_vars(rf))
			break;
		
		/* 名前変数のデシリアライズを行う */
		if (!deserialize_name_vars(rf))
			break;

		/* コンフィグのデシリアライズを行う */
		if (!deserialize_config_common(rf))
			break;

		/* ヒストリをクリアする */
		clear_history();

		/* 成功 */
		success = true;
	} while (0);

	/* ファイルをクローズする */
	close_rfile(rf);

	return success;
}

/* コマンド位置のデシリアライズを行う */
static bool deserialize_command(struct rfile *rf)
{
	char s[1024];
	int n, m;

	if (gets_rfile(rf, s, sizeof(s)) == NULL)
		return false;

	if (!load_script(s))
		return false;

	if (read_rfile(rf, &n, sizeof(n)) < sizeof(n))
		return false;

	if (read_rfile(rf, &m, sizeof(m)) < sizeof(m))
		return false;

	if (!move_to_command_index(n))
		return false;

	if (!set_return_point(m))
		return false;

	return true;
}

/* ステージのデシリアライズを行う */
static bool deserialize_stage(struct rfile *rf)
{
	char s[1024];
	struct image *img;
	int m, n, o, i;

	if (gets_rfile(rf, s, sizeof(s)) == NULL)
		return false;

	if (strcmp(s, "none") == 0) {
		set_bg_file_name(NULL);
		img = create_initial_bg();
		if (img == NULL)
			return false;;
	} else if (s[0] == '#') {
		set_bg_file_name(s);
		img = create_image_from_color_string(conf_window_width,
						     conf_window_height,
						     &s[1]);
		if (img == NULL)
			return false;
	} else {
		set_bg_file_name(s);
		if (strncmp(s, "cg/", 3) == 0)
			img = create_image_from_file(CG_DIR, &s[3]);
		else
			img = create_image_from_file(BG_DIR, s);
		if (img == NULL)
			return false;
	}

	change_bg_immediately(img);

	for (i = 0; i < CH_ALL_LAYERS; i++) {
		if (read_rfile(rf, &m, sizeof(m)) < sizeof(n))
			return false;
		if (read_rfile(rf, &n, sizeof(n)) < sizeof(m))
			return false;
		if (read_rfile(rf, &o, sizeof(o)) < sizeof(o))
			return false;
		if (gets_rfile(rf, s, sizeof(s)) == NULL)
			return false;

		assert(strcmp(s, "") != 0);
		if (strcmp(s, "none") == 0) {
			set_ch_file_name(i, NULL);
			img = NULL;
		} else {
			set_ch_file_name(i, s);
			img = create_image_from_file(CH_DIR, s);
			if (img == NULL)
				return false;
		}

		change_ch_immediately(i, img, m, n, o);
	}

	return true;
}

/* BGMをデシリアライズする */
static bool deserialize_bgm(struct rfile *rf)
{
	char s[1024];
	struct wave *w;

	if (gets_rfile(rf, s, sizeof(s)) == NULL)
		return false;

	if (strcmp(s, "none") == 0) {
		set_bgm_file_name(NULL);
		w = NULL;
	} else {
		set_bgm_file_name(s);
		w = create_wave_from_file(BGM_DIR, s, true);
		if (w == NULL)
			return false;
	}

	set_mixer_input(BGM_STREAM, w);

	return true;
}

/* ボリュームをデシリアライズする */
static bool deserialize_volumes(struct rfile *rf)
{
	float vol;
	int n;

	for (n = 0; n < MIXER_STREAMS; n++) {
		if (read_rfile(rf, &vol, sizeof(vol)) < sizeof(vol))
			return false;
		set_mixer_volume(n, vol, 0);
	}

	return true;
}

/* ローカル変数をデシリアライズする */
static bool deserialize_vars(struct rfile *rf)
{
	size_t len;

	len = LOCAL_VAR_SIZE * sizeof(int32_t);
	if (read_rfile(rf, get_local_variables_pointer(), len) < len)
		return false;

	return true;
}

/* 名前変数をデシリアライズする */
static bool deserialize_name_vars(struct rfile *rf)
{
	char name[1024];
	int i;

	for (i = 0; i < NAME_VAR_SIZE; i++) {
		if (gets_rfile(rf, name, sizeof(name)) == NULL)
			return false;
		set_name_variable(i, name);
	}

	return true;
}

/* コンフィグをデシリアライズする */
static bool deserialize_config_common(struct rfile *rf)
{
	char key[1024];
	char val[1024];

	/* 終端記号が現れるまでループする */
	while (1) {
		/* ロードするキーを取得する */
		if (gets_rfile(rf, key, sizeof(key)) == NULL)
			return false;

		/* 終端記号の場合はループを終了する */
		if (strcmp(key, END_OF_CONFIG) == 0)
			break;

		/* 値を取得する(文字列として保存されている) */
		if (gets_rfile(rf, val, sizeof(val)) == NULL)
			return false;

		/* コンフィグを上書きする */
		if (!overwrite_config(key, val))
			return false;
	}

	return true;
}

/* セーブデータから基本情報を読み込む */
static void load_basic_save_data(void)
{
	struct rfile *rf;
	char buf[128];
	uint64_t t;
	int i;

	/* セーブスロットごとに読み込む */
	for (i = 0; i < SAVE_SLOTS; i++) {
		/* セーブデータファイルを開く */
		snprintf(buf, sizeof(buf), "%03d.sav", i);
		rf = open_rfile(SAVE_DIR, buf, true);
		if (rf != NULL) {
			/* 読み込む */
			load_basic_save_data_file(rf, i);
			close_rfile(rf);
		}
	}

	/* セーブデータファイルを開く */
	rf = open_rfile(SAVE_DIR, QUICK_SAVE_FILE_NAME, true);
	if (rf != NULL) {
		/* セーブ時刻を取得する */
		if (read_rfile(rf, &t, sizeof(t)) == sizeof(t))
			quick_save_time = (time_t)t;
		close_rfile(rf);
	}
}

/* セーブデータファイルから基本情報を読み込む */
static void load_basic_save_data_file(struct rfile *rf, int index)
{
	uint64_t t;
	size_t img_size;
	pixel_t *dst;
	const unsigned char *src;
	uint32_t r, g, b;
	int x, y;

	/* セーブ時刻を取得する */
	if (read_rfile(rf, &t, sizeof(t)) < sizeof(t))
		return;
	save_time[index] = (time_t)t;

	/* 章題を取得する */
	if (gets_rfile(rf, tmp_str, sizeof(tmp_str)) == NULL)
		return;
	save_title[index] = strdup(tmp_str);
	if (save_title[index] == NULL) {
		log_memory();
		return;
	}

	/* メッセージを取得する */
	if (gets_rfile(rf, tmp_str, sizeof(tmp_str)) == NULL)
		return;
	save_message[index] = strdup(tmp_str);
	if (save_message[index] == NULL) {
		log_memory();
		return;
	}

	/* サムネイルを取得する */
	img_size = (size_t)(conf_save_data_thumb_width *
			    conf_save_data_thumb_height * 3);
	if (read_rfile(rf, tmp_pixels, img_size) < img_size)
		return;

	/* サムネイルの画像を生成する */
	save_thumb[index] = create_image(conf_save_data_thumb_width,
					 conf_save_data_thumb_height);
	if (save_thumb[index] == NULL)
		return;
	lock_image(save_thumb[index]);
	dst = get_image_pixels(save_thumb[index]);
	src = tmp_pixels;
	for (y = 0; y < conf_save_data_thumb_height; y++) {
		for (x = 0; x < conf_save_data_thumb_width; x++) {
			r = *src++;
			g = *src++;
			b = *src++;
			*dst++ = make_pixel_slow(0xff, r, g, b);
		}
	}
	unlock_image(save_thumb[index]);
}

/*
 * グローバル変数
 */

/* グローバルデータのロードを行う */
static void load_global_data(void)
{
	char fname[128];
	struct rfile *rf;
	float f;
	int i;

	/* ファイルを開く */
	rf = open_rfile(SAVE_DIR, GLOBAL_VARS_FILE, true);
	if (rf == NULL)
		return;

	/* グローバル変数をデシリアライズする */
	read_rfile(rf, get_global_variables_pointer(),
		   GLOBAL_VAR_SIZE * sizeof(int32_t));

	/*
	 * load_global_data()はinit_mixer()より後に呼ばれる
	 */

	/* グローバルボリュームをデシリアライズする */
	for (i = 0; i < MIXER_STREAMS; i++) {
		if (read_rfile(rf, &f, sizeof(f)) < sizeof(f))
			break;
		f = (f < 0 || f > 1.0f) ? 1.0f : f;
		set_mixer_global_volume(i, f);
	}

	/* キャラクタボリュームをデシリアライズする */
	for (i = 0; i < CH_VOL_SLOTS; i++) {
		if (read_rfile(rf, &f, sizeof(f)) < sizeof(f))
			break;
		f = (f < 0 || f > 1.0f) ? 1.0f : f;
		set_character_volume(i, f);
	}

	/* テキストスピードをデシリアライズする */
	read_rfile(rf, &msg_text_speed, sizeof(f));
	msg_text_speed =
		(msg_text_speed < 0 || msg_text_speed > 10000.0f) ?
		1.0f : msg_text_speed;

	/* オートモードスピードをデシリアライズする */
	read_rfile(rf, &msg_auto_speed, sizeof(f));
	msg_auto_speed =
		(msg_auto_speed < 0 || msg_auto_speed > 10000.0f) ?
		1.0f : msg_auto_speed;

	/* フォントファイル名をデシリアライズする */
	if (gets_rfile(rf, fname, sizeof(fname)) != NULL)
		preinit_set_global_font_file_name(fname);

	/* コンフィグをデシリアライズする */
	deserialize_config_common(rf);

	/* ファイルを閉じる */
	close_rfile(rf);
}

/*
 * グローバルデータのセーブを行う
 */
void save_global_data(void)
{
	struct wfile *wf;
	const char *fname;
	float f;
	int i;

	/* セーブディレクトリを作成する */
	make_sav_dir();

	/* ファイルを開く */
	wf = open_wfile(SAVE_DIR, GLOBAL_VARS_FILE);
	if (wf == NULL)
		return;

	/* グローバル変数をシリアライズする */
	write_wfile(wf, get_global_variables_pointer(),
		    GLOBAL_VAR_SIZE * sizeof(int32_t));

	/* グローバルボリュームをシリアライズする */
	for (i = 0; i < MIXER_STREAMS; i++) {
		f = get_mixer_global_volume(i);
		if (write_wfile(wf, &f, sizeof(f)) < sizeof(f))
			break;
	}

	/* キャラクタボリュームをシリアライズする */
	for (i = 0; i < CH_VOL_SLOTS; i++) {
		f = get_character_volume(i);
		if (write_wfile(wf, &f, sizeof(f)) < sizeof(f))
			break;
	}

	/* テキストスピードをシリアライズする */
	write_wfile(wf, &msg_text_speed, sizeof(f));
	
	/* オートモードスピードをシリアライズする */
	write_wfile(wf, &msg_auto_speed, sizeof(f));

	/* フォントファイル名をシリアライズする */
	fname = get_global_font_file_name();
	write_wfile(wf, fname, strlen(fname) + 1);

	/* コンフィグをデシリアライズする */
	serialize_config_helper(wf, true);

	/* ファイルを閉じる */
	close_wfile(wf);
}

/*
 * セーブデータの削除を処理する
 */
void delete_save_data(int index)
{
	char s[128];

	/* セーブデータがない場合、何もしない */
	if (save_time[index] == 0)
		return;

	/* プロンプトを表示する */
	if (!delete_dialog())
		return;

	/* ファイル名を求める */
	snprintf(s, sizeof(s), "%03d.sav", index);

	/* セーブファイルを削除する */
	remove_file(SAVE_DIR, s);

	/* セーブデータを消去する */
	save_time[index] = 0;
	if (save_title[index] != NULL) {
		free(save_title[index]);
		save_title[index] = NULL;
	}
	if (save_message[index] != NULL) {
		free(save_message[index]);
		save_message[index] = NULL;
	}
	if (save_thumb[index] != NULL) {
		destroy_image(save_thumb[index]);
		save_thumb[index] = NULL;
	}
}

/*
 * 章題と最後のメッセージ
 */

/*
 * 章題を設定する
 */
bool set_chapter_name(const char *name)
{
	free(chapter_name);

	chapter_name = strdup(name);
	if (chapter_name == NULL) {
		log_memory();
		return false;
	}

	return true;
}

/*
 * 章題を取得する
 */
const char *get_chapter_name(void)
{
	if (chapter_name == NULL)
		return "";

	return chapter_name;
}

/*
 * 最後のメッセージを設定する
 */
bool set_last_message(const char *msg)
{
	free(last_message);

	last_message = strdup(msg);
	if (last_message == NULL) {
		log_memory();
		return false;
	}

	return true;
}

/*
 * テキストスピードを設定する
 */
void set_text_speed(float val)
{
	assert(val >= 0 && val <= 1.0f);

	msg_text_speed = val;
}

/*
 * テキストスピードを取得する
 */
float get_text_speed(void)
{
	return msg_text_speed;
}

/*
 * オートスピードを設定する
 */
void set_auto_speed(float val)
{
	assert(val >= 0 && val <= 1.0f);

	msg_auto_speed = val;
}

/*
 * オートスピードを取得する
 */
float get_auto_speed(void)
{
	return msg_auto_speed;
}
