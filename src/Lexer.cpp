#include "Lexer.h"
#include "Token.h"
#include <iostream>
#include <ostream>
#include <vector>

// Initialize static keyword map once
const std::map<std::string, TokenType> Lexer::keywords = {
    {"plus", TokenType::PLUS},
    {"minus", TokenType::MINUS},
    {"into", TokenType::INTO},
    {"div", TokenType::DIV},
    {"reminder", TokenType::REMINDER},
    
    {"greater", TokenType::GREATER},
    {"lesser", TokenType::LESSER},
    {"equal", TokenType::EQUAL},
    {"not", TokenType::NOT},
    {"and", TokenType::AND},
    {"or", TokenType::OR},
    
    {"when", TokenType::WHEN},
    {"then", TokenType::THEN},
    {"nothing", TokenType::NOTHING},
    
    {"with", TokenType::WITH},
    {"from", TokenType::FROM},
    {"to", TokenType::TO},
    
    {"how", TokenType::HOW},
    {"so", TokenType::SO},
    {"thats", TokenType::THATS},
    
    {"please", TokenType::PLEASE},
    {"give", TokenType::GIVE},
    
    {"var", TokenType::VAR},
    {"is", TokenType::IS},
    {"read", TokenType::READ},
    {"as", TokenType::AS},
    {"say", TokenType::SAY}
};

Lexer::Lexer(const std::string &source) : source(source) {
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
      
      // Handle comments
      if (text == "comment") {
          // Consume rest of the line
          while (peek() != '\n' && !isAtEnd()) advance();
          break; // Don't add token
      }

      TokenType type = TokenType::IDENTIFIER;
      
      auto it = keywords.find(text);
      if (it != keywords.end()) {
          type = it->second;
      }
      
      tokens.push_back({type, text, line});
    } else {
      std::cerr << "[line  " << line << "] Error : unexpected char " << c << "."
                << std::endl;
    }
    break;
  }
}
