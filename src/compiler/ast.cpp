#include <iostream>
#include <memory>

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Value.h>

#include "ast.hpp"
#include "types.hpp"

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

Empty::Empty() {}

Block::Block(std::vector<stmt_ptr> stmt_list)
    : stmt_list(std::move(stmt_list)) {}

VarNames::VarNames(std::vector<std::string> names, typeinfo_ptr type)
    : names(names), type(std::move(type)) {}

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

Formal::Formal(bool pass_by_reference, std::vector<std::string> names, typeinfo_ptr type)
    : pass_by_reference(pass_by_reference), names(names), type(std::move(type)) {}

Body::Body(std::vector<local_ptr> local_decls, block_ptr block)
    : local_decls(std::move(local_decls)), block(std::move(block)) {}

Fun::Fun(std::string fun_name, typeinfo_ptr return_type, std::vector<formal_ptr> formal_parameters)
    : fun_name(fun_name), return_type(std::move(return_type)), formal_parameters(std::move(formal_parameters)) {}

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
//------------------------------Print-------------------------------//
//------------------------------------------------------------------//

// Helper function that prints the current indent level number of spaces
void print_level(std::ostream& out, int level) {
  out << std::string(level, ' ');
}

void Boolean::print(std::ostream& out, int level) const {
  print_level(out, level);
  out << "Boolean(" << val << ")" << std::endl;
}

void Char::print(std::ostream& out, int level) const {
  print_level(out, level);
  out << "Char(" << val << ")" << std::endl;
}

void Integer::print(std::ostream& out, int level) const {
  print_level(out, level);
  out << "Integer(" << val << ")" << std::endl;
}

void Real::print(std::ostream& out, int level) const {
  print_level(out, level);
  out << "Real(" << val << ")" << std::endl;
}

void String::print(std::ostream& out, int level) const {
  print_level(out, level);
  out << "String(" << val << ")" << std::endl;
}

void Nil::print(std::ostream& out, int level) const {
  print_level(out, level);
  out << "Nil()" << std::endl;
}

void Variable::print(std::ostream& out, int level) const {
  print_level(out, level);
  out << "Variable(" << name << ")" << std::endl;
}

void Array::print(std::ostream& out, int level) const {
  print_level(out, level);
  out << "Array(arr, offset):" << std::endl;
  arr->print(out, level + 1);
  offset->print(out, level + 1);
}

void Deref::print(std::ostream& out, int level) const {
  print_level(out, level);
  out << "Deref(ptr):" << std::endl;
  ptr->print(out, level + 1);
}

void AddressOf::print(std::ostream& out, int level) const {
  print_level(out, level);
  out << "AddressOf(var):" << std::endl;
  var->print(out, level + 1);
}

void CallExpr::print(std::ostream& out, int level) const {
  print_level(out, level);
  out << "CallExpr(" << fun_name << ", parameters):" << std::endl;
  for (auto &p : parameters)
    p->print(out, level + 1);
}

void Result::print(std::ostream& out, int level) const {
  print_level(out, level);
  out << "Result()" << std::endl;
}

void BinaryExpr::print(std::ostream& out, int level) const {
  print_level(out, level);
  out << "BinaryExpr(" << op << ", left, right):" << std::endl;
  left->print(out, level + 1);
  right->print(out, level + 1);
}

void UnaryOp::print(std::ostream& out, int level) const {
  print_level(out, level);
  out << "UnaryOp(" << op << ", operand):" << std::endl;
  operand->print(out, level + 1);
}

void Empty::print(std::ostream& out, int level) const {
  print_level(out, level);
  out << "Empty()" << std::endl;
}

void Block::print(std::ostream& out, int level) const {
  print_level(out, level);
  out << "Block(stmt_list):" << std::endl;
  for (auto &s : stmt_list)
    s->print(out, level + 1);
}

void VarNames::print(std::ostream& out, int level) const {
  print_level(out, level);
  out << "VarNames(";
  type->print(out);
  out << ", names):" << std::endl;
  for (auto &n : names) {
    print_level(out, level + 1);
    out << n << std::endl;
  }
}

void VarDecl::print(std::ostream& out, int level) const {
  print_level(out, level);
  out << "VarDecl(var_names):" << std::endl;
  for (auto &v : var_names)
    v->print(out, level + 1);
}

void LabelDecl::print(std::ostream& out, int level) const {
  print_level(out, level);
  out << "LabelDecl(names):" << std::endl;
  for (auto &n : names) {
    print_level(out, level + 1);
    out << n << std::endl;
  }
}

