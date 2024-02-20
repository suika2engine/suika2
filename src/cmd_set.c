/* -*- coding: utf-8; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Suika2
 * Copyright (C) 2001-2023, Keiichi Tabata. All rights reserved.
 */

/*
 * [Changes]
 *  - 2016/06/29 作成
 *  - 2023/06/11 名前変数に対応
 */

#include "suika.h"

/* ランダムな値を意味する変数名 */
#define RANDOM_VARIABLE	"$RAND"

/* 前方参照 */
static bool process_normal_var(const char *lhs, const char *op,
			       const char *rhs);
static bool process_name_var(const char *lhs, const char *op, const char *rhs);

/*
 * setコマンドの実装
 */
bool set_command(void)
{
	const char *lhs, *op, *rhs;

	lhs = get_string_param(SET_PARAM_LHS);
	op = get_string_param(SET_PARAM_OP);
	rhs = get_string_param(SET_PARAM_RHS);

	if (lhs[0] == '%') {
		/* 名前変数の場合 */
		if (!process_name_var(lhs, op, rhs))
			return false;
	} else {
		/* ローカル変数/グローバル変数の場合 */
		if (!process_normal_var(lhs, op, rhs))
			return false;
	}

	return move_to_next_command();
}

/* ローカル変数/グローバル変数の場合を処理する */
static bool process_normal_var(const char *lhs,
			       const char *op,
			       const char *rhs)
{
	int lval_index, lval, rval_index, rval, val;

	/* 左辺の値を求める */
	if (lhs[0] != '$' || strlen(lhs) == 1) {
		log_script_lhs_not_variable(lhs);
		log_script_exec_footer();
		return false;
	}
	if (lhs[1] == '{') {
		char *stop = strstr(lhs, "}");
		int len, i;
		if (stop == NULL) {
			log_script_not_variable(lhs);
			return false;
		}
		len = (int)(stop - lhs) - 2;
		lval_index = -1;
		for (i = 0; i < NAMED_VAR_COUNT; i++) {
			if (conf_var_name[i] == NULL)
				continue;
			if (strncmp(lhs + 2, conf_var_name[i], (size_t)len) == 0){
				lval_index = i;
				break;
			}
		}
		if (lval_index == -1) {
			log_script_not_variable(lhs);
			return false;
		}
	} else {
		lval_index = atoi(&lhs[1]);
	}
	if (lval_index < 0 || lval_index >= VAR_SIZE) {
		log_script_var_index(lval_index);
		log_script_exec_footer();
		return false;
	}
	lval = get_variable(lval_index);

	/* 右辺の値を求める */
	if (strcmp(rhs, RANDOM_VARIABLE) == 0) {
		srand((unsigned int)time(NULL));
		rval = rand();
	} else if (strcmp(rhs, "yes") == 0) {
		rval = 1;
	} else if (strcmp(rhs, "no") == 0) {
		rval = 0;
	} else if (strcmp(rhs, "true") == 0) {
		rval = 1;
	} else if (strcmp(rhs, "false") == 0) {
		rval = 0;
	} else if (strcmp(rhs, U8("はい")) == 0) {
		rval = 1;
	} else if (strcmp(rhs, U8("いいえ")) == 0) {
		rval = 0;
	} else if (rhs[0] == '$' && strlen(rhs) > 1) {
		rval_index = atoi(&rhs[1]);
		if (rval_index < 0 || rval_index >= VAR_SIZE) {
			log_script_var_index(rval_index);
			log_script_exec_footer();
			return false;
		}
		rval = get_variable(rval_index);
	} else {
		rval = atoi(rhs);
	}

	/* 計算する */
	if (strcmp(op, "=") == 0) {
		val = rval;
	} else if (strcmp(op, "+=") == 0) {
		val = lval + rval;
	} else if (strcmp(op, "-=") == 0) {
		val = lval - rval;
	} else if (strcmp(op, "*=") == 0) {
		val = lval * rval;
	} else if (strcmp(op, "/=") == 0) {
		val = lval / rval;
	} else if (strcmp(op, "%=") == 0) {
		val = lval % rval;
	} else {
		log_script_op_error(op);
		log_script_exec_footer();
		return false;
	}
	set_variable(lval_index, val);

	return true;
}

/* 名前変数の場合を処理する */
static bool process_name_var(const char *lhs, const char *op, const char *rhs)
{
	int lval_index;

	/* 左辺の値を求める */
	if (strlen(lhs) != 2 || !(lhs[1] >= 'a' && lhs[1] <= 'z')) {
		log_script_lhs_not_variable(lhs);
		log_script_exec_footer();
		return false;
	}
	lval_index = lhs[1] - 'a';
	assert(lval_index >= 0 && lval_index < 27);

	/* 代入する */
	if (strcmp(op, "=") == 0) {
		set_name_variable(lval_index, rhs);
	} else {
		log_script_op_error(op);
		log_script_exec_footer();
		return false;
	}

	return true;
}
