#include <memory>
#include <optional>
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

void SymbolTable::insert(std::string symbol, std::shared_ptr<TypeInfo> t) {
  scopes.back().insert(symbol, std::move(t));
}

std::optional<SymbolEntry> SymbolTable::lookup(std::string name) {
  for (auto r_it = std::rbegin(scopes); r_it != std::rend(scopes); ++r_it) {
    auto result = r_it->lookup(name);
    if (result)
      return result;
  }
  return std::nullopt;
}

int SymbolTable::get_size_of_current_scope() const {
  return scopes.back().get_size();
}
