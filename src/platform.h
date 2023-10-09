/* -*- coding: utf-8; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Suika 2
 * Copyright (C) 2001-2016, TABATA Keiichi. All rights reserved.
 */

/* 
 * HAL API (Hardware Abstraction layer)
 */

#ifndef SUIKA_PLATFORM_H
#define SUIKA_PLATFORM_H

#include "types.h"

/* イメージ */
struct image;

/* PCMストリーム */
struct wave;

/*
 * ログを出力する
 *  - ログを出力してよいのはメインスレッドのみとする
 *  - ネイティブの文字コードを渡すこととする
 */
bool log_info(const char *s, ...);
bool log_warn(const char *s, ...);
bool log_error(const char *s, ...);

/* セーブディレクトリを作成する */
bool make_sav_dir(void);

/* データのディレクトリ名とファイル名を指定して有効なパスを取得する */
char *make_valid_path(const char *dir, const char *fname);

/* GPUを使うか調べる */
bool is_gpu_accelerated(void);

/* OpenGLが有効か調べる */
bool is_opengl_enabled(void);

/* テクスチャをロックする */
bool lock_texture(int width, int height, pixel_t *pixels,
		  pixel_t **locked_pixels, void **texture);

/* テクスチャをアンロックする */
void unlock_texture(int width, int height, pixel_t *pixels,
		    pixel_t **locked_pixels, void **texture);

/* テクスチャを破棄する */
void destroy_texture(void *texture);

/* 画面にイメージをレンダリングする */
void render_image(int dst_left, int dst_top, struct image * RESTRICT src_image,
		  int width, int height, int src_left, int src_top, int alpha,
		  int bt);

/* 画面にイメージを暗くレンダリングする */
void render_image_dim(int dst_left, int dst_top,
		      struct image * RESTRICT src_image, int width, int height,
		      int src_left, int src_top);

/* 画面にイメージをルール付きでレンダリングする */
void render_image_rule(struct image * RESTRICT src_img,
		       struct image * RESTRICT rule_img,
		       int threshold);

/* 画面にイメージをルール付き(メルト)でレンダリングする */
void render_image_melt(struct image * RESTRICT src_img,
		       struct image * RESTRICT rule_img,
		       int threshold);

/* タイマをリセットする */
void reset_stop_watch(stop_watch_t *t);

/* タイマのラップをミリ秒単位で取得する */
int get_stop_watch_lap(stop_watch_t *t);

/* サウンドを再生を開始する */
bool play_sound(int stream, struct wave *w);

/* サウンドの再生を停止する */
bool stop_sound(int stream);

/* サウンドのボリュームを設定する */
bool set_sound_volume(int stream, float vol);

/* サウンドが再生終了したか調べる */
bool is_sound_finished(int stream);

/* 終了ダイアログを表示する */
bool exit_dialog(void);

/* タイトルに戻るダイアログを表示する */
bool title_dialog(void);

/* 削除ダイアログを表示する */
bool delete_dialog(void);

/* 上書きダイアログを表示する */
bool overwrite_dialog(void);

/* 初期設定ダイアログを表示する */
bool default_dialog(void);

/* ビデオを再生する */
bool play_video(const char *fname, bool is_skippable);

/* ビデオを停止する */
void stop_video(void);

/* ビデオが再生中か調べる */
bool is_video_playing(void);

/* ウィンドウタイトルを更新する */
void update_window_title(void);

/* フルスクリーンモードがサポートされるか調べる */
bool is_full_screen_supported(void);

/* フルスクリーンモードであるか調べる */
bool is_full_screen_mode(void);

/* フルスクリーンモードを開始する */
void enter_full_screen_mode(void);

/* フルスクリーンモードを終了する */
void leave_full_screen_mode(void);

/* システムのロケールを取得する */
const char *get_system_locale(void);

/*
 * デバッガの場合のみ
 */
#ifdef USE_DEBUGGER
/* 再開ボタンが押されたか調べる */
bool is_resume_pushed(void);

/* 次へボタンが押されたか調べる */
bool is_next_pushed(void);

/* 停止ボタンが押されたか調べる */
bool is_pause_pushed(void);

/* 実行するスクリプトファイルが変更されたか調べる */
bool is_script_changed(void);

/* 変更された実行するスクリプトファイル名を取得する */
const char *get_changed_script(void);

/* 実行する行番号が変更されたか調べる */
bool is_line_changed(void);

/* 変更された実行するスクリプトファイル名を取得する */
int get_changed_line(void);

/* コマンドがアップデートされたかを調べる */
bool is_command_updated(void);

/* アップデートされたコマンド文字列を取得する */
const char *get_updated_command(void);

/* スクリプトがリロードされたかを調べる */
bool is_script_reloaded(void);

/* コマンドの実行中状態を設定する */
void set_running_state(bool running, bool request_stop);

/* デバッグ情報を更新する */
void update_debug_info(bool script_changed);
#endif

#ifdef USE_CAPTURE
/* キャプチャモジュールを初期化する */
bool init_capture(void);

/* キャプチャモジュールを終了する */
void cleanup_capture(void);

/* 入力をキャプチャする */
bool capture_input(void);

/* 出力をキャプチャする */
bool capture_output(void);
#endif

#ifdef USE_REPLAY
/* リプレイモジュールを初期化する */
#ifdef WIN
bool init_replay(int argc, wchar_t *argv[]);
#else
bool init_replay(int argc, char *argv[]);
#endif

/* リプレイモジュールを終了する */
void cleanup_replay(void);

/* 入力をリプレイする */
bool capture_input(void);

/* リプレイ結果の出力をキャプチャする */
bool capture_output(void);
#endif

#if defined(USE_CAPTURE) || defined(USE_REPLAY)

#ifdef MAC
FILE *open_file(const char *dir, const char *file, const char *mode)
#endif

/* ミリ秒の時刻を取得する */
uint64_t get_tick_count64(void);

/* 出力データのディレクトリを作り直す */
bool reconstruct_dir(const char *dir);
#endif

/*
 * Text-To-Speech
 */
#if defined(WIN)
/* TTSによる読み上げを行う */
void speak_text(const char *text);
#endif

#endif
