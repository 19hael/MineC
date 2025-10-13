#pragma once
#include "token.h"
#include "ast.h"
#include <vector>
#include <map>
#include <string>

class VM {
    std::vector<Instruction> code;
    std::vector<int64_t> stack;
    std::vector<int64_t> globals;
    struct CallFrame {
        size_t returnAddress;
        std::vector<int64_t> locals;
    };
    std::vector<CallFrame> callStack;
    CPUState cpu;
    size_t pc;
    bool running;
    bool stepMode;

    void ensureGlobal(size_t index);

public:
    VM();
    void loadProgram(const std::vector<Instruction>& program);
    void run();
    void step();
    void setStepMode(bool enabled);
    void printState();
    void executeInstruction();
    void executeASM(const std::string& asmCode);
    CPUState& getCPU() { return cpu; }
};