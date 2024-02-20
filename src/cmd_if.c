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

/* 前方参照 */
static bool process_name_var(const char *lhs, const char *op, const char *rhs,
			     const char *label, const char *finally_label);
static bool process_normal_var(const char *lhs, const char *op,
			       const char *rhs, const char *label,
			       const char *finally_label);

/*
 * ifコマンドの実装
 */
bool if_command(void)
{
	const char *lhs, *op, *rhs, *label, *finally_label;

	lhs = get_string_param(IF_PARAM_LHS);
	op = get_string_param(IF_PARAM_OP);
	rhs = get_string_param(IF_PARAM_RHS);
	label = get_string_param(IF_PARAM_LABEL);
	if (get_command_type() == COMMAND_UNLESS)
		finally_label = get_string_param(UNLESS_PARAM_FINALLY);
	else
		finally_label = "";

	/* 左辺が名前変数の場合 */
	if (lhs[0] == '%')
		return process_name_var(lhs, op, rhs, label, finally_label);

	/* 左辺がローカル変数/グローバル変数の場合 */
	return process_normal_var(lhs, op, rhs, label, finally_label);
}

/* 左辺が名前変数の場合を処理する */
static bool process_name_var(const char *lhs, const char *op, const char *rhs,
			     const char *label, const char *finally_label)
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

	if (get_command_type() == COMMAND_IF) {
		/* 比較結果が真ならラベルにジャンプする  */
		if (cmp)
			return move_to_label(label);
	} else {
		/* 比較結果が偽ならラベルにジャンプする  */
		if (!cmp)
			return move_to_label_finally(label, finally_label);
	}

	/* 次のコマンドに移動する */
	return move_to_next_command();
}

/* 左辺がローカル変数/グローバル変数の場合を処理する */
static bool process_normal_var(const char *lhs, const char *op,
			       const char *rhs, const char *label,
			       const char *finally_label)
{
	int lval_index, lval, rval, cmp;

	/* 左辺の値を求める */
	if (lhs[0] != '$' || strlen(lhs) == 1) {
		log_script_lhs_not_variable(lhs);
		log_script_exec_footer();
		return false;
	}
	lval_index = -1;
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
	lval = get_variable(lval_index);

	/* 右辺の値を求める */
	if (rhs[0] == '$' && strlen(rhs) > 1)
		rval = get_variable(atoi(&rhs[1]));
	else if (strcmp(rhs, "true") == 0)
		rval = 1;
	else if (strcmp(rhs, "false") == 0)
		rval = 0;
	else if (strcmp(rhs, "yes") == 0)
		rval = 1;
	else if (strcmp(rhs, "no") == 0)
		rval = 0;
	else if (strcmp(rhs, U8("はい")) == 0)
		rval = 1;
	else if (strcmp(rhs, U8("いいえ")) == 0)
		rval = 0;
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

	if (get_command_type() == COMMAND_IF) {
		/* 比較結果が真ならラベルにジャンプする  */
		if (cmp)
			return move_to_label(label);
	} else {
		/* 比較結果が偽ならラベルにジャンプする  */
		if (!cmp) {
			return move_to_label_finally(label, finally_label);
		}
	}

	/* 次のコマンドに移動する */
	return move_to_next_command();
}
