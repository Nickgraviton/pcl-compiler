#include <iostream>
#include <memory>
#include <optional>
#include <string>

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Value.h>

#include "ast.hpp"
#include "scope.hpp"
#include "symbol_entry.hpp"
#include "symbol_table.hpp"
#include "types.hpp"

using namespace llvm;

static LLVMContext TheContext;
static IRBuilder<> Builder(TheContext);
static std::unique_ptr<Module> TheModule;
static std::map<std::string, Value*> NamedValues;
static SymbolTable symbol_table;

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

//------------------------------------------------------------------//
//---------------------------Constructors---------------------------//
//------------------------------------------------------------------//

std::shared_ptr<TypeInfo> Expr::get_type() {
  return this->type;
}

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

Array::Array(expr_ptr arr, expr_ptr index)
  : arr(std::move(arr)), index(std::move(index)) {}

Deref::Deref(expr_ptr var)
  : var(std::move(var)) {}

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

VarNames::VarNames(std::vector<std::string> names, type_ptr type)
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

Formal::Formal(bool pass_by_reference, std::vector<std::string> names, type_ptr type)
  : pass_by_reference(pass_by_reference), names(names), type(type) {}

Body::Body(std::vector<local_ptr> local_decls, block_ptr block)
  : local_decls(std::move(local_decls)), block(std::move(block)) {}

Fun::Fun(std::string fun_name, type_ptr return_type, std::vector<formal_ptr> formal_parameters)
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
//------------------------------Print-------------------------------//
//------------------------------------------------------------------//

// Helper function that prints indent levels for the AST
void print_level(std::ostream& out, int level) {
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
  out << "Deref(var):" << std::endl;
  this->var->print(out, level + 1);
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
  out << "BinaryExpr(op: " << this->op << ", left, right):" << std::endl;
  this->left->print(out, level + 1);
  this->right->print(out, level + 1);
}

void UnaryOp::print(std::ostream& out, int level) const {
  print_level(out, level);
  out << "UnaryOp(op: " << this->op << ", operand):" << std::endl;
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

void VarAssign::print(std::ostream& out, int level) const {
  print_level(out, level);
  out << "VarAssign(left, right):" << std::endl;
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
  out << "While(cond, stmt):" << std::endl;
  this->cond->print(out, level + 1);
  this->stmt->print(out, level + 1);
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
  }
  out << ", formal_parameters, body, is_forward: " << this->is_forward << "):" << std::endl;
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

//------------------------------------------------------------------//
//----------------------------Semantic------------------------------//
//------------------------------------------------------------------//

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

// Helper function
std::optional<Entry> lookup(std::string name) {
  auto result = symbol_table.lookup(name);
  if (!result)
    std::cerr << "Error: Identifier " << name << " hasn't been declared" << std::endl;
  return result;
}

void Variable::semantic() {
  auto result = lookup(this->name);
 //ERROR this->type = result.value().get_type();
}

void Array::semantic() {
  this->arr->semantic();
  auto arr_type = this->arr->get_type();
  if (arr_type->is(BasicType::Array)) {
    std::shared_ptr<ArrType> a_t = std::static_pointer_cast<ArrType>(arr_type);
    this->type = a_t->get_subtype();
  } else if (arr_type->is(BasicType::IArray)) {
    std::shared_ptr<IArrType> ia_t = std::static_pointer_cast<IArrType>(arr_type);
    this->type = ia_t->get_subtype();
  } else {
    std::cerr << "Variable is not of array type" << std::endl;
  }

  this->index->semantic();
  auto index_type = this->index->get_type();
  if (!index_type->is(BasicType::Integer))
    std::cerr << "Array index is not of integer type" << std::endl;
}

void Deref::semantic() {
  this->var->semantic();
  auto var_type = this->var->get_type();
  if (var_type->is(BasicType::Pointer)) {
    std::shared_ptr<PtrType> p_t = std::static_pointer_cast<PtrType>(var_type);
    this->type = p_t->get_subtype();
  } else {
    std::cerr << "Variable is not of pointer type" << std::endl;
  }
}

void AddressOf::semantic() {
  this->var->semantic();
  auto var_type = this->var->get_type();
  this->type = std::make_shared<PtrType>(var_type);
}

void CallExpr::semantic() {
  // Lookup function and set its return type as expr type
  auto result = lookup(this->fun_name);
  //ERRORthis->type = result.value().get_type();

  //check if all the parameters have the correct type compared to the function definition
}

void Result::semantic() {

  //check if inside a function and not a procedure
  //set type as the function's return type
}

void BinaryExpr::semantic() {
  //check and possibly cast integers to real for arithmetic operations
  //check if valid types for mod and div
  //check if valid types for bool
}

void UnaryOp::semantic() {
  //check if not has bool
  //check if plus and minus have arithmetic types and make result same type
}

void Empty::semantic() {}

void Block::semantic() {
  for (auto& s : stmt_list)
    s->semantic();
}

void VarNames::semantic() {
  //loop names vector and entries to symbol table
}

void VarDecl::semantic() {
  //loop var_names declarations and call semantic on each of them
}

void LabelDecl::semantic() {
  //loop names vector andd labels to symbol table
}

void VarAssign::semantic() {
  //check if assignment is possible
}

void Goto::semantic() {
  //check if label has been delcared
}

void Label::semantic() {
  //check if label has been declared
  //call semantic on statement
}

void If::semantic() {
  //check if condition is bool
  //call semantic on if and on else if it exsists
}

void While::semantic() {
  //check if condition is bool
  //call semantic on block
}

void Formal::semantic() {

}

void Body::semantic() {
  symbol_table.open_scope();

  for (auto& l : local_decls)
    l->semantic();
  block->semantic();

  symbol_table.close_scope();
}

void Fun::semantic() {}

void CallStmt::semantic() {
//check if all the parameters have the correct type compared to the function definition

}

void Return::semantic() {}

void New::semantic() {}

void Dispose::semantic() {}

void Program::semantic() {
  //call semantic on class variables
}

//------------------------------------------------------------------//
//-----------------------------Codegen------------------------------//
//------------------------------------------------------------------//

Value* Boolean::codegen() const {
  return ConstantInt::get(TheContext, APInt(8, val, true));
}

Value* Char::codegen() const {
  return ConstantInt::get(TheContext, APInt(8, val, true));
}

Value* Integer::codegen() const {
  return ConstantInt::get(TheContext, APInt(16, val, true));
}

Value* Real::codegen() const {
  return ConstantFP::get(TheContext, APFloat(val));
}

Value* String::codegen() const {}

Value* Nil::codegen() const {}

Value* Variable::codegen() const {}

Value* Array::codegen() const {}

Value* Deref::codegen() const {}

Value* AddressOf::codegen() const {}

Value* CallExpr::codegen() const {}

Value* Result::codegen() const {}

Value* BinaryExpr::codegen() const {}

Value* UnaryOp::codegen() const {}

Value* Empty::codegen() const {}

Value* Block::codegen() const {}

Value* VarNames::codegen() const {}

Value* VarDecl::codegen() const {}

Value* LabelDecl::codegen() const {}

Value* VarAssign::codegen() const {}

Value* Goto::codegen() const {}

Value* Label::codegen() const {}

Value* If::codegen() const {}

Value* While::codegen() const {}

Value* Formal::codegen() const {}

Value* Body::codegen() const {}

Value* Fun::codegen() const {}

Value* CallStmt::codegen() const {}

Value* Return::codegen() const {}

Value* New::codegen() const {}

Value* Dispose::codegen() const {}

Value* Program::codegen() const {}
