#include "Compiler.hpp"
#include "../Lexer/Lexer.hpp"
#include "../Parser/Ast/ConstBool.hpp"
#include "../Parser/Ast/ConstChar.hpp"
#include "../Parser/Ast/ConstFloat.hpp"
#include "../Parser/Ast/ConstInt.hpp"
#include "../Parser/Ast/ConstString.hpp"
#include "../Parser/Ast/ImportNode.hpp"
#include "../Parser/Ast/LoopNode.hpp"
#include "../Parser/Parser.hpp"
#include <fstream>
#include <iostream>
#include <llvm/ADT/APFloat.h>
#include <llvm/ADT/APInt.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/GlobalVariable.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/ValueSymbolTable.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/raw_ostream.h>
#include <set>
#include <unordered_map>

using namespace llvm;

llvm::Type *Compiler::getLLVMType(const std::string &typeName) {
  if (typeName == "i8")
    return Type::getInt8Ty(*context);
  if (typeName == "i16")
    return Type::getInt16Ty(*context);
  if (typeName == "i32")
    return Type::getInt32Ty(*context);
  if (typeName == "i64")
    return Type::getInt64Ty(*context);
  if (typeName == "i128")
    return Type::getInt128Ty(*context);
  if (typeName == "u8")
    return Type::getInt8Ty(*context);
  if (typeName == "u16")
    return Type::getInt16Ty(*context);
  if (typeName == "u32")
    return Type::getInt32Ty(*context);
  if (typeName == "u64")
    return Type::getInt64Ty(*context);
  if (typeName == "u128")
    return Type::getInt128Ty(*context);
  if (typeName == "f32")
    return Type::getFloatTy(*context);
  if (typeName == "f64")
    return Type::getDoubleTy(*context);
  if (typeName == "ch")
    return Type::getInt8Ty(*context);
  if (typeName == "str")
    return Type::getInt8Ty(*context)->getPointerTo();
  if (typeName == "bool")
    return Type::getInt1Ty(*context);
  return Type::getVoidTy(*context);
}

Compiler::Compiler() {
  context = std::make_unique<LLVMContext>();
  module = std::make_unique<Module>("prex_module", *context);
  builder = std::make_unique<IRBuilder<>>(*context);
}

Compiler::~Compiler() {}

static std::set<std::string> importedModules;

static std::string modulePathToFile(const std::string &modulePath) {
  std::string file = modulePath;
  for (auto &c : file)
    if (c == '.')
      c = '/';
  return file + ".prx";
}

void Compiler::loadAndCompileModule(const std::string &modulePath) {
  if (importedModules.count(modulePath))
    return;
  importedModules.insert(modulePath);
  std::string filePath = modulePathToFile(modulePath);
  std::ifstream in(filePath);
  if (!in.is_open()) {
    std::cerr << "Could not open module file: " << filePath << std::endl;
    std::exit(1);
  }
  std::string code((std::istreambuf_iterator<char>(in)),
                   std::istreambuf_iterator<char>());
  in.close();
  Lexer lexer(code, filePath);
  auto tokens = lexer.tokenize();
  Parser parser(tokens, code, filePath);
  RootNode *root = parser.parse();
  Compiler subCompiler;
  subCompiler.root = root;
  subCompiler.compile();
  // Merge subCompiler.module into this->module (TODO: lepsze scalanie, na razie
  // tylko funkcje)
  for (auto &f : subCompiler.module->functions()) {
    if (!module->getFunction(f.getName()))
      module->getFunctionList().push_back(&f);
  }
  for (auto &g : subCompiler.module->globals()) {
    if (!module->getGlobalVariable(g.getName())) {
      // Create a new global variable in the current module with the same
      // properties
      auto *newGlobal = new llvm::GlobalVariable(
          *module, g.getValueType(), g.isConstant(), g.getLinkage(),
          g.hasInitializer() ? g.getInitializer() : nullptr, g.getName(),
          nullptr, g.getThreadLocalMode(), g.getType()->getAddressSpace());
      newGlobal->setAlignment(g.getAlign());
    }
  }
}

