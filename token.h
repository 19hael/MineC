#pragma once

#include <array>
#include <cstdint>
#include <string>

enum class TokenType {
    END_OF_FILE,
    NUMBER,
    IDENTIFIER,
    INT,
    VOID,
    ASM,
    ASM_CODE,
    RETURN,
    IF,
    ELSE,
    WHILE,
    PRINT,
    PLUS,
    MINUS,
    STAR,
    SLASH,
    ASSIGN,
    EQ,
    NEQ,
    LT,
    GT,
    LEQ,
    GEQ,
    LPAREN,
    RPAREN,
    LBRACE,
    RBRACE,
    LSQUARE,
    RSQUARE,
    SEMICOLON,
    COMMA
};

struct Token {
    TokenType type;
    std::string value;
    int line;
};

enum class OpCode {
    NOP,
    PUSH,
    POP,
    ADD,
    SUB,
    MUL,
    DIV,
    NEG,
    STORE_GLOBAL,
    LOAD_GLOBAL,
    STORE_LOCAL,
    LOAD_LOCAL,
    CMP_EQ,
    CMP_NEQ,
    CMP_LT,
    CMP_GT,
    CMP_LEQ,
    CMP_GEQ,
    JMP,
    JMP_IF_FALSE,
    CALL,
    RET,
    PRINT,
    EXEC_ASM,
    HALT
};

enum class Register {
    RAX,
    RBX,
    RCX,
    RDX,
    COUNT
};

struct CPURegisterFile {
    std::array<int64_t, static_cast<size_t>(Register::COUNT)> data;

    CPURegisterFile() {
        data.fill(0);
    }

    int64_t& operator[](Register reg) {
        return data[static_cast<size_t>(reg)];
    }

    const int64_t& operator[](Register reg) const {
        return data[static_cast<size_t>(reg)];
    }
};

struct CPUState {
    CPURegisterFile regs;
    bool zf;
    bool sf;

    CPUState() : regs(), zf(false), sf(false) {}

    void updateFlags(int64_t value) {
        zf = (value == 0);
        sf = (value < 0);
    }
};

struct Instruction {
    OpCode op;
    int64_t operand;
    std::string text;

    Instruction(OpCode opcode, int64_t opValue = 0, std::string textOperand = {})
        : op(opcode), operand(opValue), text(std::move(textOperand)) {}
};
