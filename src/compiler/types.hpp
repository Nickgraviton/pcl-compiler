#ifndef __TYPES_HPP__
#define __TYPES_HPP__

enum class Type {
  Integer,
  Real,
  Boolean,
  Char,
  Array,
  Pointer
};

// Both `array [n] of t` and `array of t` types need a complete type t
// The only incomplete type is `array of t` without a specified size
class TypeInfo {
  Type t;
  bool complete;

public:
  TypeInfo(Type t, bool complete);
  virtual ~TypeInfo() = default;

  virtual void print(std::ostream& out) const = 0;
};
using typeinfo_ptr = std::unique_ptr<TypeInfo>;

class IntType : public TypeInfo {
public:
  IntType();

  void print(std::ostream& out) const override;
};

class RealType : public TypeInfo {
public:
  RealType();

  void print(std::ostream& out) const override;
};

class BoolType : public TypeInfo {
public:
  BoolType();

  void print(std::ostream& out) const override;
};

class CharType : public TypeInfo {
public:
  CharType();

  void print(std::ostream& out) const override;
};

class ComplArrayType : public TypeInfo {
  int size;
  typeinfo_ptr subtype;

public:
  ComplArrayType(int size, typeinfo_ptr subtype);

  void print(std::ostream& out) const override;
};

class IncomplArrayType : public TypeInfo {
  typeinfo_ptr subtype;

public:
  IncomplArrayType(typeinfo_ptr subtype);

  void print(std::ostream& out) const override;
};

class PointerType : public TypeInfo {
  typeinfo_ptr subtype;

public:
  PointerType(typeinfo_ptr subtype);

  void print(std::ostream& out) const override;
};

#endif
