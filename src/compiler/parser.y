%{
#include <iostream>
#include <memory>
#include <vector>

#include "ast.hpp"
#include "lexer.hpp"
#include "parser.hpp"

program_ptr root;
extern yy::parser::symbol_type yylex();
%}

%require "3.2"
%language "c++"
%define api.value.type variant
%define api.token.constructor
%define parse.error verbose

%token              ARRAY OF
%token              DISPOSE NEW CARET AT
%token              BEGIN_ST DO END_ST IF THEN ELSE WHILE 
%token              AND OR NOT
%token              BOOLEAN CHAR INTEGER REAL
%token              FORWARD FUNCTION PROCEDURE PROGRAM RESULT RETURN
%token              VAR ASSIGN SEMI_COLON DOT COLON COMMA LABEL
%token              GOTO

%token<char>        CHAR_CONST
%token<double>      REAL_CONST
%token<int>         INT_CONST
%token<std::string> ID
%token<std::string> STRING_LITERAL
%token              TRUE FALSE NIL

%token              PLUS MINUS MUL DIV INT_DIV MOD
%token              EQUAL NOT_EQUAL GT LT GE LE
%token              OP_PAR CLOS_PAR OP_BRACK CLOS_BRACK

%left               EQUAL NOT_EQUAL GT LT GE LE
%left               PLUS MINUS OR
%left               MUL DIV INT_DIV MOD AND
%nonassoc           NOT CARET UNOP R_VAL AT

%type<block_ptr>   block
%type<body_ptr>    body
%type<expr_ptr>    optional_expr expr l_value r_value
%type<formal_ptr>  formal
%type<fun_ptr>     header
%type<local_ptr>   local
%type<program_ptr> program
%type<stmt_ptr>    stmt

%type<std::vector<expr_ptr>>     next_parameter
%type<std::vector<formal_ptr>>   next_arg
%type<std::vector<local_ptr>>    next_local
%type<std::vector<stmt_ptr>>     next_stmt
%type<std::vector<varnames_ptr>> next_var
%type<std::vector<std::string>>  next_id

%type<bool>        optional_var optional_bracket
%type<int>         optional_size
%type<std::string> type unop

%expect 1

%start program
 
%%

program:
    PROGRAM ID SEMI_COLON body DOT { root = std::make_unique<Program>($2, std::move($4)); }
;

body:
    next_local block { $$ = std::make_unique<Body>(std::move($1), std::move($2)); }
;

next_local:
    next_local local { $$ = std::move($1); $$.push_back(std::move($2)); }
|   /* empty */      { $$ = std::vector<local_ptr>();                   }
;

local:
    VAR next_var                      { $$ = std::make_unique<VarDecl>(std::move($2));                           }
|   LABEL next_id SEMI_COLON          { $$ = std::make_unique<LabelDecl>($2);                                    }
|   header SEMI_COLON body SEMI_COLON { $1->set_body(std::move($3)); $1->set_forward(false); $$ = std::move($1); }
|   FORWARD header SEMI_COLON         { $2->set_forward(true); $$ = std::move($2);                               }
;

next_var:
    next_id COLON type SEMI_COLON          { $$ = std::vector<varnames_ptr>(); $$.push_back(std::make_unique<VarNames>($1, $3)); }
|   next_var next_id COLON type SEMI_COLON { $$ = std::move($1); $$.push_back(std::make_unique<VarNames>($2, $4));               }
;

next_id:
    ID               { $$ = std::vector<std::string>(); $$.push_back($1); }
|   next_id COMMA ID { $$ = $1; $$.push_back($3);                         }
;

header:
    PROCEDURE ID OP_PAR next_arg CLOS_PAR           { $$ = std::make_unique<Fun>($2, "None", std::move($4)); }
|   FUNCTION ID OP_PAR next_arg CLOS_PAR COLON type { $$ = std::make_unique<Fun>($2, $7, std::move($4));     }
;

next_arg:
    formal                     { $$ = std::vector<formal_ptr>(); $$.push_back(std::move($1)); }
|   next_arg SEMI_COLON formal { $$ = std::move($1); $$.push_back(std::move($3));             }
|   /* empty */                { $$ = std::vector<formal_ptr>();                              }
;

formal:
    optional_var next_id COLON type { $$ = std::make_unique<Formal>($1, $2, $4); }
;

optional_var:
    VAR         { $$ = true;  }
|   /* empty */ { $$ = false; }
;

type:
    INTEGER                     { $$ = "integer";       }
|   REAL                        { $$ = "real";          }
|   BOOLEAN                     { $$ = "boolean";       }
|   CHAR                        { $$ = "char";          }
|   ARRAY optional_size OF type { $$ = "array of" + $4; }
|   CARET type                  { $$ = "caret";         }
;

optional_size:
    OP_BRACK INT_CONST CLOS_BRACK { $$ = $2; }
|   /* empty */                   { $$ = 0;  }
;

block:
    BEGIN_ST next_stmt END_ST { $$ = std::make_unique<Block>(std::move($2)); }
;

next_stmt:
    stmt                      { $$ = std::vector<stmt_ptr>(); $$.push_back(std::move($1)); }
|   next_stmt SEMI_COLON stmt { $$ = std::move($1); $$.push_back(std::move($3));           }
;

stmt:
    l_value ASSIGN expr               { $$ = std::make_unique<VarAssign>(std::move($1), std::move($3));         }
