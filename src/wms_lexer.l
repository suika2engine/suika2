%{
/*
 * Watermelon Script
 * Copyright (c) 2022, Keiichi Tabata. All rights reserved.
 */

#include "wms_parser.tab.h"
#include "wms_core.h"

#ifdef _MSC_VER
#define strdup _strdup
#define fileno _fileno
#endif

#define YY_NO_INPUT
#define YY_NO_UNPUT

extern YYLTYPE yylloc;
%}

%option reentrant

%%
\/\/.*\n 	{
			wms_yylloc.first_line = wms_yylloc.last_line;
			wms_yylloc.first_column = wms_yylloc.last_column + 1;
			wms_yylloc.last_line++;
			wms_yylloc.last_column = 0;
		}
[0-9]*\.[0-9]+	{
			sscanf(yytext, "%lf", &wms_yylval.fval);
			wms_yylloc.first_line = wms_yylloc.last_line;
			wms_yylloc.first_column = wms_yylloc.last_column + 1;
			wms_yylloc.last_column += yyleng;
			return TOKEN_FLOAT;
		}
[0-9]*		{
			sscanf(yytext, "%d", &wms_yylval.ival);
			wms_yylloc.first_line = wms_yylloc.last_line;
			wms_yylloc.first_column = wms_yylloc.last_column + 1;
			wms_yylloc.last_column += yyleng;
			return TOKEN_INT;
		}
["]([^"\\\n]|\\(.|\n))*["] {
			wms_yylval.sval = strdup(yytext + 1);
			wms_yylval.sval[yyleng - 2] = '\0';
			wms_yylloc.first_line = wms_yylloc.last_line;
			wms_yylloc.first_column = wms_yylloc.last_column + 1;
			wms_yylloc.last_column += yyleng;
			return TOKEN_STR;
		}
"+"		{
			wms_yylloc.first_line = wms_yylloc.last_line;
			wms_yylloc.first_column = wms_yylloc.last_column + 1;
			wms_yylloc.last_column += yyleng;
			return TOKEN_PLUS;
		}
"-"		{
			wms_yylloc.first_line = wms_yylloc.last_line;
			wms_yylloc.first_column = wms_yylloc.last_column + 1;
			wms_yylloc.last_column += yyleng;
			return TOKEN_MINUS;
		}
"*"		{
			wms_yylloc.first_line = wms_yylloc.last_line;
			wms_yylloc.first_column = wms_yylloc.last_column + 1;
			wms_yylloc.last_column += yyleng;
			return TOKEN_MUL;
		}
"/"		{
			wms_yylloc.first_line = wms_yylloc.last_line;
			wms_yylloc.first_column = wms_yylloc.last_column + 1;
			wms_yylloc.last_column += yyleng;
			return TOKEN_DIV;
		}
"="		{
			wms_yylloc.first_line = wms_yylloc.last_line;
			wms_yylloc.first_column = wms_yylloc.last_column + 1;
			wms_yylloc.last_column += yyleng;
			return TOKEN_ASSIGN;
		}
">"		{
			wms_yylloc.first_line = wms_yylloc.last_line;
			wms_yylloc.first_column = wms_yylloc.last_column + 1;
			wms_yylloc.last_column += yyleng;
			return TOKEN_GT;
		}
">="		{
			wms_yylloc.first_line = wms_yylloc.last_line;
			wms_yylloc.first_column = wms_yylloc.last_column + 1;
			wms_yylloc.last_column += yyleng;
			return TOKEN_GTE;
		}
"<"		{
			wms_yylloc.first_line = wms_yylloc.last_line;
			wms_yylloc.first_column = wms_yylloc.last_column + 1;
			wms_yylloc.last_column += yyleng;
			return TOKEN_LT;
		}
"<="		{
			wms_yylloc.first_line = wms_yylloc.last_line;
			wms_yylloc.first_column = wms_yylloc.last_column + 1;
			wms_yylloc.last_column += yyleng;
			return TOKEN_LTE;
		}
"=="		{
			wms_yylloc.first_line = wms_yylloc.last_line;
			wms_yylloc.first_column = wms_yylloc.last_column + 1;
			wms_yylloc.last_column += yyleng;
			return TOKEN_EQ;
		}
"&&"		{
			wms_yylloc.first_line = wms_yylloc.last_line;
			wms_yylloc.first_column = wms_yylloc.last_column + 1;
			wms_yylloc.last_column += yyleng;
			return TOKEN_AND;
		}
"||"		{
			wms_yylloc.first_line = wms_yylloc.last_line;
			wms_yylloc.first_column = wms_yylloc.last_column + 1;
			wms_yylloc.last_column += yyleng;
			return TOKEN_OR;
		}
";"		{
			wms_yylloc.first_line = wms_yylloc.last_line;
			wms_yylloc.first_column = wms_yylloc.last_column + 1;
			wms_yylloc.last_column += yyleng;
			return TOKEN_SEMICOLON;
		}
