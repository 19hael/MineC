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
    ASTPtr parseFuncDecl(TokenType returnType);
    ASTPtr parseBlock();
    ASTPtr parseStatement();
    ASTPtr parseIfStatement();
    ASTPtr parseWhileStatement();
    ASTPtr parseReturnStatement();
    ASTPtr parsePrintStatement();
    ASTPtr parseExpressionStatement();
    ASTPtr parseAsmBlock();
    ASTPtr parseExpr();
    ASTPtr parseAssignment();
    ASTPtr parseEquality();
    ASTPtr parseComparison();
    ASTPtr parseTerm();
    ASTPtr parseFactor();
    ASTPtr parseUnary();
    ASTPtr parsePrimary();
    ASTPtr finishCall(const std::string& name);
    
public:
    Parser(const std::vector<Token>& toks);
    ASTPtr parse();
};