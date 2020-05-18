#ifndef __SYMBOL_TABLE_HPP__
#define __SYMBOL_TABLE_HPP__

#include <memory>
#include <string>
#include <vector>

class Scope;

class SymbolTable {
  std::vector<Scope> scopes;

public:
  void open_scope();
  void close_scope();
  void insert(std::string, std::shared_ptr<Type> t);
  int get_size_of_current_scope() const;
};

#endif
