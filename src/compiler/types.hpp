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
  IArray,
  Pointer
};

// Both `array [n] of t` and `array of t` types need a complete type t
// The only incomplete type is `array of t` without a specified size
class TypeInfo {
  BasicType t;
  bool complete;

public:
  TypeInfo(BasicType t, bool complete);
  virtual ~TypeInfo() = default;

  BasicType get_basic_type();

  bool is_complete();
  bool is(BasicType t);
  bool same_type_as(std::shared_ptr<TypeInfo> t);
  bool assignable_to(std::shared_ptr<TypeInfo> t);

  virtual void print(std::ostream& out) const = 0;
};

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

// Complete array type with size n
class ArrType : public TypeInfo {
  int size;
  std::shared_ptr<TypeInfo> subtype;

public:
  ArrType(int size, std::shared_ptr<TypeInfo> subtype);

  std::shared_ptr<TypeInfo> get_subtype();
  void print(std::ostream& out) const override;
};

// Incomplete array type
class IArrType : public TypeInfo {
  int size;
  std::shared_ptr<TypeInfo> subtype;

public:
  IArrType(std::shared_ptr<TypeInfo> subtype);

  std::shared_ptr<TypeInfo> get_subtype();
  void set_size(int size);
  void print(std::ostream& out) const override;
};

// Pointer type
class PtrType : public TypeInfo {
  std::shared_ptr<TypeInfo> subtype;

public:
  PtrType(std::shared_ptr<TypeInfo> subtype);

  std::shared_ptr<TypeInfo> get_subtype();
  void print(std::ostream& out) const override;
};

#endif
