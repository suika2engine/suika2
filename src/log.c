/* -*- coding: utf-8-with-signature; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Suika 2
 * Copyright (C) 2001-2021, TABATA Keiichi. All rights reserved.
 */

/*
 * [Changes]
 *  - 2016/05/27 作成
 *  - 2017/08/13 スイッチに対応
 *  - 2018/08/28 Add English messages.
 *  - 2021/06/05 @bg, @chのエフェクト名エラーを追加
 *  - 2021/06/10 @chaの加速タイプ名エラーを追加
 *  - 2021/06/12 @shakeの移動タイプ名エラーを追加
 *  - 2021/06/15 @setsaveのパラメタのエラーを追加
 *  - 2021/07/07 @goto $SAVEのエラーを追加
 */

#include <stddef.h>
#include <assert.h>

#include "suika.h"

static bool is_english_mode(void);

/* 英語モードであるかチェックする */
static bool is_english_mode(void)
{
	/* コンフィグlanguageが未指定の場合は日本語モードである */
	if (conf_language == NULL)
		return false;

	/* コンフィグlanguageがEnglishの場合は英語モードである */
	if (strcmp(conf_language, "English") == 0)
		return true;

	/* その他の言語が指定された場合は英語モードとする */
	return true;
}

/*
 * APIのエラーを記録する
 */
void log_api_error(const char *api)
{
	if (is_english_mode())
		log_error("API %s failed.\n", api);
	else
		log_error("API %s が失敗しました。\n", api);
}

/*
 * オーディオファイルのエラーを記録する
 */
void log_audio_file_error(const char *dir, const char *file)
{
	if (is_english_mode()) {
		log_error("Failed to load audio file \"%s/%s\".\n", dir,
			  conv_utf8_to_native(file));
	} else {
		log_error("オーディオファイル\"%s/%s\"を読み込めません。\n",
			  dir, conv_utf8_to_native(file));
	}
}

/*
 * ファイル名に英数字以外が含まれるエラーを記録する
 */
void log_file_name(const char *dir, const char *file)
{
	if (is_english_mode()) {
		log_error("File name includes non-ASCII character(s). "
			  "\"%s/%s\"\n", dir, conv_utf8_to_native(file));
	} else {
		log_error("ファイル名に半角英数字以外の文字が使われています。 "
			  "\"%s/%s\"\n", dir, conv_utf8_to_native(file));
	}
}

/*
 * ファイルオープンエラーを記録する
 */
void log_dir_file_open(const char *dir, const char *file)
{
	if (is_english_mode()) {
		log_error("Cannot open file \"%s/%s\".\n", dir,
			  conv_utf8_to_native(file));
	} else {
		log_error("ファイル\"%s/%s\"を開けません。\n", dir,
			  conv_utf8_to_native(file));
	}
}

/*
 * ファイルオープンエラーを記録する
 */
void log_file_open(const char *fname)
{
	if (is_english_mode()) {
		log_error("Cannot open file \"%s\".\n",
			  conv_utf8_to_native(fname));
	} else {
		log_error("ファイル\"%s\"を開けません。\n",
			  conv_utf8_to_native(fname));
	}
}

/*
 * フォントファイルのエラーを記録する
 */
void log_font_file_error(const char *font)
{
	if (is_english_mode()) {
		log_error("Failed to load font file \"%s\".\n",
		  conv_utf8_to_native(font));
	} else {
		log_error("フォントファイル\"%s\"を読み込めません。\n",
			  conv_utf8_to_native(font));
	}
}

/*
 * イメージファイルのエラーを記録する
 */
void log_image_file_error(const char *dir, const char *file)
{
	if (is_english_mode()) {
		log_error("Failed to load image file \"%s/%s\".\n", dir,
			  conv_utf8_to_native(file));
	} else {
		log_error("イメージファイル\"%s/%s\"を読み込めません。\n", dir,
			  conv_utf8_to_native(file));
	}
}

/*
 * メモリ確保エラーを記録する
 */
void log_memory(void)
{
	if (is_english_mode())
		log_error("Out of memory.\n");
	else
		log_error("メモリの確保に失敗しました。\n");
}

/*
 * パッケージファイルのエラーを記録する
 */
