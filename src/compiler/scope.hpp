#ifndef __SCOPE_HPP__
#define __SCOPE_HPP__

#include <map>
#include <memory>
#include <optional>
#include <string>

class SymbolEntry;
class Type;

class Scope {
  std::map<std::string, SymbolEntry> locals;

public:
  Scope();
  int get_size() const;
  void insert(std::string name, std::shared_ptr<Type> t);
  std::optional<SymbolEntry> lookup(std::string name);
};

#endif
