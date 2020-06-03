#ifndef __SCOPE_HPP__
#define __SCOPE_HPP__

#include <map>
#include <memory>
#include <optional>
#include <string>

class Entry;
class TypeInfo;

class Scope {
  std::map<std::string, std::shared_ptr<Entry>> locals;

public:
  Scope();

  int get_size() const;
  void insert(std::string name, std::shared_ptr<Entry> entry);
  std::optional<std::shared_ptr<Entry>> lookup(std::string name);
};

#endif
