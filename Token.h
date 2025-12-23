#pragma once

#include <string>
#include <vector>

enum class TokenType {
  VAR,
  IS,

  PLUS,
  MINUS,
  INTO,
  DIV,
  REMINDER,

  GREATER,
  LESSER,
  EQUAL,
  NOT,

  AND,
  OR,

  WHEN,
  THEN,
  NOTHING,
  READ,
  SAY,

  IDENTIFIER,
  STRING,
  NUMBER,

  LPAREN,
  RPAREN,
  COMMA,
  DOT,

  HASH,
  END_OF_FILE
};

struct Token {
  TokenType type;
  std::string lexeme;
  int line;
};
