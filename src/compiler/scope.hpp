#ifndef __SCOPE_HPP__
#define __SCOPE_HPP__

#include <map>
#include <memory>
#include <set>
#include <string>

class Entry;
class TypeInfo;

class Scope {
  std::set<std::string> labels;
  std::map<std::string, std::shared_ptr<Entry>> entries;

public:
  void insert(std::string label);
  bool has_label(std::string label);

  void insert(std::string name, std::shared_ptr<Entry> entry);
  std::shared_ptr<Entry> lookup(std::string name);
};

#endif
