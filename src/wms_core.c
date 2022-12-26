/*
 * Watermelon Script
 * Copyright (c) 2022, Keiichi Tabata. All rights reserved.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>
#include "wms_core.h"

#ifdef _MSC_VER
#define strdup _strdup
#endif

#define NEVER_COME_HERE		(0)
#define UNIMPLEMENTED		(0)

#define AST_MEM_CHECK(p)	do { if (p == NULL) { out_of_memory(); return NULL; } } while (0)
#define RT_MEM_CHECK(p)		do { if (p == NULL) { return rterror(rt, "Out of memory"); } } while (0)

/*
 * Generated AST
 */
static struct wms_func_list *ast;

/*
 * Lexer/Parser
 */

int wms_parser_error_line;
int wms_parser_error_column;

int wms_yy_scan_string(const char *yystr);
int wms_yylex_destroy(void);
int wms_yyparse(void);

/*
 * Forward Declarations
 */

static void free_func_list(struct wms_func_list *func_list);
static void free_func(struct wms_func *func);
static void free_param(struct wms_param *param);
static void free_stmt_list(struct wms_stmt_list *stmt_list);
static void free_stmt(struct wms_stmt *stmt);
static void free_expr(struct wms_expr *expr);
static void free_term(struct wms_term *term);
static void free_frame(struct wms_runtime *rt, struct wms_frame *frame);
static void free_variable(struct wms_runtime *rt, struct wms_variable *var);
static void free_ffi_func(struct wms_ffi_func *ff);
static bool eval_func(struct wms_runtime *rt, const struct wms_func *func, const struct wms_arg_list *arg_list, struct wms_value *result);
static bool eval_stmt_list(struct wms_runtime *rt, struct wms_stmt_list *stmt_list, struct wms_value *val, bool *ret, bool *brk, bool *cont);
static bool eval_stmt(struct wms_runtime *rt, const struct wms_stmt *stmt, struct wms_value *val, bool *ret, bool *brk, bool *cont);
static bool eval_expr_stmt(struct wms_runtime *rt, const struct wms_expr_stmt *es, struct wms_value *val);
static bool eval_assign_stmt(struct wms_runtime *rt, const struct wms_assign_stmt *as, struct wms_value *val);
static bool eval_if_stmt(struct wms_runtime *rt, const struct wms_if_stmt *is, struct wms_value *val, bool *ret, bool *brk, bool *cont);
static bool eval_elif_stmt(struct wms_runtime *rt, const struct wms_elif_stmt *eis, struct wms_value *val, bool *ret, bool *brk, bool *cont);
static bool eval_else_stmt(struct wms_runtime *rt, const struct wms_else_stmt *es, struct wms_value *val, bool *ret, bool *brk, bool *cont);
static bool eval_while_stmt(struct wms_runtime *rt, const struct wms_while_stmt *ws, struct wms_value *val, bool *ret);
static bool eval_for_stmt(struct wms_runtime *rt, const struct wms_for_stmt *ws, struct wms_value *val, bool *ret);
static bool eval_return_stmt(struct wms_runtime *rt, const struct wms_return_stmt *rs, struct wms_value *val);
static bool eval_expr(struct wms_runtime *rt, struct wms_expr *expr, struct wms_value *val);
static bool calc_lt(struct wms_runtime *rt, struct wms_value val1, struct wms_value val2, struct wms_value *result);
static bool calc_lte(struct wms_runtime *rt, struct wms_value val1, struct wms_value val2, struct wms_value *result);
static bool calc_gt(struct wms_runtime *rt, struct wms_value val1, struct wms_value val2, struct wms_value *result);
static bool calc_gte(struct wms_runtime *rt, struct wms_value val1, struct wms_value val2, struct wms_value *result);
static bool calc_eq(struct wms_runtime *rt, struct wms_value val1, struct wms_value val2, struct wms_value *result);
static bool calc_plus(struct wms_runtime *rt, struct wms_value val1, struct wms_value val2, struct wms_value *result);
static bool calc_str_plus_int(struct wms_runtime *rt, struct wms_value val1, struct wms_value val2, struct wms_value *result);
static bool calc_str_plus_float(struct wms_runtime *rt, struct wms_value val1, struct wms_value val2, struct wms_value *result);
static bool calc_str_plus_str(struct wms_runtime *rt, struct wms_value val1, struct wms_value val2, struct wms_value *result);
static bool calc_minus(struct wms_runtime *rt, struct wms_value val1, struct wms_value val2, struct wms_value *result);
static bool calc_mul(struct wms_runtime *rt, struct wms_value val1, struct wms_value val2, struct wms_value *result);
static bool calc_div(struct wms_runtime *rt, struct wms_value val1, struct wms_value val2, struct wms_value *result);
static bool calc_and(struct wms_runtime *rt, struct wms_value val1, struct wms_value val2, struct wms_value *result);
static bool calc_or(struct wms_runtime *rt, struct wms_value val1, struct wms_value val2, struct wms_value *result);
static bool calc_neg(struct wms_runtime *rt, struct wms_value val, struct wms_value *result);
static bool eval_term(struct wms_runtime *rt, struct wms_term *term, struct wms_value *val);
static bool do_call(struct wms_runtime *rt, struct wms_term *term, struct wms_value *val);
static bool call_ffi_func(struct wms_runtime *rt, struct wms_ffi_func *ffi_func, struct wms_arg_list *arg_list, struct wms_value *result);
static struct wms_value value_by_int(int i);
static struct wms_value value_by_float(double f);
static bool value_by_str(struct wms_runtime *rt, struct wms_value *val, const char *s);
static int alloc_str_index(struct wms_runtime *rt);
static int alloc_elem_index(struct wms_runtime *rt, bool is_head);
static bool push_args(struct wms_runtime *rt, const struct wms_param_list *param_list, const struct wms_arg_list *arg_list);
static bool pop_return(struct wms_runtime *rt, struct wms_value *val);
static bool put_scalar_value(struct wms_runtime *rt, const char *symbol, struct wms_value val);
static bool put_array_elem_value(struct wms_runtime *rt, const char *symbol, struct wms_value index, struct wms_value val);
static bool put_array_elem_value_helper(struct wms_runtime *rt, struct wms_value array, struct wms_value index, struct wms_value val);
static bool get_scalar_value(struct wms_runtime *rt, const char *symbol, struct wms_value *val);
static bool get_scalar_value_pointer(struct wms_runtime *rt, const char *symbol, struct wms_value **val);
static bool get_array_elem_value(struct wms_runtime *rt, const char *symbol, struct wms_value index, struct wms_value *val);
static int compare_values(struct wms_runtime *rt, struct wms_value val1, struct wms_value val2);
static const char *index_to_string(struct wms_runtime *rt, struct wms_value index);
static struct wms_array_elem *get_array_head(struct wms_runtime *rt, int a_index);
static const char *get_str(struct wms_runtime *rt, int s_index);
static void increment_str_ref(struct wms_runtime *rt, int s_index);
static void decrement_str_ref(struct wms_runtime *rt, int s_index);
static void set_str_return_value_ref(struct wms_runtime *rt, int s_index);
static void reset_str_return_value_ref(struct wms_runtime *rt, int s_index);
static void increment_array_ref(struct wms_runtime *rt, int a_index);
static void decrement_array_ref(struct wms_runtime *rt, int a_index);
static void set_array_return_value_ref(struct wms_runtime *rt, int a_index);
static void print_value(struct wms_runtime *rt, struct wms_value val, bool quote);
static bool register_ffi_func(struct wms_runtime *rt, wms_ffi_func_ptr func_ptr, const char *func_name, const char *param_name[]);
static void out_of_memory(void);
static bool rterror(struct wms_runtime *rt, const char *msg, ...);

/*
 * Intrinsics
 */

typedef bool (*intrinsic_func)(struct wms_runtime *rt, struct wms_arg_list *arg_list, struct wms_value *ret);

static bool intrinsic_print(struct wms_runtime *rt, struct wms_arg_list *arg_list, struct wms_value *ret);
static bool intrinsic_remove(struct wms_runtime *rt, struct wms_arg_list *arg_list, struct wms_value *ret);
static bool intrinsic_size(struct wms_runtime *rt, struct wms_arg_list *arg_list, struct wms_value *ret);
static bool intrinsic_isint(struct wms_runtime *rt, struct wms_arg_list *arg_list, struct wms_value *ret);
static bool intrinsic_isfloat(struct wms_runtime *rt, struct wms_arg_list *arg_list, struct wms_value *ret);
static bool intrinsic_isstr(struct wms_runtime *rt, struct wms_arg_list *arg_list, struct wms_value *ret);
static bool intrinsic_isarray(struct wms_runtime *rt, struct wms_arg_list *arg_list, struct wms_value *ret);

struct intrinsic {
	const char *name;
	intrinsic_func func;
} intrinsic[] = {
	{"print", intrinsic_print},
	{"remove", intrinsic_remove},
	{"size", intrinsic_size},
	{"isint", intrinsic_isint},
	{"isfloat", intrinsic_isfloat},
	{"isstr", intrinsic_isstr},
	{"isarray", intrinsic_isarray},
};

/*
 * AST Construction
 */

struct wms_func_list *
wms_make_func_list(
	struct wms_func_list *func_list,
	struct wms_func *func)
{
	struct wms_func *f;

	assert(func != NULL);
	assert(func->next == NULL);

	if (func_list == NULL) {
		func_list = malloc(sizeof(struct wms_func_list));
		AST_MEM_CHECK(func_list);
		memset(func_list, 0, sizeof(struct wms_func_list));
		func_list->list = func;
		ast = func_list;
		return func_list;
	}

	assert(func_list->list != NULL);
	f = func_list->list;
	while (f->next != NULL)
		f = f->next;
	f->next = func;
	return func_list;
}

struct wms_func *
wms_make_func(
	char *name,
	struct wms_param_list *param_list,
	struct wms_stmt_list *stmt_list)
{
	struct wms_func *f;

	assert(name != NULL);

	f = malloc(sizeof(struct wms_func));
	AST_MEM_CHECK(f);
	memset(f, 0, sizeof(struct wms_func));
	f->name = name;
	f->param_list = param_list;
	f->stmt_list = stmt_list;
	return f;
}

struct wms_param_list *
wms_make_param_list(
	struct wms_param_list *param_list,
	char *symbol)
{
	struct wms_param *param, *p;

	assert(symbol != NULL);

	param = malloc(sizeof(struct wms_param));
	AST_MEM_CHECK(param);
	memset(param, 0, sizeof(struct wms_param));
	param->symbol = symbol;

	if (param_list == NULL) {
		param_list = malloc(sizeof(struct wms_param_list));
		AST_MEM_CHECK(param_list);
		memset(param_list, 0, sizeof(struct wms_param_list));
		param_list->list = param;
		return param_list;
	}

	assert(param_list->list != NULL);
	p = param_list->list;
	while (p->next != NULL)
		p = p->next;
	p->next = param;
	return param_list;
}

struct wms_stmt_list *
wms_make_stmt_list(
	struct wms_stmt_list *stmt_list,
	struct wms_stmt *stmt)
{
	struct wms_stmt *s;

	assert(stmt != NULL);
	assert(stmt->next == NULL);

	if (stmt_list == NULL) {
		stmt_list = malloc(sizeof(struct wms_stmt_list));
		AST_MEM_CHECK(stmt_list);
		memset(stmt_list, 0, sizeof(struct wms_stmt_list));
		stmt_list->list = stmt;
		return stmt_list;
	}

	assert(stmt_list->list != NULL);
	s = stmt_list->list;
	while (s->next != NULL)
		s = s->next;
	s->next = stmt;
	return stmt_list;
}

void
wms_set_stmt_position(
	struct wms_stmt *stmt,
	int line)
{
	assert(stmt != NULL);

	stmt->line = line;
}

struct wms_stmt *
wms_make_stmt_with_nothing(void)
{
	struct wms_stmt *stmt;

	stmt = malloc(sizeof(struct wms_stmt));
	AST_MEM_CHECK(stmt);
	memset(stmt, 0, sizeof(struct wms_stmt));
	stmt->type.is_empty = 1;
	return stmt;
}

struct wms_stmt *
wms_make_stmt_with_expr(
	struct wms_expr *expr)
{
	struct wms_stmt *stmt;
	struct wms_expr_stmt *expr_stmt;

	stmt = malloc(sizeof(struct wms_stmt));
	AST_MEM_CHECK(stmt);
	memset(stmt, 0, sizeof(struct wms_stmt));
	stmt->type.is_expr = 1;

	expr_stmt = malloc(sizeof(struct wms_expr_stmt));
	AST_MEM_CHECK(expr_stmt);
	memset(expr_stmt, 0, sizeof(struct wms_expr_stmt));
	expr_stmt->expr = expr;

	stmt->of.expr = expr_stmt;
	return stmt;
}

struct wms_stmt *
wms_make_stmt_with_symbol_assign(
	char *symbol,
	struct wms_expr *rhs)
{
	struct wms_stmt *stmt;
	struct wms_assign_stmt *assign_stmt;

	stmt = malloc(sizeof(struct wms_stmt));
	AST_MEM_CHECK(stmt);
	memset(stmt, 0, sizeof(struct wms_stmt));
	stmt->type.is_assign = 1;

