#**********************************************************************
#* The following automatic variables were used:
#*     $@ : The file name of the target of the rule
#*     $^ : The names of all the prerequisites with spaces between them
#*     $< : The name of the first prerequisite
#**********************************************************************

CXX=clang++
CFLAGS=-Wall
RM=rm -f

all: parser

parser: lexer.o parser.o
	$(CXX) $(CFLAGS) -o $@ $^

lexer.cpp: lexer.l
	flex -s -t $< > $@

lexer.o: lexer.cpp lexer.hpp parser.hpp

parser.hpp parser.cpp: parser.y
	bison -dv -o parser.cpp $<

parser.o: parser.cpp lexer.hpp

.PHONY: clean distclean

clean:
	$(RM) lexer.cpp parser.cpp parser.hpp parser.output *.o

distclean: clean
	$(RM) parser
