#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>

#include <llvm/IR/DataLayout.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/Verifier.h>
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"
#include <llvm/Transforms/InstCombine/InstCombine.h>
#include <llvm/Transforms/Scalar.h>
#include <llvm/Transforms/Scalar/GVN.h>
#include <llvm/Transforms/Utils.h>

#include "ast.hpp"
#include "codegen_table.hpp"
#include "lexer.hpp"
#include "symbol_table.hpp"
#include "types.hpp"

using expr_ptr = std::unique_ptr<Expr>;
using stmt_ptr = std::unique_ptr<Stmt>;
using local_ptr = std::unique_ptr<Local>;
using block_ptr = std::unique_ptr<Block>;
using varnames_ptr = std::unique_ptr<VarNames>;
using formal_ptr = std::unique_ptr<Formal>;
using body_ptr = std::unique_ptr<Body>;
using fun_ptr = std::unique_ptr<Fun>;
using program_ptr = std::unique_ptr<Program>;
using type_ptr = std::shared_ptr<TypeInfo>;

static SymbolTable symbol_table;

using namespace llvm;

static LLVMContext TheContext;
static IRBuilder<> Builder(TheContext);
static std::unique_ptr<Module> TheModule;
static std::unique_ptr<legacy::FunctionPassManager> TheFPM;

static CodegenTable codegen_table;

//---------------------------------------------------------------------//
//------------------Constructors/Getters/Setters-----------------------//
//---------------------------------------------------------------------//

Node::Node() {
  this->line = line_num;
}

int Node::get_line() const {
  return this->line;
}

Expr::Expr()
  : Node() {}

type_ptr Expr::get_type() const {
  return this->type;
}

Stmt::Stmt()
  : Node() {}

Boolean::Boolean(bool val)
  : Expr(), val(val) {}

Char::Char(char val)
  : Expr(), val(val) {}

Integer::Integer(int val)
  : Expr(), val(val) {}

Real::Real(double val)
  : Expr(), val(val) {}

String::String(std::string val)
  : Expr(), val(val) {}

Nil::Nil() 
  : Expr() {}

Variable::Variable(std::string name)
  : Expr(), name(name) {}

Array::Array(expr_ptr arr, expr_ptr index)
  : Expr(), arr(std::move(arr)), index(std::move(index)) {}

Deref::Deref(expr_ptr ptr)
  : Expr(), ptr(std::move(ptr)) {}

AddressOf::AddressOf(expr_ptr var)
  : Expr(), var(std::move(var)) {}

CallExpr::CallExpr(std::string fun_name, std::vector<expr_ptr> parameters)
  : Expr(), fun_name(fun_name), parameters(std::move(parameters)) {}

Result::Result()
  : Expr() {}

BinaryExpr::BinaryExpr(BinOp op, expr_ptr left, expr_ptr right)
  : Expr(), op(op), left(std::move(left)), right(std::move(right)) {}

UnaryExpr::UnaryExpr(UnOp op, expr_ptr operand)
  : Expr(), op(op), operand(std::move(operand)) {}

Local::Local()
  : Stmt() {}

Empty::Empty()
  : Stmt() {}

Block::Block(std::vector<stmt_ptr> stmt_list)
  : Stmt(), stmt_list(std::move(stmt_list)) {}

VarNames::VarNames(std::vector<std::string> names, type_ptr type)
  : Stmt(), names(names), type(type) {}

VarDecl::VarDecl(std::vector<varnames_ptr> var_names)
  : Local(), var_names(std::move(var_names)) {}

LabelDecl::LabelDecl(std::vector<std::string> names)
  : Local(), names(names) {}

Assign::Assign(expr_ptr left, expr_ptr right)
  : Stmt(), left(std::move(left)), right(std::move(right)) {}

Goto::Goto(std::string label)
  : Stmt(), label(label) {}

Label::Label(std::string label, stmt_ptr stmt)
  : Stmt(), label(label), stmt(std::move(stmt)) {}

If::If(expr_ptr cond, stmt_ptr if_stmt, stmt_ptr else_stmt)
  : Stmt(), cond(std::move(cond)), if_stmt(std::move(if_stmt)), else_stmt(std::move(else_stmt)) {}

While::While(expr_ptr cond, stmt_ptr body)
  : Stmt(), cond(std::move(cond)), body(std::move(body)) {}

Formal::Formal(bool pass_by_reference, std::vector<std::string> names, type_ptr type)
  : Stmt(), pass_by_reference(pass_by_reference), names(names), type(type) {}

bool Formal::get_pass_by_reference() const {
  return this->pass_by_reference;
}

std::vector<std::string> Formal::get_names() const {
  return this->names;
}

type_ptr Formal::get_type() const {
  return this->type;
}

Body::Body(std::vector<local_ptr> local_decls, block_ptr block)
  : Stmt(), local_decls(std::move(local_decls)), block(std::move(block)) {}

Fun::Fun(std::string fun_name, type_ptr return_type, std::vector<formal_ptr> formal_parameters)
  : Local(), fun_name(fun_name), return_type(return_type), formal_parameters(std::move(formal_parameters)) {}

void Fun::set_body(body_ptr body) {
  this->body = std::move(body);
}

void Fun::set_forward(bool forward_declaration) {
  this->forward_declaration = forward_declaration;
}

CallStmt::CallStmt(std::string fun_name, std::vector<expr_ptr> parameters)
  : Stmt(), fun_name(fun_name), parameters(std::move(parameters)) {}

Return::Return()
  : Stmt() {}

New::New(expr_ptr size, expr_ptr l_value)
  : Stmt(), size(std::move(size)), l_value(std::move(l_value)) {}

Dispose::Dispose(bool has_brackets, expr_ptr l_value)
  : Stmt(), has_brackets(has_brackets), l_value(std::move(l_value)) {}

Program::Program(std::string name, body_ptr body)
  : Stmt(), name(name), body(std::move(body)), optimize(false), asm_output(false), imm_output(false) {}

void Program::set_file_name(std::string file_name) {
  this->file_name = file_name;
}

void Program::set_optimize(bool optimize) {
  this->optimize = optimize;
}

void Program::set_asm_output(bool asm_output) {
  this->asm_output = asm_output;
}

void Program::set_imm_output(bool imm_output) {
  this->imm_output = imm_output;
}

//---------------------------------------------------------------------//
//----------------------------Print------------------------------------//
//---------------------------------------------------------------------//

// Helper function that prints indent levels for the AST
static void print_level(std::ostream& out, int level) {
  while(level-- > 0)
    out << "  |";
  out << "--";
}

void Boolean::print(std::ostream& out, int level) const {
  print_level(out, level);
  out << "Boolean(" << this->val << ")" << std::endl;
}

void Char::print(std::ostream& out, int level) const {
  print_level(out, level);
  out << "Char(" << this->val << ")" << std::endl;
}

void Integer::print(std::ostream& out, int level) const {
  print_level(out, level);
  out << "Integer(" << this->val << ")" << std::endl;
}

void Real::print(std::ostream& out, int level) const {
  print_level(out, level);
  out << "Real(" << this->val << ")" << std::endl;
}

void String::print(std::ostream& out, int level) const {
  print_level(out, level);
  out << "String(" << this->val << ")" << std::endl;
}

void Nil::print(std::ostream& out, int level) const {
  print_level(out, level);
  out << "Nil()" << std::endl;
}

void Variable::print(std::ostream& out, int level) const {
  print_level(out, level);
  out << "Variable(name: " << this->name << ")" << std::endl;
}

void Array::print(std::ostream& out, int level) const {
  print_level(out, level);
  out << "Array(arr, index):" << std::endl;
  this->arr->print(out, level + 1);
  this->index->print(out, level + 1);
}

void Deref::print(std::ostream& out, int level) const {
  print_level(out, level);
  out << "Deref(ptr):" << std::endl;
  this->ptr->print(out, level + 1);
}

void AddressOf::print(std::ostream& out, int level) const {
  print_level(out, level);
  out << "AddressOf(var):" << std::endl;
  this->var->print(out, level + 1);
}

void CallExpr::print(std::ostream& out, int level) const {
  print_level(out, level);
  out << "CallExpr(fun_name: " << this->fun_name << ", parameters):" << std::endl;
  for (auto& p : this->parameters)
    p->print(out, level + 1);
}

void Result::print(std::ostream& out, int level) const {
  print_level(out, level);
  out << "Result()" << std::endl;
}

void BinaryExpr::print(std::ostream& out, int level) const {
  print_level(out, level);
  out << "BinaryExpr(op: " << binop_to_string(this->op) << ", left, right):" << std::endl;
  this->left->print(out, level + 1);
  this->right->print(out, level + 1);
}

void UnaryExpr::print(std::ostream& out, int level) const {
  print_level(out, level);
  out << "UnaryExpr(op: " << unop_to_string(this->op) << ", operand):" << std::endl;
  this->operand->print(out, level + 1);
}

void Empty::print(std::ostream& out, int level) const {
  print_level(out, level);
  out << "Empty()" << std::endl;
}

void Block::print(std::ostream& out, int level) const {
  print_level(out, level);
  out << "Block(stmt_list):" << std::endl;
  for (auto& s : this->stmt_list)
    s->print(out, level + 1);
}

void VarNames::print(std::ostream& out, int level) const {
  print_level(out, level);
  out << "VarNames(type: ";
  this->type->print(out);
  out << ", names):" << std::endl;
  for (auto& n : this->names) {
    print_level(out, level + 1);
    out << n << std::endl;
  }
}

