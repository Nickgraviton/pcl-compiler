%option noinput
%option nounput
%option noyywrap
%x COMMENT

%{
#include <iostream>
#include <sstream>

#include "ast.hpp"
#include "lexer.hpp"
#include "parser.hpp"

#define YY_DECL yy::parser::symbol_type yylex()

void lexer_error(const std::string& msg);
int line_num = 1;
%}

ALPHA            [a-zA-Z]
DIGIT            [0-9]
EXPONENT         [eE][+-]?{DIGIT}+
PRINTABLE        [ -!#-&(-\[\]-~]
ESCAPE_SEQ       \\[ntr0\\\'\"]
SINGLE_CHARACTER {PRINTABLE}|{ESCAPE_SEQ}
 

%%


"and"       return yy::parser::make_AND();
"array"     return yy::parser::make_ARRAY();
"begin"     return yy::parser::make_BEGIN_ST();
"boolean"   return yy::parser::make_BOOLEAN();
"char"      return yy::parser::make_CHAR();
"dispose"   return yy::parser::make_DISPOSE();
"div"       return yy::parser::make_INT_DIV();
"do"        return yy::parser::make_DO();
"else"      return yy::parser::make_ELSE();
"end"       return yy::parser::make_END_ST();
"false"     return yy::parser::make_FALSE();
"forward"   return yy::parser::make_FORWARD();
"function"  return yy::parser::make_FUNCTION();
"goto"      return yy::parser::make_GOTO();
"if"        return yy::parser::make_IF();
"integer"   return yy::parser::make_INTEGER();
"label"     return yy::parser::make_LABEL();
"mod"       return yy::parser::make_MOD();
"new"       return yy::parser::make_NEW();
"nil"       return yy::parser::make_NIL();
"not"       return yy::parser::make_NOT();
"of"        return yy::parser::make_OF();
"or"        return yy::parser::make_OR();
"procedure" return yy::parser::make_PROCEDURE();
"program"   return yy::parser::make_PROGRAM();
"real"      return yy::parser::make_REAL();
"result"    return yy::parser::make_RESULT();
"return"    return yy::parser::make_RETURN();
"then"      return yy::parser::make_THEN();
"true"      return yy::parser::make_TRUE();
"var"       return yy::parser::make_VAR();
"while"     return yy::parser::make_WHILE();

"="         return yy::parser::make_EQUAL();
">"         return yy::parser::make_GT();
"<"         return yy::parser::make_LT();
"<>"        return yy::parser::make_NOT_EQUAL();
">="        return yy::parser::make_GE();
"<="        return yy::parser::make_LE();
"+"         return yy::parser::make_PLUS();
"-"         return yy::parser::make_MINUS();
"*"         return yy::parser::make_MUL();
"/"         return yy::parser::make_DIV();
"^"         return yy::parser::make_CARET();
"@"         return yy::parser::make_AT();
":="        return yy::parser::make_ASSIGN();
";"         return yy::parser::make_SEMI_COLON();
"."         return yy::parser::make_DOT();
"("         return yy::parser::make_OP_PAR();
")"         return yy::parser::make_CLOS_PAR();
":"         return yy::parser::make_COLON();
","         return yy::parser::make_COMMA();
"["         return yy::parser::make_OP_BRACK();
"]"         return yy::parser::make_CLOS_BRACK();

{ALPHA}({ALPHA}|{DIGIT}|_)*   return yy::parser::make_ID(std::string(yytext));
{DIGIT}+                      return yy::parser::make_INT_CONST(atoi(yytext));
{DIGIT}+\.{DIGIT}+{EXPONENT}? return yy::parser::make_REAL_CONST(atof(yytext));
\'{SINGLE_CHARACTER}?\'       return yy::parser::make_CHAR_CONST(yytext[0]);
\"{SINGLE_CHARACTER}*\"       return yy::parser::make_STRING_LITERAL(std::string(yytext));

[ \t\r]          /* nothing */
\n               ++line_num;

"(*"             BEGIN(COMMENT);
<COMMENT>"*)"    BEGIN(INITIAL);
<COMMENT>\n      ++line_num;
<COMMENT>"*"     /* nothing */
<COMMENT>[^*]+   /* nothing */
<COMMENT><<EOF>> { std::stringstream ss;
                   ss << "Unexpected end of file within comment section";
                   lexer_error(ss.str());
                   exit(1); }

.                { std::stringstream ss;
                   ss << "Illegal character with code " 
                      << (yytext[0] >= 32 ? yytext[0] : '?');
                   lexer_error(ss.str());
                   exit(1); }

%%


// Local lexer error function because parser's yyerror needs a parser object instance
void lexer_error(const std::string& msg) {
  std::cout << "Lexer error: "
            << "\"" << msg << "\" "
            << "in line " << line_num << "\n";
}
