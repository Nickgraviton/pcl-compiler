#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "scope.hpp"
#include "symbol_table.hpp"
#include "symbol_entry.hpp"
#include "types.hpp"

void SymbolTable::open_scope() {
  this->scopes.push_back(Scope());
}

void SymbolTable::close_scope() {
  this->scopes.pop_back();
}

void SymbolTable::insert(std::string name, std::shared_ptr<Entry> entry) {
  this->scopes.back().insert(name, entry);
}

std::optional<Entry> SymbolTable::lookup(std::string name) {
  for (auto r_it = std::rbegin(this->scopes); r_it != std::rend(this->scopes); ++r_it) {
    auto result = r_it->lookup(name);
    if (result)
      return result;
  }
  return std::nullopt;
}

int SymbolTable::get_size_of_current_scope() const {
  return this->scopes.back().get_size();
}
