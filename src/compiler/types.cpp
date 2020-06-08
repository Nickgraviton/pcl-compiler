#include <iostream>
#include <memory>

#include "types.hpp"

using type_ptr = std::shared_ptr<TypeInfo>;

//------------------------------------------------------------------//
//-------------------Constructors/Getters/Setters-------------------//
//------------------------------------------------------------------//

TypeInfo::TypeInfo(BasicType t, bool complete)
  : t(t), complete(complete) {}

BasicType TypeInfo::get_basic_type() {
  return this->t;
}

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

type_ptr ArrType::get_subtype() {
  return this->subtype;
}

IArrType::IArrType(type_ptr subtype)
  : TypeInfo(BasicType::IArray, false), size(0), subtype(subtype) {}

type_ptr IArrType::get_subtype() {
  return this->subtype;
}

void IArrType::set_size(int size) {
  this->size = size;
}

PtrType::PtrType(type_ptr subtype)
  : TypeInfo(BasicType::Pointer, true), subtype(subtype) {}

type_ptr PtrType::get_subtype() {
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

bool TypeInfo::is_complete() {
  return this->complete;
}

bool TypeInfo::is(BasicType t) {
  return this->t == t;
}

bool TypeInfo::same_type_as(type_ptr t) {
  if (this->t != t->get_basic_type()) {
    return false;
  } else if (this->t == BasicType::Array || this->t == BasicType::IArray) {
    return false;
  } else {
    return true;
  }
}

bool TypeInfo::assignable_to(type_ptr t) {
  return true;
  // NEEDS TO BE IMPLEMENTED
}
