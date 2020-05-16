#ifndef KALEIDOSCOPE_KALEIDOSCOPE_KALEIDOSCOPE_SRC_IR_VISITOR_H
#define KALEIDOSCOPE_KALEIDOSCOPE_KALEIDOSCOPE_SRC_IR_VISITOR_H

#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Transforms/InstCombine/InstCombine.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Scalar/GVN.h"

class InterpreterError : public std::exception {
public:
  explicit InterpreterError(const std::string &);

  const char *what() const noexcept override;

public:
  std::string message_;
};

class NumberExprAST;
class CallExprAST;
class FunctionAST;
class VariableExprAST;
class BinaryExprAST;
class PrototypeAST;

class CodeVisitor {
public:
  llvm::LLVMContext context_;
  llvm::IRBuilder<> builder_ = llvm::IRBuilder<>(context_);
  std::unique_ptr<llvm::Module> module_;
  std::map<std::string, llvm::Value *> named_values_;

  CodeVisitor();
  llvm::Value *VisitNumber(NumberExprAST &);
  llvm::Value *VisitCall(CallExprAST &);
  llvm::Value *VisitFunction(FunctionAST &);
  llvm::Value *VisitVariable(VariableExprAST &);
  llvm::Value *VisitBinary(BinaryExprAST &);
  llvm::Value *VisitPrototype(PrototypeAST &);
};

#endif // KALEIDOSCOPE_KALEIDOSCOPE_KALEIDOSCOPE_SRC_IR_VISITOR_H
