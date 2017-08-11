/* -*- coding: utf-8-with-signature; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Suika 2
 * Copyright (C) 2001-2016, TABATA Keiichi. All rights reserved.
 */

/*
 * [Changes]
 *  - 2016/06/29 作成
 */

#include "suika.h"

/*
 * 変数テーブル
 */
static int32_t var_tbl[VAR_SIZE];

/*
 * 変数の初期化処理を行う
 */
void init_vars(void)
{
	int i;

	/* Androidでは再利用されるので初期化する */
	for (i = 0; i < VAR_SIZE; i++)
		var_tbl[i] = 0;
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

	return var_tbl[index];
}

/*
 * 変数を設定する
 */
void set_variable(int index, int32_t val)
{
	assert(index < VAR_SIZE);

	var_tbl[index] = val;
}

/*
 * 変数を文字列で指定して取得する
 */
bool get_variable_by_string(const char *var, int32_t *val)
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

	*val = var_tbl[index];
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

	var_tbl[index] = val;
	return true;
}

/*
 * 文字列の中の変数を展開して返す
 */
char *expand_variable(const char *msg)
{
	char dst[1024];
	char var[5];
	char *d;
	int i, index;

	d = dst;
	while (*msg && d < &dst[sizeof(dst) - 2]) {
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
					d += snprintf(d, sizeof(dst) -
						      (size_t)(d - dst), "%d",
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
	return strdup(dst);
}