	assign_stmt = malloc(sizeof(struct wms_assign_stmt));
	AST_MEM_CHECK(assign_stmt);
	memset(assign_stmt, 0, sizeof(struct wms_assign_stmt));
	assign_stmt->type.is_var = 1;
	assign_stmt->lhs.symbol = symbol;
	assign_stmt->rhs = rhs;

	stmt->of.assign = assign_stmt;
	return stmt;
}

struct wms_stmt *
wms_make_stmt_with_array_assign(
	char *symbol,
	struct wms_expr *subscript,
	struct wms_expr *rhs)
{
	struct wms_stmt *stmt;
	struct wms_assign_stmt *assign_stmt;

	stmt = malloc(sizeof(struct wms_stmt));
	AST_MEM_CHECK(stmt);
	memset(stmt, 0, sizeof(struct wms_stmt));
	stmt->type.is_assign = 1;

	assign_stmt = malloc(sizeof(struct wms_assign_stmt));
	AST_MEM_CHECK(assign_stmt);
	memset(assign_stmt, 0, sizeof(struct wms_assign_stmt));
	assign_stmt->type.is_array = 1;
	assign_stmt->lhs.array.symbol = symbol;
	assign_stmt->lhs.array.subscript = subscript;
	assign_stmt->rhs = rhs;

	stmt->of.assign = assign_stmt;
	return stmt;
}

struct wms_stmt *
wms_make_stmt_with_if(
	struct wms_expr *cond,
	struct wms_stmt_list *stmt_list)
{
	struct wms_stmt *stmt;
	struct wms_if_stmt *if_stmt;

	stmt = malloc(sizeof(struct wms_stmt));
	AST_MEM_CHECK(stmt);
	memset(stmt, 0, sizeof(struct wms_stmt));
	stmt->type.is_if = 1;

	if_stmt = malloc(sizeof(struct wms_if_stmt));
	AST_MEM_CHECK(if_stmt);
	memset(if_stmt, 0, sizeof(struct wms_if_stmt));
	if_stmt->cond = cond;
	if_stmt->stmt_list = stmt_list;

	stmt->of._if = if_stmt;
	return stmt;
}

struct wms_stmt *
wms_make_stmt_with_elif(struct wms_expr *cond, struct wms_stmt_list *stmt_list)
{
	struct wms_stmt *stmt;
	struct wms_elif_stmt *elif_stmt;

	stmt = malloc(sizeof(struct wms_stmt));
	AST_MEM_CHECK(stmt);
	memset(stmt, 0, sizeof(struct wms_stmt));
	stmt->type.is_elif = 1;

	elif_stmt = malloc(sizeof(struct wms_elif_stmt));
	AST_MEM_CHECK(elif_stmt);
	memset(elif_stmt, 0, sizeof(struct wms_elif_stmt));
	elif_stmt->cond = cond;
	elif_stmt->stmt_list = stmt_list;

	stmt->of.elif = elif_stmt;
	return stmt;
}

struct wms_stmt *
wms_make_stmt_with_else(
	struct wms_stmt_list *stmt_list)
{
	struct wms_stmt *stmt;
	struct wms_else_stmt *else_stmt;

	stmt = malloc(sizeof(struct wms_stmt));
	AST_MEM_CHECK(stmt);
	memset(stmt, 0, sizeof(struct wms_stmt));
	stmt->type.is_else = 1;

	else_stmt = malloc(sizeof(struct wms_else_stmt));
	AST_MEM_CHECK(else_stmt);
	memset(else_stmt, 0, sizeof(struct wms_else_stmt));
	else_stmt->stmt_list = stmt_list;

	stmt->of._else = else_stmt;
	return stmt;
}

struct wms_stmt *
wms_make_stmt_with_while(
	struct wms_expr *cond,
	struct wms_stmt_list *stmt_list)
{
	struct wms_stmt *stmt;
	struct wms_while_stmt *while_stmt;

	stmt = malloc(sizeof(struct wms_stmt));
	AST_MEM_CHECK(stmt);
	memset(stmt, 0, sizeof(struct wms_stmt));
	stmt->type.is_while = 1;

	while_stmt = malloc(sizeof(struct wms_while_stmt));
	AST_MEM_CHECK(while_stmt);
	memset(while_stmt, 0, sizeof(struct wms_while_stmt));
	while_stmt->cond = cond;
	while_stmt->stmt_list = stmt_list;

	stmt->of._while = while_stmt;
	return stmt;
}

struct wms_stmt *
wms_make_stmt_with_for(
	char *key,
	char *val,
	struct wms_expr *array,
	struct wms_stmt_list *stmt_list)
{
	struct wms_stmt *stmt;
	struct wms_for_stmt *for_stmt;

	stmt = malloc(sizeof(struct wms_stmt));
	AST_MEM_CHECK(stmt);
	memset(stmt, 0, sizeof(struct wms_stmt));
	stmt->type.is_for = 1;

	for_stmt = malloc(sizeof(struct wms_for_stmt));
	AST_MEM_CHECK(for_stmt);
	memset(for_stmt, 0, sizeof(struct wms_for_stmt));
	for_stmt->is_range = false;
	for_stmt->key_symbol = key;
	for_stmt->value_symbol = val;
	for_stmt->array_expr = array;
	for_stmt->stmt_list = stmt_list;

	stmt->of._for = for_stmt;
	return stmt;
}

struct wms_stmt *
wms_make_stmt_with_for_range(
	char *val,
	int start,
	int end,
	struct wms_stmt_list *stmt_list)
{
	struct wms_stmt *stmt;
	struct wms_for_stmt *for_stmt;

	stmt = malloc(sizeof(struct wms_stmt));
	AST_MEM_CHECK(stmt);
	memset(stmt, 0, sizeof(struct wms_stmt));
	stmt->type.is_for = 1;

	for_stmt = malloc(sizeof(struct wms_for_stmt));
	AST_MEM_CHECK(for_stmt);
	memset(for_stmt, 0, sizeof(struct wms_for_stmt));
	for_stmt->is_range = true;
	for_stmt->start = start;
	for_stmt->end = end;
	for_stmt->value_symbol = val;
	for_stmt->stmt_list = stmt_list;

	stmt->of._for = for_stmt;
	return stmt;
}

struct wms_stmt *
wms_make_stmt_with_return(
	struct wms_expr *expr)
{
	struct wms_stmt *stmt;
	struct wms_return_stmt *return_stmt;

	stmt = malloc(sizeof(struct wms_stmt));
	AST_MEM_CHECK(stmt);
	memset(stmt, 0, sizeof(struct wms_stmt));
	stmt->type.is_return = 1;

	return_stmt = malloc(sizeof(struct wms_return_stmt));
	AST_MEM_CHECK(return_stmt);
	memset(return_stmt, 0, sizeof(struct wms_return_stmt));
	return_stmt->expr = expr;

	stmt->of._return = return_stmt;
	return stmt;
}

struct wms_stmt *
wms_make_stmt_with_break(void)
{
	struct wms_stmt *stmt;

	stmt = malloc(sizeof(struct wms_stmt));
	AST_MEM_CHECK(stmt);
	memset(stmt, 0, sizeof(struct wms_stmt));
	stmt->type.is_break = 1;
	return stmt;
}

struct wms_stmt *
wms_make_stmt_with_continue(void)
{
	struct wms_stmt *stmt;

	stmt = malloc(sizeof(struct wms_stmt));
	AST_MEM_CHECK(stmt);
	memset(stmt, 0, sizeof(struct wms_stmt));
	stmt->type.is_continue = 1;
	return stmt;
}

struct wms_expr *
wms_make_expr_with_term(
	struct wms_term *term)
{
	struct wms_expr *expr;

	expr = malloc(sizeof(struct wms_expr));
	AST_MEM_CHECK(expr);
	memset(expr, 0, sizeof(struct wms_expr));
	expr->type.is_term = 1;
	expr->val.term = term;
	return expr;
}

struct wms_expr *
wms_make_expr_with_gt(
	struct wms_expr *expr1,
	struct wms_expr *expr2)
{
	struct wms_expr *expr;

	expr = malloc(sizeof(struct wms_expr));
	AST_MEM_CHECK(expr);
	memset(expr, 0, sizeof(struct wms_expr));
	expr->type.is_gt = 1;
	expr->val.expr[0] = expr1;
	expr->val.expr[1] = expr2;
	return expr;
}

struct wms_expr *
wms_make_expr_with_gte(
	struct wms_expr *expr1,
	struct wms_expr *expr2)
{
	struct wms_expr *expr;

	expr = malloc(sizeof(struct wms_expr));
	AST_MEM_CHECK(expr);
	memset(expr, 0, sizeof(struct wms_expr));
	expr->type.is_gte = 1;
	expr->val.expr[0] = expr1;
	expr->val.expr[1] = expr2;
	return expr;
}

struct wms_expr *
wms_make_expr_with_lt(
	struct wms_expr *expr1,
	struct wms_expr *expr2)
{
	struct wms_expr *expr;

	expr = malloc(sizeof(struct wms_expr));
	AST_MEM_CHECK(expr);
	memset(expr, 0, sizeof(struct wms_expr));
	expr->type.is_lt = 1;
	expr->val.expr[0] = expr1;
	expr->val.expr[1] = expr2;
	return expr;
}

struct wms_expr *
wms_make_expr_with_lte(
	struct wms_expr *expr1,
	struct wms_expr *expr2)
{
	struct wms_expr *expr;

	expr = malloc(sizeof(struct wms_expr));
	AST_MEM_CHECK(expr);
	memset(expr, 0, sizeof(struct wms_expr));
	expr->type.is_lte = 1;
	expr->val.expr[0] = expr1;
	expr->val.expr[1] = expr2;
	return expr;
}

struct wms_expr *
wms_make_expr_with_eq(
	struct wms_expr *expr1,
	struct wms_expr *expr2)
{
	struct wms_expr *expr;

	expr = malloc(sizeof(struct wms_expr));
	AST_MEM_CHECK(expr);
	memset(expr, 0, sizeof(struct wms_expr));
	expr->type.is_eq = 1;
	expr->val.expr[0] = expr1;
	expr->val.expr[1] = expr2;
	return expr;
}

struct wms_expr *
wms_make_expr_with_plus(
	struct wms_expr *expr1,
	struct wms_expr *expr2)
{
	struct wms_expr *expr;

	expr = malloc(sizeof(struct wms_expr));
	AST_MEM_CHECK(expr);
	memset(expr, 0, sizeof(struct wms_expr));
	expr->type.is_plus = 1;
	expr->val.expr[0] = expr1;
	expr->val.expr[1] = expr2;
	return expr;
}

struct wms_expr *
wms_make_expr_with_minus(
	struct wms_expr *expr1,
	struct wms_expr *expr2)
{
	struct wms_expr *expr;

	expr = malloc(sizeof(struct wms_expr));
	AST_MEM_CHECK(expr);
	memset(expr, 0, sizeof(struct wms_expr));
	expr->type.is_minus = 1;
	expr->val.expr[0] = expr1;
	expr->val.expr[1] = expr2;
	return expr;
}

struct wms_expr *
wms_make_expr_with_mul(
	struct wms_expr *expr1,
	struct wms_expr *expr2)
{
	struct wms_expr *expr;

	expr = malloc(sizeof(struct wms_expr));
	AST_MEM_CHECK(expr);
	memset(expr, 0, sizeof(struct wms_expr));
	expr->type.is_mul = 1;
	expr->val.expr[0] = expr1;
	expr->val.expr[1] = expr2;
	return expr;
}

struct wms_expr *
wms_make_expr_with_div(
	struct wms_expr *expr1,
	struct wms_expr *expr2)
{
	struct wms_expr *expr;

	expr = malloc(sizeof(struct wms_expr));
	AST_MEM_CHECK(expr);
	memset(expr, 0, sizeof(struct wms_expr));
	expr->type.is_div = 1;
	expr->val.expr[0] = expr1;
	expr->val.expr[1] = expr2;
	return expr;
}

struct wms_expr *
wms_make_expr_with_and(
	struct wms_expr *expr1,
	struct wms_expr *expr2)
{
	struct wms_expr *expr;

	expr = malloc(sizeof(struct wms_expr));
	AST_MEM_CHECK(expr);
	memset(expr, 0, sizeof(struct wms_expr));
	expr->type.is_and = 1;
	expr->val.expr[0] = expr1;
	expr->val.expr[1] = expr2;
	return expr;
}

struct wms_expr *
wms_make_expr_with_or(
	struct wms_expr *expr1,
	struct wms_expr *expr2)
{
	struct wms_expr *expr;

	expr = malloc(sizeof(struct wms_expr));
	AST_MEM_CHECK(expr);
	memset(expr, 0, sizeof(struct wms_expr));
	expr->type.is_or = 1;
	expr->val.expr[0] = expr1;
	expr->val.expr[1] = expr2;
	return expr;
}

struct wms_expr *
wms_make_expr_with_neg(
	struct wms_expr *expr)
{
	struct wms_expr *e;

