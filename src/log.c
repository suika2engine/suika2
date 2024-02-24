/* -*- coding: utf-8; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Suika2
 * Copyright (C) 2001-2023, Keiichi Tabata. All rights reserved.
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
 *  - 2022/06/14 Suika2 Pro for Creators
 *  - 2022/11/06 UTF-8
 *  - 2023/01/06 パラメータ名のエラーを追加
 *
 * [TODO]
 *  - uimsgを利用する
 */

#include "suika.h"

/* Forward declaration */
static bool is_english_mode(void);

/* 英語モードであるかチェックする */
static bool is_english_mode(void)
{
	/* TODO: uimsgを利用して多言語対応する */

	/* FIXME: 日本語ロケールでなければ英語メッセージする */
	if (strcmp(get_system_locale(), "ja") == 0)
		return false;
	else
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
		log_error(U8("API %s が失敗しました。\n"), api);
}

/*
 * オーディオファイルのエラーを記録する
 */
void log_audio_file_error(const char *dir, const char *file)
{
	if (is_english_mode()) {
		log_error("Failed to load audio file \"%s/%s\".\n", dir, file);
	} else {
		log_error(U8("オーディオファイル\"%s/%s\"を読み込めません。\n"),
			  dir, file);
	}
}

/*
 * ファイル名に英数字以外が含まれるエラーを記録する
 */
void log_file_name(const char *dir, const char *file)
{
	if (is_english_mode()) {
		log_error("File name includes non-ASCII character(s). "
			  "\"%s/%s\"\n", dir, file);
	} else {
		log_error(U8("ファイル名に半角英数字以外の文字が使われています。 ")
			  U8("\"%s/%s\"\n"), dir, file);
	}
}

/*
 * ファイルオープンエラーを記録する
 */
void log_dir_file_open(const char *dir, const char *file)
{
	if (is_english_mode())
		log_error("Cannot open file \"%s/%s\".\n", dir, file);
	else
		log_error(U8("ファイル\"%s/%s\"を開けません。\n"), dir, file);
}

/*
 * ファイルオープンエラーを記録する
 */
void log_file_open(const char *fname)
{
	if (is_english_mode())
		log_error("Cannot open file \"%s\".\n", fname);
	else
		log_error(U8("ファイル\"%s\"を開けません。\n"), fname);
}

/*
 * ファイル読み込みエラーを記録する
 */
void log_file_read(const char *dir, const char *file)
{
	if (is_english_mode())
		log_error("Cannot read file \"%s/%s\".\n", dir, file);
	else
		log_error(U8("ファイル\"%s/%s\"を読み込めません。\n"), dir, file);
}

/*
 * フォントファイルのエラーを記録する
 */
void log_font_file_error(const char *font)
{
	if (is_english_mode())
		log_error("Failed to load font file \"%s\".\n", font);
	else
		log_error(U8("フォントファイル\"%s\"を読み込めません。\n"), font);
}

/*
 * イメージファイルのエラーを記録する
 */
void log_image_file_error(const char *dir, const char *file)
{
	if (is_english_mode()) {
		log_error("Failed to load image file \"%s/%s\".\n", dir, file);
	} else {
		log_error(U8("イメージファイル\"%s/%s\"を読み込めません。\n"),
			  dir, file);
	}
}

/*
 * メモリ確保エラーを記録する
 */
void log_memory_helper(const char *file, int line)
{
	if (is_english_mode())
		log_error("Out of memory. (%s:%d)\n", file, line);
	else
		log_error(U8("メモリの確保に失敗しました。(%s:%d)\n"), file, line);

	abort();
}

/*
 * パッケージファイルのエラーを記録する
 */
void log_package_file_error(void)
{
	if (is_english_mode())
		log_error("Failed to load the package file.\n");
	else
		log_error(U8("パッケージファイルの読み込みに失敗しました。\n"));
}

/*
 * 重複したコンフィグを記録する
 */
void log_duplicated_conf(const char *key)
{
	if (is_english_mode())
		log_error("Config key \"%s\" already exists.\n", key);
	else
		log_error(U8("コンフィグで\"%s\"が重複しています。\n"), key);
}

/*
 * 未定義のコンフィグを記録する
 */