"func"		{
			wms_yylloc.first_line = wms_yylloc.last_line;
			wms_yylloc.first_column = wms_yylloc.last_column + 1;
			wms_yylloc.last_column += yyleng;
			return TOKEN_FUNC;
		}
"("		{
			wms_yylloc.first_line = wms_yylloc.last_line;
			wms_yylloc.first_column = wms_yylloc.last_column + 1;
			wms_yylloc.last_column += yyleng;
			return TOKEN_LPAR;
		}
")"		{
			wms_yylloc.first_line = wms_yylloc.last_line;
			wms_yylloc.first_column = wms_yylloc.last_column + 1;
			wms_yylloc.last_column += yyleng;
			return TOKEN_RPAR;
		}
"{"		{
			wms_yylloc.first_line = wms_yylloc.last_line;
			wms_yylloc.first_column = wms_yylloc.last_column + 1;
			wms_yylloc.last_column += yyleng;
			return TOKEN_LBLK;
		}
"}"		{
			wms_yylloc.first_line = wms_yylloc.last_line;
			wms_yylloc.first_column = wms_yylloc.last_column + 1;
			wms_yylloc.last_column += yyleng;
			return TOKEN_RBLK;
		}
"["		{
			wms_yylloc.first_line = wms_yylloc.last_line;
			wms_yylloc.first_column = wms_yylloc.last_column + 1;
			wms_yylloc.last_column += yyleng;
			return TOKEN_LARR;
		}
"]"		{
			wms_yylloc.first_line = wms_yylloc.last_line;
			wms_yylloc.first_column = wms_yylloc.last_column + 1;
			wms_yylloc.last_column += yyleng;
			return TOKEN_RARR;
		}
"if"		{
			wms_yylloc.first_line = wms_yylloc.last_line;
			wms_yylloc.first_column = wms_yylloc.last_column + 1;
			wms_yylloc.last_column += yyleng;
			return TOKEN_IF;
		}
"else"		{
			wms_yylloc.first_line = wms_yylloc.last_line;
			wms_yylloc.first_column = wms_yylloc.last_column + 1;
			wms_yylloc.last_column += yyleng;
			return TOKEN_ELSE;
		}
"while"		{
			wms_yylloc.first_line = wms_yylloc.last_line;
			wms_yylloc.first_column = wms_yylloc.last_column + 1;
			wms_yylloc.last_column += yyleng;
			return TOKEN_WHILE;
		}
"for"		{
			wms_yylloc.first_line = wms_yylloc.last_line;
			wms_yylloc.first_column = wms_yylloc.last_column + 1;
			wms_yylloc.last_column += yyleng;
			return TOKEN_FOR;
		}
","		{
			wms_yylloc.first_line = wms_yylloc.last_line;
			wms_yylloc.first_column = wms_yylloc.last_column + 1;
			wms_yylloc.last_column += yyleng;
			return TOKEN_COMMA;
		}
"in"		{
			wms_yylloc.first_line = wms_yylloc.last_line;
			wms_yylloc.first_column = wms_yylloc.last_column + 1;
			wms_yylloc.last_column += yyleng;
			return TOKEN_IN;
		}
".."		{
			wms_yylloc.first_line = wms_yylloc.last_line;
			wms_yylloc.first_column = wms_yylloc.last_column + 1;
			wms_yylloc.last_column += yyleng;
			return TOKEN_DOTDOT;
		}
"return"	{
			wms_yylloc.first_line = wms_yylloc.last_line;
			wms_yylloc.first_column = wms_yylloc.last_column + 1;
			wms_yylloc.last_column += yyleng;
			return TOKEN_RETURN;
		}
"break"		{
			wms_yylloc.first_line = wms_yylloc.last_line;
			wms_yylloc.first_column = wms_yylloc.last_column + 1;
			wms_yylloc.last_column += yyleng;
			return TOKEN_BREAK;
		}
"continue"	{
			wms_yylloc.first_line = wms_yylloc.last_line;
			wms_yylloc.first_column = wms_yylloc.last_column + 1;
			wms_yylloc.last_column += yyleng;
			return TOKEN_CONTINUE;
		}
[ \t\r]		{
			wms_yylloc.first_line = wms_yylloc.last_line;
			wms_yylloc.first_column = wms_yylloc.last_column + 1;
			wms_yylloc.last_column += yyleng;
		}
"\n"		{
			wms_yylloc.first_line = wms_yylloc.last_line;
			wms_yylloc.first_column = wms_yylloc.last_column + 1;
			wms_yylloc.last_line++;
			wms_yylloc.last_column = 0;
		}
[a-zA-Z_0-9]+	{
			wms_yylval.sval = strdup(yytext);
			wms_yylloc.first_line = wms_yylloc.last_line;
			wms_yylloc.first_column = wms_yylloc.last_column + 1;
			wms_yylloc.last_column += yyleng;
			return TOKEN_SYMBOL;
		}
%%

int wms_yywrap(yyscan_t scanner)
{
	(void)scanner;
	return 1;
}
