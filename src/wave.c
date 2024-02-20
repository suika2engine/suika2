/* -*- coding: utf-8; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Suika2
 * Copyright (C) 2001-2023, Keiichi Tabata. All rights reserved.
 */

/*
 * [Changes]
 * 2003/05/03 作成
 * 2004/01/10 LXVorbisInputStream (2003/6を元に改造)
 * 2016/06/04 struct wave
 * 2016/06/17 vorbisfileに書き換え
 * 2023/10/01 LOOPSTARTに対応
 */

#include "suika.h"

/* vorbisfile.hのconversionの問題を無視する */
#if defined(__llvm__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wshorten-64-to-32"
#include <vorbis/vorbisfile.h>
#pragma clang diagnostic pop
#elif defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
#include <vorbis/vorbisfile.h>
#pragma GCC diagnostic pop
#else
#include <vorbis/vorbisfile.h>
#endif

#define SAMPLING_RATE	(44100)
#define IOSIZE		(4096)

/*
 * 44.1kHz 16bit stereoのPCMストリーム
 */
struct wave {
	/* 入力ファイル */
	char *dir;
	char *file;
	struct rfile *rf;
	bool loop;
	uint32_t loop_start;
	uint32_t loop_length;
	int times;	/* loop=trueのとき、-1なら無限、0以上は残り回数 */
	bool monaural;

	/* 状態 */
	bool eos;
	bool err;
	bool do_skip;
	long consumed_bytes;

	/* Vorbisのオブジェクト */
	OggVorbis_File ovf;
};

/*
 * 前方参照
 */
static bool reopen(struct wave *w, bool loop);
static size_t read_func(void *ptr, size_t size, size_t nmemb,
			void *datasource);
static int close_func(void *datasource);
static int get_wave_samples_monaural(struct wave *w, uint32_t *buf, int samples);
static int get_wave_samples_stereo(struct wave *w, uint32_t *buf, int samples);
static void skip_if_needed(struct wave *w, int sample_bytes);

/*
 * ファイルからPCMストリームを作成する
 */
struct wave *create_wave_from_file(const char *dir, const char *fname,
				   bool loop)
{
	struct wave *w;
	vorbis_info *vi;
	vorbis_comment *vc;
	int i;

	const char *LOOPSTART = "LOOPSTART=";
	const char *LOOPLENGTH = "LOOPLENGTH=";
	const size_t LOOPSTART_LEN = strlen(LOOPSTART);
	const size_t LOOPLENGTH_LEN = strlen(LOOPLENGTH);

	UNUSED_PARAMETER(OV_CALLBACKS_DEFAULT);
	UNUSED_PARAMETER(OV_CALLBACKS_NOCLOSE);
	UNUSED_PARAMETER(OV_CALLBACKS_STREAMONLY);
	UNUSED_PARAMETER(OV_CALLBACKS_STREAMONLY_NOCLOSE);

	/* wave構造体のメモリを確保する */
	w = malloc(sizeof(struct wave));
	if (w == NULL) {
		log_memory();
		return NULL;
	}

	/* ディレクトリ名を保存する */
	w->dir = strdup(dir);
	if (w->dir == NULL) {
		log_memory();
		free(w);
		return NULL;
	}

	/* ファイル名を保存する */
	w->file = strdup(fname);
	if (w->file == NULL) {
		log_memory();
		free(w->dir);
		free(w);
		return NULL;
	}

	/* ファイルをオープンする */
	if (!reopen(w, false))
		return NULL;

	/* TODO: ov_info()でサンプリングレートとチャンネル数をチェック */
	vi = ov_info(&w->ovf, -1);
	w->monaural = vi->channels == 1 ? true : false;

	/* wave構造体を初期化する */
	w->loop = loop;
	w->loop_start = 0;
	w->loop_length = 0;
	w->times = -1;
	w->eos = false;
	w->err = false;
	w->do_skip = false;
	w->consumed_bytes = 0;

	/* LOOPSTARTとLOOPLENGTHを取得する */
	vc = ov_comment(&w->ovf, -1);
	if (vc != NULL) {
		for (i = 0; i < vc->comments; i++) {
			if (strncmp(vc->user_comments[i], LOOPSTART, LOOPSTART_LEN) == 0) {
				w->loop = true;
				w->loop_start = (uint32_t)atol(vc->user_comments[i] + LOOPSTART_LEN);
			} else if (strncmp(vc->user_comments[i], LOOPLENGTH, LOOPLENGTH_LEN) == 0) {
				w->loop = true;
				w->loop_length = (uint32_t)atol(vc->user_comments[i] + LOOPLENGTH_LEN);
			}
		}
	}

	/* 成功 */
	return w;
}