void log_undefined_conf(const char *key)
{
	if (is_english_mode()) {
#ifndef SUIKA_TARGET_WASM
		log_error("Missing key \"%s\" in config.txt\n", key);
#else
		log_error("Missing key \"%s\" in config.txt\n"
			  "You are probably uploaded an older version of the index files.\n"
			  "If not, it's a problem of browser caches.\n"
			  "Please clear the entire history of your browser.",
			  key);
#endif
	} else {
#ifndef SUIKA_TARGET_WASM
		log_error(U8("コンフィグに\"%s\"が記述されていません。\n"), key);
#else
		log_error(U8("コンフィグに\"%s\"が記述されていません。\n")
			  U8("古いバージョンのindexファイルをアップロードした可能性があります。\n")
			  U8("そうでない場合はブラウザのキャッシュの問題です。\n")
			  U8("ブラウザの履歴を完全に消去する必要があります。"),
			  key);
#endif
	}
}

/*
 * 不明なコンフィグを記録する
 */
void log_unknown_conf(const char *key)
{
	if (is_english_mode()) {
		log_error("Configuration key \"%s\" is not recognized.\n",
			  key);
	} else {
		log_error(U8("コンフィグの\"%s\"は認識されません。\n"), key);
	}
}

/*
 * 空のコンフィグ文字列を記録する
 */
void log_empty_conf_string(const char *key)
{
	if (is_english_mode()) {
		log_error("Empty string is specified for"
			  " config key \"%s\"\n", key);
	} else {
		log_error(U8("コンフィグの\"%s\"に空の文字列が指定されました。\n"),
			  key);
	}
}

/*
 * 音声ファイルの入力エラーを記録する
 */
void log_wave_error(const char *fname)
{
	assert(fname != NULL);

	if (is_english_mode())
		log_error("Failed to play \"%s\".\n", fname);
	else
		log_error(U8("ファイル\"%s\"の再生に失敗しました。\n"), fname);
}

/*
 * メッセージボックスの前景と背景が異なるサイズであるエラーを記録する
 */
void log_invalid_msgbox_size(void)
{
	if (is_english_mode()) {
		log_error("The sizes of message box bg and fg differ.\n");
	} else {
		log_error(U8("メッセージボックスのBGとFGでサイズが異なります。\n"));
	}
}

/*
 * セーブデータのバージョンが一致しないエラーを記録する
 */
void log_save_ver(void)
{
	if (is_english_mode()) {
		log_error("Ignoring save data: old save file format detected.\n");
	} else {
		log_error(U8("セーブデータを無視します:")
			  U8("古いバージョンのセーブデータを検出しました。\n"));
	}
}

/*
 * スクリプト実行エラーの位置を記録する
 */
void log_script_exec_footer(void)
{
#ifndef USE_EDITOR
	const char *file;
	int line;

	file = get_script_file_name();
	assert(file != NULL);

	line = get_line_num() + 1;

	if (is_english_mode()) {
		log_error("> Script execution error: %s:%d\n"
			  "> %s\n",
			  file, line, get_line_string());
	} else {
		log_error(U8("> スクリプト実行エラー: %s %d行目\n")
			  U8("> %s\n"),
			  file, line, get_line_string());
	}
#else
	/* '@'コマンドを'!'メッセージに変換する */
	translate_command_to_message_for_runtime_error(get_command_index());
#endif
}

/*
 * ２階層目のインクルードのエラーを記録する
 */
void log_script_deep_include(const char *inc_name)
{
	if (is_english_mode())
		log_error("Include within include files is not supported yet."
			  " \"%s\".\n", inc_name);
	else
		log_error(U8("インクルードファイル内でのインクルードは")
			  U8("まだサポートされていません。 \"%s\"\n"),
			  inc_name);
}

/*
 * コマンド名がみつからないエラーを記録する
 */
void log_script_command_not_found(const char *name)
{
	if (is_english_mode())
		log_error("Invalid command \"%s\".\n", name);
	else
		log_error(U8("コマンド\"%s\"がみつかりません\n"), name);
}

/*
 * セリフが空白であるエラーを記録する
 */
void log_script_empty_serif(void)
{
	if (is_english_mode())
		log_error("Character message or name is empty.\n");
	else
		log_error(U8("セリフか名前が空白です\n"));
}

/*
 * キャラの位置指定が間違っているエラーを記録する
 */
void log_script_ch_position(const char *pos)
{
	if (is_english_mode()) {
		log_error("Character position \"%s\" is invalid.\n", pos);
	} else {
		log_error(U8("キャラクタの位置指定\"%s\"は間違っています。\n"),
			  pos);
	}
}

