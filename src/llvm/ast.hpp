#ifndef __AST_HPP__
#define __AST_HPP__

class AST {
public:
    virtual ~AST() {}
    virtual Value *codegen() = 0;
};

class Expr:  public AST {};
class Stmt:  public AST {};

class Id:    public Expr {};
class Const: public Expr {};
class Binop: public Expr {};
class If:    public Expr {};
class For:   public Stmt {};
class Block: public Stmt {};

#endif
