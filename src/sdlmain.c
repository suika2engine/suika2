/* -*- tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Suika 2
 * Copyright (C) 2001-2022, TABATA Keiichi. All rights reserved.
 */

/*
 * [Changes]
 *  2022-11-08 Created
 */

#include <SDL.h>

#include <locale.h>

#include <sys/types.h>
#include <sys/stat.h>	/* stat(), mkdir() */
#include <sys/time.h>	/* gettimeofday() */

#include "suika.h"

#ifdef SSE_VERSIONING
#include "x86.h"
#endif

/*
 * The Log File
 */
static FILE *log_fp;

/*
 * SDL2 Objects
 */
static SDL_Window *window;
static SDL_Renderer *render;

/*
 * forward declaration
 */
static bool init(int argc, char *argv[]);
static void cleanup(void);
static bool open_log_file(void);
static void close_log_file(void);
static void run_game_loop(void);

/*
 * メイン
 */
int main(int argc, char *argv[])
{
	int ret;

	setlocale(LC_ALL, "");

	/* 互換レイヤの初期化処理を行う */
	if (init(argc, argv)) {
		/* アプリケーション本体の初期化処理を行う */
		if (on_event_init()) {
			/* ゲームループを実行する */
			run_game_loop();

			/* 成功 */
			ret = 0;
		} else {
			/* 失敗 */
			ret = 1;
		}

		/* アプリケーション本体の終了処理を行う */
		on_event_cleanup();
	} else {
		/* エラーメッセージを表示する */
		if (log_fp != NULL)
			printf("Check " LOG_FILE "\n");

		/* 失敗 */
		ret = 1;
	}

	/* 互換レイヤの終了処理を行う */
	cleanup();

	return ret;
}

/* 互換レイヤの初期化処理を行う */
static bool init(int argc, char *argv[])
{
	UNUSED_PARAMETER(argc);
	UNUSED_PARAMETER(argv);

#ifdef SSE_VERSIONING
	/* ベクトル命令の対応を確認する */
	x86_check_cpuid_flags();
#endif

	/* Initialize the SDL2. */
	if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO) != 0) {
		log_error("Failed to initialize SDL: %s", SDL_GetError());
		return false;
	}

	/* ロケールを初期化する */
	init_locale_code();

	/* ログファイルを開く */
	if (!open_log_file())
		return false;

	/* ファイル読み書きの初期化処理を行う */
	if (!init_file())
		return false;

	/* コンフィグの初期化処理を行う */
	if (!init_conf())
		return false;

	/* Create a window. */
	window = SDL_CreateWindow(conf_window_title,
				  SDL_WINDOWPOS_CENTERED,
				  SDL_WINDOWPOS_CENTERED,
				  conf_window_width,
				  conf_window_height,
				  0);

	/* Create a rendering context. */
	render = SDL_CreateRenderer(window, -1, 0);
	
	return true;
}

/* 互換レイヤの終了処理を行う */
static void cleanup(void)
{
	/* コンフィグの終了処理を行う */
	cleanup_conf();

	/* ファイル読み書きの終了処理を行う */
	cleanup_file();

	/* ログファイルを閉じる */
	close_log_file();
}

/*
 * ログ
 */

/* ログをオープンする */
static bool open_log_file(void)
{
	if (log_fp == NULL) {
		log_fp = fopen(LOG_FILE, "w");
		if (log_fp == NULL) {
			printf("Can't open log file.\n");
			return false;
		}
	}
	return true;
}

/* ログをクローズする */
static void close_log_file(void)
{
	if (log_fp != NULL)
		fclose(log_fp);
}

/*
 * SDL2 Event Processing
 */

/* The event loop. */
static void run_game_loop(void)
{
	SDL_Event ev;
	int x, y, w, h;
	bool cont;

	cont = true;
	do {
		while (SDL_PollEvent(&ev)) {
			switch (ev.type) {
			case SDL_QUIT:
				return;
			}
		}

		/* Clear the background. */
		if (conf_window_white)
			SDL_SetRenderDrawColor(render, 0xff, 0xff, 0xff, 0xff);
		else
			SDL_SetRenderDrawColor(render, 0, 0, 0, 0xff);
		SDL_RenderClear(render);

		/* Do a frame event. */
		cont = on_event_frame(&x, &y, &w, &h);

		/* Show a frame. */
		SDL_RenderPresent(render);
	} while (cont);
}

/*
 * platform.hの実装
 */

/*
 * INFOログを出力する
 */
bool log_info(const char *s, ...)
{
	va_list ap;

	va_start(ap, s);
	if (log_fp != NULL) {
		vfprintf(stderr, s, ap);
		vfprintf(log_fp, s, ap);
		fflush(log_fp);
		if (ferror(log_fp))
			return false;
	}
	va_end(ap);
	return true;
}

