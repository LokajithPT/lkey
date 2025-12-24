#include "Parser.h"
#include <stdexcept>
#include <iostream>

Parser::Parser(const std::vector<Token> &tokens) : tokens(tokens), hadError(false) {}

std::vector<std::unique_ptr<Stmt>> Parser::parse() {
  std::vector<std::unique_ptr<Stmt>> statements;
  while (!isAtEnd()) {
    try {
        statements.push_back(declaration());
    } catch (std::runtime_error &error) {
        hadError = true;
        std::cerr << "Parse Error: " << error.what() << std::endl;
        synchronize(); // Skip to next statement
    }
  }
  return statements;
}

std::unique_ptr<Stmt> Parser::declaration() {
  if (match({TokenType::HOW})) {
      if (match({TokenType::TO})) {
          // It's a class declaration: "hows to ClassName has"
          return classDeclaration();
      } else {
          // It's a function declaration: "how to functionName has"
          return functionDeclaration("function");
      }
  }
  if (match({TokenType::CLASS})) {
      return classDeclaration();
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
    
    // Only consume SO if this is not a class declaration
    if (!check(TokenType::DOT) && !check(TokenType::THATS)) {
        consume(TokenType::SO, "Expected 'so' before " + kind + " body.");
    }
    
    std::vector<std::unique_ptr<Stmt>> body;
    while (!check(TokenType::THATS) && !isAtEnd()) {
        body.push_back(declaration());
    }
    
    consume(TokenType::THATS, "Expected 'thats' after " + kind + " body.");
    consume(TokenType::HOW, "Expected 'how' after 'thats'.");
    
    return std::make_unique<Function>(name, params, std::move(body));
}

std::unique_ptr<Stmt> Parser::howsToDeclaration() {
    consume(TokenType::IS, "Expected 'is' after class name.");
    Token name = previous();  // Get class name
    
    std::vector<std::unique_ptr<Function>> methods;
    
    // Parse class methods
    while (!check(TokenType::END_OF_FILE) && !check(TokenType::DOT)) {
        if (match({TokenType::HOW})) {
            // Method declaration: "how to methodName has"
            auto methodDecl = functionDeclaration("method");
            auto methodPtr = dynamic_cast<Function*>(methodDecl.get());
            if (methodPtr) {
                std::unique_ptr<Function> method = std::make_unique<Function>(
                    methodPtr->name, methodPtr->params, std::move(methodPtr->body)
                );
                methods.push_back(std::move(method));
            }
        } else {
            // Skip unexpected tokens
            advance();
        }
    }
    
    consume(TokenType::DOT, "Expected '.' after class definition.");
    return std::make_unique<Class>(name, std::move(methods));
}
            } else if (check({TokenType::IDENTIFIER})) {
                // Might be a method without 'to': "how methodName has"
                auto methodName = advance(); // consume method name
                consume(TokenType::WITH, "Expected 'with' after method name.");
                consume(TokenType::SO, "Expected 'so' before method body.");
                
                // Parse method parameters and body
                std::vector<Token> params;
                if (match({TokenType::WITH})) {
                    do {
                        params.push_back(consume(TokenType::IDENTIFIER, "Expected parameter name."));
                    } while (match({TokenType::COMMA}));
                }
                
                consume(TokenType::SO, "Expected 'so' before method body.");
                
                std::vector<std::unique_ptr<Stmt>> body;
                while (!check(TokenType::THATS) && !isAtEnd()) {
                    body.push_back(declaration());
                }
                
                consume(TokenType::THATS, "Expected 'thats' after method body.");
                consume(TokenType::HOW, "Expected 'how' after 'thats'.");
                
                auto method = std::make_unique<Function>(methodName, params, std::move(body));
                methods.push_back(std::move(method));
            } else {
                // Skip unexpected tokens
                advance();
            }
        } else {
            // Skip unexpected tokens
            advance();
        }
    }
    
    consume(TokenType::DOT, "Expected '.' after class definition.");
    return std::make_unique<Class>(name, std::move(methods));
}

