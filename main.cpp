#include "lexer.h"
#include "parser.h"
#include "compiler.h"
#include "vm.h"
#include "debugger.h"
#include <iostream>
#include <fstream>
#include <sstream>

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "Usage: microc <file.mc> [--debug]" << std::endl;
        return 1;
    }
    
    std::ifstream file(argv[1]);
    if (!file) {
        std::cerr << "Error: Cannot open file " << argv[1] << std::endl;
        return 1;
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string source = buffer.str();
    
    bool debugMode = (argc > 2 && std::string(argv[2]) == "--debug");
    
    try {
        Lexer lexer(source);
        auto tokens = lexer.tokenize();
        
        Parser parser(tokens);
        auto ast = parser.parse();
        
        VM vm;
        Compiler compiler(&vm);
        auto bytecode = compiler.compile(ast);
        
        vm.loadProgram(bytecode);
        
        if (debugMode) {
            Debugger debugger(&vm);
            debugger.start();
        } else {
            vm.run();
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}