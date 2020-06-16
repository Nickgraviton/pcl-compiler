#include <string>
#include <vector>

#include <llvm/IR/Instructions.h>

#include "codegen_map.hpp"

using namespace llvm;

void CodegenMap::open_scope() {
  this->map.push_back(std::map<std::string, AllocaInst*>());
}

void CodegenMap::close_scope() {
  this->map.pop_back();
}

void CodegenMap::insert(std::string name, AllocaInst* alloca) {
  this->map.back()[name] = alloca;
}

AllocaInst* CodegenMap::lookup(std::string name) {
  for (auto r_it = std::rbegin(this->map); r_it != std::rend(this->map); r_it++) {
    auto local_scope = *r_it;
    auto it = local_scope.find(name);
    if (it != local_scope.end())
      return local_scope[name];
  }
  return nullptr;
}
