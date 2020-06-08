#include <memory>
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

void SymbolTable::insert(std::string label) {
  this->scopes.back().insert(label);
}

bool SymbolTable::has_label(std::string label) {
  return this->scopes.back().has_label(label);
}

void SymbolTable::insert(std::string name, std::shared_ptr<Entry> entry) {
  this->scopes.back().insert(name, entry);
}

std::shared_ptr<Entry> SymbolTable::lookup(std::string name) {
  for (auto r_it = std::rbegin(this->scopes); r_it != std::rend(this->scopes); ++r_it) {
    auto result = r_it->lookup(name);
    if (result)
      return result;
  }
  return nullptr;
}

std::shared_ptr<Entry> SymbolTable::current_scope_lookup(std::string name) {
  return this->scopes.back().lookup(name);
}
