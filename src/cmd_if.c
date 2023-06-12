/* -*- coding: utf-8; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Suika 2
 * Copyright (C) 2001-2023, TABATA Keiichi. All rights reserved.
 */

/*
 * [Changes]
 *  - 2016/06/29 作成
 *  - 2023/06/11 名前変数に対応
 */

#include "suika.h"

/* 前方参照 */
static bool process_name_var(const char *lhs, const char *op, const char *rhs,
			     const char *label);
static bool process_normal_var(const char *lhs, const char *op,
			       const char *rhs, const char *label);

/*
 * ifコマンドの実装
 */
bool if_command(void)
{
	const char *lhs, *op, *rhs, *label;

	lhs = get_string_param(IF_PARAM_LHS);
	op = get_string_param(IF_PARAM_OP);
	rhs = get_string_param(IF_PARAM_RHS);
	label = get_string_param(IF_PARAM_LABEL);

	/* 左辺が名前変数の場合 */
	if (lhs[0] == '%')
		return process_name_var(lhs, op, rhs, label);

	/* 左辺がローカル変数/グローバル変数の場合 */
	return process_normal_var(lhs, op, rhs, label);
}

/* 左辺が名前変数の場合を処理する */
static bool process_name_var(const char *lhs, const char *op, const char *rhs,
			     const char *label)
{
	const char *lval_s;
	int index, cmp;

	/* 左辺の値を求める */
	if (strlen(lhs) != 2 || !(lhs[1] >= 'a' && lhs[1] <= 'z')) {
		log_script_lhs_not_variable(lhs);
		log_script_exec_footer();
		return false;
	}
	index = lhs[1] - 'a';
	lval_s = get_name_variable(index);

	/* 計算する */
	if (strcmp(op, "==") == 0) {
		cmp = strcmp(lval_s, rhs) == 0;
	} else if (strcmp(op, "!=") == 0) {
		cmp = strcmp(lval_s, rhs) != 0;
	} else {
		log_script_op_error(op);
		log_script_exec_footer();
		return false;
	}

	/* 比較結果が真ならラベルにジャンプする  */
	if (cmp)
		return move_to_label(label);

	/* 比較結果が偽なら次のコマンドに移動する */
	return move_to_next_command();
}

/* 左辺がローカル変数/グローバル変数の場合を処理する */
static bool process_normal_var(const char *lhs, const char *op,
			       const char *rhs, const char *label)
{
	int lval, rval, cmp;

	/* 左辺の値を求める */
	if (lhs[0] != '$' || strlen(lhs) == 1) {
		log_script_lhs_not_variable(lhs);
		log_script_exec_footer();
		return false;
	}
	lval = get_variable(atoi(&lhs[1]));

	/* 右辺の値を求める */
	if (rhs[0] == '$' && strlen(rhs) > 1)
		rval = get_variable(atoi(&rhs[1]));
	else
		rval = atoi(rhs);

	/* 計算する */
	if (strcmp(op, ">") == 0) {
		cmp = lval > rval;
	} else if (strcmp(op, ">=") == 0) {
		cmp = lval >= rval;
	} else if (strcmp(op, "==") == 0) {
		cmp = lval == rval;
	} else if (strcmp(op, "<=") == 0) {
		cmp = lval <= rval;
	} else if (strcmp(op, "<") == 0) {
		cmp = lval < rval;
	} else if (strcmp(op, "!=") == 0) {
		cmp = lval != rval;
	} else {
		log_script_op_error(op);
		log_script_exec_footer();
		return false;
	}

	/* 比較結果が真ならラベルにジャンプする  */
	if (cmp)
		return move_to_label(label);

	/* 比較結果が偽なら次のコマンドに移動する */
	return move_to_next_command();
}