void Compiler::declareLibcFunctions() {
  // printf
  if (!module->getFunction("printf")) {
    std::vector<llvm::Type *> args;
    args.push_back(llvm::Type::getInt8Ty(*context)->getPointerTo());
    auto ftype =
        llvm::FunctionType::get(llvm::Type::getInt32Ty(*context), args, true);
    llvm::Function::Create(ftype, llvm::Function::ExternalLinkage, "printf",
                           module.get());
  }
  // scanf
  if (!module->getFunction("scanf")) {
    std::vector<llvm::Type *> args;
    args.push_back(llvm::Type::getInt8Ty(*context)->getPointerTo());
    auto ftype =
        llvm::FunctionType::get(llvm::Type::getInt32Ty(*context), args, true);
    llvm::Function::Create(ftype, llvm::Function::ExternalLinkage, "scanf",
                           module.get());
  }
  // strcmp
  if (!module->getFunction("strcmp")) {
    std::vector<llvm::Type *> args(
        2, llvm::Type::getInt8Ty(*context)->getPointerTo());
    auto ftype =
        llvm::FunctionType::get(llvm::Type::getInt32Ty(*context), args, false);
    llvm::Function::Create(ftype, llvm::Function::ExternalLinkage, "strcmp",
                           module.get());
  }
  // strlen
  if (!module->getFunction("strlen")) {
    std::vector<llvm::Type *> args(
        1, llvm::Type::getInt8Ty(*context)->getPointerTo());
    auto ftype =
        llvm::FunctionType::get(llvm::Type::getInt64Ty(*context), args, false);
    llvm::Function::Create(ftype, llvm::Function::ExternalLinkage, "strlen",
                           module.get());
  }
  // malloc
  if (!module->getFunction("malloc")) {
    std::vector<llvm::Type *> args(1, llvm::Type::getInt64Ty(*context));
    auto ftype = llvm::FunctionType::get(
        llvm::Type::getInt8Ty(*context)->getPointerTo(), args, false);
    llvm::Function::Create(ftype, llvm::Function::ExternalLinkage, "malloc",
                           module.get());
  }
  // free
  if (!module->getFunction("free")) {
    std::vector<llvm::Type *> args(
        1, llvm::Type::getInt8Ty(*context)->getPointerTo());
    auto ftype =
        llvm::FunctionType::get(llvm::Type::getVoidTy(*context), args, false);
    llvm::Function::Create(ftype, llvm::Function::ExternalLinkage, "free",
                           module.get());
  }
  // memcpy
  if (!module->getFunction("memcpy")) {
    std::vector<llvm::Type *> args = {
        llvm::Type::getInt8Ty(*context)->getPointerTo(),
        llvm::Type::getInt8Ty(*context)->getPointerTo(),
        llvm::Type::getInt64Ty(*context)};
    auto ftype = llvm::FunctionType::get(
        llvm::Type::getInt8Ty(*context)->getPointerTo(), args, false);
    llvm::Function::Create(ftype, llvm::Function::ExternalLinkage, "memcpy",
                           module.get());
  }
  // memset
  if (!module->getFunction("memset")) {
    std::vector<llvm::Type *> args = {
        llvm::Type::getInt8Ty(*context)->getPointerTo(),
        llvm::Type::getInt32Ty(*context), llvm::Type::getInt64Ty(*context)};
    auto ftype = llvm::FunctionType::get(
        llvm::Type::getInt8Ty(*context)->getPointerTo(), args, false);
    llvm::Function::Create(ftype, llvm::Function::ExternalLinkage, "memset",
                           module.get());
  }
  // strcpy
  if (!module->getFunction("strcpy")) {
    std::vector<llvm::Type *> args = {
        llvm::Type::getInt8Ty(*context)->getPointerTo(),
        llvm::Type::getInt8Ty(*context)->getPointerTo()};
    auto ftype = llvm::FunctionType::get(
        llvm::Type::getInt8Ty(*context)->getPointerTo(), args, false);
    llvm::Function::Create(ftype, llvm::Function::ExternalLinkage, "strcpy",
                           module.get());
  }
  // strcat
  if (!module->getFunction("strcat")) {
    std::vector<llvm::Type *> args = {
        llvm::Type::getInt8Ty(*context)->getPointerTo(),
        llvm::Type::getInt8Ty(*context)->getPointerTo()};
    auto ftype = llvm::FunctionType::get(
        llvm::Type::getInt8Ty(*context)->getPointerTo(), args, false);
    llvm::Function::Create(ftype, llvm::Function::ExternalLinkage, "strcat",
                           module.get());
  }
  // atoi
  if (!module->getFunction("atoi")) {
    std::vector<llvm::Type *> args(
        1, llvm::Type::getInt8Ty(*context)->getPointerTo());
    auto ftype =
        llvm::FunctionType::get(llvm::Type::getInt32Ty(*context), args, false);
    llvm::Function::Create(ftype, llvm::Function::ExternalLinkage, "atoi",
                           module.get());
  }
  // atof
  if (!module->getFunction("atof")) {
    std::vector<llvm::Type *> args(
        1, llvm::Type::getInt8Ty(*context)->getPointerTo());
    auto ftype =
        llvm::FunctionType::get(llvm::Type::getDoubleTy(*context), args, false);
    llvm::Function::Create(ftype, llvm::Function::ExternalLinkage, "atof",
                           module.get());
  }
  // exit
  if (!module->getFunction("exit")) {
    std::vector<llvm::Type *> args(1, llvm::Type::getInt32Ty(*context));
    auto ftype =
        llvm::FunctionType::get(llvm::Type::getVoidTy(*context), args, false);
    llvm::Function::Create(ftype, llvm::Function::ExternalLinkage, "exit",
                           module.get());
  }
  // system
  if (!module->getFunction("system")) {
    std::vector<llvm::Type *> args(
        1, llvm::Type::getInt8Ty(*context)->getPointerTo());
    auto ftype =
        llvm::FunctionType::get(llvm::Type::getInt32Ty(*context), args, false);
    llvm::Function::Create(ftype, llvm::Function::ExternalLinkage, "system",
                           module.get());
  }
}

