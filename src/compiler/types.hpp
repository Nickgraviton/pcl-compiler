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
};
using typeinfo_ptr = std::unique_ptr<TypeInfo>;

class IntType : public TypeInfo {
public:
  IntType();
};

class RealType : public TypeInfo {
public:
  RealType();
};

class BoolType : public TypeInfo {
public:
  BoolType();
};

class CharType : public TypeInfo {
public:
  CharType();
};

class ComplArrayType : public TypeInfo {
  int size;
  typeinfo_ptr subtype;

public:
  ComplArrayType(int size, typeinfo_ptr subtype);
};

class IncomplArrayType : public TypeInfo {
  typeinfo_ptr subtype;

public:
  IncomplArrayType(typeinfo_ptr subtype);
};

class PointerType : public TypeInfo {
  typeinfo_ptr subtype;

public:
  PointerType(typeinfo_ptr subtype);
};

#endif
