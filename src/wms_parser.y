%{
/*
 * Watermelon Script
 * Copyright (c) 2022, Keiichi Tabata. All rights reserved.
 */
#include <stdio.h>
#include "wms_core.h"

#undef DEBUG
#ifdef DEBUG
static void _debug(const char *s);
#define debug(s) _debug(s)
#else
#define debug(s)
#endif

extern int wms_parser_error_line;
extern int wms_parser_error_column;

int wms_yylex(void);
void wms_yyerror(const char *s);
%}

%union {
	int ival;
	double fval;
	char *sval;

	struct wms_func_list *func_list;
	struct wms_func *func;
	struct wms_param_list *param_list;
	struct wms_stmt_list *stmt_list;
	struct wms_stmt *stmt;
	struct wms_expr *expr;
	struct wms_term *term;
	struct wms_arg_list *arg_list;
}

%token <sval> TOKEN_SYMBOL TOKEN_STR
%token <ival> TOKEN_INT
%token <fval> TOKEN_FLOAT
%token TOKEN_FUNC TOKEN_PLUS TOKEN_MINUS TOKEN_MUL TOKEN_DIV TOKEN_ASSIGN
%token TOKEN_LPAR TOKEN_RPAR TOKEN_LBLK TOKEN_RBLK TOKEN_LARR TOKEN_RARR
%token TOKEN_SEMICOLON TOKEN_COMMA TOKEN_IF TOKEN_ELSE TOKEN_WHILE TOKEN_FOR
%token TOKEN_IN TOKEN_DOTDOT TOKEN_GT TOKEN_GTE TOKEN_LT TOKEN_LTE TOKEN_EQ
%token TOKEN_RETURN TOKEN_BREAK TOKEN_CONTINUE TOKEN_AND TOKEN_OR

%type <func_list> func_list;
%type <func> func;
%type <param_list> param_list;
%type <stmt_list> stmt_list;
%type <stmt> stmt;
%type <stmt> empty_stmt;
%type <stmt> expr_stmt;
%type <stmt> assign_stmt;
%type <stmt> if_stmt;
%type <stmt> elif_stmt;
%type <stmt> else_stmt;
%type <stmt> while_stmt;
%type <stmt> for_stmt;
%type <stmt> return_stmt;
%type <stmt> break_stmt;
%type <stmt> continue_stmt;
%type <expr> expr;
%type <term> term;
%type <arg_list> arg_list;

%left TOKEN_OR
%left TOKEN_AND
%left TOKEN_LT
%left TOKEN_LTE
%left TOKEN_GT
%left TOKEN_GTE
%left TOKEN_EQ
%left TOKEN_PLUS
%left TOKEN_MINUS
%left TOKEN_MUL
%left TOKEN_DIV

%locations

%initial-action {
	wms_yylloc.last_line = yylloc.first_line = 0;
	wms_yylloc.last_column = yylloc.first_column = 0;
}

%%
func_list	: func
		{
			$$ = wms_make_func_list(NULL, $1);
			debug("single func func_list");
		}
		| func_list func
		{
			$$ = wms_make_func_list($1, $2);
			debug("multiple func func_list");
		}
		;
func		: TOKEN_FUNC TOKEN_SYMBOL TOKEN_LPAR param_list TOKEN_RPAR TOKEN_LBLK stmt_list TOKEN_RBLK
		{
			$$ = wms_make_func($2, $4, $7);
			debug("param_list and stmt_list function");
		}
		| TOKEN_FUNC TOKEN_SYMBOL TOKEN_LPAR param_list TOKEN_RPAR TOKEN_LBLK TOKEN_RBLK
		{
			$$ = wms_make_func($2, $4, NULL);
			debug("param_list function");
		}
		| TOKEN_FUNC TOKEN_SYMBOL TOKEN_LPAR TOKEN_RPAR TOKEN_LBLK stmt_list TOKEN_RBLK
		{
			$$ = wms_make_func($2, NULL, $6);
			debug("stmt_list function");
		}
		| TOKEN_FUNC TOKEN_SYMBOL TOKEN_LPAR TOKEN_RPAR TOKEN_LBLK TOKEN_RBLK
		{
			$$ = wms_make_func($2, NULL, NULL);
			debug("empty function");
		}
		;
