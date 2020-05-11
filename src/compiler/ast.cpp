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

//------------------------------------------------------------------//
//---------------------------Constructors---------------------------//
//------------------------------------------------------------------//

Boolean::Boolean(bool val) : val(val) {}

Char::Char(char val) : val(val) {}

Integer::Integer(int val) : val(val) {}

Real::Real(double val) : val(val) {}

String::String(std::string val) : val(val) {}

Nil::Nil() {}

Variable::Variable(std::string name, bool get_addr, int offset)
    : name(name), get_addr(get_addr), offset(offset) {}

BinaryExpr::BinaryExpr(std::string op, std::unique_ptr<Expr> left,
                       std::unique_ptr<Expr> right)
    : op(op), left(std::move(left)), right(std::move(right)) {}

UnaryOp::UnaryOp(std::string op, std::unique_ptr<Expr> operand)
    : op(op), operand(std::move(operand)) {}

Block::Block(std::vector<std::unique_ptr<Stmt>> stmt_list)
    : stmt_list(std::move(stmt_list)) {}

VarNames::VarNames(std::vector<std::string> names, std::string type)
    : names(names), type(type) {}

VarDecl::VarDecl(std::vector<std::unique_ptr<VarNames>> var_names)
    : var_names(std::move(var_names)) {}

LabelDecl::LabelDecl(std::vector<std::string> names) : names(names) {}

VarAssign::VarAssign(std::unique_ptr<Expr> left, std::unique_ptr<Expr> right)
    : left(std::move(left)), right(std::move(right)) {}

Goto::Goto(std::string label) : label(label) {}

Label::Label(std::string label, std::unique_ptr<Stmt> statement)
    : label(label), statement(std::move(statement)) {}

If::If(std::unique_ptr<Expr> cond, std::unique_ptr<Stmt> if_statement,
       std::unique_ptr<Stmt> else_statement)
    : cond(std::move(cond)), if_statement(std::move(if_statement)),
      else_statement(std::move(else_statement)) {}

While::While(std::unique_ptr<Expr> cond, std::unique_ptr<Stmt> statement)
    : cond(std::move(cond)), statement(std::move(statement)) {}

Formal::Formal(bool pass_by_reference, std::vector<std::string> names,
               std::string type)
    : pass_by_reference(pass_by_reference), names(names), type(type) {}

Body::Body(std::vector<std::unique_ptr<Local>> local_decls,
           std::unique_ptr<Block> block)
    : local_decls(std::move(local_decls)), block(std::move(block)) {}

Fun::Fun(std::string fun_name, std::string return_type,
         std::vector<std::unique_ptr<Formal>> formal_parameters)
    : fun_name(fun_name), return_type(return_type),
      formal_parameters(std::move(formal_parameters)) {}

Call::Call(std::string fun_name, std::vector<std::unique_ptr<Expr>> parameters)
    : fun_name(fun_name), parameters(std::move(parameters)) {}

Return::Return() {}

Result::Result() {}

New::New(int size, std::unique_ptr<Expr> l_value)
    : size(size), l_value(std::move(l_value)) {}

Dispose::Dispose(bool has_brackets, std::unique_ptr<Expr> l_value)
    : has_brackets(has_brackets), l_value(std::move(l_value)) {}

Program::Program(std::string name, std::unique_ptr<Body> body)
    : name(name), body(std::move(body)) {}

//------------------------------------------------------------------//
//-----------------------------Codegen------------------------------//
//------------------------------------------------------------------//
/*
Value *Boolean::codegen() {
  return ConstantInt::get(TheContext, APInt(8, val, true));
}

Value *Char::codegen() {
  return ConstantInt::get(TheContext, APInt(8, val, true));
}

Value *Integer::codegen() {
  return ConstantInt::get(TheContext, APInt(16, val, true));
}

Value *Real::codegen() { return ConstantFP::get(TheContext, APFloat(val)); }

Value *Array_n::codegen() {}

Value *Array::codegen() {}

Value *Pointer::codegen() {}

Value *Id::codegen() {}

Value *BinaryExpr::codegen() {
  Value *l = left->codegen();
  Value *r = right->codegen();
  if (!l || !r)
    return nullptr;
  // Add instruction for integers and fadd for floats
  switch (op) {
  case "+":
    return;
  case "-":
    return;
  case "*":
    return;
  case "/":
    return;
  case "div":
    return;
  case "mod":
    return;
  case "or":
    return;
  case "and":
    return;
  case "=":
    return;
  case "<>":
    return;
  case "<":
    return;
  case "<=":
    return;
  case ">":
    return;
    case ">=";
        return;
    }
    return nullptr;
}

Value *If::codegen() {
}

Value *Block::codegen() {
}

Value *Fun::codegen() {
}*/
