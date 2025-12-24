#include "Lexer.h"
#include "Token.h"
#include "Parser.h"
#include "Stmt.h"
#include "Interpreter.h"

#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <fstream>

// Exit codes - following EX_* conventions
enum class ExitCode {
    SUCCESS = 0,
    IO_ERROR = 74,      // EX_IOERR - input/output error
    PARSE_ERROR = 65,   // EX_DATAERR - malformed data
    RUNTIME_ERROR = 70  // EX_SOFTWARE - internal software error
};

// Helper to read file content
std::string readFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "Could not open file: " << path << std::endl;
        exit(static_cast<int>(ExitCode::IO_ERROR));
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

int main(int argc, char *argv[]) {
    if (argc > 1) {
        std::string path = argv[1];
        
        // Check extension
        if (path.length() < 5 || path.substr(path.length() - 5) != ".lkey") {
             // Just a warning, or enforce it? Let's just warn.
             // std::cerr << "Warning: File does not end with .lkey" << std::endl;
        }

        std::string source = readFile(path);
        
        Lexer lexer(source);
        std::vector<Token> tokens = lexer.scanTokens();
        
        // Check for lexer errors? (scanTokens usually prints them)
        
        Parser parser(tokens);
        std::vector<std::unique_ptr<Stmt>> statements = parser.parse();
        
        // If parser failed (returned empty or printed errors), we might want to stop.
        // But our parser is simple and returns what it can.
        
        Interpreter interpreter;
        interpreter.interpret(statements);

    } else {
        std::cout << "Usage: lkeycpp <file.lkey>" << std::endl;
        std::cout << "Lowkey Interpreter v1.0" << std::endl;
    }

    return 0;
}