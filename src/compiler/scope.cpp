#include <string>
#include <optional>

#include "scope.hpp"
#include "symbol_entry.hpp"
#include "types.hpp"

Scope::Scope() {}

int Scope::get_size() const {
  return this->locals.size();
}

void Scope::insert(std::string name, std::shared_ptr<Entry> entry) {
  auto it = this->locals.find(name);
  if (it != this->locals.end())
    std::cout << "Name \"" << name << "\" has already been declared" << std::endl;
  this->locals[name] = entry;
}

std::optional<std::shared_ptr<Entry>> Scope::lookup(std::string name) {
  auto it = this->locals.find(name);
  if (it != this->locals.end())
    return this->locals[name];
  else
    return std::nullopt;
}
