#include <string>
#include <vector>

#include <llvm/IR/Instructions.h>

class CodegenMap {
  std::vector<std::map<std::string, llvm::AllocaInst*>> map;

public:
  void open_scope();
  void close_scope();

  void insert(std::string name, llvm::AllocaInst* alloca);
  llvm::AllocaInst* lookup(std::string name);
};
