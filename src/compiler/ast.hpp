#ifndef __AST_HPP__
#define __AST_HPP__

#include <vector>

#include <llvm/IR/Value.h>

#include "types.hpp"

class Node {
public:
  virtual ~Node() = default;
  virtual llvm::Value *codegen() = 0;
};

class Expr : public Node {};
using expr_ptr = std::unique_ptr<Expr>;

class Stmt : public Node {};
using stmt_ptr = std::unique_ptr<Stmt>;

//------------------------------------------------------------//
//--------------------Constant expressions--------------------//
//------------------------------------------------------------//

// Name: boolean
// Size: 1 byte
// Info: false(=0) and true(=1)
class Boolean : public Expr {
  bool val;

public:
  Boolean(bool val);

  llvm::Value *codegen() override;
};

// Name: char
// Size: 1 byte
// Info: ASCII representation
class Char : public Expr {
  char val;

public:
  Char(char val);

  llvm::Value *codegen() override;
};

// Name: integer
// Size: At least 2 bytes
// Info: Two's compliment representation
class Integer : public Expr {
  int val;

public:
  Integer(int val);

  llvm::Value *codegen() override;
};

// Name: real
// Size: 10 bytes
// Info: IEEE 754 representation
class Real : public Expr {
  double val;

public:
  Real(double val);

  llvm::Value *codegen() override;
};

// Type: array[n] of char
// Info: Null terminated string literal
class String : public Expr {
  std::string val;

public:
  String(std::string val);

  llvm::Value *codegen() override;
};

// Name: nil
// Type: ^t for any valid type t
// Info: Null pointer. Cannot be dereferenced
class Nil : public Expr {
public:
  Nil();

  llvm::Value *codegen() override;
};

//------------------------------------------------------------//
//--------------------Variable expressions--------------------//
//------------------------------------------------------------//

// Variable expression
class Variable : public Expr {
  std::string name;

public:
  Variable(std::string name);

  llvm::Value *codegen() override;
};

// Array expression
class Array : public Expr {
  expr_ptr arr, offset;

public:
  Array(expr_ptr arr, expr_ptr offset);

  llvm::Value *codegen() override;
};

// Dereference expression
class Deref : public Expr {
  expr_ptr ptr;

public:
  Deref(expr_ptr ptr);

  llvm::Value *codegen() override;
};

// Address of variable expression
class AddressOf : public Expr {
  expr_ptr var;

public:
  AddressOf(expr_ptr var);

  llvm::Value *codegen() override;
};

// Expr version of a call
class CallExpr : public Expr {
  std::string fun_name;
  std::vector<expr_ptr> parameters;

public:
  CallExpr(std::string fun_name, std::vector<expr_ptr> parameters);

  llvm::Value *codegen() override;
};

// Result variable for functions
class Result : public Expr {
public:
  Result();

  llvm::Value *codegen() override;
};

// Binary expression using arithmetic, comparison or logical operators
class BinaryExpr : public Expr {
  std::string op;
  expr_ptr left, right;

public:
  BinaryExpr(std::string op, expr_ptr left, expr_ptr right);

  llvm::Value *codegen() override;
};

// Unary operator one of: not, +, -
class UnaryOp : public Expr {
  std::string op;
  expr_ptr operand;

public:
  UnaryOp(std::string op, expr_ptr operand);

  llvm::Value *codegen() override;
};

//------------------------------------------------------------//
//-------------------------Statements-------------------------//
//------------------------------------------------------------//

// Superclass of local declarations
class Local : public Stmt {};
using local_ptr = std::unique_ptr<Local>;

// Empty statement
class Empty : public Stmt {
public:
  Empty();

  llvm::Value *codegen() override;
};

// Code block comprised of multiple instructions
class Block : public Stmt {
  std::vector<stmt_ptr> stmt_list;

public:
  Block(std::vector<stmt_ptr> stmt_list);

  llvm::Value *codegen() override;
};
using block_ptr = std::unique_ptr<Block>;

