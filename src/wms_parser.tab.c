/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison implementation for Yacc-like parsers in C

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

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output, and Bison version.  */
#define YYBISON 30802

/* Bison version string.  */
#define YYBISON_VERSION "3.8.2"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1


/* Substitute the variable and function names.  */
#define yyparse         wms_yyparse
#define yylex           wms_yylex
#define yyerror         wms_yyerror
#define yydebug         wms_yydebug
#define yynerrs         wms_yynerrs
#define yylval          wms_yylval
#define yychar          wms_yychar
#define yylloc          wms_yylloc

/* First part of user prologue.  */
#line 1 "wms_parser.y"

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

#line 102 "wms_parser.tab.c"

# ifndef YY_CAST
#  ifdef __cplusplus
#   define YY_CAST(Type, Val) static_cast<Type> (Val)
#   define YY_REINTERPRET_CAST(Type, Val) reinterpret_cast<Type> (Val)
#  else
#   define YY_CAST(Type, Val) ((Type) (Val))
#   define YY_REINTERPRET_CAST(Type, Val) ((Type) (Val))
#  endif
# endif
# ifndef YY_NULLPTR
#  if defined __cplusplus
#   if 201103L <= __cplusplus
#    define YY_NULLPTR nullptr
#   else
#    define YY_NULLPTR 0
#   endif
#  else
#   define YY_NULLPTR ((void*)0)
#  endif
# endif

#include "wms_parser.tab.h"
/* Symbol kind.  */
enum yysymbol_kind_t
{
  YYSYMBOL_YYEMPTY = -2,
  YYSYMBOL_YYEOF = 0,                      /* "end of file"  */
  YYSYMBOL_YYerror = 1,                    /* error  */
  YYSYMBOL_YYUNDEF = 2,                    /* "invalid token"  */
  YYSYMBOL_TOKEN_SYMBOL = 3,               /* TOKEN_SYMBOL  */
  YYSYMBOL_TOKEN_STR = 4,                  /* TOKEN_STR  */
  YYSYMBOL_TOKEN_INT = 5,                  /* TOKEN_INT  */
  YYSYMBOL_TOKEN_FLOAT = 6,                /* TOKEN_FLOAT  */
  YYSYMBOL_TOKEN_FUNC = 7,                 /* TOKEN_FUNC  */
  YYSYMBOL_TOKEN_PLUS = 8,                 /* TOKEN_PLUS  */
  YYSYMBOL_TOKEN_MINUS = 9,                /* TOKEN_MINUS  */
  YYSYMBOL_TOKEN_MUL = 10,                 /* TOKEN_MUL  */
  YYSYMBOL_TOKEN_DIV = 11,                 /* TOKEN_DIV  */
  YYSYMBOL_TOKEN_ASSIGN = 12,              /* TOKEN_ASSIGN  */
  YYSYMBOL_TOKEN_LPAR = 13,                /* TOKEN_LPAR  */
  YYSYMBOL_TOKEN_RPAR = 14,                /* TOKEN_RPAR  */
  YYSYMBOL_TOKEN_LBLK = 15,                /* TOKEN_LBLK  */
  YYSYMBOL_TOKEN_RBLK = 16,                /* TOKEN_RBLK  */
  YYSYMBOL_TOKEN_LARR = 17,                /* TOKEN_LARR  */
  YYSYMBOL_TOKEN_RARR = 18,                /* TOKEN_RARR  */
  YYSYMBOL_TOKEN_SEMICOLON = 19,           /* TOKEN_SEMICOLON  */
  YYSYMBOL_TOKEN_COMMA = 20,               /* TOKEN_COMMA  */
  YYSYMBOL_TOKEN_IF = 21,                  /* TOKEN_IF  */
  YYSYMBOL_TOKEN_ELSE = 22,                /* TOKEN_ELSE  */
  YYSYMBOL_TOKEN_WHILE = 23,               /* TOKEN_WHILE  */
  YYSYMBOL_TOKEN_FOR = 24,                 /* TOKEN_FOR  */
  YYSYMBOL_TOKEN_IN = 25,                  /* TOKEN_IN  */
  YYSYMBOL_TOKEN_DOTDOT = 26,              /* TOKEN_DOTDOT  */
  YYSYMBOL_TOKEN_GT = 27,                  /* TOKEN_GT  */
  YYSYMBOL_TOKEN_GTE = 28,                 /* TOKEN_GTE  */
  YYSYMBOL_TOKEN_LT = 29,                  /* TOKEN_LT  */
  YYSYMBOL_TOKEN_LTE = 30,                 /* TOKEN_LTE  */
  YYSYMBOL_TOKEN_EQ = 31,                  /* TOKEN_EQ  */
  YYSYMBOL_TOKEN_RETURN = 32,              /* TOKEN_RETURN  */
  YYSYMBOL_TOKEN_BREAK = 33,               /* TOKEN_BREAK  */
  YYSYMBOL_TOKEN_CONTINUE = 34,            /* TOKEN_CONTINUE  */
  YYSYMBOL_TOKEN_AND = 35,                 /* TOKEN_AND  */
  YYSYMBOL_TOKEN_OR = 36,                  /* TOKEN_OR  */
  YYSYMBOL_YYACCEPT = 37,                  /* $accept  */
  YYSYMBOL_func_list = 38,                 /* func_list  */
  YYSYMBOL_func = 39,                      /* func  */
  YYSYMBOL_param_list = 40,                /* param_list  */
  YYSYMBOL_stmt_list = 41,                 /* stmt_list  */
  YYSYMBOL_stmt = 42,                      /* stmt  */
  YYSYMBOL_empty_stmt = 43,                /* empty_stmt  */
  YYSYMBOL_expr_stmt = 44,                 /* expr_stmt  */
  YYSYMBOL_assign_stmt = 45,               /* assign_stmt  */
  YYSYMBOL_if_stmt = 46,                   /* if_stmt  */
  YYSYMBOL_elif_stmt = 47,                 /* elif_stmt  */
  YYSYMBOL_else_stmt = 48,                 /* else_stmt  */
  YYSYMBOL_while_stmt = 49,                /* while_stmt  */
  YYSYMBOL_for_stmt = 50,                  /* for_stmt  */
  YYSYMBOL_return_stmt = 51,               /* return_stmt  */
  YYSYMBOL_break_stmt = 52,                /* break_stmt  */
  YYSYMBOL_continue_stmt = 53,             /* continue_stmt  */
  YYSYMBOL_expr = 54,                      /* expr  */
  YYSYMBOL_term = 55,                      /* term  */
  YYSYMBOL_arg_list = 56                   /* arg_list  */
};
typedef enum yysymbol_kind_t yysymbol_kind_t;




#ifdef short
# undef short
#endif

/* On compilers that do not define __PTRDIFF_MAX__ etc., make sure
   <limits.h> and (if available) <stdint.h> are included
   so that the code can choose integer types of a good width.  */

#ifndef __PTRDIFF_MAX__
# include <limits.h> /* INFRINGES ON USER NAME SPACE */
# if defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stdint.h> /* INFRINGES ON USER NAME SPACE */
#  define YY_STDINT_H
# endif
#endif

/* Narrow types that promote to a signed type and that can represent a
   signed or unsigned integer of at least N bits.  In tables they can
   save space and decrease cache pressure.  Promoting to a signed type
   helps avoid bugs in integer arithmetic.  */

#ifdef __INT_LEAST8_MAX__
typedef __INT_LEAST8_TYPE__ yytype_int8;
#elif defined YY_STDINT_H
typedef int_least8_t yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef __INT_LEAST16_MAX__
typedef __INT_LEAST16_TYPE__ yytype_int16;
#elif defined YY_STDINT_H
typedef int_least16_t yytype_int16;
#else
typedef short yytype_int16;
#endif

/* Work around bug in HP-UX 11.23, which defines these macros
   incorrectly for preprocessor constants.  This workaround can likely
   be removed in 2023, as HPE has promised support for HP-UX 11.23
   (aka HP-UX 11i v2) only through the end of 2022; see Table 2 of
   <https://h20195.www2.hpe.com/V2/getpdf.aspx/4AA4-7673ENW.pdf>.  */
#ifdef __hpux
# undef UINT_LEAST8_MAX
# undef UINT_LEAST16_MAX
# define UINT_LEAST8_MAX 255
# define UINT_LEAST16_MAX 65535
#endif

#if defined __UINT_LEAST8_MAX__ && __UINT_LEAST8_MAX__ <= __INT_MAX__
typedef __UINT_LEAST8_TYPE__ yytype_uint8;
#elif (!defined __UINT_LEAST8_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST8_MAX <= INT_MAX)
typedef uint_least8_t yytype_uint8;
#elif !defined __UINT_LEAST8_MAX__ && UCHAR_MAX <= INT_MAX
typedef unsigned char yytype_uint8;
#else
typedef short yytype_uint8;
#endif

