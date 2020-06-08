#include <memory>

#include "symbol_entry.hpp"
#include "types.hpp"

Entry::Entry(std::shared_ptr<TypeInfo> type)
  : type(type) {}

std::shared_ptr<TypeInfo> Entry::get_type() const {
  return this->type;
}

VariableEntry::VariableEntry(std::shared_ptr<TypeInfo> type)
  : Entry(type) {}

FunctionParameter::FunctionParameter(bool pass_by_reference, std::shared_ptr<TypeInfo> type)
  : pass_by_reference(pass_by_reference), type(type) {}

std::shared_ptr<TypeInfo> FunctionParameter::get_type() {
  return this->type;
}

FunctionEntry::FunctionEntry(std::shared_ptr<TypeInfo> type)
  : Entry(type) {}

void FunctionEntry::add_parameter(FunctionParameter parameter) {
  this->parameters.push_back(parameter);
}

std::vector<FunctionParameter>& FunctionEntry::get_parameters() {
  return this->parameters;
}
