/* -*- coding: utf-8; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Suika 2
 * Copyright (C) 2001-2022, TABATA Keiichi. All rights reserved.
 */

/*
 * [Changes]
 *  - 2016/06/28 作成
 */

#include "suika.h"

#ifdef _MSC_VER
#define strcasecmp _stricmp
#endif

#ifdef SUIKA_TARGET_WIN32
#include <fcntl.h>
#endif

/* Obfuscation Key */
#include "key.h"

static volatile uint64_t key_obfuscated =
	~((((OBFUSCATION_KEY >> 56) & 0xff) << 0) |
	  (((OBFUSCATION_KEY >> 48) & 0xff) << 8) |
	  (((OBFUSCATION_KEY >> 40) & 0xff) << 16) |
	  (((OBFUSCATION_KEY >> 32) & 0xff) << 24) |
	  (((OBFUSCATION_KEY >> 24) & 0xff) << 32) |
	  (((OBFUSCATION_KEY >> 16) & 0xff) << 40) |
	  (((OBFUSCATION_KEY >> 8)  & 0xff) << 48) |
	  (((OBFUSCATION_KEY >> 0)  & 0xff) << 56));
static volatile uint64_t key_reversed;
static volatile uint64_t *key_ref = &key_reversed;

/* ファイル読み込みストリーム */
struct rfile {
	/* パッケージ内のファイルであるか */
	bool is_packaged;

	/* 難読化されているか */
	bool is_obfuscated;

	/* パッケージファイルもしくは個別のファイルへのファイルポインタ */
	FILE *fp;

	/* パッケージ内のファイルを使う場合にのみ用いる情報 */
	uint64_t index;
	uint64_t size;
	uint64_t offset;
	uint64_t pos;
	uint64_t next_random;
	uint64_t prev_random;
};

/* ファイル書き込みストリーム */
struct wfile {
	FILE *fp;
	uint64_t next_random;
};

/* パッケージのファイルエントリ */
static struct file_entry entry[FILE_ENTRY_SIZE];

/* パッケージのファイルエントリ数 */
static uint64_t entry_count;

/* パッケージファイルのパス */
static char *package_path;

#ifdef SUIKA_TARGET_WIN32
const wchar_t *conv_utf8_to_utf16(const char *s);
#endif

/*
 * 前方参照
 */
#if 0
static bool check_file_name(const char *file);
#endif
static void ungetc_rfile(struct rfile *rf, char c);
static void set_random_seed(uint64_t index, uint64_t *next_random);
static char get_next_random(uint64_t *next_random, uint64_t *prev_random);
static void rewind_random(uint64_t *next_random, uint64_t *prev_random);

/*
 * 初期化
 */

/*
 * ファイル読み込みの初期化処理を行う
 */
bool init_file(void)
{
#if defined(USE_EDITOR) || defined(USE_DEBUGGER)
	/* ユーザの気持ちを考えて、デバッガ版ではパッケージを開けない */
	return true;
#else
	FILE *fp;
	uint64_t i, next_random;
	int j;

	/* パッケージファイルのパスを求める */
	package_path = make_valid_path(NULL, PACKAGE_FILE);
	if (package_path == NULL)
		return false;

	/* パッケージファイルを開いてみる */
#ifdef SUIKA_TARGET_WIN32
	_fmode = _O_BINARY;
	fp = _wfopen(conv_utf8_to_utf16(package_path), L"r");
#else
	fp = fopen(package_path, "r");
#endif
	if (fp == NULL) {
		free(package_path);
		package_path = NULL;
#if defined(SUIKA_TARGET_IOS) || defined(SUIKA_TARGET_WASM)
		/* 開ける必要がある */
		return false;
#else
		/* 開けなくてもよい */
		return true;
#endif
	}

	/* パッケージのファイルエントリ数を取得する */
	if (fread(&entry_count, sizeof(uint64_t), 1, fp) < 1) {
		log_package_file_error();
		fclose(fp);
		return false;
	}
	if (entry_count > FILE_ENTRY_SIZE) {
		log_package_file_error();
		fclose(fp);
		return false;
	}

	/* パッケージのファイルエントリを読み込む */
	for (i = 0; i < entry_count; i++) {
		if (fread(&entry[i].name, FILE_NAME_SIZE, 1, fp) < 1)
			break;
		set_random_seed(i, &next_random);
		for (j = 0; j < FILE_NAME_SIZE; j++)
			entry[i].name[j] ^= get_next_random(&next_random, NULL);
		if (fread(&entry[i].size, sizeof(uint64_t), 1, fp) < 1)
			break;
		if (fread(&entry[i].offset, sizeof(uint64_t), 1, fp) < 1)
			break;
	}
	if (i != entry_count) {
		log_package_file_error();
		fclose(fp);
		return false;
	}

	fclose(fp);
	return true;
#endif
}

