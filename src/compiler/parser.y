%{
#include <iostream>
#include <memory>

#include "ast.hpp"
#include "lexer.hpp"

std::unique_ptr<Program> root;
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
    PROGRAM IDENTIFIER SEMI_COLON body DOT { $$ = root = std::make_unique<Program>($2, std::move($4)); }
;

body:
    next_local block { $$ = std::make_unique<Body>(std::move($1), std::move($2)); }
;

next_local:
    next_local local { $$ = std::move($1); $$.push_back(std::move($2)); }
|   /* empty */      { $$ = std::vector<std::unique_ptr<Local>>();      }
;

local:
    VAR next_id COLON type SEMI_COLON next_var { auto var_names = std::move($6);
                                                 var_names.push_back(std::make_unique<VarNames>($2, $4));
                                                 $$ = std::make_unique<VarDecl>(std::move(var_names));         }
|   LABEL next_id SEMI_COLON                   { $$ = std::make_unique<LabelDecl>($2);                         }
|   header SEMI_COLON body SEMI_COLON          { $$ = std::move($1); $$->set_body($3); $$->set_forward(false); }
|   FORWARD header SEMI_COLON                  { $$ = std::move($2); $$->set_forward(true);                    }
;

next_var:
    next_var next_id COLON type SEMI_COLON { $$ = std::move($1); $$.push_back(std::make_unique<VarNames>($2, $4); }
|   /* empty */                            { $$ = std::vector<std::unique_ptr<VarNames>>();                       }
;

next_id:
    IDENTIFIER               { $$ = std::vector<std::string>(); $$.push_back($1); }
|   next_id COMMA IDENTIFIER { $$ = $1; $$.push_back($3);                         }
|   /* empty */              { $$ = std::vector<std::string>();                   }
;

header:
    PROCEDURE IDENTIFIER OP_PAR optional_arguments CLOS_PAR           { $$ = std::make_unique<Fun>($2, $4, "None"); }
|   FUNCTION IDENTIFIER OP_PAR optional_arguments CLOS_PAR COLON type { $$ = std::make_unique<Fun>($2, $4, $6);     }
;

optional_arguments:
    formal next_arg { $$ = std::move($2); $$.push_back(std::move($1)); }
|   /* empty */     { $$ = std::vector<std::unique_ptr<Formal>>();     }
;

next_arg:
    next_arg SEMI_COLON formal { $$ = std::move($1); $$.push_back(std::move($3)); }
|   /* empty */                { $$ = std::vector<std::unique_ptr<Formal>>();     }
;

formal:
    optional_var next_id COLON type { $$ = std::make_unique<Formal>($1, $2, $4); }
;

optional_var:
    VAR         { $$ = true;  }
|   /* empty */ { $$ = false; }
;

type:
    INTEGER                     { $$ = "integer";         }
|   REAL                        { $$ = "real";            }
|   BOOLEAN                     { $$ = "boolean";         }
|   CHAR                        { $$ = "char";            }
|   ARRAY optional_size OF type { $$ = "array of" + type; }
|   CARET type                  { $$ = "caret";           }
;

optional_size:
    OP_BRACK INT_CONST CLOS_BRACK { $$ = $2 }
|   /* empty */                   { $$ = 0  }
;

block:
    BEGIN_ST next_stmt END { $$ = std::make_unique<Block>(std::move($2), nullptr); }
;

next_stmt:
    stmt                      { $$ = std::vector<std::unique_ptr<Stmt>>(); $$.push_back(std::move($1)); }
|   next_stmt SEMI_COLON stmt { $$ = std::move($1); $$.push_back(std::move($3));                        }
|   /* empty */               { $$ = std::vector<std::unique_ptr<Stmt>>();                              }
;

stmt:
    l_value ASSIGN expr              { $$ = std::make_unique<VarAssign>(std::move($1), std::move($3));         }
|   block                            { $$ = std::move($1);                                                     }
|   call                             { $$ = std::move($1);                                                     }
|   IF expr THEN stmt                { $$ = std::make_unique<If>(std::move($2), std::move($4), nullptr);       }
|   IF expr THEN stmt ELSE stmt      { $$ = std::make_unique<If>(std::move($2), std::move($4), std::move($6)); }
|   WHILE expr DO stmt               { $$ = std::make_unique<While>(std::move($2), std::move($4));             }
|   IDENTIFIER COLON stmt            { $$ = std::make_unique<Label>($1, std::move($3));                        }
|   GOTO IDENTIFIER                  { $$ = std::make_unique<Goto>($2);                                        }
|   RETURN                           { $$ = std::make_unique<Return>();                                        }
|   NEW optional_expr l_value        { $$ = std::make_unique<New>(std::move($2), std::move($3));               }
|   DISPOSE optional_bracket l_value { $$ = std::make_unique<Dispose>(std::move($2), std::move($3));           }
;

optional_expr:
    OP_BRACK expr CLOS_BRACK { $$ = std::move($2);                }
|   /* empty */              { $$ = std::make_unique<Integer>(0); } // Perhaps change a 0 integer with some other way of signifying the absence of brackets
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
    IDENTIFIER                       { $$ = std::make_unique<Variable>($1, false, 0);                                      }
|   RESULT                           { $$ = std::make_unique<Result>();                                                    }
|   STRING_LITERAL                   { $$ = std::make_unique<String>($1);                                                  }
|   l_value OP_BRACK expr CLOS_BRACK { $$ = std::make_unique<Variable>(std::move($1), false, std::move($3));               }
|   expr CARET                       { $$ = std::make_unique<Variable>(std::move($1), true, std::make_unique<Integer>(0)); } // Pass some value other than an expression within brackets
|   OP_PAR l_value CLOS_PAR          { $$ = std::move($2);                                                                 }
;

r_value:
    INT_CONST               { $$ = std::make_unique<Integer>($1);                                   }
|   TRUE                    { $$ = std::make_unique<Boolean>($1);                                   }
|   FALSE                   { $$ = std::make_unique<Boolean>($1);                                   }
|   REAL_CONST              { $$ = std::make_unique<Real>($1);                                      }
|   CHAR_CONST              { $$ = std::make_unique<Char>($1);                                      }
|   NIL                     { $$ = std::make_unique<Nil>();                                         }
|   OP_PAR r_value CLOS_PAR { $$ = std::move($2);                                                   }
|   call                    { $$ = std::move($1);                                                   }
|   AT expr                 { $$ = std::make_unique<Variable>(std::move($2), true, nullptr);        }
    /* Correct rule is `AT l_value` but l_value creates reduce/reduce
       conflict and since r_value is nonassoc we can just leave this as expr*/
|   unop expr %prec UNOP    { $$ = std::make_unique<UnaryOp>($1, std::move($2));                    }
|   expr PLUS expr          { $$ = std::make_unique<BinaryOp>("+", std::move($1), std::move($3));   }
|   expr MINUS expr         { $$ = std::make_unique<BinaryOp>("-", std::move($1), std::move($3));   }
|   expr MUL expr           { $$ = std::make_unique<BinaryOp>("*", std::move($1), std::move($3));   }
|   expr DIV expr           { $$ = std::make_unique<BinaryOp>("/", std::move($1), std::move($3));   }
|   expr INT_DIV expr       { $$ = std::make_unique<BinaryOp>("div", std::move($1), std::move($3)); }
|   expr MOD expr           { $$ = std::make_unique<BinaryOp>("mod", std::move($1), std::move($3)); }
|   expr OR expr            { $$ = std::make_unique<BinaryOp>("or", std::move($1), std::move($3));  }
|   expr AND expr           { $$ = std::make_unique<BinaryOp>("and", std::move($1), std::move($3)); }
|   expr EQUAL expr         { $$ = std::make_unique<BinaryOp>("=", std::move($1), std::move($3));   }
|   expr NOT_EQUAL expr     { $$ = std::make_unique<BinaryOp>("<>", std::move($1), std::move($3));  }
|   expr LT expr            { $$ = std::make_unique<BinaryOp>("<", std::move($1), std::move($3));   }
|   expr LE expr            { $$ = std::make_unique<BinaryOp>("<=", std::move($1), std::move($3));  }
|   expr GT expr            { $$ = std::make_unique<BinaryOp>(">", std::move($1), std::move($3));   }
|   expr GE expr            { $$ = std::make_unique<BinaryOp>(">=", std::move($1), std::move($3));  }
;

call:
    IDENTIFIER OP_PAR optional_parameters CLOS_PAR { $$ = std::make_unique<Call>($1, std::move($3)); }
;

optional_parameters:
    expr                           { $$ = std::vector<unique_ptr<Expr>>(); $$.push_back(std::move($1)); }
|   optional_parameters COMMA expr { $$ = std::move($1); $$.push_back(std::move($3));                   }
|   /* empty */                    { $$ = std::vector<unique_ptr<Expr>>();                              }
;

unop:
    NOT   { $$ = "not"; }
|   PLUS  { $$ = "-";   }
|   MINUS { $$ = "+";   }
;

%%
