#include "scope.hpp"
#include "symbol_table.hpp"
#include "types.hpp"

void SymbolTable::open_scope() {
  int offset = scopes.empty() ? 0 : scopes.back().get_offset();
  scopes.push_back(Scope(offset));
}

void SymbolTable::close_scope() {
  scopes.pop_back();
}

void SymbolTable::insert(std::string symbol, type_ptr t) {
  scopes.back().insert(symbol, std::move(t));
}

int SymbolTable::get_size_of_current_scope() const {
  return scopes.back().get_size();
}
