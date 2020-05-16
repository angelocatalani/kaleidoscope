#include "interpreter.h"

Interpreter::Interpreter(Parser &Parser) : parser_(Parser) {}

std::string Interpreter::Interpret() {

  Token token;
  CodeVisitor visitor;
  while ((token = parser_.GetTokenType()) != tok_eof) {
    switch (token) {
    case tok_semicolon: // ignore top-level semicolons.
      parser_.SkipToken();
      break;

    case tok_def: {
      auto def = parser_.HandleDefinition();
      def->GenerateCode(visitor);
    } break;

    case tok_extern: {
      auto ext = parser_.HandleExtern();
      ext->GenerateCode(visitor);
    } break;

    default: {
      auto top = parser_.HandleTopLevelExpression();
      top->GenerateCode(visitor);
    } break;
    }
  }
  std::string code;
  llvm::raw_string_ostream code_stream(code);
  code_stream << *visitor.module_;
  code_stream.flush();
  return code;
}
