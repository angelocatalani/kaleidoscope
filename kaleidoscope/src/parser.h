#ifndef KALEIDOSCOPE_KALEIDOSCOPE_KALEIDOSCOPE_SRC_PARSER_H
#define KALEIDOSCOPE_KALEIDOSCOPE_KALEIDOSCOPE_SRC_PARSER_H
#include "ast.h"
#include "lexer.h"
#include <map>

class ParserError : public std::exception {
public:
  explicit ParserError(const std::string &);

  const char *what() const noexcept override;

public:
  std::string message_;
};

class Parser {
  const static std::map<Token, int> opPrecedence_;
  Lexer lexer_;

  int GetTokPrecedence();
  std::vector<std::unique_ptr<ExprAST>> ParseCallArgsExpr();
  std::unique_ptr<ExprAST> ParseExpression();
  std::unique_ptr<ExprAST> ParseVariableExpr();
  std::unique_ptr<ExprAST> ParseNumberExpr();
  std::unique_ptr<ExprAST> ParseParenExpr();
  std::unique_ptr<ExprAST> ParseIdentifierExpr();
  std::unique_ptr<ExprAST> ParsePrimary();
  std::unique_ptr<ExprAST> ParseBinOpRHS(int ExprPrec,
                                         std::unique_ptr<ExprAST> LHS);
  std::unique_ptr<PrototypeAST> ParsePrototype();
  std::unique_ptr<FunctionAST> ParseDefinition();
  std::unique_ptr<FunctionAST> ParseTopLevelExpr();
  std::unique_ptr<PrototypeAST> ParseExtern();

public:
  explicit Parser(std::string);
  std::unique_ptr<FunctionAST> HandleDefinition();
  std::unique_ptr<PrototypeAST> HandleExtern();
  std::unique_ptr<FunctionAST> HandleTopLevelExpression();
  Token GetTokenType();
  void SkipToken();
};

#endif // KALEIDOSCOPE_KALEIDOSCOPE_KALEIDOSCOPE_SRC_PARSER_H
