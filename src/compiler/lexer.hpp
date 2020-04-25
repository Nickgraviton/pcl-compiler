#ifndef __LEXER_HPP__
#define __LEXER_HPP__

int yylex();
void yyerror(std::string msg);
extern int line_num;

#endif
