#include <iostream>
#include <memory>
#include <optional>

#include "types.hpp"

using type_ptr = std::shared_ptr<TypeInfo>;

//------------------------------------------------------------------//
//---------------------------Constructors---------------------------//
//------------------------------------------------------------------//

TypeInfo::TypeInfo(BasicType t, bool complete)
  : t(t), complete(complete) {}

IntType::IntType()
  : TypeInfo(BasicType::Integer, true) {}

RealType::RealType()
  : TypeInfo(BasicType::Real, true) {}

BoolType::BoolType()
  : TypeInfo(BasicType::Boolean, true) {}

CharType::CharType()
  : TypeInfo(BasicType::Char, true) {}

ArrType::ArrType(int size, type_ptr subtype)
  : TypeInfo(BasicType::Array, true), size(size), subtype(subtype) {}

std::shared_ptr<TypeInfo> ArrType::get_subtype() {
  return this->subtype;
}

IArrType::IArrType(type_ptr subtype)
  : TypeInfo(BasicType::IArray, false), size(std::nullopt), subtype(subtype) {}

std::shared_ptr<TypeInfo> IArrType::get_subtype() {
  return this->subtype;
}

PtrType::PtrType(type_ptr subtype)
  : TypeInfo(BasicType::Pointer, true), subtype(subtype) {}

std::shared_ptr<TypeInfo> PtrType::get_subtype() {
  return this->subtype;
}

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

void ArrType::print(std::ostream& out) const {
  out << "Array with size " << size << " of ";
  subtype->print(out);
}

void IArrType::print(std::ostream& out) const {
  out << "Array of ";
  subtype->print(out);
}

void PtrType::print(std::ostream& out) const {
  out << "Pointer to ";
  subtype->print(out);
}

//------------------------------------------------------------------//
//-----------------------------Misc---------------------------------//
//------------------------------------------------------------------//

bool TypeInfo::is(BasicType t) {
  return this->t == t;
}
