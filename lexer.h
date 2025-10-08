#pragma once
#include "token.h"
#include <vector>
#include <string>

class Lexer {
    std::string source;
    size_t pos;
    int line;
    
    void skipWhitespace();
    Token nextToken();
    Token readNumber();
    Token readIdentifier();
    Token readAsmBlock();
    
public:
    Lexer(const std::string& src);
    std::vector<Token> tokenize();
};