#if defined __UINT_LEAST16_MAX__ && __UINT_LEAST16_MAX__ <= __INT_MAX__
typedef __UINT_LEAST16_TYPE__ yytype_uint16;
#elif (!defined __UINT_LEAST16_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST16_MAX <= INT_MAX)
typedef uint_least16_t yytype_uint16;
#elif !defined __UINT_LEAST16_MAX__ && USHRT_MAX <= INT_MAX
typedef unsigned short yytype_uint16;
#else
typedef int yytype_uint16;
#endif

#ifndef YYPTRDIFF_T
# if defined __PTRDIFF_TYPE__ && defined __PTRDIFF_MAX__
#  define YYPTRDIFF_T __PTRDIFF_TYPE__
#  define YYPTRDIFF_MAXIMUM __PTRDIFF_MAX__
# elif defined PTRDIFF_MAX
#  ifndef ptrdiff_t
#   include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  endif
#  define YYPTRDIFF_T ptrdiff_t
#  define YYPTRDIFF_MAXIMUM PTRDIFF_MAX
# else
#  define YYPTRDIFF_T long
#  define YYPTRDIFF_MAXIMUM LONG_MAX
# endif
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned
# endif
#endif

#define YYSIZE_MAXIMUM                                  \
  YY_CAST (YYPTRDIFF_T,                                 \
           (YYPTRDIFF_MAXIMUM < YY_CAST (YYSIZE_T, -1)  \
            ? YYPTRDIFF_MAXIMUM                         \
            : YY_CAST (YYSIZE_T, -1)))

#define YYSIZEOF(X) YY_CAST (YYPTRDIFF_T, sizeof (X))


/* Stored state numbers (used for stacks). */
typedef yytype_uint8 yy_state_t;

/* State numbers in computations.  */
typedef int yy_state_fast_t;

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif


#ifndef YY_ATTRIBUTE_PURE
# if defined __GNUC__ && 2 < __GNUC__ + (96 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_PURE __attribute__ ((__pure__))
# else
#  define YY_ATTRIBUTE_PURE
# endif
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# if defined __GNUC__ && 2 < __GNUC__ + (7 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_UNUSED __attribute__ ((__unused__))
# else
#  define YY_ATTRIBUTE_UNUSED
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YY_USE(E) ((void) (E))
#else
# define YY_USE(E) /* empty */
#endif

/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
#if defined __GNUC__ && ! defined __ICC && 406 <= __GNUC__ * 100 + __GNUC_MINOR__
# if __GNUC__ * 100 + __GNUC_MINOR__ < 407
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")
# else
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")              \
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# endif
# define YY_IGNORE_MAYBE_UNINITIALIZED_END      \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif

#if defined __cplusplus && defined __GNUC__ && ! defined __ICC && 6 <= __GNUC__
# define YY_IGNORE_USELESS_CAST_BEGIN                          \
    _Pragma ("GCC diagnostic push")                            \
    _Pragma ("GCC diagnostic ignored \"-Wuseless-cast\"")
# define YY_IGNORE_USELESS_CAST_END            \
    _Pragma ("GCC diagnostic pop")
#endif
#ifndef YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_END
#endif


#define YY_ASSERT(E) ((void) (0 && (E)))

#if !defined yyoverflow

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
             && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* !defined yyoverflow */

#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL \
             && defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yy_state_t yyss_alloc;
  YYSTYPE yyvs_alloc;
  YYLTYPE yyls_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (YYSIZEOF (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (YYSIZEOF (yy_state_t) + YYSIZEOF (YYSTYPE) \
             + YYSIZEOF (YYLTYPE)) \
      + 2 * YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYPTRDIFF_T yynewbytes;                                         \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * YYSIZEOF (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / YYSIZEOF (*yyptr);                        \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, YY_CAST (YYSIZE_T, (Count)) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYPTRDIFF_T yyi;                      \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  5
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   778

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  37
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  20
/* YYNRULES -- Number of rules.  */
#define YYNRULES  66
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  153

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   291


/* YYTRANSLATE(TOKEN-NUM) -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, with out-of-bounds checking.  */
#define YYTRANSLATE(YYX)                                \
  (0 <= (YYX) && (YYX) <= YYMAXUTOK                     \
   ? YY_CAST (yysymbol_kind_t, yytranslate[YYX])        \
   : YYSYMBOL_YYUNDEF)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex.  */
static const yytype_int8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36
};

#if YYDEBUG
/* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,    88,    88,    93,    99,   104,   109,   114,   120,   125,
     131,   136,   142,   148,   154,   160,   166,   172,   178,   184,
     190,   197,   204,   211,   217,   223,   228,   234,   239,   245,
     250,   256,   261,   267,   272,   278,   283,   288,   293,   298,
     303,   309,   315,   321,   327,   332,   337,   342,   347,   352,
     357,   362,   367,   372,   377,   382,   387,   392,   398,   403,
     408,   413,   418,   423,   428,   434,   439
};
#endif

/** Accessing symbol of state STATE.  */
#define YY_ACCESSING_SYMBOL(State) YY_CAST (yysymbol_kind_t, yystos[State])

#if YYDEBUG || 0
/* The user-facing name of the symbol whose (internal) number is
   YYSYMBOL.  No bounds checking.  */
static const char *yysymbol_name (yysymbol_kind_t yysymbol) YY_ATTRIBUTE_UNUSED;

/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "\"end of file\"", "error", "\"invalid token\"", "TOKEN_SYMBOL",
  "TOKEN_STR", "TOKEN_INT", "TOKEN_FLOAT", "TOKEN_FUNC", "TOKEN_PLUS",
  "TOKEN_MINUS", "TOKEN_MUL", "TOKEN_DIV", "TOKEN_ASSIGN", "TOKEN_LPAR",
  "TOKEN_RPAR", "TOKEN_LBLK", "TOKEN_RBLK", "TOKEN_LARR", "TOKEN_RARR",
  "TOKEN_SEMICOLON", "TOKEN_COMMA", "TOKEN_IF", "TOKEN_ELSE",
  "TOKEN_WHILE", "TOKEN_FOR", "TOKEN_IN", "TOKEN_DOTDOT", "TOKEN_GT",
  "TOKEN_GTE", "TOKEN_LT", "TOKEN_LTE", "TOKEN_EQ", "TOKEN_RETURN",
  "TOKEN_BREAK", "TOKEN_CONTINUE", "TOKEN_AND", "TOKEN_OR", "$accept",
  "func_list", "func", "param_list", "stmt_list", "stmt", "empty_stmt",
  "expr_stmt", "assign_stmt", "if_stmt", "elif_stmt", "else_stmt",
  "while_stmt", "for_stmt", "return_stmt", "break_stmt", "continue_stmt",
  "expr", "term", "arg_list", YY_NULLPTR
};

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  return yytname[yysymbol];
}
#endif

#define YYPACT_NINF (-39)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-1)

