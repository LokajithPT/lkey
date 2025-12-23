#include "Interpreter.h"
#include "Token.h" // For error reporting
#include <stdexcept>
#include <string>
#include <typeinfo> // For std::bad_any_cast
#include <variant>  // Potentially useful for internal value representation, but std::any covers basics
#include <cmath>    // Required for fmod

// --- Environment methods ---
// No separate implementation file needed for Environment's simple methods for now.

// --- Interpreter methods ---

void Interpreter::interpret(const std::vector<std::unique_ptr<Stmt>>& statements) {
    try {
        for (const auto& stmt : statements) {
            execute(*stmt);
        }
    } catch (const std::runtime_error& error) {
        std::cerr << "Runtime Error: " << error.what() << std::endl;
    }
}

void Interpreter::execute(const Stmt& stmt) {
    if (const Print* printStmt = dynamic_cast<const Print*>(&stmt)) {
        std::any value = evaluate(*printStmt->expression);
        std::cout << stringify(value) << std::endl;
    } else if (const Var* varStmt = dynamic_cast<const Var*>(&stmt)) {
        std::any value = evaluate(*varStmt->initializer);
        environment.define(varStmt->name.lexeme, value);
    } else if (const Expression* exprStmt = dynamic_cast<const Expression*>(&stmt)) {
        evaluate(*exprStmt->expression); // Evaluate, but discard result
    } else if (const If* ifStmt = dynamic_cast<const If*>(&stmt)) {
        // Handle 'when nothing' else blocks - these return a simple statement
        if (!ifStmt->condition) { // This case is for the final 'when nothing'
            execute(*ifStmt->thenBranch);
            return;
        }

        if (isTruthy(evaluate(*ifStmt->condition))) {
            execute(*ifStmt->thenBranch);
        } else if (ifStmt->elseBranch) {
            execute(*ifStmt->elseBranch);
        }
    } else if (const While* whileStmt = dynamic_cast<const While*>(&stmt)) {
        while (isTruthy(evaluate(*whileStmt->condition))) {
            execute(*whileStmt->body);
        }
    } else if (const Block* blockStmt = dynamic_cast<const Block*>(&stmt)) {
        for (const auto& statement : blockStmt->statements) {
            execute(*statement);
        }
    } else {
        throw std::runtime_error("Unknown statement type encountered.");
    }
}

std::any Interpreter::evaluate(const Expr& expr) {
    if (const Literal* literal = dynamic_cast<const Literal*>(&expr)) {
        if (literal->type == TokenType::NUMBER) {
            return std::stod(literal->value); // Convert string number to double
        } else if (literal->type == TokenType::STRING) {
            return literal->value;
        }
        // Add boolean literal handling later if needed, for now use C++ bool for true/false
        return std::any(); // Should not happen for NUMBER/STRING
    } else if (const Variable* variable = dynamic_cast<const Variable*>(&expr)) {
        return environment.get(variable->name);
    } else if (const Unary* unary = dynamic_cast<const Unary*>(&expr)) {
        std::any right = evaluate(*unary->right);

        switch (unary->op.type) {
            case TokenType::NOT:
                return !isTruthy(right);
            case TokenType::MINUS:
                checkNumberOperand(unary->op, right);
                try {
                    return -std::any_cast<double>(right);
                } catch (const std::bad_any_cast& e) {
                    throw std::runtime_error("Operand must be a number for unary minus at line " + std::to_string(unary->op.line) + ".");
                }
            default:
                // Should not happen
                break;
        }
    } else if (const Binary* binary = dynamic_cast<const Binary*>(&expr)) {
        std::any left = evaluate(*binary->left);
        std::any right = evaluate(*binary->right);

        switch (binary->op.type) {
            case TokenType::PLUS:
                // Handle both number addition and string concatenation
                if (left.type() == typeid(double) && right.type() == typeid(double)) {
                    return std::any_cast<double>(left) + std::any_cast<double>(right);
                }
                if (left.type() == typeid(std::string) && right.type() == typeid(std::string)) {
                    return std::any_cast<std::string>(left) + std::any_cast<std::string>(right);
                }
                
                // Auto-convert number to string if one operand is a string
                if (left.type() == typeid(std::string) && right.type() == typeid(double)) {
                    return std::any_cast<std::string>(left) + stringify(right);
                }
                if (left.type() == typeid(double) && right.type() == typeid(std::string)) {
                    return stringify(left) + std::any_cast<std::string>(right);
                }
                
                throw std::runtime_error("Operands must be two numbers or two strings for '+' at line " + std::to_string(binary->op.line) + ".");
            case TokenType::MINUS:
                checkNumberOperands(binary->op, left, right);
                return std::any_cast<double>(left) - std::any_cast<double>(right);
            case TokenType::INTO: // Multiplication
                checkNumberOperands(binary->op, left, right);
                return std::any_cast<double>(left) * std::any_cast<double>(right);
            case TokenType::DIV: // Division
                checkNumberOperands(binary->op, left, right);
                if (std::any_cast<double>(right) == 0.0) {
                    throw std::runtime_error("Division by zero at line " + std::to_string(binary->op.line) + ".");
                }
                return std::any_cast<double>(left) / std::any_cast<double>(right);
            case TokenType::REMINDER: // Modulo
                checkNumberOperands(binary->op, left, right);
                return fmod(std::any_cast<double>(left), std::any_cast<double>(right)); // fmod for doubles
            
            case TokenType::GREATER:
                checkNumberOperands(binary->op, left, right);
                return std::any_cast<double>(left) > std::any_cast<double>(right);
            case TokenType::LESSER:
                checkNumberOperands(binary->op, left, right);
                return std::any_cast<double>(left) < std::any_cast<double>(right);
            case TokenType::EQUAL: // Lowkey's 'equal'
                return isEqual(left, right);
            case TokenType::AND: // Logical AND (handled by short-circuiting in parser, but could be here too)
            case TokenType::OR:  // Logical OR (handled by short-circuiting in parser, but could be here too)
                // For now, these are evaluated directly by Parser.
                // If they appear as binary ops here, it implies non-short-circuiting logic.
                // We'll treat them as errors if they reach here directly in the AST, as they
                // should be handled by the Parser's logic_and/logic_or.
                throw std::runtime_error("Logical operators AND/OR should be handled by parser directly.");

            default:
                // Should not happen for binary operators
                break;
        }
    }
    return std::any(); // Fallback for unknown expression types
}