	e = malloc(sizeof(struct wms_expr));
	AST_MEM_CHECK(e);
	memset(e, 0, sizeof(struct wms_expr));
	e->type.is_neg = 1;
	e->val.expr[0] = expr;
	return e;
}

struct wms_expr *
wms_make_expr_with_par(
	struct wms_expr *expr)
{
	return expr;
}

struct wms_term *
wms_make_term_with_int(
	int i)
{
	struct wms_term *term;

	term = malloc(sizeof(struct wms_term));
	AST_MEM_CHECK(term);
	memset(term, 0, sizeof(struct wms_term));
	term->type.is_int = 1;
	term->val.i = i;
	return term;
}

struct wms_term *
wms_make_term_with_float(
	double f)
{
	struct wms_term *term;

	term = malloc(sizeof(struct wms_term));
	AST_MEM_CHECK(term);
	memset(term, 0, sizeof(struct wms_term));
	term->type.is_float= 1;
	term->val.f = f;
	return term;
}

struct wms_term *
wms_make_term_with_str(
	char *s)
{
	struct wms_term *term;

	term = malloc(sizeof(struct wms_term));
	AST_MEM_CHECK(term);
	memset(term, 0, sizeof(struct wms_term));
	term->type.is_str = 1;
	term->val.s = s;
	return term;
}

struct wms_term *
wms_make_term_with_symbol(
	char *symbol)
{
	struct wms_term *term;

	term = malloc(sizeof(struct wms_term));
	AST_MEM_CHECK(term);
	memset(term, 0, sizeof(struct wms_term));
	term->type.is_symbol = 1;
	term->val.symbol = symbol;
	return term;
}

struct wms_term *
wms_make_term_with_array(
	char *symbol,
	struct wms_expr *subscript)
{
	struct wms_term *term;

	term = malloc(sizeof(struct wms_term));
	AST_MEM_CHECK(term);
	memset(term, 0, sizeof(struct wms_term));
	term->type.is_array = 1;
	term->val.array.symbol = symbol;
	term->val.array.subscript = subscript;
	return term;
}

struct wms_term *
wms_make_term_with_call(
	char *func,
	struct wms_arg_list *arg_list)
{
	struct wms_term *term;

	term = malloc(sizeof(struct wms_term));
	AST_MEM_CHECK(term);
	memset(term, 0, sizeof(struct wms_term));
	term->type.is_call = 1;
	term->val.call.func = func;
	term->val.call.arg_list = arg_list;
	return term;
}

struct wms_arg_list *
wms_make_arg_list(
	struct wms_arg_list *arg_list,
	struct wms_expr *expr)
{
	struct wms_expr *e;

	assert(expr != NULL);
	assert(expr->next == NULL);

	if (arg_list == NULL) {
		arg_list = malloc(sizeof(struct wms_arg_list));
		AST_MEM_CHECK(arg_list);
		memset(arg_list, 0, sizeof(struct wms_arg_list));
		arg_list->list = expr;
		return arg_list;
	}

	assert(arg_list->list != NULL);
	e = arg_list->list;
	while (e->next != NULL)
		e = e->next;
	e->next = expr;
	return arg_list;
}

/*
 * AST Destruction
 */

static void
free_func_list(
	struct wms_func_list *func_list)
{
	assert(func_list->list != NULL);
	free_func(func_list->list);
	free(func_list);
}

static void
free_func(
	struct wms_func *func)
{
	if (func->next != NULL)
		free_func(func->next);
	
	free(func->name);
	if (func->param_list != NULL) {
		assert(func->param_list->list != NULL);
		free_param(func->param_list->list);
		free(func->param_list);
	}
	if (func->stmt_list != NULL)
		free_stmt_list(func->stmt_list);
	free(func);
}

static void
free_param(struct wms_param *param)
{
	if (param->next != NULL)
		free_param(param->next);

	free(param->symbol);
	free(param);
}

static void
free_stmt_list(
	struct wms_stmt_list *stmt_list)
{
	assert(stmt_list != NULL);

	free_stmt(stmt_list->list);
	free(stmt_list);
}

static void
free_stmt(
	struct wms_stmt *stmt)
{
	if (stmt->next != NULL)
		free_stmt(stmt->next);

	if (stmt->type.is_expr) {
		free_expr(stmt->of.expr->expr);
		free(stmt->of.expr);
	} else if (stmt->type.is_assign) {
		if (stmt->of.assign->type.is_var) {
			free(stmt->of.assign->lhs.symbol);
		} else {
			free(stmt->of.assign->lhs.array.symbol);
			free_expr(stmt->of.assign->lhs.array.subscript);
		}
		free_expr(stmt->of.assign->rhs);
		free(stmt->of.assign);
	} else if (stmt->type.is_if) {
		free_expr(stmt->of._if->cond);
		if (stmt->of._if->stmt_list != NULL)
			free_stmt_list(stmt->of._if->stmt_list);
		free(stmt->of._if);
	} else if (stmt->type.is_elif) {
		free_expr(stmt->of.elif->cond);
		if (stmt->of.elif->stmt_list != NULL)
			free_stmt_list(stmt->of.elif->stmt_list);
		free(stmt->of.elif);
	} else if (stmt->type.is_else) {
		if (stmt->of._else->stmt_list != NULL)
			free_stmt_list(stmt->of._else->stmt_list);
		free(stmt->of._else);
	} else if (stmt->type.is_while) {
		free_expr(stmt->of._while->cond);
		if (stmt->of._while->stmt_list != NULL)
			free_stmt_list(stmt->of._while->stmt_list);
		free(stmt->of._while);
	} else if (stmt->type.is_for) {
		free(stmt->of._for->key_symbol);
		free(stmt->of._for->value_symbol);
		if (stmt->of._for->array_expr != NULL)
			free_expr(stmt->of._for->array_expr);
		if (stmt->of._for->stmt_list != NULL)
			free_stmt_list(stmt->of._for->stmt_list);
		free(stmt->of._while);
	} else if (stmt->type.is_return) {
		free_expr(stmt->of._return->expr);
		free(stmt->of._return);
	}
	free(stmt);
}

static void
free_expr(
	struct wms_expr *expr)
{
	if (expr->next != NULL)
		free_expr(expr->next);

	if (expr->type.is_term) {
		free_term(expr->val.term);
	} else if (expr->type.is_lt || expr->type.is_lte ||
		   expr->type.is_gt || expr->type.is_gte ||
		   expr->type.is_lt || expr->type.is_lte || expr->type.is_eq ||
		   expr->type.is_plus || expr->type.is_minus ||
		   expr->type.is_mul || expr->type.is_div ||
		   expr->type.is_and || expr->type.is_or) {
		free_expr(expr->val.expr[0]);
		free_expr(expr->val.expr[1]);
	} else if (expr->type.is_neg) {
		free_expr(expr->val.expr[0]);
	}
	free(expr);
}

static void
free_term(
	struct wms_term *term)
{
	if (term->type.is_str) {
		free(term->val.s);
	} else if (term->type.is_symbol) {
		free(term->val.symbol);
	} else if (term->type.is_array) {
		free(term->val.array.symbol);
		free_expr(term->val.array.subscript);
	} else if (term->type.is_call) {
		free(term->val.call.func);
		if (term->val.call.arg_list != NULL) {
			free_expr(term->val.call.arg_list->list);
			free(term->val.call.arg_list);
		}
	}
	free(term);
}

/*
 * Execution
 */

struct wms_runtime *
wms_make_runtime(
	const char *script)
{
	struct wms_runtime *rt;

	/* Parse. */
	wms_yy_scan_string(script);
	if (wms_yyparse() != 0)
		return NULL;
	wms_yylex_destroy();

	/* Create runtime. */
	rt = malloc(sizeof(struct wms_runtime));
	AST_MEM_CHECK(rt);
	memset(rt, 0, sizeof(struct wms_runtime));
	rt->func_list = ast;
	ast = NULL;
	return rt;
}

int
wms_get_parse_error_line(void)
{
	return wms_parser_error_line;
}

int
wms_get_parse_error_column(void)
{
	return wms_parser_error_column;
}

int
wms_get_runtime_error_line(
	struct wms_runtime *rt)
{
	assert(rt != NULL);

	return rt->error_line;
}

const char *
wms_get_runtime_error_message(
	struct wms_runtime *rt)
{
	assert(rt != NULL);

	return rt->error_message;
}

void
wms_free_runtime(
	struct wms_runtime *rt)
{
	int i;

	assert(rt != NULL);
	assert(rt->func_list != NULL);

	free_func_list(rt->func_list);
	if (rt->frame != NULL)
		free_frame(rt, rt->frame);
	for (i = 0; i < WMS_STR_POOL; i++)
		if (rt->str_pool[i].is_used)
			free(rt->str_pool[i].s);
	if (rt->error_message != NULL)
		free(rt->error_message);
	if (rt->ffi_func_list != NULL)
		free_ffi_func(rt->ffi_func_list);
	free(rt);
}

static void
free_frame(
	struct wms_runtime *rt,
	struct wms_frame *frame)
{
	if (frame->next != NULL)
		free_frame(rt, frame->next);
	if (frame->var_list != NULL)
		free_variable(rt, frame->var_list);
	free(frame);	
}

static void
free_variable(
	struct wms_runtime *rt,
	struct wms_variable *var)
{
	if (var->next != NULL)
		free_variable(rt, var->next);
	if (var->val.type.is_str)
		decrement_str_ref(rt, var->val.val.s_index);
	else if (var->val.type.is_array)
		decrement_array_ref(rt, var->val.val.a_index);
	free(var->name);
	free(var);
}

static void
free_ffi_func(
	struct wms_ffi_func *ff)
{
	assert(ff != NULL);

	if (ff->next != NULL)
		free_ffi_func(ff->next);
	free(ff->name);
	if (ff->param_list != NULL) {
		free_param(ff->param_list->list);
		free(ff->param_list);
	}
	free(ff);
}

bool
wms_run(
	struct wms_runtime *rt)
{
	struct wms_func *f;
	struct wms_value val;

	f = rt->func_list->list;
	while (f != NULL) {
		if (strcmp(f->name, "main") == 0)
			return eval_func(rt, f, NULL, &val);
		f = f->next;
	}

	return rterror(rt, "No function named \"main\"");
}

static bool
eval_func(
	struct wms_runtime *rt,
	const struct wms_func *func,
	const struct wms_arg_list *arg_list,
	struct wms_value *result)
{
	struct wms_frame *frame;
	struct wms_value val;
	bool ret, brk, cont;

	if (func->stmt_list == NULL)
		return true;

	/* Prepare frame. */
	frame = malloc(sizeof(struct wms_frame));
	RT_MEM_CHECK(frame);
	memset(frame, 0, sizeof(struct wms_frame));
	frame->next = rt->frame;
	rt->frame = frame;
	if (!push_args(rt, func->param_list, arg_list))
		return false;

	/* Evaluate statements. */
	ret = brk = cont = false;
	if (!eval_stmt_list(rt, func->stmt_list, &val, &ret, &brk, &cont))
		return false;
	if (brk)
		return rterror(rt, "break outside loop");
	if (cont)
		return rterror(rt, "continue outside loop");

	/* Get the return variable. */
	if (!pop_return(rt, result))
		return false;

	/* Destroy frame */
	frame = rt->frame;
	rt->frame = rt->frame->next;
	frame->next = NULL;
	free_frame(rt, frame);

	return result;
}

static bool
eval_stmt_list(
	struct wms_runtime *rt,
	struct wms_stmt_list *stmt_list,
	struct wms_value *val,
	bool *ret,
	bool *brk,
	bool *cont)
{
	struct wms_stmt *stmt;

	assert(rt != NULL);
	assert(stmt_list);

	stmt = stmt_list->list;
	while (stmt != NULL) {
		if (!eval_stmt(rt, stmt, val, ret, brk, cont))
			return false;
		if (*ret || *brk || *cont)
			return true;
		stmt = stmt->next;
	}
	return true;
}

static bool
eval_stmt(
	struct wms_runtime *rt,
	const struct wms_stmt *stmt,
	struct wms_value *val,
	bool *ret,
	bool *brk,
	bool *cont)
{
	assert(rt != NULL);
	assert(stmt != NULL);
	assert(val != NULL);

	rt->cur_stmt_line = stmt->line;

	if (stmt->type.is_empty) {
		*val = value_by_int(0);
	} else if (stmt->type.is_expr) {
		if (!eval_expr_stmt(rt, stmt->of.expr, val))
			return false;
	} else if (stmt->type.is_assign) {
		if (!eval_assign_stmt(rt, stmt->of.assign, val))
			return false;
	} else if (stmt->type.is_if) {
		if (!eval_if_stmt(rt, stmt->of._if, val, ret, brk, cont))
			return false;
		rt->is_after_if = true;
		if (val->type.is_int && val->val.i == 0)
			rt->is_after_false_if = true;
		return true;
	} else if (stmt->type.is_elif) {
		if (!eval_elif_stmt(rt, stmt->of.elif, val, ret, brk, cont))
			return false;
		rt->is_after_if = true;
		if (val->type.is_int && val->val.i == 0)
			rt->is_after_false_if = true;
		return true;
	} else if (stmt->type.is_else) {
		if (!eval_else_stmt(rt, stmt->of._else, val, ret, brk, cont))
			return false;
	} else if (stmt->type.is_while) {
		if (!eval_while_stmt(rt, stmt->of._while, val, ret))
			return false;
	} else if (stmt->type.is_for) {
		if (!eval_for_stmt(rt, stmt->of._for, val, ret))
			return false;
	} else if (stmt->type.is_return) {
		if (!eval_return_stmt(rt, stmt->of._return, val))
			return false;
		*ret = true;
	} else if (stmt->type.is_break) {
		*val = value_by_int(0);
		*brk = true;
	} else if (stmt->type.is_continue) {
		*val = value_by_int(0);
		*cont = true;
	} else {
		return rterror(rt, "Invalid stmt type");
	}
	rt->is_after_if = false;
	rt->is_after_false_if = false;
	return true;
}

