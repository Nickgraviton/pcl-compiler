#ifndef __CODEGEN_TABLE_HPP__
#define __CODEGEN_TABLE_HPP__

#include <memory>
#include <string>
#include <vector>

namespace llvm {
  class Value;
  class BasicBlock;
}

class CodegenScope;
class FunDef;

class CodegenTable {
  std::vector<CodegenScope> scopes;

public:
  void open_scope();
  void close_scope();

  void insert_var(std::string name, llvm::Value* alloca);
  void insert_label(std::string name, llvm::BasicBlock* block);
  void insert_fun(std::string name, std::shared_ptr<FunDef> fun);

  llvm::Value* lookup_var(std::string name);
  llvm::BasicBlock* lookup_label(std::string name);
  std::shared_ptr<FunDef> lookup_fun(std::string name);
};

#endif
