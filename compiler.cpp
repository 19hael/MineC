#include "compiler.h"
#include <iostream>

Compiler::Compiler(VM* vmInstance) : varCounter(0), vm(vmInstance) {}

void Compiler::emit(OpCode op, int64_t operand) {
    code.push_back({op, operand});
}

void Compiler::compileExpr(ASTPtr node) {
    if (node->type == ASTType::NUMBER) {
        emit(OpCode::PUSH, std::stoll(node->value));
    }
    else if (node->type == ASTType::IDENTIFIER) {
        if (varMap.find(node->value) != varMap.end()) {
            emit(OpCode::LOAD, varMap[node->value]);
        }
    }
    else if (node->type == ASTType::BINARY_OP) {
        compileExpr(node->children[0]);
        compileExpr(node->children[1]);
        
        if (node->value == "+") emit(OpCode::ADD);
        else if (node->value == "-") emit(OpCode::SUB);
        else if (node->value == "*") emit(OpCode::MUL);
        else if (node->value == "/") emit(OpCode::DIV);
    }
}

void Compiler::compileNode(ASTPtr node) {
    if (!node) return;
    
    switch (node->type) {
        case ASTType::PROGRAM:
            for (auto& child : node->children) {
                compileNode(child);
            }
            emit(OpCode::HALT);
            break;
            
        case ASTType::VAR_DECL:
            varMap[node->value] = varCounter++;
            compileExpr(node->children[0]);
            emit(OpCode::STORE, varMap[node->value]);
            break;
            
        case ASTType::FUNC_DECL:
            for (auto& stmt : node->children[0]->children) {
                compileNode(stmt);
            }
            break;
            
        case ASTType::PRINT:
            compileExpr(node->children[0]);
            emit(OpCode::PRINT);
            break;
            
        case ASTType::ASM_BLOCK:
            vm->executeASM(node->value);
            break;
            
        default:
            break;
    }
}

std::vector<Instruction> Compiler::compile(ASTPtr ast) {
    code.clear();
    compileNode(ast);
    return code;
}