/* -*- coding: utf-8; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Suika 2
 * Copyright (C) 2001-2022, TABATA Keiichi. All rights reserved.
 */

/*
 * [Changes]
 *  - 2016/06/29 作成
 *  - 2017/08/17 グローバル変数に対応
 *  - 2022/06/09 デバッガに対応
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

/* expand_variable()のバッファ */
static char expand_variable_buf[4096];

#ifdef USE_DEBUGGER
static bool flag_var_updated;
static int updated_index;
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
#ifdef USE_DEBUGGER
	flag_var_updated = false;
	clear_variable_changed();
#endif
}

/*
 * 変数の終了処理を行う
 */
void cleanup_vars(void)
{
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

#ifdef USE_DEBUGGER
	if (index >= VAR_SIZE)
		return;
	flag_var_updated = true;
	updated_index = index;
	is_var_changed[index] = true;
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

	index = atoi(&var[1]);
	if (index < 0 || index >= VAR_SIZE) {
		log_script_var_index(index);
		log_script_exec_footer();
		return false;
	}

	set_variable(index, val);
	return true;
}

/*
 * 文字列の中の変数を展開して返す
 */
const char *expand_variable(const char *msg)
{
	char var[5];
	char *d;
	size_t buf_size;
	int i, index;

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
			for (i = 0; i < 4; i++) {
				if (isdigit(*msg))
					var[i] = *msg++;
				else
					break;
			}
			var[i] = '\0';

			/* 変数番号から値を取得して文字列にする */
			if (i > 0) {
				index = atoi(var);
				if (index >= 0 && index < VAR_SIZE) {
					d += snprintf(d,
						      buf_size -
						      (size_t)(d -
							       expand_variable_buf),
						      "%d",
						      get_variable(index));
				}
			} else {
				/* 不正な変数番号の場合、$を出力する */
				*d++ = '$';
			}
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
 * デバッガ用
 */
#ifdef USE_DEBUGGER
/*
 * 変数の値が更新されたかをチェックする
 */
bool check_variable_updated(void)
{
	bool ret = flag_var_updated;
	flag_var_updated = false;
	return ret;
}

/*
 * 更新された変数のインデックスを取得する
 */
int get_updated_variable_index(void)
{
	return updated_index;
}

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
