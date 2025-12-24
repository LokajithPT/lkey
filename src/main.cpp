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

using namespace std;

// Helper to read file content
string readFile(const string& path) {
    ifstream file(path);
    if (!file.is_open()) {
        cerr << "Could not open file: " << path << endl;
        exit(74); // IO Error exit code
    }
    stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

int main(int argc, char *argv[]) {
    if (argc > 1) {
        string path = argv[1];
        
        // Check extension
        if (path.length() < 5 || path.substr(path.length() - 5) != ".lkey") {
             // Just a warning, or enforce it? Let's just warn.
             // cerr << "Warning: File does not end with .lkey" << endl;
        }

        string source = readFile(path);
        
        Lexer lexer(source);
        vector<Token> tokens = lexer.scanTokens();
        
        // Check for lexer errors? (scanTokens usually prints them)
        
        Parser parser(tokens);
        vector<unique_ptr<Stmt>> statements = parser.parse();
        
        // If parser failed (returned empty or printed errors), we might want to stop.
        // But our parser is simple and returns what it can.
        
        Interpreter interpreter;
        interpreter.interpret(statements);

    } else {
        cout << "Usage: lkeycpp <file.lkey>" << endl;
        cout << "Lowkey Interpreter v1.0" << endl;
    }

    return 0;
}