void VarDecl::print(std::ostream& out, int level) const {
  print_level(out, level);
  out << "VarDecl(var_names):" << std::endl;
  for (auto& v : this->var_names)
    v->print(out, level + 1);
}

void LabelDecl::print(std::ostream& out, int level) const {
  print_level(out, level);
  out << "LabelDecl(names):" << std::endl;
  for (auto& n : this->names) {
    print_level(out, level + 1);
    out << n << std::endl;
  }
}

void Assign::print(std::ostream& out, int level) const {
  print_level(out, level);
  out << "Assign(left, right):" << std::endl;
  this->left->print(out, level + 1);
  this->right->print(out, level + 1);
}

void Goto::print(std::ostream& out, int level) const {
  print_level(out, level);
  out << "Goto(label: " << this->label << ")" << std::endl;
}

void Label::print(std::ostream& out, int level) const {
  print_level(out, level);
  out << "Label(label: " << this->label << ", stmt):" << std::endl;
  this->stmt->print(out, level + 1);
}

void If::print(std::ostream& out, int level) const {
  print_level(out, level);
  out << "If(cond, if_stmt, else_stmt):" << std::endl;
  this->cond->print(out, level + 1);
  this->if_stmt->print(out, level + 1);
  if (this->else_stmt)
    this->else_stmt->print(out, level + 1);
}

void While::print(std::ostream& out, int level) const {
  print_level(out, level);
  out << "While(cond, body):" << std::endl;
  this->cond->print(out, level + 1);
  this->body->print(out, level + 1);
}

void Formal::print(std::ostream& out, int level) const {
  print_level(out, level);
  out << "Formal(pass_by_reference: " << this->pass_by_reference << ", names, type: ";
  this->type->print(out);
  out << "):" << std::endl;
  for (auto& n : this->names) {
    print_level(out, level + 1);
    out << n << std::endl;
  }
}

void Body::print(std::ostream& out, int level) const {
  print_level(out, level);
  out << "Body(local_decls, block):" << std::endl;
  for (auto& l : this->local_decls)
    l->print(out, level + 1);
  this->block->print(out, level + 1);
}

void Fun::print(std::ostream& out, int level) const {
  print_level(out, level);
  out << "Fun(fun_name: " << this->fun_name << ", ";
  if (this->return_type) {
    out << "type: ";
    this->return_type->print(out);
    out << ", ";
  }
  out << "formal_parameters, body, forward_declaration: " << this->forward_declaration << "):" << std::endl;
  for (auto& f : this->formal_parameters)
    f->print(out, level + 1);
  this->body->print(out, level + 1);
}

void CallStmt::print(std::ostream& out, int level) const {
  print_level(out, level);
  out << "CallStmt(fun_name: " << this->fun_name << ", parameters):" << std::endl;
  for (auto& p : this->parameters)
    p->print(out, level + 1);
}

void Return::print(std::ostream& out, int level) const {
  print_level(out, level);
  out << "Return()" << std::endl;
}

void New::print(std::ostream& out, int level) const {
  print_level(out, level);
  out << "New(size, l_value):" << std::endl;
  if (this->size)
    this->size->print(out, level + 1);
  this->l_value->print(out, level + 1);
}

void Dispose::print(std::ostream& out, int level) const {
  print_level(out, level);
  out << "Dispose(has_brackets: " << this->has_brackets << ", l_value):" << std::endl;
  this->l_value->print(out, level + 1);
}

void Program::print(std::ostream& out, int level) const {
  print_level(out, level);
  out << "Program(name: " << this->name << ", body):" << std::endl;
  this->body->print(out, level + 1);
}


//---------------------------------------------------------------------//
//---------------------------Semantic----------------------------------//
//---------------------------------------------------------------------//

// Make the library functions visible
static void semantic_library_functions() {
  auto fun_entry = std::make_shared<FunctionEntry>(false, nullptr);
  auto parameter = std::make_pair(false, std::make_shared<VariableEntry>(std::make_shared<IntType>()));
  fun_entry->add_parameter(parameter);
  symbol_table.insert("writeInteger", fun_entry);

  fun_entry = std::make_shared<FunctionEntry>(false, nullptr);
  parameter = std::make_pair(false, std::make_shared<VariableEntry>(std::make_shared<BoolType>()));
  fun_entry->add_parameter(parameter);
  symbol_table.insert("writeBoolean", fun_entry);

  fun_entry = std::make_shared<FunctionEntry>(false, nullptr);
  parameter = std::make_pair(false, std::make_shared<VariableEntry>(std::make_shared<CharType>()));
  fun_entry->add_parameter(parameter);
  symbol_table.insert("writeChar", fun_entry);

  fun_entry = std::make_shared<FunctionEntry>(false, nullptr);
  parameter = std::make_pair(false, std::make_shared<VariableEntry>(std::make_shared<RealType>()));
  fun_entry->add_parameter(parameter);
  symbol_table.insert("writeReal", fun_entry);

  fun_entry = std::make_shared<FunctionEntry>(false, nullptr);
  parameter = std::make_pair(true, std::make_shared<VariableEntry>(std::make_shared<IArrType>(std::make_shared<CharType>())));
  fun_entry->add_parameter(parameter);
  symbol_table.insert("writeString", fun_entry);

  fun_entry = std::make_shared<FunctionEntry>(false, std::make_shared<IntType>());
  symbol_table.insert("readInteger", fun_entry);

  fun_entry = std::make_shared<FunctionEntry>(false, std::make_shared<BoolType>());
  symbol_table.insert("readBoolean", fun_entry);

  fun_entry = std::make_shared<FunctionEntry>(false, std::make_shared<CharType>());
  symbol_table.insert("readChar", fun_entry);

  fun_entry = std::make_shared<FunctionEntry>(false, std::make_shared<RealType>());
  symbol_table.insert("readReal", fun_entry);

  fun_entry = std::make_shared<FunctionEntry>(false, nullptr);
  parameter = std::make_pair(false, std::make_shared<VariableEntry>(std::make_shared<IntType>()));
  fun_entry->add_parameter(parameter);

  parameter = std::make_pair(true, std::make_shared<VariableEntry>(std::make_shared<IArrType>(std::make_shared<CharType>())));
  fun_entry->add_parameter(parameter);
  symbol_table.insert("readString", fun_entry);

  fun_entry = std::make_shared<FunctionEntry>(false, std::make_shared<IntType>());
  parameter = std::make_pair(false, std::make_shared<VariableEntry>(std::make_shared<IntType>()));
  fun_entry->add_parameter(parameter);
  symbol_table.insert("abs", fun_entry);

  fun_entry = std::make_shared<FunctionEntry>(false, std::make_shared<RealType>());
  parameter = std::make_pair(false, std::make_shared<VariableEntry>(std::make_shared<RealType>()));
  fun_entry->add_parameter(parameter);
  symbol_table.insert("fabs", fun_entry);

  fun_entry = std::make_shared<FunctionEntry>(false, std::make_shared<RealType>());
  parameter = std::make_pair(false, std::make_shared<VariableEntry>(std::make_shared<RealType>()));
  fun_entry->add_parameter(parameter);
  symbol_table.insert("sqrt", fun_entry);

  fun_entry = std::make_shared<FunctionEntry>(false, std::make_shared<RealType>());
  parameter = std::make_pair(false, std::make_shared<VariableEntry>(std::make_shared<RealType>()));
  fun_entry->add_parameter(parameter);
  symbol_table.insert("sin", fun_entry);

  fun_entry = std::make_shared<FunctionEntry>(false, std::make_shared<RealType>());
  parameter = std::make_pair(false, std::make_shared<VariableEntry>(std::make_shared<RealType>()));
  fun_entry->add_parameter(parameter);
  symbol_table.insert("cos", fun_entry);

  fun_entry = std::make_shared<FunctionEntry>(false, std::make_shared<RealType>());
  parameter = std::make_pair(false, std::make_shared<VariableEntry>(std::make_shared<RealType>()));
  fun_entry->add_parameter(parameter);
  symbol_table.insert("tan", fun_entry);

  fun_entry = std::make_shared<FunctionEntry>(false, std::make_shared<RealType>());
  parameter = std::make_pair(false, std::make_shared<VariableEntry>(std::make_shared<RealType>()));
  fun_entry->add_parameter(parameter);
  symbol_table.insert("arctan", fun_entry);

  fun_entry = std::make_shared<FunctionEntry>(false, std::make_shared<RealType>());
  parameter = std::make_pair(false, std::make_shared<VariableEntry>(std::make_shared<RealType>()));
  fun_entry->add_parameter(parameter);
  symbol_table.insert("exp", fun_entry);

  fun_entry = std::make_shared<FunctionEntry>(false, std::make_shared<RealType>());
  parameter = std::make_pair(false, std::make_shared<VariableEntry>(std::make_shared<RealType>()));
  fun_entry->add_parameter(parameter);
  symbol_table.insert("ln", fun_entry);

  fun_entry = std::make_shared<FunctionEntry>(false, std::make_shared<RealType>());
  symbol_table.insert("pi", fun_entry);

  fun_entry = std::make_shared<FunctionEntry>(false, std::make_shared<IntType>());
  parameter = std::make_pair(false, std::make_shared<VariableEntry>(std::make_shared<RealType>()));
  fun_entry->add_parameter(parameter);
  symbol_table.insert("trunc", fun_entry);

  fun_entry = std::make_shared<FunctionEntry>(false, std::make_shared<IntType>());
  parameter = std::make_pair(false, std::make_shared<VariableEntry>(std::make_shared<RealType>()));
  fun_entry->add_parameter(parameter);
  symbol_table.insert("round", fun_entry);

  fun_entry = std::make_shared<FunctionEntry>(false, std::make_shared<IntType>());
  parameter = std::make_pair(false, std::make_shared<VariableEntry>(std::make_shared<CharType>()));
  fun_entry->add_parameter(parameter);
  symbol_table.insert("ord", fun_entry);

  fun_entry = std::make_shared<FunctionEntry>(false, std::make_shared<CharType>());
  parameter = std::make_pair(false, std::make_shared<VariableEntry>(std::make_shared<IntType>()));
  fun_entry->add_parameter(parameter);
  symbol_table.insert("chr", fun_entry);
}

