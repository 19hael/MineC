#include "parser.h"
#include <stdexcept>

Parser::Parser(const std::vector<Token>& toks) : tokens(toks), pos(0) {}

Token Parser::current() {
    return tokens[pos];
}

Token Parser::peek(int offset) {
    if (pos + offset < tokens.size()) return tokens[pos + offset];
    return tokens.back();
}

bool Parser::match(TokenType type) {
    if (current().type == type) {
        pos++;
        return true;
    }
    return false;
}

Token Parser::consume(TokenType type) {
    if (current().type != type) {
        throw std::runtime_error("Unexpected token at line " + std::to_string(current().line));
    }
    return tokens[pos++];
}

ASTPtr Parser::parse() {
    return parseProgram();
}

ASTPtr Parser::parseProgram() {
    auto prog = std::make_shared<ASTNode>(ASTType::PROGRAM);
    while (current().type != TokenType::END_OF_FILE) {
        prog->children.push_back(parseDeclaration());
    }
    return prog;
}

ASTPtr Parser::parseDeclaration() {
    if (current().type == TokenType::INT || current().type == TokenType::VOID) {
        Token typeToken = current();
        pos++;
        Token name = consume(TokenType::IDENTIFIER);
        
        if (current().type == TokenType::LPAREN) {
            pos--;
            pos--;
            return parseFuncDecl();
        } else {
            pos--;
            pos--;
            return parseVarDecl();
        }
    }
    throw std::runtime_error("Expected declaration");
}

ASTPtr Parser::parseVarDecl() {
    consume(TokenType::INT);
    Token name = consume(TokenType::IDENTIFIER);
    consume(TokenType::ASSIGN);
    auto expr = parseExpr();
    consume(TokenType::SEMICOLON);
    
    auto decl = std::make_shared<ASTNode>(ASTType::VAR_DECL, name.value);
    decl->children.push_back(expr);
    return decl;
}

ASTPtr Parser::parseFuncDecl() {
    consume(TokenType::VOID);
    Token name = consume(TokenType::IDENTIFIER);
    consume(TokenType::LPAREN);
    consume(TokenType::RPAREN);
    auto body = parseBlock();
    
    auto func = std::make_shared<ASTNode>(ASTType::FUNC_DECL, name.value);
    func->children.push_back(body);
    return func;
}

ASTPtr Parser::parseBlock() {
    consume(TokenType::LBRACE);
    auto block = std::make_shared<ASTNode>(ASTType::BLOCK);
    
    while (current().type != TokenType::RBRACE && current().type != TokenType::END_OF_FILE) {
        block->children.push_back(parseStatement());
    }
    
    consume(TokenType::RBRACE);
    return block;
}

ASTPtr Parser::parseStatement() {
    if (current().type == TokenType::INT) {
        return parseVarDecl();
    }
    if (current().type == TokenType::ASM) {
        return parseAsmBlock();
    }
    if (current().type == TokenType::PRINT) {
        pos++;
        consume(TokenType::LPAREN);
        auto expr = parseExpr();
        consume(TokenType::RPAREN);
        consume(TokenType::SEMICOLON);
        auto print = std::make_shared<ASTNode>(ASTType::PRINT);
        print->children.push_back(expr);
        return print;
    }
    if (current().type == TokenType::RETURN) {
        pos++;
        auto ret = std::make_shared<ASTNode>(ASTType::RETURN);
        if (current().type != TokenType::SEMICOLON) {
            ret->children.push_back(parseExpr());
        }
        consume(TokenType::SEMICOLON);
        return ret;
    }
    
    auto expr = parseExpr();
    consume(TokenType::SEMICOLON);
    return expr;
}

ASTPtr Parser::parseAsmBlock() {
    consume(TokenType::ASM);
    consume(TokenType::LBRACE);
    
    std::string asmCode;
    while (current().type != TokenType::RBRACE && current().type != TokenType::END_OF_FILE) {
        asmCode += current().value + " ";
        pos++;
    }
    
    consume(TokenType::RBRACE);
    
    return std::make_shared<ASTNode>(ASTType::ASM_BLOCK, asmCode);
}

ASTPtr Parser::parseExpr() {
    auto left = parseTerm();
    
    while (current().type == TokenType::PLUS || current().type == TokenType::MINUS ||
           current().type == TokenType::EQ || current().type == TokenType::NEQ ||
           current().type == TokenType::LT || current().type == TokenType::GT) {
        Token op = current();
        pos++;
        auto right = parseTerm();
        auto binOp = std::make_shared<ASTNode>(ASTType::BINARY_OP, op.value);
        binOp->children.push_back(left);
        binOp->children.push_back(right);
        left = binOp;
    }
    
    return left;
}

ASTPtr Parser::parseTerm() {
    auto left = parseFactor();
    
    while (current().type == TokenType::STAR || current().type == TokenType::SLASH) {
        Token op = current();
        pos++;
        auto right = parseFactor();
        auto binOp = std::make_shared<ASTNode>(ASTType::BINARY_OP, op.value);
        binOp->children.push_back(left);
        binOp->children.push_back(right);
        left = binOp;
    }
    
    return left;
}

ASTPtr Parser::parseFactor() {
    if (current().type == TokenType::NUMBER) {
        auto num = std::make_shared<ASTNode>(ASTType::NUMBER, current().value);
        pos++;
        return num;
    }
    if (current().type == TokenType::IDENTIFIER) {
        auto id = std::make_shared<ASTNode>(ASTType::IDENTIFIER, current().value);
        pos++;
        return id;
    }
    if (current().type == TokenType::LPAREN) {
        pos++;
        auto expr = parseExpr();
        consume(TokenType::RPAREN);
        return expr;
    }
    
    throw std::runtime_error("Unexpected token in expression");
}