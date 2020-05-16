#include "ast.h"
#include <utility>

/// ExprAST - Base class for all expression nodes.
ExprAST::~ExprAST() = default;

/// NumberExprAST - Expression class for numeric literals like "1.0".
NumberExprAST::NumberExprAST(double val) { val_ = val; }
bool NumberExprAST::Equals(const std::unique_ptr<ExprAST> &expression) const {
  auto number_expr = dynamic_cast<NumberExprAST *>(expression.get());
  if (number_expr == nullptr)
    return false;
  return val_ == number_expr->val_;
}
llvm::Value *NumberExprAST::GenerateCode(CodeVisitor &visitor) const {
  return visitor.VisitNumber(const_cast<NumberExprAST &>(*this));
}

/// VariableExprAST - Expression class for referencing a variable, like "a".
VariableExprAST::VariableExprAST(const std::string &name) { name_ = name; }
bool VariableExprAST::Equals(const std::unique_ptr<ExprAST> &expression) const {
  auto variable_expr = dynamic_cast<VariableExprAST *>(expression.get());
  if (variable_expr == nullptr)
    return false;
  return name_ == variable_expr->name_;
}
llvm::Value *VariableExprAST::GenerateCode(CodeVisitor &visitor) const {
  return visitor.VisitVariable(const_cast<VariableExprAST &>(*this));
}

/// BinaryExprAST - Expression class for a binary operator.
BinaryExprAST::BinaryExprAST(Token op, std::unique_ptr<ExprAST> lhs,
                             std::unique_ptr<ExprAST> rhs) {
  op_ = op;
  lhs_ = std::move(lhs);
  rhs_ = std::move(rhs);
}
bool BinaryExprAST::Equals(const std::unique_ptr<ExprAST> &expression) const {
  auto bin_expr = dynamic_cast<BinaryExprAST *>(expression.get());
  if (bin_expr == nullptr)
    return false;

  return bin_expr->op_ == op_ && bin_expr->lhs_->Equals(lhs_) &&
         bin_expr->rhs_->Equals(rhs_);
}
llvm::Value *BinaryExprAST::GenerateCode(CodeVisitor &visitor) const {
  return visitor.VisitBinary(const_cast<BinaryExprAST &>(*this));
}

/// CallExprAST - Expression class for function calls.
CallExprAST::CallExprAST(const std::string &callee,
                         std::vector<std::unique_ptr<ExprAST>> args) {
  callee_ = callee;
  args_ = std::move(args);
}
bool CallExprAST::Equals(const std::unique_ptr<ExprAST> &expression) const {
  auto call_expr = dynamic_cast<CallExprAST *>(expression.get());
  if (call_expr == nullptr)
    return false;

  if (call_expr->callee_ == callee_ &&
      call_expr->args_.size() == args_.size()) {
    auto arg = std::move(call_expr->args_);
    int pos_arg = 0;
    for (auto &it : arg) {
      if (!it->Equals(args_[pos_arg++]))
        return false;
    }
    return true;
  }
  return false;
}
llvm::Value *CallExprAST::GenerateCode(CodeVisitor &visitor) const {
  return visitor.VisitCall(const_cast<CallExprAST &>(*this));
}

/// PrototypeAST - This class represents the "prototype" for a function,
/// which captures its name, and its argument names (thus implicitly the number
/// of arguments the function takes).
PrototypeAST::PrototypeAST(const std::string &name,
                           std::vector<std::string> args) {
  name_ = name;
  args_ = std::move(args);
}
bool PrototypeAST::Equals(const std::unique_ptr<ExprAST> &expression) const {
  auto proto_expr = dynamic_cast<PrototypeAST *>(expression.get());
  if (proto_expr == nullptr)
    return false;
  return proto_expr->name_ == name_ && proto_expr->args_ == args_;
}
llvm::Value *PrototypeAST::GenerateCode(CodeVisitor &visitor) const {
  return visitor.VisitPrototype(const_cast<PrototypeAST &>(*this));
}

/// FunctionAST - This class represents a function definition itself.
FunctionAST::FunctionAST(std::unique_ptr<PrototypeAST> proto,
                         std::unique_ptr<ExprAST> body) {
  proto_ = std::move(proto);
  body_ = std::move(body);
}
bool FunctionAST::Equals(const std::unique_ptr<ExprAST> &expression) const {
  auto fun_expr = dynamic_cast<FunctionAST *>(expression.get());
  if (fun_expr == nullptr)
    return false;

  std::unique_ptr<ExprAST> expr_proto = std::move(fun_expr->proto_);
  std::unique_ptr<ExprAST> expr_body = std::move(fun_expr->body_);
  return proto_->Equals(expr_proto) && body_->Equals(expr_body);
}
llvm::Value *FunctionAST::GenerateCode(CodeVisitor &visitor) const {
  return visitor.VisitFunction(const_cast<FunctionAST &>(*this));
}
