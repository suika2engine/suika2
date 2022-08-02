/* -*- coding: utf-8-with-signature; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Suika 2
 * Copyright (C) 2001-2021, TABATA Keiichi. All rights reserved.
 */

/*
 * [Changes]
 *  - 2021/07/31 作成
 */

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "suika.h"

/* 既読フラグ */
#ifndef USE_DEBUGGER
static bool seen_flag[SCRIPT_CMD_SIZE];
#endif

/* 初期化済みか */
static bool is_initialized;

/* 前方参照 */
#ifndef USE_DEBUGGER
static const char *hash(const char *file);
static char hex(int c);
#endif

/*
 * 既読フラグ管理を初期化する
 */
bool init_seen(void)
{
	/* 既読フラグをロードする */
	load_seen();

	is_initialized = true;

	return true;
}

/*
 * 既読フラグ管理の終了処理を行う
 */
void cleanup_seen(void)
{
	if (is_initialized) {
		/* 既読フラグをセーブする */
		save_seen();

		is_initialized = false;
	}
}

/*
 * 現在のスクリプトに対応する既読フラグをロードする
 */
bool load_seen(void)
{
#ifdef USE_DEBUGGER
	return true;
#else
	struct rfile *rf;
	const char *fname;
	bool success;

	/* 読み込む前に全部未読にする */
	memset(seen_flag, 0, sizeof(seen_flag));

	/* ファイル名を求める */
	fname = hash(get_script_file_name());

	/* ファイルを開く */
	rf = open_rfile(SAVE_DIR, fname, true);
	if (rf == NULL)
		return false;

	success = false;
	do {
		/* 既読フラグを読み込む */
		if (read_rfile(rf, &seen_flag, sizeof(seen_flag)) <
		    sizeof(seen_flag))
			break;

		/* 成功 */
		success = true;
	} while (0);

	/* ファイルをクローズする */
	close_rfile(rf);

	/* 読み込みに失敗した場合全部未読にする */
	if (!success)
		memset(seen_flag, 0, sizeof(seen_flag));

	return success;
#endif
}

/*
 * 現在のスクリプトに対応する既読フラグをセーブする
 */
bool save_seen(void)
{
#ifdef USE_DEBUGGER
	return true;
#else
	struct wfile *wf;
	const char *fname;
	bool success;

	/* セーブディレクトリを作成する */
	make_sav_dir();

	/* ファイル名を求める */
	fname = hash(get_script_file_name());

	/* ファイルを開く */
	wf = open_wfile(SAVE_DIR, fname);
	if (wf == NULL)
		return false;

	success = false;
	do {
		/* 既読フラグを書き込む */
		if (write_wfile(wf, &seen_flag, sizeof(seen_flag)) <
		    sizeof(seen_flag))
			break;

		/* 成功 */
		success = true;
	} while (0);

	/* ファイルをクローズする */
	close_wfile(wf);

	return success;
#endif
}

/*
 * 現在のコマンドが既読かを返す
 */
bool get_seen(void)
{
#ifdef USE_DEBUGGER
	return true;
#else
	int index;

	index = get_command_index();
	assert(index >= 0 && index < SCRIPT_CMD_SIZE);

	return seen_flag[index];
#endif
}

/*
 * 現在のコマンドを既読にする
 */
void set_seen(void)
{
#ifndef USE_DEBUGGER
	int index;

	index = get_command_index();
	assert(index >= 0 && index < SCRIPT_CMD_SIZE);

	seen_flag[index] = true;
#endif
}

#ifndef USE_DEBUGGER
/* スクリプトファイル名からハッシュを求める */
static const char *hash(const char *file)
{
	static char h[129];
	int len, i;

	len = (int)strlen(file);
	if (len > 128)
		len = 128;

	memset(h, 0, sizeof(h));

	for (i = 0; i < len; i++) {
		h[i * 2] = hex(file[i] >> 4);
		h[i * 2 + 1] = hex(file[i] & 0x0f);
	}

	return h;
}

/* 十六進文字を取得する */
static char hex(int c)
{
	assert(c >= 0 && c <= 15);

	if (c >= 0 && c <= 9)
		return (char)('0' + (char)c);
	else
		return (char)('a' + (char)(c - 10));
}
#endif
