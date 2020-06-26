#include <memory>
#include <string>
#include <vector>

#include "symbol_scope.hpp"
#include "symbol_table.hpp"
#include "symbol_entry.hpp"
#include "types.hpp"

using entry_ptr = std::shared_ptr<Entry>;

void SymbolTable::open_scope() {
  this->scopes.push_back(SymbolScope());
}

void SymbolTable::close_scope() {
  this->scopes.pop_back();
}

bool SymbolTable::add_label(std::string label) {
  return this->scopes.back().add_label(label);
}

bool SymbolTable::has_label(std::string label) {
  return this->scopes.back().has_label(label);
}

bool SymbolTable::insert(std::string name, entry_ptr entry) {
  return this->scopes.back().insert(name, entry);
}

entry_ptr SymbolTable::lookup(std::string name) {
  for (auto r_it = std::rbegin(this->scopes); r_it != std::rend(this->scopes); ++r_it) {
    auto result = r_it->lookup(name);
    if (result)
      return result;
  }
  return nullptr;
}