static bool
eval_expr_stmt(
	struct wms_runtime *rt,
	const struct wms_expr_stmt *es,
	struct wms_value *val)
{
	assert(rt != NULL);
	assert(es != NULL);
	assert(es->expr != NULL);
	assert(val != NULL);

	return eval_expr(rt, es->expr, val);
}

static bool
eval_assign_stmt(
	struct wms_runtime *rt,
	const struct wms_assign_stmt *as,
	struct wms_value *val)
{
	struct wms_value rhs_val, index_val;

	assert(rt != NULL);
	assert(as != NULL);
	assert(as->rhs != NULL);
	assert(val != NULL);

	/* Evaluate RHS. */
	if (!eval_expr(rt, as->rhs, &rhs_val))
		return false;

	if (as->type.is_var) {
		assert(as->lhs.symbol != NULL);

		/* Assign as a scalar value. */
		put_scalar_value(rt, as->lhs.symbol, rhs_val);
	} else if (as->type.is_array) {
		assert(as->lhs.array.symbol != NULL);
		assert(as->lhs.array.subscript != NULL);

		/* Evaluate subscript. */
		if (!eval_expr(rt, as->lhs.array.subscript, &index_val))
			return false;

		/* Assign as an array element. */
		put_array_elem_value(rt, as->lhs.array.symbol, index_val,
				     rhs_val);
	} else {
		return rterror(rt, "Invalid assign type.");
	}

	*val = rhs_val;
	return true;
}

static bool
eval_if_stmt(
	struct wms_runtime *rt,
	const struct wms_if_stmt *is,
	struct wms_value *val,
	bool *ret,
	bool *brk,
	bool *cont)
{
	struct wms_value cond_val;

	assert(rt != NULL);
	assert(is != NULL);
	assert(val != NULL);

	/* Evaluate condition. */
	if (!eval_expr(rt, is->cond, &cond_val))
		return false;

	/* If false: */
	if (cond_val.type.is_int && cond_val.val.i == 0) {
		/* false */
		*val = value_by_int(0);
		return true;
	}

	/* Execute stmt_list. */
	if (is->stmt_list != NULL)
		if (!eval_stmt_list(rt, is->stmt_list, val, ret, brk, cont))
			return false;

	/* true */
	*val = value_by_int(1);
	return true;
}

static bool
eval_elif_stmt(
	struct wms_runtime *rt,
	const struct wms_elif_stmt *eis,
	struct wms_value *val,
	bool *ret,
	bool *brk,
	bool *cont)
{
	struct wms_value cond_val;

	assert(rt != NULL);
	assert(eis != NULL);
	assert(val != NULL);

	if (!rt->is_after_if)
		return rterror(rt, "else before if");
	if (!rt->is_after_false_if) {
		*val = value_by_int(0);
		return true;
	}

	/* Evaluate condition. */
	if (!eval_expr(rt, eis->cond, &cond_val))
		return false;

	/* If false: */
	if (cond_val.type.is_int && cond_val.val.i == 0) {
		/* false */
		*val = value_by_int(0);
		return true;
	}

	/* Execute stmt_list. */
	if (eis->stmt_list != NULL)
		if (!eval_stmt_list(rt, eis->stmt_list, val, ret, brk, cont))
			return false;

	/* true */
	*val = value_by_int(1);
	return true;
}

static bool
eval_else_stmt(
	struct wms_runtime *rt,
	const struct wms_else_stmt *es,
	struct wms_value *val,
	bool *ret,
	bool *brk,
	bool *cont)
{
	assert(rt != NULL);
	assert(es != NULL);
	assert(val != NULL);

	if (!rt->is_after_if)
		return rterror(rt, "else before if");
	if (!rt->is_after_false_if) {
		*val = value_by_int(0);
		return true;
	}

	/* Execute stmt_list. */
	if (es->stmt_list != NULL)
		if (!eval_stmt_list(rt, es->stmt_list, val, ret, brk, cont))
			return false;

	*val = value_by_int(0);
	return true;
}

static bool
eval_while_stmt(
	struct wms_runtime *rt,
	const struct wms_while_stmt *ws,
	struct wms_value *val,
	bool *ret)
{
	struct wms_value cond_val;
	bool brk, cont;

	assert(rt != NULL);
	assert(ws != NULL);
	assert(val != NULL);

	while (1) {
		/* Evaluate condition. */
		if (!eval_expr(rt, ws->cond, &cond_val))
			return false;

		/* If false: */
		if (cond_val.type.is_int && cond_val.val.i == 0)
			break;

		/* Execute stmt_list. */
		brk = cont = false;
		if (ws->stmt_list != NULL)
			if (!eval_stmt_list(rt, ws->stmt_list, val, ret, &brk, &cont))
				return false;
		if (*ret || brk)
			break;
		if (cont)
			cont = false;
	}
	*val = value_by_int(0);
	return true;
}

static bool
eval_for_stmt(
	struct wms_runtime *rt,
	const struct wms_for_stmt *fs,
	struct wms_value *val,
	bool *ret)
{
	struct wms_value array_val;
	struct wms_array_elem *elem;
	int i;
	bool brk, cont;

	assert(rt != NULL);
	assert(fs != NULL);
	assert(fs->value_symbol != NULL);
	assert(val != NULL);

	/* for range style */
	if (fs->is_range) {
		assert(fs->array_expr == NULL);

		for (i = fs->start; i <= fs->end; i++) {
			/* Set the value. */
			put_scalar_value(rt, fs->value_symbol, value_by_int(i));
		
			/* Execute stmt_list. */
			brk = cont = false;
			if (fs->stmt_list != NULL)
				if (!eval_stmt_list(rt, fs->stmt_list, val, ret, &brk, &cont))
					return false;
			if (*ret || brk)
				break;
			if (cont)
				cont = false;
		}
		*val = value_by_int(0);
		return true;
	}

	/* foreach style */
	assert(fs->array_expr != NULL);

	/* Evaluate the array term. */
	if (!eval_expr(rt, fs->array_expr, &array_val))
		return false;
	if (!array_val.type.is_array)
		return rterror(rt, "Expected an array");

	/* Traverse. */
	elem = get_array_head(rt, array_val.val.a_index)->next;
	while (elem != NULL) {
		/* Set the key. */
		if (fs->key_symbol != NULL)
			put_scalar_value(rt, fs->key_symbol, elem->index);

		/* Set the value. */
		put_scalar_value(rt, fs->value_symbol, elem->val);
		
		/* Execute stmt_list. */
		brk = cont = false;
		if (fs->stmt_list != NULL)
			if (!eval_stmt_list(rt, fs->stmt_list, val, ret, &brk, &cont))
				return false;
		if (*ret || brk)
			break;
		if (cont)
			cont = false;

		elem = elem->next;
	}
	*val = value_by_int(0);
	return true;
}

static bool
eval_return_stmt(
	struct wms_runtime *rt,
	const struct wms_return_stmt *rs,
	struct wms_value *val)
{
	assert(rt != NULL);
	assert(rs != NULL);
	assert(rs->expr != NULL);

	/* Evaluate the return value. */
	if (!eval_expr(rt, rs->expr, val))
		return false;

	/* Push the __return variable. */
	if (!put_scalar_value(rt, "__return", *val))
		return false;

	return true;
}

static bool
eval_expr(
	struct wms_runtime *rt,
	struct wms_expr *expr,
	struct wms_value *val)
{
	struct wms_value val1, val2;

	assert(rt != NULL);
	assert(expr != NULL);
	assert(val != NULL);

	if (expr->type.is_term) {
		return eval_term(rt, expr->val.term, val);
	} else if (expr->type.is_lt) {
		if (!eval_expr(rt, expr->val.expr[0], &val1))
			return false;
		if (!eval_expr(rt, expr->val.expr[1], &val2))
			return false;
		return calc_lt(rt, val1, val2, val);
	} else if (expr->type.is_lte) {
		if (!eval_expr(rt, expr->val.expr[0], &val1))
			return false;
		if (!eval_expr(rt, expr->val.expr[1], &val2))
			return false;
		return calc_lte(rt, val1, val2, val);
	} else if (expr->type.is_gt) {
		if (!eval_expr(rt, expr->val.expr[0], &val1))
			return false;
		if (!eval_expr(rt, expr->val.expr[1], &val2))
			return false;
		return calc_gt(rt, val1, val2, val);
	} else if (expr->type.is_gte) {
		if (!eval_expr(rt, expr->val.expr[0], &val1))
			return false;
		if (!eval_expr(rt, expr->val.expr[1], &val2))
			return false;
		return calc_gte(rt, val1, val2, val);
	} else if (expr->type.is_eq) {
		if (!eval_expr(rt, expr->val.expr[0], &val1))
			return false;
		if (!eval_expr(rt, expr->val.expr[1], &val2))
			return false;
		return calc_eq(rt, val1, val2, val);
	} else if (expr->type.is_plus) {
		if (!eval_expr(rt, expr->val.expr[0], &val1))
			return false;
		if (!eval_expr(rt, expr->val.expr[1], &val2))
			return false;
		return calc_plus(rt, val1, val2, val);
	} else if (expr->type.is_minus) {
		if (!eval_expr(rt, expr->val.expr[0], &val1))
			return false;
		if (!eval_expr(rt, expr->val.expr[1], &val2))
			return false;
		return calc_minus(rt, val1, val2, val);
	} else if (expr->type.is_mul) {
		if (!eval_expr(rt, expr->val.expr[0], &val1))
			return false;
		if (!eval_expr(rt, expr->val.expr[1], &val2))
			return false;
		return calc_mul(rt, val1, val2, val);
	} else if (expr->type.is_div) {
		if (!eval_expr(rt, expr->val.expr[0], &val1))
			return false;
		if (!eval_expr(rt, expr->val.expr[1], &val2))
			return false;
		return calc_div(rt, val1, val2, val);
	} else if (expr->type.is_and) {
		if (!eval_expr(rt, expr->val.expr[0], &val1))
			return false;
		if (!eval_expr(rt, expr->val.expr[1], &val2))
			return false;
		return calc_and(rt, val1, val2, val);
	} else if (expr->type.is_or) {
		if (!eval_expr(rt, expr->val.expr[0], &val1))
			return false;
		if (!eval_expr(rt, expr->val.expr[1], &val2))
			return false;
		return calc_or(rt, val1, val2, val);
	} else if (expr->type.is_neg) {
		if (!eval_expr(rt, expr->val.expr[0], &val1))
			return false;
		return calc_neg(rt, val1, val);
	}
	assert(NEVER_COME_HERE);
	return false;
}

static bool
calc_lt(
	struct wms_runtime *rt,
	struct wms_value val1,
	struct wms_value val2,
	struct wms_value *result)
{
	assert(result != NULL);

	if (val1.type.is_int) {
		if (val2.type.is_int) {
			*result = value_by_int(val1.val.i < val2.val.i ? 1 : 0);
			return true;
		} else if (val2.type.is_float) {
			*result = value_by_int((double)val1.val.i < val2.val.f ? 1 : 0);
			return true;
		}
	} else if (val1.type.is_float) {
		if (val2.type.is_int) {
			*result = value_by_int(val1.val.f < (double)val2.val.i ? 1 : 0);
			return true;
		} else if (val2.type.is_float) {
			*result = value_by_int(val1.val.f < val2.val.f ? 1 : 0);
			return true;
		}
	}
	return rterror(rt, "Type error ('<' operator)");
}

static bool
calc_lte(
	struct wms_runtime *rt,
	struct wms_value val1,
	struct wms_value val2,
	struct wms_value *result)
{
	assert(result != NULL);

	if (val1.type.is_int) {
		if (val2.type.is_int) {
			*result = value_by_int(val1.val.i <= val2.val.i ? 1 : 0);
			return true;
		} else if (val2.type.is_float) {
			*result = value_by_int((double)val1.val.i <= val2.val.f ? 1 : 0);
			return true;
		}
	} else if (val1.type.is_float) {
		if (val2.type.is_int) {
			*result = value_by_int(val1.val.f <= (double)val2.val.i ? 1 : 0);
			return true;
		} else if (val2.type.is_float) {
			*result = value_by_int(val1.val.f <= val2.val.f ? 1 : 0);
			return true;
		}
	}
	return rterror(rt, "Type error ('<=' operator)");
}

static bool
calc_gt(
	struct wms_runtime *rt,
	struct wms_value val1,
	struct wms_value val2,
	struct wms_value *result)
{
	assert(result != NULL);