void Compiler::compile() {
  declareLibcFunctions();
  if (!root)
    return;
  for (auto node : root->nodes) {
    if (auto import = dynamic_cast<ImportNode *>(node)) {
      loadAndCompileModule(import->modulePath);
    } else if (auto def = dynamic_cast<DefunNode *>(node)) {
      codegenDefun(def);
    } else if (auto var = dynamic_cast<VarNode *>(node)) {
      codegenVar(var);
    } else if (auto assign = dynamic_cast<VarAssignNode *>(node)) {
      codegenVarAssign(assign);
    }
  }
}

void Compiler::printLlvm() { module->print(llvm::outs(), nullptr); }

void Compiler::writeLlvmToFile(const std::string &filename) {
  std::error_code EC;
  llvm::raw_fd_ostream out(filename, EC, llvm::sys::fs::OF_None);
  if (EC) {
    fprintf(stderr, "can't open file %s: %s\n", filename.c_str(),
            EC.message().c_str());
    return;
  }
  module->print(out, nullptr);
}

Function *Compiler::codegenDefun(DefunNode *def) {
  enterScope();
  std::vector<Type *> argTypes;
  for (auto &arg : def->args) {
    argTypes.push_back(getLLVMType(arg.type));
  }
  Type *retType = getLLVMType(def->ret_type);
  FunctionType *funcType = FunctionType::get(retType, argTypes, false);
  Function *function = Function::Create(funcType, Function::ExternalLinkage,
                                        def->name, module.get());
  BasicBlock *bb = BasicBlock::Create(*context, "entry", function);
  builder->SetInsertPoint(bb);
  // alloc arguments as local vars
  unsigned idx = 0;
  for (auto &arg : function->args()) {
    auto &argInfo = def->args[idx];
    arg.setName(argInfo.name);
    llvm::Type *llvmType = getLLVMType(argInfo.type);
    Value *alloca = builder->CreateAlloca(llvmType, nullptr, argInfo.name);
    builder->CreateStore(&arg, alloca);
    declareVar(argInfo.name, alloca);
    idx++;
  }
  if (def->body && !def->body->nodes.empty()) {
    for (auto node : def->body->nodes) {
      if (auto var = dynamic_cast<VarNode *>(node)) {
        codegenVar(var); // local vars
      } else if (auto assign = dynamic_cast<VarAssignNode *>(node)) {
        codegenVarAssign(assign);
      } else if (auto exprStmt = dynamic_cast<ExprNode *>(node)) {
        codegenExpr(exprStmt->value);
      } else if (auto ret = dynamic_cast<RetNode *>(node)) {
        if (ret->expr) {
          Value *retVal = codegenExpr(ret->expr->value);
          builder->CreateRet(retVal);
        }
      } else if (auto ifNode = dynamic_cast<IfNode *>(node)) {
        codegenIf(ifNode);
      } else if (auto loop = dynamic_cast<LoopNode *>(node)) {
        codegenLoop(loop);
      }
    }
  } else {
    if (!retType->isVoidTy())
      builder->CreateRet(Constant::getNullValue(retType));
    else
      builder->CreateRetVoid();
  }
  verifyFunction(*function);
  exitScope();
  return function;
}

