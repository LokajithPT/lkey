#include "Interpreter.h"
#include "Token.h"
#include <stdexcept>
#include <string>
#include <typeinfo>
#include <variant>
#include <cmath>
#include <sstream>

// Exception for return statements
struct ReturnException {
    std::any value;
    ReturnException(std::any value) : value(value) {}
};

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

void Interpreter::executeBlock(const std::vector<std::unique_ptr<Stmt>>& statements, Environment* newEnvironment) {
    Environment* previous = this->environment;
    try {
        this->environment = newEnvironment;
        for (const auto& stmt : statements) {
            execute(*stmt);
        }
    } catch (...) {
        this->environment = previous;
        throw;
    }
    this->environment = previous;
}

void Interpreter::execute(const Stmt& stmt) {
    if (const Print* printStmt = dynamic_cast<const Print*>(&stmt)) {
        std::any value = evaluate(*printStmt->expression);
        std::cout << stringify(value) << std::endl;
    } else if (const Read* readStmt = dynamic_cast<const Read*>(&stmt)) {
        if (readStmt->prompt) {
             std::any val = evaluate(*readStmt->prompt);
             std::cout << stringify(val);
        }
        
        std::string input;
        std::getline(std::cin, input);
        
        // Auto-detect number
        try {
            size_t idx;
            double d = std::stod(input, &idx);
            if (idx == input.length()) {
                environment->define(readStmt->name.lexeme, d);
            } else {
                environment->define(readStmt->name.lexeme, input);
            }
        } catch (...) {
            environment->define(readStmt->name.lexeme, input);
        }
    } else if (const Var* varStmt = dynamic_cast<const Var*>(&stmt)) {
        std::any value = evaluate(*varStmt->initializer);
        environment->define(varStmt->name.lexeme, value);
    } else if (const Function* funcStmt = dynamic_cast<const Function*>(&stmt)) {
        // Store the function AST node in the environment
        environment->define(funcStmt->name.lexeme, funcStmt);
    } else if (const Return* returnStmt = dynamic_cast<const Return*>(&stmt)) {
        std::any value = std::any(); // Default to null/void
        if (returnStmt->value) {
            value = evaluate(*returnStmt->value);
        }
        throw ReturnException(value);
    } else if (const Expression* exprStmt = dynamic_cast<const Expression*>(&stmt)) {
        evaluate(*exprStmt->expression);
    } else if (const If* ifStmt = dynamic_cast<const If*>(&stmt)) {
        if (!ifStmt->condition) {
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
    } else if (const ForEach* forStmt = dynamic_cast<const ForEach*>(&stmt)) {
        // 1. Get source string
        std::any sourceVal = environment->get(forStmt->source);
        if (sourceVal.type() != typeid(std::string)) {
             throw std::runtime_error("Can only iterate over strings.");
        }
        std::string sourceStr = std::any_cast<std::string>(sourceVal);

        // 2. Prepare environment for iterator
        Environment loopEnv(environment);
        loopEnv.define(forStmt->iterator.lexeme, std::string("")); // Define iterator variable

        // 3. Iterate
        if (forStmt->mode == "letter") {
            for (char c : sourceStr) {
                loopEnv.assign(forStmt->iterator, std::string(1, c));
                
                // Execute body with loopEnv
                Environment* previous = this->environment;
                this->environment = &loopEnv;
                try {
                    execute(*forStmt->body);
                } catch (...) {
                    this->environment = previous;
                    throw;
                }
                this->environment = previous;
            }
        } else if (forStmt->mode == "words") {
            std::stringstream ss(sourceStr);
            std::string word;
            while (ss >> word) {
                loopEnv.assign(forStmt->iterator, word);
                
                // Execute body
                Environment* previous = this->environment;
                this->environment = &loopEnv;
                try {
                    execute(*forStmt->body);
                } catch (...) {
                    this->environment = previous;
                    throw;
                }
                this->environment = previous;
            }
        }
    } else if (const Block* blockStmt = dynamic_cast<const Block*>(&stmt)) {
        // Create a new scope for the block on the stack (RAII)
        Environment blockEnv(environment);
        executeBlock(blockStmt->statements, &blockEnv);
    } else {
        throw std::runtime_error("Unknown statement type encountered.");
    }
}

std::any Interpreter::evaluate(const Expr& expr) {
    if (const Literal* literal = dynamic_cast<const Literal*>(&expr)) {
        if (literal->type == TokenType::NUMBER) {
            return std::stod(literal->value);
        } else if (literal->type == TokenType::STRING) {
            return literal->value;
        }
        return std::any();
    } else if (const Variable* variable = dynamic_cast<const Variable*>(&expr)) {
        return environment->get(variable->name);
    } else if (const Assign* assign = dynamic_cast<const Assign*>(&expr)) {
        std::any value = evaluate(*assign->value);
        environment->assign(assign->name, value);
        return value;
    } else if (const Call* call = dynamic_cast<const Call*>(&expr)) {
        std::any callee = evaluate(*call->callee);
        
        const Function* function = nullptr;
        try {
            function = std::any_cast<const Function*>(callee);
        } catch (...) {
            throw std::runtime_error("Can only call functions.");
        }
        
        if (call->arguments.size() != function->params.size()) {
             throw std::runtime_error("Expected " + std::to_string(function->params.size()) + " args but got " + std::to_string(call->arguments.size()) + ".");
        }
        
        // Create function environment on stack (RAII)
        Environment fnEnv(&globals); 
        
        for (size_t i = 0; i < function->params.size(); ++i) {
            fnEnv.define(function->params[i].lexeme, evaluate(*call->arguments[i]));
        }
        
        try {
            executeBlock(function->body, &fnEnv);
        } catch (const ReturnException& returnValue) {
            return returnValue.value;
        }
        
        return std::any(); // Void return
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
                    throw std::runtime_error("Operand must be a number for unary minus.");
                }
            default: break;
        }
    } else if (const Binary* binary = dynamic_cast<const Binary*>(&expr)) {
        std::any left = evaluate(*binary->left);
        std::any right = evaluate(*binary->right);

        switch (binary->op.type) {
            case TokenType::PLUS:
                if (left.type() == typeid(double) && right.type() == typeid(double)) {
                    return std::any_cast<double>(left) + std::any_cast<double>(right);
                }
                if (left.type() == typeid(std::string) && right.type() == typeid(std::string)) {
                    return std::any_cast<std::string>(left) + std::any_cast<std::string>(right);
                }
                if (left.type() == typeid(std::string) && right.type() == typeid(double)) {
                    return std::any_cast<std::string>(left) + stringify(right);
                }
                if (left.type() == typeid(double) && right.type() == typeid(std::string)) {
                    return stringify(left) + std::any_cast<std::string>(right);
                }
                throw std::runtime_error("Operands must be two numbers or two strings for '+'.");
            case TokenType::MINUS:
                checkNumberOperands(binary->op, left, right);
                return std::any_cast<double>(left) - std::any_cast<double>(right);
            case TokenType::INTO:
                checkNumberOperands(binary->op, left, right);
                return std::any_cast<double>(left) * std::any_cast<double>(right);
            case TokenType::DIV:
                checkNumberOperands(binary->op, left, right);
                if (std::any_cast<double>(right) == 0.0) throw std::runtime_error("Division by zero.");
                return std::any_cast<double>(left) / std::any_cast<double>(right);
            case TokenType::REMINDER:
                checkNumberOperands(binary->op, left, right);
                return fmod(std::any_cast<double>(left), std::any_cast<double>(right));
            case TokenType::GREATER:
                checkNumberOperands(binary->op, left, right);
                return std::any_cast<double>(left) > std::any_cast<double>(right);
            case TokenType::LESSER:
                checkNumberOperands(binary->op, left, right);
                return std::any_cast<double>(left) < std::any_cast<double>(right);
            case TokenType::EQUAL:
                return isEqual(left, right);
            default: break;
        }
    }
    return std::any();
}

