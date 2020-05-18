#ifndef __SYMBOL_ENTRY_HPP__
#define __SYMBOL_ENTRY_HPP__

#include <memory>

class Type;

class SymbolEntry {
  std::shared_ptr<Type> t;

public:
  SymbolEntry();
  SymbolEntry(std::shared_ptr<Type> t);
};

#endif