void log_package_file_error(void)
{
	if (is_english_mode())
		log_error("Failed to load the package file.\n");
	else
		log_error("パッケージファイルの読み込みに失敗しました。\n");
}

/*
 * 重複したコンフィグを記録する
 */
void log_duplicated_conf(const char *key)
{
	if (is_english_mode())
		log_error("Config key \"%s\" already exists.\n", key);
	else
		log_error("コンフィグで\"%s\"が重複しています。\n", key);
}

/*
 * 未定義のコンフィグを記録する
 */
void log_undefined_conf(const char *key)
{
	if (is_english_mode())
		log_error("Missing key \"%s\" in config.txt\n", key);
	else
		log_error("コンフィグに\"%s\"が記述されていません。\n", key);
}

/*
 * 不明なコンフィグを記録する
 */
void log_unknown_conf(const char *key)
{
	if (is_english_mode()) {
		log_error("Configuration key \"%s\" is not recognized.\n",
			  conv_utf8_to_native(key));
	} else {
		log_error("コンフィグの\"%s\"は認識されません。\n",
			  conv_utf8_to_native(key));
	}
}

/*
 * 音声ファイルの入力エラーを記録する
 */
void log_wave_error(const char *fname)
{
	assert(fname != NULL);

	if (is_english_mode()) {
		log_error("Failed to play \"%s\".\n",
			  conv_utf8_to_native(fname));
	} else {
		log_error("ファイル\"%s\"の再生に失敗しました。\n",
			  conv_utf8_to_native(fname));
	}
}

/*
 * メッセージボックスの前景と背景が異なるサイズであるエラーを記録する
 */
void log_invalid_msgbox_size(void)
{
	if (is_english_mode()) {
		log_error("The sizes of message box bg and fg differ.\n");
	} else {
		log_error("メッセージボックスのBGとFGで"
			  "サイズが異なります。.\n");
	}
}

/*
 * スクリプト実行エラーの位置を記録する
 */
void log_script_exec_footer(void)
{
#ifndef USE_DEBUGGER
	const char *file;
	int line;

	file = get_script_file_name();
	assert(file != NULL);

	line = get_line_num() + 1;

	if (is_english_mode()) {
		log_error("> Script execution error: %s:%d\n", file, line);
		log_error("> %s\n", conv_utf8_to_native(get_line_string()));
	} else {
		log_error("> スクリプト実行エラー: %s %d行目\n", file, line);
		log_error("> %s\n", conv_utf8_to_native(get_line_string()));
	}
#else
	char *line;

	/* コマンドをメッセージに変換する */
	line = strdup(get_line_string());
	if (line == NULL)
		log_memory();
	else
		set_error_command(get_command_index(), line);
	dbg_set_error_state();
#endif
}

/*
 * コマンド名がみつからないエラーを記録する
 */
void log_script_command_not_found(const char *name)
{
	if (is_english_mode()) {
		log_error("Invalid command \"%s\".\n",
			  conv_utf8_to_native(name));
	} else {
		log_error("コマンド\"%s\"がみつかりません\n",
			  conv_utf8_to_native(name));
	}
}

/*
 * セリフが空白であるエラーを記録する
 */
void log_script_empty_serif(void)
{
	if (is_english_mode())
		log_error("Character message or name is empty.\n");
	else
		log_error("セリフか名前が空白です\n");
}

/*
 * キャラの位置指定が間違っているエラーを記録する
 */
void log_script_ch_position(const char *pos)
{
	if (is_english_mode()) {
		log_error("Character position \"%s\" is invalid.\n",
			  conv_utf8_to_native(pos));
	} else {
		log_error("キャラクタの位置指定\"%s\"は間違っています。\n",
		  conv_utf8_to_native(pos));
	}
}

/*
 * フェードの方法指定が間違っているエラーを記録する
 */
void log_script_fade_method(const char *method)
{
	if (is_english_mode()) {
		log_error("Fade method \"%s\" is invalid.\n",
			  conv_utf8_to_native(method));
	} else {
		log_error("フェードの方法指定\"%s\"は間違っています。\n",
		  conv_utf8_to_native(method));
	}
}

/*
 * ラベルがみつからないエラーを記録する
 */