	if (val1.type.is_int) {
		if (val2.type.is_int) {
			*result = value_by_int(val1.val.i > val2.val.i ? 1 : 0);
			return true;
		} else if (val2.type.is_float) {
			*result = value_by_int((double)val1.val.i > val2.val.f ? 1 : 0);
			return true;
		}
	} else if (val1.type.is_float) {
		if (val2.type.is_int) {
			*result = value_by_int(val1.val.f > (double)val2.val.i ? 1 : 0);
			return true;
		} else if (val2.type.is_float) {
			*result = value_by_int(val1.val.f > val2.val.f ? 1 : 0);
			return true;
		}
	}
	return rterror(rt, "Type error ('>' operator)");
}

static bool
calc_gte(
	struct wms_runtime *rt,
	struct wms_value val1,
	struct wms_value val2,
	struct wms_value *result)
{
	assert(result != NULL);

	if (val1.type.is_int) {
		if (val2.type.is_int) {
			*result = value_by_int(val1.val.i >= val2.val.i ? 1 : 0);
			return true;
		} else if (val2.type.is_float) {
			*result = value_by_int((double)val1.val.i >= val2.val.f ? 1 : 0);
			return true;
		}
	} else if (val1.type.is_float) {
		if (val2.type.is_int) {
			*result = value_by_int(val1.val.f >= (double)val2.val.i ? 1 : 0);
			return true;
		} else if (val2.type.is_float) {
			*result = value_by_int(val1.val.f >= val2.val.f ? 1 : 0);
			return true;
		}
	}
	return rterror(rt, "Type error ('>=' operator)");
}

static bool
calc_eq(
	struct wms_runtime *rt,
	struct wms_value val1,
	struct wms_value val2,
	struct wms_value *result)
{
	assert(result != NULL);

	if (val1.type.is_int) {
		if (val2.type.is_int) {
			*result = value_by_int(val1.val.i == val2.val.i ? 1 : 0);
			return true;
		} else if (val2.type.is_float) {
			*result = value_by_int((double)val1.val.i == val2.val.f ? 1 : 0);
			return true;
		}
	} else if (val1.type.is_float) {
		if (val2.type.is_int) {
			*result = value_by_int(val1.val.f == (double)val2.val.i ? 1 : 0);
			return true;
		} else if (val2.type.is_float) {
			*result = value_by_int(val1.val.f == val2.val.f ? 1 : 0);
			return true;
		}
	} else if (val1.type.is_str) {
		if (val2.type.is_str) {
			*result = value_by_int(strcmp(get_str(rt, val1.val.s_index), get_str(rt, val2.val.s_index)));
			return true;
		}
	}
	return rterror(rt, "Type error ('==' operator)");
}

static bool
calc_plus(
	struct wms_runtime *rt,
	struct wms_value val1,
	struct wms_value val2,
	struct wms_value *result)
{
	assert(result != NULL);

	if (val1.type.is_int) {
		if (val2.type.is_int) {
			*result = value_by_int(val1.val.i + val2.val.i);
			return true;
		} else if (val2.type.is_float) {
			*result = value_by_float((double)val1.val.i +
						 val2.val.f);
			return true;
		} else if (val2.type.is_str) {
			*result = value_by_int(
					val1.val.i +
					atoi(rt->str_pool[val2.val.s_index].s));
			reset_str_return_value_ref(rt, val2.val.s_index);
			return true;
		} else if (val2.type.is_array) {
			return rterror(rt, "Type error (int + array)");
		}
		assert(NEVER_COME_HERE);
		return false;
	} else if (val1.type.is_float) {
		if (val2.type.is_int) {
			*result = value_by_float(val1.val.f +
						 (double)val2.val.i);
			return true;
		} else if (val2.type.is_float) {
			*result = value_by_float(val1.val.f + val2.val.f);
			return true;
		} else if (val2.type.is_str) {
			*result = value_by_float(
					val1.val.f +
					atof(rt->str_pool[val2.val.s_index].s));
			reset_str_return_value_ref(rt, val2.val.s_index);
			return true;
		} else if (val2.type.is_array) {
			return rterror(rt, "Type error (int + array)");
		}
		assert(NEVER_COME_HERE);
		return false;
	} else if (val1.type.is_str) {
		reset_str_return_value_ref(rt, val1.val.s_index);
		if (val2.type.is_int) {
			return calc_str_plus_int(rt, val1, val2, result);
		} else if (val2.type.is_float) {
			return calc_str_plus_float(rt, val1, val2, result);
		} else if (val2.type.is_str) {
			reset_str_return_value_ref(rt, val2.val.s_index);
			return calc_str_plus_str(rt, val1, val2, result);
		} else if (val2.type.is_array) {
			return rterror(rt, "Type error (str + array)");
		}
		assert(NEVER_COME_HERE);
		return false;
	} else if (val1.type.is_array) {
		return rterror(rt, "Type error (array + any)");
	}
	assert(NEVER_COME_HERE);
	return false;
}

static bool
calc_str_plus_int(
	struct wms_runtime *rt,
	struct wms_value val1,
	struct wms_value val2,
	struct wms_value *result)
{
	const char *s1;
	char s2[100];
	char *tmp;
	size_t len;

	assert(rt != NULL);
	assert(val1.type.is_str);
	assert(val2.type.is_int);

	s1 = get_str(rt, val1.val.s_index);
	snprintf(s2, sizeof(s2), "%d", val2.val.i);

	len = strlen(s1) + strlen(s2);
	tmp = malloc(len + 1);
	RT_MEM_CHECK(tmp);
	strcpy(tmp, s1);
	strcat(tmp, s2);
	if (!value_by_str(rt, result, tmp))
		return false;
	free(tmp);
	return true;
}

static bool
calc_str_plus_float(
	struct wms_runtime *rt,
	struct wms_value val1,
	struct wms_value val2,
	struct wms_value *result)
{
	const char *s1;
	char s2[100];
	char *tmp;
	size_t len;

	assert(rt != NULL);
	assert(val1.type.is_str);
	assert(val2.type.is_float);

	s1 = get_str(rt, val1.val.s_index);
	snprintf(s2, sizeof(s2), "%f", val2.val.f);

	len = strlen(s1) + strlen(s2);
	tmp = malloc(len + 1);
	RT_MEM_CHECK(tmp);
	strcpy(tmp, s1);
	strcat(tmp, s2);
	if (!value_by_str(rt, result, tmp))
		return false;
	free(tmp);
	return true;
}

static bool
calc_str_plus_str(
	struct wms_runtime *rt,
	struct wms_value val1,
	struct wms_value val2,
	struct wms_value *result)
{
	const char *s1, *s2;
	char *tmp;
	size_t len;

	assert(rt != NULL);
	assert(val1.type.is_str);
	assert(val2.type.is_str);

	s1 = get_str(rt, val1.val.s_index);
	s2 = get_str(rt, val2.val.s_index);

	len = strlen(s1) + strlen(s2);
	tmp = malloc(len + 1);
	RT_MEM_CHECK(tmp);
	strcpy(tmp, s1);
	strcat(tmp, s2);
	if (!value_by_str(rt, result, tmp))
		return false;
	free(tmp);
	return true;
}

static bool
calc_minus(
	struct wms_runtime *rt,
	struct wms_value val1,
	struct wms_value val2,
	struct wms_value *result)
{
	assert(result != NULL);

	if (val1.type.is_int) {
		if (val2.type.is_int) {
			*result = value_by_int(val1.val.i - val2.val.i);
			return true;
		} else if (val2.type.is_float) {
			*result = value_by_float((double)val1.val.i -
						 val2.val.f);
			return true;
		}
	} else if (val1.type.is_float) {
		if (val2.type.is_int) {
			*result = value_by_float(val1.val.f -
						 (double)val2.val.i);
			return true;
		} else if (val2.type.is_float) {
			*result = value_by_float(val1.val.f - val2.val.f);
			return true;
		}
	}
	return rterror(rt, "Type error (minus operator)");
}

static bool
calc_mul(
	struct wms_runtime *rt,
	struct wms_value val1,
	struct wms_value val2,
	struct wms_value *result)
{
	assert(result != NULL);

	if (val1.type.is_int) {
		if (val2.type.is_int) {
			*result = value_by_int(val1.val.i * val2.val.i);
			return true;
		} else if (val2.type.is_float) {
			*result = value_by_float((double)val1.val.i *
						 val2.val.f);
			return true;
		}
	} else if (val1.type.is_float) {
		if (val2.type.is_int) {
			*result = value_by_float(val1.val.f *
						 (double)val2.val.i);
			return true;
		} else if (val2.type.is_float) {
			*result = value_by_float(val1.val.f * val2.val.f);
			return true;
		}
	}
	return rterror(rt, "Type error (multiply operator)");
}

static bool
calc_div(
	struct wms_runtime *rt,
	struct wms_value val1,
	struct wms_value val2,
	struct wms_value *result)
{
	assert(result != NULL);

	/* Check division by zero. */
	if ((val2.type.is_int && val2.val.i == 0) ||
	    (val2.type.is_float && val2.val.f == 0))
		return rterror(rt, "Division by zero");

	if (val1.type.is_int) {
		if (val2.type.is_int) {
			*result = value_by_int(val1.val.i / val2.val.i);
			return true;
		} else if (val2.type.is_float) {
			*result = value_by_int((int)((double)val1.val.i /
						     val2.val.f));
			return true;
		}
	} else if (val1.type.is_float) {
		if (val2.type.is_int) {
			*result = value_by_float(val1.val.f /
						 (double)val2.val.i);
			return true;
		} else if (val2.type.is_float) {
			*result = value_by_float(val1.val.f / val2.val.f);
			return true;
		}
	}
	return rterror(rt, "Type error (divide operator)");
}

static bool
calc_and(
	struct wms_runtime *rt,
	struct wms_value val1,
	struct wms_value val2,
	struct wms_value *result)
{
	assert(rt != NULL);
	assert(result != NULL);

	if (!val1.type.is_int || !val2.type.is_int)
		return rterror(rt, "Non integer expression specified for && operator");

	if (val1.val.i && val2.val.i) {
		*result = value_by_int(1);
		return true;
	}

	*result = value_by_int(0);
	return true;
}

static bool
calc_or(
	struct wms_runtime *rt,
	struct wms_value val1,
	struct wms_value val2,
	struct wms_value *result)
{
	assert(rt != NULL);
	assert(result != NULL);

	if (!val1.type.is_int || !val2.type.is_int)
		return rterror(rt, "Non integer expression specified for || operator");

	if (val1.val.i || val2.val.i) {
		*result = value_by_int(1);
		return true;
	}

	*result = value_by_int(0);
	return true;
}

static bool
calc_neg(
	struct wms_runtime *rt,
	struct wms_value val,
	struct wms_value *result)
{
	assert(result != NULL);

	if (val.type.is_int) {
		*result = value_by_int(-val.val.i);
		return true;
	} else if (val.type.is_float) {
		*result = value_by_float(-val.val.f);
		return true;
	}
	return rterror(rt, "Type error (negative operator)");
}

static bool
eval_term(
	struct wms_runtime *rt,
	struct wms_term *term,
	struct wms_value *val)
{
	struct wms_value index;

	assert(rt != NULL);
	assert(term != NULL);
	assert(val != NULL);

	if (term->type.is_int) {
		*val = value_by_int(term->val.i);
		return true;
	} else if (term->type.is_float) {
		*val = value_by_float(term->val.f);
		return true;
	} else if (term->type.is_str) {
		return value_by_str(rt, val, term->val.s);
	} else if (term->type.is_symbol) {
		return get_scalar_value(rt, term->val.symbol, val);
	} else if (term->type.is_array) {
		if (!eval_expr(rt, term->val.array.subscript, &index))
			return false;
		return get_array_elem_value(rt, term->val.array.symbol,
					    index, val);
	} else if (term->type.is_call) {
		return do_call(rt, term, val);
	}
	return rterror(rt, "Invalid term type");
}

static bool
do_call(
	struct wms_runtime *rt,
	struct wms_term *term,
	struct wms_value *val)
{
	struct wms_func *func;
	struct wms_ffi_func *ffi_func;
	struct wms_variable *var;
	const char *func_name;
	int i;

	assert(rt != NULL);
	assert(rt->func_list != NULL);
	assert(term != NULL);
	assert(val != NULL);

	/* Search for string variables. (Replace the function name.) */
	var = rt->frame->var_list;
	func_name = term->val.call.func;
	while (var != NULL) {
		if (var->val.type.is_str &&
		    strcmp(var->name, term->val.call.func) == 0) {
			func_name = get_str(rt, var->val.val.s_index);
			break;
		}
		var = var->next;
	}

	/* Search for intrinsics. */
	for (i = 0; i < (int)(sizeof(intrinsic) / sizeof(struct intrinsic)); i++)
		if (strcmp(intrinsic[i].name, func_name) == 0)
			return intrinsic[i].func(rt, term->val.call.arg_list, val);

