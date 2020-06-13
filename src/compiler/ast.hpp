#ifndef __AST_HPP__
#define __AST_HPP__

#include <iostream>
#include <vector>
#include <memory>

#include <llvm/IR/Value.h>

enum class UnOp;
enum class BinOp;

class TypeInfo;

class Node {
  int line;

public:
  Node();
  virtual ~Node() = default;

  int get_line();
  virtual void print(std::ostream& out, int level) const = 0;
  virtual void semantic() = 0;
  virtual llvm::Value* codegen() const = 0;
};

class Expr : public Node {
protected:
  std::shared_ptr<TypeInfo> type;

public:
  Expr();

  std::shared_ptr<TypeInfo> get_type();
};

class Stmt : public Node {
public:
  Stmt();
};

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

  void print(std::ostream& out, int level) const override;
  void semantic() override;
  llvm::Value* codegen() const override;
};

// Name: char
// Size: 1 byte
// Info: ASCII representation
class Char : public Expr {
  char val;

public:
  Char(char val);

  void print(std::ostream& out, int level) const override;
  void semantic() override;
  llvm::Value* codegen() const override;
};

// Name: integer
// Size: At least 2 bytes
// Info: Two's compliment representation
class Integer : public Expr {
  int val;

public:
  Integer(int val);

  void print(std::ostream& out, int level) const override;
  void semantic() override;
  llvm::Value* codegen() const override;
};

// Name: real
// Size: 10 bytes
// Info: IEEE 754 representation
class Real : public Expr {
  double val;

public:
  Real(double val);

  void print(std::ostream& out, int level) const override;
  void semantic() override;
  llvm::Value* codegen() const override;
};

// Type: array[n] of char
// Info: Null terminated string literal
class String : public Expr {
  std::string val;

public:
  String(std::string val);

  void print(std::ostream& out, int level) const override;
  void semantic() override;
  llvm::Value* codegen() const override;
};

// Name: nil
// Type: ^t for any valid type t
// Info: Null pointer. Cannot be dereferenced
class Nil : public Expr {
public:
  Nil();

  void print(std::ostream& out, int level) const override;
  void semantic() override;
  llvm::Value* codegen() const override;
};

//------------------------------------------------------------//
//--------------------Variable expressions--------------------//
//------------------------------------------------------------//

// Variable expression
class Variable : public Expr {
  std::string name;

public:
  Variable(std::string name);

  void print(std::ostream& out, int level) const override;
  void semantic() override;
  llvm::Value* codegen() const override;
};

// Array expression
class Array : public Expr {
  std::unique_ptr<Expr> arr, index;

public:
  Array(std::unique_ptr<Expr> arr, std::unique_ptr<Expr> index);

  void print(std::ostream& out, int level) const override;
  void semantic() override;
  llvm::Value* codegen() const override;
};

// Dereference expression
class Deref : public Expr {
  std::unique_ptr<Expr> var;

public:
  Deref(std::unique_ptr<Expr> var);

  void print(std::ostream& out, int level) const override;
  void semantic() override;
  llvm::Value* codegen() const override;
};

// Address of variable expression
class AddressOf : public Expr {
  std::unique_ptr<Expr> var;

public:
  AddressOf(std::unique_ptr<Expr> var);

  void print(std::ostream& out, int level) const override;
  void semantic() override;
  llvm::Value* codegen() const override;
};

// Expr version of a call
class CallExpr : public Expr {
  std::string fun_name;
  std::vector<std::unique_ptr<Expr>> parameters;

public:
  CallExpr(std::string fun_name, std::vector<std::unique_ptr<Expr>> parameters);

  void print(std::ostream& out, int level) const override;
  void semantic() override;
  llvm::Value* codegen() const override;
};

// Result variable for functions
class Result : public Expr {
public:
  Result();

  void print(std::ostream& out, int level) const override;
  void semantic() override;
  llvm::Value* codegen() const override;
};

// Binary expression using arithmetic, comparison or logical operators
class BinaryExpr : public Expr {
  BinOp op;
  std::unique_ptr<Expr> left, right;

public:
  BinaryExpr(BinOp op, std::unique_ptr<Expr> left, std::unique_ptr<Expr> right);

  void print(std::ostream& out, int level) const override;
  void semantic() override;
  llvm::Value* codegen() const override;
};

// Unary operator one of: not, +, -
class UnaryExpr : public Expr {
  UnOp op;
  std::unique_ptr<Expr> operand;

public:
  UnaryExpr(UnOp op, std::unique_ptr<Expr> operand);

  void print(std::ostream& out, int level) const override;
  void semantic() override;
  llvm::Value* codegen() const override;
};

//------------------------------------------------------------//
//-------------------------Statements-------------------------//
//------------------------------------------------------------//

// Superclass of local declarations
class Local : public Stmt {
public:
  Local();
};

// Empty statement
class Empty : public Stmt {
public:
  Empty();

  void print(std::ostream& out, int level) const override;
  void semantic() override;
  llvm::Value* codegen() const override;
};

// Code block comprised of multiple instructions
class Block : public Stmt {
  std::vector<std::unique_ptr<Stmt>> stmt_list;

public:
  Block(std::vector<std::unique_ptr<Stmt>> stmt_list);

  void print(std::ostream& out, int level) const override;
  void semantic() override;
  llvm::Value* codegen() const override;
};

// Variable names of the same type
class VarNames : public Stmt {
  std::vector<std::string> names;
  std::shared_ptr<TypeInfo> type;

public:
  VarNames(std::vector<std::string> names, std::shared_ptr<TypeInfo> type);

