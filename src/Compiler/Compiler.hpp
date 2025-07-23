#pragma once
#include "../Parser/Ast/Arg.hpp"
#include "../Parser/Ast/BinOpNode.hpp"
#include "../Parser/Ast/BodyNode.hpp"
#include "../Parser/Ast/ConstChar.hpp"
#include "../Parser/Ast/ConstFloat.hpp"
#include "../Parser/Ast/ConstIdentifier.hpp"
#include "../Parser/Ast/ConstString.hpp"
#include "../Parser/Ast/DefunNode.hpp"
#include "../Parser/Ast/ExprNode.hpp"
#include "../Parser/Ast/FunctionCallNode.hpp"
#include "../Parser/Ast/IfNode.hpp"
#include "../Parser/Ast/LoopNode.hpp"
#include "../Parser/Ast/RetNode.hpp"
#include "../Parser/Ast/RootNode.hpp"
#include "../Parser/Ast/UnaryOpNode.hpp"
#include "../Parser/Ast/VarAssignNode.hpp"
#include "../Parser/Ast/VarNode.hpp"
#include "../Parser/Node.hpp"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/Verifier.h>
#include <memory>
#include <stack>
#include <string>
#include <unordered_map>
#include <vector>
class Compiler {
public:
  Compiler();
  ~Compiler();
  RootNode *root;
  void compile();
  void printLlvm();
  void writeLlvmToFile(const std::string &filename);
  void loadAndCompileModule(const std::string &modulePath);

  // Helper to declare libc functions in LLVM module
  void declareLibcFunctions();

  // LLVM context
  std::unique_ptr<llvm::LLVMContext> context;
  std::unique_ptr<llvm::Module> module;
  std::unique_ptr<llvm::IRBuilder<>> builder;

private:
  llvm::Function *codegenDefun(DefunNode *def);
  llvm::Value *codegenExpr(Expression *expr);
  llvm::Value *codegenVar(VarNode *var);
  void codegenVarAssign(VarAssignNode *assign);
  llvm::Value *codegenFunctionCall(FunctionCallNode *call);
  llvm::Type *getLLVMType(const std::string &typeName);

  // Stos map lokalnych zmiennych (nazwa -> alloca)
  std::vector<std::unordered_map<std::string, llvm::Value *>> localsStack;

  void enterScope();
  void exitScope();
  void declareVar(const std::string &name, llvm::Value *value);
  llvm::Value *lookupVar(const std::string &name);

  // Dodaj deklarację obsługi if
  void codegenIf(IfNode *ifNode);
  void codegenLoop(LoopNode *loop);
};