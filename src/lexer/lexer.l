%option noinput
%option nounput
%option noyywrap
%x COMMENT

%{
#include <iostream>
#include <string>
#include <sstream>
int line_num = 1;
void yyerror(std::string msg);

#define T_eof            0
#define T_and            1001
#define T_array          1002
#define T_begin          1003
#define T_boolean        1004
#define T_char           1005
#define T_dispose        1006
#define T_int_div        1007
#define T_do             1008
#define T_else           1009
#define T_end            1010
#define T_false          1011
#define T_forward        1012
#define T_function       1013
#define T_goto           1014
#define T_if             1015
#define T_integer        1016
#define T_label          1017
#define T_mod            1018
#define T_new            1019
#define T_nil            1020
#define T_not            1021
#define T_of             1022
#define T_or             1023
#define T_procedure      1024
#define T_program        1025
#define T_real           1026
#define T_result         1027
#define T_return         1028
#define T_then           1029
#define T_true           1030
#define T_var            1031
#define T_while          1032

#define T_equal          1033
#define T_gt             1034
#define T_lt             1035
#define T_not_equal      1036
#define T_ge             1037
#define T_le             1038
#define T_plus           1039
#define T_minus          1040
#define T_mul            1041
#define T_div            1042
#define T_deref          1043
#define T_addr           1044
#define T_assign         1045
#define T_end_stmt       1046
#define T_dot            1047
#define T_op_par         1048
#define T_clos_par       1049
#define T_colon          1050
#define T_comma          1051
#define T_op_brack       1052
#define T_clos_brack     1053

#define T_id             1054
#define T_int_const      1055
#define T_real_const     1056
#define T_character      1057
#define T_string_literal 1058


%}

alpha            [a-zA-Z]
digit            [0-9]
exponent         [eE][+-]?{digit}+
printable        [ -!#-&(-\[\]-~]
escape_seq       \\[ntr0\\\'\"]
single_character {printable}|{escape_seq}
 

%%


"and"       return T_and;
"array"     return T_array;
"begin"     return T_begin;
"boolean"   return T_boolean;
"char"      return T_char;
"dispose"   return T_dispose;
"div"       return T_int_div;
"do"        return T_do;
"else"      return T_else;
"end"       return T_end;
"false"     return T_false;
"forward"   return T_forward;
"function"  return T_function;
"goto"      return T_goto;
"if"        return T_if;
"integer"   return T_integer;
"label"     return T_label;
"mod"       return T_mod;
"new"       return T_new;
"nil"       return T_nil;
"not"       return T_not;
"of"        return T_of;
"or"        return T_or;
"procedure" return T_procedure;
"program"   return T_program;
"real"      return T_real;
"result"    return T_result;
"return"    return T_return;
"then"      return T_then;
"true"      return T_true;
"var"       return T_var;
"while"     return T_while;

"="         return T_equal;
">"         return T_gt;
"<"         return T_lt;
"<>"        return T_not_equal;
">="        return T_ge;
"<="        return T_le;
"+"         return T_plus;
"-"         return T_minus;
"*"         return T_mul;
"/"         return T_div;
"^"         return T_deref;
"@"         return T_addr;
":="        return T_assign;
";"         return T_end_stmt;
"."         return T_dot;
"("         return T_op_par;
")"         return T_clos_par;
":"         return T_colon;
","         return T_comma;
"["         return T_op_brack;
"]"         return T_clos_brack;

{alpha}({alpha}|{digit}|_)*   return T_id;
{digit}+                      return T_int_const;
{digit}+\.{digit}+{exponent}? return T_real_const;
\'{single_character}?\'       return T_character;
\"{single_character}*\"       return T_string_literal;

[ \t\r]          /* nothing */
\n               ++line_num;

"(*"             BEGIN(COMMENT);
<COMMENT>"*)"    BEGIN(INITIAL);
<COMMENT>\n      ++line_num;
<COMMENT>"*"     /* nothing */
<COMMENT>[^*\n]+ /* nothing */
<COMMENT><<EOF>> { yyerror("Unexpected end of file within comment section");
                   exit(1); }

<<EOF>>          return T_eof;
.                { std::stringstream ss;
                   ss << "Illegal character with code " 
                      << (yytext[0] >= 32 ? yytext[0] : '?');
                   yyerror(ss.str());
                   exit(1); }


%%

void yyerror(std::string msg) {
    std::cerr << "Error: \"" << msg 
              << "\" in line " << line_num
              << '\n';
}

int main() {
    int token;
    do {
        token = yylex();
        std::cout << "token=" << token
                  << ", lexeme=\"" << yytext
                  << "\"\n";
    } while (token != T_eof);
}
