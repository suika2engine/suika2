/* -*- coding: utf-8; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Suika2
 * Copyright (C) 2001-2023, Keiichi Tabata. All rights reserved.
 */

/*
 * [Changes]
 *  - 2016/06/29 作成
 *  - 2017/08/17 グローバル変数に対応
 *  - 2022/06/09 デバッガに対応
 *  - 2023/06/11 名前変数に対応
 */

#include "suika.h"

/*
 * ローカル変数テーブル
 */
static int32_t local_var_tbl[LOCAL_VAR_SIZE];

/*
 * グローバル変数テーブル
 */
static int32_t global_var_tbl[GLOBAL_VAR_SIZE];

/*
 * 名前変数テーブル('a' to 'z')
 */
static char *name_var_tbl[NAME_VAR_SIZE];

/* expand_variable()のバッファ */
static char expand_variable_buf[4096];

#ifdef USE_EDITOR
static bool is_var_changed[LOCAL_VAR_SIZE + GLOBAL_VAR_SIZE];
#endif

/*
 * 変数の初期化処理を行う
 */
void init_vars(void)
{
	int i;

	/* Androidでは再利用されるので初期化する */
	for (i = 0; i < LOCAL_VAR_SIZE; i++)
		local_var_tbl[i] = 0;
	for (i = 0; i < GLOBAL_VAR_SIZE; i++)
		global_var_tbl[i] = 0;
	for (i = 0; i < NAME_VAR_SIZE; i++) {
		if (name_var_tbl[i] != NULL)
			free(name_var_tbl[i]);
		name_var_tbl[i] = NULL;
	}

#ifdef USE_EDITOR
	clear_variable_changed();
#endif
}

/*
 * 変数の終了処理を行う
 */
void cleanup_vars(void)
{
	int i;

	for (i = 0; i < NAME_VAR_SIZE; i++) {
		if (name_var_tbl[i] != NULL) {
			free(name_var_tbl[i]);
			name_var_tbl[i] = NULL;
		}
	}
}

/*
 * 変数を取得する
 */
int32_t get_variable(int index)
{
	assert(index < VAR_SIZE);

	if (index < GLOBAL_VAR_OFFSET)
		return local_var_tbl[index];
	else
		return global_var_tbl[index - GLOBAL_VAR_OFFSET];
}

/*
 * 変数を設定する
 */
void set_variable(int index, int32_t val)
{
	assert(index < VAR_SIZE);

#ifdef USE_EDITOR
	if (index >= VAR_SIZE)
		return;
	is_var_changed[index] = true;
	on_update_variable();
#endif

	if (index < GLOBAL_VAR_OFFSET)
		local_var_tbl[index] = val;
	else
		global_var_tbl[index - GLOBAL_VAR_OFFSET] = val;
}

/*
 * 変数を文字列で指定して取得する
 */
bool get_variable_by_string(const char *var, int32_t *val)
{
	int index;

	if (var[0] != '$' || strlen(var) == 1) {
		log_script_not_variable(var);
		return false;
	}

	if (var[1] == '{') {
		char *stop = strstr(var, "}");
		int len, i;
		if (stop == NULL) {
			log_script_not_variable(var);
			return false;
		}
		len = (int)(stop - var) - 2;
		for (i = 0; i < NAMED_VAR_COUNT; i++) {
			if (conf_var_name[i] == NULL)
				continue;
			if (strncmp(var + 2, conf_var_name[i], (size_t)len) == 0){
				*val = get_variable(i);
				return true;
			}
		}
		log_script_not_variable(var);
		return false;
	}

	index = atoi(&var[1]);
	if (index < 0 || index >= VAR_SIZE) {
		log_script_var_index(index);
		return false;
	}

	*val = get_variable(index);
	return true;
}

/*
 * 変数を文字列で指定して設定する
 */
bool set_variable_by_string(const char *var, int32_t val)
{
	int index;

	if (var[0] != '$' || strlen(var) == 1) {
		log_script_not_variable(var);
		log_script_exec_footer();
		return false;
	}

	index = -1;
	if (var[1] == '{') {
		char *stop = strstr(var, "}");
		int len, i;
		if (stop == NULL) {
			log_script_not_variable(var);
			return false;
		}
		len = (int)(stop - var) - 2;
		for (i = 0; i < NAMED_VAR_COUNT; i++) {
			if (conf_var_name[i] == NULL)
				continue;
			if (strncmp(var + 2, conf_var_name[i], (size_t)len) == 0){
				index = i;
				break;
			}
		}
		if (index == -1) {
			log_script_not_variable(var);
			return false;
		}
	} else {
		index = atoi(&var[1]);
	}

	if (index < 0 || index >= VAR_SIZE) {
		log_script_var_index(index);
		log_script_exec_footer();
		return false;
	}

	set_variable(index, val);
	return true;
}

