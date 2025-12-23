#include "Lexer.h"
#include "Token.h"
#include "Parser.h"
#include "Expr.h"
#include "Stmt.h"
#include "Interpreter.h"

#include <iostream>
#include <vector>
#include <string>
#include <sstream>

using namespace std;

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
  
  case TokenType::NOTHING: return "NOTHING";

  case TokenType::END_OF_FILE: return "END_OF_FILE";
  default: return "UNKNOWN";
  }
}

int main(int argc, char *argv[]) {
  cout << "lowkey interpreter " << endl;

  string source = "var age is 25 \n"
                  "say age plus 5 \n"
                  "say \"Hello\" \n"
                  "when (age greater than 18) then \n"
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

  Interpreter interpreter;
  interpreter.interpret(statements);

  cout << "-----------------------------" << endl;

  return 0;
}
