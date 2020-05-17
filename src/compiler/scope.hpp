#ifndef __SCOPE_HPP__
#define __SCOPE_HPP__

#include <map>
#include <memory>
#include <string>

#include "symbol_entry.hpp"
#include "types.hpp"

class Scope {
  std::map<std::string, SymbolEntry> locals;
  int offset;
  int size;

public:
  Scope(int offset);
  int get_offset() const;
  int get_size() const;
  SymbolEntry *lookup(std::string name);
  void insert(std::string, type_ptr t);
};

#endif