Value *Compiler::codegenVar(VarNode *var) {
  llvm::Type *llvmType = getLLVMType(var->type);
  if (builder->GetInsertBlock() && builder->GetInsertBlock()->getParent()) {
    // local var
    Value *alloca = builder->CreateAlloca(llvmType, nullptr, var->name);
    if (var->value && var->value->value) {
      Value *init = codegenExpr(var->value->value);
      builder->CreateStore(init, alloca);
    }
    declareVar(var->name, alloca);
    return alloca;
  } else {

    GlobalVariable *gvar =
        new GlobalVariable(*module, llvmType, false,
                           GlobalValue::ExternalLinkage, nullptr, var->name);
    if (var->value && var->value->value) {
      Value *init = codegenExpr(var->value->value);
      if (auto c = dyn_cast<Constant>(init)) {
        gvar->setInitializer(c);
      }
    }
    return gvar;
  }
}

Value *Compiler::codegenExpr(Expression *expr) {
  if (auto exprNode = dynamic_cast<ExprNode *>(expr)) {
    if (exprNode->value)
      return codegenExpr(exprNode->value);
  }
  if (auto cint = dynamic_cast<ConstInt *>(expr)) {
    return ConstantInt::get(Type::getInt32Ty(*context), cint->getValue());
  }
  if (auto cfloat = dynamic_cast<ConstFloat *>(expr)) {
    return ConstantFP::get(Type::getDoubleTy(*context), cfloat->getValue());
  }
  if (auto cchar = dynamic_cast<ConstChar *>(expr)) {
    return ConstantInt::get(Type::getInt8Ty(*context), cchar->getValue());
  }
  if (auto cstr = dynamic_cast<ConstString *>(expr)) {
    return builder->CreateGlobalStringPtr(cstr->getValue());
  }
  if (auto cbool = dynamic_cast<ConstBool *>(expr)) {
    return ConstantInt::get(Type::getInt1Ty(*context), cbool->getValue());
  }
  if (auto id = dynamic_cast<ConstIdentifier *>(expr)) {
    // read local var
    Value *val = lookupVar(id->name);
    if (val) {
      llvm::Type *elemType = nullptr;
      if (auto allocaInst = llvm::dyn_cast<llvm::AllocaInst>(val))
        elemType = allocaInst->getAllocatedType();
      else
        elemType = val->getType();
      return builder->CreateLoad(elemType, val, id->name);
    }
    // read global var
    if (auto gvar = module->getGlobalVariable(id->name)) {
      return builder->CreateLoad(gvar->getValueType(), gvar, id->name);
    }
  }
  if (auto binop = dynamic_cast<BinOpNode *>(expr)) {
    if (binop->op == "=") {
      auto leftId = dynamic_cast<ConstIdentifier *>(binop->left->value);
      if (leftId) {
        Value *rhs = codegenExpr(binop->right->value);
        Value *lhsVal = lookupVar(leftId->name);
        if (lhsVal) {
          builder->CreateStore(rhs, lhsVal);
          return rhs;
        }
        if (auto gvar = module->getGlobalVariable(leftId->name)) {
          builder->CreateStore(rhs, gvar);
          return rhs;
        }
      }
    }
    Value *l = codegenExpr(binop->left->value);
    Value *r = codegenExpr(binop->right->value);
    // --- ADDED: string comparison via strcmp ---
    // Check if both arguments are strings (str)
    llvm::Type *lType = l->getType();
    llvm::Type *rType = r->getType();
    auto i8ptr = llvm::PointerType::getUnqual(llvm::Type::getInt8Ty(*context));
    bool isStr = lType == i8ptr && rType == i8ptr;
    if ((binop->op == "==" || binop->op == "!=") && isStr) {
      std::vector<llvm::Type *> strcmpArgs = {i8ptr, i8ptr};
      auto strcmpType = llvm::FunctionType::get(
          llvm::Type::getInt32Ty(*context), strcmpArgs, false);
      llvm::Function *strcmpFunc = module->getFunction("strcmp");
      if (!strcmpFunc) {
        strcmpFunc =
            llvm::Function::Create(strcmpType, llvm::Function::ExternalLinkage,
                                   "strcmp", module.get());
      }
      llvm::Value *cmp = builder->CreateCall(strcmpFunc, {l, r}, "strcmpcall");
      if (binop->op == "==")
        return builder->CreateICmpEQ(
            cmp, llvm::ConstantInt::get(llvm::Type::getInt32Ty(*context), 0),
            "eqstr");
      else
        return builder->CreateICmpNE(
            cmp, llvm::ConstantInt::get(llvm::Type::getInt32Ty(*context), 0),
            "nestr");
    }

    if (binop->op == "+")
      return builder->CreateAdd(l, r, "addtmp");
    if (binop->op == "-")
      return builder->CreateSub(l, r, "subtmp");
    if (binop->op == "*")
      return builder->CreateMul(l, r, "multmp");
    if (binop->op == "/")
      return builder->CreateSDiv(l, r, "divtmp");
    if (binop->op == "==")
      return builder->CreateICmpEQ(l, r, "eqtmp");
    if (binop->op == "!=")
      return builder->CreateICmpNE(l, r, "netmp");
    if (binop->op == "<")
      return builder->CreateICmpSLT(l, r, "lttmp");
    if (binop->op == ">")
      return builder->CreateICmpSGT(l, r, "gttmp");
    if (binop->op == "<=")
      return builder->CreateICmpSLE(l, r, "letmp");
    if (binop->op == ">=")
      return builder->CreateICmpSGE(l, r, "getmp");
    if (binop->op == "&&") {
      llvm::Function *function = builder->GetInsertBlock()->getParent();
      llvm::BasicBlock *lhsBB = builder->GetInsertBlock();
      llvm::BasicBlock *rhsBB =
          llvm::BasicBlock::Create(*context, "and.rhs", function);
      llvm::BasicBlock *mergeBB =
          llvm::BasicBlock::Create(*context, "and.cont", function);

      l = builder->CreateICmpNE(l, llvm::ConstantInt::get(l->getType(), 0),
                                "and.lbool");
      builder->CreateCondBr(l, rhsBB, mergeBB);

      // right operand
      builder->SetInsertPoint(rhsBB);
      llvm::Value *rVal = codegenExpr(binop->right->value);
      rVal = builder->CreateICmpNE(
          rVal, llvm::ConstantInt::get(rVal->getType(), 0), "and.rbool");
      llvm::BasicBlock *rhsEvalBB = builder->GetInsertBlock();
      builder->CreateBr(mergeBB);

      // Merge
      builder->SetInsertPoint(mergeBB);
      llvm::PHINode *phi =
          builder->CreatePHI(llvm::Type::getInt1Ty(*context), 2, "andtmp");
      phi->addIncoming(llvm::ConstantInt::getFalse(*context),
                       lhsBB);           // false if left false
      phi->addIncoming(rVal, rhsEvalBB); // value of right if left true
      return phi;
    }
    if (binop->op == "||") {
      llvm::Function *function = builder->GetInsertBlock()->getParent();
      llvm::BasicBlock *lhsBB = builder->GetInsertBlock();
      llvm::BasicBlock *rhsBB =
          llvm::BasicBlock::Create(*context, "or.rhs", function);
      llvm::BasicBlock *mergeBB =
          llvm::BasicBlock::Create(*context, "or.cont", function);

      l = builder->CreateICmpNE(l, llvm::ConstantInt::get(l->getType(), 0),
                                "or.lbool");
      builder->CreateCondBr(l, mergeBB, rhsBB);

      // right operand
      builder->SetInsertPoint(rhsBB);
      llvm::Value *rVal = codegenExpr(binop->right->value);
      rVal = builder->CreateICmpNE(
          rVal, llvm::ConstantInt::get(rVal->getType(), 0), "or.rbool");
      llvm::BasicBlock *rhsEvalBB = builder->GetInsertBlock();
      builder->CreateBr(mergeBB);

      // Merge
      builder->SetInsertPoint(mergeBB);
      llvm::PHINode *phi =
          builder->CreatePHI(llvm::Type::getInt1Ty(*context), 2, "ortmp");
      phi->addIncoming(llvm::ConstantInt::getTrue(*context),
                       lhsBB);           // true if left true
      phi->addIncoming(rVal, rhsEvalBB); // value of right if left false
      return phi;
    }
  }
  if (auto unop = dynamic_cast<UnaryOpNode *>(expr)) {
    std::cout << "[codegenExpr] UnaryOp: " << unop->op << std::endl;
    Value *val = codegenExpr(unop->expr->value);
    if (unop->op == "-")
      return builder->CreateNeg(val, "negtmp");
    if (unop->op == "+")
      return val;
    if (unop->op == "!")
      return builder->CreateNot(val, "nottmp");
    if (unop->op == "&") {
      // If operand is ConstIdentifier, return pointer
      if (auto id = dynamic_cast<ConstIdentifier *>(unop->expr->value)) {
        Value *val = lookupVar(id->name);
        if (val)
          return val;
        if (auto gvar = module->getGlobalVariable(id->name))
          return gvar;
      }
      return nullptr;
    }
    if (unop->op == "*") {
      llvm::Type *elemType = nullptr;
      if (auto ptrType = llvm::dyn_cast<llvm::PointerType>(val->getType()))
        elemType = builder->getInt8Ty(); // Use i8 as a generic pointer type
                                         // TODO: other types
      else
        elemType = val->getType();
      return builder->CreateLoad(elemType, val, "deref");
    }
  }
  if (auto call = dynamic_cast<FunctionCallNode *>(expr)) {
    return codegenFunctionCall(call);
  }
  return nullptr;
}

