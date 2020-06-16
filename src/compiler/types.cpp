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

int ArrType::get_size() {
  return this->size;
}

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

void PtrType::set_subtype(type_ptr subtype) {
  this->subtype = subtype;
}

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

std::string unop_to_string(UnOp op) {
  switch(op) {
    case UnOp::NOT:
      return "not";
    case UnOp::MINUS:
      return "-";
    case UnOp::PLUS:
      return "+";
  }
}

std::string binop_to_string(BinOp op) {
  switch(op) {
    case BinOp::PLUS:
      return "+";
    case BinOp::MINUS:
      return "-";
    case BinOp::MUL:
      return "*";
    case BinOp::DIV:
      return "/";
    case BinOp::INT_DIV:
      return "div";
    case BinOp::MOD:
      return "mod";
    case BinOp::OR:
      return "or";
    case BinOp::AND:
      return "and";
    case BinOp::EQ:
      return "=";
    case BinOp::NE:
      return "<>";
    case BinOp::LT:
      return "<";
    case BinOp::LE:
      return "<=";
    case BinOp::GT:
      return ">";
    case BinOp::GE:
      return ">=";
  }
}

bool TypeInfo::is_complete() {
  return this->complete;
}

bool TypeInfo::is(BasicType t) {
  return this->t == t;
}

// Function that checks if two types can be comared for (in)equality
bool same_type(type_ptr left, type_ptr right) {
  auto left_basic_type = left->get_basic_type();
  auto right_basic_type = right->get_basic_type();

  if (left_basic_type != right_basic_type) {
    return false;
  } else if (left_basic_type == BasicType::Array || left_basic_type == BasicType::IArray) {
    return false;
  } else {
    auto left_ptr_type = std::static_pointer_cast<PtrType>(left);
    auto right_ptr_type = std::static_pointer_cast<PtrType>(right);

    auto left_subtype = left_ptr_type->get_subtype();
    auto right_subtype = right_ptr_type->get_subtype();

    // Nil pointer can be of any type
    // Also assign a proper type to nil during the semantic pass
    if (!right_subtype) {
      right_ptr_type->set_subtype(left_subtype);
      return true;
    }

    if (!left_subtype) {
      left_ptr_type->set_subtype(right_subtype);
      return true;
    }

    return same_type(left_subtype, right_subtype);
  }
}

// Function that checks if two types are compatible for assignment
bool compatible_types(type_ptr left, type_ptr right) {
  auto left_basic_type = left->get_basic_type();
  auto right_basic_type = right->get_basic_type();

  if (left_basic_type != right_basic_type) {
    if (left_basic_type == BasicType::Real && right_basic_type == BasicType::Integer) {
      // Integer can be assigned to real

      return true;
    } else if (left_basic_type == BasicType::IArray && right_basic_type == BasicType::Array) {
      // Arrays can be assigned to incomplete arrays

      auto left_arr_type = std::static_pointer_cast<IArrType>(left);
      auto right_arr_type = std::static_pointer_cast<IArrType>(right);

      auto left_subtype = left_arr_type->get_subtype();
      auto right_subtype = right_arr_type->get_subtype();

      return compatible_types(left_subtype, right_subtype);
    } else {
      // Any two other different types cannot be assigned to each other

      return false;
    }
  } else if (left_basic_type == BasicType::Array) {
    auto left_arr_type = std::static_pointer_cast<ArrType>(left);
    auto right_arr_type = std::static_pointer_cast<ArrType>(right);

    auto left_subtype = left_arr_type->get_subtype();
    auto right_subtype = right_arr_type->get_subtype();

    return compatible_types(left_subtype, right_subtype);
  } else if (left_basic_type == BasicType::IArray) {
    auto left_iarr_type = std::static_pointer_cast<IArrType>(left);
    auto right_iarr_type = std::static_pointer_cast<IArrType>(right);

    auto left_subtype = left_iarr_type->get_subtype();
    auto right_subtype = right_iarr_type->get_subtype();

    return compatible_types(left_subtype, right_subtype);
  } else if (left_basic_type == BasicType::Pointer) {
    auto left_ptr_type = std::static_pointer_cast<PtrType>(left);
    auto right_ptr_type = std::static_pointer_cast<PtrType>(right);

    auto left_subtype = left_ptr_type->get_subtype();
    auto right_subtype = right_ptr_type->get_subtype();

    // Nil pointer can be assigned to any pointer
    // Also assign a proper type to nil during the semantic pass
    if (!right_subtype) {
      right_ptr_type->set_subtype(left_subtype);
      return true;
    }

    return compatible_types(left_subtype, right_subtype);
  } else {
    return true;
  }
}