param_list	: TOKEN_SYMBOL
		{
			$$ = wms_make_param_list(NULL, $1);
			debug("single param param_list");
		}
		| param_list TOKEN_COMMA TOKEN_SYMBOL
		{
			$$ = wms_make_param_list($1, $3);
			debug("multiple params");
		}
		;
stmt_list	: stmt
		{
			$$ = wms_make_stmt_list(NULL, $1);
			debug("single stmt stmt_list");
		}
		| stmt_list stmt
		{
			$$ = wms_make_stmt_list($1, $2);
			debug("multiple stmt stmt_list");
		}
		;
stmt		: empty_stmt
		{
			$$ = $1;
			wms_set_stmt_position($1, wms_yylloc.first_line + 1);
			debug("stmt");
		}
		| expr_stmt
		{
			$$ = $1;
			wms_set_stmt_position($1, wms_yylloc.first_line + 1);
			debug("stmt");
		}
		| assign_stmt
		{
			$$ = $1;
			wms_set_stmt_position($1, wms_yylloc.first_line + 1);
			debug("stmt");
		}
		| if_stmt
		{
			$$ = $1;
			wms_set_stmt_position($1, wms_yylloc.first_line + 1);
			debug("stmt");
		}
		| elif_stmt
		{
			$$ = $1;
			wms_set_stmt_position($1, wms_yylloc.first_line + 1);
			debug("stmt");
		}
		| else_stmt
		{
			$$ = $1;
			wms_set_stmt_position($1, wms_yylloc.first_line + 1);
			debug("stmt");
		}
		| while_stmt
		{
			$$ = $1;
			wms_set_stmt_position($1, wms_yylloc.first_line + 1);
			debug("stmt");
		}
		| for_stmt
		{
			$$ = $1;
			wms_set_stmt_position($1, wms_yylloc.first_line + 1);
			debug("stmt");
		}
		| return_stmt
		{
			$$ = $1;
			wms_set_stmt_position($1, wms_yylloc.first_line + 1);
			debug("stmt");
		}
		;
		| break_stmt
		{
			$$ = $1;
			wms_set_stmt_position($1, wms_yylloc.first_line + 1);
			debug("stmt");
		}
		;
		| continue_stmt
		{
			$$ = $1;
			wms_set_stmt_position($1, wms_yylloc.first_line + 1);
			debug("stmt");
		}
		;
empty_stmt	: TOKEN_SEMICOLON
		{
			$$ = wms_make_stmt_with_nothing();
			debug("empty stmt");
		}
		;
expr_stmt	: expr TOKEN_SEMICOLON
		{
			$$ = wms_make_stmt_with_expr($1);
			debug("expr stmt");
		}
		;
assign_stmt	: TOKEN_SYMBOL TOKEN_ASSIGN expr TOKEN_SEMICOLON
		{
			$$ = wms_make_stmt_with_symbol_assign($1, $3);
			debug("symbol assign stmt");
		}
		| TOKEN_SYMBOL TOKEN_LARR expr TOKEN_RARR TOKEN_ASSIGN expr TOKEN_SEMICOLON
		{
			$$ = wms_make_stmt_with_array_assign($1, $3, $6);
			debug("array assign stmt");
		}
		;
if_stmt		: TOKEN_IF TOKEN_LPAR expr TOKEN_RPAR TOKEN_LBLK stmt_list TOKEN_RBLK
		{
			$$ = wms_make_stmt_with_if($3, $6);
			debug("if { stmt_list } stmt");
		}
		| TOKEN_IF TOKEN_LPAR expr TOKEN_RPAR TOKEN_LBLK TOKEN_RBLK
		{
			$$ = wms_make_stmt_with_if($3, NULL);
			debug("if {} stmt");
		}
		;
