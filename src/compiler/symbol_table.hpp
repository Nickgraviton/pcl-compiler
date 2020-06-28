#ifndef __SYMBOL_TABLE_HPP__
#define __SYMBOL_TABLE_HPP__

#include <map>
#include <memory>
#include <set>
#include <string>
#include <utility>
#include <vector>

class TypeInfo;

struct var_info {
  var_info(std::string name, int nesting_level, std::shared_ptr<TypeInfo> type);

  std::string name;
  int nesting_level;
  std::shared_ptr<TypeInfo> type;
};

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
  std::vector<std::pair<bool, std::shared_ptr<VariableEntry>>> parameters;

public:
  FunctionEntry(bool forward_declaration, std::shared_ptr<TypeInfo> type);

  void add_parameter(std::pair<bool, std::shared_ptr<VariableEntry>> parameter);
  std::vector<std::pair<bool, std::shared_ptr<VariableEntry>>>& get_parameters();

  bool is_forward();
};

class SymbolScope {
  std::set<std::string> labels;
  std::vector<std::string> vars;
  std::map<std::string, std::shared_ptr<Entry>> entries;

public:
  bool add_label(std::string label);
  bool has_label(std::string label);

  std::vector<std::string>& get_vars();

  bool insert(std::string name, std::shared_ptr<Entry> entry);
  std::shared_ptr<Entry> lookup(std::string name);
};

class SymbolTable {
  std::vector<SymbolScope> scopes;

public:
  int get_nesting_level();
  std::vector<std::shared_ptr<var_info>> get_prev_scope_vars();

  void open_scope();
  void close_scope();

  bool add_label(std::string label);
  bool has_label(std::string label);

  bool insert(std::string name, std::shared_ptr<Entry> entry);
  std::shared_ptr<Entry> lookup(std::string name);
};

#endif