/*
 * WARNログを出力する
 */
bool log_warn(const char *s, ...)
{
	va_list ap;

	va_start(ap, s);
	if (log_fp != NULL) {
		vfprintf(stderr, s, ap);
		vfprintf(log_fp, s, ap);
		fflush(log_fp);
		if (ferror(log_fp))
			return false;
	}
	va_end(ap);
	return true;
}

/*
 * ERRORログを出力する
 */
bool log_error(const char *s, ...)
{
	va_list ap;

	va_start(ap, s);
	if (log_fp != NULL) {
		vfprintf(stderr, s, ap);
		vfprintf(log_fp, s, ap);
		fflush(log_fp);
		if (ferror(log_fp))
			return false;
	}
	va_end(ap);
	return true;
}

/*
 * UTF-8のメッセージをネイティブの文字コードに変換する
 *  - 変換の必要がないので引数をそのまま返す
 */
const char *conv_utf8_to_native(const char *utf8_message)
{
	assert(utf8_message != NULL);
	return utf8_message;
}

/*
 * GPUを使うか調べる
 */
bool is_gpu_accelerated(void)
{
	return true;
}

/*
 * OpenGLが有効か調べる
 */
bool is_opengl_enabled(void)
{
	return true;
}

/*
 * テクスチャをロックする
 */
bool lock_texture(int width, int height, pixel_t *pixels,
		  pixel_t **locked_pixels, void **texture)
{
	UNUSED_PARAMETER(width);
	UNUSED_PARAMETER(height);
	UNUSED_PARAMETER(pixels);
	UNUSED_PARAMETER(locked_pixels);
	UNUSED_PARAMETER(texture);

	/* We'll just draw on "pixels" */
	*locked_pixels = pixels;

	return true;
}

/*
 * テクスチャをアンロックする
 */
void unlock_texture(int width, int height, pixel_t *pixels,
		    pixel_t **locked_pixels, void **texture)
{
	SDL_Surface *surface;
	SDL_Texture *tex;

	UNUSED_PARAMETER(width);
	UNUSED_PARAMETER(height);
	UNUSED_PARAMETER(pixels);
	UNUSED_PARAMETER(locked_pixels);
	UNUSED_PARAMETER(texture);

	/* Re-create a SDL_Texture. */
	surface = SDL_CreateRGBSurfaceFrom(pixels,
					   width,
					   height,
					   32,
					   width * 4,
					   0x000000ff,
					   0x0000ff00,
					   0x00ff0000,
					   0xff000000);
	tex = SDL_CreateTextureFromSurface(render, surface);
	SDL_FreeSurface(surface);

	/* Free the old texture, and retain the new one. */
	if (*texture != NULL)
		SDL_DestroyTexture(*texture);
	*texture = tex;

	*locked_pixels = NULL;
}

/*
 * テクスチャを破棄する
 */
void destroy_texture(void *texture)
{
	assert(texture != NULL);
	SDL_DestroyTexture(texture);
}

/*
 * イメージをレンダリングする
 */
void render_image(int dst_left, int dst_top, struct image * RESTRICT src_image,
                  int width, int height, int src_left, int src_top, int alpha,
                  int bt)
{
	UNUSED_PARAMETER(dst_left);
	UNUSED_PARAMETER(dst_top);
	UNUSED_PARAMETER(src_image);
	UNUSED_PARAMETER(width);
	UNUSED_PARAMETER(height);
	UNUSED_PARAMETER(src_left);
	UNUSED_PARAMETER(src_top);
	UNUSED_PARAMETER(alpha);
	UNUSED_PARAMETER(bt);
	SDL_Rect s_rc, d_rc;

	d_rc.x = dst_left;
	d_rc.y = dst_top;
	d_rc.w = width;
	d_rc.h = height;

	s_rc.x = src_left;
	s_rc.y = src_top;
	s_rc.w = width;
	s_rc.h = height;

	SDL_RenderCopy(render, get_texture_object(src_image), &s_rc, &d_rc);
}

/*
 * 画面にイメージをテンプレート指定でレンダリングする
 */
void render_image_rule(struct image * RESTRICT src_img,
		       struct image * RESTRICT rule_img,
		       int threshold)
{
	UNUSED_PARAMETER(src_img);
	UNUSED_PARAMETER(rule_img);
	UNUSED_PARAMETER(threshold);
}

/*
 * セーブディレクトリを作成する
 */
bool make_sav_dir(void)
{
	struct stat st = {0};

	if (stat(SAVE_DIR, &st) == -1)
		mkdir(SAVE_DIR, 0700);

	return true;
}

