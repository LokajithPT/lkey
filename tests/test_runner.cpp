#include "Lexer.h"
#include "Parser.h"
#include "Interpreter.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <cassert>

// Test framework for lkey interpreter
class TestRunner {
public:
    int totalTests = 0;
    int passedTests = 0;
    void runTest(const std::string& testName, const std::string& code, const std::string& expectedOutput) {
        totalTests++;
        std::cout << "Running test: " << testName << "... ";
        
        try {
            // Create temp file with test code
            std::ofstream tempFile("temp_test.lkey");
            tempFile << code;
            tempFile.close();
            
            // Capture output
            std::ostringstream output;
            std::streambuf* oldCout = std::cout.rdbuf(output.rdbuf());
            
            // Run interpreter
            Lexer lexer(code);
            auto tokens = lexer.scanTokens();
            Parser parser(tokens);
            auto statements = parser.parse();
            Interpreter interpreter;
            interpreter.interpret(statements);
            
            // Restore cout
            std::cout.rdbuf(oldCout);
            
            // Check output
            std::string actualOutput = output.str();
            if (actualOutput == expectedOutput) {
                std::cout << "PASSED" << std::endl;
                passedTests++;
            } else {
                std::cout << "FAILED" << std::endl;
                std::cout << "  Expected: '" << expectedOutput << "'" << std::endl;
                std::cout << "  Actual:   '" << actualOutput << "'" << std::endl;
            }
            
            // Cleanup
            std::remove("temp_test.lkey");
            
        } catch (const std::exception& e) {
            std::cout << "FAILED (Exception: " << e.what() << ")" << std::endl;
        }
    }
    
    void printSummary() {
        std::cout << "\n=== Test Summary ===" << std::endl;
        std::cout << "Total: " << totalTests << std::endl;
        std::cout << "Passed: " << passedTests << std::endl;
        std::cout << "Failed: " << (totalTests - passedTests) << std::endl;
        std::cout << "Success Rate: " << (passedTests * 100 / totalTests) << "%" << std::endl;
    }
};

int main() {
    TestRunner runner;
    
    // Test basic math
    runner.runTest("Basic Math", 
                  "say 5 plus 3", 
                  "8\n");
    
    // Test variables
    runner.runTest("Variables", 
                  "var x is 10\nsay x", 
                  "10\n");
    
    // Test conditionals
    runner.runTest("Conditionals", 
                  "var score is 85\nwhen (score greater than 80) then say \"Pass\".", 
                  "Pass\n");
    
    // Test loops
    runner.runTest("Range Loop", 
                  "with i from 1 to 3 then say i.", 
                  "1\n2\n3\n");
    
    // Test arrays
    runner.runTest("Arrays", 
                  "var arr is (1, 2, 3)\nsay arr(1)", 
                  "2\n");
    
    // Test functions
    runner.runTest("Functions", 
                  "how to add with a, b so please give a plus b thats how\nsay add(5, 3)", 
                  "8\n");
    
    // Test error recovery
    runner.runTest("Error Recovery", 
                  "var good is 5\nvar good is 10\nsay good", 
                  "10\n");
    
    runner.printSummary();
    
    return (runner.passedTests == runner.totalTests) ? 0 : 1;
}