/* ファイルをリオープンする */
static bool reopen(struct wave *w, bool loop)
{
	ov_callbacks cb;
	int err;

	/* ファイル入力ストリームを開く */
	w->rf = open_rfile(w->dir, w->file, false);
	if (w->rf == NULL) {
		free(w->file);
		free(w->dir);
		free(w);
		return false;
	}

	/* コールバックを使ってファイルを開く */
	memset(&cb, 0, sizeof(cb));
	cb.read_func = read_func;
	cb.close_func = close_func;
	cb.seek_func = NULL;
	cb.tell_func = NULL;
	err = ov_open_callbacks(w, &w->ovf, NULL, 0, cb);
	if (err != 0) {
		log_audio_file_error(w->dir, w->file);
		free(w->file);
		free(w->dir);
		free(w);
		return false;
	}

	w->do_skip = loop;
	w->consumed_bytes = 0;

	return true;
}

/* ファイル読み込みコールバック */
static size_t read_func(void *ptr, size_t size, size_t nmemb, void *datasource)
{
	struct wave *w;	
	size_t len;

	assert(ptr != NULL);
	assert(datasource != NULL);

	w = (struct wave *)datasource;

	len = read_rfile(w->rf, ptr, size * nmemb);

	return len / size;
}

/* ファイルクローズコールバック */
static int close_func(void *datasource)
{
	struct wave *w;

	assert(datasource != NULL);

	w = (struct wave *)datasource;

	close_rfile(w->rf);

	return 0;
}

/*
 * PCMストリームのループ回数を設定する
 */
void set_wave_repeat_times(struct wave *w, int n)
{
	assert(w != NULL);
	assert(n > 0);

	w->times = n;
}

/*
 * PCMストリームを破棄する
 */
void destroy_wave(struct wave *w)
{
	ov_clear(&w->ovf);
	free(w->dir);
	free(w->file);
	free(w);
}

/*
 * PCMストリームが終端に達しているか取得する
 */
bool is_wave_eos(struct wave *w)
{
	return w->eos;
}

/*
 * PCMストリームからサンプルを取得する
 */
int get_wave_samples(struct wave *w, uint32_t *buf, int samples)
{
	/* 再生が終了している場合 */
	if (w->eos)
		return 0;

	/* モノラルの場合 */
	if (w->monaural)
		return get_wave_samples_monaural(w, buf, samples);

	/* ステレオの場合 */
	return get_wave_samples_stereo(w, buf, samples);
}

