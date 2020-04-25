#ifndef __AST_HPP__
#define __AST_HPP__

#include <llvm/IR/Value.h>

using namespace llvm;

extern int line_num;

class Node {
public:
    virtual ~Node() = default;
    virtual Value *codegen() = 0;
};

/***********************LANGUAGE TYPES***********************/

/* Name: integer
 * Size: At least 2 bytes
 * Info: Two's compliment representation
 */
class Integer: public Node {
    int val;
public:
    Value *codegen() override;
};

/* 
 * Name: boolean
 * Size: 1 byte
 * Info: false(=0) and true(=1)
 */
class Boolean: public Node {
    bool val;
public:
    Value *codegen() override;
};

/* 
 * Name: char
 * Size: 1 byte
 * Info: ASCII representation
 */
class Char: public Node {
    char val;
public:
    Value *codegen() override;
};

/* 
 * Name: real
 * Size: 10 bytes
 * Info: IEEE 754 representation
 */
class Real: public Node {
    double val;
public:
    Value *codegen() override;
};

/* 
 * Name: array[n] of t
 * Size: n * sizeof(t) bytes
 * Info: Increasing order of addresses
 */
class Array_n: public Node {
public:
    Value *codegen() override;
};

/* 
 * Name: array of t
 * Info: Unknown number of elements
 */
class Array: public Node {
public:
    Value *codegen() override;
};

/* 
 * Name: ^t
 * Size: 2 bytes
 * Info: Pointer to valid type t
 */
class Pointers: public Node {
public:
    Value *codegen() override;
};

class Var: public Node {
};

class BinOp: public Node {
};

class If: public Node {
};

class For: public Node {
};

class Block: public Node {
};

class Function: public Node {
};

class While: public Node {
};

#endif
