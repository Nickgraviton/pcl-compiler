/* A Bison parser, made by GNU Bison 3.0.4.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

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

#ifndef YY_YY_PARSER_HPP_INCLUDED
# define YY_YY_PARSER_HPP_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    EOF_T = 258,
    ARRAY = 259,
    OF = 260,
    DISPOSE = 261,
    NEW = 262,
    CARET = 263,
    AT = 264,
    BEGIN_ST = 265,
    DO = 266,
    END = 267,
    IF = 268,
    THEN = 269,
    ELSE = 270,
    WHILE = 271,
    AND = 272,
    OR = 273,
    NOT = 274,
    BOOLEAN = 275,
    CHAR = 276,
    INTEGER = 277,
    REAL = 278,
    FORWARD = 279,
    FUNCTION = 280,
    PROCEDURE = 281,
    PROGRAM = 282,
    RESULT = 283,
    RETURN = 284,
    VAR = 285,
    ASSIGN = 286,
    SEMI_COLON = 287,
    DOT = 288,
    COLON = 289,
    COMMA = 290,
    LABEL = 291,
    GOTO = 292,
    IDENTIFIER = 293,
    INT_CONST = 294,
    REAL_CONST = 295,
    CHAR_CONST = 296,
    STRING_LITERAL = 297,
    TRUE = 298,
    FALSE = 299,
    NIL = 300,
    PLUS = 301,
    MINUS = 302,
    MUL = 303,
    DIV = 304,
    INT_DIV = 305,
    MOD = 306,
    EQUAL = 307,
    NOT_EQUAL = 308,
    GT = 309,
    LT = 310,
    GE = 311,
    LE = 312,
    OP_PAR = 313,
    CLOS_PAR = 314,
    OP_BRACK = 315,
    CLOS_BRACK = 316,
    UNOP = 317,
    R_VAL = 318
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef int YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_PARSER_HPP_INCLUDED  */