/*
 * フェードの方法指定が間違っているエラーを記録する
 */
void log_script_fade_method(const char *method)
{
	if (is_english_mode()) {
		log_error("Fade method \"%s\" is invalid.\n", method);
	} else {
		log_error(U8("フェードの方法指定\"%s\"は間違っています。\n"),
			  method);
	}
}

/*
 * ラベルがみつからないエラーを記録する
 */
void log_script_label_not_found(const char *name)
{
	if (is_english_mode())
		log_error("Label \"%s\" not found.\n", name);
	else
		log_error(U8("ラベル\"%s\"がみつかりません。\n"), name);
}

/*
 * 左辺値が変数でないエラーを記録する
 */
void log_script_lhs_not_variable(const char *lhs)
{
	if (is_english_mode())
		log_error("Invalid variable name on LHS. (%s).\n", lhs);
	else
		log_error(U8("左辺(%s)が変数名ではありません。\n"), lhs);
}

/*
 * スクリプトにコマンドが含まれないエラーを記録する
 */
void log_script_no_command(const char *file)
{
	if (is_english_mode())
		log_error("Script \"%s\" is empty.\n", file);
	else
		log_error(U8("スクリプト%sにコマンドが含まれません。\n"), file);
}

/*
 * 左辺値が変数でないエラーを記録する
 */
void log_script_not_variable(const char *name)
{
	if (is_english_mode())
		log_error("Invalid variable name. (%s)\n", name);
	else
		log_error(U8("変数名ではない名前(%s)が指定されました。\n"), name);
}

/*
 * サイズに正の値が指定されなかったエラーを記録する
 */
void log_script_non_positive_size(int val)
{
	if (is_english_mode())
		log_error("Negative size value. (%d)\n", val);
	else
		log_error(U8("サイズに正の値が指定されませんでした。(%d)\n"), val);
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
		log_error(U8("引数が足りません。最低%d個必要ですが、")
			  U8("%d個しか指定されませんでした。\n"), min, real);
		if (strstr(get_line_string(), U8("　")) != NULL) {
			log_error(U8("行に全角スペースが含まれています。\n")
				  U8("半角にするべきか確認してください。\n"));
		}
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
		log_error(U8("引数が多すぎます。最大%d個ですが、")
			  U8("%d個指定されました。\n"), max, real);
	}
}

/*
 * スクリプトの演算子が間違っているエラーを記録する
 */
void log_script_op_error(const char *op)
{
	if (is_english_mode())
		log_error("Invalid operator \"%s\".\n", op);
	else
		log_error(U8("演算子\"%s\"は間違っています。\n"), op);
}

/*
 * スクリプトパースエラーの位置を記録する
 */