/*
 * ファイル読み込みの終了処理を行う
 */
void cleanup_file(void)
{
	free(package_path);
}

/*
 * 読み込み
 */

/*
 * ファイル読み込みストリームを開く
 */
struct rfile *open_rfile(const char *dir, const char *file, bool save_data)
{
	char entry_name[FILE_NAME_SIZE];
	char *real_path;
	struct rfile *rf;
	uint64_t i;

#if 0
	/* ファイル名に半角英数字以外が含まれるかチェックする */
	if (!check_file_name(file)) {
		log_file_name(dir, file);
		return NULL;
	}
#endif

	/* rfile構造体のメモリを確保する */
	rf = malloc(sizeof(struct rfile));
	if (rf == NULL) {
		log_memory();
		return NULL;
	}

	/* パスを生成する */
	real_path = make_valid_path(dir, file);
	if (real_path == NULL) {
		log_memory();
		free(rf);
		return NULL;
	}

	/* まずファイルシステム上のファイルを開いてみる */
#ifdef SUIKA_TARGET_WIN32
	_fmode = _O_BINARY;
	rf->fp = _wfopen(conv_utf8_to_utf16(real_path), L"r");
#else
	rf->fp = fopen(real_path, "r");
#endif
	if (rf->fp != NULL) {
		/* 開けた場合、ファイルシステム上のファイルを用いる */
		free(real_path);
		rf->is_packaged = false;
		if (save_data) {
			rf->is_obfuscated = true;
			set_random_seed(0, &rf->next_random);
		} else {
			rf->is_obfuscated = false;
		}
		return rf;
	}
	free(real_path);

	/* セーブデータはパッケージから探さない */
	if (save_data) {
		free(rf);
		return NULL;
	}

	/* パッケージがなければエラーとする */
	if (package_path == NULL) {
		log_dir_file_open(dir, file);
		free(rf);
		return NULL;
	}

	/* 次にパッケージ上のファイルエントリを探す(TODO: sort and search) */
	snprintf(entry_name, FILE_NAME_SIZE, "%s/%s", dir, file);
	for (i = 0; i < entry_count; i++) {
		if (strcasecmp(entry[i].name, entry_name) == 0)
			break;
	}
	if (i == entry_count) {
		/* みつからなかった場合 */
		log_dir_file_open(dir, file);
		free(rf);
		return NULL;
	}

	/* みつかった場合、パッケージファイルを別なファイルポインタで開く */
#ifdef SUIKA_TARGET_WIN32
	_fmode = _O_BINARY;
	rf->fp = _wfopen(conv_utf8_to_utf16(package_path), L"r");
#else
	rf->fp = fopen(package_path, "r");
#endif
	if (rf->fp == NULL) {
		log_file_open(PACKAGE_FILE);
		free(rf);
		return NULL;
	}

	/* 読み込み位置にシークする */
	if (fseek(rf->fp, (long)entry[i].offset, SEEK_SET) != 0) {
		log_package_file_error();
		fclose(rf->fp);
		free(rf);
		return 0;
	}

