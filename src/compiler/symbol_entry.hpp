#ifndef __SYMBOL_ENTRY_HPP__
#define __SYMBOL_ENTRY_HPP__

#include <map>
#include <memory>
#include <optional>

class TypeInfo;

class Entry {
public:
  Entry();
};

class VariableEntry : public Entry {
  std::shared_ptr<TypeInfo> type;

public:
  VariableEntry();
  VariableEntry(std::shared_ptr<TypeInfo> type);

  std::shared_ptr<TypeInfo> get_type();
};

class FunctionParameter {
  bool pass_by_reference;
  std::shared_ptr<TypeInfo> type;

public:
  FunctionParameter();
  FunctionParameter(bool pass_by_reference, std::shared_ptr<TypeInfo> type);

  std::shared_ptr<TypeInfo> get_type();
};

class FunctionEntry : public Entry {
  std::map<std::string, FunctionParameter> parameters;

public:
  FunctionEntry();

  void insert(std::string name, FunctionParameter parameter);
  std::optional<FunctionParameter> lookup(std::string name);
};
#endif
