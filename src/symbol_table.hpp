#ifndef __SYMBOL_TABLE_HPP__
#define __SYMBOL_TABLE_HPP__

#include <map>
#include <memory>
#include <set>
#include <string>
#include <utility>
#include <vector>

class TypeInfo;

// Varialbe information
// name: the name of the variable
// nesting_level: the nesting level where the variable is located
// type: the type of the variable
class VarInfo {
  std::string name;
  int nesting_level;
  std::shared_ptr<TypeInfo> type;

public:
  VarInfo(std::string name, int nesting_level, std::shared_ptr<TypeInfo> type);

  std::string get_name();
  int get_nesting_level();
  std::shared_ptr<TypeInfo> get_type();
};


class Entry {
  std::shared_ptr<TypeInfo> type;

public:
  Entry(std::shared_ptr<TypeInfo> type);
  virtual ~Entry() = default;

  std::shared_ptr<TypeInfo> get_type();
};

// Variable entry
// type: type of the variable
class VariableEntry : public Entry {
public:
  VariableEntry(std::shared_ptr<TypeInfo> type);
};

// Function entry
// forward_declartion: denotes whether the declaration is forward
// parameters: a pair of a bool and a variable entry that denotes whether each parameter is passed
//             by reference and that holds the type of the variable
class FunctionEntry : public Entry {
  bool forward_declaration;
  std::vector<std::pair<bool, std::shared_ptr<VariableEntry>>> parameters;

public:
  FunctionEntry(bool forward_declaration, std::shared_ptr<TypeInfo> type);

  void add_parameter(std::pair<bool, std::shared_ptr<VariableEntry>> parameter);
  std::vector<std::pair<bool, std::shared_ptr<VariableEntry>>>& get_parameters();

  bool is_forward();
};

// Scope of the symbol table
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

// Synbol table
// scopes: scopes are implemented by a vector. Each time we enter a deeper scope we push back a scope
//         and each time we exit one we pop it
class SymbolTable {
  std::vector<SymbolScope> scopes;

public:
  int get_nesting_level();
  std::vector<std::shared_ptr<VarInfo>> get_prev_scope_vars();

  void open_scope();
  void close_scope();

  bool add_label(std::string label);
  bool has_label(std::string label);

  bool insert(std::string name, std::shared_ptr<Entry> entry);
  std::shared_ptr<Entry> lookup(std::string name);
};

#endif
