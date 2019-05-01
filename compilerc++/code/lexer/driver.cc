#include<iostream>

#include "Lexer.h"

int main() {
	Lexer lexer;
	int token;

	do {
		token = lexer.lex();
		std::cout << "Found token " << token
			  << " with string " << lexer.matched()
			  << '\n';
	} while (token != lexer.EOF_T);
}
