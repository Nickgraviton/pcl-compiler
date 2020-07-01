#include <map>
#include <memory>
#include <string>
#include <vector>

#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Value.h>

#include "codegen_table.hpp"
#include "symbol_table.hpp"
#include "types.hpp"

using namespace llvm;

FunDef::FunDef(Type* return_type, std::vector<bool>& parameters, Function* F, std::vector<std::shared_ptr<VarInfo>> prev_scope_vars, int nesting_level)
  : return_type(return_type), parameters(parameters), F(F), prev_scope_vars(prev_scope_vars), nesting_level(nesting_level) {}

void FunDef::set_prev_scope_vars(std::vector<std::shared_ptr<VarInfo>>& prev_scope_vars) {
  this->prev_scope_vars = prev_scope_vars;
}

Type* FunDef::get_return_type() {
  return this->return_type;
}

std::vector<bool>& FunDef::get_parameters() {
  return this->parameters;
}

Function* FunDef::get_function() {
  return this->F;
}

std::vector<std::shared_ptr<VarInfo>>& FunDef::get_prev_scope_vars() {
  return this->prev_scope_vars;
}

int FunDef::get_nesting_level() {
  return this->nesting_level;
}

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

std::string CodegenScope::reverse_lookup_fun(Function* F) {
  for (auto it = std::begin(fun_map); it != std::end(fun_map); it++)
    if (it->second->get_function() == F)
      return it->first;
  return "";
}

int CodegenTable::get_nesting_level() {
  return this->scopes.size();
}

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
  return this->scopes.back().lookup_var(name);
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

std::string CodegenTable::reverse_lookup_fun(Function* F) {
  for (auto r_it = std::rbegin(this->scopes); r_it != std::rend(this->scopes); r_it++) {
    auto result = r_it->reverse_lookup_fun(F);
    if (result != "")
      return result;
  }
  return nullptr;
}
