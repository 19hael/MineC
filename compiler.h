#pragma once
#include "ast.h"
#include "token.h"
#include "vm.h"
#include <vector>
#include <map>

class Compiler {
    std::vector<Instruction> code;
    std::map<std::string, int> varMap;
    int varCounter;
    VM* vm;
    
    void compileNode(ASTPtr node);
    void compileExpr(ASTPtr node);
    void emit(OpCode op, int64_t operand = 0);
    
public:
    Compiler(VM* vmInstance);
    std::vector<Instruction> compile(ASTPtr ast);
};