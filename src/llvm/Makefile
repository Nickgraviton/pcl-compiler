#*********************************************************************
#* The following automatic variables were used:
#*     $@ : The file name of the target of the rule
#*     $^ : The names of all the prerequisites with spaces between them
#*     $< : The name of the first prerequisite
#**********************************************************************

CXX=clang++
CXXFLAGS=-Wall $(shell llvm-config --cxxflags)
LDFLAGS=$(shell llvm-config --ldflags --system-libs --libs all)
RM=rm -f

all: pcl

pcl: lexer.o parser.o ast.o pcl.o
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

pcl.o: pcl.cpp parser.hpp

ast.o: ast.cpp

lexer.cpp: lexer.l
	flex -s -t $< > $@

lexer.o: lexer.cpp parser.hpp

parser.hpp parser.cpp: parser.y
	bison -dv -o parser.cpp $<

parser.o: parser.cpp

.PHONY: clean distclean

clean:
	$(RM) lexer.cpp parser.cpp parser.hpp parser.output *.o

distclean: clean
	$(RM) pcl
