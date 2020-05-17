#ifndef __SYMBOL_TABLE_HPP__
#define __SYMBOL_TABLE_HPP__

#include <string>
#include <vector>

#include "scope.hpp"
#include "types.hpp"

class SymbolTable {
  std::vector<Scope> scopes;

public:
  void open_scope();
  void close_scope();
  void insert(std::string, type_ptr t);
  int get_size_of_current_scope() const;
};

#endif
