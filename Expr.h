#pragma once

#include "Token.h"
#include <memory>
#include <vector>
#include <string>

// Base class for all expressions
struct Expr {
    virtual ~Expr() = default;
};

// Represents a binary operation (e.g., a plus b, a greater than b)
struct Binary : Expr {
    std::unique_ptr<Expr> left;
    Token op; // The operator token (PLUS, MINUS, GREATER, etc.)
    std::unique_ptr<Expr> right;

    Binary(std::unique_ptr<Expr> left, Token op, std::unique_ptr<Expr> right)
        : left(std::move(left)), op(op), right(std::move(right)) {}
};

// Represents a unary operation (e.g., not true, -5)
struct Unary : Expr {
    Token op;
    std::unique_ptr<Expr> right;

    Unary(Token op, std::unique_ptr<Expr> right)
        : op(op), right(std::move(right)) {}
};

// Represents a literal value (number, string)
struct Literal : Expr {
    std::string value; 
    TokenType type;

    Literal(std::string value, TokenType type) 
        : value(value), type(type) {}
};

// Represents a variable access (e.g., 'age' in 'age plus 1')
struct Variable : Expr {
    Token name;

    Variable(Token name) : name(name) {}
};