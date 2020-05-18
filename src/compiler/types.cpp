#include <iostream>
#include <memory>

#include "types.hpp"

using type_ptr = std::shared_ptr<Type>;

//------------------------------------------------------------------//
//---------------------------Constructors---------------------------//
//------------------------------------------------------------------//

Type::Type(BasicType t, bool complete)
    : t(t), complete(complete) {}

IntType::IntType()
    : Type(BasicType::Integer, true) {}

RealType::RealType()
    : Type(BasicType::Real, true) {}

BoolType::BoolType()
    : Type(BasicType::Boolean, true) {}

CharType::CharType()
    : Type(BasicType::Char, true) {}

ArrayType::ArrayType(int size, type_ptr subtype)
    : Type(BasicType::Array, true), size(size), subtype(std::move(subtype)) {}

IArrayType::IArrayType(type_ptr subtype)
    : Type(BasicType::Array, false), subtype(std::move(subtype)) {}

PointerType::PointerType(type_ptr subtype)
    : Type(BasicType::Pointer, true), subtype(std::move(subtype)) {}

//------------------------------------------------------------------//
//------------------------------Print-------------------------------//
//------------------------------------------------------------------//

void IntType::print(std::ostream& out) const {
  out << "Integer";
}

void RealType::print(std::ostream& out) const {
  out << "Real";
}

void BoolType::print(std::ostream& out) const {
  out << "Boolean";
}

void CharType::print(std::ostream& out) const {
  out << "Char";
}

void ArrayType::print(std::ostream& out) const {
  out << "Array with size " << size << " of ";
  subtype->print(out);
}

void IArrayType::print(std::ostream& out) const {
  out << "Array of ";
  subtype->print(out);
}

void PointerType::print(std::ostream& out) const {
  out << "Pointer to ";
  subtype->print(out);
}
