# PCL
Compiler implemented in C++ using flex, bison, llvm and clang. A toy compiler  for the PCL toy language which has many similarities with Pascal.

## Project structure
```
|-- compile.sh
|-- data
|   |-- bsort.pcl
|   |-- hanoi.pcl
|   |-- mean.pcl
|   |-- new_dispose.pcl
|   |-- primes.pcl
|   `-- reverse.pcl
|-- pcl2019.pdf
|-- README.md
`-- src
    |-- ast.cpp
    |-- ast.hpp
    |-- codegen_table.cpp
    |-- codegen_table.hpp
    |-- lexer.hpp
    |-- lexer.l
    |-- libpcl.c
    |-- Makefile
    |-- parser.y
    |-- pcl.cpp
    |-- symbol_table.cpp
    |-- symbol_table.hpp
    |-- types.cpp
    `-- types.hpp
```
Root directory:

- `compile.sh:` Simple helper script that takes the file to be compiled as input, builds the compiler
and outputs an executable named `a.out`
- `data:` The data folder contains some basic example programs in pcl
- `pcl2019.pdf:` This file contains the language description
- `README.md:` This file
- `src:` The source files for the compiler

src directory:

- `ast.cpp/ast.hpp:` Files describing the AST that is responsible for the semantic and code generation passes
- `codegen_table.cpp/codegen_table.hpp:` A data structure to store the llvm constructs during code generation
- `lexer.hpp:` File containing the current line variable declaration
- `lexer.l:` Flex input file to generate the compiler's scanner
- `libpcl.c:` Built in library functions. The missing functions use the C math library directly instead
- `Makefile:` A standard makefile
- `parser.y:` Bison input file with the language's grammar to generate the program's parser
- `pcl.cpp:` Main driver program that reads the command line arguments and calls the necessary functions to parse, check
and generate the output
- `symbol_table.cpp/symbol_table.hpp:` A data structure to do record keeping for variables and functions during the
semantic pass
- `types.cpp/types.hpp:` Type declarations that are used during the semantic pass to perform checks

## Dependencies

NOTE: Bison needs to be at least version 3.2. 

**Fedora**

The compiler has been tested on Fedora 32 with llvm 10.0.0, clang 10.0.0, flex 2.6.4 and bison 3.5  
To install the necessary dependencies run:  
`sudo dnf install make llvm llvm-devel clang flex bison`

**Arch**

The compiler has been tested on Arch Linux with llvm 10.0.0, clang 10.0.0, flex 2.6.4 and bison 3.6.4  
To install the necessary dependencies run:  
`sudo pacman -S make llvm clang flex bison`

**Ubuntu**

The compiler has been tested on Ubuntu 20.04 with llvm 10.0.0, clang 10.0.0, flex 2.6.4 and bison 3.5.1  
To install the necessary dependencies run:  
`sudo apt install make llvm clang flex bison`

## How to build, run and make an executable directly

In order to compile a file directly use the `compile.sh` script that takes the file to be compiled as input
and outputs an executable named `a.out`

To use the script either invoke the shell directly:

`sh compile.sh <input_file>`

or add execution permissions to the script and then execute it:

```
chmod +x compile.sh
./compile.sh <input_file>
```

After the executable is created in the current folder simply execute it:

`./a.out`

## How to build
Inside the src folder run:

- `make` to build the compiler executable and the pcl library
- `make clean` to delete all intermediate files
- `make distclean` to delete all intermediate files and the compiler

## How to run

When the `-O` flag is supplied, llvm optimization passes are activated.

- `pcl [-O] <input_file>.pcl` to produce two files. One with the `.imm` extension containing the llvm IR of the input program
and one with the `.asm` extension containing the assembly output of the input program.

- `pcl [-O] [-i|-f]` when the input program is given in standard input and the output is given in standard output.
When the `-i` flag is specified the output contains the llvm IR of the input program and when the `-f` flag is specified
the output contains the assmebly output of the input program.

NOTE: The `.imm` and `.asm` files are created in the same directory as the input file.

Having the `.asm` file of the input, we can then link our output file with the `libpcl.a` library and the C math library using clang:

`clang <input_file>.asm /path/to/libpcl.a [-o <output_file>] -lm`

## How to run with Docker(Ubuntu 20.04 base image)
(Not recommended as the resulting image file can be quite big and the output file is inside the container unless a directory is mounted inside of it)

Upon building the docker image, the `data` and `src` folders along with the `compile.sh` file get copied into the image.
Therefore input files for the compiler should be inside the `data` folder before building the image.

1. Run `docker build -t compiler .`
2. Run `docker run -it compiler`
3. You will be dropped into an interactive shell where you can run `sh compile.sh <input_file>`
4. Run the produced executable by executing it `./a.out`
