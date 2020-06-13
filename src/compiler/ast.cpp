#include <iostream>
#include <memory>
#include <optional>
#include <string>

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Transforms/InstCombine/InstCombine.h>
#include <llvm/Transforms/Scalar.h>
#include <llvm/Transforms/Scalar/GVN.h>
#include <llvm/Transforms/Utils.h>

#include "ast.hpp"
#include "lexer.hpp"
#include "scope.hpp"
#include "symbol_entry.hpp"
#include "symbol_table.hpp"
#include "types.hpp"

using namespace llvm;

static LLVMContext TheContext;
static IRBuilder<> Builder(TheContext);
static std::unique_ptr<Module> TheModule;
static std::unique_ptr<legacy::FunctionPassManager> TheFPM;
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
//--------------Constructors/Getters/Setters------------------------//
//------------------------------------------------------------------//

Node::Node() {
  this->line = line_num;
}

int Node::get_line() {
  return this->line;
}

Expr::Expr()
  : Node() {}

Stmt::Stmt()
  : Node() {}

type_ptr Expr::get_type() {
  return this->type;
}

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

Deref::Deref(expr_ptr var)
  : Expr(), var(std::move(var)) {}

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

VarAssign::VarAssign(expr_ptr left, expr_ptr right)
  : Stmt(), left(std::move(left)), right(std::move(right)) {}

Goto::Goto(std::string label)
  : Stmt(), label(label) {}

Label::Label(std::string label, stmt_ptr stmt)
  : Stmt(), label(label), stmt(std::move(stmt)) {}

If::If(expr_ptr cond, stmt_ptr if_stmt, stmt_ptr else_stmt)
  : Stmt(), cond(std::move(cond)), if_stmt(std::move(if_stmt)), else_stmt(std::move(else_stmt)) {}

While::While(expr_ptr cond, stmt_ptr stmt)
  : Stmt(), cond(std::move(cond)), stmt(std::move(stmt)) {}

Formal::Formal(bool pass_by_reference, std::vector<std::string> names, type_ptr type)
  : Stmt(), pass_by_reference(pass_by_reference), names(names), type(type) {}

bool Formal::get_pass_by_reference() {
  return this->pass_by_reference;
}

std::vector<std::string>& Formal::get_names() {
  return this->names;
}