// Helpers
bool Interpreter::isTruthy(const std::any& value) {
    if (!value.has_value()) return false;
    if (value.type() == typeid(bool)) return std::any_cast<bool>(value);
    if (value.type() == typeid(double)) return std::any_cast<double>(value) != 0.0;
    if (value.type() == typeid(std::string)) return !std::any_cast<std::string>(value).empty();
    return true;
}

bool Interpreter::isEqual(const std::any& a, const std::any& b) {
    if (!a.has_value() && !b.has_value()) return true;
    if (!a.has_value() || !b.has_value()) return false;
    if (a.type() == typeid(double) && b.type() == typeid(double)) return std::any_cast<double>(a) == std::any_cast<double>(b);
    if (a.type() == typeid(std::string) && b.type() == typeid(std::string)) return std::any_cast<std::string>(a) == std::any_cast<std::string>(b);
    if (a.type() == typeid(bool) && b.type() == typeid(bool)) return std::any_cast<bool>(a) == std::any_cast<bool>(b);
    return false;
}

void Interpreter::checkNumberOperand(const Token& op, const std::any& operand) {
    if (operand.type() == typeid(double)) return;
    throw std::runtime_error("Operand must be a number.");
}

void Interpreter::checkNumberOperands(const Token& op, const std::any& left, const std::any& right) {
    if (left.type() == typeid(double) && right.type() == typeid(double)) return;
    throw std::runtime_error("Operands must be numbers.");
}

std::string Interpreter::stringify(const std::any& value) {
    if (!value.has_value()) return "nil";
    if (value.type() == typeid(double)) {
        double d = std::any_cast<double>(value);
        if (d == static_cast<long long>(d)) return std::to_string(static_cast<long long>(d));
        return std::to_string(d);
    }
    if (value.type() == typeid(std::string)) return std::any_cast<std::string>(value);
    if (value.type() == typeid(bool)) return std::any_cast<bool>(value) ? "true" : "false";
    return "UNKNOWN_TYPE";
}
