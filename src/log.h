/* -*- coding: utf-8-with-signature; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Suika 2
 * Copyright (C) 2001-2022, TABATA Keiichi. All rights reserved.
 */

/*
 * [Changes]
 *  - 2016/05/27 作成
 *  - 2017/08/13 スイッチに対応
 *  - 2021/06/05 @bg, @chのエフェクト名エラーを追加
 *  - 2021/06/10 @chaの加速タイプ名エラーを追加
 *  - 2021/06/12 @shakeの移動タイプ名エラーを追加
 *  - 2021/06/15 @setsaveのパラメタのエラーを追加
 *  - 2021/07/07 @goto $SAVEのエラーを追加
 *  - 2022/06/14 Suika2 Pro for Creators
 *  - 2022/07/28 GUIモジュール対応
 */

#ifndef SUIKA_LOG_H
#define SUIKA_LOG_H

/*
 * ログを出力してよいのはメインスレッドのみとする。
 */

void log_api_error(const char *api);
void log_audio_file_error(const char *dir, const char *file);
void log_dir_file_open(const char *dir, const char *file);
void log_file_name(const char *dir, const char *file);
void log_file_open(const char *fname);
void log_file_read(const char *dir, const char *file);
void log_font_file_error(const char *font);
void log_image_file_error(const char *dir, const char *file);
void log_memory(void);
void log_package_file_error(void);
void log_duplicated_conf(const char *key);
void log_undefined_conf(const char *key);
void log_unknown_conf(const char *key);
void log_wave_error(const char *fname);
void log_invalid_msgbox_size(void);
void log_script_exec_footer(void);
void log_script_command_not_found(const char *name);
void log_script_empty_serif(void);
void log_script_ch_position(const char *pos);
void log_script_fade_method(const char *method);
void log_script_label_not_found(const char *name);
void log_script_lhs_not_variable(const char *lhs);
void log_script_no_command(const char *file);
void log_script_not_variable(const char *lhs);
void log_script_non_positive_size(int val);
void log_script_too_few_param(int min, int real);
void log_script_too_many_param(int max, int real);
void log_script_op_error(const char *op);
void log_script_parse_footer(const char *file, int line, const char *buf);
void log_script_return_error(void);
void log_script_rgb_negative(int val);
void log_script_size(int size);
void log_script_switch_no_label(void);
void log_script_switch_no_item(void);
void log_script_var_index(int index);
void log_script_vol_value(float vol);
void log_script_mixer_stream(const char *stream);
void log_script_cha_accel(const char *accel);
void log_script_shake_move(const char *move);
void log_script_enable_disable(const char *param);
void log_script_final_command(void);
void log_video_error(const char *reason);
void log_script_choose_no_message(void);
void log_script_empty_string(void);
void log_file_write(const char *file);
void log_script_rule(void);
void log_gui_parse_char(const char *file, int line, char c);
void log_gui_parse_long_word(const char *file, int line);
void log_gui_parse_empty_word(const char *file, int line);
void log_gui_parse_invalid_eof(const char *file, int line);
void log_gui_unknown_global_key(const char *file, const char *key);
void log_gui_too_many_buttons(const char *file);
void log_gui_unknown_button_type(const char *file, const char *type);
void log_gui_unknown_button_property(const char *file, const char *key);

#ifdef USE_DEBUGGER
void log_command_update_error(void);
void log_script_line_size(void);
void log_dir_not_found(const char *dir);
void log_too_many_files(void);
#endif

#endif
