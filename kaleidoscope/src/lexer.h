#ifndef KALEIDOSCOPE_LEXER_H
#define KALEIDOSCOPE_LEXER_H

#include <exception>
#include <iostream>
#include <vector>

class LexerError : public std::exception {
public:
  LexerError();
  explicit LexerError(const std::string &);

  const char *what() const noexcept override;

public:
  std::string message_;
};

enum Token {
  tok_eof = -1,

  // commands
  tok_def = -2,
  tok_extern = -3,

  // primary
  tok_identifier = -4,
  tok_number = -5,

  tok_comma = ',',
  tok_open_parenthesis = '(',
  tok_close_parenthesis = ')',
  tok_semicolon = ';',
  tok_plus = '+',
  tok_multiply = '*',
  tok_divide = '/',
  tok_minus = '-',
  tok_less = '<',
  tok_greater = '>',

};

// The Lexer converts plain text into a stream of tokens.
class Lexer {
  static const std::vector<char> vCharTokens_;
  std::string text_;
  int text_length_;
  int index_{0};
  char last_char_{' '};

  std::string token_value_{""};
  Token token_type_{tok_eof};

  char GetChar();
  bool IsTextMissing() const;

public:
  Lexer();
  explicit Lexer(std::string);
  void SetText(std::string);

  void Next();

  std::string GetTokenValue();

  Token GetTokenType();
};

#endif // KALEIDOSCOPE_LEXER_H