// Helper error function
static void error(const std::string& msg, const int& line) {
  std::cerr << "Line: " << line << " Error: " << msg << std::endl;
  exit(1);
}

void Boolean::semantic() {
  this->type = std::make_shared<BoolType>();
}

void Char::semantic() {
  this->type = std::make_shared<CharType>();
}

void Integer::semantic() {
  this->type = std::make_shared<IntType>();
}

void Real::semantic() {
  this->type = std::make_shared<RealType>();
}

void String::semantic() {
  this->type = std::make_shared<ArrType>(val.length(), std::make_shared<CharType>());
}

void Nil::semantic() {
  this->type = std::make_shared<PtrType>(nullptr);
}

void Variable::semantic() {
  auto entry = symbol_table.lookup(this->name);
  if (!entry)
    error("Identifier " + this->name + " hasn't been declared", this->get_line());

  auto variable_entry = std::dynamic_pointer_cast<VariableEntry>(entry);
  if (!variable_entry) 
    error("Name \"" + this->name + "\" has already been declared and is not a variable", this->get_line());

  this->type = variable_entry->get_type();
}

void Array::semantic() {
  this->arr->semantic();

  auto arr_type = this->arr->get_type();
  if (arr_type->is(BasicType::Array)) {
    auto a_t = std::static_pointer_cast<ArrType>(arr_type);
    this->type = a_t->get_subtype();
  } else if (arr_type->is(BasicType::IArray)) {
    auto ia_t = std::static_pointer_cast<IArrType>(arr_type);
    this->type = ia_t->get_subtype();
  } else {
    error("Variable is not of array type", this->get_line());
  }

  this->index->semantic();

  auto index_type = this->index->get_type();
  if (!index_type->is(BasicType::Integer))
    error("Array index is not of integer type", this->get_line());
}

void Deref::semantic() {
  this->ptr->semantic();

  auto ptr_type = this->ptr->get_type();
  if (!ptr_type->is(BasicType::Pointer))
    error("Variable is not of pointer type", this->get_line());

  auto p_t = std::static_pointer_cast<PtrType>(ptr_type);
  this->type = p_t->get_subtype();
}

void AddressOf::semantic() {
  this->var->semantic();

  auto var_type = this->var->get_type();

  this->type = std::make_shared<PtrType>(var_type);
}

// Helper function for the two call nodes
type_ptr call_semantic(const std::string& fun_name, const std::vector<expr_ptr>& call_parameters, const int& line) {
  auto entry = symbol_table.lookup(fun_name);
  if (!entry)
    error("Name of function " + fun_name + " not found", line);

  auto function_entry = std::dynamic_pointer_cast<FunctionEntry>(entry);
  if (!function_entry)
    error("Name \"" + fun_name + "\" has already been used and is not a function", line);

  for (auto& parameter : call_parameters)
    parameter->semantic();

  // Function parameters are a pair of a bool (denoting whether the variable is passed by reference)
  // and a variable entry which we can use to get its type
  auto fun_parameters = function_entry->get_parameters();
  int fun_param_count = fun_parameters.size();
  int call_param_count = call_parameters.size();

  if (call_param_count < fun_param_count) {
    error("Not enough arguments provided for the call of function \"" + fun_name + "\"", line);
  } else if (call_param_count > fun_param_count) {
    error("Too many arguments provided for the call of function \"" + fun_name + "\"", line);
  } else {
    for (int i = 0; i < call_param_count; i++) {
      auto call_param_type = call_parameters[i]->get_type();
      auto fun_param_type = fun_parameters[i].second->get_type();

      bool pass_by_reference = fun_parameters[i].first;
      bool array = ((call_param_type->is(BasicType::Array) || call_param_type->is(BasicType::IArray))
        || (fun_param_type->is(BasicType::Array) || fun_param_type->is(BasicType::IArray)))
        && !pass_by_reference;

      if (array)
        error("Arrays cannot be passed by value", line);
      
      if (!compatible_types(fun_param_type, call_param_type))
        error("Type of argument in function call does not match function definition", line);
    }
  }

  return function_entry->get_type();
}

void CallExpr::semantic() {
  this->type = call_semantic(this->fun_name, this->parameters, this->get_line());
}

void Result::semantic() {
  auto result = symbol_table.lookup("result");
  if (!result)
    error("\"result\" variable not used within the body of a function that returns a result", this->get_line());

  this->type = result->get_type();
}

void BinaryExpr::semantic() {
  this->left->semantic();
  this->right->semantic();

  auto left_type = this->left->get_type();
  auto right_type = this->right->get_type();

  switch(this->op) {
    case BinOp::PLUS:
    case BinOp::MINUS:
    case BinOp::MUL:
      // Check if both operands are arithmetic types and set the expression's type
      // to real if at least one is a real number
      if (!left_type->is(BasicType::Integer) && !left_type->is(BasicType::Real))
        error(binop_to_string(this->op) + " operands need to be either of real or integer type", this->get_line());

      if (!right_type->is(BasicType::Integer) && !right_type->is(BasicType::Real))
        error(binop_to_string(this->op) + " operands need to be either of real or integer type", this->get_line());

      if (left_type->is(BasicType::Real) || right_type->is(BasicType::Real))
        this->type = std::make_shared<RealType>();
      else
        this->type = std::make_shared<IntType>();

      break;

    case BinOp::DIV:
      // Check if both operands are arithmetic types and set the expression's type
      // to real
       if (!left_type->is(BasicType::Integer) && !left_type->is(BasicType::Real))
        error(binop_to_string(this->op) + " operands need to be either of real or integer type", this->get_line());

      if (!right_type->is(BasicType::Integer) && !right_type->is(BasicType::Real))
        error(binop_to_string(this->op) + " operands need to be either of real or integer type", this->get_line());

      this->type = std::make_shared<RealType>();

      break;

    case BinOp::INT_DIV:
    case BinOp::MOD:
      // Check if both operands are integers and set the expression's type
      // to integer
      if (!left_type->is(BasicType::Integer) || !right_type->is(BasicType::Integer))
        error(binop_to_string(this->op) + " operands need to be of integer type", this->get_line());

      this->type = std::make_shared<IntType>();

      break;

    case BinOp::EQ:
    case BinOp::NE:
    {
      // Check if both operands are arithmetic types or of the same type but not arrays
      // and set the expression's type to boolean
      bool arithmetic = (left_type->is(BasicType::Integer) || left_type->is(BasicType::Real))
        && (right_type->is(BasicType::Integer) || right_type->is(BasicType::Real));

      bool array = (left_type->is(BasicType::Array) || left_type->is(BasicType::IArray))
        || (right_type->is(BasicType::Array) || right_type->is(BasicType::IArray));

      if (!arithmetic && (array || !same_type(left_type, right_type)))
        error(binop_to_string(this->op) + " needs either arithmetic types or variables of the same type but not arrays", this->get_line());

      this->type = std::make_shared<BoolType>();

      break;
    }
    case BinOp::LT:
    case BinOp::GT:
    case BinOp::LE:
    case BinOp::GE:
      // Check if both operands are arithmetic types and set the expression's type to boolean
      if ((!left_type->is(BasicType::Integer) && !left_type->is(BasicType::Real))
            || (!right_type->is(BasicType::Integer) && !right_type->is(BasicType::Real)))
        error(binop_to_string(this->op) + " needs arithmetic types", this->get_line());

      this->type = std::make_shared<BoolType>();

      break;

   case BinOp::AND:
   case BinOp::OR:
      // Check if both operands are booleans and set the expression's type to boolean
      if (!left_type->is(BasicType::Boolean) || !right_type->is(BasicType::Boolean))
        error(binop_to_string(this->op) + " operands need to be of boolean type", this->get_line());
      
      this->type = std::make_shared<BoolType>();

      break;

   default:
    // Won't be reached
    error("Unkown operator", this->get_line());
  }
}

void UnaryExpr::semantic() {
  this->operand->semantic();

  auto operand_type = this->operand->get_type();

  switch(this->op) {
    case UnOp::PLUS:
    case UnOp::MINUS:
      if (!operand_type->is(BasicType::Integer) && !operand_type->is(BasicType::Real))
        error(unop_to_string(this->op) + " operand needs to be integer or real", this->get_line());

      break;

    case UnOp::NOT:
      if (!operand_type->is(BasicType::Boolean))
        error(unop_to_string(this->op) + " operand needs to be boolean", this->get_line());

      break;

    default:
      // Won't be reached
      error("Unkown unary operator", this->get_line());
  }

  this->type = operand_type;
}

