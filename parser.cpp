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
        TokenType typeToken = current().type;
        pos++;
        Token name = consume(TokenType::IDENTIFIER);

        bool isFunction = (current().type == TokenType::LPAREN);
        pos -= 2;

        if (isFunction) {
            return parseFuncDecl(typeToken);
        }

        if (typeToken != TokenType::INT) {
            throw std::runtime_error("Only int variables can be declared");
        }
        return parseVarDecl();
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

ASTPtr Parser::parseFuncDecl(TokenType returnType) {
    consume(returnType);
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
    switch (current().type) {
        case TokenType::INT:
            return parseVarDecl();
        case TokenType::ASM:
            return parseAsmBlock();
        case TokenType::IF:
            return parseIfStatement();
        case TokenType::WHILE:
            return parseWhileStatement();
        case TokenType::RETURN:
            return parseReturnStatement();
        case TokenType::PRINT:
            return parsePrintStatement();
        case TokenType::LBRACE:
            return parseBlock();
        default:
            return parseExpressionStatement();
    }
}

ASTPtr Parser::parseIfStatement() {
    consume(TokenType::IF);
    consume(TokenType::LPAREN);
    auto condition = parseExpr();
    consume(TokenType::RPAREN);

    auto wrapBlock = [](const ASTPtr& stmt) {
        if (stmt->type == ASTType::BLOCK) {
            return stmt;
        }
        auto block = std::make_shared<ASTNode>(ASTType::BLOCK);
        block->children.push_back(stmt);
        return block;
    };

    auto thenBranch = wrapBlock(parseStatement());

    ASTPtr elseBranch = nullptr;
    if (current().type == TokenType::ELSE) {
        pos++;
        elseBranch = wrapBlock(parseStatement());
    }

    auto ifNode = std::make_shared<ASTNode>(ASTType::IF_STMT);
    ifNode->children.push_back(condition);
    ifNode->children.push_back(thenBranch);
    if (elseBranch) {
        ifNode->children.push_back(elseBranch);
    }
    return ifNode;
}

ASTPtr Parser::parseWhileStatement() {
    consume(TokenType::WHILE);
    consume(TokenType::LPAREN);
    auto condition = parseExpr();
    consume(TokenType::RPAREN);

    auto wrapBlock = [](const ASTPtr& stmt) {
        if (stmt->type == ASTType::BLOCK) {
            return stmt;
        }
        auto block = std::make_shared<ASTNode>(ASTType::BLOCK);
        block->children.push_back(stmt);
        return block;
    };

    auto body = wrapBlock(parseStatement());

    auto whileNode = std::make_shared<ASTNode>(ASTType::WHILE_STMT);
    whileNode->children.push_back(condition);
    whileNode->children.push_back(body);
    return whileNode;
}

ASTPtr Parser::parseReturnStatement() {
    consume(TokenType::RETURN);
    auto ret = std::make_shared<ASTNode>(ASTType::RETURN);
    if (current().type != TokenType::SEMICOLON) {
        ret->children.push_back(parseExpr());
    }
    consume(TokenType::SEMICOLON);
    return ret;
}

ASTPtr Parser::parsePrintStatement() {
    consume(TokenType::PRINT);
    consume(TokenType::LPAREN);
    auto expr = parseExpr();
    consume(TokenType::RPAREN);
    consume(TokenType::SEMICOLON);
    auto print = std::make_shared<ASTNode>(ASTType::PRINT);
    print->children.push_back(expr);
    return print;
}

ASTPtr Parser::parseExpressionStatement() {
    auto expr = parseExpr();
    consume(TokenType::SEMICOLON);
    auto stmt = std::make_shared<ASTNode>(ASTType::EXPR_STMT);
    stmt->children.push_back(expr);
    return stmt;
}

ASTPtr Parser::parseAsmBlock() {
    consume(TokenType::ASM);
    consume(TokenType::LBRACE);

    std::string asmCode;
    while (current().type != TokenType::RBRACE && current().type != TokenType::END_OF_FILE) {
        asmCode += current().value;
        asmCode += ' ';
        pos++;
    }

    consume(TokenType::RBRACE);
    if (!asmCode.empty() && asmCode.back() == ' ') {
        asmCode.pop_back();
    }

    return std::make_shared<ASTNode>(ASTType::ASM_BLOCK, asmCode);
}

ASTPtr Parser::parseExpr() {
    return parseAssignment();
}

ASTPtr Parser::parseAssignment() {
    auto left = parseEquality();

    if (current().type == TokenType::ASSIGN) {
        if (left->type != ASTType::IDENTIFIER) {
            throw std::runtime_error("Invalid assignment target");
        }
        consume(TokenType::ASSIGN);
        auto value = parseAssignment();
        auto assign = std::make_shared<ASTNode>(ASTType::ASSIGN, left->value);
        assign->children.push_back(value);
        return assign;
    }

    return left;
}

ASTPtr Parser::parseEquality() {
    auto left = parseComparison();

    while (current().type == TokenType::EQ || current().type == TokenType::NEQ) {
        Token op = current();
        pos++;
        auto right = parseComparison();
        auto binOp = std::make_shared<ASTNode>(ASTType::BINARY_OP, op.value);
        binOp->children.push_back(left);
        binOp->children.push_back(right);
        left = binOp;
    }

    return left;
}

ASTPtr Parser::parseComparison() {
    auto left = parseTerm();

    while (current().type == TokenType::LT || current().type == TokenType::LEQ ||
           current().type == TokenType::GT || current().type == TokenType::GEQ) {
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

    while (current().type == TokenType::PLUS || current().type == TokenType::MINUS) {
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
    auto left = parseUnary();

    while (current().type == TokenType::STAR || current().type == TokenType::SLASH) {
        Token op = current();
        pos++;
        auto right = parseUnary();
        auto binOp = std::make_shared<ASTNode>(ASTType::BINARY_OP, op.value);
        binOp->children.push_back(left);
        binOp->children.push_back(right);
        left = binOp;
    }

    return left;
}

ASTPtr Parser::parseUnary() {
    if (current().type == TokenType::MINUS) {
        consume(TokenType::MINUS);
        auto right = parseUnary();
        auto zero = std::make_shared<ASTNode>(ASTType::NUMBER, "0");
        auto binOp = std::make_shared<ASTNode>(ASTType::BINARY_OP, "-");
        binOp->children.push_back(zero);
        binOp->children.push_back(right);
        return binOp;
    }
    return parsePrimary();
}

ASTPtr Parser::parsePrimary() {
    if (current().type == TokenType::NUMBER) {
        auto num = std::make_shared<ASTNode>(ASTType::NUMBER, current().value);
        pos++;
        return num;
    }

    if (current().type == TokenType::IDENTIFIER) {
        Token id = current();
        pos++;
        if (current().type == TokenType::LPAREN) {
            return finishCall(id.value);
        }
        return std::make_shared<ASTNode>(ASTType::IDENTIFIER, id.value);
    }

    if (current().type == TokenType::LPAREN) {
        consume(TokenType::LPAREN);
        auto expr = parseExpr();
        consume(TokenType::RPAREN);
        return expr;
    }

    throw std::runtime_error("Unexpected token in expression");
}

ASTPtr Parser::finishCall(const std::string& name) {
    consume(TokenType::LPAREN);
    auto call = std::make_shared<ASTNode>(ASTType::CALL, name);

    if (current().type != TokenType::RPAREN) {
        while (true) {
            call->children.push_back(parseExpr());
            if (current().type == TokenType::COMMA) {
                pos++;
                continue;
            }
            break;
        }
    }

    consume(TokenType::RPAREN);
    return call;
}
