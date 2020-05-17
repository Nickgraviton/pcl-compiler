#ifndef __SYMBOL_ENTRY_HPP__
#define __SYMBOL_ENTRY_HPP__

#include "types.hpp"

class SymbolEntry {
  type_ptr t;
  int offset;

public:
  SymbolEntry(type_ptr t, int offset);
};

#endif
