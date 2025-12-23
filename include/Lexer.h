#pragma once

#include "Token.h"
#include <map>
#include <string>
#include <vector>

class Lexer {
public:
  Lexer(const std::string &source);
  std::vector<Token> scanTokens();

private:
  const std::string &source;
  std::vector<Token> tokens;
  std::map<std::string, TokenType> keywords;

  int start = 0;
  int current = 0;
  int line = 1;

  bool isAtEnd();
  void scanToken();
  char advance();
  char peek();
  bool isDigit(char c);
  bool isAlpha(char c);
  bool isAlphaNumeric(char c);
};
