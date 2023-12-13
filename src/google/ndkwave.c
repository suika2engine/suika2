/* -*- coding: utf-8; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Suika 2
 * Copyright (C) 2001-2016, TABATA Keiichi. All rights reserved.
 */

#include "suika.h"

/*
 * [Changes]
 * 2016/08/12 作成
 */

/*
 * 44.1kHz 16bit stereoのPCMストリーム
 */
struct wave {
	char *file;
	bool loop;
};

/*
 * ファイルからPCMストリームを作成する
 */
struct wave *create_wave_from_file(const char *dir, const char *file,
				   bool loop)
{
	struct wave *w;
	size_t len;

	/* wave構造体のメモリを確保する */
	w = malloc(sizeof(struct wave));
	if (w == NULL) {
		log_memory();
		return NULL;
	}

        /* ファイル名を保存する */
	len = strlen(dir) + 1 + strlen(file) + 1;
        w->file = malloc(len);
        if (w->file == NULL) {
		log_memory();
		free(w);
		return NULL;
        }
	strcpy(w->file, dir);
	strcat(w->file, "/");
	strcat(w->file, file);

	/* ループの有無を保存する */
	w->loop = loop;

	return w;
}

/*
 * PCMストリームのループ回数を設定する
 */
void set_wave_repeat_times(struct wave *w, int n)
{
	/* FIXME */
}

/*
 * PCMストリームを破棄する
 */
void destroy_wave(struct wave *w)
{
	free(w->file);
	free(w);
}

/*
 * PCMストリームのファイル名を取得する(NDK)
 */
const char *get_wave_file_name(struct wave *w)
{
	return w->file;
}

/*
 * PCMストリームがループ再生されるかを取得する(NDK)
 */
bool is_wave_looped(struct wave *w)
{
	return w->loop;
}
