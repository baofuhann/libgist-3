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

#ifndef YY_YY_CMDLINE_GRAM_TAB_H_INCLUDED
# define YY_YY_CMDLINE_GRAM_TAB_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
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
    ID = 258,                      /* ID  */
    STRCONST = 259,                /* STRCONST  */
    INTCONST = 260,                /* INTCONST  */
    IOPARAM = 261,                 /* IOPARAM  */
    FLOATCONST = 262,              /* FLOATCONST  */
    CREATE = 263,                  /* CREATE  */
    INSERT = 264,                  /* INSERT  */
    OPEN = 265,                    /* OPEN  */
    CLOSE = 266,                   /* CLOSE  */
    QUIT = 267,                    /* QUIT  */
    SELECT = 268,                  /* SELECT  */
    DELETE = 269,                  /* DELETE  */
    NL = 270,                      /* NL  */
    ERROR = 271,                   /* ERROR  */
    CHECK = 272,                   /* CHECK  */
    SPLIT = 273,                   /* SPLIT  */
    HELP = 274,                    /* HELP  */
    DUMP = 275,                    /* DUMP  */
    ONLY = 276,                    /* ONLY  */
    SET = 277,                     /* SET  */
    STRUCT = 278,                  /* STRUCT  */
    WRITE = 279,                   /* WRITE  */
    ECHO_TOKEN = 280,              /* ECHO_TOKEN  */
    EVALSPLIT = 281,               /* EVALSPLIT  */
    EVALPENALTY = 282,             /* EVALPENALTY  */
    LOADFILE = 283,                /* LOADFILE  */
    PREDINFO = 284,                /* PREDINFO  */
    CREATEANL = 285,               /* CREATEANL  */
    OPENANL = 286,                 /* OPENANL  */
    CLOSEANL = 287,                /* CLOSEANL  */
    WKLDSTATS = 288,               /* WKLDSTATS  */
    SPLITSTATS = 289,              /* SPLITSTATS  */
    PENALTYSTATS = 290,            /* PENALTYSTATS  */
    ANLINFO = 291                  /* ANLINFO  */
  };
  typedef enum yytokentype yytoken_kind_t;
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 18 "cmdline_gram.y"

    char *string;
    shpid_t pgno;
    int number;
    float flt;

#line 107 "cmdline_gram.tab.h"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;


int yyparse (void);


#endif /* !YY_YY_CMDLINE_GRAM_TAB_H_INCLUDED  */