void Empty::semantic() {}

void Block::semantic() {
  for (auto& stmt : this->stmt_list)
    stmt->semantic();
}

void VarNames::semantic() {
  for (auto& name : this->names) {
    bool success = symbol_table.insert(name, std::make_shared<VariableEntry>(this->type));
    if (!success)
      error("Variable name " + name + " has already been declared", this->get_line());
  }
}

void VarDecl::semantic() {
  for (auto& element : this->var_names)
    element->semantic();
}

void LabelDecl::semantic() {
  for (auto& name : this->names) {
    bool success = symbol_table.add_label(name);
    if (!success)
      error("Label " + name + " has already been declared", this->get_line());
  }
}

void Assign::semantic() {
  this->right->semantic();
  this->left->semantic();

  auto right_type = this->right->get_type();
  auto left_type = this->left->get_type();

  bool array = (right_type->is(BasicType::Array) || right_type->is(BasicType::IArray))
    || (left_type->is(BasicType::Array) || left_type->is(BasicType::IArray));

  if (array)
    error("Arrays cannot be assigned to directly", this->get_line());

  if (!compatible_types(left_type, right_type))
    error("Value cannot be assigned due to type mismatch", this->get_line());
}

void Goto::semantic() {
  if (!symbol_table.has_label(this->label))
    error("Label \"" + this->label + "\" hasn't been declared", this->get_line());
}

void Label::semantic() {
  if (!symbol_table.has_label(this->label))
    error("Label \"" + this->label + "\" hasn't been declared", this->get_line());

  this->stmt->semantic();
}

void If::semantic() {
  this->cond->semantic();

  auto condition_type = this->cond->get_type();
  if (!condition_type->is(BasicType::Boolean))
    error("Condition of if statement is not a boolean expression", this->get_line());

  this->if_stmt->semantic();

  if (this->else_stmt)
    this->else_stmt->semantic();
}

void While::semantic() {
  this->cond->semantic();

  auto condition_type = this->cond->get_type();
  if (!condition_type->is(BasicType::Boolean))
    error("Condition of while statement is not a boolean expression", this->get_line());

  this->body->semantic();
}

void Formal::semantic() {}

void Body::semantic() {
  for (auto& l : this->local_decls)
    l->semantic();

  this->block->semantic();
}

void Fun::semantic() {
  // Check if function entry exists in the symbol table and if it does make sure that functions
  // with bodies are allowed only if the already existing entry belongs to a forward declaration 
  auto entry = symbol_table.lookup(this->fun_name);
  if (entry) {
    auto function_entry = std::dynamic_pointer_cast<FunctionEntry>(entry);
    if (!function_entry)
      error(this->fun_name + " has already been declared and is not a function", this->get_line());
    if (this->forward_declaration || (!this->forward_declaration && !function_entry->is_forward()))
      error("Redeclaration of function is not permitted", this->get_line());
  }

  // Create the function entry in the symbol table that is inserted in the current scope
  auto fun_entry = std::make_shared<FunctionEntry>(this->forward_declaration, this->return_type);

  for (auto& formal : this->formal_parameters) {
    for (auto& name : formal->get_names()) {
      auto entry = std::make_shared<VariableEntry>(formal->get_type());
      fun_entry->add_parameter(std::make_pair(formal->get_pass_by_reference(), entry));
    }
  }

  bool success = symbol_table.insert(this->fun_name, fun_entry);
  if (!success)
    error(this->fun_name + " has already been declared and is not a function", this->get_line());

  // Open function's scope and insert the local variables and the result variable if not a procedure
  if (!this->forward_declaration) {
    this->prev_scope_vars = symbol_table.get_prev_scope_vars();

    symbol_table.open_scope();

    this->nesting_level = symbol_table.get_nesting_level();

    for (auto& formal : this->formal_parameters)
      for (auto& name : formal->get_names())
        symbol_table.insert(name, std::make_shared<VariableEntry>(formal->get_type()));

    if (this->return_type)
      symbol_table.insert("result", std::make_shared<VariableEntry>(this->return_type));
    else
      symbol_table.insert("result", nullptr);

    this->body->semantic();

    symbol_table.close_scope();
  }
}

void CallStmt::semantic() {
  call_semantic(this->fun_name, this->parameters, this->get_line());
}

void Return::semantic() {}

void New::semantic() {
  if (this->size)
    this->size->semantic();

  this->l_value->semantic();

  auto l_value_type = this->l_value->get_type();
  if (!l_value_type->is(BasicType::Pointer)) {
    error("New requires an l value of pointer type", this->get_line());
  } else {
    auto ptr_type = std::static_pointer_cast<PtrType>(l_value_type);
    auto subtype = ptr_type->get_subtype();

    if (this->size) {
      if (!subtype->is(BasicType::IArray))
        error("New with a size argument requires a pointer to array type", this->get_line());
      
      auto size_type = size->get_type();
      if (!size_type->is(BasicType::Integer))
        error("Expression within brackets needs to be of integer type", this->get_line());
    } else {
      if (!subtype->is_complete())
        error("New without a size argument requires a complete type", this->get_line());
    }
  }
}

void Dispose::semantic() {
  this->l_value->semantic();

  auto l_value_type = this->l_value->get_type();
  if (!l_value_type->is(BasicType::Pointer)) {
    error("Dispose requires an l value of pointer type", this->get_line());
  } else {
    auto ptr_type = std::static_pointer_cast<PtrType>(l_value_type);
    auto subtype = ptr_type->get_subtype();

    if (this->has_brackets) {
      if (!subtype->is(BasicType::IArray))
        error("Dispose with brackets requires a pointer to array type", this->get_line());
    } else {
      if (!subtype->is_complete())
        error("Dispose without brackets requires a complete type", this->get_line());
    }
  }
}

void Program::semantic() {
  symbol_table.open_scope();

  semantic_library_functions();

  this->body->semantic();

  symbol_table.close_scope();
}

//---------------------------------------------------------------------//
//----------------------------Util-------------------------------------//
//---------------------------------------------------------------------//

// Type shortcuts for:
// char,bool: i8  (1 byte)
// integer:   i32 (4 bytes)
// real:      f64 (8 bytes)
static Type* i8 = Type::getInt8Ty(TheContext);
static Type* i32 = Type::getInt32Ty(TheContext);
static Type* f64 = Type::getDoubleTy(TheContext);

static ConstantInt* c8(bool b) {
  return ConstantInt::get(TheContext, APInt(8, b, true));
}

static ConstantInt* c8(char c) {
  return ConstantInt::get(TheContext, APInt(8, c, true));
}

static ConstantInt* c32(int n) {
  return ConstantInt::get(TheContext, APInt(32, n, true));
}

static ConstantFP* c64(double d) {
  return ConstantFP::get(TheContext, APFloat(d));
}

static Type* to_llvm_type(type_ptr type) {
  if (!type)
    return Type::getVoidTy(TheContext);

  switch(type->get_basic_type()) {
    case BasicType::Integer:
      return i32;
    case BasicType::Real:
      return f64;
    case BasicType::Boolean:
      return i8;
    case BasicType::Char:
      return i8;
    case BasicType::Array:
    {
      auto array = std::static_pointer_cast<ArrType>(type);
      return ArrayType::get(to_llvm_type(array->get_subtype()), array->get_size());
    }
    case BasicType::IArray:
    {
      auto iarray = std::static_pointer_cast<IArrType>(type);
      return to_llvm_type(iarray->get_subtype());
    }
    case BasicType::Pointer:
    {
      auto ptr = std::static_pointer_cast<PtrType>(type);
      return to_llvm_type(ptr->get_subtype())->getPointerTo();
    }
    default:
      return nullptr;
  }
}

static void init_module_and_pass_manager(bool optimize) {
  TheModule = std::make_unique<Module>("PCL program", TheContext);

  TheFPM = std::make_unique<legacy::FunctionPassManager>(TheModule.get());

  if (optimize) {
    TheFPM->add(createPromoteMemoryToRegisterPass());
    TheFPM->add(createInstructionCombiningPass());
    TheFPM->add(createGVNPass());
    TheFPM->add(createCFGSimplificationPass());
  }

  TheFPM->doInitialization();

  InitializeAllTargetInfos();
  InitializeAllTargets();
  InitializeAllTargetMCs();
  InitializeAllAsmParsers();
  InitializeAllAsmPrinters();

  auto TargetTriple = sys::getDefaultTargetTriple();
  TheModule->setTargetTriple(TargetTriple);

  std::string Error;
  auto Target = TargetRegistry::lookupTarget(TargetTriple, Error);

  if (!Target) {
    errs() << Error;
    exit(1);
  }

  auto CPU = "generic";
  auto Features = "";

  TargetOptions opt;
  auto RM = Optional<Reloc::Model>();
  auto TheTargetMachine = Target->createTargetMachine(TargetTriple, CPU, Features, opt, RM);

  TheModule->setDataLayout(TheTargetMachine->createDataLayout());
}