std::unique_ptr<Stmt> Parser::howsToDeclaration() {
    consume(TokenType::IS, "Expected 'is' after class name.");
    Token name = previous();  // Get class name
    
    std::vector<std::unique_ptr<Function>> methods;
    
    // Parse class methods
    while (!check(TokenType::END_OF_FILE) && !check(TokenType::DOT)) {
        if (match({TokenType::HOW})) {
            // Method declaration: "how to methodName has"
            auto methodDecl = functionDeclaration("method");
            auto methodPtr = dynamic_cast<Function*>(methodDecl.get());
            if (methodPtr) {
                std::unique_ptr<Function> method = std::make_unique<Function>(
                    methodPtr->name, methodPtr->params, std::move(methodPtr->body)
                );
                methods.push_back(std::move(method));
            }
        } else {
            // Skip unexpected tokens
            advance();
        }
    }
    
    consume(TokenType::DOT, "Expected '.' after class definition.");
    return std::make_unique<Class>(name, std::move(methods));
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
    if (match({TokenType::READ})) {
        return readStatement();
    }
    if (check(TokenType::PLEASE)) {
        return returnStatement();
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
        Token name = consume(TokenType::IDENTIFIER, "Expected variable name.");
        
        if (match({TokenType::FROM})) {
            // For Loop: with i from 1 to 10 then ...
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

            // 3. Increment: i is i plus 1 (Assignment, not Var declaration)
            std::unique_ptr<Expr> incrementValue = std::make_unique<Binary>(
                 std::make_unique<Variable>(name),
                 Token{TokenType::PLUS, "plus", name.line},
                 std::make_unique<Literal>("1", TokenType::NUMBER)
            );
            std::unique_ptr<Expr> assignment = std::make_unique<Assign>(name, std::move(incrementValue));
            bodyStmts.push_back(std::make_unique<Expression>(std::move(assignment)));
            
            std::unique_ptr<Stmt> whileBody = std::make_unique<Block>(std::move(bodyStmts));
            
            outerStmts.push_back(std::make_unique<While>(std::move(condition), std::move(whileBody)));
            
            return std::make_unique<Block>(std::move(outerStmts));

        } else if (match({TokenType::IS})) {
            // For Each Loop: with name is w as words ...
            Token iterator = consume(TokenType::IDENTIFIER, "Expected iterator variable name.");
            consume(TokenType::AS, "Expected 'as'.");
            Token modeToken = consume(TokenType::IDENTIFIER, "Expected loop mode ('words' or 'letter').");
            
            if (modeToken.lexeme != "words" && modeToken.lexeme != "letter") {
                 throw std::runtime_error("Invalid loop mode '" + modeToken.lexeme + "'. Expected 'words' or 'letter'.");
            }

            // Optional 'then' to be friendly, or strictly enforced? 
            // Existing loops enforce it. Let's enforce it for consistency.
            if (check(TokenType::THEN)) {
                consume(TokenType::THEN, "Expected 'then'.");
            }
            
            // Parse block of statements until '.'
            std::vector<std::unique_ptr<Stmt>> bodyStmts;
            while (!check(TokenType::DOT) && !isAtEnd()) {
                bodyStmts.push_back(declaration());
            }
            consume(TokenType::DOT, "Expected '.' after loop body.");
            
            std::unique_ptr<Stmt> body = std::make_unique<Block>(std::move(bodyStmts));
            
            return std::make_unique<ForEach>(iterator, name, modeToken.lexeme, std::move(body));
        } else {
            throw std::runtime_error("Expected 'from' or 'is' after variable name in 'with' statement.");
        }

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

std::unique_ptr<Stmt> Parser::readStatement() {
    Token name = consume(TokenType::IDENTIFIER, "Expected variable name after 'read'.");
    std::unique_ptr<Expr> prompt = nullptr;
    
    if (match({TokenType::AS})) {
        prompt = expression();
    }
    
    return std::make_unique<Read>(name, std::move(prompt));
}

std::unique_ptr<Stmt> Parser::returnStatement() {
    consume(TokenType::PLEASE, "Expected 'please'.");
    consume(TokenType::GIVE, "Expected 'give' after 'please'.");
    
    std::unique_ptr<Expr> value = nullptr;
    if (check(TokenType::NUMBER) || check(TokenType::STRING) || check(TokenType::IDENTIFIER) || 
        check(TokenType::LPAREN) || check(TokenType::MINUS) || check(TokenType::NOT)) {
        value = expression();
    }
    
    return std::make_unique<Return>(std::move(value));
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
  return assignment();
}

std::unique_ptr<Expr> Parser::assignment() {
  std::unique_ptr<Expr> expr = logic_or();

  if (match({TokenType::IS})) {
    Token equals = previous();
    std::unique_ptr<Expr> value = assignment();

    if (Variable* v = dynamic_cast<Variable*>(expr.get())) {
        Token name = v->name;
        return std::make_unique<Assign>(name, std::move(value));
    }
    
    throw std::runtime_error("Invalid assignment target at line " + std::to_string(equals.line) + ".");
  }

  return expr;
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
    // Check for empty array '()'
    if (check(TokenType::RPAREN)) {
         advance(); // consume ')'
         return std::make_unique<ArrayLiteral>(std::vector<std::unique_ptr<Expr>>{});
    }

    std::unique_ptr<Expr> expr = expression();
    
    // If we see a comma, it's an array
    if (check(TokenType::COMMA)) {
        std::vector<std::unique_ptr<Expr>> elements;
        elements.push_back(std::move(expr));
        
        while (match({TokenType::COMMA})) {
            if (check(TokenType::RPAREN)) break; // trailing comma support (1,)
            elements.push_back(expression());
        }
        consume(TokenType::RPAREN, "Expected ')' after array elements.");
        return std::make_unique<ArrayLiteral>(std::move(elements));
    }

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
  if (check(type)) return advance();
  error(peek(), message);
  throw std::runtime_error(message + " at line " + std::to_string(peek().line));
}

void Parser::error(Token token, std::string message) {
    hadError = true;
    std::cerr << "[line " << token.line << "] Error: " << message << std::endl;
}

void Parser::synchronize() {
    // Skip tokens until we find a statement boundary
    advance();
    
    while (!isAtEnd()) {
        if (previous().type == TokenType::DOT) return;
        
        switch (peek().type) {
            case TokenType::HOW:
            case TokenType::VAR:
            case TokenType::CLASS:
            case TokenType::SAY:
            case TokenType::PLEASE:
            case TokenType::WITH:
            case TokenType::WHEN:
                return;
            default:
                break;
        }
        advance();
    }
}
