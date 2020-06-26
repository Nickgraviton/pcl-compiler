#ifndef __CODEGEN_SCOPE_HPP__
#define __CODEGEN_SCOPE_HPP__

#include <map>
#include <memory>
#include <string>

namespace llvm {
  class Value;
  class BasicBlock;
}

class FunDef;

class CodegenScope {
  std::map<std::string, llvm::Value*> var_map;
  std::map<std::string, llvm::BasicBlock*> label_map;
  std::map<std::string, std::shared_ptr<FunDef>> fun_map;

public:
  void insert_var(std::string name, llvm::Value* alloca);
  void insert_label(std::string name, llvm::BasicBlock* block);
  void insert_fun(std::string name, std::shared_ptr<FunDef> fun);

  llvm::Value* lookup_var(std::string name);
  llvm::BasicBlock* lookup_label(std::string name);
  std::shared_ptr<FunDef> lookup_fun(std::string name);
};

#endif
