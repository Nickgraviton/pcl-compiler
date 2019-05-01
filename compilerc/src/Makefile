CC=g++
CFLAGS=-Wall
RM=rm -f


all: compiler

compiler.lex.cc: compiler.l
	flex --c++ -t compiler.l > compiler.lex.cc
compiler: compiler.lex.cc
	$(CC) $(FLAGS) compiler.lex.cc -o compiler


.PHONY: clean distclean

clean:
	$(RM) *.o *.cc compiler.tab.* core

distclean: clean
	$(RM) compiler
