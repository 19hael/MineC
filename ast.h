#pragma once
#include <string>
#include <vector>
#include <memory>

enum class ASTType {
    PROGRAM,
    VAR_DECL,
    FUNC_DECL,
    BLOCK,
    BINARY_OP,
    NUMBER,
    IDENTIFIER,
    CALL,
    RETURN,
    ASM_BLOCK,
    IF_STMT,
    WHILE_STMT,
    PRINT,
    ASSIGN,
    EXPR_STMT
};

struct ASTNode {
    ASTType type;
    std::string value;
    std::vector<std::shared_ptr<ASTNode>> children;
    
    ASTNode(ASTType t) : type(t) {}
    ASTNode(ASTType t, const std::string& v) : type(t), value(v) {}
};

using ASTPtr = std::shared_ptr<ASTNode>;