#include "code_visitor.h"
#include "ast.h"

InterpreterError::InterpreterError(const std::string &message) {
  message_ = message;
}
const char *InterpreterError::what() const noexcept { return message_.c_str(); }

CodeVisitor::CodeVisitor() {
  module_ = std::make_unique<llvm::Module>("my cool jit", context_);
}
llvm::Value *CodeVisitor::VisitVariable(VariableExprAST &ast) {
  llvm::Value *var = named_values_[ast.name_];
  if (!var)
    throw InterpreterError("IR code for: " + std::string(ast.name_) +
                           " not found");
  return var;
}
llvm::Value *CodeVisitor::VisitNumber(NumberExprAST &ast) {
  return llvm::ConstantFP::get(context_, llvm::APFloat(ast.val_));
}

llvm::Value *CodeVisitor::VisitBinary(BinaryExprAST &ast) {
  llvm::Value *lhs_val = ast.lhs_->GenerateCode(*this);
  llvm::Value *rhs_val = ast.lhs_->GenerateCode(*this);
  if (!lhs_val || !rhs_val)
    throw InterpreterError("Error parsing a binary expression");

  switch (ast.op_) {
  case tok_plus:
    return builder_.CreateFAdd(lhs_val, rhs_val, "addtmp");
  case tok_minus:
    return builder_.CreateFSub(lhs_val, rhs_val, "subtmp");
  case tok_multiply:
    return builder_.CreateFMul(lhs_val, rhs_val, "multmp");
  case tok_less:
    lhs_val = builder_.CreateFCmpULT(lhs_val, rhs_val, "cmptmp");
    // Convert bool 0/1 to double 0.0 or 1.0
    return builder_.CreateUIToFP(lhs_val, llvm::Type::getDoubleTy(context_),
                                 "booltmp");
  default:
    throw InterpreterError("Unknown operator of binary expression");
  }
}

llvm::Value *CodeVisitor::VisitCall(CallExprAST &ast) {
  // Look up the name in the global module table.
  llvm::Function *callee = module_->getFunction(ast.callee_);
  if (!callee)
    throw InterpreterError("Unknown function referenced");
  // If argument mismatch error.
  if (callee->arg_size() != ast.args_.size())
    throw InterpreterError("Incorrect # arguments passed");

  std::vector<llvm::Value *> arg_values;
  for (auto &arg : ast.args_) {
    arg_values.push_back(
        arg->GenerateCode(*this)); // generate code for the actual value of the
                                   // function parameters
    if (!arg_values.back())
      throw InterpreterError("Null value during translation of parameter");
  }
  return builder_.CreateCall(callee, arg_values, "calltmp");
}

llvm::Value *CodeVisitor::VisitFunction(FunctionAST &ast) {
  // First, check for an existing function and delete it
  llvm::Function *function = module_->getFunction(ast.proto_->name_);
  if (function) {
    function->replaceAllUsesWith(llvm::UndefValue::get(function->getType()));
    function->eraseFromParent();
  }

  function = static_cast<llvm::Function *>(ast.proto_->GenerateCode(*this));

  if (!function)
    throw InterpreterError("Null value function value translation");

  // Create a new basic block to start insertion into.
  llvm::BasicBlock *BB = llvm::BasicBlock::Create(context_, "entry", function);
  builder_.SetInsertPoint(BB);

  // Record the function arguments in the NamedValues map.
  named_values_.clear();
  for (auto &Arg : function->args())
    named_values_[std::string(Arg.getName())] =
        &Arg; // generate code for the function parameters

  if (llvm::Value *ret_val = ast.body_->GenerateCode(*this)) {
    // Finish off the function.
    builder_.CreateRet(ret_val);

    // Validate the generated code, checking for consistency.
    verifyFunction(*function);

    return function;
  }

  // Error reading body, remove function.
  function->eraseFromParent();
  throw InterpreterError("Error reading the body function");
}

llvm::Value *CodeVisitor::VisitPrototype(PrototypeAST &ast) {
  // Make the function type:  double(double,double) etc.
  std::vector<llvm::Type *> doubles(ast.args_.size(),
                                    llvm::Type::getDoubleTy(context_));
  llvm::FunctionType *function_template = llvm::FunctionType::get(
      llvm::Type::getDoubleTy(context_), doubles, false);

  llvm::Function *function =
      llvm::Function::Create(function_template, llvm::Function::ExternalLinkage,
                             ast.name_, module_.get());

  // Set names for all arguments.
  unsigned idx = 0;
  for (auto &Arg : function->args())
    Arg.setName(ast.args_[idx++]);

  return function;
}
