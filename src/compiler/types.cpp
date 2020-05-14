#include <iostream>
#include <memory>

#include "types.hpp"

//------------------------------------------------------------------//
//---------------------------Constructors---------------------------//
//------------------------------------------------------------------//

TypeInfo::TypeInfo(Type t, bool complete)
    : t(t), complete(complete) {}

IntType::IntType()
    : TypeInfo(Type::Integer, true) {}

RealType::RealType()
    : TypeInfo(Type::Real, true) {}

BoolType::BoolType()
    : TypeInfo(Type::Boolean, true) {}

CharType::CharType()
    : TypeInfo(Type::Char, true) {}

ComplArrayType::ComplArrayType(int size, typeinfo_ptr subtype)
    : TypeInfo(Type::Array, true), size(size), subtype(std::move(subtype)) {}

IncomplArrayType::IncomplArrayType(typeinfo_ptr subtype)
    : TypeInfo(Type::Array, false), subtype(std::move(subtype)) {}

PointerType::PointerType(typeinfo_ptr subtype)
    : TypeInfo(Type::Pointer, true), subtype(std::move(subtype)) {}

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

void ComplArrayType::print(std::ostream& out) const {
  out << "Array with size " << size << " of ";
  subtype->print(out);
}

void IncomplArrayType::print(std::ostream& out) const {
  out << "Array of ";
  subtype->print(out);
}

void PointerType::print(std::ostream& out) const {
  out << "Pointer to ";
  subtype->print(out);
}
