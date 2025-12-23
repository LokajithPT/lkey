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