#define yytable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int16 yypact[] =
{
       3,    24,     5,   -39,    30,   -39,   -39,    25,   -39,    36,
      21,    -2,    57,    74,   477,   -39,   -39,   -39,    79,    79,
     -39,   -39,    68,    58,    73,    78,    79,    70,    80,   107,
     -39,   -39,   -39,   -39,   -39,   -39,   -39,   -39,   -39,   -39,
     -39,   -39,   476,   -39,   129,   -39,    79,    20,    79,    27,
      54,   488,    79,   151,    84,    79,    90,   517,   -39,   -39,
     -39,   -39,    79,    79,    79,    79,   -39,    79,    79,    79,
      79,    79,    79,    79,   -39,   173,   530,   -39,    39,    76,
     559,    79,   -39,   570,   -39,   195,    79,   599,    -7,   -39,
     499,    54,    92,   -39,   565,   606,   736,   523,   685,   731,
     722,   -39,   -39,   -39,    79,    89,   613,    93,   -39,   642,
      94,   112,   765,    39,    79,   -39,   217,   102,   239,    97,
      98,   652,   681,   -39,   261,   283,   -39,   305,    79,   120,
     121,   -39,   -39,   -39,   327,   -39,   693,   113,   349,   -39,
     122,   128,   -39,   371,   393,   415,   -39,   -39,   437,   -39,
     459,   -39,   -39
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_int8 yydefact[] =
{
       0,     0,     0,     2,     0,     1,     3,     0,     8,     0,
       0,     0,     0,     0,    61,    60,    58,    59,     0,     0,
       7,    23,     0,     0,     0,     0,     0,     0,     0,     0,
      10,    12,    13,    14,    15,    16,    17,    18,    19,    20,
      21,    22,     0,    44,     0,     9,     0,     0,     0,    61,
      56,     0,     0,     0,     0,     0,     0,     0,    42,    43,
       6,    11,     0,     0,     0,     0,    24,     0,     0,     0,
       0,     0,     0,     0,     5,     0,     0,    63,    65,     0,
       0,     0,    57,     0,    32,     0,     0,     0,     0,    41,
      52,    53,    54,    55,    49,    50,    47,    48,    51,    46,
      45,     4,    25,    64,     0,    62,     0,     0,    31,     0,
       0,     0,     0,    66,     0,    62,     0,     0,     0,     0,
      58,     0,     0,    28,     0,     0,    34,     0,     0,     0,
       0,    26,    27,    30,     0,    33,     0,     0,     0,    29,
       0,     0,    38,     0,     0,     0,    37,    36,     0,    40,
       0,    35,    39
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
     -39,   -39,   142,   -39,   -38,   -29,   -39,   -39,   -39,   -39,
     -39,   -39,   -39,   -39,   -39,   -39,   -39,   -10,   -39,   -39
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int8 yydefgoto[] =
{
       0,     2,     3,    10,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    79
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_uint8 yytable[] =
{
      61,    14,    15,    16,    17,     5,    75,    18,    50,    51,
       1,    19,     1,   111,    20,    85,    57,    21,   112,    22,
      23,    24,    25,    49,    15,    16,    17,     4,     8,    18,
      26,    27,    28,    19,    77,    12,    76,    78,    80,     9,
      47,    13,    83,     7,    81,    87,    61,    62,    63,    64,
      65,    11,    90,    91,    92,    93,    61,    94,    95,    96,
      97,    98,    99,   100,    64,    65,    67,    68,    69,    70,
      71,   106,    44,    53,    72,    73,   109,    45,   124,    54,
     127,    52,    49,    15,    16,    17,    55,   134,    18,    58,
     103,    56,    19,    88,   113,    61,   104,    86,    61,    59,
     143,   114,   121,    65,   122,    61,   148,   150,   116,   118,
      14,    15,    16,    17,    61,   119,    18,   125,   136,    61,
      19,    61,   128,    60,   129,   137,    21,   141,    22,    23,
      24,    25,    14,    15,    16,    17,   138,   144,    18,    26,
      27,    28,    19,   145,     6,    74,     0,     0,    21,     0,
      22,    23,    24,    25,    14,    15,    16,    17,     0,     0,
      18,    26,    27,    28,    19,     0,     0,    84,     0,     0,
      21,     0,    22,    23,    24,    25,    14,    15,    16,    17,
       0,     0,    18,    26,    27,    28,    19,     0,     0,   101,
       0,     0,    21,     0,    22,    23,    24,    25,    14,    15,
      16,    17,     0,     0,    18,    26,    27,    28,    19,     0,
       0,   108,     0,     0,    21,     0,    22,    23,    24,    25,
      14,    15,    16,    17,     0,     0,    18,    26,    27,    28,
      19,     0,     0,   123,     0,     0,    21,     0,    22,    23,
      24,    25,    14,    15,    16,    17,     0,     0,    18,    26,
      27,    28,    19,     0,     0,   126,     0,     0,    21,     0,
      22,    23,    24,    25,    14,    15,    16,    17,     0,     0,
      18,    26,    27,    28,    19,     0,     0,   132,     0,     0,
      21,     0,    22,    23,    24,    25,    14,    15,    16,    17,
       0,     0,    18,    26,    27,    28,    19,     0,     0,   133,
       0,     0,    21,     0,    22,    23,    24,    25,    14,    15,
      16,    17,     0,     0,    18,    26,    27,    28,    19,     0,
       0,   135,     0,     0,    21,     0,    22,    23,    24,    25,
      14,    15,    16,    17,     0,     0,    18,    26,    27,    28,
      19,     0,     0,   139,     0,     0,    21,     0,    22,    23,
      24,    25,    14,    15,    16,    17,     0,     0,    18,    26,
      27,    28,    19,     0,     0,   142,     0,     0,    21,     0,
      22,    23,    24,    25,    14,    15,    16,    17,     0,     0,
      18,    26,    27,    28,    19,     0,     0,   146,     0,     0,
      21,     0,    22,    23,    24,    25,    14,    15,    16,    17,
       0,     0,    18,    26,    27,    28,    19,     0,     0,   147,
       0,     0,    21,     0,    22,    23,    24,    25,    14,    15,
      16,    17,     0,     0,    18,    26,    27,    28,    19,     0,
       0,   149,     0,     0,    21,     0,    22,    23,    24,    25,
      14,    15,    16,    17,     0,     0,    18,    26,    27,    28,
      19,     0,     0,   151,     0,     0,    21,     0,    22,    23,
      24,    25,    14,    15,    16,    17,     0,     0,    18,    26,
      27,    28,    19,     0,     0,   152,     0,     0,    21,     0,
      22,    23,    24,    25,    62,    63,    64,    65,     0,    46,
      47,    26,    27,    28,    48,    66,    62,    63,    64,    65,
       0,     0,    82,    67,    68,    69,    70,    71,    63,    64,
      65,    72,    73,     0,     0,    67,    68,    69,    70,    71,
       0,     0,     0,    72,    73,    62,    63,    64,    65,     0,
       0,    62,    63,    64,    65,     0,    89,     0,    62,    63,
      64,    65,     0,     0,    67,    68,    69,    70,    71,   102,
      67,    68,    72,    73,    71,     0,     0,    67,    68,    69,
      70,    71,     0,     0,     0,    72,    73,    62,    63,    64,
      65,     0,     0,    62,    63,    64,    65,   105,    62,    63,
      64,    65,     0,     0,   107,     0,    67,    68,    69,    70,
      71,     0,     0,    68,    72,    73,    71,    67,    68,    69,
      70,    71,     0,     0,     0,    72,    73,    62,    63,    64,
      65,     0,     0,   110,    62,    63,    64,    65,     0,     0,
       0,    62,    63,    64,    65,     0,    67,    68,    69,    70,
      71,   115,     0,     0,    72,    73,     0,    71,     0,     0,
      67,    68,    69,    70,    71,     0,     0,     0,    72,    73,
      62,    63,    64,    65,     0,     0,   117,     0,     0,     0,
      62,    63,    64,    65,     0,     0,   130,     0,     0,    67,
      68,    69,    70,    71,     0,     0,     0,    72,    73,    67,
      68,    69,    70,    71,     0,     0,     0,    72,    73,    62,
      63,    64,    65,    62,    63,    64,    65,     0,     0,     0,
     131,    62,    63,    64,    65,     0,     0,   140,    67,    68,
      69,    70,    71,     0,     0,     0,    72,    73,     0,     0,
      67,    68,    69,    70,    71,     0,     0,     0,    72,    73,
      62,    63,    64,    65,     0,     0,     0,     0,     0,    62,
      63,    64,    65,     0,    62,    63,    64,    65,     0,    67,
      68,    69,    70,    71,     0,     0,     0,    72,    67,    68,
      69,    70,    71,    67,    68,     0,    70,    71,    49,    15,
     120,    17,     0,     0,    18,     0,     0,     0,    19
};

static const yytype_int16 yycheck[] =
{
      29,     3,     4,     5,     6,     0,    44,     9,    18,    19,
       7,    13,     7,    20,    16,    53,    26,    19,    25,    21,
      22,    23,    24,     3,     4,     5,     6,     3,     3,     9,
      32,    33,    34,    13,    14,    14,    46,    47,    48,    14,
      13,    20,    52,    13,    17,    55,    75,     8,     9,    10,
      11,    15,    62,    63,    64,    65,    85,    67,    68,    69,
      70,    71,    72,    73,    10,    11,    27,    28,    29,    30,
      31,    81,    15,    15,    35,    36,    86,     3,   116,    21,
     118,    13,     3,     4,     5,     6,    13,   125,     9,    19,
      14,    13,    13,     3,   104,   124,    20,    13,   127,    19,
     138,    12,   112,    11,   114,   134,   144,   145,    15,    15,
       3,     4,     5,     6,   143,     3,     9,    15,   128,   148,
      13,   150,    25,    16,    26,     5,    19,    14,    21,    22,
      23,    24,     3,     4,     5,     6,    15,    15,     9,    32,
      33,    34,    13,    15,     2,    16,    -1,    -1,    19,    -1,
      21,    22,    23,    24,     3,     4,     5,     6,    -1,    -1,
       9,    32,    33,    34,    13,    -1,    -1,    16,    -1,    -1,
      19,    -1,    21,    22,    23,    24,     3,     4,     5,     6,
      -1,    -1,     9,    32,    33,    34,    13,    -1,    -1,    16,
      -1,    -1,    19,    -1,    21,    22,    23,    24,     3,     4,
       5,     6,    -1,    -1,     9,    32,    33,    34,    13,    -1,
      -1,    16,    -1,    -1,    19,    -1,    21,    22,    23,    24,
       3,     4,     5,     6,    -1,    -1,     9,    32,    33,    34,
      13,    -1,    -1,    16,    -1,    -1,    19,    -1,    21,    22,
      23,    24,     3,     4,     5,     6,    -1,    -1,     9,    32,
      33,    34,    13,    -1,    -1,    16,    -1,    -1,    19,    -1,
      21,    22,    23,    24,     3,     4,     5,     6,    -1,    -1,
       9,    32,    33,    34,    13,    -1,    -1,    16,    -1,    -1,
      19,    -1,    21,    22,    23,    24,     3,     4,     5,     6,
      -1,    -1,     9,    32,    33,    34,    13,    -1,    -1,    16,
      -1,    -1,    19,    -1,    21,    22,    23,    24,     3,     4,
       5,     6,    -1,    -1,     9,    32,    33,    34,    13,    -1,
      -1,    16,    -1,    -1,    19,    -1,    21,    22,    23,    24,
       3,     4,     5,     6,    -1,    -1,     9,    32,    33,    34,
      13,    -1,    -1,    16,    -1,    -1,    19,    -1,    21,    22,
      23,    24,     3,     4,     5,     6,    -1,    -1,     9,    32,
      33,    34,    13,    -1,    -1,    16,    -1,    -1,    19,    -1,
      21,    22,    23,    24,     3,     4,     5,     6,    -1,    -1,
       9,    32,    33,    34,    13,    -1,    -1,    16,    -1,    -1,
      19,    -1,    21,    22,    23,    24,     3,     4,     5,     6,
      -1,    -1,     9,    32,    33,    34,    13,    -1,    -1,    16,
      -1,    -1,    19,    -1,    21,    22,    23,    24,     3,     4,
       5,     6,    -1,    -1,     9,    32,    33,    34,    13,    -1,
      -1,    16,    -1,    -1,    19,    -1,    21,    22,    23,    24,
       3,     4,     5,     6,    -1,    -1,     9,    32,    33,    34,
      13,    -1,    -1,    16,    -1,    -1,    19,    -1,    21,    22,
      23,    24,     3,     4,     5,     6,    -1,    -1,     9,    32,
      33,    34,    13,    -1,    -1,    16,    -1,    -1,    19,    -1,
      21,    22,    23,    24,     8,     9,    10,    11,    -1,    12,
      13,    32,    33,    34,    17,    19,     8,     9,    10,    11,
      -1,    -1,    14,    27,    28,    29,    30,    31,     9,    10,
      11,    35,    36,    -1,    -1,    27,    28,    29,    30,    31,
      -1,    -1,    -1,    35,    36,     8,     9,    10,    11,    -1,
      -1,     8,     9,    10,    11,    -1,    19,    -1,     8,     9,
      10,    11,    -1,    -1,    27,    28,    29,    30,    31,    19,
      27,    28,    35,    36,    31,    -1,    -1,    27,    28,    29,
      30,    31,    -1,    -1,    -1,    35,    36,     8,     9,    10,
      11,    -1,    -1,     8,     9,    10,    11,    18,     8,     9,
      10,    11,    -1,    -1,    14,    -1,    27,    28,    29,    30,
      31,    -1,    -1,    28,    35,    36,    31,    27,    28,    29,
      30,    31,    -1,    -1,    -1,    35,    36,     8,     9,    10,
      11,    -1,    -1,    14,     8,     9,    10,    11,    -1,    -1,
      -1,     8,     9,    10,    11,    -1,    27,    28,    29,    30,
      31,    18,    -1,    -1,    35,    36,    -1,    31,    -1,    -1,
      27,    28,    29,    30,    31,    -1,    -1,    -1,    35,    36,
       8,     9,    10,    11,    -1,    -1,    14,    -1,    -1,    -1,
       8,     9,    10,    11,    -1,    -1,    14,    -1,    -1,    27,
      28,    29,    30,    31,    -1,    -1,    -1,    35,    36,    27,
      28,    29,    30,    31,    -1,    -1,    -1,    35,    36,     8,
       9,    10,    11,     8,     9,    10,    11,    -1,    -1,    -1,
      19,     8,     9,    10,    11,    -1,    -1,    14,    27,    28,
      29,    30,    31,    -1,    -1,    -1,    35,    36,    -1,    -1,
      27,    28,    29,    30,    31,    -1,    -1,    -1,    35,    36,
       8,     9,    10,    11,    -1,    -1,    -1,    -1,    -1,     8,
       9,    10,    11,    -1,     8,     9,    10,    11,    -1,    27,
      28,    29,    30,    31,    -1,    -1,    -1,    35,    27,    28,
      29,    30,    31,    27,    28,    -1,    30,    31,     3,     4,
       5,     6,    -1,    -1,     9,    -1,    -1,    -1,    13
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_int8 yystos[] =
{
       0,     7,    38,    39,     3,     0,    39,    13,     3,    14,
      40,    15,    14,    20,     3,     4,     5,     6,     9,    13,
      16,    19,    21,    22,    23,    24,    32,    33,    34,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    15,     3,    12,    13,    17,     3,
      54,    54,    13,    15,    21,    13,    13,    54,    19,    19,
      16,    42,     8,     9,    10,    11,    19,    27,    28,    29,
      30,    31,    35,    36,    16,    41,    54,    14,    54,    56,
      54,    17,    14,    54,    16,    41,    13,    54,     3,    19,
      54,    54,    54,    54,    54,    54,    54,    54,    54,    54,
      54,    16,    19,    14,    20,    18,    54,    14,    16,    54,
      14,    20,    25,    54,    12,    18,    15,    14,    15,     3,
       5,    54,    54,    16,    41,    15,    16,    41,    25,    26,
      14,    19,    16,    16,    41,    16,    54,     5,    15,    16,
      14,    14,    16,    41,    15,    15,    16,    16,    41,    16,
      41,    16,    16
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr1[] =
{
       0,    37,    38,    38,    39,    39,    39,    39,    40,    40,
      41,    41,    42,    42,    42,    42,    42,    42,    42,    42,
      42,    42,    42,    43,    44,    45,    45,    46,    46,    47,
      47,    48,    48,    49,    49,    50,    50,    50,    50,    50,
      50,    51,    52,    53,    54,    54,    54,    54,    54,    54,
      54,    54,    54,    54,    54,    54,    54,    54,    55,    55,
      55,    55,    55,    55,    55,    56,    56
};

/* YYR2[RULE-NUM] -- Number of symbols on the right-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     1,     2,     8,     7,     7,     6,     1,     3,
       1,     2,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     2,     4,     7,     7,     6,     8,
       7,     4,     3,     7,     6,    11,    10,     9,     8,    11,
      10,     3,     2,     2,     1,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     2,     3,     1,     1,
       1,     1,     4,     3,     4,     1,     3
};


enum { YYENOMEM = -2 };

#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab
#define YYNOMEM         goto yyexhaustedlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                    \
  do                                                              \
    if (yychar == YYEMPTY)                                        \
      {                                                           \
        yychar = (Token);                                         \
        yylval = (Value);                                         \
        YYPOPSTACK (yylen);                                       \
        yystate = *yyssp;                                         \
        goto yybackup;                                            \
      }                                                           \
    else                                                          \
      {                                                           \
        yyerror (YY_("syntax error: cannot back up")); \
        YYERROR;                                                  \
      }                                                           \
  while (0)

/* Backward compatibility with an undocumented macro.
   Use YYerror or YYUNDEF. */
#define YYERRCODE YYUNDEF

/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)                                \
    do                                                                  \
      if (N)                                                            \
        {                                                               \
          (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;        \
          (Current).first_column = YYRHSLOC (Rhs, 1).first_column;      \
          (Current).last_line    = YYRHSLOC (Rhs, N).last_line;         \
          (Current).last_column  = YYRHSLOC (Rhs, N).last_column;       \
        }                                                               \
      else                                                              \
        {                                                               \
          (Current).first_line   = (Current).last_line   =              \
            YYRHSLOC (Rhs, 0).last_line;                                \
          (Current).first_column = (Current).last_column =              \
            YYRHSLOC (Rhs, 0).last_column;                              \
        }                                                               \
    while (0)
#endif

#define YYRHSLOC(Rhs, K) ((Rhs)[K])


/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)


/* YYLOCATION_PRINT -- Print the location on the stream.
   This macro was not mandated originally: define only if we know
   we won't break user code: when these are the locations we know.  */

# ifndef YYLOCATION_PRINT

#  if defined YY_LOCATION_PRINT

   /* Temporary convenience wrapper in case some people defined the
      undocumented and private YY_LOCATION_PRINT macros.  */
#   define YYLOCATION_PRINT(File, Loc)  YY_LOCATION_PRINT(File, *(Loc))

#  elif defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL

/* Print *YYLOCP on YYO.  Private, do not rely on its existence. */

YY_ATTRIBUTE_UNUSED
static int
yy_location_print_ (FILE *yyo, YYLTYPE const * const yylocp)
{
  int res = 0;
  int end_col = 0 != yylocp->last_column ? yylocp->last_column - 1 : 0;
  if (0 <= yylocp->first_line)
    {
      res += YYFPRINTF (yyo, "%d", yylocp->first_line);
      if (0 <= yylocp->first_column)
        res += YYFPRINTF (yyo, ".%d", yylocp->first_column);
    }
  if (0 <= yylocp->last_line)
    {
      if (yylocp->first_line < yylocp->last_line)
        {
          res += YYFPRINTF (yyo, "-%d", yylocp->last_line);
          if (0 <= end_col)
            res += YYFPRINTF (yyo, ".%d", end_col);
        }
      else if (0 <= end_col && yylocp->first_column < end_col)
        res += YYFPRINTF (yyo, "-%d", end_col);
    }
  return res;
}

#   define YYLOCATION_PRINT  yy_location_print_

    /* Temporary convenience wrapper in case some people defined the
       undocumented and private YY_LOCATION_PRINT macros.  */
#   define YY_LOCATION_PRINT(File, Loc)  YYLOCATION_PRINT(File, &(Loc))

#  else

#   define YYLOCATION_PRINT(File, Loc) ((void) 0)
    /* Temporary convenience wrapper in case some people defined the
       undocumented and private YY_LOCATION_PRINT macros.  */
#   define YY_LOCATION_PRINT  YYLOCATION_PRINT

#  endif
# endif /* !defined YYLOCATION_PRINT */


# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Kind, Value, Location); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*-----------------------------------.
| Print this symbol's value on YYO.  |
`-----------------------------------*/

static void
yy_symbol_value_print (FILE *yyo,
                       yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp)
{
  FILE *yyoutput = yyo;
  YY_USE (yyoutput);
  YY_USE (yylocationp);
  if (!yyvaluep)
    return;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/*---------------------------.
| Print this symbol on YYO.  |
`---------------------------*/

static void
yy_symbol_print (FILE *yyo,
                 yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp)
{
  YYFPRINTF (yyo, "%s %s (",
             yykind < YYNTOKENS ? "token" : "nterm", yysymbol_name (yykind));

  YYLOCATION_PRINT (yyo, yylocationp);
  YYFPRINTF (yyo, ": ");
  yy_symbol_value_print (yyo, yykind, yyvaluep, yylocationp);
  YYFPRINTF (yyo, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yy_state_t *yybottom, yy_state_t *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yy_state_t *yyssp, YYSTYPE *yyvsp, YYLTYPE *yylsp,
                 int yyrule)
{
  int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %d):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       YY_ACCESSING_SYMBOL (+yyssp[yyi + 1 - yynrhs]),
                       &yyvsp[(yyi + 1) - (yynrhs)],
                       &(yylsp[(yyi + 1) - (yynrhs)]));
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, yylsp, Rule); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args) ((void) 0)
# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif






/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg,
            yysymbol_kind_t yykind, YYSTYPE *yyvaluep, YYLTYPE *yylocationp)
{
  YY_USE (yyvaluep);
  YY_USE (yylocationp);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yykind, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/* Lookahead token kind.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;
/* Location data for the lookahead symbol.  */
YYLTYPE yylloc
# if defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL
  = { 1, 1, 1, 1 }
# endif
;
/* Number of syntax errors so far.  */
int yynerrs;




/*----------.
| yyparse.  |
`----------*/

int
yyparse (void)
{
    yy_state_fast_t yystate = 0;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus = 0;

    /* Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* Their size.  */
    YYPTRDIFF_T yystacksize = YYINITDEPTH;

    /* The state stack: array, bottom, top.  */
    yy_state_t yyssa[YYINITDEPTH];
    yy_state_t *yyss = yyssa;
    yy_state_t *yyssp = yyss;

    /* The semantic value stack: array, bottom, top.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs = yyvsa;
    YYSTYPE *yyvsp = yyvs;

    /* The location stack: array, bottom, top.  */
    YYLTYPE yylsa[YYINITDEPTH];
    YYLTYPE *yyls = yylsa;
    YYLTYPE *yylsp = yyls;

  int yyn;
  /* The return value of yyparse.  */
  int yyresult;
  /* Lookahead symbol kind.  */
  yysymbol_kind_t yytoken = YYSYMBOL_YYEMPTY;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;
  YYLTYPE yyloc;

  /* The locations where the error started and ended.  */
  YYLTYPE yyerror_range[3];



#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N), yylsp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yychar = YYEMPTY; /* Cause a token to be read.  */


/* User initialization code.  */
#line 82 "wms_parser.y"
{
	wms_yylloc.last_line = yylloc.first_line = 0;
	wms_yylloc.last_column = yylloc.first_column = 0;
}

#line 1277 "wms_parser.tab.c"

  yylsp[0] = yylloc;
  goto yysetstate;


/*------------------------------------------------------------.
| yynewstate -- push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;


/*--------------------------------------------------------------------.
| yysetstate -- set current state (the top of the stack) to yystate.  |
`--------------------------------------------------------------------*/
yysetstate:
  YYDPRINTF ((stderr, "Entering state %d\n", yystate));
  YY_ASSERT (0 <= yystate && yystate < YYNSTATES);
  YY_IGNORE_USELESS_CAST_BEGIN
  *yyssp = YY_CAST (yy_state_t, yystate);
  YY_IGNORE_USELESS_CAST_END
  YY_STACK_PRINT (yyss, yyssp);

  if (yyss + yystacksize - 1 <= yyssp)
#if !defined yyoverflow && !defined YYSTACK_RELOCATE
    YYNOMEM;
#else
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYPTRDIFF_T yysize = yyssp - yyss + 1;

# if defined yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        yy_state_t *yyss1 = yyss;
        YYSTYPE *yyvs1 = yyvs;
        YYLTYPE *yyls1 = yyls;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * YYSIZEOF (*yyssp),
                    &yyvs1, yysize * YYSIZEOF (*yyvsp),
                    &yyls1, yysize * YYSIZEOF (*yylsp),
                    &yystacksize);
        yyss = yyss1;
        yyvs = yyvs1;
        yyls = yyls1;
      }
# else /* defined YYSTACK_RELOCATE */
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        YYNOMEM;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yy_state_t *yyss1 = yyss;
        union yyalloc *yyptr =
          YY_CAST (union yyalloc *,
                   YYSTACK_ALLOC (YY_CAST (YYSIZE_T, YYSTACK_BYTES (yystacksize))));
        if (! yyptr)
          YYNOMEM;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
        YYSTACK_RELOCATE (yyls_alloc, yyls);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;
      yylsp = yyls + yysize - 1;

      YY_IGNORE_USELESS_CAST_BEGIN
      YYDPRINTF ((stderr, "Stack size increased to %ld\n",
                  YY_CAST (long, yystacksize)));
      YY_IGNORE_USELESS_CAST_END

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }
#endif /* !defined yyoverflow && !defined YYSTACK_RELOCATE */


  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;


