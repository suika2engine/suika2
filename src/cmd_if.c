/* -*- coding: utf-8; tab-width: 8; indent-tabs-mode: t; -*- */

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
 * ifコマンドの実装
 */
bool if_command(void)
{
	const char *lhs, *op, *rhs, *label;
	int lval, rval, cmp;

	lhs = get_string_param(IF_PARAM_LHS);
	op = get_string_param(IF_PARAM_OP);
	rhs = get_string_param(IF_PARAM_RHS);
	label = get_string_param(IF_PARAM_LABEL);

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