/* モノラルのサンプルを取得する */
static int get_wave_samples_monaural(struct wave *w, uint32_t *buf, int samples)
{
	unsigned char mbuf[IOSIZE];
	long read_bytes, ret_bytes, last_ret_bytes;
	int retain, bitstream, i;
	bool loop_end;

	/* サンプルの取得が完了するか、終端に達するまで続ける */
	retain = 0;
	last_ret_bytes = -1;
	while (retain < samples) {
		/* LOOPSTARTのためのスキップを処理する */
		skip_if_needed(w, 2);

		/* デコードする */
		read_bytes = (samples - retain) * 2 > IOSIZE ? IOSIZE :
			     (samples - retain) * 2;
		loop_end = false;
		if (w->loop_length > 0 &&
		    w->consumed_bytes + read_bytes >= (long)w->loop_start + (long)w->loop_length) {
			read_bytes = (long)(w->loop_start + w->loop_length) - w->consumed_bytes;
			loop_end = true;
			w->consumed_bytes += read_bytes;
		}
		ret_bytes = ov_read(&w->ovf, (char *)mbuf, (int)read_bytes, 0, 2, 1, &bitstream);
		if (ret_bytes == 0) {
			/* 終端に達した */
			if ((w->loop && (w->times == -1 || w->times > 0)) || loop_end) {
				/* ストリームを再度オープンする */
				if (last_ret_bytes == 0)
					return 0; 	/* エラー */
				ov_clear(&w->ovf);
				if (!reopen(w, true))
					return 0;	/* エラー */
				last_ret_bytes = 0;
				if (w->times != -1)
					w->times--;
			} else {
				/* 読み込んだサンプル数を返す */
				w->eos = true;
				return retain;
			}
		}

		/* ステレオに変換する */
		for (i = 0; i < ret_bytes / 2; i++) {
			buf[retain + i] = (uint32_t)(mbuf[i*2] |
						     (mbuf[i*2 + 1] << 8) |
						     (mbuf[i*2] << 16) |
						     (mbuf[i*2 + 1] << 24));
		}
		retain += (int)ret_bytes / 2;
	}

	/* 指定されたサンプル数の分だけ取得できた */
	return samples;
}

/* ステレオのサンプルを取得する */
static int get_wave_samples_stereo(struct wave *w, uint32_t *buf, int samples)
{
	long read_bytes, ret_bytes, last_ret_bytes;
	int retain, bitstream;
	bool loop_end;

	/* サンプルの取得が完了するか、終端に達するまで続ける */
	retain = 0;
	last_ret_bytes = -1;
	while (retain < samples) {
		/* LOOPSTARTのためのスキップを処理する */
		skip_if_needed(w, 4);

		/* デコードする */
		read_bytes = (samples - retain) * 4;
		loop_end = false;
		if (w->loop_length > 0 &&
		    w->consumed_bytes + read_bytes >= (long)w->loop_start + (long)w->loop_length) {
			read_bytes = (long)(w->loop_start + w->loop_length) - w->consumed_bytes;
			loop_end = true;
			w->consumed_bytes += read_bytes;
		}
		ret_bytes = ov_read(&w->ovf, (char *)(buf + retain), (int)read_bytes, 0, 2, 1, &bitstream);
		if (ret_bytes == 0) {
			/* 終端に達した */
			if ((w->loop && (w->times == -1 || w->times > 0)) || loop_end) {
				/* ストリームを再度オープンする */
				if (last_ret_bytes == 0)
					return 0; 	/* エラー */
				ov_clear(&w->ovf);
				if (!reopen(w, true))
					return 0;	/* エラー */
				last_ret_bytes = 0;
				if (w->times != -1)
					w->times--;
			} else {
				/* 読み込んだサンプル数を返す */
				w->eos = true;
				return retain;
			}
		}
		retain += (int)ret_bytes / 4;
	}

	/* 指定されたサンプル数の分だけ取得できた */
	return samples;
}

/* LOOPSTART分をスキップする */
static void skip_if_needed(struct wave *w, int sample_bytes)
{
	uint32_t buf[1024];
	size_t remain_samples;
	size_t get_samples;
	long ret_bytes;
	int bitstream;

	if (!w->do_skip || w->loop_start == 0)
		return;

	remain_samples = w->loop_start;
	while (remain_samples > 0) {
		get_samples = remain_samples > 1024 ? 1024 : remain_samples;

		ret_bytes = ov_read(&w->ovf,
				    (char *)buf,
				    (int)get_samples * sample_bytes,
				    0,
				    2,
				    1,
				    &bitstream);
		if (ret_bytes <= 0)
			break;

		remain_samples -= (size_t)(ret_bytes / sample_bytes);
	}

	w->do_skip = false;
	w->consumed_bytes = (long)w->loop_start;
}
