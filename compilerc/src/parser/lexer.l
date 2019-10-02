%option noyywrap
%x COMMENT

%{
#include <iostream>
#include <sstream>
#include "lexer.hpp"
#include "parser.hpp"
int line_num = 1;
%}

ALPHA            [a-zA-Z]
DIGIT            [0-9]
EXPONENT         [eE][+-]?{DIGIT}+
PRINTABLE        [ -!#-&(-\[\]-~]
ESCAPE_SEQ       \\[ntr0\\\'\"]
SINGLE_CHARACTER {PRINTABLE}|{ESCAPE_SEQ}
 

%%


"and"       return AND;
"array"     return ARRAY;
"begin"     return BEGIN_ST;
"boolean"   return BOOLEAN;
"char"      return CHAR;
"dispose"   return DISPOSE;
"dive"      return INT_DIV;
"do"        return DO;
"else"      return ELSE;
"end"       return END;
"false"     return FALSE;
"forward"   return FORWARD;
"function"  return FUNCTION;
"goto"      return GOTO;
"if"        return IF;
"integer"   return INTEGER;
"label"     return LABEL;
"mod"       return MOD;
"new"       return NEW;
"nil"       return NIL;
"not"       return NOT;
"of"        return OF;
"or"        return OR;
"procedure" return PROCEDURE;
"program"   return PROGRAM;
"real"      return REAL;
"result"    return RESULT;
"return"    return RETURN;
"then"      return THEN;
"true"      return TRUE;
"var"       return VAR;
"while"     return WHILE;

"="         return EQUAL;
">"         return GT;
"<"         return LT;
"<>"        return NOT_EQUAL;
">="        return GE;
"<="        return LE;
"+"         return PLUS;
"-"         return MINUS;
"*"         return MUL;
"/"         return DIV;
"^"         return CARET;
"@"         return AT;
":="        return ASSIGN;
";"         return SEMI_COLON;
"."         return DOT;
"("         return OP_PAR;
")"         return CLOS_PAR;
":"         return COLON;
","         return COMMA;
"["         return OP_BRACK;
"]"         return CLOS_BRACK;

{ALPHA}({ALPHA}|{DIGIT}|_)*   return IDENTIFIER;
{DIGIT}+                      return INT_CONST;
{DIGIT}+\.{DIGIT}+{EXPONENT}? return REAL_CONST;
\'{SINGLE_CHARACTER}?\'       return CHAR_CONST;
\"{SINGLE_CHARACTER}*\"       return STRING_LITERAL;

[ \t\r\n]          /* nothing */

"(*"             BEGIN(COMMENT);
<COMMENT>"*)"    BEGIN(INITIAL);
<COMMENT>"*"     /* nothing */
<COMMENT>[^*]+   /* nothing */
<COMMENT><<EOF>> {
                     std::stringstream ss;
                     ss  << "ERROR: Unexpected end of "
                         << "file within comment section";
                     yyerror(ss.str());
                     exit(1);
                 }

<<EOF>>          return EOF_T;
.                {
                     std::stringstream ss;
                     ss << "Illegal character with code " 
                        << (yytext[0] >= 32 ? yytext[0] : '?');
                     yyerror(ss.str());
                     exit(1);
                 }

%%

void yyerror(std::string msg) {
    std::cerr << "Error: \"" << msg 
              << "\" in line " << line_num
              << '\n';
}
