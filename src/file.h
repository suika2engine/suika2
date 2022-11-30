/* -*- coding: utf-8; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Suika 2
 * Copyright (C) 2001-2016, TABATA Keiichi. All rights reserved.
 */

/*
 * [Changes]
 *  - 2016/06/28 作成
 */

#ifndef SUIKA_FILE_H
#define SUIKA_FILE_H

#include "types.h"

/*
 * [Archive File Design]
 * 
 * struct header {
 *     u64 file_count;
 *     struct file_entry {
 *         u8  file_name[256]; // Encrypted
 *         u64 file_size;
 *         u64 file_offset;
 *     } [file_count];
 * };
 * u8 file_body[file_count][file_length]; // Encrypted
 */

/*
 * パッケージファイル名
 */
#define PACKAGE_FILE		"data01.arc"

/*
 * パッケージ内のファイルエントリの最大数
 */
#define FILE_ENTRY_SIZE		(65536)

/*
 * パッケージ内のファイル名のサイズ
 */
#define FILE_NAME_SIZE		(256)

/*
 * パッケージ内のファイルエントリ
 */
struct file_entry {
	/* ファイル名 */
	char name[FILE_NAME_SIZE];

	/* ファイルサイズ */
	uint64_t size;

	/* パッケージ内のファイルオフセット */
	uint64_t offset;
};

/*
 * ファイル読み込みストリーム
 */
struct rfile;

/*
 * ファイル書き込みストリーム
 */
struct wfile;

/*
 * ファイル読み書きの初期化処理を行う
 */
bool init_file(void);

/*
 * ファイル読み書きの初期化処理を行う
 */
void cleanup_file(void);

/*
 * ファイル読み込みストリームを開く
 */
struct rfile *open_rfile(const char *dir, const char *file, bool save_data);

/*
 * ファイルのサイズを取得する
 */
size_t get_rfile_size(struct rfile *rf);

/*
 * ファイル読み込みストリームから読み込む
 */
size_t read_rfile(struct rfile *rf, void *buf, size_t size);

/*
 * ファイル読み込みストリームから1行読み込む
 */
const char *gets_rfile(struct rfile *rf, char *buf, size_t size);

/*
 * ファイル読み込みストリームを閉じる
 */
void close_rfile(struct rfile *rf);

/*
 * ファイル書き込みストリームを開く
 */
struct wfile *open_wfile(const char *dir, const char *file);

/*
 * ファイル書き込みストリームに書き込む
 */
size_t write_wfile(struct wfile *wf, const void *buf, size_t size);

/*
 * ファイル読み込みストリームを閉じる
 */
void close_wfile(struct wfile *wf);

/*
 * ファイルを削除する
 */
void remove_file(const char *dir, const char *file);

#endif
