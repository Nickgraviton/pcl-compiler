#*********************************************************************
#* The following automatic variables were used:
#*     $@ : The file name of the target of the rule
#*     $^ : The names of all the prerequisites with spaces between them
#*     $< : The name of the first prerequisite
#**********************************************************************

CXX=clang++
CXXFLAGS=-Wall $(shell llvm-config --cxxflags) -std=c++17
LDFLAGS=$(shell llvm-config --ldflags --libs all)
RM=rm -f

all: pcl

pcl: lexer.o parser.o ast.o types.o pcl.o symbol_table.o scope.o symbol_entry.o
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

pcl.o: parser.cpp

lexer.cpp: lexer.l parser.cpp
	flex -s -t $< > $@

parser.hpp parser.cpp: parser.y
	bison -dv -o parser.cpp $<

.PHONY: clean distclean

clean:
	$(RM) lexer.cpp parser.cpp parser.hpp parser.output *.o

distclean: clean
	$(RM) pcl
