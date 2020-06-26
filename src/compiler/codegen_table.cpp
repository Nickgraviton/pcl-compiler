#include <memory>
#include <string>
#include <vector>

#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Value.h>

#include "codegen_entry.hpp"
#include "codegen_scope.hpp"
#include "codegen_table.hpp"

using namespace llvm;

void CodegenTable::open_scope() {
  this->scopes.push_back(CodegenScope());
}

void CodegenTable::close_scope() {
  this->scopes.pop_back();
}

void CodegenTable::insert_var(std::string name, Value* alloca) {
  this->scopes.back().insert_var(name, alloca);
}

void CodegenTable::insert_label(std::string name, BasicBlock* block) {
  this->scopes.back().insert_label(name, block);
}

void CodegenTable::insert_fun(std::string name, std::shared_ptr<FunDef> fun) {
  this->scopes.back().insert_fun(name, fun);
}

Value* CodegenTable::lookup_var(std::string name) {
  for (auto r_it = std::rbegin(this->scopes); r_it != std::rend(this->scopes); r_it++) {
    auto result = r_it->lookup_var(name);
    if (result)
      return result;
  }
  return nullptr;
}

BasicBlock* CodegenTable::lookup_label(std::string name) {
  return this->scopes.back().lookup_label(name);
}

std::shared_ptr<FunDef> CodegenTable::lookup_fun(std::string name) {
  for (auto r_it = std::rbegin(this->scopes); r_it != std::rend(this->scopes); r_it++) {
    auto result = r_it->lookup_fun(name);
    if (result)
      return result;
  }
  return nullptr;
}