// Variable names of the same type
class VarNames : public Stmt {
  std::vector<std::string> names;
  typeinfo_ptr type;

public:
  VarNames(std::vector<std::string> names, typeinfo_ptr type);

  llvm::Value *codegen() override;
};
using varnames_ptr = std::unique_ptr<VarNames>;

// Variable declarations
class VarDecl : public Local {
  std::vector<varnames_ptr> var_names;

public:
  VarDecl(std::vector<varnames_ptr> var_names);

  llvm::Value *codegen() override;
};

// Label declaration
class LabelDecl : public Local {
  std::vector<std::string> names;

public:
  LabelDecl(std::vector<std::string> names);

  llvm::Value *codegen() override;
};

// Variable assignment statement
class VarAssign : public Stmt {
  expr_ptr left, right;

public:
  VarAssign(expr_ptr left, expr_ptr right);

  llvm::Value *codegen() override;
};

// Goto statement that jumps to label in the same block
class Goto : public Stmt {
  std::string label;

public:
  Goto(std::string label);

  llvm::Value *codegen() override;
};

// Label before a statement where we can goto
class Label : public Stmt {
  std::string label;
  stmt_ptr stmt;

public:
  Label(std::string label, stmt_ptr stmt);

  llvm::Value *codegen() override;
};

// If statement with an optional else clause
class If : public Stmt {
  expr_ptr cond;
  stmt_ptr if_stmt, else_stmt;

public:
  If(expr_ptr cond, stmt_ptr if_stmt, stmt_ptr else_stmt);

  llvm::Value *codegen() override;
};

// While loop
class While : public Stmt {
  expr_ptr cond;
  stmt_ptr stmt;

public:
  While(expr_ptr cond, stmt_ptr stmt);

  llvm::Value *codegen() override;
};

// Formal parameters for functions
// Objects contain names of variables that have the same type and pass by
// value/reference policy
class Formal : public Stmt {
  bool pass_by_reference;
  std::vector<std::string> names;
  typeinfo_ptr type;

public:
  Formal(bool pass_by_reference, std::vector<std::string> names, typeinfo_ptr type);

  llvm::Value *codegen() override;
};
using formal_ptr = std::unique_ptr<Formal>;

// Body of function or program containing declarations and a block of statements
class Body : public Stmt {
  std::vector<local_ptr> local_decls;
  block_ptr block;

public:
  Body(std::vector<local_ptr> local_decls, block_ptr block);

  llvm::Value *codegen() override;
};
using body_ptr = std::unique_ptr<Body>;

// Two types of functions: procedures and functions
// Procedures don't return a result
class Fun : public Local {
  std::string fun_name;
  typeinfo_ptr return_type;
  std::vector<formal_ptr> formal_parameters;

  body_ptr body;
  bool is_forward;

public:
  Fun(std::string fun_name, typeinfo_ptr return_type, std::vector<formal_ptr> formal_parameters);

  void set_body(body_ptr body);
  void set_forward(bool is_forward);

  llvm::Value *codegen() override;
};
using fun_ptr = std::unique_ptr<Fun>;

// Stmt version of a call
class CallStmt : public Stmt {
  std::string fun_name;
  std::vector<expr_ptr> parameters;

public:
  CallStmt(std::string fun_name, std::vector<expr_ptr> parameters);

  llvm::Value *codegen() override;
};

// Return statement
class Return : public Stmt {
public:
  Return();

  llvm::Value *codegen() override;
};

// Dynamic memory allocation
class New : public Stmt {
  expr_ptr size, l_value;

public:
  New(expr_ptr size, expr_ptr l_value);

  llvm::Value *codegen() override;
};

// Deallocation of dynamically allocated memory
class Dispose : public Stmt {
  bool has_brackets;
  expr_ptr l_value;

public:
  Dispose(bool has_brackets, expr_ptr l_value);

  llvm::Value *codegen() override;
};

// AST Root and initial program declaration
class Program : public Stmt {
  std::string name;
  body_ptr body;

public:
  Program(std::string name, body_ptr body);

  llvm::Value *codegen() override;
};
using program_ptr = std::unique_ptr<Program>;

#endif
