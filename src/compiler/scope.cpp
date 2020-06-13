#include <string>

#include "scope.hpp"
#include "symbol_entry.hpp"
#include "types.hpp"

using entry_ptr = std::shared_ptr<Entry>;

bool Scope::insert(std::string label) {
  auto it = this->labels.find(label);
  if (it != this->labels.end()) {
    return false;
  } else {
    this->labels.insert(label);
    return true;
  }
}

bool Scope::has_label(std::string label) {
  auto it = this->labels.find(label);
  if (it != this->labels.end())
    return true;
  else
    return false;
}

bool Scope::insert(std::string name, entry_ptr entry) {
  auto it = this->entries.find(name);
  if (it != this->entries.end()) {
    return false;
  } else {
    this->entries[name] = entry;
    return true;
  }
}

entry_ptr Scope::lookup(std::string name) {
  auto it = this->entries.find(name);
  if (it != this->entries.end())
    return this->entries[name];
  else
    return nullptr;
}