	/* Search for FFI functions. */
	ffi_func = rt->ffi_func_list;
	while (ffi_func != NULL) {
		if (strcmp(ffi_func->name, func_name) == 0)
			return call_ffi_func(rt, ffi_func, term->val.call.arg_list, val);
		ffi_func = ffi_func->next;
	}

	/* Search for normal functions. */
	func = rt->func_list->list;
	while (func != NULL) {
		if (strcmp(func->name, func_name) == 0)
			return eval_func(rt, func, term->val.call.arg_list, val);
		func = func->next;
	}

	return rterror(rt, "Function %s is not defined", term->val.call.func);
}

static bool
call_ffi_func(
	struct wms_runtime *rt,
	struct wms_ffi_func *ffi_func,
	struct wms_arg_list *arg_list,
	struct wms_value *result)
{
	struct wms_frame *frame;

	/* Prepare frame. */
	frame = malloc(sizeof(struct wms_frame));
	RT_MEM_CHECK(frame);
	memset(frame, 0, sizeof(struct wms_frame));
	frame->next = rt->frame;
	rt->frame = frame;
	if (!push_args(rt, ffi_func->param_list, arg_list))
		return false;

	/* Call. */
	if (!ffi_func->func(rt))
		return rterror(rt, "FFI function returned error");

	/* Get the return variable. */
	if (!pop_return(rt, result))
		return false;

	/* Destroy frame */
	frame = rt->frame;
	rt->frame = rt->frame->next;
	frame->next = NULL;
	free_frame(rt, frame);

	return true;
}

static struct wms_value
value_by_int(
	int i)
{
	struct wms_value val;

	memset(&val, 0, sizeof(struct wms_value));
	val.type.is_int = 1;
	val.val.i = i;
	return val;
}

static struct wms_value
value_by_float(
	double f)
{
	struct wms_value val;

	memset(&val, 0, sizeof(struct wms_value));
	val.type.is_float = 1;
	val.val.f = f;
	return val;
}

static bool
value_by_str(
	struct wms_runtime *rt,
	struct wms_value *val,
	const char *s)
{
	int s_index;

	assert(rt != NULL);
	assert(val != NULL);

	s_index = alloc_str_index(rt);
	if (s_index == -1)
		return false;

	memset(val, 0, sizeof(struct wms_value));
	val->type.is_str = 1;
	val->val.s_index = s_index;

	rt->str_pool[s_index].s = strdup(s);
	RT_MEM_CHECK(rt->str_pool[s_index].s);

	return true;
}

static int
alloc_str_index(
	struct wms_runtime *rt)
{
	int i;
	for (i = 0; i < WMS_STR_POOL; i++) {
		if (!rt->str_pool[i].is_used) {
			rt->str_pool[i].is_used = true;
			rt->str_pool[i].ref = 1;
			return i;
		}
	}
	rterror(rt, "Too many strings");
	return -1;
}

static int
alloc_elem_index(
	 struct wms_runtime *rt,
	 bool is_head)
{
	int i;

	assert(rt != NULL);

	for (i = 0; i < WMS_ELEM_POOL; i++) {
		if (!rt->elem_pool[i].is_used) {
			rt->elem_pool[i].is_used = true;
			rt->elem_pool[i].is_head = is_head;
			rt->elem_pool[i].ref = is_head ? 1 : -1;
			return i;
		}
	}
	rterror(rt, "Too many array elements");
	return -1;
}

static bool
push_args(
	struct wms_runtime *rt,
	const struct wms_param_list *param_list,
	const struct wms_arg_list *arg_list)
{
	struct wms_expr *expr;
	struct wms_param *param;
	struct wms_frame *frame;
	struct wms_value val;

	assert(rt != NULL);
	assert(rt->frame != NULL);

	if (arg_list == NULL)
		return true;
	if (param_list == NULL)
		return true;

	expr = arg_list->list;
	param = param_list->list;
	while (expr != NULL && param != NULL) {
		/* We'll temporarily use the caller's frame to evaluation. */
		frame = rt->frame;
		rt->frame = rt->frame->next;

		/* Evaluate. */
		if (!eval_expr(rt, expr, &val))
			return false;

		/* Use the callee's stack to push the variable. */
		rt->frame = frame;

		/* Push the variable. */
		put_scalar_value(rt, param->symbol, val);

		expr = expr->next;
		param = param->next;
	}
	return true;
}

static bool
pop_return(
	struct wms_runtime *rt,
	struct wms_value *val)
{
	struct wms_variable *var;

	assert(rt != NULL);
	assert(rt->frame != NULL);
	assert(val != NULL);

	var = rt->frame->var_list;
	while (var != NULL) {
		if (strcmp(var->name, "__return") == 0) {
			*val = var->val;

			/* Set a temporary reference flag. */
			if (var->val.type.is_str)
				set_str_return_value_ref(rt, var->val.val.s_index);
			else if (var->val.type.is_array)
				set_array_return_value_ref(rt, var->val.val.a_index);
			return true;
		}
		var = var->next;
	}
	*val = value_by_int(0);
	return true;
}

/* Scalar or array reference assignment. */
static bool
put_scalar_value(
	struct wms_runtime *rt,
	const char *symbol,
	struct wms_value val)
{
	struct wms_variable *var;

	assert(rt != NULL);
	assert(rt->frame != NULL);
	assert(symbol != NULL);

	if (val.type.is_array)
		increment_array_ref(rt, val.val.a_index);
	else if (val.type.is_str)
		increment_str_ref(rt, val.val.s_index);

	var = rt->frame->var_list;
	while (var != NULL) {
		if (strcmp(var->name, symbol) == 0) {
			if (var->val.type.is_array)
				decrement_array_ref(rt, var->val.val.a_index);
			else if (var->val.type.is_str)
				decrement_str_ref(rt, var->val.val.s_index);
			var->val = val;
			return true;
		}
		var = var->next;
	}

	var = malloc(sizeof(struct wms_variable));
	RT_MEM_CHECK(var);
	memset(var, 0, sizeof(struct wms_variable));
	var->name = strdup(symbol);
	RT_MEM_CHECK(var->name);
	var->val = val;
	var->next = rt->frame->var_list;
	rt->frame->var_list = var;
	return true;
}

/* Array element assignment. */
static bool
put_array_elem_value(
	struct wms_runtime *rt,
	const char *symbol,
	struct wms_value index,
	struct wms_value val)
{
	struct wms_variable *var;

	assert(rt != NULL);
	assert(rt->frame != NULL);
	assert(symbol != NULL);

	/* Find a variable. */
	var = rt->frame->var_list;
	while (var != NULL) {
		if (strcmp(var->name, symbol) == 0)
			break;
		var = var->next;
	}
	if (var == NULL) {
		/* If not found, create new variable. */
		var = malloc(sizeof(struct wms_variable));
		RT_MEM_CHECK(var);
		memset(var, 0, sizeof(struct wms_variable));
		var->name = strdup(symbol);
		RT_MEM_CHECK(var->name);
		var->val.type.is_array = 1;
		var->val.val.a_index = alloc_elem_index(rt, true);
		if (var->val.val.a_index == -1)
			return false;
		var->next = rt->frame->var_list;
		rt->frame->var_list = var;
	} else if (!var->val.type.is_array) {
		/* If found and it's not an array, recreate variable. */
		if (var->val.type.is_str)
			decrement_str_ref(rt, var->val.val.s_index);
		memset(&var->val, 0, sizeof(struct wms_value));
		var->val.type.is_array = 1;
		var->val.val.a_index = alloc_elem_index(rt, true);
		if (var->val.val.a_index == -1)
			return false;
	}

	return put_array_elem_value_helper(rt, var->val, index, val);
}

static bool
put_array_elem_value_helper(
	struct wms_runtime *rt,
	struct wms_value array,
	struct wms_value index,
	struct wms_value val)
{
	struct wms_array_elem *elem, *head;
	int new_index;
	
	assert(rt != NULL);
	assert(array.type.is_array);
	assert(!index.type.is_array);

	/* Increment reference count. */
	if (val.type.is_str)
		increment_str_ref(rt, val.val.s_index);
	if (val.type.is_array)
		increment_array_ref(rt, val.val.a_index);

	/* Find an element. */
	head = get_array_head(rt, array.val.a_index);
	elem = head->next;
	while (elem != NULL) {
		if (compare_values(rt, elem->index, index) == 0) {
			if (elem->val.type.is_str)
				decrement_array_ref(rt, elem->val.val.a_index);
			else if (elem->val.type.is_array)
				decrement_array_ref(rt, elem->val.val.a_index);

			/*  Assign an array element. */
			elem->val = val;
			return true;
		}
		elem = elem->next;
	}

	/* Create an array element. */
	new_index = alloc_elem_index(rt, false);
	if (new_index == -1)
		return false;
	elem = &rt->elem_pool[new_index];
	elem->index = index;
	elem->val = val;
	elem->next = head->next;
	head->next = elem;
	return true;
}


static bool
get_scalar_value(
	struct wms_runtime *rt,
	const char *symbol,
	struct wms_value *val)
{
	struct wms_variable *var;

	assert(rt != NULL);
	assert(rt->frame != NULL);

	var = rt->frame->var_list;
	while (var != NULL) {
		if (strcmp(var->name, symbol) == 0) {
			*val = var->val;
			return true;
		}
		var = var->next;
	}
	return rterror(rt, "Variable %s not found", symbol);
}

static bool
get_scalar_value_pointer(
	struct wms_runtime *rt,
	const char *symbol,
	struct wms_value **val)
{
	struct wms_variable *var;

	assert(rt != NULL);
	assert(rt->frame != NULL);

	var = rt->frame->var_list;
	while (var != NULL) {
		if (strcmp(var->name, symbol) == 0) {
			*val = &var->val;
			return true;
		}
		var = var->next;
	}
	return rterror(rt, "Variable %s not found", symbol);
}

static bool
get_array_elem_value(
	struct wms_runtime *rt,
	const char *symbol,
	struct wms_value index,
	struct wms_value *val)
{
	struct wms_variable *var;
	struct wms_array_elem *elem;

	assert(rt != NULL);
	assert(rt->frame != NULL);
	assert(symbol != NULL);

	var = rt->frame->var_list;
	while (var != NULL) {
		if (strcmp(var->name, symbol) == 0) {
			if (!var->val.type.is_array)
				return rterror(rt, "Variable %s is not an array", symbol);
			break;
		}
		var = var->next;
	}
	if (var == NULL)
		return rterror(rt, "Variable %s not defined", symbol);

	if (index.type.is_array)
		return rterror(rt, "Array is passed as index");

	elem = get_array_head(rt, var->val.val.a_index)->next;
	while (elem != NULL) {
		if (compare_values(rt, elem->index, index) == 0) {
			*val = elem->val;
			return true;
		}
		elem = elem->next;
	}
	return rterror(rt, "Index %s is not defied for array variable %s",
		       index_to_string(rt, index), symbol);
}

static int
compare_values(
	struct wms_runtime *rt,
	struct wms_value val1,
	struct wms_value val2)
{
	assert(rt != NULL);

	if (val1.type.is_int && val2.type.is_int) {
		return val1.val.i - val2.val.i;
	} else if (val1.type.is_str && val2.type.is_str) {
		return strcmp(rt->str_pool[val1.val.s_index].s,
			      rt->str_pool[val2.val.s_index].s);
	}
	return -1;
}

static const char *
index_to_string(
	struct wms_runtime *rt,
	struct wms_value index)
{
	static char buf[1024];

	if (index.type.is_int) {
		snprintf(buf, sizeof(buf), "%d", index.val.i);
		return buf;
	} else if (index.type.is_str) {
		return rt->str_pool[index.val.s_index].s;
	}
	
	assert(NEVER_COME_HERE);
	return NULL;
}

static struct wms_array_elem *
get_array_head(
	struct wms_runtime *rt,
	int a_index)
{
	/* Empty array. */
	if (a_index == -1)
		return NULL;

	/* Return top element. */
	return &rt->elem_pool[a_index];
}

static const char *
get_str(
	struct wms_runtime *rt,
	int s_index)
{
	assert(rt->str_pool[s_index].is_used);
	return rt->str_pool[s_index].s;
}

static void
increment_str_ref(
	struct wms_runtime *rt,
	int s_index)
{
	assert(rt->str_pool[s_index].is_used);
	assert(rt->str_pool[s_index].ref > 0);
	rt->str_pool[s_index].ref++;
	rt->str_pool[s_index].ret_ref = false;
}

static void
decrement_str_ref(
	struct wms_runtime *rt,
	int s_index)
{
	assert(rt->str_pool[s_index].is_used);
	assert(rt->str_pool[s_index].ref > 0);
	rt->str_pool[s_index].ref--;
	if (rt->str_pool[s_index].ref > 0)
		return;
	if (rt->str_pool[s_index].ret_ref)
		return;

	/* GC */
	free(rt->str_pool[s_index].s);
	memset(&rt->str_pool[s_index], 0, sizeof(struct wms_str_elem));
}

static void
set_str_return_value_ref(
	struct wms_runtime *rt,
	int s_index)
{
	assert(rt->str_pool[s_index].is_used);
	rt->str_pool[s_index].ret_ref = true;
}

static void
reset_str_return_value_ref(
	struct wms_runtime *rt,
	int s_index)
{
	assert(rt->str_pool[s_index].is_used);
	rt->str_pool[s_index].ret_ref = false;
}

