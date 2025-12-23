#include "Parser.h"
#include <iostream>

Parser::Parser(const std::vector<Token> &tokens) : tokens(tokens) {}

std::vector<std::unique_ptr<Stmt>> Parser::parse() {
  std::vector<std::unique_ptr<Stmt>> statements;
  while (!isAtEnd()) {
    try {
        statements.push_back(declaration());
    } catch (std::runtime_error &error) {
        // Synchronize here if we had error recovery
        return statements; // For now, just stop or return what we have
    }
  }
  return statements;
}

std::unique_ptr<Stmt> Parser::declaration() {
  if (match({TokenType::VAR})) {
      return varDeclaration();
  }
  return statement();
}

std::unique_ptr<Stmt> Parser::varDeclaration() {
    Token name = consume(TokenType::IDENTIFIER, "Expected variable name.");
    consume(TokenType::IS, "Expected 'is' after variable name.");
    std::unique_ptr<Expr> initializer = expression();
    return std::make_unique<Var>(name, std::move(initializer));
}

std::unique_ptr<Stmt> Parser::statement() {
    if (match({TokenType::SAY})) {
        return printStatement();
    }
    if (check(TokenType::WHEN)) {
        return ifStatement();
    }
    return expressionStatement();
}

std::unique_ptr<Stmt> Parser::ifStatement() {
    consume(TokenType::WHEN, "Expected 'when'.");

    // Handle 'when nothing' (the final else block)
    if (match({TokenType::NOTHING})) {
        consume(TokenType::THEN, "Expected 'then' after 'nothing'.");
        std::unique_ptr<Stmt> branch = statement();
        consume(TokenType::DOT, "Expected '.' after final when block.");
        return branch; 
        // Note: We return just the statement. The caller (the previous 'if') will 
        // put this into its 'elseBranch'.
    }

    // Handle 'when (condition)'
    consume(TokenType::LPAREN, "Expected '(' after 'when'.");
    std::unique_ptr<Expr> condition = expression();
    consume(TokenType::RPAREN, "Expected ')' after condition.");

    consume(TokenType::THEN, "Expected 'then' after condition.");
    std::unique_ptr<Stmt> thenBranch = statement();
    std::unique_ptr<Stmt> elseBranch = nullptr;

    if (match({TokenType::COMMA})) {
        // Chained 'when': "..., when ..."
        // Recursively parse the next 'when' as the else branch
        elseBranch = ifStatement();
    } else {
        consume(TokenType::DOT, "Expected '.' or ',' after statement in 'when' block.");
    }

    return std::make_unique<If>(std::move(condition), std::move(thenBranch), std::move(elseBranch));
}

std::unique_ptr<Stmt> Parser::printStatement() {
    std::unique_ptr<Expr> value = expression();
    return std::make_unique<Print>(std::move(value));
}

std::unique_ptr<Stmt> Parser::expressionStatement() {
    std::unique_ptr<Expr> expr = expression();
    return std::make_unique<Expression>(std::move(expr));
}

// ---------------- Grammar Rules ----------------

std::unique_ptr<Expr> Parser::expression() {
  return logic_or();
}

std::unique_ptr<Expr> Parser::logic_or() {
  std::unique_ptr<Expr> expr = logic_and();

  while (match({TokenType::OR})) {
    Token op = previous();
    std::unique_ptr<Expr> right = logic_and();
    expr = std::make_unique<Binary>(std::move(expr), op, std::move(right));
  }

  return expr;
}

std::unique_ptr<Expr> Parser::logic_and() {
  std::unique_ptr<Expr> expr = equality();

  while (match({TokenType::AND})) {
    Token op = previous();
    std::unique_ptr<Expr> right = equality();
    expr = std::make_unique<Binary>(std::move(expr), op, std::move(right));
  }

  return expr;
}