static void codegen_library_functions() {
  Type* ret_type;
  std::vector<Type*> args;
  std::vector<bool> parameters;
  FunctionType* FT;
  Function* F;

  ret_type = Type::getVoidTy(TheContext);
  args = std::vector<Type*>{i32};
  parameters = std::vector<bool>{false};
  FT = FunctionType::get(ret_type, args, false);
  F = Function::Create(FT, Function::ExternalLinkage, "writeInteger", TheModule.get());

  codegen_table.insert_fun("writeInteger",
      std::make_shared<FunDef>(ret_type, parameters, F));

  ret_type = Type::getVoidTy(TheContext);
  args = std::vector<Type*>{i8};
  parameters = std::vector<bool>{false};
  FT = FunctionType::get(ret_type, args, false);
  F = Function::Create(FT, Function::ExternalLinkage, "writeBoolean", TheModule.get());

  codegen_table.insert_fun("writeBoolean",
      std::make_shared<FunDef>(ret_type, parameters, F));

  ret_type = Type::getVoidTy(TheContext);
  args = std::vector<Type*>{i8};
  parameters = std::vector<bool>{false};
  FT = FunctionType::get(ret_type, args, false);
  F = Function::Create(FT, Function::ExternalLinkage, "writeChar", TheModule.get());

  codegen_table.insert_fun("writeChar",
      std::make_shared<FunDef>(ret_type, parameters, F));

  ret_type = Type::getVoidTy(TheContext);
  args = std::vector<Type*>{f64};
  parameters = std::vector<bool>{false};
  FT = FunctionType::get(ret_type, args, false);
  F = Function::Create(FT, Function::ExternalLinkage, "writeReal", TheModule.get());

  codegen_table.insert_fun("writeReal",
      std::make_shared<FunDef>(ret_type, parameters, F));

  ret_type = Type::getVoidTy(TheContext);
  args = std::vector<Type*>{i8->getPointerTo()};
  parameters = std::vector<bool>{true};
  FT = FunctionType::get(ret_type, args, false);
  F = Function::Create(FT, Function::ExternalLinkage, "writeString", TheModule.get());

  codegen_table.insert_fun("writeString",
      std::make_shared<FunDef>(ret_type, parameters, F));

  ret_type = i32;
  args = std::vector<Type*>{};
  parameters = std::vector<bool>{};
  FT = FunctionType::get(ret_type, args, false);
  F = Function::Create(FT, Function::ExternalLinkage, "readInteger", TheModule.get());

  codegen_table.insert_fun("readInteger",
      std::make_shared<FunDef>(ret_type, parameters, F));

  ret_type = i8;
  args = std::vector<Type*>{};
  parameters = std::vector<bool>{};
  FT = FunctionType::get(ret_type, args, false);
  F = Function::Create(FT, Function::ExternalLinkage, "readBoolean", TheModule.get());

  codegen_table.insert_fun("readBoolean",
      std::make_shared<FunDef>(ret_type, parameters, F));

  ret_type = i8;
  args = std::vector<Type*>{};
  parameters = std::vector<bool>{};
  FT = FunctionType::get(ret_type, args, false);
  F = Function::Create(FT, Function::ExternalLinkage, "readChar", TheModule.get());

  codegen_table.insert_fun("readChar",
      std::make_shared<FunDef>(ret_type, parameters, F));

  ret_type = Type::getVoidTy(TheContext);
  args = std::vector<Type*>{i32, i8->getPointerTo()};
  parameters = std::vector<bool>{false, true};
  FT = FunctionType::get(ret_type, args, false);
  F = Function::Create(FT, Function::ExternalLinkage, "readString", TheModule.get());

  codegen_table.insert_fun("readString",
      std::make_shared<FunDef>(ret_type, parameters, F));

  ret_type = i32;
  args = std::vector<Type*>{i32};
  parameters = std::vector<bool>{false};
  FT = FunctionType::get(ret_type, args, false);
  F = Function::Create(FT, Function::ExternalLinkage, "abs", TheModule.get());

  codegen_table.insert_fun("abs",
      std::make_shared<FunDef>(ret_type, parameters, F));

  ret_type = f64;
  args = std::vector<Type*>{f64};
  parameters = std::vector<bool>{false};
  FT = FunctionType::get(ret_type, args, false);
  F = Function::Create(FT, Function::ExternalLinkage, "fabs", TheModule.get());

  codegen_table.insert_fun("fabs",
      std::make_shared<FunDef>(ret_type, parameters, F));

  ret_type = f64;
  args = std::vector<Type*>{f64};
  parameters = std::vector<bool>{false};
  FT = FunctionType::get(ret_type, args, false);
  F = Function::Create(FT, Function::ExternalLinkage, "sqrt", TheModule.get());

  codegen_table.insert_fun("sqrt",
      std::make_shared<FunDef>(ret_type, parameters, F));

  ret_type = f64;
  args = std::vector<Type*>{f64};
  parameters = std::vector<bool>{false};
  FT = FunctionType::get(ret_type, args, false);
  F = Function::Create(FT, Function::ExternalLinkage, "sin", TheModule.get());

  codegen_table.insert_fun("sin",
      std::make_shared<FunDef>(ret_type, parameters, F));

  ret_type = f64;
  args = std::vector<Type*>{f64};
  parameters = std::vector<bool>{false};
  FT = FunctionType::get(ret_type, args, false);
  F = Function::Create(FT, Function::ExternalLinkage, "cos", TheModule.get());

  codegen_table.insert_fun("cos",
      std::make_shared<FunDef>(ret_type, parameters, F));

  ret_type = f64;
  args = std::vector<Type*>{f64};
  parameters = std::vector<bool>{false};
  FT = FunctionType::get(ret_type, args, false);
  F = Function::Create(FT, Function::ExternalLinkage, "tan", TheModule.get());

  codegen_table.insert_fun("tan",
      std::make_shared<FunDef>(ret_type, parameters, F));

  ret_type = f64;
  args = std::vector<Type*>{f64};
  parameters = std::vector<bool>{false};
  FT = FunctionType::get(ret_type, args, false);
  F = Function::Create(FT, Function::ExternalLinkage, "arctan", TheModule.get());

  codegen_table.insert_fun("arctan",
      std::make_shared<FunDef>(ret_type, parameters, F));

  ret_type = f64;
  args = std::vector<Type*>{f64};
  parameters = std::vector<bool>{false};
  FT = FunctionType::get(ret_type, args, false);
  F = Function::Create(FT, Function::ExternalLinkage, "exp", TheModule.get());

  codegen_table.insert_fun("exp",
      std::make_shared<FunDef>(ret_type, parameters, F));

  ret_type = f64;
  args = std::vector<Type*>{f64};
  parameters = std::vector<bool>{false};
  FT = FunctionType::get(ret_type, args, false);
  F = Function::Create(FT, Function::ExternalLinkage, "ln", TheModule.get());

  codegen_table.insert_fun("ln",
      std::make_shared<FunDef>(ret_type, parameters, F));

  ret_type = f64;
  args = std::vector<Type*>{};
  parameters = std::vector<bool>{};
  FT = FunctionType::get(ret_type, args, false);
  F = Function::Create(FT, Function::ExternalLinkage, "pi", TheModule.get());

  codegen_table.insert_fun("pi",
      std::make_shared<FunDef>(ret_type, parameters, F));

  ret_type = i32;
  args = std::vector<Type*>{f64};
  parameters = std::vector<bool>{false};
  FT = FunctionType::get(ret_type, args, false);
  F = Function::Create(FT, Function::ExternalLinkage, "trunc", TheModule.get());

  codegen_table.insert_fun("trunc",
      std::make_shared<FunDef>(ret_type, parameters, F));

  ret_type = i32;
  args = std::vector<Type*>{f64};
  parameters = std::vector<bool>{false};
  FT = FunctionType::get(ret_type, args, false);
  F = Function::Create(FT, Function::ExternalLinkage, "round", TheModule.get());

  codegen_table.insert_fun("round",
      std::make_shared<FunDef>(ret_type, parameters, F));

  ret_type = i32;
  args = std::vector<Type*>{i8};
  parameters = std::vector<bool>{false};
  FT = FunctionType::get(ret_type, args, false);
  F = Function::Create(FT, Function::ExternalLinkage, "ord", TheModule.get());

  codegen_table.insert_fun("ord",
      std::make_shared<FunDef>(ret_type, parameters, F));

  ret_type = i8;
  args = std::vector<Type*>{i32};
  parameters = std::vector<bool>{false};
  FT = FunctionType::get(ret_type, args, false);
  F = Function::Create(FT, Function::ExternalLinkage, "chr", TheModule.get());

  codegen_table.insert_fun("chr",
      std::make_shared<FunDef>(ret_type, parameters, F));

  ret_type = i8->getPointerTo();
  args = std::vector<Type*>{Type::getInt64Ty(TheContext)};
  parameters = std::vector<bool>{false};
  FT = FunctionType::get(ret_type, args, false);
  F = Function::Create(FT, Function::ExternalLinkage, "malloc", TheModule.get());

  codegen_table.insert_fun("malloc",
      std::make_shared<FunDef>(ret_type, parameters, F));

  ret_type = Type::getVoidTy(TheContext);
  args = std::vector<Type*>{i8->getPointerTo()};
  parameters = std::vector<bool>{false};
  FT = FunctionType::get(ret_type, args, false);
  F = Function::Create(FT, Function::ExternalLinkage, "free", TheModule.get());

  codegen_table.insert_fun("free",
      std::make_shared<FunDef>(ret_type, parameters, F));
}

//---------------------------------------------------------------------//
//--------------------------Codegen------------------------------------//
//---------------------------------------------------------------------//

Value* Boolean::codegen() const {
  return c8(this->val);
}