static void
increment_array_ref(
	struct wms_runtime *rt,
	int a_index)
{
	assert(rt->elem_pool[a_index].is_used);

	/* ref==0 is possible when the array is referenced by a return value. */
	assert(rt->elem_pool[a_index].ref >= 0);

	rt->elem_pool[a_index].ref++;
	rt->elem_pool[a_index].ret_ref = false;
}

static void
decrement_array_ref(
	struct wms_runtime *rt,
	int a_index)
{
	struct wms_array_elem *elem, *next;

	assert(rt->elem_pool[a_index].is_used);
	assert(rt->elem_pool[a_index].ref > 0);
	rt->elem_pool[a_index].ref--;
	if (rt->elem_pool[a_index].ref > 0)
		return;
	if (rt->elem_pool[a_index].ret_ref)
		return;

	/* GC */
	elem = get_array_head(rt, a_index);
	while (elem != NULL) {
		next = elem->next;
		if (!elem->is_head) {
			if (elem->val.type.is_str)
				decrement_str_ref(rt, elem->val.val.s_index);
			else if (elem->val.type.is_array)
				decrement_array_ref(rt, elem->val.val.a_index);
		}
		memset(elem, 0, sizeof(struct wms_array_elem));
		elem = next;
	}
}

static void
set_array_return_value_ref(
	struct wms_runtime *rt,
	int a_index)
{
	assert(rt != NULL);
	assert(rt->elem_pool[a_index].is_used);

	rt->elem_pool[a_index].ret_ref = true;
}

/*
 * Intrinsics
 */

static bool
intrinsic_print(
	struct wms_runtime *rt,
	struct wms_arg_list *arg_list,
	struct wms_value *val)
{
	struct wms_expr *arg;

	assert(rt != NULL);
	assert(val != NULL);

	if (arg_list == NULL) {
		*val = value_by_int(0);
		return true;
	}

	arg = arg_list->list;
	while (arg != NULL) {
		if (!eval_expr(rt, arg, val))
			return false;
		print_value(rt, *val, false);
		arg = arg->next;
	}
	wms_printf("\n");
	return true;
}

static void
print_value(
	struct wms_runtime *rt,
	struct wms_value val,
	bool quote)
{
	struct wms_array_elem *elem;

	if (val.type.is_int) {
		wms_printf("%d", val.val.i);
	} else if (val.type.is_float) {
		wms_printf("%f", val.val.f);
	} else if (val.type.is_str) {
		if (quote)
			wms_printf("\"%s\"", get_str(rt, val.val.s_index));
		else
			wms_printf("%s", get_str(rt, val.val.s_index));
	} else if (val.type.is_array) {
		wms_printf("[");
		elem = get_array_head(rt, val.val.a_index)->next;
		while (elem != NULL) {
			print_value(rt, elem->index, true);
			wms_printf(":");
			print_value(rt, elem->val, true);
			elem = elem->next;
			if (elem != NULL)
				wms_printf(", ");
		}
		wms_printf("]");
	}
}

static bool
intrinsic_remove(
	struct wms_runtime *rt,
	struct wms_arg_list *arg_list,
	struct wms_value *ret)
{
	struct wms_expr *array_expr, *key_expr;
	struct wms_value array_val, key_val;
	struct wms_array_elem *elem, *prev_elem;

	assert(rt != NULL);
	assert(ret != NULL);

	/* Check if the first argument exists. */
	if (arg_list == NULL)
		return rterror(rt, "No arguments for remove()");
	assert(arg_list->list != NULL);
	array_expr = arg_list->list;

	/* Check if the second argument exists */
	if (arg_list->list->next == NULL)
		return rterror(rt, "Too few arguments for remove()");
	key_expr = arg_list->list->next;

	/* Evaluate array expr. */
	if (!eval_expr(rt, array_expr, &array_val))
		return false;
	if (!array_val.type.is_array)
		return rterror(rt, "Non-array value is specified for remove()");

	/* Evaluate key expr. */
	if (!eval_expr(rt, key_expr, &key_val))
		return false;
	if (!(key_val.type.is_int || key_val.type.is_float ||
	      key_val.type.is_str))
		return rterror(rt, "Non-key value is specified for remove()");

	/* Remove the key-value from the array. */
	prev_elem = get_array_head(rt, array_val.val.a_index);
	elem = prev_elem->next;
	while (elem != NULL) {
		if (compare_values(rt, elem->index, key_val) == 0) {
			prev_elem->next = elem->next;
			elem->is_used = false;
			break;
		}
		prev_elem = elem;
		elem = elem->next;
	}
	if (elem == NULL)
		return rterror(rt, "No key found for array");
	*ret = value_by_int(0);
	return true;
}

static bool
intrinsic_size(struct wms_runtime *rt, struct wms_arg_list *arg_list, struct wms_value *ret)
{
	struct wms_expr *array_expr;
	struct wms_value array_val;
	struct wms_array_elem *elem;
	int count;

	assert(rt != NULL);
	assert(ret != NULL);

	/* Check if the first argument exists. */
	if (arg_list == NULL)
		return rterror(rt, "No arguments for size()");
	assert(arg_list->list != NULL);
	array_expr = arg_list->list;

	/* Evaluate array expr. */
	if (!eval_expr(rt, array_expr, &array_val))
		return false;
	if (!array_val.type.is_array)
		return rterror(rt, "Non-array value is specified for size()");

	/* Remove the key-value from the array. */
	count = 0;
	elem = get_array_head(rt, array_val.val.a_index)->next;
	while (elem != NULL) {
		elem = elem->next;
		count++;
	}
	*ret = value_by_int(count);
	return true;
}

static bool
intrinsic_isint(struct wms_runtime *rt, struct wms_arg_list *arg_list, struct wms_value *ret)
{
	struct wms_expr *expr;
	struct wms_value val;

	assert(rt != NULL);
	assert(ret != NULL);

	/* Check if the first argument exists. */
	if (arg_list == NULL)
		return rterror(rt, "No arguments for size()");
	assert(arg_list->list != NULL);
	expr = arg_list->list;

	/* Evaluate the expr. */
	if (!eval_expr(rt, expr, &val))
		return false;

	/* Check type type. */
	if (!val.type.is_int)
		*ret = value_by_int(1);
	else
		*ret = value_by_int(0);
	return true;
}

static bool
intrinsic_isfloat(struct wms_runtime *rt, struct wms_arg_list *arg_list, struct wms_value *ret)
{
	struct wms_expr *expr;
	struct wms_value val;

	assert(rt != NULL);
	assert(ret != NULL);

	/* Check if the first argument exists. */
	if (arg_list == NULL)
		return rterror(rt, "No arguments for size()");
	assert(arg_list->list != NULL);
	expr = arg_list->list;

	/* Evaluate the expr. */
	if (!eval_expr(rt, expr, &val))
		return false;

	/* Check type type. */
	if (!val.type.is_float)
		*ret = value_by_int(1);
	else
		*ret = value_by_int(0);
	return true;
}

static bool
intrinsic_isstr(struct wms_runtime *rt, struct wms_arg_list *arg_list, struct wms_value *ret)
{
	struct wms_expr *expr;
	struct wms_value val;

	assert(rt != NULL);
	assert(ret != NULL);

	/* Check if the first argument exists. */
	if (arg_list == NULL)
		return rterror(rt, "No arguments for size()");
	assert(arg_list->list != NULL);
	expr = arg_list->list;

	/* Evaluate the expr. */
	if (!eval_expr(rt, expr, &val))
		return false;

	/* Check type type. */
	if (!val.type.is_str)
		*ret = value_by_int(1);
	else
		*ret = value_by_int(0);
	return true;
}

static bool
intrinsic_isarray(struct wms_runtime *rt, struct wms_arg_list *arg_list, struct wms_value *ret)
{
	struct wms_expr *expr;
	struct wms_value val;

	assert(rt != NULL);
	assert(ret != NULL);

	/* Check if the first argument exists. */
	if (arg_list == NULL)
		return rterror(rt, "No arguments for size()");
	assert(arg_list->list != NULL);
	expr = arg_list->list;

	/* Evaluate the expr. */
	if (!eval_expr(rt, expr, &val))
		return false;

	/* Check type type. */
	if (!val.type.is_array)
		*ret = value_by_int(1);
	else
		*ret = value_by_int(0);
	return true;
}

/*
 * Foreign Function Support
 */

bool
wms_register_ffi_func_tbl(
	struct wms_runtime *rt,
	struct wms_ffi_func_tbl *ffi_func_tbl,
	int count)
{
	int i;

	for (i = 0; i < count; i++) {
		if (!register_ffi_func(rt,
				       ffi_func_tbl[i].func_ptr,
				       ffi_func_tbl[i].func_name,
				       ffi_func_tbl[i].param_name))
			return false;
	}
	return true;
}

static bool
register_ffi_func(
	struct wms_runtime *rt,
	wms_ffi_func_ptr func_ptr,
	const char *func_name,
	const char *param_name[])
{
	struct wms_ffi_func *ff;
	struct wms_param *param, *prev_param;
	int i;
	assert(rt != NULL);

	ff = malloc(sizeof(struct wms_ffi_func));
	RT_MEM_CHECK(ff);
	ff->func = func_ptr;
	ff->name = strdup(func_name);
	RT_MEM_CHECK(ff->name);
	if (param_name[0] != NULL) {
		ff->param_list = malloc(sizeof(struct wms_param_list));
		RT_MEM_CHECK(ff->param_list);
		memset(ff->param_list, 0, sizeof(struct wms_param_list));

		i = 0;
		prev_param = NULL;
		while (param_name[i] != NULL) {
			param = malloc(sizeof(struct wms_param));
			RT_MEM_CHECK(param);
			memset(param, 0, sizeof(struct wms_param));
			param->symbol = strdup(param_name[i]);
			RT_MEM_CHECK(param->symbol);
			if (prev_param == NULL) {
				ff->param_list->list = param;
				prev_param = param;
			} else {
				prev_param->next = param;
				prev_param = param;
			}
			i++;
		}
	}
	ff->next = rt->ffi_func_list;
	rt->ffi_func_list = ff;
	return true;
}

bool
wms_get_var_value(
	struct wms_runtime *rt,
	const char *symbol,
	struct wms_value **ret)
{
	struct wms_variable *var;

	assert(rt != NULL);
	assert(symbol != NULL);
	assert(ret != NULL);

	/* Search for the variable. */
	var = rt->frame->var_list;
	while (var != NULL) {
		if (strcmp(var->name, symbol) == 0) {
			/* Found. */
			*ret = &var->val;
			return true;
		}
		var = var->next;
	}

	/* Not found. */
	return false;
}

bool
wms_is_int(
	struct wms_runtime *rt,
	struct wms_value *val)
{
	(void)rt;
	assert(val != NULL);
	return val->type.is_int;
}

bool
wms_is_float(
	struct wms_runtime *rt,
	struct wms_value *val)
{
	(void)rt;
	assert(val != NULL);
	return val->type.is_float;
}

bool
wms_is_str(
	struct wms_runtime *rt,
	struct wms_value *val)
{
	(void)rt;
	assert(val != NULL);
	return val->type.is_str;
}

bool
wms_is_array(
	struct wms_runtime *rt,
	struct wms_value *val)
{
	(void)rt;
	assert(val != NULL);
	return val->type.is_array;
}

bool
wms_get_int_value(
	struct wms_runtime *rt,
	struct wms_value *val,
	int *ret)
{
	assert(rt != NULL);
	assert(val != NULL);
	assert(val->type.is_int);
	assert(ret != NULL);
	
	*ret = val->val.i;
	return true;
}

bool
wms_get_float_value(
	struct wms_runtime *rt,
	struct wms_value *val,
	double *ret)
{
	assert(rt != NULL);
	assert(val != NULL);
	assert(val->type.is_float);
	assert(ret != NULL);
	
	*ret = val->val.f;
	return true;
}

bool
wms_get_str_value(
	struct wms_runtime *rt,
	struct wms_value *val,
	const char **ret)
{
	assert(rt != NULL);
	assert(val != NULL);
	assert(val->type.is_str);
	assert(ret != NULL);
	
	*ret = get_str(rt, val->val.s_index);
	return true;
}

struct wms_array_elem *
wms_get_first_array_elem(
	struct wms_runtime *rt,
	struct wms_value *array)
{
	assert(rt != NULL);
	assert(array != NULL);
	assert(array->type.is_array);

	return get_array_head(rt, array->val.a_index)->next;
}

struct wms_array_elem *
wms_get_next_array_elem(
	struct wms_runtime *rt,
	struct wms_array_elem *prev)
{
	assert(rt != NULL);
	assert(prev != NULL);

	return prev->next;
}

bool
wms_make_int_var(
	struct wms_runtime *rt,
	const char *symbol,
	int val,
	struct wms_value **ret)
{
	assert(rt != NULL);
	assert(symbol != NULL);

	if (!put_scalar_value(rt, symbol, value_by_int(val)))
		return false;
	if (ret != NULL)
		if (!get_scalar_value_pointer(rt, symbol, ret))
			return false;
	return true;
}

