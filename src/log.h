/* -*- coding: utf-8-with-signature; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Suika 2
 * Copyright (C) 2001-2016, TABATA Keiichi. All rights reserved.
 */

/*
 * [Changes]
 *  - 2016/05/27 作成
 */

#ifndef SUIKA_LOG_H
#define SUIKA_LOG_H

/*
 * ログを出力してよいのはメインスレッドのみとする。
 */

void log_api_error(const char *api);
void log_audio_file_error(const char *dir, const char *file);
void log_ch_position(const char *pos);
void log_dir_file_open(const char *dir, const char *file);
void log_file_open(const char *fname);
void log_font_file_error(const char *font);
void log_image_file_error(const char *dir, const char *file);
void log_memory(void);
void log_package_file_error(void);
void log_script_command_not_found(const char *name);
void log_script_empty_serif(void);
void log_script_exec_footer(void);
void log_script_label_not_found(const char *name);
void log_script_lhs_not_variable(const char *lhs);
void log_script_no_command(const char *file);
void log_script_not_variable(const char *lhs);
void log_script_non_positive_size(int val);
void log_script_too_few_param(int min, int real);
void log_script_too_many_param(int max, int real);
void log_script_op_error(const char *op);
void log_script_param_string(int param);
void log_script_parse_footer(const char *file, int line, const char *buf);
void log_script_rgb_negative(int val);
void log_script_size(int size);
void log_script_switch_no_item(void);
void log_script_var_index(int index);
void log_undefined_conf(const char *key);
void log_unknown_conf(const char *key);
void log_wave_error(const char *fname);

#endif
