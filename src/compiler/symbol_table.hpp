#ifndef __SYMBOL_TABLE_HPP__
#define __SYMBOL_TABLE_HPP__

#include <memory>
#include <string>
#include <vector>

class Entry;
class SymbolScope;

class SymbolTable {
  std::vector<SymbolScope> scopes;

public:
  void open_scope();
  void close_scope();

  bool add_label(std::string label);
  bool has_label(std::string label);

  bool insert(std::string name, std::shared_ptr<Entry> entry);
  std::shared_ptr<Entry> lookup(std::string name);
};

#endif
