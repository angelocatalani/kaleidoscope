#include "parser.h"
#include "lexer.h"
#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

void PrintLog(std::string msg) { std::cout << msg << std::endl; }

const std::map<Token, int> Parser::opPrecedence_ = {
    {tok_less, 10},  {tok_greater, 10},  {tok_plus, 20},
    {tok_minus, 20}, {tok_multiply, 40}, {tok_divide, 40},
};
int Parser::GetTokPrecedence() {
  auto token = lexer_.GetTokenType();
  auto it = opPrecedence_.find(token);
  if (it == opPrecedence_.end())
    return -1;
  return it->second;
}

std::unique_ptr<ExprAST> Parser::ParseNumberExpr() {
  double number = std::stod(lexer_.GetTokenValue());
  lexer_.Next();
  return std::make_unique<NumberExprAST>(number);
}

std::unique_ptr<ExprAST> Parser::ParseVariableExpr() {
  auto identifier = (lexer_.GetTokenValue());
  lexer_.Next();
  return std::make_unique<VariableExprAST>(identifier);
}

std::unique_ptr<PrototypeAST> Parser::ParsePrototype() {
  if (lexer_.GetTokenType() != tok_identifier)
    throw ParserError("identifier not found during prototype parsing");
  auto name = lexer_.GetTokenValue();
  auto args = std::vector<std::string>();
  lexer_.Next();
  if (lexer_.GetTokenType() != tok_open_parenthesis)
    throw ParserError("open parenthesis not found during prototype parsing");
  lexer_.Next();
  while (lexer_.GetTokenType() == tok_identifier) {
    args.push_back(lexer_.GetTokenValue());
    lexer_.Next();
  }
  if (lexer_.GetTokenType() != tok_close_parenthesis)
    throw ParserError("close parenthesis not found during prototype parsing");

  lexer_.Next();
  return std::make_unique<PrototypeAST>(std::move(name), std::move(args));
}

std::unique_ptr<PrototypeAST> Parser::ParseExtern() {
  lexer_.Next();
  return ParsePrototype();
}

std::unique_ptr<FunctionAST> Parser::ParseDefinition() {
  lexer_.Next();
  auto prototype = ParsePrototype();
  assert(prototype != nullptr);
  auto body = ParseExpression();
  assert(body != nullptr);
  return std::make_unique<FunctionAST>(std::move(prototype), std::move(body));
}

std::unique_ptr<ExprAST> Parser::ParseExpression() {
  auto lhs = ParsePrimary();
  assert(lhs != nullptr);
  return ParseBinOpRHS(0, std::move(lhs));
}

std::unique_ptr<ExprAST> Parser::ParseParenExpr() {
  lexer_.Next();
  auto expression = ParseExpression();
  assert(expression != nullptr);
  if (lexer_.GetTokenType() != tok_close_parenthesis)
    throw ParserError(
        "close parenthesis not found during parenthesis expression parsing");
  lexer_.Next();
  return expression;
}

std::vector<std::unique_ptr<ExprAST>> Parser::ParseCallArgsExpr() {
  lexer_.Next();
  auto args = std::vector<std::unique_ptr<ExprAST>>();
  while (lexer_.GetTokenType() != tok_close_parenthesis) {
    auto arg = ParseExpression();
    assert(arg != nullptr);
    args.push_back(std::move(arg));
  }
  lexer_.Next(); // eat ')'
  return args;
}

std::unique_ptr<ExprAST> Parser::ParseIdentifierExpr() {
  auto callee = lexer_.GetTokenValue();
  auto variable = ParseVariableExpr();
  assert(variable != nullptr);
  if (lexer_.GetTokenType() != tok_open_parenthesis)
    return variable;
  auto args = ParseCallArgsExpr();
  return std::make_unique<CallExprAST>(callee, std::move(args));
}

std::unique_ptr<ExprAST> Parser::ParsePrimary() {
  switch (lexer_.GetTokenType()) {
  default:
    throw ParserError("unknown token when expecting an expression");
  case tok_identifier:
    return ParseIdentifierExpr();
  case tok_number:
    return ParseNumberExpr();
  case tok_open_parenthesis:
    return ParseParenExpr();
  }
}

std::unique_ptr<FunctionAST> Parser::ParseTopLevelExpr() {
  auto expression = ParseExpression();
  assert(expression != nullptr);
  // Make an anonymous proto.
  auto Proto =
      std::make_unique<PrototypeAST>("__anon_expr", std::vector<std::string>());
  return std::make_unique<FunctionAST>(std::move(Proto), std::move(expression));
}

std::unique_ptr<ExprAST> Parser::ParseBinOpRHS(int ExprPrec,
                                               std::unique_ptr<ExprAST> LHS) {
  // If this is a binop, find its precedence.
  while (true) {
    int TokPrec = GetTokPrecedence();

    // If this is a binop that binds at least as tightly as the current binop,
    // consume it, otherwise we are done.
    if (TokPrec < ExprPrec)
      return LHS;

    // Okay, we know this is a binop.
    Token BinOp = lexer_.GetTokenType();
    lexer_.Next(); // eat binop

    // Parse the primary expression after the binary operator.
    auto RHS = ParsePrimary();
    if (!RHS)
      return nullptr;

    // If BinOp binds less tightly with RHS than the operator after RHS, let
    // the pending operator take RHS as its LHS.
    int NextPrec = GetTokPrecedence();
    if (TokPrec < NextPrec) {
      RHS = ParseBinOpRHS(TokPrec + 1, std::move(RHS));
      if (!RHS)
        return nullptr;
    }

    // Merge LHS/RHS.
    LHS =
        std::make_unique<BinaryExprAST>(BinOp, std::move(LHS), std::move(RHS));
  }
}
std::unique_ptr<FunctionAST> Parser::HandleDefinition() {
  auto def = ParseDefinition();
  assert(def != nullptr);
  PrintLog("[INFO]: parsed a definition");
  return def;
}

std::unique_ptr<PrototypeAST> Parser::HandleExtern() {
  auto ext = ParseExtern();
  assert(ext != nullptr);
  PrintLog("[INFO]: parsed a extern\n");
  return ext;
}
std::unique_ptr<FunctionAST> Parser::HandleTopLevelExpression() {
  auto top = ParseTopLevelExpr();
  assert(top != nullptr);
  PrintLog("[INFO]: parsed a top level expression");
  return top;
}

Parser::Parser(std::string text) { lexer_ = Lexer(std::move(text)); }

void Parser::SkipToken() { lexer_.Next(); }
Token Parser::GetTokenType() { return lexer_.GetTokenType(); }

ParserError::ParserError(const std::string &message) { message_ = message; }

const char *ParserError::what() const noexcept { return message_.c_str(); }