Value *Compiler::codegenFunctionCall(FunctionCallNode *call) {
  std::vector<Value *> argsV;
  for (auto arg : call->args) {
    argsV.push_back(codegenExpr(arg->value));
  }
  Function *calleeF = module->getFunction(call->name);
  if (!calleeF)
    return nullptr;
  return builder->CreateCall(calleeF, argsV);
}

void Compiler::codegenVarAssign(VarAssignNode *assign) {
  Value *lhsVal = lookupVar(assign->name);
  if (lhsVal) {
    Value *rhs = codegenExpr(assign->value->value);
    builder->CreateStore(rhs, lhsVal);
  } else if (auto gvar = module->getGlobalVariable(assign->name)) {
    Value *rhs = codegenExpr(assign->value->value);
    builder->CreateStore(rhs, gvar);
  }
}

void Compiler::codegenIf(IfNode *ifNode) {
  llvm::Value *condValue = codegenExpr(ifNode->condition->value);
  condValue = builder->CreateICmpNE(
      condValue, llvm::ConstantInt::get(condValue->getType(), 0), "ifcond");

  llvm::Function *function = builder->GetInsertBlock()->getParent();
  llvm::BasicBlock *thenBB =
      llvm::BasicBlock::Create(*context, "then", function);
  llvm::BasicBlock *elseBB = nullptr;
  llvm::BasicBlock *mergeBB =
      llvm::BasicBlock::Create(*context, "ifcont", function);

  if (ifNode->elseIf || ifNode->elseBody) {
    elseBB = llvm::BasicBlock::Create(*context, "else", function);
    builder->CreateCondBr(condValue, thenBB, elseBB);
  } else {
    builder->CreateCondBr(condValue, thenBB, mergeBB);
  }

  // Emit then block
  builder->SetInsertPoint(thenBB);
  enterScope();
  for (auto node : ifNode->body->nodes) {
    if (auto var = dynamic_cast<VarNode *>(node)) {
      codegenVar(var);
    } else if (auto assign = dynamic_cast<VarAssignNode *>(node)) {
      codegenVarAssign(assign);
    } else if (auto exprStmt = dynamic_cast<ExprNode *>(node)) {
      codegenExpr(exprStmt->value);
    } else if (auto ret = dynamic_cast<RetNode *>(node)) {
      if (ret->expr) {
        llvm::Value *retVal = codegenExpr(ret->expr->value);
        builder->CreateRet(retVal);
      }
    } else if (auto innerIf = dynamic_cast<IfNode *>(node)) {
      codegenIf(innerIf);
    } else if (auto loop = dynamic_cast<LoopNode *>(node)) {
      codegenLoop(loop);
    }
  }
  exitScope();
  if (!thenBB->getTerminator())
    builder->CreateBr(mergeBB);

  // Emit else/else if block
  if (elseBB) {
    builder->SetInsertPoint(elseBB);
    enterScope();
    if (ifNode->elseIf) {
      codegenIf(ifNode->elseIf);
      // After nested else if, if the block has no terminator, append br
      if (!builder->GetInsertBlock()->getTerminator())
        builder->CreateBr(mergeBB);
    } else if (ifNode->elseBody) {
      for (auto node : ifNode->elseBody->nodes) {
        if (auto var = dynamic_cast<VarNode *>(node)) {
          codegenVar(var);
        } else if (auto assign = dynamic_cast<VarAssignNode *>(node)) {
          codegenVarAssign(assign);
        } else if (auto exprStmt = dynamic_cast<ExprNode *>(node)) {
          codegenExpr(exprStmt->value);
        } else if (auto ret = dynamic_cast<RetNode *>(node)) {
          if (ret->expr) {
            llvm::Value *retVal = codegenExpr(ret->expr->value);
            builder->CreateRet(retVal);
          }
        } else if (auto innerIf = dynamic_cast<IfNode *>(node)) {
          codegenIf(innerIf);
        } else if (auto loop = dynamic_cast<LoopNode *>(node)) {
          codegenLoop(loop);
        }
      }

      if (!builder->GetInsertBlock()->getTerminator())
        builder->CreateBr(mergeBB);
    } else {
      if (!builder->GetInsertBlock()->getTerminator())
        builder->CreateBr(mergeBB);
    }
    exitScope();
  }

  builder->SetInsertPoint(mergeBB);
}

