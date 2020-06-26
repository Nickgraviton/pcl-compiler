#include <memory>
#include <utility>
#include <vector>

#include <llvm/IR/Function.h>
#include <llvm/IR/Type.h>

#include "codegen_entry.hpp"

using namespace llvm;

FunDef::FunDef(Type* return_type, std::vector<bool>& parameters, Function* F)
  : return_type(return_type), parameters(parameters), F(F) {}

Type* FunDef::get_return_type() {
  return this->return_type;
}

std::vector<bool>& FunDef::get_parameters() {
  return this->parameters;
}

Function* FunDef::get_function() {
  return this->F;
}
