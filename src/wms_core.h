/*
 * Watermelon Script
 * Copyright (c) 2022, Keiichi Tabata. All rights reserved.
 */

#ifndef WMS_COMMON_H
#define WMS_COMMON_H

#include "wms.h"
#include <stdbool.h>

/*
 * Error Line Number
 */
extern int wms_error_line;

/*
 * AST Element Structures
 */

struct wms_func_list;
struct wms_func;
struct wms_param_list;
struct wms_param;
struct wms_stmt_list;
struct wms_stmt;
struct wms_expr_stmt;
struct wms_assign_stmt;
struct wms_if_stmt;
struct wms_elif_stmt;
struct wms_else_stmt;
struct wms_while_stmt;
struct wms_for_stmt;
struct wms_return_stmt;
struct wms_expr;
struct wms_term;
struct wms_arg_list;

struct wms_func_list {
	struct wms_func *list;
};

struct wms_func {
	char *name;
	struct wms_param_list *param_list;
	struct wms_stmt_list *stmt_list;

	struct wms_func *next;
};

struct wms_param_list {
	struct wms_param *list;
};

struct wms_param {
	char *symbol;
	struct wms_param *next;
};

struct wms_stmt_list {
	struct wms_stmt *list;
};

struct wms_stmt {
	struct {
		unsigned int is_empty : 1;
		unsigned int is_expr : 1;
		unsigned int is_assign : 1;
		unsigned int is_if : 1;
		unsigned int is_elif : 1;
		unsigned int is_else : 1;
		unsigned int is_while : 1;
		unsigned int is_for : 1;
		unsigned int is_return : 1;
		unsigned int is_break : 1;
		unsigned int is_continue : 1;
	} type;
	union {
		struct wms_expr_stmt *expr;
		struct wms_assign_stmt *assign;
		struct wms_if_stmt *_if;
		struct wms_elif_stmt *elif;
		struct wms_else_stmt *_else;
		struct wms_while_stmt *_while;
		struct wms_for_stmt *_for;
		struct wms_return_stmt *_return;
	} of;
	int line;
	int column;

	struct wms_stmt *next;
};

struct wms_expr_stmt {
	struct wms_expr *expr;
};

struct wms_assign_stmt {
	struct {
		unsigned int is_var : 1;
		unsigned int is_array : 1;
	} type;
	union {
		char *symbol;
		struct {
			char *symbol;
			struct wms_expr *subscript;
		} array;
	} lhs;
	struct wms_expr *rhs;
};

struct wms_if_stmt {
	struct wms_expr *cond;
	struct wms_stmt_list *stmt_list;
};

struct wms_elif_stmt {
	struct wms_expr *cond;
	struct wms_stmt_list *stmt_list;
};

struct wms_else_stmt {
	struct wms_stmt_list *stmt_list;
};

struct wms_while_stmt {
	struct wms_expr *cond;
	struct wms_stmt_list *stmt_list;
};

struct wms_for_stmt {
	bool is_range;
	int start;
	int end;
	char *key_symbol;
	char *value_symbol;
	struct wms_expr *array_expr;
	struct wms_stmt_list *stmt_list;
};

struct wms_return_stmt {
	struct wms_expr *expr;
};

struct wms_expr {
	struct {
		unsigned int is_term : 1;
		unsigned int is_lt : 1;
		unsigned int is_lte : 1;
		unsigned int is_gt : 1;
		unsigned int is_gte : 1;
		unsigned int is_eq : 1;
		unsigned int is_neq : 1;
		unsigned int is_plus : 1;
		unsigned int is_minus : 1;
		unsigned int is_mul : 1;
		unsigned int is_div : 1;
		unsigned int is_and : 1;
		unsigned int is_or : 1;
		unsigned int is_neg : 1;
	} type;
	union {
		struct wms_term *term;
		struct wms_expr *expr[2];
	} val;
	struct wms_expr *next;
};

struct wms_term {
	struct {
		unsigned int is_int : 1;
		unsigned int is_float : 1;
		unsigned int is_str : 1;
		unsigned int is_symbol : 1;
		unsigned int is_array : 1;
		unsigned int is_call : 1;
	} type;
	union {
		int i;
		double f;
		char *s;
		char *symbol;
		struct {
			char *symbol;
			struct wms_expr *subscript;
		} array;
		struct {
			char *func;
			struct wms_arg_list *arg_list;
		} call;
	} val;
};

struct wms_arg_list {
	struct wms_expr *list;
};

/*
 * Runtime Constants
 */

#define WMS_STR_POOL	(4096)
#define WMS_ELEM_POOL	(4096)

/*
 * Runtime Structures
 */

struct wms_runtime;
struct wms_frame;
struct wms_variable;
struct wms_value;
struct wms_array_elem;

struct wms_value {
	struct {
		unsigned int is_int : 1;
		unsigned int is_float : 1;
		unsigned int is_str : 1;
		unsigned int is_array : 1;
	} type;
	union {
		int i;
		double f;
		int s_index;
		int a_index;
	} val;
};

struct wms_str_elem {
	bool is_used;

	/* Reference count for variable references. */
	int ref;

	/* Temporary reference flag for return value. */
	bool ret_ref;

	char *s;
};

struct wms_array_elem {
	bool is_used;

	/* Reference count for variable references. */
	int ref;

