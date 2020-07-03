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

// Function definition
// return_type: the return type of the function or nullptr if void
// parameters: a vector of bools that denotes whether each variable is passed by reference
// F: pointer to an llvm Function object
// prev_scope_vars: a vector that holds basic info about variables that are visible form previous scopes
// nesting_level: the nesting level of the function
// lib_fun: boool that denotes whether this is a library function or not
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

// Scope of the codegen table
// var_map: a hash map that correlates the name of a variable to its memory address in the stack that was returned by an alloca instruction
//          or that has the value of the variable itself it is a constant
// label_map: a hash map that correlates the name of a label to its basic block we can jump to
// fun_map: a hash map that correlates the name of a function to each definition
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

// Codegen table
// scopes: scopes are implemented by a vector. Each time we enter a deeper scope we push back a scope
//         and each time we exit one we pop it
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