/*-----------.
| yybackup.  |
`-----------*/
yybackup:
  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either empty, or end-of-input, or a valid lookahead.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token\n"));
      yychar = yylex ();
    }

  if (yychar <= YYEOF)
    {
      yychar = YYEOF;
      yytoken = YYSYMBOL_YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else if (yychar == YYerror)
    {
      /* The scanner already issued an error message, process directly
         to error recovery.  But do not keep the error token as
         lookahead, it is too special and may lead us to an endless
         loop in error recovery. */
      yychar = YYUNDEF;
      yytoken = YYSYMBOL_YYerror;
      yyerror_range[1] = yylloc;
      goto yyerrlab1;
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);
  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END
  *++yylsp = yylloc;

  /* Discard the shifted token.  */
  yychar = YYEMPTY;
  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     '$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];

  /* Default location. */
  YYLLOC_DEFAULT (yyloc, (yylsp - yylen), yylen);
  yyerror_range[1] = yyloc;
  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
  case 2: /* func_list: func  */
#line 89 "wms_parser.y"
                {
			(yyval.func_list) = wms_make_func_list(NULL, (yyvsp[0].func));
			debug("single func func_list");
		}
#line 1493 "wms_parser.tab.c"
    break;

  case 3: /* func_list: func_list func  */
#line 94 "wms_parser.y"
                {
			(yyval.func_list) = wms_make_func_list((yyvsp[-1].func_list), (yyvsp[0].func));
			debug("multiple func func_list");
		}
#line 1502 "wms_parser.tab.c"
    break;

  case 4: /* func: TOKEN_FUNC TOKEN_SYMBOL TOKEN_LPAR param_list TOKEN_RPAR TOKEN_LBLK stmt_list TOKEN_RBLK  */
#line 100 "wms_parser.y"
                {
			(yyval.func) = wms_make_func((yyvsp[-6].sval), (yyvsp[-4].param_list), (yyvsp[-1].stmt_list));
			debug("param_list and stmt_list function");
		}
#line 1511 "wms_parser.tab.c"
    break;

  case 5: /* func: TOKEN_FUNC TOKEN_SYMBOL TOKEN_LPAR param_list TOKEN_RPAR TOKEN_LBLK TOKEN_RBLK  */
#line 105 "wms_parser.y"
                {
			(yyval.func) = wms_make_func((yyvsp[-5].sval), (yyvsp[-3].param_list), NULL);
			debug("param_list function");
		}
#line 1520 "wms_parser.tab.c"
    break;

  case 6: /* func: TOKEN_FUNC TOKEN_SYMBOL TOKEN_LPAR TOKEN_RPAR TOKEN_LBLK stmt_list TOKEN_RBLK  */
#line 110 "wms_parser.y"
                {
			(yyval.func) = wms_make_func((yyvsp[-5].sval), NULL, (yyvsp[-1].stmt_list));
			debug("stmt_list function");
		}
#line 1529 "wms_parser.tab.c"
    break;

  case 7: /* func: TOKEN_FUNC TOKEN_SYMBOL TOKEN_LPAR TOKEN_RPAR TOKEN_LBLK TOKEN_RBLK  */
#line 115 "wms_parser.y"
                {
			(yyval.func) = wms_make_func((yyvsp[-4].sval), NULL, NULL);
			debug("empty function");
		}
#line 1538 "wms_parser.tab.c"
    break;

  case 8: /* param_list: TOKEN_SYMBOL  */
#line 121 "wms_parser.y"
                {
			(yyval.param_list) = wms_make_param_list(NULL, (yyvsp[0].sval));
			debug("single param param_list");
		}
#line 1547 "wms_parser.tab.c"
    break;

  case 9: /* param_list: param_list TOKEN_COMMA TOKEN_SYMBOL  */
#line 126 "wms_parser.y"
                {
			(yyval.param_list) = wms_make_param_list((yyvsp[-2].param_list), (yyvsp[0].sval));
			debug("multiple params");
		}
#line 1556 "wms_parser.tab.c"
    break;

  case 10: /* stmt_list: stmt  */
#line 132 "wms_parser.y"
                {
			(yyval.stmt_list) = wms_make_stmt_list(NULL, (yyvsp[0].stmt));
			debug("single stmt stmt_list");
		}
#line 1565 "wms_parser.tab.c"
    break;

  case 11: /* stmt_list: stmt_list stmt  */
#line 137 "wms_parser.y"
                {
			(yyval.stmt_list) = wms_make_stmt_list((yyvsp[-1].stmt_list), (yyvsp[0].stmt));
			debug("multiple stmt stmt_list");
		}
#line 1574 "wms_parser.tab.c"
    break;

  case 12: /* stmt: empty_stmt  */
#line 143 "wms_parser.y"
                {
			(yyval.stmt) = (yyvsp[0].stmt);
			wms_set_stmt_position((yyvsp[0].stmt), wms_yylloc.first_line + 1);
			debug("stmt");
		}
#line 1584 "wms_parser.tab.c"
    break;

  case 13: /* stmt: expr_stmt  */
#line 149 "wms_parser.y"
                {
			(yyval.stmt) = (yyvsp[0].stmt);
			wms_set_stmt_position((yyvsp[0].stmt), wms_yylloc.first_line + 1);
			debug("stmt");
		}
#line 1594 "wms_parser.tab.c"
    break;

  case 14: /* stmt: assign_stmt  */
#line 155 "wms_parser.y"
                {
			(yyval.stmt) = (yyvsp[0].stmt);
			wms_set_stmt_position((yyvsp[0].stmt), wms_yylloc.first_line + 1);
			debug("stmt");
		}
#line 1604 "wms_parser.tab.c"
    break;

  case 15: /* stmt: if_stmt  */
#line 161 "wms_parser.y"
                {
			(yyval.stmt) = (yyvsp[0].stmt);
			wms_set_stmt_position((yyvsp[0].stmt), wms_yylloc.first_line + 1);
			debug("stmt");
		}
#line 1614 "wms_parser.tab.c"
    break;

  case 16: /* stmt: elif_stmt  */
#line 167 "wms_parser.y"
                {
			(yyval.stmt) = (yyvsp[0].stmt);
			wms_set_stmt_position((yyvsp[0].stmt), wms_yylloc.first_line + 1);
			debug("stmt");
		}
#line 1624 "wms_parser.tab.c"
    break;

  case 17: /* stmt: else_stmt  */
#line 173 "wms_parser.y"
                {
			(yyval.stmt) = (yyvsp[0].stmt);
			wms_set_stmt_position((yyvsp[0].stmt), wms_yylloc.first_line + 1);
			debug("stmt");
		}
#line 1634 "wms_parser.tab.c"
    break;

  case 18: /* stmt: while_stmt  */
#line 179 "wms_parser.y"
                {
			(yyval.stmt) = (yyvsp[0].stmt);
			wms_set_stmt_position((yyvsp[0].stmt), wms_yylloc.first_line + 1);
			debug("stmt");
		}
#line 1644 "wms_parser.tab.c"
    break;

  case 19: /* stmt: for_stmt  */
#line 185 "wms_parser.y"
                {
			(yyval.stmt) = (yyvsp[0].stmt);
			wms_set_stmt_position((yyvsp[0].stmt), wms_yylloc.first_line + 1);
			debug("stmt");
		}
#line 1654 "wms_parser.tab.c"
    break;

  case 20: /* stmt: return_stmt  */
#line 191 "wms_parser.y"
                {
			(yyval.stmt) = (yyvsp[0].stmt);
			wms_set_stmt_position((yyvsp[0].stmt), wms_yylloc.first_line + 1);
			debug("stmt");
		}
#line 1664 "wms_parser.tab.c"
    break;

  case 21: /* stmt: break_stmt  */
#line 198 "wms_parser.y"
                {
			(yyval.stmt) = (yyvsp[0].stmt);
			wms_set_stmt_position((yyvsp[0].stmt), wms_yylloc.first_line + 1);
			debug("stmt");
		}
#line 1674 "wms_parser.tab.c"
    break;

  case 22: /* stmt: continue_stmt  */
#line 205 "wms_parser.y"
                {
			(yyval.stmt) = (yyvsp[0].stmt);
			wms_set_stmt_position((yyvsp[0].stmt), wms_yylloc.first_line + 1);
			debug("stmt");
		}
#line 1684 "wms_parser.tab.c"
    break;

  case 23: /* empty_stmt: TOKEN_SEMICOLON  */
#line 212 "wms_parser.y"
                {
			(yyval.stmt) = wms_make_stmt_with_nothing();
			debug("empty stmt");
		}
#line 1693 "wms_parser.tab.c"
    break;

  case 24: /* expr_stmt: expr TOKEN_SEMICOLON  */
#line 218 "wms_parser.y"
                {
			(yyval.stmt) = wms_make_stmt_with_expr((yyvsp[-1].expr));
			debug("expr stmt");
		}
#line 1702 "wms_parser.tab.c"
    break;

  case 25: /* assign_stmt: TOKEN_SYMBOL TOKEN_ASSIGN expr TOKEN_SEMICOLON  */
#line 224 "wms_parser.y"
                {
			(yyval.stmt) = wms_make_stmt_with_symbol_assign((yyvsp[-3].sval), (yyvsp[-1].expr));
			debug("symbol assign stmt");
		}
#line 1711 "wms_parser.tab.c"
    break;

  case 26: /* assign_stmt: TOKEN_SYMBOL TOKEN_LARR expr TOKEN_RARR TOKEN_ASSIGN expr TOKEN_SEMICOLON  */
#line 229 "wms_parser.y"
                {
			(yyval.stmt) = wms_make_stmt_with_array_assign((yyvsp[-6].sval), (yyvsp[-4].expr), (yyvsp[-1].expr));
			debug("array assign stmt");
		}
#line 1720 "wms_parser.tab.c"
    break;

  case 27: /* if_stmt: TOKEN_IF TOKEN_LPAR expr TOKEN_RPAR TOKEN_LBLK stmt_list TOKEN_RBLK  */
#line 235 "wms_parser.y"
                {
			(yyval.stmt) = wms_make_stmt_with_if((yyvsp[-4].expr), (yyvsp[-1].stmt_list));
			debug("if { stmt_list } stmt");
		}
#line 1729 "wms_parser.tab.c"
    break;

  case 28: /* if_stmt: TOKEN_IF TOKEN_LPAR expr TOKEN_RPAR TOKEN_LBLK TOKEN_RBLK  */
#line 240 "wms_parser.y"
                {
			(yyval.stmt) = wms_make_stmt_with_if((yyvsp[-3].expr), NULL);
			debug("if {} stmt");
		}
#line 1738 "wms_parser.tab.c"
    break;

  case 29: /* elif_stmt: TOKEN_ELSE TOKEN_IF TOKEN_LPAR expr TOKEN_RPAR TOKEN_LBLK stmt_list TOKEN_RBLK  */
#line 246 "wms_parser.y"
                {
			(yyval.stmt) = wms_make_stmt_with_elif((yyvsp[-4].expr), (yyvsp[-1].stmt_list));
			debug("elif { stmt_list } stmt");
		}
#line 1747 "wms_parser.tab.c"
    break;

  case 30: /* elif_stmt: TOKEN_ELSE TOKEN_IF TOKEN_LPAR expr TOKEN_RPAR TOKEN_LBLK TOKEN_RBLK  */
#line 251 "wms_parser.y"
                {
			(yyval.stmt) = wms_make_stmt_with_elif((yyvsp[-3].expr), NULL);
			debug("elif {} stmt");
		}
#line 1756 "wms_parser.tab.c"
    break;

  case 31: /* else_stmt: TOKEN_ELSE TOKEN_LBLK stmt_list TOKEN_RBLK  */
#line 257 "wms_parser.y"
                {
			(yyval.stmt) = wms_make_stmt_with_else((yyvsp[-1].stmt_list));
			debug("else { stmt_list } stmt");
		}
#line 1765 "wms_parser.tab.c"
    break;

  case 32: /* else_stmt: TOKEN_ELSE TOKEN_LBLK TOKEN_RBLK  */
#line 262 "wms_parser.y"
                {
			(yyval.stmt) = wms_make_stmt_with_else(NULL);
			debug("else { } stmt");
		}
#line 1774 "wms_parser.tab.c"
    break;

  case 33: /* while_stmt: TOKEN_WHILE TOKEN_LPAR expr TOKEN_RPAR TOKEN_LBLK stmt_list TOKEN_RBLK  */
#line 268 "wms_parser.y"
                {
			(yyval.stmt) = wms_make_stmt_with_while((yyvsp[-4].expr), (yyvsp[-1].stmt_list));
			debug("while { stmt_list } stmt");
		}
#line 1783 "wms_parser.tab.c"
    break;

  case 34: /* while_stmt: TOKEN_WHILE TOKEN_LPAR expr TOKEN_RPAR TOKEN_LBLK TOKEN_RBLK  */
#line 273 "wms_parser.y"
                {
			(yyval.stmt) = wms_make_stmt_with_while((yyvsp[-3].expr), NULL);
			debug("while { } stmt");
		}
#line 1792 "wms_parser.tab.c"
    break;

  case 35: /* for_stmt: TOKEN_FOR TOKEN_LPAR TOKEN_SYMBOL TOKEN_COMMA TOKEN_SYMBOL TOKEN_IN expr TOKEN_RPAR TOKEN_LBLK stmt_list TOKEN_RBLK  */
#line 279 "wms_parser.y"
                {
			(yyval.stmt) = wms_make_stmt_with_for((yyvsp[-8].sval), (yyvsp[-6].sval), (yyvsp[-4].expr), (yyvsp[-1].stmt_list));
			debug("for(k, v in array) { stmt_list } stmt");
		}
#line 1801 "wms_parser.tab.c"
    break;

  case 36: /* for_stmt: TOKEN_FOR TOKEN_LPAR TOKEN_SYMBOL TOKEN_COMMA TOKEN_SYMBOL TOKEN_IN expr TOKEN_RPAR TOKEN_LBLK TOKEN_RBLK  */
#line 284 "wms_parser.y"
                {
			(yyval.stmt) = wms_make_stmt_with_for((yyvsp[-7].sval), (yyvsp[-5].sval), (yyvsp[-3].expr), NULL);
			debug("for(k, v in array) { } stmt");
		}
#line 1810 "wms_parser.tab.c"
    break;

  case 37: /* for_stmt: TOKEN_FOR TOKEN_LPAR TOKEN_SYMBOL TOKEN_IN expr TOKEN_RPAR TOKEN_LBLK stmt_list TOKEN_RBLK  */
#line 289 "wms_parser.y"
                {
			(yyval.stmt) = wms_make_stmt_with_for(NULL, (yyvsp[-6].sval), (yyvsp[-4].expr), (yyvsp[-1].stmt_list));
			debug("for(k in array) { stmt_list } stmt");
		}
#line 1819 "wms_parser.tab.c"
    break;

  case 38: /* for_stmt: TOKEN_FOR TOKEN_LPAR TOKEN_SYMBOL TOKEN_IN expr TOKEN_RPAR TOKEN_LBLK TOKEN_RBLK  */
#line 294 "wms_parser.y"
                {
			(yyval.stmt) = wms_make_stmt_with_for((yyvsp[-5].sval), NULL, (yyvsp[-3].expr), NULL);
			debug("for(k in array) { } stmt");
		}
#line 1828 "wms_parser.tab.c"
    break;

  case 39: /* for_stmt: TOKEN_FOR TOKEN_LPAR TOKEN_SYMBOL TOKEN_IN TOKEN_INT TOKEN_DOTDOT TOKEN_INT TOKEN_RPAR TOKEN_LBLK stmt_list TOKEN_RBLK  */
#line 299 "wms_parser.y"
                {
			(yyval.stmt) = wms_make_stmt_with_for_range((yyvsp[-8].sval), (yyvsp[-6].ival), (yyvsp[-4].ival), (yyvsp[-1].stmt_list));
			debug("for(v in i..j) { stmt_list } stmt");
		}
#line 1837 "wms_parser.tab.c"
    break;

  case 40: /* for_stmt: TOKEN_FOR TOKEN_LPAR TOKEN_SYMBOL TOKEN_IN TOKEN_INT TOKEN_DOTDOT TOKEN_INT TOKEN_RPAR TOKEN_LBLK TOKEN_RBLK  */
#line 304 "wms_parser.y"
                {
			(yyval.stmt) = wms_make_stmt_with_for_range((yyvsp[-7].sval), (yyvsp[-5].ival), (yyvsp[-3].ival), NULL);
			debug("for(v in i..j) { stmt_list } stmt");
		}
#line 1846 "wms_parser.tab.c"
    break;

  case 41: /* return_stmt: TOKEN_RETURN expr TOKEN_SEMICOLON  */
#line 310 "wms_parser.y"
                {
			(yyval.stmt) = wms_make_stmt_with_return((yyvsp[-1].expr));
			debug("rerurn stmt");
		}
#line 1855 "wms_parser.tab.c"
    break;

  case 42: /* break_stmt: TOKEN_BREAK TOKEN_SEMICOLON  */
#line 316 "wms_parser.y"
                {
			(yyval.stmt) = wms_make_stmt_with_break();
			debug("break stmt");
		}
#line 1864 "wms_parser.tab.c"
    break;

  case 43: /* continue_stmt: TOKEN_CONTINUE TOKEN_SEMICOLON  */
#line 322 "wms_parser.y"
                {
			(yyval.stmt) = wms_make_stmt_with_continue();
			debug("continue stmt");
		}
#line 1873 "wms_parser.tab.c"
    break;

  case 44: /* expr: term  */
#line 328 "wms_parser.y"
                {
			(yyval.expr) = wms_make_expr_with_term((yyvsp[0].term));
			debug("single term expr");
		}
#line 1882 "wms_parser.tab.c"
    break;

  case 45: /* expr: expr TOKEN_OR expr  */
#line 333 "wms_parser.y"
                {
			(yyval.expr) = wms_make_expr_with_or((yyvsp[-2].expr), (yyvsp[0].expr));
			debug("or expr");
		}
#line 1891 "wms_parser.tab.c"
    break;

  case 46: /* expr: expr TOKEN_AND expr  */
#line 338 "wms_parser.y"
                {
			(yyval.expr) = wms_make_expr_with_and((yyvsp[-2].expr), (yyvsp[0].expr));
			debug("and expr");
		}
#line 1900 "wms_parser.tab.c"
    break;

  case 47: /* expr: expr TOKEN_LT expr  */
#line 343 "wms_parser.y"
                {
			(yyval.expr) = wms_make_expr_with_lt((yyvsp[-2].expr), (yyvsp[0].expr));
			debug("lt expr");
		}
#line 1909 "wms_parser.tab.c"
    break;

  case 48: /* expr: expr TOKEN_LTE expr  */
#line 348 "wms_parser.y"
                {
			(yyval.expr) = wms_make_expr_with_lte((yyvsp[-2].expr), (yyvsp[0].expr));
			debug("lte expr");
		}
#line 1918 "wms_parser.tab.c"
    break;

  case 49: /* expr: expr TOKEN_GT expr  */
#line 353 "wms_parser.y"
                {
			(yyval.expr) = wms_make_expr_with_gt((yyvsp[-2].expr), (yyvsp[0].expr));
			debug("gt expr");
		}
#line 1927 "wms_parser.tab.c"
    break;

  case 50: /* expr: expr TOKEN_GTE expr  */
#line 358 "wms_parser.y"
                {
			(yyval.expr) = wms_make_expr_with_gte((yyvsp[-2].expr), (yyvsp[0].expr));
			debug("gte expr");
		}
#line 1936 "wms_parser.tab.c"
    break;

  case 51: /* expr: expr TOKEN_EQ expr  */
#line 363 "wms_parser.y"
                {
			(yyval.expr) = wms_make_expr_with_eq((yyvsp[-2].expr), (yyvsp[0].expr));
			debug("eq expr");
		}
#line 1945 "wms_parser.tab.c"
    break;

  case 52: /* expr: expr TOKEN_PLUS expr  */
#line 368 "wms_parser.y"
                {
			(yyval.expr) = wms_make_expr_with_plus((yyvsp[-2].expr), (yyvsp[0].expr));
			debug("add expr");
		}
#line 1954 "wms_parser.tab.c"
    break;

  case 53: /* expr: expr TOKEN_MINUS expr  */
#line 373 "wms_parser.y"
                {
			(yyval.expr) = wms_make_expr_with_minus((yyvsp[-2].expr), (yyvsp[0].expr));
			debug("sub expr");
		}
#line 1963 "wms_parser.tab.c"
    break;

  case 54: /* expr: expr TOKEN_MUL expr  */
#line 378 "wms_parser.y"
                {
			(yyval.expr) = wms_make_expr_with_mul((yyvsp[-2].expr), (yyvsp[0].expr));
			debug("mul expr");
		}
#line 1972 "wms_parser.tab.c"
    break;

  case 55: /* expr: expr TOKEN_DIV expr  */
#line 383 "wms_parser.y"
                {
			(yyval.expr) = wms_make_expr_with_div((yyvsp[-2].expr), (yyvsp[0].expr));
			debug("div expr");
		}
#line 1981 "wms_parser.tab.c"
    break;

  case 56: /* expr: TOKEN_MINUS expr  */
#line 388 "wms_parser.y"
                {
			(yyval.expr) = wms_make_expr_with_neg((yyvsp[0].expr));
			debug("neg expr");
		}
#line 1990 "wms_parser.tab.c"
    break;

  case 57: /* expr: TOKEN_LPAR expr TOKEN_RPAR  */
#line 393 "wms_parser.y"
                {
			(yyval.expr) = (yyvsp[-1].expr);
			debug("(expr) expr");
		}
#line 1999 "wms_parser.tab.c"
    break;

  case 58: /* term: TOKEN_INT  */
#line 399 "wms_parser.y"
                {
			(yyval.term) = wms_make_term_with_int((yyvsp[0].ival));
			debug("int term");
		}
#line 2008 "wms_parser.tab.c"
    break;

  case 59: /* term: TOKEN_FLOAT  */
#line 404 "wms_parser.y"
                {
			(yyval.term) = wms_make_term_with_float((yyvsp[0].fval));
			debug("float term");
		}
#line 2017 "wms_parser.tab.c"
    break;

  case 60: /* term: TOKEN_STR  */
#line 409 "wms_parser.y"
                {
			(yyval.term) = wms_make_term_with_str((yyvsp[0].sval));
			debug("str term");
		}
#line 2026 "wms_parser.tab.c"
    break;

  case 61: /* term: TOKEN_SYMBOL  */
#line 414 "wms_parser.y"
                {
			(yyval.term) = wms_make_term_with_symbol((yyvsp[0].sval));
			debug("symbol term");
		}
#line 2035 "wms_parser.tab.c"
    break;

  case 62: /* term: TOKEN_SYMBOL TOKEN_LARR expr TOKEN_RARR  */
#line 419 "wms_parser.y"
                {
			(yyval.term) = wms_make_term_with_array((yyvsp[-3].sval), (yyvsp[-1].expr));
			debug("array[subscr]");
		}
#line 2044 "wms_parser.tab.c"
    break;

  case 63: /* term: TOKEN_SYMBOL TOKEN_LPAR TOKEN_RPAR  */
#line 424 "wms_parser.y"
                {
			(yyval.term) = wms_make_term_with_call((yyvsp[-2].sval), NULL);
			debug("call() term");
		}
#line 2053 "wms_parser.tab.c"
    break;

  case 64: /* term: TOKEN_SYMBOL TOKEN_LPAR arg_list TOKEN_RPAR  */
#line 429 "wms_parser.y"
                {
			(yyval.term) = wms_make_term_with_call((yyvsp[-3].sval), (yyvsp[-1].arg_list));
			debug("call(param_list) term");
		}
#line 2062 "wms_parser.tab.c"
    break;

  case 65: /* arg_list: expr  */
#line 435 "wms_parser.y"
                {
			(yyval.arg_list) = wms_make_arg_list(NULL, (yyvsp[0].expr));
			debug("single expr arg_list");
		}
#line 2071 "wms_parser.tab.c"
    break;

  case 66: /* arg_list: arg_list TOKEN_COMMA expr  */
#line 440 "wms_parser.y"
                {
			(yyval.arg_list) = wms_make_arg_list((yyvsp[-2].arg_list), (yyvsp[0].expr));
			debug("multiple expr param_list");
		}
#line 2080 "wms_parser.tab.c"
    break;


#line 2084 "wms_parser.tab.c"

      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", YY_CAST (yysymbol_kind_t, yyr1[yyn]), &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;

  *++yyvsp = yyval;
  *++yylsp = yyloc;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */
  {
    const int yylhs = yyr1[yyn] - YYNTOKENS;
    const int yyi = yypgoto[yylhs] + *yyssp;
    yystate = (0 <= yyi && yyi <= YYLAST && yycheck[yyi] == *yyssp
               ? yytable[yyi]
               : yydefgoto[yylhs]);
  }

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYSYMBOL_YYEMPTY : YYTRANSLATE (yychar);
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
      yyerror (YY_("syntax error"));
    }

  yyerror_range[1] = yylloc;
  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
         error, discard it.  */

      if (yychar <= YYEOF)
        {
          /* Return failure if at end of input.  */
          if (yychar == YYEOF)
            YYABORT;
        }
      else
        {
          yydestruct ("Error: discarding",
                      yytoken, &yylval, &yylloc);
          yychar = YYEMPTY;
        }
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:
  /* Pacify compilers when the user code never invokes YYERROR and the
     label yyerrorlab therefore never appears in user code.  */
  if (0)
    YYERROR;
  ++yynerrs;

  /* Do not reclaim the symbols of the rule whose action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

  /* Pop stack until we find a state that shifts the error token.  */
  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYSYMBOL_YYerror;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYSYMBOL_YYerror)
            {
              yyn = yytable[yyn];
              if (0 < yyn)
                break;
            }
        }

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
        YYABORT;

      yyerror_range[1] = *yylsp;
      yydestruct ("Error: popping",
                  YY_ACCESSING_SYMBOL (yystate), yyvsp, yylsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  yyerror_range[2] = yylloc;
  ++yylsp;
  YYLLOC_DEFAULT (*yylsp, yyerror_range, 2);

  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", YY_ACCESSING_SYMBOL (yyn), yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturnlab;


/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturnlab;


/*-----------------------------------------------------------.
| yyexhaustedlab -- YYNOMEM (memory exhaustion) comes here.  |
`-----------------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  goto yyreturnlab;


/*----------------------------------------------------------.
| yyreturnlab -- parsing is finished, clean up and return.  |
`----------------------------------------------------------*/
yyreturnlab:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval, &yylloc);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  YY_ACCESSING_SYMBOL (+*yyssp), yyvsp, yylsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif

  return yyresult;
}

#line 445 "wms_parser.y"


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
