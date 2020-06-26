%{
#include <iostream>
#include <memory>
#include <vector>

#include "ast.hpp"
#include "lexer.hpp"
#include "types.hpp"
#include "parser.hpp"

std::unique_ptr<Program> root;
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
%token              EQ NE GT LT GE LE
%token              OP_PAR CLOS_PAR OP_BRACK CLOS_BRACK

%left               EQ NE GT LT GE LE
%left               PLUS MINUS OR
%left               MUL DIV INT_DIV MOD AND
%nonassoc           NOT CARET UNOP R_VAL AT

%type<std::unique_ptr<Block>>    block
%type<std::unique_ptr<Body>>     body
%type<std::unique_ptr<Expr>>     optional_expr expr l_value r_value
%type<std::unique_ptr<Formal>>   formal
%type<std::unique_ptr<Fun>>      header
%type<std::unique_ptr<Local>>    local
%type<std::unique_ptr<Program>>  program
%type<std::unique_ptr<Stmt>>     stmt
%type<std::shared_ptr<TypeInfo>> type

%type<std::vector<std::unique_ptr<Expr>>>     next_parameter
%type<std::vector<std::unique_ptr<Formal>>>   next_arg
%type<std::vector<std::unique_ptr<Local>>>    next_local
%type<std::vector<std::unique_ptr<Stmt>>>     next_stmt
%type<std::vector<std::unique_ptr<VarNames>>> next_var
%type<std::vector<std::string>>               next_id

%type<bool> optional_var optional_bracket
%type<UnOp> unop

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
| /* empty */      { $$ = std::vector<std::unique_ptr<Local>>();      }
;

local:
  VAR next_var                      { $$ = std::make_unique<VarDecl>(std::move($2));                           }
| LABEL next_id SEMI_COLON          { $$ = std::make_unique<LabelDecl>($2);                                    }
| header SEMI_COLON body SEMI_COLON { $1->set_body(std::move($3)); $1->set_forward(false); $$ = std::move($1); }
| FORWARD header SEMI_COLON         { $2->set_forward(true); $$ = std::move($2);                               }
;

next_var:
  next_id COLON type SEMI_COLON          { $$ = std::vector<std::unique_ptr<VarNames>>(); $$.push_back(std::make_unique<VarNames>($1, $3)); }
| next_var next_id COLON type SEMI_COLON { $$ = std::move($1); $$.push_back(std::make_unique<VarNames>($2, $4));                            }
;

next_id:
  ID               { $$ = std::vector<std::string>(); $$.push_back($1); }
| next_id COMMA ID { $$ = $1; $$.push_back($3);                         }
;

header:
  PROCEDURE ID OP_PAR next_arg CLOS_PAR           { $$ = std::make_unique<Fun>($2, nullptr, std::move($4)); }
| FUNCTION ID OP_PAR next_arg CLOS_PAR COLON type { $$ = std::make_unique<Fun>($2, $7, std::move($4));      }
;

next_arg:
  formal                     { $$ = std::vector<std::unique_ptr<Formal>>(); $$.push_back(std::move($1)); }
| next_arg SEMI_COLON formal { $$ = std::move($1); $$.push_back(std::move($3));                          }
| /* empty */                { $$ = std::vector<std::unique_ptr<Formal>>();                              }
;

formal:
  optional_var next_id COLON type { $$ = std::make_unique<Formal>($1, $2, $4); }
;

optional_var:
  VAR         { $$ = true;  }
| /* empty */ { $$ = false; }
;

type:
  INTEGER                                     { $$ = std::make_shared<IntType>();       }
| REAL                                        { $$ = std::make_shared<RealType>();      }
| BOOLEAN                                     { $$ = std::make_shared<BoolType>();      }
| CHAR                                        { $$ = std::make_shared<CharType>();      }
| ARRAY OP_BRACK INT_CONST CLOS_BRACK OF type { $$ = std::make_shared<ArrType>($3, $6); }
| ARRAY OF type                               { $$ = std::make_shared<IArrType>($3);    }
| CARET type                                  { $$ = std::make_shared<PtrType>($2);     }
;

block:
  BEGIN_ST next_stmt END_ST { $$ = std::make_unique<Block>(std::move($2)); }
;

next_stmt:
  stmt                      { $$ = std::vector<std::unique_ptr<Stmt>>(); $$.push_back(std::move($1)); }
| next_stmt SEMI_COLON stmt { $$ = std::move($1); $$.push_back(std::move($3));                        }
;

stmt:
  l_value ASSIGN expr               { $$ = std::make_unique<Assign>(std::move($1), std::move($3));         }
| block                             { $$ = std::move($1);                                                     }
| ID OP_PAR next_parameter CLOS_PAR { $$ = std::make_unique<CallStmt>($1, std::move($3));                     }
| IF expr THEN stmt                 { $$ = std::make_unique<If>(std::move($2), std::move($4), nullptr);       }
| IF expr THEN stmt ELSE stmt       { $$ = std::make_unique<If>(std::move($2), std::move($4), std::move($6)); }
| WHILE expr DO stmt                { $$ = std::make_unique<While>(std::move($2), std::move($4));             }
| ID COLON stmt                     { $$ = std::make_unique<Label>($1, std::move($3));                        }
| GOTO ID                           { $$ = std::make_unique<Goto>($2);                                        }
| RETURN                            { $$ = std::make_unique<Return>();                                        }
| NEW optional_expr l_value         { $$ = std::make_unique<New>(std::move($2), std::move($3));               }
| DISPOSE optional_bracket l_value  { $$ = std::make_unique<Dispose>($2, std::move($3));                      }
| /* empty */                       { $$ = std::make_unique<Empty>();                                         } 
;

optional_expr:
  OP_BRACK expr CLOS_BRACK { $$ = std::move($2); }
| /* empty */              { $$ = nullptr;       }
;

optional_bracket:
  OP_BRACK CLOS_BRACK { $$ = true;  }
| /* empty */         { $$ = false; }
;

expr:
  l_value             { $$ = std::move($1); }
| r_value %prec R_VAL { $$ = std::move($1); }
;

l_value:
  ID                               { $$ = std::make_unique<Variable>($1);                        }
| RESULT                           { $$ = std::make_unique<Result>();                            }
| STRING_LITERAL                   { $$ = std::make_unique<String>($1);                          }
| l_value OP_BRACK expr CLOS_BRACK { $$ = std::make_unique<Array>(std::move($1), std::move($3)); }
| expr CARET                       { $$ = std::make_unique<Deref>(std::move($1));                }
| OP_PAR l_value CLOS_PAR          { $$ = std::move($2);                                         }
;

r_value:
  INT_CONST                         { $$ = std::make_unique<Integer>($1);                                     }
| TRUE                              { $$ = std::make_unique<Boolean>(true);                                   }
| FALSE                             { $$ = std::make_unique<Boolean>(false);                                  }
| REAL_CONST                        { $$ = std::make_unique<Real>($1);                                        }
| CHAR_CONST                        { $$ = std::make_unique<Char>($1);                                        }
| NIL                               { $$ = std::make_unique<Nil>();                                           }
| OP_PAR r_value CLOS_PAR           { $$ = std::move($2);                                                     }
| ID OP_PAR next_parameter CLOS_PAR { $$ = std::make_unique<CallExpr>($1, std::move($3));                     }
| AT expr                           { $$ = std::make_unique<AddressOf>(std::move($2));                        }
  /* Correct rule is `AT l_value` but l_value creates reduce/reduce
     conflict and since r_value is nonassoc we can just leave this as expr*/
| unop expr %prec UNOP { $$ = std::make_unique<UnaryExpr>($1, std::move($2));                             }
| expr PLUS expr       { $$ = std::make_unique<BinaryExpr>(BinOp::PLUS, std::move($1), std::move($3));    }
| expr MINUS expr      { $$ = std::make_unique<BinaryExpr>(BinOp::MINUS, std::move($1), std::move($3));   }
| expr MUL expr        { $$ = std::make_unique<BinaryExpr>(BinOp::MUL, std::move($1), std::move($3));     }
| expr DIV expr        { $$ = std::make_unique<BinaryExpr>(BinOp::DIV, std::move($1), std::move($3));     }
| expr INT_DIV expr    { $$ = std::make_unique<BinaryExpr>(BinOp::INT_DIV, std::move($1), std::move($3)); }
| expr MOD expr        { $$ = std::make_unique<BinaryExpr>(BinOp::MOD, std::move($1), std::move($3));     }
| expr OR expr         { $$ = std::make_unique<BinaryExpr>(BinOp::OR, std::move($1), std::move($3));      }
| expr AND expr        { $$ = std::make_unique<BinaryExpr>(BinOp::AND, std::move($1), std::move($3));     }
| expr EQ expr         { $$ = std::make_unique<BinaryExpr>(BinOp::EQ, std::move($1), std::move($3));      }
| expr NE expr         { $$ = std::make_unique<BinaryExpr>(BinOp::NE, std::move($1), std::move($3));      }
| expr LT expr         { $$ = std::make_unique<BinaryExpr>(BinOp::LT, std::move($1), std::move($3));      }
| expr LE expr         { $$ = std::make_unique<BinaryExpr>(BinOp::LE, std::move($1), std::move($3));      }
| expr GT expr         { $$ = std::make_unique<BinaryExpr>(BinOp::GT, std::move($1), std::move($3));      }
| expr GE expr         { $$ = std::make_unique<BinaryExpr>(BinOp::GE, std::move($1), std::move($3));      }
;

next_parameter:
  expr                      { $$ = std::vector<std::unique_ptr<Expr>>(); $$.push_back(std::move($1)); }
| next_parameter COMMA expr { $$ = std::move($1); $$.push_back(std::move($3));                        }
| /* empty */               { $$ = std::vector<std::unique_ptr<Expr>>();                              }
;

unop:
  NOT   { $$ = UnOp::NOT;   }
| PLUS  { $$ = UnOp::MINUS; }
| MINUS { $$ = UnOp::PLUS;  }
;

%%

void yy::parser::error(const std::string& msg) {
  std::cerr << "Error: \"" << msg << "\" in line " << line_num << std::endl;
}
