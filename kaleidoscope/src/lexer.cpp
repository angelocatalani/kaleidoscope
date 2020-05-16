#include "lexer.h"
#include <utility>

const std::vector<char> Lexer::vCharTokens_ = {'+', ';', '(', ')', '-',
                                               '*', '/', '<', ',', '>'};
Lexer::Lexer() { text_length_ = -1; };

Lexer::Lexer(std::string text) {
  text_ = std::move(text);
  text_length_ = text_.length();
  Next(); // initialize the lexer
};

void Lexer::Next() {
  if (IsTextMissing())
    throw LexerError("text to parse not found");
  // Skip any whitespace.
  while (isspace(last_char_)) {
    last_char_ = GetChar();
  }

  if (isalpha(last_char_)) { // identifier: [a-zA-Z][a-zA-Z0-9]*
    token_value_ = last_char_;
    while (isalnum((last_char_ = GetChar())))
      token_value_ += last_char_;

    if (token_value_ == "def")
      token_type_ = tok_def;
    else if (token_value_ == "extern")
      token_type_ = tok_extern;
    else
      token_type_ = tok_identifier;
    return;
  }

  if (isdigit(last_char_) || last_char_ == '.') { // Number: [0-9.]+
    std::string numeric_literal;
    do {
      numeric_literal += last_char_;
      last_char_ = GetChar();
    } while (isdigit(last_char_) || last_char_ == '.');

    token_type_ = tok_number;
    token_value_ = numeric_literal;
    return;
  }

  if (last_char_ == '#') {
    // Comment until end of line.
    do
      last_char_ = GetChar();
    while (last_char_ != EOF && last_char_ != '\n' && last_char_ != '\r');

    if (last_char_ != EOF)
      return Next();
  }

  // Check for valid characters that can be valid tokens.
  if (std::find(vCharTokens_.begin(), vCharTokens_.end(), last_char_) !=
      vCharTokens_.end()) {
    token_type_ = (Token)last_char_;
    token_value_ = last_char_;
    last_char_ = GetChar();
    return;
  }

  // Check for end of file.  Don't eat the EOF.
  if (last_char_ == EOF) {
    token_type_ = tok_eof;
    token_value_ = "EOF";
    return;
  }

  // Otherwise raise an exception.
  throw LexerError("unknown char: " + std::string(1, last_char_));
}

std::string Lexer::GetTokenValue() {
  if (IsTextMissing())
    throw LexerError("text to parse not found");

  return token_value_;
}

Token Lexer::GetTokenType() {
  if (IsTextMissing())
    throw LexerError("text to parse not found");
  return token_type_;
}

char Lexer::GetChar() {
  if (index_ >= text_length_)
    return EOF;
  auto ris = text_[index_];
  index_++;
  return ris;
}
void Lexer::SetText(std::string text) {
  text_ = std::move(text);
  text_length_ = text_.length();
  Next(); // initialize the lexer
}
bool Lexer::IsTextMissing() const { return text_length_ == -1; }

LexerError::LexerError(const std::string &message) { message_ = message; }

const char *LexerError::what() const noexcept { return message_.c_str(); }
