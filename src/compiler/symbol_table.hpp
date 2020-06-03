#ifndef __SYMBOL_TABLE_HPP__
#define __SYMBOL_TABLE_HPP__

#include <memory>
#include <optional>
#include <string>
#include <vector>

class Entry;
class Scope;

class SymbolTable {
  std::vector<Scope> scopes;

public:
  void open_scope();
  void close_scope();
  void insert(std::string name, std::shared_ptr<Entry> entry);
  std::optional<Entry> lookup(std::string name);
  int get_size_of_current_scope() const;
};

#endif
