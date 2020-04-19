#**********************************************************************
#* The following automatic variables were used:
#*     $@ : The file name of the target of the rule
#*     $^ : The names of all the prerequisites with spaces between them
#*     $< : The name of the first prerequisite
#**********************************************************************

CXX=clang++
CXXFLAGS=-Wall
RM=rm -f


all: lexer

lexer.cpp: lexer.l
	flex -s -t $< > $@
lexer: lexer.cpp
	$(CXX) $(CXXFLAGS) $< -o $@


.PHONY: clean distclean

clean:
	$(RM) lexer.cpp

distclean: clean
	$(RM) lexer
