%{
#include <iostream>
#include "lexer.hpp"
%}

%token    EOF_T
%token    ARRAY OF
%token    DISPOSE NEW CARET AT
%token    BEGIN_ST DO END IF THEN ELSE WHILE 
%token    AND OR NOT UPLUS UMINUS
%token    BOOLEAN CHAR INTEGER REAL
%token    FORWARD FUNCTION PROCEDURE PROGRAM RESULT RETURN
%token    VAR ASSIGN SEMI_COLON DOT COLON COMMA LABEL
%token    GOTO

%token    IDENTIFIER INT_CONST REAL_CONST CHAR_CONST STRING_LITERAL
%token    TRUE FALSE NIL

%token    PLUS MINUS MUL DIV INT_DIV MOD
%token    EQUAL NOT_EQUAL GT LT GE LE
%token    OP_PAR CLOS_PAR OP_BRACK CLOS_BRACK

%left     PLUS MINUS
%left     MUL DIV INT_DIV MOD
%left     UMINUS UPLUS
%nonassoc EQUAL NOT_EQUAL GT LT GE LE

%expect   1

%start program
 
%%

program:
    PROGRAM IDENTIFIER SEMI_COLON body DOT
;

body:
    next_local block
;

next_local:
    next_local local
|   /* empty */
;

local:
    VAR IDENTIFIER next_id COLON type SEMI_COLON next_var
|   LABEL IDENTIFIER next_id SEMI_COLON
|   header SEMI_COLON body SEMI_COLON
|   FORWARD header SEMI_COLON
;

next_var:
    IDENTIFIER next_id COLON type SEMI_COLON next_var
|   /* empty */
;

next_id:
    COMMA IDENTIFIER next_id
|   /* empty */
;

header:
    PROCEDURE IDENTIFIER OP_PAR optional_arguments CLOS_PAR
|   FUNCTION IDENTIFIER OP_PAR optional_arguments CLOS_PAR COLON type
;

optional_arguments:
    formal next_arg
|   /* empty */
;

next_arg:
    SEMI_COLON formal next_arg
|   /* empty */
;

formal:
    optional_var IDENTIFIER next_id COLON type
;

optional_var:
    VAR
|   /* empty */
;

type:
    INTEGER
|   REAL
|   BOOLEAN
|   CHAR
|   ARRAY optional_size OF type
|   CARET type
;

optional_size:
    OP_BRACK INT_CONST CLOS_BRACK
|   /* empty */
;

block:
    BEGIN_ST stmt next_stmt END
;

next_stmt:
    SEMI_COLON stmt next_stmt
|   /* empty */
;

stmt:
    /* empty */
|   l_value ASSIGN expr
|   block
|   call
|   IF expr THEN stmt optional_else
|   WHILE expr DO stmt
|   IDENTIFIER COLON stmt
|   GOTO IDENTIFIER
|   RETURN
|   NEW optional_expr l_value
|   DISPOSE optional_bracket l_value
;

optional_else:
    ELSE stmt
|   /* empty */
;

optional_expr:
    OP_BRACK expr CLOS_BRACK
|   /* empty */
;

optional_bracket:
    OP_BRACK CLOS_BRACK
|   /* empty */
;

expr:
    l_value
|   r_value
;

l_value:
    IDENTIFIER
|   RESULT
|   STRING_LITERAL
|   l_value OP_BRACK expr CLOS_BRACK
|   expr CARET
|   OP_PAR l_value CLOS_PAR
;

r_value:
    INT_CONST
|   TRUE
|   FALSE
|   REAL_CONST
|   CHAR_CONST
|   NIL
|   OP_PAR r_value CLOS_PAR
|   call
|   AT l_value
|   unop expr
|   expr PLUS expr
|   expr MINUS expr
|   expr MUL expr
|   expr DIV expr
|   expr INT_DIV expr
|   expr MOD expr
|   expr OR expr
|   expr AND expr
|   expr EQUAL expr
|   expr NOT_EQUAL expr
|   expr LT expr
|   expr LE expr
|   expr GT expr
|   expr GE expr
;

call:
    IDENTIFIER OP_PAR optional_parameters CLOS_PAR
;

optional_parameters:
    IDENTIFIER OP_PAR expr next_expr CLOS_PAR
|   /* empty */
;

next_expr:
    COMMA expr next_expr
|   /* empty */
;

unop:
    NOT | UPLUS | UMINUS
;

%%

int main() {
    int result = yyparse();
    if (result == 0) std::cout << "Parsing successful.\n";
    return result;
}
