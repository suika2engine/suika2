/* -*- coding: utf-8; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Suika2
 * Copyright (C) 2001-2023, Keiichi Tabata. All rights reserved.
 */

/*
 * File I/O implementation for Browser's File System API
 *
 * [Changes]
 *  - 2023/11/12 作成
 */

#include "../suika.h"

#include <emscripten.h>

/* パスの長さ */
#define PATH_SIZE	(1024)

/* ファイル読み込みストリーム */
struct rfile {
	char *data;
	uint64_t size;
	uint64_t pos;
};

/*
 * ファイル書き込みストリーム
 */
struct wfile {
	char *buf;
	uint64_t buf_size;
	uint64_t size;
};

/*
 * 前方参照
 */
static void ungetc_rfile(struct rfile *rf, char c);

/*
 * 初期化
 */

/*
 * ファイル読み込みの初期化処理を行う
 */
bool init_file(void)
{
	return true;
}

/*
 * ファイル読み込みの終了処理を行う
 */
void cleanup_file(void)
{
}

/* JS: ファイルのサイズを取得する */
EM_ASYNC_JS(int, s2GetFileSize, (const char *dir_name, const char *file_name),
{
	/* TODO: 'sav'のファイルが存在しない場合に対応する */
	const dirName = UTF8ToString(dir_name);
	if (dirName === 'sav')
		return -1;

	const fileName = UTF8ToString(file_name);
	try{
		const subdirHandle = await dirHandle.getDirectoryHandle(dirName, { create: false });
		const fileHandle = await subdirHandle.getFileHandle(fileName, { create: false });
		const file = await fileHandle.getFile();
		const fileSize = file.size;
		return fileSize;
	} catch(e) {
		alert('s2GetFileSize(): error '+ e);
	}
	return -1;
});

/* JS: ファイルの内容を取得する */
EM_ASYNC_JS(int, s2ReadFile, (const char *dir_name, const char *file_name, char *data),
{
	const dirName = UTF8ToString(dir_name);
	const fileName = UTF8ToString(file_name);
	try{
		const subdirHandle = await dirHandle.getDirectoryHandle(dirName, { create: false });
		const fileHandle = await subdirHandle.getFileHandle(fileName, { create: false });
		const file = await fileHandle.getFile();
		const fileSize = file.size;
		const fileReader = new FileReader();
		const arrayBuffer = await new Promise((resolve, reject) => {
			fileReader.addEventListener('load', () => resolve(fileReader.result));
			fileReader.readAsArrayBuffer(file);
		});
		const u8Array = new Uint8Array(arrayBuffer);
		writeArrayToMemory(u8Array, data);
		return 0;
	} catch(e) {
		alert('s2ReadFile(): error ' + e);
	}
	return -1;
});

/*
 * ファイル読み込みストリームを開く
 */
struct rfile *open_rfile(const char *dir, const char *file, bool save_data)
{
	struct rfile *rf;
	uint64_t size;
	int result;

	/* ファイルのサイズを取得する */
	size = s2GetFileSize(dir, file);
	if (size == -1) {
		if (!save_data)
			log_dir_file_open(dir, file);
		return NULL;
	}

	/* rfile構造体のメモリを確保する */
	rf = malloc(sizeof(struct rfile));
	if (rf == NULL) {
		log_memory();
		return NULL;
	}
	rf->size = size;
	rf->pos = 0;
	if (size == 0) {
		rf->data = NULL;
		return rf;
	}

	/* メモリを確保する */
	rf->data = malloc(size);
	if (rf->data == NULL) {
		log_memory();
		return NULL;
	}
	memset(rf->data, 0, size);

	/* ファイルの内容を取得する */
	result = s2ReadFile(dir, file, rf->data);
	if (result == -1) {
		log_error("ASM ERROR.");
		free(rf);
		return NULL;
	}

	return rf;
}

/*
 * ファイルのサイズを取得する
 */
size_t get_rfile_size(struct rfile *rf)
{
	return rf->size;
}

/*
 * ファイル読み込みストリームから読み込む
 */
size_t read_rfile(struct rfile *rf, void *buf, size_t size)
{
	if (rf->pos + size > rf->size)
		size = (size_t)(rf->size - rf->pos);
	if (size == 0)
		return 0;
	memcpy(buf, rf->data + rf->pos, size);
	rf->pos += size;
	return size;
}

/* ファイル読み込みストリームに1文字戻す */
static void ungetc_rfile(struct rfile *rf, char c)
{
	/* HINT: cを戻していない */
	assert(rf->pos != 0);
	rf->pos--;
}

/*
 * ファイル読み込みストリームから1行読み込む
 */
const char *gets_rfile(struct rfile *rf, char *buf, size_t size)
{
	char *ptr;
	size_t len;
	char c;

	ptr = (char *)buf;

	for (len = 0; len < size - 1; len++) {
		if (read_rfile(rf, &c, 1) != 1) {
			*ptr = '\0';
			return len == 0 ? NULL : buf;
		}
		if (c == '\n' || c == '\0') {
			*ptr = '\0';
			return buf;
		}
		if (c == '\r') {
			if (read_rfile(rf, &c, 1) != 1) {
				*ptr = '\0';
				return buf;
			}
			if (c == '\n') {
				*ptr = '\0';
				return buf;
			}
			ungetc_rfile(rf, c);
			*ptr = '\0';
			return buf;
		}
		*ptr++ = c;
	}
	*ptr = '\0';
	return buf;
}

/*
 * ファイル読み込みストリームを閉じる
 */
void close_rfile(struct rfile *rf)
{
	assert(rf != NULL);

	free(rf->data);
	free(rf);
}

/*
 * ファイル書き込みストリームを開く
 */
struct wfile *open_wfile(const char *dir, const char *file)
{
	/* TODO */
	return NULL;
}

/*
 * ファイル書き込みストリームに書き込む
 */
size_t write_wfile(struct wfile *wf, const void *buf, size_t size)
{
	/* TODO */
	return 0;
}

/*
 * ファイル読み込みストリームを閉じる
 */
void close_wfile(struct wfile *wf)
{
	/* TODO */
}

/*
 * ファイルを削除する
 */
void remove_file(const char *dir, const char *file)
{
	/* TODO */
}