void VarAssign::print(std::ostream& out, int level) const {
  print_level(out, level);
  out << "VarAssign(left, right):" << std::endl;
  left->print(out, level + 1);
  right->print(out, level + 1);
}

void Goto::print(std::ostream& out, int level) const {
  print_level(out, level);
  out << "Goto(" << label << ")" << std::endl;
}

void Label::print(std::ostream& out, int level) const {
  print_level(out, level);
  out << "Label(" << label << ", stmt):" << std::endl;
  stmt->print(out, level + 1);
}

void If::print(std::ostream& out, int level) const {
  print_level(out, level);
  out << "If(cond, if_stmt, else_stmt):" << std::endl;
  cond->print(out, level + 1);
  if_stmt->print(out, level + 1);
  if (else_stmt)
    else_stmt->print(out, level + 1);
}

void While::print(std::ostream& out, int level) const {
  print_level(out, level);
  out << "While(cond, stmt):" << std::endl;
  cond->print(out, level + 1);
  stmt->print(out, level + 1);
}

void Formal::print(std::ostream& out, int level) const {
  print_level(out, level);
  out << "Formal(pass_by_reference: " << pass_by_reference << ", names, ";
  type->print(out);
  out << "):" << std::endl;
  for (auto &n : names) {
    print_level(out, level + 1);
    out << n << std::endl;
  }
}

void Body::print(std::ostream& out, int level) const {
  print_level(out, level);
  out << "Body(local_decls, block):" << std::endl;
  for (auto &l : local_decls)
    l->print(out, level + 1);
  block->print(out, level + 1);
}

void Fun::print(std::ostream& out, int level) const {
  print_level(out, level);
  out << "Fun(" << fun_name << ", ";
  if (return_type)
    return_type->print(out);
  out << ", formal_parameters, body, is_forward: " << is_forward << "):" << std::endl;
  for (auto &f : formal_parameters)
    f->print(out, level + 1);
  body->print(out, level + 1);
}

void CallStmt::print(std::ostream& out, int level) const {
  print_level(out, level);
  out << "CallStmt(" << fun_name << ", parameters):" << std::endl;
  for (auto &p : parameters)
    p->print(out, level + 1);
}

void Return::print(std::ostream& out, int level) const {
  print_level(out, level);
  out << "Return()" << std::endl;
}

void New::print(std::ostream& out, int level) const {
  print_level(out, level);
  out << "New(size, l_value):" << std::endl;
  if (size)
    size->print(out, level + 1);
  l_value->print(out, level + 1);
}

void Dispose::print(std::ostream& out, int level) const {
  print_level(out, level);
  out << "Dispose(has_brackets: " << has_brackets << ", l_value):" << std::endl;
  l_value->print(out, level + 1);
}

void Program::print(std::ostream& out, int level) const {
  print_level(out, level);
  out << "Program(" << name << ", body):" << std::endl;
  body->print(out, level + 1);
}

//------------------------------------------------------------------//
//-----------------------------Codegen------------------------------//
//------------------------------------------------------------------//

Value *Boolean::codegen() const {
  return ConstantInt::get(TheContext, APInt(8, val, true));
}

Value *Char::codegen() const {
  return ConstantInt::get(TheContext, APInt(8, val, true));
}

Value *Integer::codegen() const {
  return ConstantInt::get(TheContext, APInt(16, val, true));
}

Value *Real::codegen() const {
  return ConstantFP::get(TheContext, APFloat(val));
}

Value *String::codegen() const {}

Value *Nil::codegen() const {}

Value *Variable::codegen() const {}

Value *Array::codegen() const {}

Value *Deref::codegen() const {}

Value *AddressOf::codegen() const {}

Value *CallExpr::codegen() const {}

Value *Result::codegen() const {}

Value *BinaryExpr::codegen() const {}

Value *UnaryOp::codegen() const {}

Value *Empty::codegen() const {}

Value *Block::codegen() const {}

Value *VarNames::codegen() const {}

Value *VarDecl::codegen() const {}

Value *LabelDecl::codegen() const {}

Value *VarAssign::codegen() const {}

Value *Goto::codegen() const {}

Value *Label::codegen() const {}

Value *If::codegen() const {}

Value *While::codegen() const {}

Value *Formal::codegen() const {}

Value *Body::codegen() const {}

Value *Fun::codegen() const {}

Value *CallStmt::codegen() const {}

Value *Return::codegen() const {}

Value *New::codegen() const {}

Value *Dispose::codegen() const {}

Value *Program::codegen() const {}