void log_script_parse_footer(const char *file, int line, const char *buf)
{
#ifdef USE_EDITOR
	if (dbg_get_parse_error_count() > 0)
		return;
#endif

	line++;
	if (is_english_mode()) {
		log_error("> Script format error: %s:%d\n"
			  "> %s\n",
			  file, line, buf);
	} else {
		log_error(U8("> スクリプト書式エラー: %s %d行目\n")
			  U8("> %s\n"),
			  file, line, buf);
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
		log_error(U8("@returnの戻り先が存在しません。\n"));
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
		log_error(U8("RGB値に負の数(%d)が指定されました。\n"), val);
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
		log_error(U8("スクリプト%sが最大コマンド数%dを超えています。")
			  U8("分割してください。\n"),
			  get_script_file_name(), size);
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
		log_error(U8("スイッチの選択肢にラベルがありません。"));
}

/*
 * スイッチの選択肢がないエラーを記録する
 */
void log_script_switch_no_item(void)
{
	if (is_english_mode())
		log_error("No option for @switch.");
	else
		log_error(U8("スイッチの選択肢がありません。"));
}

/*
 * スクリプトの変数インデックスが範囲外であるエラーを記録する
 */
void log_script_var_index(int index)
{
	if (is_english_mode())
		log_error("Variable index %d is out of range.\n", index);
	else
		log_error(U8("変数インデックス%dは範囲外です。\n"), index);
}

/*
 * ミキサーのボリューム指定が間違っているエラーを記録する
 */
void log_script_vol_value(float vol)
{
	if (is_english_mode()) {
		log_error("Invalid volume value \"%.1f\".\n"
			  "Specify between 0.0 and 1.0.",
			  vol);
	} else {
		log_error(U8("ボリューム値\"%0.1f\"は正しくありません。\n")
			  U8("0.0以上1.0以下で指定してください。"),
			  vol);
	}
}

/*
 * ミキサーのストリームの指定が間違っているエラーを記録する
 */
void log_script_mixer_stream(const char *stream)
{
	if (is_english_mode()) {
		log_error("Invalid mixer stream name \"%s\".\n", stream);
	} else {
		log_error(U8("ミキサーのストリーム名\"%s\"は正しくありません。\n"),
			  stream);
	}
}

/*
 * キャラアニメの加速タイプ名が間違っているエラーを記録する
 */
void log_script_cha_accel(const char *accel)
{
	if (is_english_mode())
		log_error("Invalid movement type \"%s\".\n", accel);
	else
		log_error(U8("移動タイプ\"%s\"は正しくありません。\n"), accel);
}

/*
 * 画面揺らしエフェクトの移動タイプ名が間違っているエラーを記録する
 */
void log_script_shake_move(const char *move)
{
	if (is_english_mode())
		log_error("Invalid movement type \"%s\".\n", move);
	else
		log_error(U8("移動タイプ\"%s\"は正しくありません。\n"), move);
}

/*
 * enableかdisableの引数に違う値が与えられたエラーを記録する
 */
void log_script_enable_disable(const char *param)
{
	if (is_english_mode()) {
		log_error("Invalid parameter \"%s\". "
			  "Specify enable or disable.\n", param);
	} else {
		log_error(U8("引数\"%s\"は正しくありません。")
			  U8("enableかdisableを指定してください。\n"), param);
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
		log_error(U8("このコマンドはスクリプトの末尾に置けません。\n"));
	}
}

/*
 * パラメータ名が一致しないエラーを記録する
 */
void log_script_param_mismatch(const char *name)
{
	if (is_english_mode()) {
		log_error("Can't use parameter name \"%s\" here.\n", name);
	} else {
		log_error(U8("パラメータ名\"%s\"はこの位置で使えません。\n"),
			  name);
	}
}

/*
 * usingするファイルが多すぎるエラーを記録する
 */
void log_script_too_many_files(void)
{
	if (is_english_mode())
		log_error("Too many \"using\" files.\n");
	else
		log_error(U8("\"using\"で使用するファイルが多すぎます。"));
}

/*
 * 書き換え可能なコンフィグキーがないエラーを記録する
 */
void log_script_config_not_found(const char *key)
{
	if (is_english_mode())
		log_error("Can't set config \"%s\" here.\n", key);
	else
		log_error(U8("コンフィグ\"%s\"を変更できません。\n"), key);
}

/*
 * chaで指定されたキャラクタ位置に画像がないエラーを記録する
 */
void log_script_cha_no_image(const char *pos)
{
	if (is_english_mode()) {
		log_error("Character image on position \"%s\" is not loaded\n",
			  pos);
	} else {
		log_error(U8("キャラクタ位置\"%s\"に画像がロードされていません。\n"),
			  pos);
	}
}

/*
 * 引数名が必須のコマンドで引数名が指定されなかったエラーを記録する
 */
void log_script_parameter_name_not_specified(void)
{
	if (is_english_mode()) {
		log_error("A paramter name not specified for "
			  "a command that requires parameter names.\n");
	} else {
		log_error(U8("引数名が必須のコマンドで引数名が")
			  U8("指定されていません。\n"));
	}
}

/*
 * 引数名の順番が間違っているエラーを記録する
 */
void log_script_param_order_mismatch(void)
{
	if (is_english_mode()) {
		log_error("Wrong paramter name order.\n");
	} else {
		log_error(U8("引数名の順番が異なります。\n"));
	}
}

/*
 * ビデオ再生に失敗した際のエラーを記録する
 */
void log_video_error(const char *reason)
{
	if (is_english_mode())
		log_error("Video playback error: \"%s\"", reason);
	else
		log_error(U8("ビデオ再生エラー: \"%s\""), reason);
}

/*
 * 選択肢コマンドの引数が足りないエラーを記録する
 */	
void log_script_choose_no_message(void)
{
	if (is_english_mode())
		log_info("Too few arguments.");
	else
		log_info(U8("選択肢の指定が足りません。"));
}

/*
 * コマンドの引数に空文字列""が指定されたエラーを記録する
 */
void log_script_empty_string(void)
{
	if (is_english_mode())
		log_info("Empty string \"\" is not allowed.");
	else
		log_info(U8("空文字列\"\"は利用できません。"));
}

/*
 * ファイルの書き込みに失敗した際のエラーを記録する
 */
void log_file_write(const char *file)
{
	if (is_english_mode())
		log_info("Cannot write to \'%s\'.", file);
	else
		log_info(U8("\'%s\'へ書き込みできません。"), file);
}

/*
 * ルールファイルが指定されていない際のエラーを記録する
 */
void log_script_rule(void)
{
	if (is_english_mode())
		log_info("Rule file not specified.");
	else
		log_info(U8("ルールファイルが指定されていません。"));
}

/*
 * GUIファイルにパースできない文字がある際のエラーを記録する
 */
void log_gui_parse_char(char c)
{
	if (is_english_mode())
		log_error("Invalid character \'%c\'", c);
	else
		log_error(U8("不正な文字 \'%c\'"), c);
}

/*
 * GUIファイルの記述が長すぎる際のエラーを記録する
 */
void log_gui_parse_long_word(void)
{
	if (is_english_mode())
		log_error("Too long word.");
	else
		log_error(U8("記述が長すぎます。"));
}

/*
 * GUIファイルで必要な記述が空白である際のエラーを記録する
 */
void log_gui_parse_empty_word(void)
{
	if (is_english_mode())
		log_error("Nothing is specified.");
	else
		log_error(U8("空白が指定されました。"));
}

/*
 * GUIファイルで不正なEOFが現れた際のエラーを記録する
 */
void log_gui_parse_invalid_eof(void)
{
	if (is_english_mode())
		log_error("Invalid End-of-File.");
	else
		log_error(U8("不正なファイル終端です。"));
}

/*
 * GUIファイルで未知のグローバルキーが現れた際のエラーを記録する
 */
void log_gui_unknown_global_key(const char *key)
{
	if (is_english_mode())
		log_error("Invalid gobal key \"%s\"", key);
	else
		log_error(U8("不正なグローバルキー \"%s\""), key);
}

/*
 * GUIファイルでボタンが多すぎる際のエラーを記録する
 */
void log_gui_too_many_buttons(void)
{
	if (is_english_mode())
		log_error("Too many buttons.");
	else
		log_error(U8("ボタンが多すぎます。"));
}

/*
 * GUIファイルで未知のボタンタイプが指定された際のエラーを記録する
 */
void log_gui_unknown_button_type(const char *type)
{
	if (is_english_mode())
		log_error("Unknown button type \"%s\".", type);
	else
		log_error(U8("未知のボタンタイプ \"%s\""), type);
}

/*
 * GUIファイルで未知のボタンプロパティが指定された際のエラーを記録する
 */
void log_gui_unknown_button_property(const char *prop)
{
	if (is_english_mode())
		log_error("Unknown button property \"%s\".", prop);
	else
		log_error(U8("未知のボタンプロパティ \"%s\""), prop);
}

/*
 * GUIファイルでtypeの前に他のプロパティが記述された際のエラーを記録する
 */
void log_gui_parse_property_before_type(const char *prop)
{
	if (is_english_mode()) {
		log_error("Property \"%s\" is specified before \"type\".",
			  prop);
	} else {
		log_error(U8("プロパティ \"%s\" が \"type\" より前に")
			  U8("指定されました。"), prop);
	}
}

/*
 * GUIファイルのパースに失敗した際のエラーを記録する
 */
void log_gui_parse_footer(const char *file, int line)
{
	line++;
	if (is_english_mode())
		log_error("> GUI file error: %s:%d", file, line);
	else
		log_error(U8("> GUIファイルエラー: %s:%d"), file, line);
}

/*
 * GUIファイルでイメージが指定されていない際のエラーを記録する
 */
void log_gui_image_not_loaded(void)
{
	if (is_english_mode())
		log_error("GUI image(s) not specified.");
	else
		log_error(U8("GUI画像が指定されていません。"));
}

/*
 * WMSの構文エラーを記録する
 */
void log_wms_syntax_error(const char *file, int line, int column)
{
	if (is_english_mode()) {
		log_error("%s: Syntax error at line %d column %d.\n",
			  file, line, column);
	} else {
		log_error(U8("%s: 構文エラー %d行目 %d桁目"),
			  file, line, column);
	}
}

/*
 * WMSの実行時エラーを記録する
 */
void log_wms_runtime_error(const char *file, int line, const char *msg)
{
	if (is_english_mode())
		log_error("%s: Runtime error at line %d: %s.\n", file, line, msg);
	else
		log_error(U8("%s: 実行エラー %d行目: %s"), file, line, msg);
}

/*
 * アニメファイルにパースできない文字がある際のエラーを記録する
 */
void log_anime_parse_char(char c)
{
	if (is_english_mode())
		log_error("Invalid character \'%c\'", c);
	else
		log_error(U8("不正な文字 \'%c\'"), c);
}

/*
 * アニメファイルの記述が長すぎる際のエラーを記録する
 */
void log_anime_parse_long_word(void)
{
	if (is_english_mode())
		log_error("Too long word.");
	else
		log_error(U8("記述が長すぎます。"));
}

/*
 * アニメファイルで必要な記述が空白である際のエラーを記録する
 */
void log_anime_parse_empty_word(void)
{
	if (is_english_mode())
		log_error("Nothing is specified.");
	else
		log_error(U8("空白が指定されました。"));
}

/*
 * アニメファイルで不正なEOFが現れた際のエラーを記録する
 */
void log_anime_parse_invalid_eof(void)
{
	if (is_english_mode())
		log_error("Invalid End-of-File.");
	else
		log_error(U8("不正なファイル終端です。"));
}

/*
 * アニメーションのシーケンスが長すぎるエラーを記録する
 */
void log_anime_long_sequence(void)
{
	if (is_english_mode())
		log_error("Anime sequence too long\n");
	else
		log_error(U8("アニメーションシーケンスが長すぎます"));
}

/*
 * アニメーションファイルでレイヤ名が指定されていないエラーを記録する
 */
void log_anime_layer_not_specified(const char *key)
{
	if (is_english_mode())
		log_error("\"%s\" appeared before \"layer\"\n", key);
	else
		log_error(U8("\"layer\"の前に\"%s\"が指定されました"), key);
}

/*
 * アニメーションファイルで未定義のキーが指定されたエラーを記録する
 */
void log_anime_unknown_key(const char *key)
{
	if (is_english_mode())
		log_error("Unknown keyword \"%s\"\n", key);
	else
		log_error(U8("未知のキーワード \"%s\"が指定されました"), key);
}

/*
 * アニメファイルのパースに失敗した際のエラーを記録する
 */
void log_anime_parse_footer(const char *file, int line)
{
	line++;
	if (is_english_mode())
		log_error("> Anime file error: %s:%d", file, line);
	else
		log_error(U8("> アニメファイルエラー: %s:%d"), file, line);
}

/*
 * レイヤ名の誤りを記録する
 */
void log_invalid_layer_name(const char *name)
{
	if (is_english_mode())
		log_error("Unknown layer name \"%s\"\n", name);
	else
		log_error(U8("未知のレイヤ名 \"%s\"が指定されました"), name);
}

#ifdef USE_EDITOR
/*
 * スクリプトにエラーがあった際の情報提供を行う
 */
void log_inform_translated_commands(void)
{
	if (is_english_mode()) {
		log_info("Invalid commands were translated to messages "
			 "that start with \'!\'.\n"
			 "You can search them by a menu.\n");
	} else {
		log_info(U8("エラーを含むコマンドが'!'で始まるメッセージに")
			 U8("変換されました。\n")
			 U8("エラーはメニューから検索できます。\n"));
	}
}

/*
 * スクリプトの行数が既定値を超えた際のエラーを記録する
 */
void log_script_line_size(void)
{
	if (is_english_mode())
		log_info("Too many lines in script.");
	else
		log_info(U8("スクリプトの行数が大きすぎます。"));
}

/*
 * ディレクトリがみつからない際のエラーを記録する
 */
void log_dir_not_found(const char *dir)
{
	if (is_english_mode())
		log_info("Folder \'%s\' not found.", dir);
	else
		log_info(U8("フォルダ\'%s\'がみつかりません。"), dir);
}

/*
 * パッケージするファイルの数が多すぎる際のエラーを記録する
 */
void log_too_many_files(void)
{
	if (is_english_mode())
		log_info("Too many files to package.");
	else
		log_info(U8("パッケージするファイル数が多すぎます。"));
}
#endif
