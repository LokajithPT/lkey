#include "Lexer.h"
#include "Token.h"
#include "Parser.h"
#include "Expr.h"
#include "Stmt.h" // Include Stmt

#include <iostream>
#include <vector>
#include <string>
#include <sstream>

using namespace std;

// --- AST Printer ---
string printExpr(const Expr& expr);
void printStmt(const Stmt& stmt); // Forward declaration

string parenthesize(const string& name, const vector<const Expr*>& exprs) {
    stringstream ss;
    ss << "(" << name;
    for (const Expr* expr : exprs) {
        ss << " ";
        ss << printExpr(*expr);
    }
    ss << ")";
    return ss.str();
}

string printExpr(const Expr& expr) {
    if (const Binary* binary = dynamic_cast<const Binary*>(&expr)) {
        return parenthesize(binary->op.lexeme, {binary->left.get(), binary->right.get()});
    } else if (const Unary* unary = dynamic_cast<const Unary*>(&expr)) {
        return parenthesize(unary->op.lexeme, {unary->right.get()});
    } else if (const Literal* literal = dynamic_cast<const Literal*>(&expr)) {
        return literal->value;
    } else if (const Variable* variable = dynamic_cast<const Variable*>(&expr)) {
        return variable->name.lexeme;
    }
    return "Unknown Expr Type";
}

void printStmt(const Stmt& stmt) {
    if (const Print* print = dynamic_cast<const Print*>(&stmt)) {
        cout << "(say " << printExpr(*print->expression) << ")" << endl;
    } else if (const Var* var = dynamic_cast<const Var*>(&stmt)) {
        cout << "(var " << var->name.lexeme << " " << printExpr(*var->initializer) << ")" << endl;
    } else if (const Expression* exprStmt = dynamic_cast<const Expression*>(&stmt)) {
        cout << "(expr " << printExpr(*exprStmt->expression) << ")" << endl;
    } else if (const If* ifStmt = dynamic_cast<const If*>(&stmt)) {
        if (ifStmt->condition) {
             cout << "(when " << printExpr(*ifStmt->condition) << ")" << endl;
        } else {
             // This branch should be handled by the recursive structure, 
             // but if we ever have a raw 'else', we can print it.
             // Actually, 'when nothing' returns a Stmt (e.g. Print), 
             // not an If with null condition, in my implementation logic.
             // So this check might be redundant but safe.
             cout << "(else)" << endl;
        }
        
        cout << "  then "; printStmt(*ifStmt->thenBranch);
        
        if (ifStmt->elseBranch) {
            cout << "  else "; printStmt(*ifStmt->elseBranch);
        }
    } else {
        cout << "Unknown Stmt Type" << endl;
    }
}
// --- End AST Printer ---

string TokenTypeToString(TokenType type) {
  switch (type) {
  case TokenType::LPAREN: return "LPAREN";
  case TokenType::RPAREN: return "RPAREN";
  case TokenType::COMMA: return "COMMA";
  case TokenType::DOT: return "DOT";
  case TokenType::HASH: return "HASH";
  case TokenType::NUMBER: return "NUMBER";
  case TokenType::STRING: return "STRING";
  
  case TokenType::VAR: return "VAR";
  case TokenType::IS: return "IS";
  case TokenType::PLUS: return "PLUS";
  case TokenType::MINUS: return "MINUS";
  case TokenType::INTO: return "INTO";
  case TokenType::DIV: return "DIV";
  case TokenType::REMINDER: return "REMINDER";
  
  case TokenType::GREATER: return "GREATER";
  case TokenType::LESSER: return "LESSER";
  case TokenType::EQUAL: return "EQUAL";
  case TokenType::NOT: return "NOT";
  case TokenType::AND: return "AND";
  case TokenType::OR: return "OR";
  
  case TokenType::WHEN: return "WHEN";
  case TokenType::THEN: return "THEN";
  case TokenType::SAY: return "SAY";
  case TokenType::IDENTIFIER: return "IDENTIFIER";
  
  case TokenType::NOTHING: return "NOTHING"; // Added NOTHING

  case TokenType::END_OF_FILE: return "END_OF_FILE";
  default: return "UNKNOWN";
  }
}

int main(int argc, char *argv[]) {
  cout << "lowkey interpreter " << endl;

  // Test If-Statement
  string source = "when (age greater than 18) then \n" 
                  "  say \"Adult\", \n" 
                  "when nothing then \n" 
                  "  say \"Kid\".";

  Lexer lexer(source); 
  vector<Token> tokens = lexer.scanTokens();

  cout << "-- scanned tokens --" << endl;
  for (const auto &token : tokens) {
    cout << "type : " << TokenTypeToString(token.type)
         << ", lexeme: '" << token.lexeme << "', Line: " << token.line << endl;
  }
  cout << "----------------------" << endl;

  Parser parser(tokens);
  vector<unique_ptr<Stmt>> statements = parser.parse();

  cout << "-- Parsed AST (Statements) --" << endl;
  for (const auto& stmt : statements) {
      if (stmt) printStmt(*stmt);
  }
  cout << "-----------------------------" << endl;

  return 0;
}