std::unique_ptr<Expr> Parser::equality() {
  std::unique_ptr<Expr> expr = comparison();

  while (match({TokenType::EQUAL, TokenType::NOT})) {
    Token op = previous();
    
    // Handle "equal to" or "not equal to"
    // The lexer gives us EQUAL or NOT. 
    // If it's NOT, we expect "equal" (and maybe "to").
    // Actually, simpler: 
    // If we see 'equal', we optionally consume 'to'.
    // If we see 'not', we might be looking at 'not equal'.
    
    // For now, let's assume basic binary structure:
    // a equal b
    
    // Special handling for noise words if they exist as IDENTIFIERS
    if (check(TokenType::IDENTIFIER) && peek().lexeme == "to") {
        advance();
    }
    
    std::unique_ptr<Expr> right = comparison();
    expr = std::make_unique<Binary>(std::move(expr), op, std::move(right));
  }

  return expr;
}

std::unique_ptr<Expr> Parser::comparison() {
  std::unique_ptr<Expr> expr = term();

  while (match({TokenType::GREATER, TokenType::LESSER})) {
    Token op = previous();
    
    // Consume optional "than" if present (it comes as an IDENTIFIER)
    if (check(TokenType::IDENTIFIER) && peek().lexeme == "than") {
        advance();
    }

    std::unique_ptr<Expr> right = term();
    expr = std::make_unique<Binary>(std::move(expr), op, std::move(right));
  }

  return expr;
}

std::unique_ptr<Expr> Parser::term() {
  std::unique_ptr<Expr> expr = factor();

  while (match({TokenType::PLUS, TokenType::MINUS})) {
    Token op = previous();
    std::unique_ptr<Expr> right = factor();
    expr = std::make_unique<Binary>(std::move(expr), op, std::move(right));
  }

  return expr;
}

std::unique_ptr<Expr> Parser::factor() {
  std::unique_ptr<Expr> expr = unary();

  while (match({TokenType::INTO, TokenType::DIV, TokenType::REMINDER})) {
    Token op = previous();
    std::unique_ptr<Expr> right = unary();
    expr = std::make_unique<Binary>(std::move(expr), op, std::move(right));
  }

  return expr;
}

std::unique_ptr<Expr> Parser::unary() {
  if (match({TokenType::NOT, TokenType::MINUS})) {
    Token op = previous();
    std::unique_ptr<Expr> right = unary();
    return std::make_unique<Unary>(op, std::move(right));
  }

  return primary();
}

std::unique_ptr<Expr> Parser::primary() {
  if (match({TokenType::NUMBER})) {
    return std::make_unique<Literal>(previous().lexeme, TokenType::NUMBER);
  }

  if (match({TokenType::STRING})) {
    return std::make_unique<Literal>(previous().lexeme, TokenType::STRING);
  }
  
  if (match({TokenType::IDENTIFIER})) {
      return std::make_unique<Variable>(previous());
  }

  if (match({TokenType::LPAREN})) {
    std::unique_ptr<Expr> expr = expression();
    consume(TokenType::RPAREN, "Expected ')' after expression.");
    return expr;
  }

  throw std::runtime_error("Expect expression.");
}

// ---------------- Helper Methods ----------------

bool Parser::match(const std::vector<TokenType> &types) {
  for (TokenType type : types) {
    if (check(type)) {
      advance();
      return true;
    }
  }
  return false;
}

bool Parser::check(TokenType type) {
  if (isAtEnd())
    return false;
  return peek().type == type;
}

bool Parser::isAtEnd() { 
    return peek().type == TokenType::END_OF_FILE; 
}

Token Parser::advance() {
  if (!isAtEnd())
    current++;
  return previous();
}

Token Parser::peek() { 
    return tokens[current]; 
}

Token Parser::previous() { 
    return tokens[current - 1]; 
}

Token Parser::consume(TokenType type, std::string message) {
  if (check(type))
    return advance();

  throw std::runtime_error(message + " at line " + std::to_string(peek().line));
}
