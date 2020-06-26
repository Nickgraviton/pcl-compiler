%option noinput
%option nounput
%option noyywrap
%x COMMENT

%{
#include <iostream>
#include <sstream>
#include <string>

#include "ast.hpp"
#include "lexer.hpp"
#include "types.hpp"
#include "parser.hpp"

#define YY_DECL yy::parser::symbol_type yylex()

int line_num = 1;

static char fix_char(char input[]);
static std::string fix_string(char input[]);
static void lexer_error(const std::string& msg);
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

"="         return yy::parser::make_EQ();
">"         return yy::parser::make_GT();
"<"         return yy::parser::make_LT();
"<>"        return yy::parser::make_NE();
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
{DIGIT}+                      return yy::parser::make_INT_CONST(std::stoi(yytext));
{DIGIT}+\.{DIGIT}+{EXPONENT}? return yy::parser::make_REAL_CONST(std::stod(yytext));
\'{SINGLE_CHARACTER}?\'       return yy::parser::make_CHAR_CONST(fix_char(yytext));
\"{SINGLE_CHARACTER}*\"       return yy::parser::make_STRING_LITERAL(fix_string(yytext));

[ \t\r]          /* nothing */
\n               ++line_num;

"(*"             BEGIN(COMMENT);
<COMMENT>"*)"    BEGIN(INITIAL);
<COMMENT>\n      ++line_num;
<COMMENT>"*"     /* nothing */
<COMMENT>[^*\n]+ /* nothing */
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

char fix_char(char input[]) {
  std::string str = std::string(input);
  switch (str[1]) {
    case '\\':
    {
      switch (str[2]) {
        case 'n':  return '\n';
        case 't':  return '\t';
        case 'r':  return '\r';
        case '0':  return '\0';
        case '\\': return '\\';
        case '\'': return '\'';
        case '\"': return '\"';
      }
    }
    default:
      return str[1];
  }
}

std::string fix_string(char input[]) {
  std::string str = std::string(input);
  return str.substr(1, str.size() - 2);
}

// Local lexer error function because parser's yyerror needs a parser object instance
void lexer_error(const std::string& msg) {
  std::cerr << "Lexer error: \"" << msg << "\" in line " << line_num << std::endl;
}
