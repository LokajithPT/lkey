#pragma once

#include "Expr.h"
#include "Stmt.h"
#include "Token.h"
#include <memory>
#include <string>
#include <vector>

class Parser {

public:
  Parser(const std::vector<Token> &tokens);

  std::vector<std::unique_ptr<Stmt>> parse();

private:
  const std::vector<Token> &tokens;
  bool hadError = false;
  int current = 0;

  // Statement parsing
  std::unique_ptr<Stmt> declaration();
  std::unique_ptr<Stmt> varDeclaration();
  std::unique_ptr<Stmt> functionDeclaration(std::string kind);
  std::unique_ptr<Stmt> classDeclaration();
  std::unique_ptr<Stmt> statement();
  std::unique_ptr<Stmt> ifStatement();
  std::unique_ptr<Stmt> withStatement();
  std::unique_ptr<Stmt> readStatement();
  std::unique_ptr<Stmt> returnStatement();
  std::unique_ptr<Stmt> printStatement();
  std::unique_ptr<Stmt> expressionStatement();

  // Expression parsing
  std::unique_ptr<Expr> expression();
  std::unique_ptr<Expr> assignment();
  std::unique_ptr<Expr> logic_or();
  std::unique_ptr<Expr> logic_and();
  std::unique_ptr<Expr> equality();
  std::unique_ptr<Expr> comparison();
  std::unique_ptr<Expr> term();
  std::unique_ptr<Expr> factor();
  std::unique_ptr<Expr> unary();
  std::unique_ptr<Expr> call();
  std::unique_ptr<Expr> primary();

  bool match(const std::vector<TokenType> &types);
  bool check(TokenType type);
  bool isAtEnd();
  Token advance();
  Token peek();
  Token previous();
  Token consume(TokenType type, std::string message);
  void synchronize();
  void error(Token token, std::string message);
};
