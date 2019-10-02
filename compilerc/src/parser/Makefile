#**********************************************************************
#* The following automatic variables were used:
#*     $@ : The file name of the target of the rule
#*     $^ : The names of all the prerequisites with spaces between them
#*     $< : The name of the first prerequisite
#**********************************************************************

CC=g++
CFLAGS=-Wall
RM=rm -f

parser: compiler.lex.o compiler.tab.o symbol.o symbolc.o
	$(CC) $(CFLAGS) -o $@ $^

lexer.lex.cc: lexer.l
	flex --c++ -t $< > $@

compiler.tab.c compiler.tab.h: compiler.y
	bison -dv $<

.PHONY: clean distclean

clean:
	$(RM) *.o *.cc compiler.tab.* core

distclean: clean
	$(RM) compiler
