#include <memory>

#include "types.hpp"

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