  void print(std::ostream& out, int level) const override;
  void semantic() override;
  llvm::Value* codegen() const override;
};

// Variable declarations
class VarDecl : public Local {
  std::vector<std::unique_ptr<VarNames>> var_names;

public:
  VarDecl(std::vector<std::unique_ptr<VarNames>> var_names);

  void print(std::ostream& out, int level) const override;
  void semantic() override;
  llvm::Value* codegen() const override;
};

// Label declaration
class LabelDecl : public Local {
  std::vector<std::string> names;

public:
  LabelDecl(std::vector<std::string> names);

  void print(std::ostream& out, int level) const override;
  void semantic() override;
  llvm::Value* codegen() const override;
};

// Variable assignment statement
class VarAssign : public Stmt {
  std::unique_ptr<Expr> left, right;

public:
  VarAssign(std::unique_ptr<Expr> left, std::unique_ptr<Expr> right);

  void print(std::ostream& out, int level) const override;
  void semantic() override;
  llvm::Value* codegen() const override;
};

// Goto statement that jumps to label in the same block
class Goto : public Stmt {
  std::string label;

public:
  Goto(std::string label);

  void print(std::ostream& out, int level) const override;
  void semantic() override;
  llvm::Value* codegen() const override;
};

// Label before a statement where we can goto
class Label : public Stmt {
  std::string label;
  std::unique_ptr<Stmt> stmt;

public:
  Label(std::string label, std::unique_ptr<Stmt> stmt);

  void print(std::ostream& out, int level) const override;
  void semantic() override;
  llvm::Value* codegen() const override;
};

// If statement with an optional else clause
class If : public Stmt {
  std::unique_ptr<Expr> cond;
  std::unique_ptr<Stmt> if_stmt, else_stmt;

public:
  If(std::unique_ptr<Expr> cond, std::unique_ptr<Stmt> if_stmt, std::unique_ptr<Stmt> else_stmt);

  void print(std::ostream& out, int level) const override;
  void semantic() override;
  llvm::Value* codegen() const override;
};

// While loop
class While : public Stmt {
  std::unique_ptr<Expr> cond;
  std::unique_ptr<Stmt> stmt;

public:
  While(std::unique_ptr<Expr> cond, std::unique_ptr<Stmt> stmt);

  void print(std::ostream& out, int level) const override;
  void semantic() override;
  llvm::Value* codegen() const override;
};

// Formal parameters for functions
// Objects contain names of variables that have the same type and pass by
// value/reference policy
class Formal : public Stmt {
  bool pass_by_reference;
  std::vector<std::string> names;
  std::shared_ptr<TypeInfo> type;

public:
  Formal(bool pass_by_reference, std::vector<std::string> names, std::shared_ptr<TypeInfo> type);

  bool get_pass_by_reference();
  std::vector<std::string>& get_names();
  std::shared_ptr<TypeInfo> get_type();

  void print(std::ostream& out, int level) const override;
  void semantic() override;
  llvm::Value* codegen() const override;
};

// Body of function or program containing declarations and a block of statements
class Body : public Stmt {
  std::vector<std::unique_ptr<Local>> local_decls;
  std::unique_ptr<Block> block;

public:
  Body(std::vector<std::unique_ptr<Local>> local_decls, std::unique_ptr<Block> block);

  void print(std::ostream& out, int level) const override;
  void semantic() override;
  llvm::Value* codegen() const override;
};

// Two types of functions: procedures and functions
// Procedures don't return a result
class Fun : public Local {
  std::string fun_name;
  std::shared_ptr<TypeInfo> return_type;
  std::vector<std::unique_ptr<Formal>> formal_parameters;

  std::unique_ptr<Body> body;
  bool forward_declaration;

public:
  Fun(std::string fun_name, std::shared_ptr<TypeInfo> return_type, std::vector<std::unique_ptr<Formal>> formal_parameters);

  void set_body(std::unique_ptr<Body> body);
  void set_forward(bool forward_declaration);

  void print(std::ostream& out, int level) const override;
  void semantic() override;
  llvm::Value* codegen() const override;
};

// Stmt version of a call
class CallStmt : public Stmt {
  std::string fun_name;
  std::vector<std::unique_ptr<Expr>> parameters;

public:
  CallStmt(std::string fun_name, std::vector<std::unique_ptr<Expr>> parameters);

  void print(std::ostream& out, int level) const override;
  void semantic() override;
  llvm::Value* codegen() const override;
};

// Return statement
class Return : public Stmt {
public:
  Return();

  void print(std::ostream& out, int level) const override;
  void semantic() override;
  llvm::Value* codegen() const override;
};

// Dynamic memory allocation
class New : public Stmt {
  std::unique_ptr<Expr> size, l_value;

public:
  New(std::unique_ptr<Expr> size, std::unique_ptr<Expr> l_value);

  void print(std::ostream& out, int level) const override;
  void semantic() override;
  llvm::Value* codegen() const override;
};

// Deallocation of dynamically allocated memory
class Dispose : public Stmt {
  bool has_brackets;
  std::unique_ptr<Expr> l_value;

public:
  Dispose(bool has_brackets, std::unique_ptr<Expr> l_value);

  void print(std::ostream& out, int level) const override;
  void semantic() override;
  llvm::Value* codegen() const override;
};

// AST Root and initial program declaration
class Program : public Stmt {
  std::string name;
  std::unique_ptr<Body> body;

public:
  Program(std::string name, std::unique_ptr<Body> body);

  void print(std::ostream& out, int level) const override;
  void semantic() override;
  llvm::Value* codegen() const override;
};

#endif
