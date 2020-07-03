#*********************************************************************
#* The following automatic variables were used:
#*     $@ : The file name of the target of the rule
#*     $^ : The names of all the prerequisites with spaces between them
#*     $< : The name of the first prerequisite
#**********************************************************************

CC=clang
CXX=clang++
CXXFLAGS=-g -Wall $(shell llvm-config --cxxflags)
LDFLAGS=$(shell llvm-config --ldflags --libs all)
RM=rm -f

all: pcl libpcl.a

pcl: lexer.o parser.o ast.o codegen_table.o symbol_table.o types.o pcl.o
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

lexer.cpp: lexer.l parser.hpp
	flex -s -o $@ $<

parser.hpp parser.cpp: parser.y
	bison -dv -o parser.cpp $<

libpcl.a: libpcl.c
	$(CC) $< -c -o libpcl.o
	ar rcs $@ libpcl.o
	$(RM) libpcl.o

.PHONY: clean distclean

clean:
	$(RM) lexer.cpp parser.cpp parser.hpp parser.output *.o

distclean: clean
	$(RM) pcl libpcl.a