Value* Char::codegen() const {
  return c8(this->val);
}

Value* Integer::codegen() const {
  return c32(this->val);
}

Value* Real::codegen() const {
  return c64(this->val);
}

Value* String::codegen() const {
  return Builder.CreateGlobalStringPtr(this->val);
}

Value* Nil::codegen() const {
  auto ptr = std::static_pointer_cast<PtrType>(this->type);
  auto subtype = ptr->get_subtype();
  return ConstantPointerNull::get(to_llvm_type(subtype)->getPointerTo());
}

Value* Variable::codegen() const {
  return codegen_table.lookup_var(this->name);
}

Value* Array::codegen() const {
  Value* arr = this->arr->codegen();
  Value* index = this->index->codegen();

  index = (index->getType()->isPointerTy()) ? Builder.CreateLoad(index) : index;

  PointerType* pt = dyn_cast<PointerType>(arr->getType());
  if (pt) {
    if (pt->getElementType()->isArrayTy()) {
      return Builder.CreateInBoundsGEP(arr, std::vector<Value*>{c32(0), index}, "array_gep");
    } else {
      return Builder.CreateInBoundsGEP(arr, index, "iarray_gep");
    }
  } else {
    return nullptr;
  }
}

Value* Deref::codegen() const {
  return Builder.CreateLoad(this->ptr->codegen());
}

// Allocate and return pointer to the variable so that
// when it's loaded we get the address of the variable
Value* AddressOf::codegen() const {
  Value* var = this->var->codegen();
  AllocaInst* ptr = Builder.CreateAlloca(var->getType()->getPointerTo(), nullptr, "pointer");
  Builder.CreateStore(var, ptr, false);
  return ptr;
}

// Helper function for the two call nodes
Value* call_codegen(const std::string& fun_name, const std::vector<expr_ptr>& call_parameters, const int& line) {
  auto fun_def = codegen_table.lookup_fun(fun_name);
  auto prev_scope_vars = fun_def->get_prev_scope_vars();
  auto fun_parameters = fun_def->get_parameters();
  auto callee_nesting_level = fun_def->get_nesting_level();
  auto is_lib_fun = fun_def->is_lib_fun();

  Function* F = fun_def->get_function();

  std::vector<Value*> ArgsV;

  if (!is_lib_fun) {
    Value* prev_frame = codegen_table.lookup_var("$frame");
    if (!prev_frame) {
      StructType* st = StructType::get(TheContext, std::vector<Type*>());
      prev_frame = Builder.CreateAlloca(st, nullptr, "prev_frame");
    }

    std::vector<Type*> types;
    Value* new_frame = nullptr;

    int current_depth = codegen_table.get_nesting_level();
    if (current_depth > callee_nesting_level) {
      // If callee is in a previous scope, we pop the scopes that are not visible to it
      int callee_nesting_difference = current_depth - callee_nesting_level;

      for (int i = 0; i < callee_nesting_difference; i++) {
        new_frame = Builder.CreateStructGEP(prev_frame, 0);
        new_frame = Builder.CreateLoad(prev_frame);
      }
    } else if (current_depth == callee_nesting_level) {
      // If callee is in the same depth we keep only the variables visible to the callee
      Function* this_function = Builder.GetInsertBlock()->getParent();
      std::string current_function = codegen_table.reverse_lookup_fun(this_function);

      if (current_function == fun_name) {
        new_frame = prev_frame;
      } else {
        Value* parent_frame = Builder.CreateStructGEP(prev_frame, 0);
        parent_frame = Builder.CreateLoad(parent_frame);

        types.push_back(parent_frame->getType());

        for (auto& var : prev_scope_vars) {
          if (var->get_nesting_level() == callee_nesting_level - 1) {
            Type* var_type = to_llvm_type(var->get_type());

            if (!var_type->isPointerTy())
              var_type = var_type->getPointerTo();

            types.push_back(var_type);
          }
        }

        StructType* st = StructType::get(TheContext, types);
        new_frame = Builder.CreateAlloca(st, nullptr, "new_frame");

        Value* first_pos = Builder.CreateStructGEP(new_frame, 0);
        Builder.CreateStore(parent_frame, first_pos);

        auto my_vars = codegen_table.lookup_fun(current_function)->get_prev_scope_vars();
        int i = 0;
        for (auto& my_var : my_vars) {
          int j = 0;
          for (auto& callee_var : prev_scope_vars) {
            if (callee_var->get_name() == my_var->get_name()) {
              Value* new_v = Builder.CreateStructGEP(new_frame, j + 1);
              Value* old_v = Builder.CreateStructGEP(prev_frame, i + 1);
              old_v = Builder.CreateLoad(old_v);

              Builder.CreateStore(old_v, new_v);
              break;
            }
            j++;
          }
          i++;
        }
      }
    } else {
      // If callee is in a deeper scope we send our local variables that are visible to it
      types.push_back(prev_frame->getType());

      for (auto& var : prev_scope_vars) {
        if (var->get_nesting_level() == callee_nesting_level - 1) {
          Type* var_type = to_llvm_type(var->get_type());

          if (!var_type->isPointerTy())
            var_type = var_type->getPointerTo();

          types.push_back(var_type);
        }
      }

      StructType* st = StructType::get(TheContext, types);
      new_frame = Builder.CreateAlloca(st, nullptr, "new_frame");
    
      Value* first_pos = Builder.CreateStructGEP(new_frame, 0);
      Builder.CreateStore(prev_frame, first_pos);

      int position = 1;
      for (auto& var : prev_scope_vars) {
        if (var->get_nesting_level() == calee_nesting_level - 1) {
          Value* current_position = Builder.CreateStructGEP(new_frame, position);
          Value* local_var = codegen_table.lookup_var(var->get_name());

          Builder.CreateStore(local_var, current_position);

          position++;
        }
      }
    }
    
    ArgsV.push_back(new_frame);
  }

  // Add the caller arguments right after the frame
  int call_param_count = call_parameters.size();

  for (int i = 0; i < call_param_count; i++) {
    bool pass_by_reference = fun_parameters[i];

    Value* v = call_parameters[i]->codegen();
    if (pass_by_reference) {
      if (!v->getType()->isPointerTy()) {
        std::cerr << "Line: " << line << "Error: Pass by reference requires an l-value" << std::endl;
        exit(1);
      }

      PointerType* pt = cast<PointerType>(v->getType());
      if (pt->getElementType()->isArrayTy())
        v = Builder.CreateInBoundsGEP(v, std::vector<Value*>{c32(0), c32(0)}, "array_gep");

      ArgsV.push_back(v);
    } else {
      v = (v->getType()->isPointerTy()) ? Builder.CreateLoad(v) : v;
      ArgsV.push_back(v);
    }
  }

  return Builder.CreateCall(F, ArgsV);
}

Value* CallExpr::codegen() const {
  return call_codegen(this->fun_name, this->parameters, this->get_line());
}

Value* Result::codegen() const {
  return codegen_table.lookup_var("result");
}

