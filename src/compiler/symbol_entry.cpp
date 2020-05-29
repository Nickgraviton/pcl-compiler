#include <memory>

#include "symbol_entry.hpp"
#include "types.hpp"

SymbolEntry::SymbolEntry() {}

SymbolEntry::SymbolEntry(std::shared_ptr<TypeInfo> type)
  : type(type) {}

std::shared_ptr<TypeInfo> SymbolEntry::get_type() {
  return type;
}
