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

#ifndef YY_YY_Y_TAB_H_INCLUDED
# define YY_YY_Y_TAB_H_INCLUDED
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
    NAME = 258,                    /* NAME  */
    INT = 259,                     /* INT  */
    STRING = 260,                  /* STRING  */
    TYPE = 261,                    /* TYPE  */
    TERNARY = 262,                 /* TERNARY  */
    COLON = 263,                   /* COLON  */
    INBLOCK = 264,                 /* INBLOCK  */
    ENDBLOCK = 265,                /* ENDBLOCK  */
    STARTBLOCK = 266,              /* STARTBLOCK  */
    COMMA = 267,                   /* COMMA  */
    IF = 268,                      /* IF  */
    WHILE = 269,                   /* WHILE  */
    REPEAT = 270,                  /* REPEAT  */
    ELSE = 271,                    /* ELSE  */
    OTHERWISE = 272,               /* OTHERWISE  */
    VAR = 273,                     /* VAR  */
    IMPLICIT = 274,                /* IMPLICIT  */
    RETURN = 275,                  /* RETURN  */
    FUN = 276,                     /* FUN  */
    OPEN_PARENT = 277,             /* OPEN_PARENT  */
    CLOSE_PARENT = 278,            /* CLOSE_PARENT  */
    SEMICOLON = 279,               /* SEMICOLON  */
    ASSIGN = 280,                  /* ASSIGN  */
    OR = 281,                      /* OR  */
    AND = 282,                     /* AND  */
    BOR = 283,                     /* BOR  */
    XOR = 284,                     /* XOR  */
    BAND = 285,                    /* BAND  */
    EQ = 286,                      /* EQ  */
    NE = 287,                      /* NE  */
    LT = 288,                      /* LT  */
    LE = 289,                      /* LE  */
    GT = 290,                      /* GT  */
    GE = 291,                      /* GE  */
    PLUS = 292,                    /* PLUS  */
    MINUS = 293,                   /* MINUS  */
    MUL = 294,                     /* MUL  */
    DIV = 295,                     /* DIV  */
    REM = 296,                     /* REM  */
    NOT = 297                      /* NOT  */
  };
  typedef enum yytokentype yytoken_kind_t;
#endif
/* Token kinds.  */
#define YYEMPTY -2
#define YYEOF 0
#define YYerror 256
#define YYUNDEF 257
#define NAME 258
#define INT 259
#define STRING 260
#define TYPE 261
#define TERNARY 262
#define COLON 263
#define INBLOCK 264
#define ENDBLOCK 265
#define STARTBLOCK 266
#define COMMA 267
#define IF 268
#define WHILE 269
#define REPEAT 270
#define ELSE 271
#define OTHERWISE 272
#define VAR 273
#define IMPLICIT 274
#define RETURN 275
#define FUN 276
#define OPEN_PARENT 277
#define CLOSE_PARENT 278
#define SEMICOLON 279
#define ASSIGN 280
#define OR 281
#define AND 282
#define BOR 283
#define XOR 284
#define BAND 285
#define EQ 286
#define NE 287
#define LT 288
#define LE 289
#define GT 290
#define GE 291
#define PLUS 292
#define MINUS 293
#define MUL 294
#define DIV 295
#define REM 296
#define NOT 297

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 41 "parser.y"

    stmt_node * statement;
    list  * stmt;
    exp_node * exp;
	int ival;
    char * type;
    char * name;



    char * str;
    vardec_node * var;
    list * vars;
    fundec_node * func;
    list * funcs;
    list * funcCallParam;
    list * args;
    list * param;
	

#line 172 "y.tab.h"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;


int yyparse (void);


#endif /* !YY_YY_Y_TAB_H_INCLUDED  */