	rf->is_packaged = true;
	rf->is_obfuscated = true;
	rf->index = i;
	rf->size = entry[i].size;
	rf->offset = entry[i].offset;
	rf->pos = 0;
	set_random_seed(i, &rf->next_random);
	rf->prev_random = 0;

	return rf;
}

#if 0
/* ファイル名に半角英数字以外が含まれるかチェックする */
static bool check_file_name(const char *file)
{
	const char *c;

	c = file;
	while (*c) {
		/* UTF-8文字列に半角英数字以外が含まれる場合 */
		if (*c & 0x80)
			return false;
		c++;
	}

	return true;
}
#endif

/*
 * ファイルのサイズを取得する
 */
size_t get_rfile_size(struct rfile *rf)
{
	long pos, len;

	/* ファイルシステム上のファイルの場合 */
	if (!rf->is_packaged) {
		pos = ftell(rf->fp);
		fseek(rf->fp, 0, SEEK_END);
		len = ftell(rf->fp);
		fseek(rf->fp, pos, SEEK_SET);
		return (size_t)len;
	}

	/* パッケージ内のファイルの場合 */
	return (size_t)rf->size;
}

/*
 * ファイル読み込みストリームから読み込む
 */
size_t read_rfile(struct rfile *rf, void *buf, size_t size)
{
	size_t len, obf;

	assert(rf != NULL);
	assert(rf->fp != NULL);

	if (!rf->is_packaged) {
		/* ファイルシステム上のファイルの場合 */
		len = fread(buf, 1, size, rf->fp);

		/* 難読化を解除する */
		if (rf->is_obfuscated) {
			for (obf = 0; obf < len; obf++) {
				*(((char *)buf) + obf) ^=
					get_next_random(&rf->next_random,
							NULL);
			}
		}
	} else {
		/* パッケージ内のファイルの場合 */
		if (rf->pos + size > rf->size)
			size = (size_t)(rf->size - rf->pos);
		if (size == 0)
			return 0;
		len = fread(buf, 1, size, rf->fp);
		rf->pos += len;

		/* 難読化を解除する */
		for (obf = 0; obf < len; obf++) {
			*(((char *)buf) + obf) ^=
				get_next_random(&rf->next_random,
						&rf->prev_random);
		}
	}

	return len;
}

/*
 * ファイル読み込みストリームから1行読み込む
 */