	/* Temporary reference flag for return value. */
	bool ret_ref;

	/* Top nodes are used for dummy heads. */
	bool is_head;

	struct wms_value index;
	struct wms_value val;

	struct wms_array_elem *next;
};

struct wms_variable {
	char *name;
	struct wms_value val;

	struct wms_variable *next;
};

struct wms_frame {
	struct wms_variable *var_list;

	struct wms_frame *next;
};

struct wms_ffi_func {
	wms_ffi_func_ptr func;
	char *name;
	struct wms_param_list *param_list;

	struct wms_ffi_func *next;
};

struct wms_runtime {
	/* The AST */
	struct wms_func_list *func_list;

	/* Call stack. */
	struct wms_frame *frame;

	/* String pool. */
	struct wms_str_elem str_pool[WMS_STR_POOL];

	/* Array element pool. */
	struct wms_array_elem elem_pool[WMS_ELEM_POOL];

	/* if-elif-else flags */
	bool is_after_if;
	bool is_after_false_if;

	/* The line number of current statement. */
	int cur_stmt_line;

	/* The line number that an error occured. */
	int error_line;

	/* The error message. */
	char *error_message;

	/* Foreign function list. */
	struct wms_ffi_func *ffi_func_list;
};

/*
 * AST Construction
 */

struct wms_func_list *wms_make_func_list(struct wms_func_list *func_list, struct wms_func *func);
struct wms_func *wms_make_func(char *name, struct wms_param_list *param_list, struct wms_stmt_list *stmt_list);
struct wms_param_list *wms_make_param_list(struct wms_param_list *param_list, char *symbol);
struct wms_stmt_list *wms_make_stmt_list(struct wms_stmt_list *stmt_list, struct wms_stmt *stmt);
void wms_set_stmt_position(struct wms_stmt *stmt, int line);
struct wms_stmt *wms_make_stmt_with_nothing(void);
struct wms_stmt *wms_make_stmt_with_expr(struct wms_expr *expr);
struct wms_stmt *wms_make_stmt_with_symbol_assign(char *symbol, struct wms_expr *rhs);
struct wms_stmt *wms_make_stmt_with_array_assign(char *symbol, struct wms_expr *subscript, struct wms_expr *rhs);
struct wms_stmt *wms_make_stmt_with_if(struct wms_expr *cond, struct wms_stmt_list *stmt_list);
struct wms_stmt *wms_make_stmt_with_elif(struct wms_expr *cond, struct wms_stmt_list *stmt_list);
struct wms_stmt *wms_make_stmt_with_else(struct wms_stmt_list *stmt_list);
struct wms_stmt *wms_make_stmt_with_while(struct wms_expr *cond, struct wms_stmt_list *stmt_list);
struct wms_stmt *wms_make_stmt_with_for(char *key, char *val, struct wms_expr *array, struct wms_stmt_list *stmt_list);
struct wms_stmt *wms_make_stmt_with_for_range(char *val, int start, int end, struct wms_stmt_list *stmt_list);
struct wms_stmt *wms_make_stmt_with_return(struct wms_expr *expt);
struct wms_stmt *wms_make_stmt_with_break(void);
struct wms_stmt *wms_make_stmt_with_continue(void);
struct wms_expr *wms_make_expr_with_term(struct wms_term *term);
struct wms_expr *wms_make_expr_with_lt(struct wms_expr *expr1, struct wms_expr *expr2);
struct wms_expr *wms_make_expr_with_lte(struct wms_expr *expr1, struct wms_expr *expr2);
struct wms_expr *wms_make_expr_with_gt(struct wms_expr *expr1, struct wms_expr *expr2);
struct wms_expr *wms_make_expr_with_gte(struct wms_expr *expr1, struct wms_expr *expr2);
struct wms_expr *wms_make_expr_with_eq(struct wms_expr *expr1, struct wms_expr *expr2);
struct wms_expr *wms_make_expr_with_neq(struct wms_expr *expr1, struct wms_expr *expr2);
struct wms_expr *wms_make_expr_with_plus(struct wms_expr *expr1, struct wms_expr *expr2);
struct wms_expr *wms_make_expr_with_minus(struct wms_expr *expr1, struct wms_expr *expr2);
struct wms_expr *wms_make_expr_with_mul(struct wms_expr *expr1, struct wms_expr *expr2);
struct wms_expr *wms_make_expr_with_div(struct wms_expr *expr1, struct wms_expr *expr2);
struct wms_expr *wms_make_expr_with_and(struct wms_expr *expr1, struct wms_expr *expr2);
struct wms_expr *wms_make_expr_with_or(struct wms_expr *expr1, struct wms_expr *expr2);
struct wms_expr *wms_make_expr_with_neg(struct wms_expr *expr);
struct wms_expr *wms_make_expr_with_par(struct wms_expr *expr);
struct wms_term *wms_make_term_with_int(int i);
struct wms_term *wms_make_term_with_float(double f);
struct wms_term *wms_make_term_with_str(char *s);
struct wms_term *wms_make_term_with_symbol(char *symbol);
struct wms_term *wms_make_term_with_array(char *symbol, struct wms_expr *subscript);
struct wms_term *wms_make_term_with_call(char *func, struct wms_arg_list *arg_list);
struct wms_arg_list *wms_make_arg_list(struct wms_arg_list *arg_list, struct wms_expr *expr);

#endif
