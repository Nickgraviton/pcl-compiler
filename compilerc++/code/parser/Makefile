CC=g++
CFLAGS=--std=c++11 -Wall
RM=rm -f

all: parser_exec

parser_exec: bison_files
	$(CC) $(FLAGS) *.cc -o parser_exec

bison_files: parser
	bisonc++ $<

.PHONY: clean distclean

clean:
	$(RM) parse.cc Parserbase.h

distclean: clean
	$(RM) parser_exec
