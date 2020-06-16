#include <memory>

#include "symbol_entry.hpp"
#include "types.hpp"

using type_ptr = std::shared_ptr<TypeInfo>;

Entry::Entry(type_ptr type)
  : type(type) {}

type_ptr Entry::get_type() {
  return this->type;
}

VariableEntry::VariableEntry(type_ptr type)
  : Entry(type) {}

FunctionEntry::FunctionEntry(bool forward_declaration, type_ptr type)
  : Entry(type), forward_declaration(forward_declaration) {}

void FunctionEntry::add_parameter(std::shared_ptr<VariableEntry> parameter) {
  this->parameters.push_back(parameter);
}

std::vector<std::shared_ptr<VariableEntry>>& FunctionEntry::get_parameters() {
  return this->parameters;
}

bool FunctionEntry::is_forward() {
  return this->forward_declaration;
}
