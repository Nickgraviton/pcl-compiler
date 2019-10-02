%flex

%token    EOF_T
%token    ARRAY OF
%token    DISPOSE NEW CARET AT
%token    BEGIN DO END IF THEN ELSE WHILE 
%token    AND OR NOT
%token    BOOLEAN CHAR INTEGER REAL
%token    FORWARD FUNCTION PROCEDURE PROGRAM RESULT RETURN
%token    VAR ASSIGN SEMI_COLON DOT COLON COMMA LABEL
%token    GOTO

%token    IDENTIFIER INT_CONST REAL_CONST CHAR_CONST STRING_LITERAL
%token    TRUE FALSE NIL

%token    PLUS MINUS MUL DIV INT_DIV MOD
%token    EQUAL NOT_EQUAL GT LT GE LE
%token    OP_PAR CLOS_PAR OP_BRACK CLOS_BRACK

%start program
 
%%

program:
    PROGRAM IDENTIFIER SEMI_COLON body DOT
;

body:
    block
|   local body
;

local:
    VAR IDENTIFIER next_id COLON type SEMI_COLON decl
|   LABEL IDENTIFIER next_id SEMI_COLON
|   header SEMI_COLON body SEMI_COLON
|   FORWARD header SEMI_COLON
;

decl:
    IDENTIFIER next_id COLON type SEMI_COLON decl
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
    VAR IDENTIFIER next_id COLON type
|   IDENTIFIER next_id COLON type
;

type:
    INTEGER
|   REAL
|   BOOLEAN
|   CHAR
|   ARRAY OF type
|   ARRAY OP_BRACK INT_CONST CLOS_BRACK OF type
|   CARET type
;

block:
    BEGIN stmt next_stmt END
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
|   if_stmt
|   WHILE expr DO stmt
|   IDENTIFIER COLON stmt
|   GOTO IDENTIFIER
|   RETURN
|   NEW l_value
|   NEW OP_BRACK expr CLOS_BRACK l_value
|   DISPOSE l_value
|   DISPOSE OP_BRACK CLOS_BRACK l_value
;

full_stmt:
    /* empty */
|   l_value ASSIGN expr
|   block
|   call
|   full_if
|   WHILE expr DO stmt
|   IDENTIFIER COLON stmt
|   GOTO IDENTIFIER
|   RETURN
|   NEW l_value
|   NEW OP_BRACK expr CLOS_BRACK l_value
|   DISPOSE l_value
|   DISPOSE OP_BRACK CLOS_BRACK l_value
;

full_if:
    IF expr THEN full_stmt ELSE full_stmt
;

if_stmt:
    IF expr THEN full_stmt ELSE stmt
|   IF expr THEN stmt
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
    const
|   OP_PAR r_value CLOS_PAR
|   call
|   AT l_value
|   unop expr %prec UMINUS
|   expr binop expr
;

const:
    INT_CONST
|   TRUE
|   FALSE
|   REAL_CONST
|   CHAR_CONST
|   NIL
;

call:
    IDENTIFIER OP_PAR CLOS_PAR
|   IDENTIFIER OP_PAR expr next_expr CLOS_PAR
;

next_expr:
    COMMA expr next_expr
|   /* empty */
;

unop:
    NOT | UPLUS | UMINUS
;

binop:
    PLUS | MINUS | MUL| DIV | INT_DIV | MOD | OR | AND
|   EQUAL | NOT_EQUAL | LT | LE | GT | GE
;
