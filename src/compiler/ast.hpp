#ifndef __AST_HPP__
#define __AST_HPP__

#include <vector>

#include <llvm/IR/Value.h>

using namespace llvm;

extern int line_num;

class Node {
public:
  virtual ~Node() = default;
  virtual Value *codegen() = 0;
};

class Expr : public Node {};
class Stmt : public Node {};

typedef std::unique_ptr<Expr> expr_ptr;
typedef std::unique_ptr<Stmt> stmt_ptr;

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

  Value *codegen() override;
};

// Name: char
// Size: 1 byte
// Info: ASCII representation
class Char : public Expr {
  char val;

public:
  Char(char val);

  Value *codegen() override;
};

// Name: integer
// Size: At least 2 bytes
// Info: Two's compliment representation
class Integer : public Expr {
  int val;

public:
  Integer(int val);

  Value *codegen() override;
};

// Name: real
// Size: 10 bytes
// Info: IEEE 754 representation
class Real : public Expr {
  double val;

public:
  Real(double val);

  Value *codegen() override;
};

// Type: array[n] of char
// Info: Null terminated string literal
class String : public Expr {
  std::string val;

public:
  String(std::string val);

  Value *codegen() override;
};

// Name: nil
// Type: ^t for any valid type t
// Info: Null pointer. Cannot be dereferenced
class Nil : public Expr {
public:
  Nil();

  Value *codegen() override;
};

//------------------------------------------------------------//
//------------Variables, Unary and Binary operators-----------//
//------------------------------------------------------------//

// Variable expression
// -name: name of the variable
// -get_addr: boolean flag used with the @ operator signaling to
//            fetch the address of the variable
// -offset: determines which element of an array to fetch in expressions a[x]
class Variable : public Expr {
  std::string name;
  bool get_addr;
  int offset;

public:
  Variable(std::string name, bool get_addr, int offset);

  Value *codegen() override;
};

// Binary expression using arithmetic, comparison or logical operators
class BinaryExpr : public Expr {
  std::string op;
  expr_ptr left, right;

public:
  BinaryExpr(std::string op, expr_ptr left, expr_ptr right);

  Value *codegen() override;
};

// Unary operator one of: not, +, -
class UnaryOp : public Expr {
  std::string op;
  expr_ptr operand;

public:
  UnaryOp(std::string op, expr_ptr operand);

  Value *codegen() override;
};

//------------------------------------------------------------//
//-------------------------Statements-------------------------//
//------------------------------------------------------------//

// Superclass of local declarations
class Local : public Stmt {};

// Code block comprised of multiple instructions
class Block : public Stmt {
  std::vector<stmt_ptr> stmt_list;

public:
  Block(std::vector<stmt_ptr> stmt_list);

  Value *codegen() override;
};

// Variable names of the same type
class VarNames : public Stmt {
  std::vector<std::string> names;
  std::string type;

public:
  VarNames(std::vector<std::string> names, std::string type);

  Value *codegen() override;
};

// Variable declarations
class VarDecl : public Local {
  std::vector<std::unique_ptr<VarNames>> var_names;

public:
  VarDecl(std::vector<std::unique_ptr<VarNames>> var_names);

  Value *codegen() override;
};

// Label declaration
class LabelDecl : public Local {
  std::vector<std::string> names;

public:
  LabelDecl(std::vector<std::string> names);

  Value *codegen() override;
};

// Variable assignment statement
class VarAssign : public Stmt {
  expr_ptr left, right;

public:
  VarAssign(expr_ptr left, expr_ptr right);

  Value *codegen() override;
};

// Goto statement that jumps to label in the same block
class Goto : public Stmt {
  std::string label;

public:
  Goto(std::string label);

  Value *codegen() override;
};

// Label before a statement where we can goto
class Label : public Stmt {
  std::string label;
  stmt_ptr statement;

public:
  Label(std::string label, stmt_ptr statement);

  Value *codegen() override;
};

// If statement with an optional else clause
class If : public Stmt {
  expr_ptr cond;
  stmt_ptr if_statement, else_statement;

public:
  If(expr_ptr cond, stmt_ptr if_statement, stmt_ptr else_statement);

  Value *codegen() override;
};

// While loop
class While : public Stmt {
  expr_ptr cond;
  stmt_ptr statement;

public:
  While(expr_ptr cond, stmt_ptr statement);

  Value *codegen() override;
};

// Formal parameters for functions
// Objects contain names of variables that have the same type and pass by
// value/reference policy
class Formal : public Stmt {
  bool pass_by_reference;
  std::vector<std::string> names;
  std::string type;

public:
  Formal(bool pass_by_reference, std::vector<std::string> names,
         std::string type);

  Value *codegen() override;
};

// Body of function or program containing declarations and a block of statements
class Body : public Stmt {
  std::vector<std::unique_ptr<Local>> local_decls;
  std::unique_ptr<Block> block;

public:
  Body(std::vector<std::unique_ptr<Local>> local_decls,
       std::unique_ptr<Block> block);
};

// Two types of functions: procedures and functions
// Procedures don't return a result
class Fun : public Local {
  std::string fun_name, return_type;
  std::vector<std::unique_ptr<Formal>> formal_parameters;
  std::unique_ptr<Body> body;
  bool is_forward;

public:
  Fun(std::string fun_name, std::string return_type,
      std::vector<std::unique_ptr<Formal>> formal_parameters);

  void set_body(std::unique_ptr<Body> body);
  void set_forward(bool is_forward);

  Value *codegen() override;
};

// Function call
class Call : public Stmt {
  std::string fun_name;
  std::vector<expr_ptr> parameters;

public:
  Call(std::string fun_name, std::vector<expr_ptr> parameters);

  Value *codegen() override;
};

// Return statement
class Return : public Stmt {
public:
  Return();

  Value *codegen() override;
};

// Dynamic memory allocation
class New : public Stmt {
  int size;
  expr_ptr l_value;

public:
  New(int size, expr_ptr l_value);

  Value *codegen() override;
};

// Deallocation of dynamically allocated memory
class Dispose : public Stmt {
  bool has_brackets;
  expr_ptr l_value;

public:
  Dispose(bool has_brackets, expr_ptr l_value);

  Value *codegen() override;
};

// AST Root and initial program declaration
class Program : public Stmt {
  std::string name;
  std::unique_ptr<Body> body;

public:
  Program(std::string name, std::unique_ptr<Body> body);
};

#endif
