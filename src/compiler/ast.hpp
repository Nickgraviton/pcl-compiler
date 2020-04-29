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

class Stmt: public Node {};
class Expr: public Node {};

//------------------------------------------------------------//
//--------------------Constant expressions--------------------//
//------------------------------------------------------------//

// Name: boolean
// Size: 1 byte
// Info: false(=0) and true(=1)
class Boolean: public Expr {
    bool val;

public:
    Boolean(bool val) : val(val) {}

    Value *codegen() override;
};

// Name: char
// Size: 1 byte
// Info: ASCII representation
class Char: public Expr {
    char val;

public:
    Char(char val) : val(val) {}

    Value *codegen() override;
};

// Name: integer
// Size: At least 2 bytes
// Info: Two's compliment representation
class Integer: public Expr {
    int val;

public:
    Integer(int val) : val(val) {}

    Value *codegen() override;
};

// Name: real
// Size: 10 bytes
// Info: IEEE 754 representation
class Real: public Expr {
    double val;

public:
    Real(double val) : val(val) {}

    Value *codegen() override;
};

// Type: array[n] of char
// Info: Null terminated string literal
class String: public Expr {
    std::string val;

public:
    String(std::string val) : val(val) {}

    Value *codegen() override;   
};

// Name: nil
// Type: ^t for any valid type t
// Info: Null pointer. Cannot be dereferenced
class Nil: public Expr {
public:
    Nil() {}

    Value *codegen() override;
};

//------------------------------------------------------------//
//--------------Variables and Binary expressions--------------//
//------------------------------------------------------------//

// Variable expression
class Variable: public Expr {
    std::string name;

public:
    Variable(std::string name) : name(name) {}

    Value *codegen() override;
};

// Binary expression using arithmetic, comparison or logical operators
class BinaryExpr: public Expr {
    std::string op;
    std::unique_ptr<Expr> left, right;

public:
    BinaryExpr(std::string op, std::unique_ptr<Expr> left,
            std::unique_ptr<Expr> right)
        : op(op), left(std::move(left)), right(std::move(right)) {}

    Value *codegen() override;
};

//------------------------------------------------------------//
//-------------------------Statements-------------------------//
//------------------------------------------------------------//

// Code block comprised of multiple instructions
class Block: public Stmt {
    std::vector<std::unique_ptr<Stmt>> stmt_list;

public:
    Value *codegen() override;
};

// Variable declaration
class VarDecl: public Stmt {
    std::string name, type;

public:
    VarDecl(std::string name, std::string type) : name(name), type(type) {}

    Value *codegen() override;
};

// Variable assignment statement
class VarAssign: public Stmt {
    std::unique_ptr<Expr> left, right;
    
public:
    VarAssign(std::unique_ptr<Expr> left, std::unique_ptr<Expr> right)
        : left(std::move(left)), right(std::move(right)) {}

    Value *codegen() override;
};

// Goto statement that jumps to label in the same block
class Goto: public Stmt {
    std::string label;

public:
    Goto(std::string label) : label(label) {}

    Value *codegen() override;
};

// If statement with an optional else clause
class If: public Stmt {
public:
    Value *codegen() override;
};

// For loop
class For: public Stmt {
public:
    Value *codegen() override;
};

// While loop
class While: public Stmt {
public:
    Value *codegen() override;
};

// Two types of functions: procedures and functions
// Procedures don't return a result
class Fun: public Stmt {
public:
    Value *codegen() override;
};

// Function call
class Call: public Stmt {
public:
    Value *codegen() override;
};

// Return statement
class Return: public Stmt {
public:
    Value *codegen() override;
};

// Dynamic memory allocation
class New: public Stmt {
public:
    Value *codegen() override;
};

// Deallocation of dynamically allocated memory
class Dispose: public Stmt {
public:
    Value *codegen() override;
};

#endif