// Helper: Check if an any value is 'truthy' for conditions
bool Interpreter::isTruthy(const std::any& value) {
    if (!value.has_value()) return false; // nil is false
    if (value.type() == typeid(bool)) return std::any_cast<bool>(value);
    // Lowkey rules: numbers != 0, non-empty strings are true?
    if (value.type() == typeid(double)) return std::any_cast<double>(value) != 0.0;
    if (value.type() == typeid(std::string)) return !std::any_cast<std::string>(value).empty();
    return true; // Any other value type is true (e.g., objects)
}

// Helper: Check if two any values are equal
bool Interpreter::isEqual(const std::any& a, const std::any& b) {
    if (!a.has_value() && !b.has_value()) return true; // nil == nil
    if (!a.has_value() || !b.has_value()) return false; // one nil, one not

    if (a.type() == typeid(double) && b.type() == typeid(double)) {
        return std::any_cast<double>(a) == std::any_cast<double>(b);
    }
    if (a.type() == typeid(std::string) && b.type() == typeid(std::string)) {
        return std::any_cast<std::string>(a) == std::any_cast<std::string>(b);
    }
    if (a.type() == typeid(bool) && b.type() == typeid(bool)) {
        return std::any_cast<bool>(a) == std::any_cast<bool>(b);
    }
    return false; // Different types are not equal
}

// Helper: Check if operand is a number
void Interpreter::checkNumberOperand(const Token& op, const std::any& operand) {
    if (operand.type() == typeid(double)) return;
    throw std::runtime_error("Operand must be a number for '" + op.lexeme + "' at line " + std::to_string(op.line) + ".");
}

// Helper: Check if both operands are numbers
void Interpreter::checkNumberOperands(const Token& op, const std::any& left, const std::any& right) {
    if (left.type() == typeid(double) && right.type() == typeid(double)) return;
    throw std::runtime_error("Operands must be numbers for '" + op.lexeme + "' at line " + std::to_string(op.line) + ".");
}

// Helper: Convert std::any to string for printing
std::string Interpreter::stringify(const std::any& value) {
    if (!value.has_value()) return "nil"; // Lowkey doesn't have nil, but for internal representation.
    if (value.type() == typeid(double)) {
        // Special handling for integers in doubles to print cleanly
        double d = std::any_cast<double>(value);
        if (d == static_cast<long long>(d)) {
            return std::to_string(static_cast<long long>(d));
        }
        return std::to_string(d);
    }
    if (value.type() == typeid(std::string)) return std::any_cast<std::string>(value);
    if (value.type() == typeid(bool)) return std::any_cast<bool>(value) ? "true" : "false";
    return "UNKNOWN_TYPE_IN_ANY";
}