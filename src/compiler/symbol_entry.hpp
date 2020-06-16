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

  std::shared_ptr<TypeInfo> get_type();
};

class VariableEntry : public Entry {
public:
  VariableEntry(std::shared_ptr<TypeInfo> type);
};

class FunctionEntry : public Entry {
  bool forward_declaration;
  std::vector<std::shared_ptr<VariableEntry>> parameters;

public:
  FunctionEntry(bool forward_declaration, std::shared_ptr<TypeInfo> type);

  void add_parameter(std::shared_ptr<VariableEntry> parameter);
  std::vector<std::shared_ptr<VariableEntry>>& get_parameters();
  bool is_forward();
};

#endif
