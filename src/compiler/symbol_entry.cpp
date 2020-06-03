#include <memory>

#include "symbol_entry.hpp"
#include "types.hpp"

Entry::Entry() {}

VariableEntry::VariableEntry() {}

VariableEntry::VariableEntry(std::shared_ptr<TypeInfo> type)
  : type(type) {}

std::shared_ptr<TypeInfo> VariableEntry::get_type() {
  return this->type;
}

FunctionParameter::FunctionParameter() {}

FunctionParameter::FunctionParameter(bool pass_by_reference, std::shared_ptr<TypeInfo> type)
  : pass_by_reference(pass_by_reference), type(type) {}

std::shared_ptr<TypeInfo> FunctionParameter::get_type() {
  return this->type;
}

FunctionEntry::FunctionEntry() {}

void FunctionEntry::insert(std::string name, FunctionParameter parameter) {
  auto it = this->parameters.find(name);
  if (it != this->parameters.end())
    std::cout << "Name \"" << name << "\" has already been declared" << std::endl;
  this->parameters[name] = parameter;
}

std::optional<FunctionParameter> FunctionEntry::lookup(std::string name) {
  auto it = this->parameters.find(name);
  if (it != this->parameters.end())
    return this->parameters[name];
  else
    return std::nullopt;
}
