#include <iostream>

#include "ast.hpp"
#include "parser.hpp"

int main() {
  // Parse the input file and emmit code afterwards
  int result = yyparse();
  if (result == 0)
    std::cout << "Parsing successful.\n";
  return result;
}
