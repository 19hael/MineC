#pragma once
#include "ast.h"
#include "token.h"
#include <vector>

class Parser {
    std::vector<Token> tokens;
    size_t pos;
    
    Token current();
    Token peek(int offset = 1);
    bool match(TokenType type);
    Token consume(TokenType type);
    
    ASTPtr parseProgram();
    ASTPtr parseDeclaration();
    ASTPtr parseVarDecl();
    ASTPtr parseFuncDecl();
    ASTPtr parseBlock();
    ASTPtr parseStatement();
    ASTPtr parseExpr();
    ASTPtr parseTerm();
    ASTPtr parseFactor();
    ASTPtr parseAsmBlock();
    
public:
    Parser(const std::vector<Token>& toks);
    ASTPtr parse();
};