#include "lexer.h"
#include <cctype>

Lexer::Lexer(const std::string& src) : source(src), pos(0), line(1) {}

void Lexer::skipWhitespace() {
    while (pos < source.length() && isspace(source[pos])) {
        if (source[pos] == '\n') line++;
        pos++;
    }
}

Token Lexer::readNumber() {
    std::string num;
    while (pos < source.length() && isdigit(source[pos])) {
        num += source[pos++];
    }
    return {TokenType::NUMBER, num, line};
}

Token Lexer::readIdentifier() {
    std::string id;
    while (pos < source.length() && (isalnum(source[pos]) || source[pos] == '_')) {
        id += source[pos++];
    }
    
    if (id == "int") return {TokenType::INT, id, line};
    if (id == "void") return {TokenType::VOID, id, line};
    if (id == "asm") return {TokenType::ASM, id, line};
    if (id == "return") return {TokenType::RETURN, id, line};
    if (id == "if") return {TokenType::IF, id, line};
    if (id == "while") return {TokenType::WHILE, id, line};
    if (id == "print") return {TokenType::PRINT, id, line};
    
    return {TokenType::IDENTIFIER, id, line};
}

Token Lexer::readAsmBlock() {
    std::string code;
    int braceCount = 1;
    pos++;
    
    while (pos < source.length() && braceCount > 0) {
        if (source[pos] == '{') braceCount++;
        if (source[pos] == '}') {
            braceCount--;
            if (braceCount == 0) break;
        }
        if (source[pos] == '\n') line++;
        code += source[pos++];
    }
    
    return {TokenType::ASM_CODE, code, line};
}

Token Lexer::nextToken() {
    char c = source[pos];
    
    if (isdigit(c)) return readNumber();
    if (isalpha(c) || c == '_') return readIdentifier();
    
    pos++;
    switch (c) {
        case '+': return {TokenType::PLUS, "+", line};
        case '-': return {TokenType::MINUS, "-", line};
        case '*': return {TokenType::STAR, "*", line};
        case '/': return {TokenType::SLASH, "/", line};
        case '=':
            if (pos < source.length() && source[pos] == '=') {
                pos++;
                return {TokenType::EQ, "==", line};
            }
            return {TokenType::ASSIGN, "=", line};
        case '!':
            if (pos < source.length() && source[pos] == '=') {
                pos++;
                return {TokenType::NEQ, "!=", line};
            }
            break;
        case '<':
            if (pos < source.length() && source[pos] == '=') {
                pos++;
                return {TokenType::LEQ, "<=", line};
            }
            return {TokenType::LT, "<", line};
        case '>':
            if (pos < source.length() && source[pos] == '=') {
                pos++;
                return {TokenType::GEQ, ">=", line};
            }
            return {TokenType::GT, ">", line};
        case '(': return {TokenType::LPAREN, "(", line};
        case ')': return {TokenType::RPAREN, ")", line};
        case '{': return {TokenType::LBRACE, "{", line};
        case '}': return {TokenType::RBRACE, "}", line};
        case '[': return {TokenType::LSQUARE, "[", line};
        case ']': return {TokenType::RSQUARE, "]", line};
        case ';': return {TokenType::SEMICOLON, ";", line};
        case ',': return {TokenType::COMMA, ",", line};
    }
    
    return {TokenType::END_OF_FILE, "", line};
}

std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;
    while (pos < source.length()) {
        skipWhitespace();
        if (pos >= source.length()) break;
        tokens.push_back(nextToken());
    }
    tokens.push_back({TokenType::END_OF_FILE, "", line});
    return tokens;
}