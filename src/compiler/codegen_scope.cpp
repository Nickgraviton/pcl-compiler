#include <map>
#include <memory>
#include <string>

#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Value.h>

#include "codegen_entry.hpp"
#include "codegen_scope.hpp"

using namespace llvm;

void CodegenScope::insert_var(std::string name, Value* alloca) {
  this->var_map[name] = alloca;
}

void CodegenScope::insert_label(std::string name, BasicBlock* block) {
  this->label_map[name] = block;
}

void CodegenScope::insert_fun(std::string name, std::shared_ptr<FunDef> fun) {
  this->fun_map[name] = fun;
}

Value* CodegenScope::lookup_var(std::string name) {
  auto it = this->var_map.find(name);
  if (it != this->var_map.end())
    return this->var_map[name];
  else
    return nullptr;
}

BasicBlock* CodegenScope::lookup_label(std::string name) {
  auto it = this->label_map.find(name);
  if (it != this->label_map.end())
    return this->label_map[name];
  else
    return nullptr;
}

std::shared_ptr<FunDef> CodegenScope::lookup_fun(std::string name) {
  auto it = this->fun_map.find(name);
  if (it != this->fun_map.end())
    return this->fun_map[name];
  else
    return nullptr;
}
