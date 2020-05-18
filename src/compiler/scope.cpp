#include <string>
#include <optional>

#include "scope.hpp"
#include "symbol_entry.hpp"
#include "types.hpp"

Scope::Scope() {}

int Scope::get_size() const {
  return locals.size();
}

void Scope::insert(std::string name, std::shared_ptr<Type> t) {
  auto it = locals.find(name);
  if (it != locals.end())
    std::cout << "Variable has already been declared" << std::endl;
  locals[name] = SymbolEntry(t);
}

std::optional<SymbolEntry> Scope::lookup(std::string name) {
  auto it = locals.find(name);
  if (it != locals.end())
    return locals[name];
  else
    return std::nullopt;
}
