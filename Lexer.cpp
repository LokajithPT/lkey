#include "Lexer.h"
#include "Token.h"
#include <iostream>
#include <ostream>
#include <vector>

Lexer::Lexer(const std::string &source) : source(source) {
    keywords["plus"] = TokenType::PLUS;
    keywords["minus"] = TokenType::MINUS;
    keywords["into"] = TokenType::INTO;
    keywords["div"] = TokenType::DIV;
    keywords["reminder"] = TokenType::REMINDER;

    keywords["greater"] = TokenType::GREATER;
    keywords["lesser"] = TokenType::LESSER;
    keywords["equal"] = TokenType::EQUAL;
    keywords["not"] = TokenType::NOT;
    keywords["and"] = TokenType::AND;
    keywords["or"] = TokenType::OR;
    
    keywords["when"] = TokenType::WHEN;
    keywords["then"] = TokenType::THEN;
    keywords["nothing"] = TokenType::NOTHING;
    
    keywords["with"] = TokenType::WITH;
    keywords["from"] = TokenType::FROM;
    keywords["to"] = TokenType::TO;
    
    keywords["var"] = TokenType::VAR;
    keywords["is"] = TokenType::IS;
    keywords["read"] = TokenType::READ;
    keywords["say"] = TokenType::SAY;
}

std::vector<Token> Lexer::scanTokens() {
  while (!isAtEnd()) {
    start = current;
    scanToken();
  }

  tokens.push_back({TokenType::END_OF_FILE, "", line});
  return tokens;
}

bool Lexer::isAtEnd() { return current >= source.length(); }

char Lexer::advance() { return source[current++]; }

char Lexer::peek() {
  if (isAtEnd())
    return '\0';
  return source[current];
}

bool Lexer::isDigit(char c) { return c >= '0' && c <= '9'; }

bool Lexer::isAlpha(char c) {
  return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

bool Lexer::isAlphaNumeric(char c) {
    return isAlpha(c) || isDigit(c);
}

void Lexer::scanToken() {
  char c = advance();

  switch (c) {
  case '(':
    tokens.push_back({TokenType::LPAREN, "(", line});
    break;
  case ')':
    tokens.push_back({TokenType::RPAREN, ")", line});
    break;
  case ',':
    tokens.push_back({TokenType::COMMA, ",", line});
    break;
  case '.':
    tokens.push_back({TokenType::DOT, ".", line});
    break;
  case '#':
    tokens.push_back({TokenType::HASH, "#", line});
    break;

  case '"': 
    while (peek() != '"' && !isAtEnd()) {
        if (peek() == '\n') line++;
        advance();
    }

    if (isAtEnd()) {
        std::cerr << "[Line " << line << "] Error: Unterminated string." << std::endl;
        break;
    }

    // The closing ".
    advance();

    // Trim the surrounding quotes.
    {
        std::string value = source.substr(start + 1, current - start - 2);
        tokens.push_back({TokenType::STRING, value, line});
    }
    break;

  // Handle whitespace
  case ' ':
  case '\r':
  case '\t':
    // Ignore whitespace.
    break;

  case '\n':
    line++; // Increment line number for newlines.
    break;

  default:
    if (isDigit(c)) {
      while (isDigit(peek())) {
        advance();
      }
      tokens.push_back(
          {TokenType::NUMBER, source.substr(start, current - start), line});
    } else if (isAlpha(c)) {
      while (isAlphaNumeric(peek())) {
          advance();
      }

      std::string text = source.substr(start, current - start);
      TokenType type = TokenType::IDENTIFIER;
      
      if (keywords.find(text) != keywords.end()) {
          type = keywords[text];
      }
      
      tokens.push_back({type, text, line});
    } else {
      std::cerr << "[line  " << line << "] Error : unexpected char " << c << "."
                << std::endl;
    }
    break;
  }
}