|   block                             { $$ = std::move($1);                                                     }
|   ID OP_PAR next_parameter CLOS_PAR { $$ = std::make_unique<CallStmt>($1, std::move($3));                     }
|   IF expr THEN stmt                 { $$ = std::make_unique<If>(std::move($2), std::move($4), nullptr);       }
|   IF expr THEN stmt ELSE stmt       { $$ = std::make_unique<If>(std::move($2), std::move($4), std::move($6)); }
|   WHILE expr DO stmt                { $$ = std::make_unique<While>(std::move($2), std::move($4));             }
|   ID COLON stmt                     { $$ = std::make_unique<Label>($1, std::move($3));                        }
|   GOTO ID                           { $$ = std::make_unique<Goto>($2);                                        }
|   RETURN                            { $$ = std::make_unique<Return>();                                        }
|   NEW optional_expr l_value         { $$ = std::make_unique<New>(std::move($2), std::move($3));               }
|   DISPOSE optional_bracket l_value  { $$ = std::make_unique<Dispose>($2, std::move($3));                      }
;

optional_expr:
    OP_BRACK expr CLOS_BRACK { $$ = std::move($2); }
|   /* empty */              { $$ = nullptr;       }
;

optional_bracket:
    OP_BRACK CLOS_BRACK { $$ = true;  }
|   /* empty */         { $$ = false; }
;

expr:
    l_value             { $$ = std::move($1); }
|   r_value %prec R_VAL { $$ = std::move($1); }
;

l_value:
    ID                               { $$ = std::make_unique<Variable>($1);                        }
|   RESULT                           { $$ = std::make_unique<Result>();                            }
|   STRING_LITERAL                   { $$ = std::make_unique<String>($1);                          }
|   l_value OP_BRACK expr CLOS_BRACK { $$ = std::make_unique<Array>(std::move($1), std::move($3)); }
|   expr CARET                       { $$ = std::make_unique<Deref>(std::move($1));                }
|   OP_PAR l_value CLOS_PAR          { $$ = std::move($2);                                         }
;

r_value:
    INT_CONST                         { $$ = std::make_unique<Integer>($1);                                     }
|   TRUE                              { $$ = std::make_unique<Boolean>(true);                                   }
|   FALSE                             { $$ = std::make_unique<Boolean>(false);                                  }
|   REAL_CONST                        { $$ = std::make_unique<Real>($1);                                        }
|   CHAR_CONST                        { $$ = std::make_unique<Char>($1);                                        }
|   NIL                               { $$ = std::make_unique<Nil>();                                           }
|   OP_PAR r_value CLOS_PAR           { $$ = std::move($2);                                                     }
|   ID OP_PAR next_parameter CLOS_PAR { $$ = std::make_unique<CallExpr>($1, std::move($3));                     }
|   AT expr                           { $$ = std::make_unique<AddressOf>(std::move($2));                        }
    /* Correct rule is `AT l_value` but l_value creates reduce/reduce
       conflict and since r_value is nonassoc we can just leave this as expr*/
|   unop expr %prec UNOP              { $$ = std::make_unique<UnaryOp>($1, std::move($2));                      }
|   expr PLUS expr                    { $$ = std::make_unique<BinaryExpr>("+", std::move($1), std::move($3));   }
|   expr MINUS expr                   { $$ = std::make_unique<BinaryExpr>("-", std::move($1), std::move($3));   }
|   expr MUL expr                     { $$ = std::make_unique<BinaryExpr>("*", std::move($1), std::move($3));   }
|   expr DIV expr                     { $$ = std::make_unique<BinaryExpr>("/", std::move($1), std::move($3));   }
|   expr INT_DIV expr                 { $$ = std::make_unique<BinaryExpr>("div", std::move($1), std::move($3)); }
|   expr MOD expr                     { $$ = std::make_unique<BinaryExpr>("mod", std::move($1), std::move($3)); }
|   expr OR expr                      { $$ = std::make_unique<BinaryExpr>("or", std::move($1), std::move($3));  }
|   expr AND expr                     { $$ = std::make_unique<BinaryExpr>("and", std::move($1), std::move($3)); }
|   expr EQUAL expr                   { $$ = std::make_unique<BinaryExpr>("=", std::move($1), std::move($3));   }
|   expr NOT_EQUAL expr               { $$ = std::make_unique<BinaryExpr>("<>", std::move($1), std::move($3));  }
|   expr LT expr                      { $$ = std::make_unique<BinaryExpr>("<", std::move($1), std::move($3));   }
|   expr LE expr                      { $$ = std::make_unique<BinaryExpr>("<=", std::move($1), std::move($3));  }
|   expr GT expr                      { $$ = std::make_unique<BinaryExpr>(">", std::move($1), std::move($3));   }
|   expr GE expr                      { $$ = std::make_unique<BinaryExpr>(">=", std::move($1), std::move($3));  }
;

next_parameter:
    expr                      { $$ = std::vector<expr_ptr>(); $$.push_back(std::move($1)); }
|   next_parameter COMMA expr { $$ = std::move($1); $$.push_back(std::move($3));           }
|   /* empty */               { $$ = std::vector<expr_ptr>();                              }
;

unop:
    NOT   { $$ = "not"; }
|   PLUS  { $$ = "-";   }
|   MINUS { $$ = "+";   }
;

%%

void yy::parser::error(const std::string& msg) {
    std::cerr << "Error: \"" << msg 
              << "\" in line " << line_num
              << '\n';
}