elif_stmt	: TOKEN_ELSE TOKEN_IF TOKEN_LPAR expr TOKEN_RPAR TOKEN_LBLK stmt_list TOKEN_RBLK
		{
			$$ = wms_make_stmt_with_elif($4, $7);
			debug("elif { stmt_list } stmt");
		}
		| TOKEN_ELSE TOKEN_IF TOKEN_LPAR expr TOKEN_RPAR TOKEN_LBLK TOKEN_RBLK
		{
			$$ = wms_make_stmt_with_elif($4, NULL);
			debug("elif {} stmt");
		}
		;
else_stmt	: TOKEN_ELSE TOKEN_LBLK stmt_list TOKEN_RBLK
		{
			$$ = wms_make_stmt_with_else($3);
			debug("else { stmt_list } stmt");
		}
		| TOKEN_ELSE TOKEN_LBLK TOKEN_RBLK
		{
			$$ = wms_make_stmt_with_else(NULL);
			debug("else { } stmt");
		}
		;
while_stmt	: TOKEN_WHILE TOKEN_LPAR expr TOKEN_RPAR TOKEN_LBLK stmt_list TOKEN_RBLK
		{
			$$ = wms_make_stmt_with_while($3, $6);
			debug("while { stmt_list } stmt");
		}
		| TOKEN_WHILE TOKEN_LPAR expr TOKEN_RPAR TOKEN_LBLK TOKEN_RBLK
		{
			$$ = wms_make_stmt_with_while($3, NULL);
			debug("while { } stmt");
		}
		;
for_stmt	: TOKEN_FOR TOKEN_LPAR TOKEN_SYMBOL TOKEN_COMMA TOKEN_SYMBOL TOKEN_IN expr TOKEN_RPAR TOKEN_LBLK stmt_list TOKEN_RBLK
		{
			$$ = wms_make_stmt_with_for($3, $5, $7, $10);
			debug("for(k, v in array) { stmt_list } stmt");
		}
		| TOKEN_FOR TOKEN_LPAR TOKEN_SYMBOL TOKEN_COMMA TOKEN_SYMBOL TOKEN_IN expr TOKEN_RPAR TOKEN_LBLK TOKEN_RBLK
		{
			$$ = wms_make_stmt_with_for($3, $5, $7, NULL);
			debug("for(k, v in array) { } stmt");
		}
		| TOKEN_FOR TOKEN_LPAR TOKEN_SYMBOL TOKEN_IN expr TOKEN_RPAR TOKEN_LBLK stmt_list TOKEN_RBLK
		{
			$$ = wms_make_stmt_with_for(NULL, $3, $5, $8);
			debug("for(k in array) { stmt_list } stmt");
		}
		| TOKEN_FOR TOKEN_LPAR TOKEN_SYMBOL TOKEN_IN expr TOKEN_RPAR TOKEN_LBLK TOKEN_RBLK
		{
			$$ = wms_make_stmt_with_for($3, NULL, $5, NULL);
			debug("for(k in array) { } stmt");
		}
		| TOKEN_FOR TOKEN_LPAR TOKEN_SYMBOL TOKEN_IN TOKEN_INT TOKEN_DOTDOT TOKEN_INT TOKEN_RPAR TOKEN_LBLK stmt_list TOKEN_RBLK
		{
			$$ = wms_make_stmt_with_for_range($3, $5, $7, $10);
			debug("for(v in i..j) { stmt_list } stmt");
		}
		| TOKEN_FOR TOKEN_LPAR TOKEN_SYMBOL TOKEN_IN TOKEN_INT TOKEN_DOTDOT TOKEN_INT TOKEN_RPAR TOKEN_LBLK TOKEN_RBLK
		{
			$$ = wms_make_stmt_with_for_range($3, $5, $7, NULL);
			debug("for(v in i..j) { stmt_list } stmt");
		}
		;
return_stmt	: TOKEN_RETURN expr TOKEN_SEMICOLON
		{
			$$ = wms_make_stmt_with_return($2);
			debug("rerurn stmt");
		}
		;