Value* BinaryExpr::codegen() const {
  Value* left = this->left->codegen();

  // AND,OR operations are shortcircuited and the right operand is
  // only evaluated if the result is not known from the left operand
  Value* right;
  if (this->op != BinOp::AND && this->op != BinOp::OR)
    right = this->right->codegen();

  // If pointers to value load the value
  left = (left->getType()->isPointerTy()) ? Builder.CreateLoad(left) : left;
  if (this->op != BinOp::AND && this->op != BinOp::OR)
    right = (right->getType()->isPointerTy()) ? Builder.CreateLoad(right) : right;

  auto left_type = this->left->get_type();
  auto right_type = this->right->get_type();

  if (left_type->is(BasicType::Integer)
      && right_type->is(BasicType::Real))
    left = Builder.CreateSIToFP(left, f64, "sitofp");

  if (left_type->is(BasicType::Real)
      && right_type->is(BasicType::Integer))
    right = Builder.CreateSIToFP(right, f64, "sitofp");

  switch(this->op) {
    case BinOp::PLUS:
      if (this->type->is(BasicType::Integer))
        return Builder.CreateAdd(left, right, "add_int");
      else
        return Builder.CreateFAdd(left, right, "add_real");

    case BinOp::MINUS:
      if (this->type->is(BasicType::Integer))
        return Builder.CreateSub(left, right, "sub_int");
      else
        return Builder.CreateFSub(left, right, "sub_real");

    case BinOp::MUL:
      if (this->type->is(BasicType::Integer))
        return Builder.CreateMul(left, right, "mul_int");
      else
        return Builder.CreateFMul(left, right, "mul_real");

    case BinOp::DIV:
      if (left_type->is(BasicType::Integer))
        left = Builder.CreateSIToFP(left, f64, "sitofp");

      if (right_type->is(BasicType::Integer))
        right = Builder.CreateSIToFP(right, f64, "sitofp");

      return Builder.CreateFDiv(left, right, "div_real");

    case BinOp::INT_DIV:
      return Builder.CreateSDiv(left, right, "div_int");

    case BinOp::MOD:
      return Builder.CreateSRem(left, right, "mod_int");

    case BinOp::EQ:
    {
      Value* cmp_res;
      if(left_type->is(BasicType::Real))
        cmp_res = Builder.CreateFCmpUEQ(left, right, "fcmp_eq");
      else
        cmp_res = Builder.CreateICmpEQ(left, right, "icmp_eq");

      return Builder.CreateZExt(cmp_res, i8);
    }

    case BinOp::NE:
    {
      Value* cmp_res;
      if(left_type->is(BasicType::Real))
        cmp_res = Builder.CreateFCmpUNE(left, right, "fcmp_ne");
      else
        cmp_res = Builder.CreateICmpNE(left, right, "icmp_ne");

      return Builder.CreateZExt(cmp_res, i8);
    }

    case BinOp::LT:
    {
      Value* cmp_res;
      if(left_type->is(BasicType::Real))
        cmp_res = Builder.CreateFCmpULT(left, right, "fcmp_lt");
      else
        cmp_res = Builder.CreateICmpSLT(left, right, "icmp_lt");

      return Builder.CreateZExt(cmp_res, i8);
    }

    case BinOp::GT:
    {
      Value* cmp_res;
      if(left_type->is(BasicType::Real))
        cmp_res = Builder.CreateFCmpUGT(left, right, "fcmp_gt");
      else
        cmp_res = Builder.CreateICmpSGT(left, right, "icmp_gt");

      return Builder.CreateZExt(cmp_res, i8);
    }

    case BinOp::LE:
    {
      Value* cmp_res;
      if(left_type->is(BasicType::Real))
        cmp_res = Builder.CreateFCmpULE(left, right, "fcmp_le");
      else
        cmp_res = Builder.CreateICmpSLE(left, right, "icmp_le");

      return Builder.CreateZExt(cmp_res, i8);
    }

    case BinOp::GE:
    {
      Value* cmp_res;
      if(left_type->is(BasicType::Real))
        cmp_res = Builder.CreateFCmpUGE(left, right, "fcmp_ge");
      else
        cmp_res = Builder.CreateICmpSGE(left, right, "icmp_ge");

      return Builder.CreateZExt(cmp_res, i8);
    }

    case BinOp::AND:
    {
      // And is shortcircuited so evaluate the first operand and if it's false
      // then skip evaluating the second one
      Value* res = Builder.CreateAlloca(i8, nullptr, "and_res");

      Value* cmp_res = Builder.CreateICmpEQ(left, c8(false), "icmp_eq");

      Function* TheFunction = Builder.GetInsertBlock()->getParent();

      BasicBlock* FalseBB = BasicBlock::Create(TheContext, "and_false", TheFunction);
      BasicBlock* ElseBB = BasicBlock::Create(TheContext, "and_right_operand");
      BasicBlock* AfterBB = BasicBlock::Create(TheContext, "after");

      Builder.CreateCondBr(cmp_res, FalseBB, ElseBB);

      Builder.SetInsertPoint(FalseBB);
      Builder.CreateStore(c8(false), res);
      Builder.CreateBr(AfterBB);

      TheFunction->getBasicBlockList().push_back(ElseBB);
      Builder.SetInsertPoint(ElseBB);

      right = this->right->codegen();
      right = (right->getType()->isPointerTy()) ? Builder.CreateLoad(right) : right;

      Value* right_operand = Builder.CreateICmpEQ(right, c8(true), "icmp_eq");
      right_operand = Builder.CreateZExt(right_operand, i8);

      Builder.CreateStore(right_operand, res);
      Builder.CreateBr(AfterBB);

      TheFunction->getBasicBlockList().push_back(AfterBB);
      Builder.SetInsertPoint(AfterBB);

      return res;
    }

    case BinOp::OR:
    {
      // Or is shortcircuited so evaluate the first operand and if it's true
      // then skip evaluating the second one
      Value* res = Builder.CreateAlloca(i8, nullptr, "or_res");

      Value* cmp_res = Builder.CreateICmpEQ(left, c8(true), "icmp_eq");

      Function* TheFunction = Builder.GetInsertBlock()->getParent();

      BasicBlock* TrueBB = BasicBlock::Create(TheContext, "or_true", TheFunction);
      BasicBlock* ElseBB = BasicBlock::Create(TheContext, "or_right_operand");
      BasicBlock* AfterBB = BasicBlock::Create(TheContext, "after");

      Builder.CreateCondBr(cmp_res, TrueBB, ElseBB);

      Builder.SetInsertPoint(TrueBB);
      Builder.CreateStore(c8(true), res);
      Builder.CreateBr(AfterBB);

      TheFunction->getBasicBlockList().push_back(ElseBB);
      Builder.SetInsertPoint(ElseBB);

      right = this->right->codegen();
      right = (right->getType()->isPointerTy()) ? Builder.CreateLoad(right) : right;

      Value* right_operand = Builder.CreateICmpEQ(right, c8(true), "icmp_eq");
      right_operand = Builder.CreateZExt(right_operand, i8);

      Builder.CreateStore(right_operand, res);
      Builder.CreateBr(AfterBB);

      TheFunction->getBasicBlockList().push_back(AfterBB);
      Builder.SetInsertPoint(AfterBB);

      return res;
    }

    default:
      return nullptr;
  }
}

Value* UnaryExpr::codegen() const {
  Value* operand = this->operand->codegen();
  operand = (operand->getType()->isPointerTy()) ? Builder.CreateLoad(operand) : operand;

  switch(this->op) {
    case UnOp::PLUS:
      return operand;

    case UnOp::MINUS:
      if (this->type->is(BasicType::Integer))
        return Builder.CreateNeg(operand, "neg");
      else
        return Builder.CreateFNeg(operand, "fneg");

    case UnOp::NOT:
      return Builder.CreateNot(operand, "neg");

    default:
      return nullptr;
  }
}

Value* Empty::codegen() const {
  return nullptr;
}

Value* Block::codegen() const {
  for (auto& stmt : stmt_list)
    stmt->codegen();

  return nullptr;
}

Value* VarNames::codegen() const {
  Type* type = to_llvm_type(this->type);
  for (auto& name : this->names) {
    AllocaInst* alloca = Builder.CreateAlloca(type, nullptr, name);

    codegen_table.insert_var(name, alloca);
  }

  return nullptr;
}

Value* VarDecl::codegen() const {
  for (auto& element : this->var_names)
    element->codegen();

  return nullptr;
}

Value* LabelDecl::codegen() const {
  return nullptr;
}

Value* Assign::codegen() const {
  Value* left = this->left->codegen();
  Value* right = this->right->codegen();

  right = (right->getType()->isPointerTy()) ? Builder.CreateLoad(right) : right;
  Builder.CreateStore(right, left);
  return nullptr;
}

Value* Goto::codegen() const {
  Builder.CreateBr(codegen_table.lookup_label(this->label));
  return nullptr;
}

Value* Label::codegen() const {
  Function* TheFunction = Builder.GetInsertBlock()->getParent();

  BasicBlock* LabelBB = BasicBlock::Create(TheContext, "label_" + this->label, TheFunction);
  Builder.SetInsertPoint(LabelBB);

  codegen_table.insert_label(this->label, LabelBB);

  this->stmt->codegen();

  return nullptr;
}

Value* If::codegen() const {
  Value* cond = this->cond->codegen();
  cond = (cond->getType()->isPointerTy()) ? Builder.CreateLoad(cond) : cond;
  Value* cmp_res = Builder.CreateICmpEQ(cond, c8(true), "if_cmp");

  Function* TheFunction = Builder.GetInsertBlock()->getParent();

  BasicBlock* ThenBB = BasicBlock::Create(TheContext, "then", TheFunction);
  BasicBlock* ElseBB = BasicBlock::Create(TheContext, "else");
  BasicBlock* AfterBB = BasicBlock::Create(TheContext, "after");

  Builder.CreateCondBr(cmp_res, ThenBB, ElseBB);

  Builder.SetInsertPoint(ThenBB);
  this->if_stmt->codegen();

  // If a terminator instruction was already generated we skip the branch instruction
  if (!Builder.GetInsertBlock()->getTerminator())
    Builder.CreateBr(AfterBB);

  TheFunction->getBasicBlockList().push_back(ElseBB);
  Builder.SetInsertPoint(ElseBB);
  if (this->else_stmt)
    this->else_stmt->codegen();

  // If a terminator instruction was already generated we skip the branch instruction
  if (!Builder.GetInsertBlock()->getTerminator())
    Builder.CreateBr(AfterBB);

  TheFunction->getBasicBlockList().push_back(AfterBB);
  Builder.SetInsertPoint(AfterBB);

  return nullptr;
}

Value* While::codegen() const {
  Function* TheFunction = Builder.GetInsertBlock()->getParent();

  BasicBlock* LoopBB = BasicBlock::Create(TheContext, "loop", TheFunction);
  BasicBlock* BodyBB = BasicBlock::Create(TheContext, "body");
  BasicBlock* AfterBB = BasicBlock::Create(TheContext, "after");

  Builder.CreateBr(LoopBB);
  Builder.SetInsertPoint(LoopBB);

  Value* cond = this->cond->codegen();
  cond = (cond->getType()->isPointerTy()) ? Builder.CreateLoad(cond) : cond;
  Value* cmp_res = Builder.CreateICmpEQ(cond, c8(true), "while_cmp");
  
  Builder.CreateCondBr(cmp_res, BodyBB, AfterBB);

  TheFunction->getBasicBlockList().push_back(BodyBB);
  Builder.SetInsertPoint(BodyBB);
  this->body->codegen();
  Builder.CreateBr(LoopBB);

  TheFunction->getBasicBlockList().push_back(AfterBB);
  Builder.SetInsertPoint(AfterBB);

  return nullptr;
}

