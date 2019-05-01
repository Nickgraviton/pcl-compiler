CC=g++
CFLAGS=--std=c++11 -Wall
RM=rm -f

all: lexer_exec

lexer_exec: flex_files
	$(CC) $(FLAGS) *.cc -o lexer_exec

flex_files: lexer
	flexc++ $<

.PHONY: clean distclean

clean:
	$(RM) lex.cc Lexerbase.h

distclean: clean
	$(RM) lexer_exec