bool
wms_make_float_var(
	struct wms_runtime *rt,
	const char *symbol,
	double val,
	struct wms_value **ret)
{
	assert(rt != NULL);
	assert(symbol != NULL);

	if (!put_scalar_value(rt, symbol, value_by_float(val)))
		return false;
	if (ret != NULL)
		if (!get_scalar_value_pointer(rt, symbol, ret))
			return false;
	return true;
}

bool
wms_make_str_var(
	struct wms_runtime *rt,
	const char *symbol,
	const char *val,
	struct wms_value **ret)
{
	struct wms_value v;

	assert(rt != NULL);
	assert(symbol != NULL);
	assert(val != NULL);

	if (!value_by_str(rt, &v, val))
		return false;
	if (!put_scalar_value(rt, symbol, v))
		return false;
	if (ret != NULL)
		if (!get_scalar_value_pointer(rt, symbol, ret))
			return false;
	return true;
}

bool
wms_make_array_var(
	struct wms_runtime *rt,
	const char *symbol,
	struct wms_value **ret)
{
	struct wms_value array;
	struct wms_variable *var;

	assert(rt != NULL);
	assert(rt->frame != NULL);
	assert(symbol != NULL);
	assert(ret != NULL);

	/* Create an empty array. */
	memset(&array, 0, sizeof(struct wms_value));
	array.type.is_array = 1;
	array.val.a_index = alloc_elem_index(rt, true);
	if (array.val.a_index == -1)
		return false;
	increment_array_ref(rt, array.val.a_index);

	/* Find an existing variable. */
	var = rt->frame->var_list;
	while (var != NULL) {
		assert(var->name != NULL);
		if (strcmp(var->name, symbol) == 0) {
			/* Found. */
			if (var->val.type.is_str)
				decrement_str_ref(rt, var->val.val.s_index);
			else if (var->val.type.is_array)
				decrement_array_ref(rt, var->val.val.a_index);
			var->val = array;
			*ret = &var->val;
			return true;
		}
		var = var->next;
	}

	/* If not found, create a new variable. */
	var = malloc(sizeof(struct wms_variable));
	RT_MEM_CHECK(var);
	memset(var, 0, sizeof(struct wms_variable));
	var->name = strdup(symbol);
	RT_MEM_CHECK(var->name);
	var->val = array;
	var->next = rt->frame->var_list;
	rt->frame->var_list = var;
	*ret = &var->val;
	return true;
}

bool
wms_get_array_elem(
	struct wms_runtime *rt,
	struct wms_value *array,
	struct wms_value *index,
	struct wms_value **ret)
{
	struct wms_array_elem *elem;

	assert(rt != NULL);
	assert(rt->frame != NULL);
	assert(array != NULL);
	assert(index != NULL);
	assert(!index->type.is_array);
	assert(ret != NULL);

	elem = get_array_head(rt, array->val.a_index)->next;
	while (elem != NULL) {
		if (compare_values(rt, elem->index, *index) == 0) {
			*ret = &elem->val;
			return true;
		}
		elem = elem->next;
	}
	return false;
}

bool
wms_get_array_elem_by_int_for_int(
	struct wms_runtime *rt,
	struct wms_value *array,
	int index,
	int *ret)
{
	struct wms_value v_index, *v_ret_ptr;

	assert(rt != NULL);
	assert(array != NULL);
	assert(ret != NULL);

	v_index = value_by_int(index);
	if (!wms_get_array_elem(rt, array, &v_index, &v_ret_ptr))
		return false;
	if (!wms_get_int_value(rt, v_ret_ptr, ret))
		return false;
	return true;
}

bool
wms_get_array_elem_by_int_for_float(
	struct wms_runtime *rt,
	struct wms_value *array,
	int index,
	double *ret)
{
	struct wms_value v_index, *v_ret_ptr;

	assert(rt != NULL);
	assert(array != NULL);
	assert(ret != NULL);

	v_index = value_by_int(index);
	if (!wms_get_array_elem(rt, array, &v_index, &v_ret_ptr))
		return false;
	if (!wms_get_float_value(rt, v_ret_ptr, ret))
		return false;
	return true;
}

bool
wms_get_array_elem_by_int_for_str(
	struct wms_runtime *rt,
	struct wms_value *array,
	int index,
	const char **ret)
{
	struct wms_value v_index, *v_ret_ptr;

	assert(rt != NULL);
	assert(array != NULL);
	assert(ret != NULL);

	v_index = value_by_int(index);
	if (!wms_get_array_elem(rt, array, &v_index, &v_ret_ptr))
		return false;
	if (!wms_get_str_value(rt, v_ret_ptr, ret))
		return false;
	return true;
}

bool
wms_get_array_elem_by_int_for_array(
	struct wms_runtime *rt,
	struct wms_value *array,
	int index,
	struct wms_value **ret)
{
	struct wms_value v_index;

	assert(rt != NULL);
	assert(array != NULL);
	assert(ret != NULL);

	v_index = value_by_int(index);
	if (!wms_get_array_elem(rt, array, &v_index, ret))
		return false;
	return true;
}

bool
wms_get_array_elem_by_float_for_int(
	struct wms_runtime *rt,
	struct wms_value *array,
	double index,
	int *ret)
{
	struct wms_value v_index, *v_ret_ptr;

	assert(rt != NULL);
	assert(array != NULL);
	assert(ret != NULL);

	v_index = value_by_float(index);
	if (!wms_get_array_elem(rt, array, &v_index, &v_ret_ptr))
		return false;
	if (!wms_get_int_value(rt, v_ret_ptr, ret))
		return false;
	return true;
}

bool
wms_get_array_elem_by_float_for_float(
	struct wms_runtime *rt,
	struct wms_value *array,
	double index,
	double *ret)
{
	struct wms_value v_index, *v_ret_ptr;

	assert(rt != NULL);
	assert(array != NULL);
	assert(ret != NULL);

	v_index = value_by_float(index);
	if (!wms_get_array_elem(rt, array, &v_index, &v_ret_ptr))
		return false;
	if (!wms_get_float_value(rt, v_ret_ptr, ret))
		return false;
	return true;
}

bool
wms_get_array_elem_by_float_for_str(
	struct wms_runtime *rt,
	struct wms_value *array,
	double index,
	const char **ret)
{
	struct wms_value v_index, *v_ret_ptr;

	assert(rt != NULL);
	assert(array != NULL);
	assert(ret != NULL);

	v_index = value_by_float(index);
	if (!wms_get_array_elem(rt, array, &v_index, &v_ret_ptr))
		return false;
	if (!wms_get_str_value(rt, v_ret_ptr, ret))
		return false;
	return true;
}

bool
wms_get_array_elem_by_float_for_array(
	struct wms_runtime *rt,
	struct wms_value *array,
	double index,
	struct wms_value **ret)
{
	struct wms_value v_index;

	assert(rt != NULL);
	assert(array != NULL);
	assert(ret != NULL);

	v_index = value_by_float(index);
	if (!wms_get_array_elem(rt, array, &v_index, ret))
		return false;
	return true;
}

bool
wms_get_array_elem_by_str_for_int(
	struct wms_runtime *rt,
	struct wms_value *array,
	const char *index,
	int *ret)
{
	struct wms_value v_index, *v_ret_ptr;

	assert(rt != NULL);
	assert(array != NULL);
	assert(ret != NULL);

	if (!value_by_str(rt, &v_index, index))
		return false;
	if (!wms_get_array_elem(rt, array, &v_index, &v_ret_ptr))
		return false;
	if (!wms_get_int_value(rt, v_ret_ptr, ret))
		return false;
	return true;
}

bool
wms_get_array_elem_by_str_for_float(
	struct wms_runtime *rt,
	struct wms_value *array,
	const char *index,
	double *ret)
{
	struct wms_value v_index, *v_ret_ptr;

	assert(rt != NULL);
	assert(array != NULL);
	assert(ret != NULL);

	if (!value_by_str(rt, &v_index, index))
		return false;
	if (!wms_get_array_elem(rt, array, &v_index, &v_ret_ptr))
		return false;
	if (!wms_get_float_value(rt, v_ret_ptr, ret))
		return false;
	return true;
}

bool
wms_get_array_elem_by_str_for_str(
	struct wms_runtime *rt,
	struct wms_value *array,
	const char *index,
	const char **ret)
{
	struct wms_value v_index, *v_ret_ptr;

	assert(rt != NULL);
	assert(array != NULL);
	assert(ret != NULL);

	if (!value_by_str(rt, &v_index, index))
		return false;
	if (!wms_get_array_elem(rt, array, &v_index, &v_ret_ptr))
		return false;
	if (!wms_get_str_value(rt, v_ret_ptr, ret))
		return false;
	return true;
}

bool
wms_get_array_elem_by_str_for_array(
	struct wms_runtime *rt,
	struct wms_value *array,
	const char *index,
	struct wms_value **ret)
{
	struct wms_value v_index;

	assert(rt != NULL);
	assert(array != NULL);
	assert(ret != NULL);

	if (!value_by_str(rt, &v_index, index))
		return false;
	if (!wms_get_array_elem(rt, array, &v_index, ret))
		return false;
	return true;
}

bool
wms_set_array_elem(
	struct wms_runtime *rt,
	struct wms_value *array,
	struct wms_value *index,
	struct wms_value *val)
{
	assert(rt != NULL);
	assert(rt->frame != NULL);
	assert(array != NULL);
	assert(array->type.is_array);
	assert(index != NULL);
	assert(!index->type.is_array);

	return put_array_elem_value_helper(rt, *array, *index, *val);
}

bool
wms_set_array_elem_by_int_for_int(
	struct wms_runtime *rt,
	struct wms_value *array,
	int index,
	int val)
{
	struct wms_value v_index, v_val;

	v_index = value_by_int(index);
	v_val = value_by_int(val);
	return wms_set_array_elem(rt, array, &v_index, &v_val);
}

bool
wms_set_array_elem_by_int_for_float(
	struct wms_runtime *rt,
	struct wms_value *array,
	int index,
	double val)
{
	struct wms_value v_index, v_val;

	v_index = value_by_int(index);
	v_val = value_by_float(val);
	return wms_set_array_elem(rt, array, &v_index, &v_val);
}

bool
wms_set_array_elem_by_int_for_str(
	struct wms_runtime *rt,
	struct wms_value *array,
	int index,
	const char *val)
{
	struct wms_value v_index, v_val;

	v_index = value_by_int(index);
	if (!value_by_str(rt, &v_val, val))
		return false;
	return wms_set_array_elem(rt, array, &v_index, &v_val);
}

bool
wms_set_array_elem_by_float_for_int(
	struct wms_runtime *rt,
	struct wms_value *array,
	double index,
	int val)
{
	struct wms_value v_index, v_val;

	v_index = value_by_float(index);
	v_val = value_by_int(val);
	return wms_set_array_elem(rt, array, &v_index, &v_val);
}

bool
wms_set_array_elem_by_float_for_float(
	struct wms_runtime *rt,
	struct wms_value *array,
	double index,
	double val)
{
	struct wms_value v_index, v_val;

	v_index = value_by_float(index);
	v_val = value_by_float(val);
	return wms_set_array_elem(rt, array, &v_index, &v_val);
}

bool
wms_set_array_elem_by_float_for_str(
	struct wms_runtime *rt,
	struct wms_value *array,
	double index,
	const char *val)
{
	struct wms_value v_index, v_val;

	v_index = value_by_float(index);
	if (!value_by_str(rt, &v_val, val))
		return false;
	return wms_set_array_elem(rt, array, &v_index, &v_val);
}

bool
wms_set_array_elem_by_str_for_int(
	struct wms_runtime *rt,
	struct wms_value *array,
	const char *index,
	int val)
{
	struct wms_value v_index, v_val;

	if (!value_by_str(rt, &v_index, index))
		return false;
	v_val = value_by_int(val);
	return wms_set_array_elem(rt, array, &v_index, &v_val);
}

bool
wms_set_array_elem_by_str_for_float(
	struct wms_runtime *rt,
	struct wms_value *array,
	const char *index,
	double val)
{
	struct wms_value v_index, v_val;

	if (!value_by_str(rt, &v_index, index))
		return false;
	v_val = value_by_float(val);
	return wms_set_array_elem(rt, array, &v_index, &v_val);
}

bool
wms_set_array_elem_by_str_for_str(
	struct wms_runtime *rt,
	struct wms_value *array,
	const char *index,
	const char *val)
{
	struct wms_value v_index, v_val;

	if (!value_by_str(rt, &v_index, index))
		return false;
	if (!value_by_str(rt, &v_val, val))
		return false;
	return wms_set_array_elem(rt, array, &v_index, &v_val);
}

/*
 * Helper
 */

static void
out_of_memory(void)
{
	wms_printf("Out of memory.\n");
}

static bool 
rterror(
	struct wms_runtime *rt,
	const char *msg,
	...)
{
	char buf[1024];
	va_list ap;

	va_start(ap, msg);
	vsnprintf(buf, sizeof(buf), msg, ap);
	va_end(ap);

	rt->error_line = rt->cur_stmt_line;
	rt->error_message = strdup(buf);

	/* Always return false. */
	return false;
}
