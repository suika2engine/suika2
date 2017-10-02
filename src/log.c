/* -*- coding: utf-8-with-signature; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Suika 2
 * Copyright (C) 2001-2017, TABATA Keiichi. All rights reserved.
 */

/*
 * [Changes]
 *  - 2016/05/27 作成
 *  - 2017/08/13 スイッチに対応
 */

#include <stddef.h>
#include <assert.h>

#include "log.h"
#include "platform.h"
#include "script.h"

/*
 * APIのエラーを記録する
 */
void log_api_error(const char *api)
{
	log_error("API %s が失敗しました。\n", api);
}

/*
 * オーディオファイルのエラーを記録する
 */
void log_audio_file_error(const char *dir, const char *file)
{
	log_error("オーディオファイル%s/%sを読み込めません。\n", dir,
		  conv_utf8_to_native(file));
}

/*
 * キャラの位置指定が間違っていることを記録する
 */
void log_ch_position(const char *pos)
{
	log_error("\"%s\"はキャラクタの位置指定として間違っています。\n",
		  conv_utf8_to_native(pos));
}

/*
 * ファイル名に英数字以外が含まれるエラーを記録する
 */
void log_file_name(const char *dir, const char *file)
{
	log_error("ファイル名に半角英数字以外の文字が使われています。"
		  "(%s/%s)\n", dir, conv_utf8_to_native(file));
}

/*
 * ファイルオープンエラーを記録する
 */
void log_dir_file_open(const char *dir, const char *file)
{
	log_error("ファイル\"%s/%s\"を開けません。\n", dir,
		  conv_utf8_to_native(file));
}

/*
 * ファイルオープンエラーを記録する
 */
void log_file_open(const char *fname)
{
	log_error("ファイル\"%s\"を開けません。\n",
		  conv_utf8_to_native(fname));
}

/*
 * フォントファイルのエラーを記録する
 */
void log_font_file_error(const char *font)
{
	log_error("フォントファイル%sを読み込めません。\n",
		  conv_utf8_to_native(font));
}

/*
 * イメージファイルのエラーを記録する
 */
void log_image_file_error(const char *dir, const char *file)
{
	log_error("イメージファイル%s/%sを読み込めません。\n", dir,
		  conv_utf8_to_native(file));
}

/*
 * メモリ確保エラーを記録する
 */
void log_memory(void)
{
	log_error("メモリの確保に失敗しました。\n");
}

/*
 * パッケージファイルのエラーを記録する
 */
void log_package_file_error(void)
{
	log_error("パッケージファイルの読み込みに失敗しました。\n");
}

/*
 * コマンド名がみつからないエラーを記録する
 */
void log_script_command_not_found(const char *name)
{
	log_error("コマンド\"%s\"がみつかりません\n",
		  conv_utf8_to_native(name));
}

/*
 * セリフが空白であるエラーを記録する
 */
void log_script_empty_serif(void)
{
	log_error("セリフが空白です\n");
}

/*
 * スクリプト実行エラーの位置を記録する
 */
void log_script_exec_footer(void)
{
	const char *file;
	int line;

	file = get_script_file_name();
	assert(file != NULL);

	line = get_line_num() + 1;

	log_error("> スクリプト実行エラー: %s %d行目\n", file, line);
	log_error("> %s\n", conv_utf8_to_native(get_line_string()));
}

/*
 * ラベルがみつからないエラーを記録する
 */
void log_script_label_not_found(const char *name)
{
	log_error("ラベル%sがみつかりません。\n", conv_utf8_to_native(name));
}

/*
 * 左辺値が変数でないエラーを記録する
 */
void log_script_lhs_not_variable(const char *lhs)
{
	log_error("左辺(%s)が変数名ではありません。\n",
		  conv_utf8_to_native(lhs));
}

/*
 * スクリプトにコマンドが含まれないエラーを記録する
 */
void log_script_no_command(const char *file)
{
	log_error("スクリプト%sにコマンドが含まれません。\n",
		  conv_utf8_to_native(file));
}

/*
 * 左辺値が変数でないエラーを記録する
 */
void log_script_not_variable(const char *name)
{
	log_error("変数名ではない名前(%s)が指定されました。\n",
		  conv_utf8_to_native(name));
}

/*
 * サイズに正の値が指定されなかったエラーを記録する
 */
void log_script_non_positive_size(int val)
{
	log_error("サイズに正の値が指定されませんでした。(%d)\n", val);
}

/*
 * スクリプトのパラメータが足りないエラーを記録する
 */
void log_script_too_few_param(int min, int real)
{
	log_error("引数が足りません。最低%d個必要ですが、"
		  "%d個しか指定されませんでした。\n", min, real);
}

/* スクリプトのパラメータが多すぎるエラーを記録する */
void log_script_too_many_param(int max, int real)
{
	log_error("引数が多すぎます。最大%d個ですが、"
		  "%d個指定されました。\n", max, real);
}

/* スクリプトの演算子が間違っているエラーを記録する */
void log_script_op_error(const char *op)
{
	log_error("演算子%sは間違っています。\n", conv_utf8_to_native(op));
}

/* スクリプトに文字列が指定されていないエラーを記録する */
void log_script_param_string(int param)
{
	log_error("パラメータ%dに文字列が指定されていません。\n", param);
}

/* スクリプトパースエラーの位置を記録する */
void log_script_parse_footer(const char *file, int line, const char *buf)
{
	log_error("> スクリプト書式エラー: %s %d行目\n", file, line);
	log_error("> %s\n", conv_utf8_to_native(buf));
}

/* RGB値が負であるエラーを記録する */
void log_script_rgb_negative(int val)
{
	log_error("RGB値が負の数(%d)が指定されました。\n", val);
}

/* スクリプトが長すぎるエラーを記録する */
void log_script_size(int size)
{
	log_error("スクリプト%sが最大コマンド数%dを超えています。"
		  "分割してください。\n", get_script_file_name(), size);
}

/* スイッチの選択肢がないエラーを記録する */
void log_script_switch_no_item(void)
{
	log_error("スイッチの選択肢がありません。");
}

/* スクリプトの変数インデックスが範囲外であるエラーを記録する */
void log_script_var_index(int index)
{
	log_error("変数インデックス%dは範囲外です。\n", index);
}

/* 未定義のコンフィグを記録する */
void log_undefined_conf(const char *key)
{
	log_error("コンフィグに%sが記述されていません。\n", key);
}

/* 不明なコンフィグを記録する */
void log_unknown_conf(const char *key)
{
	log_error("コンフィグの%sは認識されません。\n",
		  conv_utf8_to_native(key));
}

/* 音声ファイルの入力エラーを記録する */
void log_wave_error(const char *fname)
{
	assert(fname != NULL);
	log_error("ファイル%sの再生に失敗しました。\n",
		  conv_utf8_to_native(fname));
}
