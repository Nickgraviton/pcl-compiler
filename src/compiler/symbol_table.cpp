#include <map>
#include <memory>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "symbol_table.hpp"
#include "types.hpp"

using entry_ptr = std::shared_ptr<Entry>;
using type_ptr = std::shared_ptr<TypeInfo>;

var_info::var_info(std::string name, int nesting_level, type_ptr type)
  : name(name), nesting_level(nesting_level), type(type) {}

Entry::Entry(type_ptr type)
  : type(type) {}

type_ptr Entry::get_type() {
  return this->type;
}

VariableEntry::VariableEntry(type_ptr type)
  : Entry(type) {}

FunctionEntry::FunctionEntry(bool forward_declaration, type_ptr type)
  : Entry(type), forward_declaration(forward_declaration) {}

void FunctionEntry::add_parameter(std::pair<bool, std::shared_ptr<VariableEntry>> parameter) {
  this->parameters.push_back(parameter);
}

std::vector<std::pair<bool, std::shared_ptr<VariableEntry>>>& FunctionEntry::get_parameters() {
  return this->parameters;
}

bool FunctionEntry::is_forward() {
  return this->forward_declaration;
}

bool SymbolScope::add_label(std::string label) {
  auto it = this->labels.find(label);
  if (it != this->labels.end()) {
    return false;
  } else {
    this->labels.insert(label);
    return true;
  }
}

bool SymbolScope::has_label(std::string label) {
  auto it = this->labels.find(label);
  if (it != this->labels.end())
    return true;
  else
    return false;
}

std::vector<std::string>& SymbolScope::get_vars() {
  return this->vars;
}

bool SymbolScope::insert(std::string name, entry_ptr entry) {
  auto it = this->entries.find(name);
  if (it != this->entries.end()) {
    return false;
  } else {
    this->vars.push_back(name);
    this->entries[name] = entry;
    return true;
  }
}

entry_ptr SymbolScope::lookup(std::string name) {
  auto it = this->entries.find(name);
  if (it != this->entries.end())
    return this->entries[name];
  else
    return nullptr;
}

int SymbolTable::get_nesting_level() {
  return this->scopes.size();
}

std::vector<std::shared_ptr<var_info>> SymbolTable::get_prev_scope_vars() {
  std::vector<std::shared_ptr<var_info>> result;

  for (int i = 0; i < this->scopes.size(); i++)
    for (auto& var : this->scopes[i].get_vars())
      result.push_back(std::make_shared<var_info>(var, i, this->lookup(var)->get_type()));

  return result;
}

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
