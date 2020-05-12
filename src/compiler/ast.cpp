#include <iostream>
#include <memory>

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

//------------------------------------------------------------------//
//---------------------------Constructors---------------------------//
//------------------------------------------------------------------//

Boolean::Boolean(bool val)
    : val(val) {}

Char::Char(char val)
    : val(val) {}

Integer::Integer(int val)
    : val(val) {}

Real::Real(double val)
    : val(val) {}

String::String(std::string val)
    : val(val) {}

Nil::Nil() {}

Variable::Variable(std::string name)
    : name(name) {}

Array::Array(expr_ptr arr, expr_ptr offset)
    : arr(std::move(arr)), offset(std::move(offset)) {}

Deref::Deref(expr_ptr ptr)
    : ptr(std::move(ptr)) {}

AddressOf::AddressOf(expr_ptr var)
    : var(std::move(var)) {}

CallExpr::CallExpr(std::string fun_name, std::vector<expr_ptr> parameters)
    : fun_name(fun_name), parameters(std::move(parameters)) {}

Result::Result() {}

BinaryExpr::BinaryExpr(std::string op, expr_ptr left, expr_ptr right)
    : op(op), left(std::move(left)), right(std::move(right)) {}

UnaryOp::UnaryOp(std::string op, expr_ptr operand)
    : op(op), operand(std::move(operand)) {}

Block::Block(std::vector<stmt_ptr> stmt_list)
    : stmt_list(std::move(stmt_list)) {}

VarNames::VarNames(std::vector<std::string> names, std::string type)
    : names(names), type(type) {}

VarDecl::VarDecl(std::vector<varnames_ptr> var_names)
    : var_names(std::move(var_names)) {}

LabelDecl::LabelDecl(std::vector<std::string> names)
    : names(names) {}

VarAssign::VarAssign(expr_ptr left, expr_ptr right)
    : left(std::move(left)), right(std::move(right)) {}

Goto::Goto(std::string label)
    : label(label) {}

Label::Label(std::string label, stmt_ptr stmt)
    : label(label), stmt(std::move(stmt)) {}

If::If(expr_ptr cond, stmt_ptr if_stmt, stmt_ptr else_stmt)
    : cond(std::move(cond)), if_stmt(std::move(if_stmt)), else_stmt(std::move(else_stmt)) {}

While::While(expr_ptr cond, stmt_ptr stmt)
    : cond(std::move(cond)), stmt(std::move(stmt)) {}

Formal::Formal(bool pass_by_reference, std::vector<std::string> names, std::string type)
    : pass_by_reference(pass_by_reference), names(names), type(type) {}

Body::Body(std::vector<local_ptr> local_decls, block_ptr block)
    : local_decls(std::move(local_decls)), block(std::move(block)) {}

Fun::Fun(std::string fun_name, std::string return_type, std::vector<formal_ptr> formal_parameters)
    : fun_name(fun_name), return_type(return_type), formal_parameters(std::move(formal_parameters)) {}

void Fun::set_body(body_ptr body) {
  this->body = std::move(body);
}

void Fun::set_forward(bool is_forward) {
  this->is_forward = is_forward;
}

CallStmt::CallStmt(std::string fun_name, std::vector<expr_ptr> parameters)
    : fun_name(fun_name), parameters(std::move(parameters)) {}

Return::Return() {}

New::New(expr_ptr size, expr_ptr l_value)
    : size(std::move(size)), l_value(std::move(l_value)) {}

Dispose::Dispose(bool has_brackets, expr_ptr l_value)
    : has_brackets(has_brackets), l_value(std::move(l_value)) {}

Program::Program(std::string name, body_ptr body)
    : name(name), body(std::move(body)) {}

//------------------------------------------------------------------//
//-----------------------------Codegen------------------------------//
//------------------------------------------------------------------//

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

Value *String::codegen() {}

Value *Nil::codegen() {}

Value *Variable::codegen() {}

Value *Array::codegen() {}

Value *Deref::codegen() {}

Value *AddressOf::codegen() {}

Value *CallExpr::codegen() {}

Value *Result::codegen() {}

Value *BinaryExpr::codegen() {}

Value *UnaryOp::codegen() {}

Value *Block::codegen() {}

Value *VarNames::codegen() {}

Value *VarDecl::codegen() {}

Value *LabelDecl::codegen() {}

Value *VarAssign::codegen() {}

Value *Goto::codegen() {}

Value *Label::codegen() {}

Value *If::codegen() {}

Value *While::codegen() {}

Value *Formal::codegen() {}

Value *Body::codegen() {}

Value *Fun::codegen() {}

Value *CallStmt::codegen() {}

Value *Return::codegen() {}

Value *New::codegen() {}

Value *Dispose::codegen() {}

Value *Program::codegen() {}
