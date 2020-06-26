#include <iostream>

#include "ast.hpp"
#include "parser.hpp"

extern std::unique_ptr<Program> root;

int main() {
  // Parse the input file and emmit code afterwards
  yy::parser parser;
  int result = parser.parse();
  if (result == 0) {
    root->print(std::cout, 0);
    std::cout << "Parsing successful." << std::endl;
    root->semantic();
    root->codegen();
  }
  return result;
}
