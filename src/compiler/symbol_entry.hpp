#ifndef __SYMBOL_ENTRY_HPP__
#define __SYMBOL_ENTRY_HPP__

#include <memory>

class TypeInfo;

class SymbolEntry {
  std::shared_ptr<TypeInfo> type;

public:
  SymbolEntry();
  SymbolEntry(std::shared_ptr<TypeInfo> type);

  std::shared_ptr<TypeInfo> get_type();
};

#endif