/*
 * 名前変数を取得する
 */
const char *get_name_variable(int index)
{
	assert(index >= 0 && index < NAME_VAR_SIZE);

	if (name_var_tbl[index] == NULL)
		return "";

	return name_var_tbl[index];
}

/*
 * 名前変数を設定する
 */
bool set_name_variable(int index, const char *val)
{
	assert(index >= 0 && index < NAME_VAR_SIZE);
	assert(val != NULL);

	if (name_var_tbl[index] != NULL) {
		free(name_var_tbl[index]);
		name_var_tbl[index] = NULL;
	}

	if (val != NULL) {
		name_var_tbl[index] = strdup(val);
		if (name_var_tbl[index] == NULL) {
			log_memory();
			return false;
		}
	}

	return true;
}

/*
 * 名前変数の最後の文字を消去する
 */
void truncate_name_variable(int index)
{
	char *s;
	uint32_t wc;
	int i, len;

	assert(index >= 0 && index < NAME_VAR_SIZE);

	s = name_var_tbl[index];
	if (s == NULL)
		return;

	len = count_utf8_chars(s);

	/* ワイド文字数で(len-1)の分だけutf-8のポインタを進める */
	for (i = 0; i < len - 1; i++)
		s += utf8_to_utf32(s, &wc);

	/* 末尾のワイド文字に相当する位置で文字列を切る */
	*s = '\0';
}

/*
 * 文字列の中の変数を展開して返す
 */
const char *expand_variable(const char *msg)
{
	return expand_variable_with_increment(msg, 0);
}

/*
 * 文字列の中の変数を展開して返す(変数のインクリメントも行う)
 */
const char *expand_variable_with_increment(const char *msg, int inc)
{
	char var[16];
	char *d;
	size_t buf_size;
	int i, index, name_index, value;

	d = expand_variable_buf;
	buf_size = sizeof(expand_variable_buf);
	while (*msg && d < &expand_variable_buf[buf_size - 2]) {
		/* 変数参照の場合 */
		if (*msg == '$') {
			/* エスケープの場合 */
			if (*(msg + 1) == '$') {
				*d++ = *msg;
				msg += 2;
				continue;
			}

			/* 変数番号を取得する */
			msg++;
			index = -1;
			if (*msg == '{') {
				char *stop = strstr(var, "}");
				int len, i;
				if (stop == NULL) {
					log_script_not_variable(var);
					return false;
				}
				len = (int)(stop - var) - 2;
				for (i = 0; i < NAMED_VAR_COUNT; i++) {
					if (conf_var_name[i] == NULL)
						continue;
					if (strncmp(var + 2, conf_var_name[i], (size_t)len) == 0){
						index = i;
						break;
					}
				}
			} else {
				for (i = 0; i < 5; i++) {
					if (isdigit((int)*msg))
						var[i] = *msg++;
					else
						break;
				}
				var[i] = '\0';
				index = atoi(var);
			}

			/* 変数番号から値を取得して文字列にする */
			if (index >= 0 && index < VAR_SIZE) {
				value = get_variable(index);
				d += snprintf(d,
					      buf_size - (size_t)(d - expand_variable_buf),
					      "%d",
					      value);
				set_variable(index, value + inc);
			} else {
				/* 不正な変数番号の場合、$を出力する */
				*d++ = '$';
			}
		} else if (*msg == '%' &&
			   (*(msg + 1) >= 'a' && *(msg + 1) <= 'z')) {
			/* 名前変数参照の場合 */
			name_index = *(msg + 1) - 'a';
			assert(name_index >= 0 && name_index < NAME_VAR_SIZE);
			d += snprintf(d,
				      buf_size -
				      (size_t)(d - expand_variable_buf),
				      "%s",
				      get_name_variable(name_index));
			msg += 2;
		} else {
			/* 変数参照でない場合 */
			*d++ = *msg++;
		}
	}

	*d = '\0';
	return expand_variable_buf;
}

/*
 * ローカル変数テーブルへのポインタを取得する
 */
int32_t *get_local_variables_pointer(void)
{
	return local_var_tbl;
}

/*
 * ローカル変数テーブルへのポインタを取得する
 */
int32_t *get_global_variables_pointer(void)
{
	return global_var_tbl;
}

/*
 * Suika2 Pro用
 */
#ifdef USE_EDITOR
/*
 * 変数が初期値から更新されているかを調べる
 */
bool is_variable_changed(int index)
{
	return is_var_changed[index];
}

/*
 * 変数の更新状態をクリアする
 */
void clear_variable_changed(void)
{
	int i;

	for (i = 0; i < LOCAL_VAR_SIZE + GLOBAL_VAR_SIZE; i++)
		is_var_changed[i] = false;
}
#endif
