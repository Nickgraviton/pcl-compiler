%{
#include <iostream>
#include <memory>

#include "ast.hpp"
#include "lexer.hpp"

std::unique_ptr<Block> root;
%}

%define parse.error verbose

%token    ARRAY OF
%token    DISPOSE NEW CARET AT
%token    BEGIN_ST DO END IF THEN ELSE WHILE 
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

%left     EQUAL NOT_EQUAL GT LT GE LE
%left     PLUS MINUS OR
%left     MUL DIV INT_DIV MOD AND
%nonassoc NOT CARET UNOP R_VAL AT

%expect   1

%start program
 
%%

program:
    PROGRAM IDENTIFIER SEMI_COLON body DOT { std::make_unique<Block>(); }
;

body:
    next_local block
;

next_local:
    next_local local
|   /* empty */
;

local:
    VAR next_id COLON type SEMI_COLON next_var { $$ = $6;
                                                 $$.push_back(std::make_unique<VarDecl>($2, $4)); }
|   LABEL next_id SEMI_COLON
|   header SEMI_COLON body SEMI_COLON
|   FORWARD header SEMI_COLON
;

next_var:
    next_var next_id COLON type SEMI_COLON { $$ = $1;
                                             $$.push_back(std::make_unique<VarDecl>($2, $4);  }
|   /* empty */                            { $$ = std::vector<std::unique_ptr<VarDecl>>();    }
;

next_id:
    IDENTIFIER               { $$ = std::vector<std::string>(); $$.push_back($1); }
|   next_id COMMA IDENTIFIER { $$ = $1; $$.push_back($3);                         }
|   /* empty */              { $$ = std::vector<std::string>();                   }
;

header:
    PROCEDURE IDENTIFIER OP_PAR optional_arguments CLOS_PAR           { $$ = std::make_unique<Fun>($2, $4, "None"); }
|   FUNCTION IDENTIFIER OP_PAR optional_arguments CLOS_PAR COLON type { $$ = std::make_unique<Fun>($2, $4, $6); }
;

optional_arguments:
    formal next_arg { $$ = $2; $$.push_back($1);                   }
|   /* empty */     { $$ = std::vector<std::unique_ptr<Formal>>(); }
;

next_arg:
    next_arg SEMI_COLON formal { $$ = $1; $$.push_back($3);                   }
|   /* empty */                { $$ = std::vector<std::unique_ptr<Formal>>(); }
;

formal:
    optional_var next_id COLON type { $$ = std::make_unique<Formal>($1, $2, $4); }
;

optional_var:
    VAR         { $$ = true;  }
|   /* empty */ { $$ = false; }
;

type:
    INTEGER {}
|   REAL {}
|   BOOLEAN {}
|   CHAR {}
|   ARRAY optional_size OF type {}
|   CARET type {}
;

optional_size:
    OP_BRACK INT_CONST CLOS_BRACK { $$ = $2 }
|   /* empty */                   { $$ = 0  }
;

block:
    BEGIN_ST next_stmt END { $$ = std::make_unique<Block>($2, nullptr); }
;

next_stmt:
    stmt                      { $$ = std::vector<Stmt>(); $$.push_back($1); }
|   next_stmt SEMI_COLON stmt { $$ = $1; $$.push_back($3);                  }
|   /* empty */               { $$ = std::vector<Stmt>();                   }
;

stmt:
    l_value ASSIGN expr              { $$ = std::make_unique<VarAssign>($1, $3);   }
|   block                            { $$ = $1;                                    }
|   call                             { $$ = $1;                                    }
|   IF expr THEN stmt                { $$ = std::make_unique<If>($2, $4, nullptr); }
|   IF expr THEN stmt ELSE stmt      { $$ = std::make_unique<If>($2, $4, $6);      }
|   WHILE expr DO stmt               { $$ = std::make_unique<While>($2, $4);       }
|   IDENTIFIER COLON stmt            { $$ = std::make_unique<Label>($1, $3);       }
|   GOTO IDENTIFIER                  { $$ = std::make_unique<Goto>($2);            }
|   RETURN                           { $$ = std::make_unique<Return>();            }
|   NEW optional_expr l_value        { $$ = std::make_unique<New>($2, $3);         }
|   DISPOSE optional_bracket l_value { $$ = std::make_unique<Dispose>($2, $3);     }
|   /* empty */                      { $$ = std::vector<unique_ptr<Stmt>>();       }
;

optional_expr:
    OP_BRACK expr CLOS_BRACK { $$ = $1; }
|   /* empty */              { $$ = 0;  }
;

optional_bracket:
    OP_BRACK CLOS_BRACK { $$ = true;  }
|   /* empty */         { $$ = false; }
;

expr:
    l_value             { $$ = $1; }
|   r_value %prec R_VAL { $$ = $1; }
;

l_value:
    IDENTIFIER                       { $$ = std::make_unique<Variable>($1, false, 0);  }
|   RESULT
|   STRING_LITERAL                   { $$ = std::make_unique<String>($1);              }
|   l_value OP_BRACK expr CLOS_BRACK { $$ = std::make_unique<Variable>($1, false, $3); }
|   expr CARET                       { $$ = std::make_unique<Variable>($1, true, 0);   }
|   OP_PAR l_value CLOS_PAR          { $$ = $2;                                        }
;

r_value:
    INT_CONST               { $$ = std::make_unique<Integer>($1);                 }
|   TRUE                    { $$ = std::make_unique<Boolean>($1);                 }
|   FALSE                   { $$ = std::make_unique<Boolean>($1);                 }
|   REAL_CONST              { $$ = std::make_unique<Real>($1);                    }
|   CHAR_CONST              { $$ = std::make_unique<Char>($1);                    }
|   NIL                     { $$ = std::make_unique<Nil>;                         }
|   OP_PAR r_value CLOS_PAR { $$ = $2;                                            }
|   call                    { $$ = $1;                                            }
|   AT expr                 { $$ = std::make_unique<Variable>($2, true, nullptr); }
    /* Correct rule is `AT l_value` but l_value creates reduce/reduce
       conflict and since r_value is nonassoc we can just leave this as expr*/
|   unop expr %prec UNOP    { $$ = std::make_unique<UnaryOp>($1, $2);             }
|   expr PLUS expr          { $$ = std::make_unique<BinaryOp>("+", $1, $3);       }
|   expr MINUS expr         { $$ = std::make_unique<BinaryOp>("-", $1, $3);       }
|   expr MUL expr           { $$ = std::make_unique<BinaryOp>("*", $1, $3);       }
|   expr DIV expr           { $$ = std::make_unique<BinaryOp>("/", $1, $3);       }
|   expr INT_DIV expr       { $$ = std::make_unique<BinaryOp>("div", $1, $3);     }
|   expr MOD expr           { $$ = std::make_unique<BinaryOp>("mod", $1, $3);     }
|   expr OR expr            { $$ = std::make_unique<BinaryOp>("or", $1, $3);      }
|   expr AND expr           { $$ = std::make_unique<BinaryOp>("and", $1, $3);     }
|   expr EQUAL expr         { $$ = std::make_unique<BinaryOp>("=", $1, $3);       }
|   expr NOT_EQUAL expr     { $$ = std::make_unique<BinaryOp>("<>", $1, $3);      }
|   expr LT expr            { $$ = std::make_unique<BinaryOp>("<", $1, $3);       }
|   expr LE expr            { $$ = std::make_unique<BinaryOp>("<=", $1, $3);      }
|   expr GT expr            { $$ = std::make_unique<BinaryOp>(">", $1, $3) ;      }
|   expr GE expr            { $$ = std::make_unique<BinaryOp>(">=", $1, $3);      }
;

call:
    IDENTIFIER OP_PAR optional_parameters CLOS_PAR { $$ = std::make_unique<Call>($1, $3); }
;

optional_parameters:
    expr                           { $$ = std::vector<unique_ptr<Expr>>(); $$.push_back($1); }
|   optional_parameters COMMA expr { $$ = $1; $$.push_back($3);                              }
|   /* empty */                    { $$ = std::vector<unique_ptr<Expr>>();                   }
;

unop:
    NOT   { $$ = "not"; }
|   PLUS  { $$ = "-";   }
|   MINUS { $$ = "+";   }
;

%%
