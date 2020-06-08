#ifndef __SYMBOL_ENTRY_HPP__
#define __SYMBOL_ENTRY_HPP__

#include <memory>
#include <vector>

class TypeInfo;

class Entry {
  std::shared_ptr<TypeInfo> type;

public:
  Entry(std::shared_ptr<TypeInfo> type);
  virtual ~Entry() = default;

  std::shared_ptr<TypeInfo> get_type() const;
};

class VariableEntry : public Entry {
public:
  VariableEntry(std::shared_ptr<TypeInfo> type);
};

class FunctionParameter {
  bool pass_by_reference;
  std::shared_ptr<TypeInfo> type;

public:
  FunctionParameter(bool pass_by_reference, std::shared_ptr<TypeInfo> type);

  std::shared_ptr<TypeInfo> get_type();
};

class FunctionEntry : public Entry {
  std::vector<FunctionParameter> parameters;

public:
  FunctionEntry(std::shared_ptr<TypeInfo> type);

  void add_parameter(FunctionParameter parameter);
  std::vector<FunctionParameter>& get_parameters();
};

#endif
