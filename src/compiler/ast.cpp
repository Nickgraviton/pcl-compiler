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
//--------------Constructors/Getters/Setters------------------------//
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

bool Formal::get_pass_by_reference() {
  return this->pass_by_reference;
}

std::vector<std::string>& Formal::get_names() {
  return this->names;
}

std::shared_ptr<TypeInfo> Formal::get_type() {
  return this->type;
}

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

void Variable::semantic() {
  auto entry = symbol_table.lookup(this->name);
  if (!entry)
    std::cerr << "Error: Identifier " << this->name << " hasn't been declared" << std::endl;

  auto variable_entry = std::dynamic_pointer_cast<VariableEntry>(entry);
  if (!variable_entry)
    std::cerr << "Error: Name \"" << this->name << "\" used in function definition" << std::endl;

  this->type = variable_entry->get_type();
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
  auto entry = symbol_table.lookup(this->fun_name);
  if (!entry)
    std::cerr << "Error: Name of function not found" << std::endl;

  auto function_entry = std::dynamic_pointer_cast<FunctionEntry>(entry);
  if (!function_entry)
    std::cerr << "Error: Name \"" << this->fun_name << "\" used in variable definition" << std::endl;

  this->type = function_entry->get_type();

  for (auto& parameter : this->parameters)
    parameter->semantic();

  auto fun_parameters = function_entry->get_parameters();
  int fun_param_count = fun_parameters.size();
  int call_param_count = this->parameters.size();

  if (call_param_count < fun_param_count) {
    std::cerr << "Error: Not enough arguments provided for the call of function \"" << this->fun_name << "\"" << std::endl;
  } else if (call_param_count > fun_param_count) {
    std::cerr << "Error: Too many arguments provided for the call of function \"" << this->fun_name << "\"" << std::endl;
  } else {
    for (int i = 0; i < call_param_count; i++) {
      auto call_param_type = this->parameters[i]->get_type();
      auto fun_param_type = fun_parameters[i].get_type();
      if (!call_param_type->assignable_to(fun_param_type))
        std::cerr << "Type of argument in function call does not match function definition" << std::endl;
    }
  }
}

void Result::semantic() {
  auto result = symbol_table.current_scope_lookup("result");
  if (!result)
    std::cerr << "Error: \"result\" variable not used within the body of a function that returns a result" << std::endl;

  this->type = result->get_type();
}

void BinaryExpr::semantic() {
  this->left->semantic();
  this->right->semantic();

  auto left_type = this->left->get_type();
  auto right_type = this->right->get_type();

  if (this->op == "+" || this->op == "-" || this->op == "*") {
    if (!left_type->is(BasicType::Integer) && !left_type->is(BasicType::Real))
      std::cerr << "Error: + operands need to be either of real or integer type";

    if (!right_type->is(BasicType::Integer) && !right_type->is(BasicType::Real))
      std::cerr << "Error: + operands need to be either of real or integer type";

    if (left_type->is(BasicType::Real) || right_type->is(BasicType::Real))
      this->type = std::make_shared<RealType>();
    else
      this->type = std::make_shared<IntType>();
  } else if (this->op == "/") {
     if (!left_type->is(BasicType::Integer) && !left_type->is(BasicType::Real))
      std::cerr << "Error: + operands need to be either of real or integer type";

    if (!right_type->is(BasicType::Integer) && !right_type->is(BasicType::Real))
      std::cerr << "Error: + operands need to be either of real or integer type";

    this->type = std::make_shared<RealType>();
  } else if (this->op == "div" || this->op == "mod") {
    if (!left_type->is(BasicType::Integer) && !right_type->is(BasicType::Integer))
      std::cerr << "Error: " << op << " operands need to be either of real or integer type";

    this->type = std::make_shared<IntType>();
  } else if (this->op == "=" || this->op == "<>") {
    if (((!left_type->is(BasicType::Integer) && !left_type->is(BasicType::Real))
          || (right_type->is(BasicType::Integer) && !right_type->is(BasicType::Real)))
        && !left_type->same_type_as(right_type))
      std::cerr << "Error: " << this->op << "needs either arithmetic types or variables of the same type" << std::endl;   

    this->type = std::make_shared<BoolType>();
  } else if (this->op == "<" || this->op == ">" || this->op == "<=" || this->op == ">=") {
    if ((!left_type->is(BasicType::Integer) && !left_type->is(BasicType::Real))
          || (right_type->is(BasicType::Integer) && !right_type->is(BasicType::Real)))
      std::cerr << "Error: " << this->op << " needs arithmetic types" << std::endl;

    this->type = std::make_shared<BoolType>();
  } else if (this->op == "and" || this->op == "or") {
    if (!left_type->is(BasicType::Boolean) && !right_type->is(BasicType::Boolean))
      std::cerr << "Error: " << this->op << "operands need to be of boolean type" << std::endl;
    
    this->type = std::make_shared<BoolType>();
  } else {
    std::cerr << "Error: Unkown operator " << this->op << std::endl;
  }
}

void UnaryOp::semantic() {
  auto operand_type = this->operand->get_type();

  if (this->op == "+" || this->op == "-")
    if (!operand_type->is(BasicType::Integer) && !operand_type->is(BasicType::Real))
      std::cerr << "Error: Operand needs to be integer or real" << std::endl;

  if (this->op == "not")
    if (!operand_type->is(BasicType::Boolean))
      std::cerr << "Error: Operand needs to be boolean" << std::endl;

  this->type = operand_type;
}

