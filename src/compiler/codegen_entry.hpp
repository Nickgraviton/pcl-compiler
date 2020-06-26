#ifndef __CODEGEN_ENTRY_HPP__
#define __CODEGEN_ENTRY_HPP__

#include <memory>
#include <utility>
#include <vector>

namespace llvm {
  class Function;
  class Type;
}

class FunDef {
  llvm::Type* return_type;
  std::vector<bool> parameters;
  llvm::Function* F;

public:
  FunDef(llvm::Type* return_type, std::vector<bool>& parameters, llvm::Function* F);

  llvm::Type* get_return_type();
  std::vector<bool>& get_parameters();
  llvm::Function* get_function();
};

#endif
