#pragma once

#include "Expr.h"
#include "Token.h"
#include <memory>
#include <vector>

struct Stmt {
    virtual ~Stmt() = default;
};

// 'say "hello"'
struct Print : Stmt {
    std::unique_ptr<Expr> expression;

    Print(std::unique_ptr<Expr> expression) 
        : expression(std::move(expression)) {}
};

// 'var name is value'
struct Var : Stmt {
    Token name;
    std::unique_ptr<Expr> initializer;

    Var(Token name, std::unique_ptr<Expr> initializer) 
        : name(name), initializer(std::move(initializer)) {}
};

// '1 + 1' (expression statement)
struct Expression : Stmt {
    std::unique_ptr<Expr> expression;

    Expression(std::unique_ptr<Expr> expression) 
        : expression(std::move(expression)) {}
};

// 'when (condition) then ...'
struct If : Stmt {
    std::unique_ptr<Expr> condition;
    std::unique_ptr<Stmt> thenBranch;
    std::unique_ptr<Stmt> elseBranch;

    If(std::unique_ptr<Expr> condition, std::unique_ptr<Stmt> thenBranch, std::unique_ptr<Stmt> elseBranch)
        : condition(std::move(condition)), thenBranch(std::move(thenBranch)), elseBranch(std::move(elseBranch)) {}
};

// 'with (condition) then ...'
struct While : Stmt {
    std::unique_ptr<Expr> condition;
    std::unique_ptr<Stmt> body;

    While(std::unique_ptr<Expr> condition, std::unique_ptr<Stmt> body) 
        : condition(std::move(condition)), body(std::move(body)) {}
};

// Block of statements
struct Block : Stmt {
    std::vector<std::unique_ptr<Stmt>> statements;

    Block(std::vector<std::unique_ptr<Stmt>> statements) 
        : statements(std::move(statements)) {}
};

// Function declaration
struct Function : Stmt {
    Token name;
    std::vector<Token> params;
    std::vector<std::unique_ptr<Stmt>> body;

    Function(Token name, std::vector<Token> params, std::vector<std::unique_ptr<Stmt>> body)
        : name(name), params(params), body(std::move(body)) {}
};
