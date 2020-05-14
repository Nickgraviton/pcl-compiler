#include <iostream>

#include "ast.hpp"
#include "parser.hpp"

extern program_ptr root;

int main() {
  // Parse the input file and emmit code afterwards
  yy::parser parser;
  int result = parser.parse();
  if (result == 0)
    std::cout << "Parsing successful." << std::endl;
  root->print(std::cout, 0);
  return result;
}
