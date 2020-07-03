#include <iostream>
#include <stdio.h>

#include "ast.hpp"
#include "parser.hpp"

extern FILE *yyin;
extern std::unique_ptr<Program> root;

static void print_usage (std::string compiler_name) {
  std::cerr << "Usage: " << compiler_name << " [-O] <input_file> || " << compiler_name << " [-O] [-i|-f]" << std::endl;
}

int main(int argc, char* argv[]) {
  if (argc < 2 || argc > 3) {
    print_usage(argv[0]);
    return 1;
  }

  bool optimize, asm_output, imm_output, input_file;
  optimize = asm_output = imm_output = input_file = false;

  std::string arg, file_name;

  if (argc == 2) {
    arg = std::string(argv[1]);
    if (arg == "-i") {
      imm_output = true;
    } else if (arg == "-f") {
      asm_output = true;
    } else {
      input_file = true;
      file_name = arg;
    }
  } else if (argc == 3) {
    arg = std::string(argv[1]);
    if (arg == "-i") {
      imm_output = true;
    } else if (arg == "-f") {
      asm_output = true;
    } else if (arg == "-O") {
      optimize = true;
    } else {
      input_file = true;
      file_name = arg;
    }

    arg = std::string(argv[2]);
    if (arg == "-i") {
      imm_output = true;
    } else if (arg == "-f") {
      asm_output = true;
    } else if (arg == "-O") {
      optimize = true;
    } else {
      input_file = true;
      file_name = arg;
    }
  }

  // Read from standard input by default and read from file if an argument has been provided
  if (input_file)
    yyin = fopen(file_name.c_str(), "r");

  // Parse the input file and emmit code afterwards
  yy::parser parser;
  int result = parser.parse();

  fclose(yyin);

  if (result == 0) {
    root->set_optimize(optimize);
    root->set_asm_output(asm_output);
    root->set_imm_output(imm_output);

    // Strip file extension
    if (input_file) {
      size_t index = file_name.find_last_of(".");
      root->set_file_name(file_name.substr(0, index));
    }

    // Uncomment the next line to print the AST
    // root->print(std::cout, 0);
    root->semantic();
    root->codegen();
  }

  return result;
}