void log_script_label_not_found(const char *name)
{
	if (is_english_mode()) {
		log_error("Label \"%s\" not found.\n",
			  conv_utf8_to_native(name));
	} else {
		log_error("ラベル\"%s\"がみつかりません。\n",
			  conv_utf8_to_native(name));
	}
}

/*
 * 左辺値が変数でないエラーを記録する
 */
void log_script_lhs_not_variable(const char *lhs)
{
	if (is_english_mode()) {
		log_error("Invalid variable name on LHS. (%s).\n",
			  conv_utf8_to_native(lhs));
	} else {
		log_error("左辺(%s)が変数名ではありません。\n",
			  conv_utf8_to_native(lhs));
	}
}

/*
 * スクリプトにコマンドが含まれないエラーを記録する
 */
void log_script_no_command(const char *file)
{
	if (is_english_mode()) {
		log_error("Script \"%s\" is empty.\n",
			  conv_utf8_to_native(file));
	} else {
		log_error("スクリプト%sにコマンドが含まれません。\n",
			  conv_utf8_to_native(file));
	}
}

/*
 * 左辺値が変数でないエラーを記録する
 */
void log_script_not_variable(const char *name)
{
	if (is_english_mode()) {
		log_error("Invalid variable name. (%s)\n",
			  conv_utf8_to_native(name));
	} else {
		log_error("変数名ではない名前(%s)が指定されました。\n",
			  conv_utf8_to_native(name));
	}
}

/*
 * サイズに正の値が指定されなかったエラーを記録する
 */
void log_script_non_positive_size(int val)
{
	if (is_english_mode())
		log_error("Negative size value. (%d)\n", val);
	else
		log_error("サイズに正の値が指定されませんでした。(%d)\n", val);
}

/*
 * スクリプトのパラメータが足りないエラーを記録する
 */
void log_script_too_few_param(int min, int real)
{
	if (is_english_mode()) {
		log_error("Too few argument(s). "
			  "At least %d argument(s) required, "
			  "but %d argument(s) passed.\n", min, real);
	} else {
		log_error("引数が足りません。最低%d個必要ですが、"
			  "%d個しか指定されませんでした。\n", min, real);
	}
}

/*
 * スクリプトのパラメータが多すぎるエラーを記録する
 */
void log_script_too_many_param(int max, int real)
{
	if (is_english_mode()) {
		log_error("Too many argument(s). "
			  "Number of maximum argument(s) is %d, "
			  "but %d argument(s) passed.\n", max, real);
	} else {
		log_error("引数が多すぎます。最大%d個ですが、"
			  "%d個指定されました。\n", max, real);
	}
}

/*
 * スクリプトの演算子が間違っているエラーを記録する
 */
void log_script_op_error(const char *op)
{
	if (is_english_mode()) {
		log_error("Invalid operator \"%s\".\n",
			  conv_utf8_to_native(op));
	} else {
		log_error("演算子\"%s\"は間違っています。\n",
			  conv_utf8_to_native(op));
	}
}

/*
 * スクリプトパースエラーの位置を記録する
 */
void log_script_parse_footer(const char *file, int line, const char *buf)
{
	char tmp[4096];

	line++;
	if (is_english_mode()) {
		snprintf(tmp, sizeof(tmp),
			 "> Script format error: %s:%d\n"
			 "> %s\n",
			 file, line, conv_utf8_to_native(buf));
		log_error(tmp);
	} else {
		snprintf(tmp, sizeof(tmp),
			 "> スクリプト書式エラー: %s %d行目\n"
			 "> %s\n",
			 file, line, conv_utf8_to_native(buf));
		log_error(tmp);
	}
}

/*
 * returnの戻り先が存在しない行であるエラーを記録する
 */
void log_script_return_error(void)
{
	if (is_english_mode())
		log_error("No return target of @return.\n");
	else
		log_error("@returnの戻り先が存在しません。\n");
}

/*
 * RGB値が負であるエラーを記録する
 */
void log_script_rgb_negative(int val)
{
	if (is_english_mode()) {
		log_error("Negative value specified as a "
			  "RGB component value. (%d)\n", val);
	} else {
		log_error("RGB値に負の数(%d)が指定されました。\n", val);
	}
}

/*
 * スクリプトが長すぎるエラーを記録する
 */
