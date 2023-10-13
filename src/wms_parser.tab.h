/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2021 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

#ifndef YY_WMS_YY_SRC_WMS_PARSER_TAB_H_INCLUDED
# define YY_WMS_YY_SRC_WMS_PARSER_TAB_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int wms_yydebug;
#endif

/* Token kinds.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    YYEMPTY = -2,
    YYEOF = 0,                     /* "end of file"  */
    YYerror = 256,                 /* error  */
    YYUNDEF = 257,                 /* "invalid token"  */
    TOKEN_SYMBOL = 258,            /* TOKEN_SYMBOL  */
    TOKEN_STR = 259,               /* TOKEN_STR  */
    TOKEN_INT = 260,               /* TOKEN_INT  */
    TOKEN_FLOAT = 261,             /* TOKEN_FLOAT  */
    TOKEN_FUNC = 262,              /* TOKEN_FUNC  */
    TOKEN_PLUS = 263,              /* TOKEN_PLUS  */
    TOKEN_MINUS = 264,             /* TOKEN_MINUS  */
    TOKEN_MUL = 265,               /* TOKEN_MUL  */
    TOKEN_DIV = 266,               /* TOKEN_DIV  */
    TOKEN_MOD = 267,               /* TOKEN_MOD  */
    TOKEN_ASSIGN = 268,            /* TOKEN_ASSIGN  */
    TOKEN_LPAR = 269,              /* TOKEN_LPAR  */
    TOKEN_RPAR = 270,              /* TOKEN_RPAR  */
    TOKEN_LBLK = 271,              /* TOKEN_LBLK  */
    TOKEN_RBLK = 272,              /* TOKEN_RBLK  */
    TOKEN_LARR = 273,              /* TOKEN_LARR  */
    TOKEN_RARR = 274,              /* TOKEN_RARR  */
    TOKEN_SEMICOLON = 275,         /* TOKEN_SEMICOLON  */
    TOKEN_COMMA = 276,             /* TOKEN_COMMA  */
    TOKEN_IF = 277,                /* TOKEN_IF  */
    TOKEN_ELSE = 278,              /* TOKEN_ELSE  */
    TOKEN_WHILE = 279,             /* TOKEN_WHILE  */
    TOKEN_FOR = 280,               /* TOKEN_FOR  */
    TOKEN_IN = 281,                /* TOKEN_IN  */
    TOKEN_DOTDOT = 282,            /* TOKEN_DOTDOT  */
    TOKEN_GT = 283,                /* TOKEN_GT  */
    TOKEN_GTE = 284,               /* TOKEN_GTE  */
    TOKEN_LT = 285,                /* TOKEN_LT  */
    TOKEN_LTE = 286,               /* TOKEN_LTE  */
    TOKEN_EQ = 287,                /* TOKEN_EQ  */
    TOKEN_NEQ = 288,               /* TOKEN_NEQ  */
    TOKEN_RETURN = 289,            /* TOKEN_RETURN  */
    TOKEN_BREAK = 290,             /* TOKEN_BREAK  */
    TOKEN_CONTINUE = 291,          /* TOKEN_CONTINUE  */
    TOKEN_AND = 292,               /* TOKEN_AND  */
    TOKEN_OR = 293                 /* TOKEN_OR  */
  };
  typedef enum yytokentype yytoken_kind_t;
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 36 "../../src/wms_parser.y"

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

#line 117 "../../src/wms_parser.tab.h"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif

/* Location type.  */
#if ! defined YYLTYPE && ! defined YYLTYPE_IS_DECLARED
typedef struct YYLTYPE YYLTYPE;
struct YYLTYPE
{
  int first_line;
  int first_column;
  int last_line;
  int last_column;
};
# define YYLTYPE_IS_DECLARED 1
# define YYLTYPE_IS_TRIVIAL 1
#endif


extern YYSTYPE wms_yylval;
extern YYLTYPE wms_yylloc;

int wms_yyparse (void *scanner);

/* "%code provides" blocks.  */
#line 32 "../../src/wms_parser.y"

#define YY_DECL int wms_yylex(void *yyscanner)

#line 150 "../../src/wms_parser.tab.h"

#endif /* !YY_WMS_YY_SRC_WMS_PARSER_TAB_H_INCLUDED  */