const char *gets_rfile(struct rfile *rf, char *buf, size_t size)
{
	char *ptr;
	size_t len, ret;
	char c;

	assert(rf != NULL);
	assert(rf->fp != NULL);
	assert(buf != NULL);
	assert(size > 0);

	ptr = buf;
	for (len = 0; len < size - 1; len++) {
		ret = read_rfile(rf, &c, 1);
		if (ret != 1) {
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
	if (len == 0)
		return NULL;
	return buf;
}

/* ファイル読み込みストリームに1文字戻す */
static void ungetc_rfile(struct rfile *rf, char c)
{
	assert(rf != NULL);
	assert(rf->fp != NULL);

	if (!rf->is_packaged) {
		/* ファイルシステム上のファイルの場合 */
		ungetc(c, rf->fp);
	} else {
		/* パッケージ内のファイルの場合 */
		assert(rf->pos != 0);
		ungetc(c, rf->fp);
		rf->pos--;
		rewind_random(&rf->next_random, &rf->prev_random);
	}
}

/*
 * ファイル読み込みストリームを閉じる
 */
void close_rfile(struct rfile *rf)
{
	assert(rf != NULL);
	assert(rf->fp != NULL);

	fclose(rf->fp);
	free(rf);
}

/* ファイルの乱数シードを設定する */
static void set_random_seed(uint64_t index, uint64_t *next_random)
{
	uint64_t i, next, lsb;

	key_reversed = ((((key_obfuscated >> 56) & 0xff) << 0) |
			(((key_obfuscated >> 48) & 0xff) << 8) |
			(((key_obfuscated >> 40) & 0xff) << 16) |
			(((key_obfuscated >> 32) & 0xff) << 24) |
			(((key_obfuscated >> 24) & 0xff) << 32) |
			(((key_obfuscated >> 16) & 0xff) << 40) |
			(((key_obfuscated >> 8)  & 0xff) << 48) |
			(((key_obfuscated >> 0)  & 0xff) << 56));
	next = ~(*key_ref);
	for (i = 0; i < index; i++) {
		next ^= 0xafcb8f2ff4fff33f;
		lsb = next >> 63;
		next = (next << 1) | lsb;
	}

	*next_random = next;
}

/* 乱数を取得する */
static char get_next_random(uint64_t *next_random, uint64_t *prev_random)
{
	uint64_t next;
	char ret;

	/* ungetc()用 */
	if (prev_random != NULL)
		*prev_random = *next_random;

	ret = (char)(*next_random);
	next = *next_random;
	next = (((~(*key_ref) & 0xff00) * next + (~(*key_ref) & 0xff)) %
		~(*key_ref)) ^ 0xfcbfaff8f2f4f3f0;
	*next_random = next;

	return ret;
}

/* 乱数を1つ元に戻す */
static void rewind_random(uint64_t *next_random, uint64_t *prev_random)
{
	*next_random = *prev_random;
	*prev_random = 0;
}

/*
 * 書き込み
 */

/*
 * ファイル書き込みストリームを開く
 */
struct wfile *open_wfile(const char *dir, const char *file)
{
	char *path;
	struct wfile *wf;

	/* wfile構造体のメモリを確保する */
	wf = malloc(sizeof(struct wfile));
	if (wf == NULL) {
		log_memory();
		return NULL;
	}

	/* パスを生成する */
	path = make_valid_path(dir, file);
	if (path == NULL) {
		log_memory();
		free(wf);
		return NULL;
	}

	/* ファイルをオープンする */
#ifdef SUIKA_TARGET_WIN32
	_fmode = _O_BINARY;
	wf->fp = _wfopen(conv_utf8_to_utf16(path), L"w");
#else
	wf->fp = fopen(path, "w");
#endif
	if (wf->fp == NULL) {
		log_file_open(path);
		free(path);
		free(wf);
		return NULL;
	}
	free(path);

	/* 乱数シードを初期化する */
	set_random_seed(0, &wf->next_random);

	return wf;
}

/*
 * ファイル書き込みストリームに書き込む
 */
size_t write_wfile(struct wfile *wf, const void *buf, size_t size)
{
	char obf[1024];
	const char *src;
	size_t block_size, out, total, i;

	assert(wf != NULL);
	assert(wf->fp != NULL);

	src = buf;
	total = 0;
	while (size > 0) {
		/* ブロックのサイズを求める(最大1024バイト) */
		if (size > sizeof(obf))
			block_size = sizeof(obf);
		else
			block_size = size;

		/* ブロックを難読化する */
		for (i = 0; i < block_size; i++) {
			obf[i] = *src++ ^ get_next_random(&wf->next_random,
							  NULL);
		}

		/* ブロックを書き出す */
		out = fwrite(obf, 1, block_size, wf->fp);
		if (out != block_size)
			return total + out;
		total += out;
		size -= out;
	}
	return total;
}

/*
 * ファイル読み込みストリームを閉じる
 */
void close_wfile(struct wfile *wf)
{
	assert(wf != NULL);
	assert(wf->fp != NULL);

	fflush(wf->fp);
	fclose(wf->fp);
	free(wf);
}

/*
 * ファイルを削除する
 */
void remove_file(const char *dir, const char *file)
{
	char *path;

	/* パスを生成する */
	path = make_valid_path(dir, file);
	if (path == NULL) {
		log_memory();
		return;
	}

	/* ファイルを削除する */
	remove(path);

	/* パスの文字列を解放する */
	free(path);
}