void Empty::semantic() {}

void Block::semantic() {
  for (auto& stmt : this->stmt_list)
    stmt->semantic();
}

void VarNames::semantic() {
  for (auto& name : this->names)
    symbol_table.insert(name, std::make_shared<VariableEntry>(this->type));
}

void VarDecl::semantic() {
  for (auto& element : this->var_names)
    element->semantic();
}

void LabelDecl::semantic() {
  for (auto& name : this->names)
    symbol_table.insert(name);
}

void VarAssign::semantic() {
  this->right->semantic();
  this->left->semantic();

  auto right_type = this->right->get_type();
  auto left_type = this->left->get_type();
  if (!right_type->assignable_to(left_type))
    std::cerr << "Error: Value cannot be assigned due to type mismatch" << std::endl;
}

void Goto::semantic() {
  if (!symbol_table.has_label(this->label))
    std::cerr << "Error: Label \"" << this->label << "\" hasn't been declared" << std::endl;
}

void Label::semantic() {
  if (!symbol_table.has_label(this->label))
    std::cerr << "Error: Label \"" << this->label << "\" hasn't been declared" << std::endl;

  this->stmt->semantic();
}

void If::semantic() {
  this->cond->semantic();

  auto condition_type = this->cond->get_type();
  if (!condition_type->is(BasicType::Boolean))
    std::cerr << "Condition of if statement is not a boolean expression" << std::endl;

  this->if_stmt->semantic();

  if (this->else_stmt)
    this->else_stmt->semantic();
}

void While::semantic() {
  this->cond->semantic();

  auto condition_type = this->cond->get_type();
  if (!condition_type->is(BasicType::Boolean))
    std::cerr << "Condition of while statement is not a boolean expression" << std::endl;

  this->stmt->semantic();
}

void Formal::semantic() {}

void Body::semantic() {
  for (auto& l : this->local_decls)
    l->semantic();

  this->block->semantic();
}

void Fun::semantic() {
  symbol_table.open_scope();

  auto fun_entry = std::make_shared<FunctionEntry>(this->return_type);

  for (auto& formal : this->formal_parameters) {
    for (auto& name : formal->get_names()) {
      fun_entry->add_parameter(FunctionParameter(formal->get_pass_by_reference(), formal->get_type()));
      symbol_table.insert(name, std::make_shared<VariableEntry>(formal->get_type()));
    }
  }

  symbol_table.insert(this->fun_name, fun_entry);
  symbol_table.insert("result", std::make_shared<VariableEntry>(this->return_type));

  if(!this->is_forward)
    this->body->semantic();

  symbol_table.close_scope();
}

void CallStmt::semantic() {
  auto entry = symbol_table.lookup(this->fun_name);
  if (!entry)
    std::cerr << "Error: Name of function not found" << std::endl;

  auto function_entry = std::dynamic_pointer_cast<FunctionEntry>(entry);
  if (!function_entry)
    std::cerr << "Error: Name \"" << this->fun_name << "\" used in variable definition" << std::endl;

  for (auto& parameter : this->parameters)
    parameter->semantic();

  auto fun_parameters = function_entry->get_parameters();
  int fun_param_count = fun_parameters.size();
  int call_param_count = this->parameters.size();

  if (call_param_count < fun_param_count) {
    std::cerr << "Error: Not enough arguments provided for the call of function \"" << this->fun_name << "\"" << std::endl;
  } else if (call_param_count > fun_param_count) {
    std::cerr << "Error: Too many arguments provided for the call of function \"" << this->fun_name << "\"" << std::endl;
  } else {
    for (int i = 0; i < call_param_count; i++) {
      auto call_param_type = this->parameters[i]->get_type();
      auto fun_param_type = fun_parameters[i].get_type();
      if (!call_param_type->assignable_to(fun_param_type))
        std::cerr << "Type of argument in function call does not match function definition" << std::endl;
    }
  }
}

void Return::semantic() {}

void New::semantic() {
  size->semantic();
  l_value->semantic();

  auto l_value_type = l_value->get_type();
  if(!l_value_type->is(BasicType::Pointer)) {
    std::cerr << "Error: Using new requires an l value of pointer type" << std::endl;
  } else {
    auto ptr_type = std::static_pointer_cast<PtrType>(l_value_type);
    auto subtype = ptr_type->get_subtype();

    if(this->size) {
      if (!subtype->is(BasicType::IArray))
        std::cerr << "Error: Using new with a size argument requires a pointer to array type" << std::endl;
      
      auto size_type = size->get_type();
      if (!size_type->is(BasicType::Integer))
        std::cerr << "Error: Expression within brackets needs to be of integer type" << std::endl;
    } else {
      if (!subtype->is_complete())
        std::cerr << "Error: Using new without a size argument requires a complete type" << std::endl;
    }
  }
}

void Dispose::semantic() {
  l_value->semantic();

  auto l_value_type = l_value->get_type();
  if(!l_value_type->is(BasicType::Pointer)) {
    std::cerr << "Error: Using delete requires an l value of pointer type" << std::endl;
  } else {
    auto ptr_type = std::static_pointer_cast<PtrType>(l_value_type);
    auto subtype = ptr_type->get_subtype();

    if(this->has_brackets) {
      if (!subtype->is(BasicType::IArray))
        std::cerr << "Error: Using new with a size argument requires a pointer to array type" << std::endl;
    } else {
      if (!subtype->is_complete())
        std::cerr << "Error: Using new without a size argument requires a complete type" << std::endl;
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