Value* Formal::codegen() const {
  return nullptr;
}

Value* Body::codegen() const {
  for (auto& l : this->local_decls)
    l->codegen();

  this->block->codegen();

  return nullptr;
}

Value* Fun::codegen() const {
  BasicBlock* Parent = Builder.GetInsertBlock();

  // Create function only once
  if (!codegen_table.lookup_fun(this->fun_name)) {
    std::vector<Type*> args;

    // Collect previous scope variables and put them inside nested structs
    std::vector<std::vector<Type*>> scope_types(this->nesting_level - 1, std::vector<Type*>());

    for (auto& var : this->prev_scope_vars) {
      Type* var_type = to_llvm_type(var->get_type());
      if (!var_type->isPointerTy())
        var_type = var_type->getPointerTo();

      scope_types[var->get_nesting_level() - 1].push_back(var_type);
    }

    Type* current_st = StructType::get(TheContext)->getPointerTo();
    for (auto& scope : scope_types) {
      std::vector<Type*> types;
      types.push_back(current_st);
      types.insert(types.end(), scope.begin(), scope.end());
      current_st = StructType::get(TheContext, types)->getPointerTo();
    }

    args.push_back(current_st);

    std::vector<bool> parameters;

    for (auto& formal : this->formal_parameters) {
      for (auto& name : formal->get_names()) {
        Type* type = to_llvm_type(formal->get_type());

        if (formal->get_pass_by_reference())
          type = type->getPointerTo();

        args.push_back(type);
        parameters.push_back(formal->get_pass_by_reference());
      }
    }

    Type* ret_type = to_llvm_type(this->return_type);

    FunctionType* FT = FunctionType::get(ret_type, args, false);
    Function* F = Function::Create(FT, Function::PrivateLinkage, this->fun_name, TheModule.get());

    auto fun_def = std::make_shared<FunDef>(ret_type, parameters, F, this->prev_scope_vars, this->nesting_level);

    codegen_table.insert_fun(this->fun_name, fun_def);
  }

  // If not forward declaration generate code
  if (!this->forward_declaration) {
    codegen_table.open_scope();

    auto fun_def = codegen_table.lookup_fun(this->fun_name);
    Function* TheFunction = fun_def->get_function();

    BasicBlock* BB = BasicBlock::Create(TheContext, "entry", TheFunction);
    Builder.SetInsertPoint(BB);

    FunctionType* FT = TheFunction->getFunctionType();     

    // Retrieve arguments
    unsigned i = 1;
    for (auto& formal : this->formal_parameters) {
      for (auto& name : formal->get_names()) {
        Type* type = FT->getParamType(i);

        Value* alloca = Builder.CreateAlloca(type, nullptr, name);
        Builder.CreateStore(TheFunction->getArg(i), alloca);
        
        if (formal->get_pass_by_reference())
          alloca = Builder.CreateLoad(alloca);

        codegen_table.insert_var(name, alloca);
        
        i++;
      }
    }

    // Retrieve enclosing scope variables
    Value* next_frame = TheFunction->getArg(0);
    codegen_table.insert_var("$frame", next_frame);

    int current_depth = this->nesting_level - 1;
    int variable_position = 1;

    // Start loading variables from previous scopes starting from the innermost
    // scope and moving outwards
    for (auto& var : this->prev_scope_vars) {
      int nesting_level = var->get_nesting_level();

      // Nesting level change so we reset the variable position and move to the next frame
      if (nesting_level != current_depth) {
        next_frame = Builder.CreateStructGEP(next_frame, 0);
        next_frame = Builder.CreateLoad(next_frame);

        variable_position = 1;
      }

      current_depth = nesting_level;
      
      Type* type = to_llvm_type(var->get_type());

      // Ignore shadowed variables that appear in previous scopes but are hidden due to redeclaration
      if (!codegen_table.lookup_var(var->get_name())) {
        Value* v = Builder.CreateStructGEP(next_frame, variable_position);

        if (!type->isPointerTy())
          v = Builder.CreateLoad(v);

        codegen_table.insert_var(var->get_name(), v);
      }

      variable_position++;
    }

    Type* ret_type = FT->getReturnType();
    if (!ret_type->isVoidTy()) {
      AllocaInst* ret = Builder.CreateAlloca(ret_type, nullptr, "result");
      codegen_table.insert_var("result", ret);
    } else {
      codegen_table.insert_var("result", nullptr);
    }

    this->body->codegen();

    // If within procedure then result variable is equal to nullptr
    // else we return its value
    Value* result_addr = codegen_table.lookup_var("result");
    if (result_addr) {
      Value* result_val = Builder.CreateLoad(result_addr);
      Builder.CreateRet(result_val);
    } else {
      Builder.CreateRetVoid();
    }

    codegen_table.close_scope();
  }

  // Restore builder to parent
  Builder.SetInsertPoint(Parent);

  return nullptr;
}

Value* CallStmt::codegen() const {
  call_codegen(this->fun_name, this->parameters, this->get_line());

  return nullptr;
}

Value* Return::codegen() const {
  // If within procedure then result variable is equal to nullptr
  // else we return its value
  Value* result_addr = codegen_table.lookup_var("result");
  if (result_addr) {
    Value* result_val = Builder.CreateLoad(result_addr);
    Builder.CreateRet(result_val);
  } else {
    Builder.CreateRetVoid();
  }

  return nullptr;
}

Value* New::codegen() const {
  Value* malloc_size;
  std::vector<Value*> Args;

  Value* l_value = this->l_value->codegen();
 
  // We use a trick to calculate the element size. By creating a GEP instruction to the nil pointer
  // of the desired type at an offset of 1 we calculate the size of a single element and we cast it
  // to a 64 bit integer
  PointerType* pt = dyn_cast<PointerType>(l_value->getType());
  Value* nil = ConstantPointerNull::get(dyn_cast<PointerType>(pt->getElementType()));
  Value* element_size = Builder.CreateGEP(nil, c32(1));
  malloc_size = Builder.CreatePtrToInt(element_size, Type::getInt64Ty(TheContext));

  // If a size was provided we multiply the element size by the number of elements
  if (this->size) {
    Value* size = this->size->codegen();

    size = Builder.CreateSExt(size, Type::getInt64Ty(TheContext));

    malloc_size = Builder.CreateMul(size, malloc_size);
  }

  Args.push_back(malloc_size);
  Function* malloc = codegen_table.lookup_fun("malloc")->get_function();
  Value* ptr_to_memory = Builder.CreateCall(malloc, Args);

  // Bitcast the result from a pointer to i8 to our type
  ptr_to_memory = Builder.CreateBitCast(ptr_to_memory, pt->getElementType());

  Builder.CreateStore(ptr_to_memory, l_value);

  return nullptr;
}

Value* Dispose::codegen() const {
  std::vector<Value*> Args;

  Value* l_value = this->l_value->codegen();
  Value* ptr = Builder.CreateLoad(l_value);

  // Bitcast from our type to pointer to i8
  Value* ptr_i8 = Builder.CreateBitCast(ptr, i8->getPointerTo());

  Args.push_back(ptr_i8);
  Function* free = codegen_table.lookup_fun("free")->get_function();
  Builder.CreateCall(free, Args);

  // Store the nil pointer after the memory is freed
  Builder.CreateStore(ConstantPointerNull::get(dyn_cast<PointerType>(ptr->getType())), l_value);

  return nullptr;
}

Value* Program::codegen() const {
  init_module_and_pass_manager(this->optimize);

  FunctionType* FT = FunctionType::get(i32, false);
  Function* program = Function::Create(FT, Function::ExternalLinkage, "main", TheModule.get());

  BasicBlock* BB = BasicBlock::Create(TheContext, "entry", program);
  Builder.SetInsertPoint(BB);

  codegen_table.open_scope();

  codegen_library_functions();

  this->body->codegen();

  codegen_table.close_scope();

  Builder.CreateRet(c32(0));

  bool invalid = verifyModule(*TheModule, &errs());
  if (invalid) {
    std::cerr << "Invalid IR" << std::endl;
    exit(1);
  }
 
  // Optional optimization
  if (this->optimize)
    TheFPM->run(*program);

  std::string imm_name = this->file_name + ".imm";
  std::string asm_name = this->file_name + ".asm";
  if (this->asm_output) {
    // Assembly output to standard output
    std::error_code EC_IMM;
    raw_fd_ostream fd_os_imm(imm_name, EC_IMM);

    if (EC_IMM) {
      errs() << "Error opening output file: " << EC_IMM.message();
      exit(1);
    }

    TheModule->print(fd_os_imm, nullptr);

    std::string comm1 = "llc " + imm_name + " -o \"-\"";
    system(comm1.c_str());

    std::string comm2 = "rm " + imm_name;
    system(comm2.c_str());
  } else if (this->imm_output) {
    // LLVM IR to standard output
    TheModule->print(outs(), nullptr);
  } else {
    // LLVM IR and assembly output to files
    std::error_code EC_IMM;
    raw_fd_ostream fd_os_imm(imm_name, EC_IMM);

    if (EC_IMM) {
      errs() << "Error opening output file: " << EC_IMM.message();
      exit(1);
    }

    TheModule->print(fd_os_imm, nullptr);

    std::string comm1 = "llc " + imm_name + " -o " + asm_name;
    system(comm1.c_str());
    }

  return nullptr;
}
