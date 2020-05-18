#include <memory>

#include "symbol_entry.hpp"
#include "types.hpp"

SymbolEntry::SymbolEntry() {}

SymbolEntry::SymbolEntry(std::shared_ptr<Type> t)
  : t(t) {}
