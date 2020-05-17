#ifndef __TYPES_HPP__
#define __TYPES_HPP__

#include <iostream>
#include <memory>

enum class BasicType {
  Integer,
  Real,
  Boolean,
  Char,
  Array,
  Pointer
};

// Both `array [n] of t` and `array of t` types need a complete type t
// The only incomplete type is `array of t` without a specified size
class Type {
  BasicType t;
  bool complete;

public:
  Type(BasicType t, bool complete);
  virtual ~Type() = default;

  virtual void print(std::ostream& out) const = 0;
};
using type_ptr = std::unique_ptr<Type>;

class IntType : public Type {
public:
  IntType();

  void print(std::ostream& out) const override;
};

class RealType : public Type {
public:
  RealType();

  void print(std::ostream& out) const override;
};

class BoolType : public Type {
public:
  BoolType();

  void print(std::ostream& out) const override;
};

class CharType : public Type {
public:
  CharType();

  void print(std::ostream& out) const override;
};

class ArrayType : public Type {
  int size;
  type_ptr subtype;

public:
  ArrayType(int size, type_ptr subtype);

  void print(std::ostream& out) const override;
};

class IArrayType : public Type {
  type_ptr subtype;

public:
  IArrayType(type_ptr subtype);

  void print(std::ostream& out) const override;
};

class PointerType : public Type {
  type_ptr subtype;

public:
  PointerType(type_ptr subtype);

  void print(std::ostream& out) const override;
};

#endif
