#ifndef __SYMBOL_TABLE_HPP__
#define __SYMBOL_TABLE_HPP__

#include <memory>
#include <string>
#include <vector>

class Entry;
class Scope;

class SymbolTable {
  std::vector<Scope> scopes;

public:
  void open_scope();
  void close_scope();

  void insert(std::string label);
  bool has_label(std::string label);

  void insert(std::string name, std::shared_ptr<Entry> entry);
  std::shared_ptr<Entry> lookup(std::string name);
  std::shared_ptr<Entry> current_scope_lookup(std::string name);
};

#endif