/*
 * データファイルのディレクトリ名とファイル名を指定して有効なパスを取得する
 */
char *make_valid_path(const char *dir, const char *fname)
{
	char *buf;
	size_t len;

	if (dir == NULL)
		dir = "";

	/* パスのメモリを確保する */
	len = strlen(dir) + 1 + strlen(fname) + 1;
	buf = malloc(len);
	if (buf == NULL) {
		log_memory();
		return NULL;
	}

	strcpy(buf, dir);
	if (strlen(dir) != 0)
		strcat(buf, "/");
	strcat(buf, fname);

	return buf;
}

/*
 * タイマをリセットする
 */
void reset_stop_watch(stop_watch_t *t)
{
	struct timeval tv;

	gettimeofday(&tv, NULL);

	*t = (stop_watch_t)(tv.tv_sec * 1000 + tv.tv_usec / 1000);
}

/*
 * タイマのラップをミリ秒単位で取得する
 */
int get_stop_watch_lap(stop_watch_t *t)
{
	struct timeval tv;
	stop_watch_t end;
	
	gettimeofday(&tv, NULL);

	end = (stop_watch_t)(tv.tv_sec * 1000 + tv.tv_usec / 1000);

	if (end < *t) {
		/* オーバーフローの場合、タイマをリセットして0を返す */
		reset_stop_watch(t);
		return 0;
	}

	return (int)(end - *t);
}

/*
 * 終了ダイアログを表示する
 */
bool exit_dialog(void)
{
	/* stub */
	return true;
}

/*
 * タイトルに戻るダイアログを表示する
 */
bool title_dialog(void)
{
	/* stub */
	return true;
}

/*
 * 削除ダイアログを表示する
 */
bool delete_dialog(void)
{
	/* stub */
	return true;
}

/*
 * 上書きダイアログを表示する
 */
bool overwrite_dialog(void)
{
	/* stub */
	return true;
}

/*
 * 初期設定ダイアログを表示する
 */
bool default_dialog(void)
{
	/* stub */
	return true;
}

/*
 * ビデオを再生する
 */
bool play_video(const char *fname, bool is_skippable)
{
	UNUSED_PARAMETER(fname);
	UNUSED_PARAMETER(is_skippable);
	return true;
}

/*
 * ビデオを停止する
 */
void stop_video(void)
{
}

/*
 * ビデオが再生中か調べる
 */
bool is_video_playing(void)
{
	return false;
}

/*
 * ウィンドウタイトルを更新する
 */
void update_window_title(void)
{
}

/*
 * フルスクリーンモードがサポートされるか調べる
 */
bool is_full_screen_supported(void)
{
	return false;
}

/*
 * フルスクリーンモードであるか調べる
 */
bool is_full_screen_mode(void)
{
	return false;
}

/*
 * フルスクリーンモードを開始する
 */
void enter_full_screen_mode(void)
{
	/* stub */
}

/*
 * フルスクリーンモードを終了する
 */
void leave_full_screen_mode(void)
{
	/* stub */
}

/*
 * システムのロケールを取得する
 */
const char *get_system_locale(void)
{
	const char *locale;

	locale = setlocale(LC_ALL, "");
	if (locale[0] == '\0' || locale[1] == '\0')
		return "en";
	else if (strncmp(locale, "en", 2) == 0)
		return "en";
	else if (strncmp(locale, "fr", 2) == 0)
		return "fr";
	else if (strncmp(locale, "de", 2) == 0)
		return "fr";
	else if (strncmp(locale, "it", 2) == 0)
		return "it";
	else if (strncmp(locale, "es", 2) == 0)
		return "es";
	else if (strncmp(locale, "el", 2) == 0)
		return "el";
	else if (strncmp(locale, "ru", 2) == 0)
		return "ru";
	else if (strncmp(locale, "zh_CN", 5) == 0)
		return "zh";
	else if (strncmp(locale, "zh_TW", 5) == 0)
		return "tw";
	else if (strncmp(locale, "ja", 2) == 0)
		return "ja";

	return "other";
}

/*
 * Sound
 */

/* サウンドを再生を開始する */
bool play_sound(int stream, struct wave *w)
{
	UNUSED_PARAMETER(stream);
	UNUSED_PARAMETER(w);
	return true;
}

/* サウンドの再生を停止する */
bool stop_sound(int stream)
{
	UNUSED_PARAMETER(stream);
	return true;
}

/* サウンドのボリュームを設定する */
bool set_sound_volume(int stream, float vol)
{
	UNUSED_PARAMETER(stream);
	UNUSED_PARAMETER(vol);
	return true;
}

/* サウンドが再生終了したか調べる */
bool is_sound_finished(int stream)
{
	UNUSED_PARAMETER(stream);
	return true;
}
