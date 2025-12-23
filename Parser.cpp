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
  if (match({TokenType::HOW})) {
      return functionDeclaration("function");
  }
  if (match({TokenType::VAR})) {
      return varDeclaration();
  }
  return statement();
}

std::unique_ptr<Stmt> Parser::functionDeclaration(std::string kind) {
    consume(TokenType::TO, "Expected 'to' after 'how'.");
    Token name = consume(TokenType::IDENTIFIER, "Expected " + kind + " name.");
    
    std::vector<Token> params;
    if (match({TokenType::WITH})) {
        do {
            params.push_back(consume(TokenType::IDENTIFIER, "Expected parameter name."));
        } while (match({TokenType::COMMA}));
    }
    
    consume(TokenType::SO, "Expected 'so' before " + kind + " body.");
    
    std::vector<std::unique_ptr<Stmt>> body;
    while (!check(TokenType::THAS) && !isAtEnd()) {
        body.push_back(declaration());
    }
    
    consume(TokenType::THAS, "Expected 'thas' after " + kind + " body.");
    consume(TokenType::HOW, "Expected 'how' after 'thas'.");
    
    return std::make_unique<Function>(name, params, std::move(body));
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
    if (check(TokenType::WITH)) {
        return withStatement();
    }
    return expressionStatement();
}

std::unique_ptr<Stmt> Parser::withStatement() {
    consume(TokenType::WITH, "Expected 'with'.");

    if (check(TokenType::IDENTIFIER)) {
        // For Loop: with i from 1 to 10 then ...
        Token name = consume(TokenType::IDENTIFIER, "Expected variable name.");
        consume(TokenType::FROM, "Expected 'from'.");
        std::unique_ptr<Expr> start = expression();
        consume(TokenType::TO, "Expected 'to'.");
        std::unique_ptr<Expr> end = expression();
        consume(TokenType::THEN, "Expected 'then'.");
        
        // Parse block of statements until '.'
        std::vector<std::unique_ptr<Stmt>> bodyStmts;
        while (!check(TokenType::DOT) && !isAtEnd()) {
            bodyStmts.push_back(declaration());
        }
        consume(TokenType::DOT, "Expected '.' after loop body.");

        // Desugar to:
        // var i is start
        // while (not (i greater end)) {
        //   body
        //   var i is i plus 1
        // }
        
        std::vector<std::unique_ptr<Stmt>> outerStmts;
        
        // 1. var i is start
        outerStmts.push_back(std::make_unique<Var>(name, std::move(start)));
        
        // 2. Condition: not (i greater end)
        std::unique_ptr<Expr> condition = std::make_unique<Unary>(
            Token{TokenType::NOT, "not", name.line},
            std::make_unique<Binary>(
                std::make_unique<Variable>(name), 
                Token{TokenType::GREATER, "greater", name.line},
                std::move(end)
            )
        );

        // 3. Increment: var i is i plus 1
        std::unique_ptr<Expr> increment = std::make_unique<Binary>(
             std::make_unique<Variable>(name),
             Token{TokenType::PLUS, "plus", name.line},
             std::make_unique<Literal>("1", TokenType::NUMBER)
        );
        bodyStmts.push_back(std::make_unique<Var>(name, std::move(increment)));
        
        std::unique_ptr<Stmt> whileBody = std::make_unique<Block>(std::move(bodyStmts));
        
        outerStmts.push_back(std::make_unique<While>(std::move(condition), std::move(whileBody)));
        
        return std::make_unique<Block>(std::move(outerStmts));

    } else {
        // While Loop: with (condition) then ...
        consume(TokenType::LPAREN, "Expected '(' after 'with'.");
        std::unique_ptr<Expr> condition = expression();
        consume(TokenType::RPAREN, "Expected ')' after condition.");
        consume(TokenType::THEN, "Expected 'then'.");
        
        // Parse block of statements until '.'
        std::vector<std::unique_ptr<Stmt>> bodyStmts;
        while (!check(TokenType::DOT) && !isAtEnd()) {
            bodyStmts.push_back(declaration());
        }
        consume(TokenType::DOT, "Expected '.' after loop body.");
        
        std::unique_ptr<Stmt> body = std::make_unique<Block>(std::move(bodyStmts));
        
        return std::make_unique<While>(std::move(condition), std::move(body));
    }
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

  return call();
}

std::unique_ptr<Expr> Parser::call() {
  std::unique_ptr<Expr> expr = primary();

  while (true) {
    if (match({TokenType::LPAREN})) {
      std::vector<std::unique_ptr<Expr>> arguments;
      if (!check(TokenType::RPAREN)) {
        do {
            arguments.push_back(expression());
        } while (match({TokenType::COMMA}));
      }
      consume(TokenType::RPAREN, "Expected ')' after arguments.");
      expr = std::make_unique<Call>(std::move(expr), std::move(arguments));
    } else {
      break;
    }
  }

  return expr;
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