type_ptr Formal::get_type() {
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
  : Stmt(), name(name), body(std::move(body)) {}

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
  out << ", formal_parameters, body, forward_declaration: " << this->forward_declaration << "):" << std::endl;
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

// Helper error function
void error(const std::string& msg, const int& line) {
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
  this->var->semantic();

  auto var_type = this->var->get_type();
  if (!var_type->is(BasicType::Pointer))
    error("Variable is not of pointer type", this->get_line());

  auto p_t = std::static_pointer_cast<PtrType>(var_type);
  this->type = p_t->get_subtype();
}

void AddressOf::semantic() {
  this->var->semantic();

  auto var_type = this->var->get_type();

  this->type = std::make_shared<PtrType>(var_type);
}

// Helper function for the two call nodes
type_ptr call_semantic(std::string fun_name, std::vector<expr_ptr>& parameters, int line) {
  auto entry = symbol_table.lookup(fun_name);
  if (!entry)
    error("Name of function " + fun_name + " not found", line);

  auto function_entry = std::dynamic_pointer_cast<FunctionEntry>(entry);
  if (!function_entry)
    error("Name \"" + fun_name + "\" has already been used and is not a function", line);

  for (auto& parameter : parameters)
    parameter->semantic();

  auto fun_parameters = function_entry->get_parameters();
  int fun_param_count = fun_parameters.size();
  int call_param_count = parameters.size();

  if (call_param_count < fun_param_count) {
    error("Not enough arguments provided for the call of function \"" + fun_name + "\"", line);
  } else if (call_param_count > fun_param_count) {
    error("Too many arguments provided for the call of function \"" + fun_name + "\"", line);
  } else {
    for (int i = 0; i < call_param_count; i++) {
      auto call_param_type = parameters[i]->get_type();
      auto fun_param_type = fun_parameters[i].get_type();
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
  auto result = symbol_table.current_scope_lookup("result");
  if (!result)
    error("\"result\" variable not used within the body of a function that returns a result", this->get_line());

  this->type = result->get_type();
}

void BinaryExpr::semantic() {
  this->left->semantic();
  this->right->semantic();

  auto left_type = this->left->get_type();
  auto right_type = this->right->get_type();

  switch(op) {
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

      if (!left_type->is(BasicType::Integer) && !right_type->is(BasicType::Integer))
        error(binop_to_string(this->op) + " operands need to be of integer type", this->get_line());

      this->type = std::make_shared<IntType>();

      break;

    case BinOp::EQ:
    case BinOp::NE:
      // Check if both operands are arithmetic types or of the same type and set the
      // expression's type to boolean

      if (((!left_type->is(BasicType::Integer) && !left_type->is(BasicType::Real))
            || (!right_type->is(BasicType::Integer) && !right_type->is(BasicType::Real)))
          && !same_type(left_type, right_type))
        error(binop_to_string(this->op) + " needs either arithmetic types or variables of the same type", this->get_line());

      this->type = std::make_shared<BoolType>();

      break;

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

      if (!left_type->is(BasicType::Boolean) && !right_type->is(BasicType::Boolean))
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

  switch(op) {
    case UnOp::PLUS:
    case UnOp::MINUS:
      if (!operand_type->is(BasicType::Integer) && !operand_type->is(BasicType::Real))
        error("Operand needs to be integer or real", this->get_line());

      break;

    case UnOp::NOT:
      if (!operand_type->is(BasicType::Boolean))
        error("Operand needs to be boolean", this->get_line());

      break;

    default:
      // Won't be reached
      error("Unkown binary operator", this->get_line());
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
    bool success = symbol_table.insert(name);
    if (!success)
      error("Label " + name + " has already been declared", this->get_line());
  }
}

void VarAssign::semantic() {
  this->right->semantic();
  this->left->semantic();

  auto right_type = this->right->get_type();
  auto left_type = this->left->get_type();
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

  this->stmt->semantic();
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
      fun_entry->add_parameter(FunctionParameter(formal->get_pass_by_reference(), formal->get_type()));
    }
  }

  bool success = symbol_table.insert(this->fun_name, fun_entry);
  if (!success)
    error(this->fun_name + " has already been declared and is not a function", this->get_line());

  // Open function's scope and insert the local variable's and the result variable if not a procedure
  
  if (!this->forward_declaration) {
    symbol_table.open_scope();

    for (auto& formal : this->formal_parameters) {
      for (auto& name : formal->get_names()) {
        symbol_table.insert(name, std::make_shared<VariableEntry>(formal->get_type()));
      }
    }

    if (this->return_type)
      symbol_table.insert("result", std::make_shared<VariableEntry>(this->return_type));

    this->body->semantic();

    symbol_table.close_scope();
  }
}

void CallStmt::semantic() {
  call_semantic(this->fun_name, this->parameters, this->get_line());
}

void Return::semantic() {}

void New::semantic() {
  size->semantic();
  l_value->semantic();

  auto l_value_type = l_value->get_type();
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
  l_value->semantic();

  auto l_value_type = l_value->get_type();
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

  this->body->semantic();

  symbol_table.close_scope();
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

Value* String::codegen() const {
  return nullptr;
}

Value* Nil::codegen() const {
  return nullptr;
}

Value* Variable::codegen() const {
  return nullptr;
}

Value* Array::codegen() const {
  return nullptr;
}

Value* Deref::codegen() const {
  return nullptr;
}

Value* AddressOf::codegen() const {
  return nullptr;
}

Value* CallExpr::codegen() const {
  return nullptr;
}

Value* Result::codegen() const {
  return nullptr;
}

Value* BinaryExpr::codegen() const {
  return nullptr;
}

Value* UnaryExpr::codegen() const {
  return nullptr;
}

Value* Empty::codegen() const {
  return nullptr;
}

Value* Block::codegen() const {
  return nullptr;
}

Value* VarNames::codegen() const {
  return nullptr;
}

Value* VarDecl::codegen() const {
  return nullptr;
}

Value* LabelDecl::codegen() const {
  return nullptr;
}

Value* VarAssign::codegen() const {
  return nullptr;
}

Value* Goto::codegen() const {
  return nullptr;
}

Value* Label::codegen() const {
  return nullptr;
}

Value* If::codegen() const {
  return nullptr;
}

Value* While::codegen() const {
  return nullptr;
}

Value* Formal::codegen() const {
  return nullptr;
}

Value* Body::codegen() const {
  return nullptr;
}

Value* Fun::codegen() const {
  return nullptr;
}

Value* CallStmt::codegen() const {
  return nullptr;
}

Value* Return::codegen() const {
  return nullptr;
}

Value* New::codegen() const {
  return nullptr;
}

Value* Dispose::codegen() const {
  return nullptr;
}

Value* Program::codegen() const {
  TheModule = std::make_unique<Module>("PCL program", TheContext);
  TheFPM = std::make_unique<legacy::FunctionPassManager>(TheModule.get());
  TheFPM->add(createPromoteMemoryToRegisterPass());
  TheFPM->add(createInstructionCombiningPass());
  TheFPM->add(createGVNPass());
  TheFPM->add(createCFGSimplificationPass());
  TheFPM->doInitialization();

  return nullptr;
}