void log_script_size(int size)
{
	if (is_english_mode()) {
		log_error("Script \"%s\" exceeds the limit of lines %d. "
			  "Please split the script.\n",
			  get_script_file_name(), size);
	} else {
		log_error("スクリプト%sが最大コマンド数%dを超えています。"
			  "分割してください。\n", get_script_file_name(),
			  size);
	}
}

/*
 * スイッチの選択肢にラベルがないエラーを記録する
 * Record error that switch option has no label
 */
void log_script_switch_no_label(void)
{
	if (is_english_mode())
		log_error("No label for @switch option.");
	else
		log_error("スイッチの選択肢にラベルがありません。");
}

/*
 * スイッチの選択肢がないエラーを記録する
 */
void log_script_switch_no_item(void)
{
	if (is_english_mode())
		log_error("No option for @switch.");
	else
		log_error("スイッチの選択肢がありません。");
}

/*
 * スクリプトの変数インデックスが範囲外であるエラーを記録する
 */
void log_script_var_index(int index)
{
	if (is_english_mode())
		log_error("Variable index %d is out of range.\n", index);
	else
		log_error("変数インデックス%dは範囲外です。\n", index);
}

/*
 * ミキサーのストリームの指定が間違っているエラーを記録する
 */
void log_script_mixer_stream(const char *stream)
{
	if (is_english_mode()) {
		log_error("Invalid mixer stream name \"%s\".\n",
			  conv_utf8_to_native(stream));
	} else {
		log_error("ミキサーのストリーム名\"%s\"は正しくありません。\n",
			  conv_utf8_to_native(stream));
	}
}

/*
 * キャラアニメの加速タイプ名が間違っているエラーを記録する
 */
void log_script_cha_accel(const char *accel)
{
	if (is_english_mode()) {
		log_error("Invalid movement type \"%s\".\n",
			  conv_utf8_to_native(accel));
	} else {
		log_error("移動タイプ\"%s\"は正しくありません。\n",
			  conv_utf8_to_native(accel));
	}
}

/*
 * 画面揺らしエフェクトの移動タイプ名が間違っているエラーを記録する
 */
void log_script_shake_move(const char *move)
{
	if (is_english_mode()) {
		log_error("Invalid movement type \"%s\".\n",
			  conv_utf8_to_native(move));
	} else {
		log_error("移動タイプ\"%s\"は正しくありません。\n",
			  conv_utf8_to_native(move));
	}
}

/*
 * enableかdisableの引数に違う値が与えられたエラーを記録する
 */
void log_script_enable_disable(const char *param)
{
	if (is_english_mode()) {
		log_error("Invalid parameter \"%s\". "
			  "Specify enable or disable.\n",
			  conv_utf8_to_native(param));
	} else {
		log_error("引数\"%s\"は正しくありません。"
			  "enableかdisableを指定してください。\n",
			  conv_utf8_to_native(param));
	}
}

/*
 * @goto $SAVEがスクリプトの最後のコマンドとして実行されたエラーを記録する
 */
void log_script_final_command(void)
{
	if (is_english_mode()) {
		log_error("You can't put this command on "
			  "the end of the script.\n");
	} else {
		log_error("このコマンドはスクリプトの末尾に置けません。\n");
	}
}

/*
 * ビデオ再生に失敗した際のエラーを記録する
 */
void log_video_error(const char *reason)
{
	if (is_english_mode()) {
		log_error("Video playback error: \"%s\"",
			  conv_utf8_to_native(reason));
	} else {
		log_error("ビデオ再生エラー: \"%s\"",
			  conv_utf8_to_native(reason));
	}
}

#ifdef USE_DEBUGGER
/*
 * コマンドのアップデートに失敗した際のエラーを記録する
 */
void log_command_update_error(void)
{
	if (is_english_mode()) {
		log_info("Incorrect commands are changed to messages which start with '!'\n"
			 "You can search it from menu.\n"
			 "Edit and update it to reconvert to a correct command.");
	} else {
		log_info("エラーを含むコマンドは'!'で始まるメッセージに変換されます。\n"
			 "メニューから検索できます。\n"
			 "編集と更新で正しいコマンドに戻すことが可能です。");
	}
}
#endif