break_stmt	: TOKEN_BREAK TOKEN_SEMICOLON
		{
			$$ = wms_make_stmt_with_break();
			debug("break stmt");
		}
		;
continue_stmt	: TOKEN_CONTINUE TOKEN_SEMICOLON
		{
			$$ = wms_make_stmt_with_continue();
			debug("continue stmt");
		}
		;
expr		: term
		{
			$$ = wms_make_expr_with_term($1);
			debug("single term expr");
		}
		| expr TOKEN_OR expr
		{
			$$ = wms_make_expr_with_or($1, $3);
			debug("or expr");
		}
		| expr TOKEN_AND expr
		{
			$$ = wms_make_expr_with_and($1, $3);
			debug("and expr");
		}
		| expr TOKEN_LT expr
		{
			$$ = wms_make_expr_with_lt($1, $3);
			debug("lt expr");
		}
		| expr TOKEN_LTE expr
		{
			$$ = wms_make_expr_with_lte($1, $3);
			debug("lte expr");
		}
		| expr TOKEN_GT expr
		{
			$$ = wms_make_expr_with_gt($1, $3);
			debug("gt expr");
		}
		| expr TOKEN_GTE expr
		{
			$$ = wms_make_expr_with_gte($1, $3);
			debug("gte expr");
		}
		| expr TOKEN_EQ expr
		{
			$$ = wms_make_expr_with_eq($1, $3);
			debug("eq expr");
		}
		| expr TOKEN_PLUS expr
		{
			$$ = wms_make_expr_with_plus($1, $3);
			debug("add expr");
		}
		| expr TOKEN_MINUS expr
		{
			$$ = wms_make_expr_with_minus($1, $3);
			debug("sub expr");
		}
		| expr TOKEN_MUL expr
		{
			$$ = wms_make_expr_with_mul($1, $3);
			debug("mul expr");
		}
		| expr TOKEN_DIV expr
		{
			$$ = wms_make_expr_with_div($1, $3);
			debug("div expr");
		}
		| TOKEN_MINUS expr
		{
			$$ = wms_make_expr_with_neg($2);
			debug("neg expr");
		}
		| TOKEN_LPAR expr TOKEN_RPAR
		{
			$$ = $2;
			debug("(expr) expr");
		}
		;
term		: TOKEN_INT
		{
			$$ = wms_make_term_with_int($1);
			debug("int term");
		}
		| TOKEN_FLOAT
		{
			$$ = wms_make_term_with_float($1);
			debug("float term");
		}
		| TOKEN_STR
		{
			$$ = wms_make_term_with_str($1);
			debug("str term");
		}
		| TOKEN_SYMBOL
		{
			$$ = wms_make_term_with_symbol($1);
			debug("symbol term");
		}
		| TOKEN_SYMBOL TOKEN_LARR expr TOKEN_RARR
		{
			$$ = wms_make_term_with_array($1, $3);
			debug("array[subscr]");
		}
		| TOKEN_SYMBOL TOKEN_LPAR TOKEN_RPAR
		{
			$$ = wms_make_term_with_call($1, NULL);
			debug("call() term");
		}
		| TOKEN_SYMBOL TOKEN_LPAR arg_list TOKEN_RPAR
		{
			$$ = wms_make_term_with_call($1, $3);
			debug("call(param_list) term");
		}
		;
arg_list	: expr
		{
			$$ = wms_make_arg_list(NULL, $1);
			debug("single expr arg_list");
		}
		| arg_list TOKEN_COMMA expr
		{
			$$ = wms_make_arg_list($1, $3);
			debug("multiple expr param_list");
		}
		;
%%

#ifdef DEBUG
static void _debug(const char *s)
{
	fprintf(stderr, "%s\n", s);
}
#endif

void wms_yyerror(const char *s)
{
	(void)s;
	wms_parser_error_line = wms_yylloc.last_line + 1;
	wms_parser_error_column = wms_yylloc.last_column + 1;
}
