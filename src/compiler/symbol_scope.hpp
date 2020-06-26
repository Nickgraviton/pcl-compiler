#ifndef __SYMBOL_SCOPE_HPP__
#define __SYMBOL_SCOPE_HPP__

#include <map>
#include <memory>
#include <set>
#include <string>

class Entry;
class TypeInfo;

class SymbolScope {
  std::set<std::string> labels;
  std::map<std::string, std::shared_ptr<Entry>> entries;

public:
  bool add_label(std::string label);
  bool has_label(std::string label);

  bool insert(std::string name, std::shared_ptr<Entry> entry);
  std::shared_ptr<Entry> lookup(std::string name);
};

#endif
