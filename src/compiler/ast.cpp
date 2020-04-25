#include <iostream>

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Value.h>

#include "ast.hpp"

using namespace llvm;

static LLVMContext TheContext;
static IRBuilder<> Builder(TheContext);
static std::unique_ptr<Module> TheModule;
static std::map<std::string, Value *> NamedValues;

Value *Boolean::codegen() {
    return ConstantInt::get(TheContext, APInt(8, val, true));
}

Value *Char::codegen() {
    return ConstantInt::get(TheContext, APInt(8, val, true));
}

Value *Integer::codegen() {
    return ConstantInt::get(TheContext, APInt(16, val, true));
}

Value *Real::codegen() {
    return ConstantFP::get(TheContext, APFloat(val));
}

Value *Array_n::codegen() {
}

Value *Array::codegen() {
}

Value *Pointer::codegen() {
}

Value *BinOp::codegen() {
}

Value *If::codegen() {
}

Value *For::codegen() {
}

Value *Block::codegen() {
}

Value *Fun::codegen() {
}
