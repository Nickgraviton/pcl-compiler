#include <memory>
#include <string>
#include <vector>

#include "scope.hpp"
#include "symbol_table.hpp"
#include "symbol_entry.hpp"
#include "types.hpp"

void SymbolTable::open_scope() {
  scopes.push_back(Scope());
}

void SymbolTable::close_scope() {
  scopes.pop_back();
}

void SymbolTable::insert(std::string symbol, std::shared_ptr<Type> t) {
  scopes.back().insert(symbol, std::move(t));
}

int SymbolTable::get_size_of_current_scope() const {
  return scopes.back().get_size();
}