void Compiler::codegenLoop(LoopNode *loop) {
  llvm::Function *function = builder->GetInsertBlock()->getParent();
  llvm::BasicBlock *condBB =
      llvm::BasicBlock::Create(*context, "loop.cond", function);
  llvm::BasicBlock *bodyBB =
      llvm::BasicBlock::Create(*context, "loop.body", function);
  llvm::BasicBlock *afterBB =
      llvm::BasicBlock::Create(*context, "loop.after", function);

  builder->CreateBr(condBB);
  builder->SetInsertPoint(condBB);
  llvm::Value *condValue = codegenExpr(loop->condition->value);
  condValue = builder->CreateICmpNE(
      condValue, llvm::ConstantInt::get(condValue->getType(), 0), "loopcond");
  builder->CreateCondBr(condValue, bodyBB, afterBB);

  builder->SetInsertPoint(bodyBB);
  enterScope();
  for (auto node : loop->body->nodes) {
    if (auto var = dynamic_cast<VarNode *>(node)) {
      codegenVar(var);
    } else if (auto assign = dynamic_cast<VarAssignNode *>(node)) {
      codegenVarAssign(assign);
    } else if (auto exprStmt = dynamic_cast<ExprNode *>(node)) {
      codegenExpr(exprStmt->value);
    } else if (auto ret = dynamic_cast<RetNode *>(node)) {
      if (ret->expr) {
        llvm::Value *retVal = codegenExpr(ret->expr->value);
        builder->CreateRet(retVal);
      }
    } else if (auto innerIf = dynamic_cast<IfNode *>(node)) {
      codegenIf(innerIf);
    } else if (auto innerLoop = dynamic_cast<LoopNode *>(node)) {
      codegenLoop(innerLoop);
    }
  }
  exitScope();

  if (!builder->GetInsertBlock()->getTerminator()) {
    builder->CreateBr(condBB);
  }

  builder->SetInsertPoint(afterBB);
}

// --- Scope management methods ---
void Compiler::enterScope() { localsStack.push_back({}); }

void Compiler::exitScope() {
  if (!localsStack.empty())
    localsStack.pop_back();
}

void Compiler::declareVar(const std::string &name, llvm::Value *value) {
  if (!localsStack.empty())
    localsStack.back()[name] = value;
}

llvm::Value *Compiler::lookupVar(const std::string &name) {
  for (auto it = localsStack.rbegin(); it != localsStack.rend(); ++it) {
    auto found = it->find(name);
    if (found != it->end())
      return found->second;
  }
  return nullptr;
}
