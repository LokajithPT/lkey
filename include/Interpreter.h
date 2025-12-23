#pragma once

#include "Expr.h"
#include "Stmt.h"
#include "Token.h" // For TokenType::STRING, TokenType::NUMBER etc.

#include <vector>
#include <map>
#include <string>
#include <any> // For holding runtime values (C++17)
#include <iostream>

// Represents the runtime environment (variables and their values)
class Environment {
private:
    std::map<std::string, std::any> values;
    Environment* enclosing;

public:
    Environment() : enclosing(nullptr) {}
    Environment(Environment* enclosing) : enclosing(enclosing) {}

    // Define a new variable in the current environment
    void define(const std::string& name, std::any value) {
        values[name] = value;
    }

    // Get the value of a variable from the current environment or enclosing ones
    std::any get(const Token& name) {
        if (values.count(name.lexeme)) {
            return values[name.lexeme];
        }
        if (enclosing != nullptr) {
            return enclosing->get(name);
        }
        throw std::runtime_error("Undefined variable '" + name.lexeme + "' at line " + std::to_string(name.line) + ".");
    }

    // Assign a new value to an existing variable
    void assign(const Token& name, std::any value) {
        if (values.count(name.lexeme)) {
            values[name.lexeme] = value;
            return;
        }
        if (enclosing != nullptr) {
            enclosing->assign(name, value);
            return;
        }
        throw std::runtime_error("Undefined variable '" + name.lexeme + "' at line " + std::to_string(name.line) + ".");
    }
};

class Interpreter {
public:
    Interpreter() : environment(&globals) {}

    // Main interpretation function
    void interpret(const std::vector<std::unique_ptr<Stmt>>& statements);
    
    // Execute a block of statements with a specific environment
    void executeBlock(const std::vector<std::unique_ptr<Stmt>>& statements, Environment* newEnvironment);

private:
    Environment globals;
    Environment* environment; // The current runtime environment

    // Execute a single statement
    void execute(const Stmt& stmt);

    // Evaluate a single expression
    std::any evaluate(const Expr& expr);

    // Helper functions for type checking and operations
    void checkNumberOperand(const Token& op, const std::any& operand);
    void checkNumberOperands(const Token& op, const std::any& left, const std::any& right);

    // Helpers to convert to boolean for 'if' conditions
    bool isTruthy(const std::any& value);
    // Helpers for equality comparison
    bool isEqual(const std::any& a, const std::any& b);
    
    // Convert std::any to string for printing
    std::string stringify(const std::any& value);
};
