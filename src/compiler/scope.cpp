#include <string>

#include "scope.hpp"
#include "symbol_entry.hpp"
#include "types.hpp"

void Scope::insert(std::string label) {
  this->labels.insert(label);
}

bool Scope::has_label(std::string label) {
  auto it = this->labels.find(label);
  if (it != this->labels.end())
    return true;
  else
    return false;
}

void Scope::insert(std::string name, std::shared_ptr<Entry> entry) {
  auto it = this->entries.find(name);
  if (it != this->entries.end())
    std::cout << "Name \"" << name << "\" has already been declared" << std::endl;
  this->entries[name] = entry;
}

std::shared_ptr<Entry> Scope::lookup(std::string name) {
  auto it = this->entries.find(name);
  if (it != this->entries.end())
    return this->entries[name];
  else
    return nullptr;
}
