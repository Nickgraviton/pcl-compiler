#ifndef __CODEGEN_TABLE_HPP__
#define __CODEGEN_TABLE_HPP__

#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace llvm {
  class BasicBlock;
  class Function;
  class Type;
  class Value;
}

class VarInfo;

class FunDef {
  llvm::Type* return_type;
  std::vector<bool> parameters;
  llvm::Function* F;
  std::vector<std::shared_ptr<VarInfo>> prev_scope_vars;
  int nesting_level;
  bool lib_fun;

public:
  FunDef(llvm::Type* return_type, std::vector<bool>& parameters, llvm::Function* F);
  FunDef(llvm::Type* return_type, std::vector<bool>& parameters, llvm::Function* F, std::vector<std::shared_ptr<VarInfo>> prev_scope_vars, int nesting_level);
  
  void set_prev_scope_vars(std::vector<std::shared_ptr<VarInfo>>& prev_scope_vars);

  llvm::Type* get_return_type();
  std::vector<bool>& get_parameters();
  llvm::Function* get_function();
  std::vector<std::shared_ptr<VarInfo>>& get_prev_scope_vars();
  int get_nesting_level();
  bool is_lib_fun();
};

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

  std::string reverse_lookup_fun(llvm::Function* F);
};

class CodegenTable {
  std::vector<CodegenScope> scopes;

public:
  int get_nesting_level();

  void open_scope();
  void close_scope();

  void insert_var(std::string name, llvm::Value* alloca);
  void insert_label(std::string name, llvm::BasicBlock* block);
  void insert_fun(std::string name, std::shared_ptr<FunDef> fun);

  llvm::Value* lookup_var(std::string name);
  llvm::BasicBlock* lookup_label(std::string name);
  std::shared_ptr<FunDef> lookup_fun(std::string name);

  std::string reverse_lookup_fun(llvm::Function* F);
};

#endif
