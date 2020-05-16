#ifndef KALEIDOSCOPE_KALEIDOSCOPE_KALEIDOSCOPE_SRC_AST_H
#define KALEIDOSCOPE_KALEIDOSCOPE_KALEIDOSCOPE_SRC_AST_H

#include "code_visitor.h"
#include "lexer.h"
#include <iostream>
#include <vector>

/// ExprAST - Base class for all expression nodes.
class ExprAST {
public:
  virtual ~ExprAST();
  virtual bool Equals(const std::unique_ptr<ExprAST> &) const = 0;
  virtual llvm::Value *GenerateCode(CodeVisitor &) const = 0;
};

/// NumberExprAST - Expression class for numeric literals like "1.0".
class NumberExprAST : public ExprAST {

public:
  double val_;
  explicit NumberExprAST(double val);
  bool Equals(const std::unique_ptr<ExprAST> &) const override;
  llvm::Value *GenerateCode(CodeVisitor &) const override;
};

/// VariableExprAST - Expression class for referencing a variable, like "a".
class VariableExprAST : public ExprAST {

public:
  std::string name_;
  explicit VariableExprAST(const std::string &name_);
  bool Equals(const std::unique_ptr<ExprAST> &) const override;
  llvm::Value *GenerateCode(CodeVisitor &) const override;
};

/// BinaryExprAST - Expression class for a binary operator.
class BinaryExprAST : public ExprAST {

public:
  Token op_;
  std::unique_ptr<ExprAST> lhs_, rhs_;
  BinaryExprAST(Token op, std::unique_ptr<ExprAST> lhs,
                std::unique_ptr<ExprAST> rhs);
  bool Equals(const std::unique_ptr<ExprAST> &) const override;
  llvm::Value *GenerateCode(CodeVisitor &) const override;
};

/// CallExprAST - Expression class for function calls.
class CallExprAST : public ExprAST {

public:
  std::string callee_;
  std::vector<std::unique_ptr<ExprAST>> args_;
  CallExprAST(const std::string &callee,
              std::vector<std::unique_ptr<ExprAST>> args);
  bool Equals(const std::unique_ptr<ExprAST> &) const override;
  llvm::Value *GenerateCode(CodeVisitor &) const override;
};

/// PrototypeAST - This class represents the "prototype" for a function,
/// which captures its name, and its argument names (thus implicitly the number
/// of arguments the function takes).
class PrototypeAST : public ExprAST {

public:
  std::string name_;
  std::vector<std::string> args_;
  PrototypeAST(const std::string &Name, std::vector<std::string> args);
  bool Equals(const std::unique_ptr<ExprAST> &) const override;
  llvm::Value *GenerateCode(CodeVisitor &) const override;
};

/// FunctionAST - This class represents a function definition itself.
class FunctionAST : public ExprAST {

public:
  std::unique_ptr<PrototypeAST> proto_;
  std::unique_ptr<ExprAST> body_;
  FunctionAST(std::unique_ptr<PrototypeAST> Proto,
              std::unique_ptr<ExprAST> Body);
  bool Equals(const std::unique_ptr<ExprAST> &) const override;
  llvm::Value *GenerateCode(CodeVisitor &) const override;
};

#endif // KALEIDOSCOPE_KALEIDOSCOPE_KALEIDOSCOPE_SRC_AST_H
