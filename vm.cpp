#include "vm.h"
#include <algorithm>
#include <iostream>
#include <sstream>
#include <stdexcept>

VM::VM() : pc(0), running(false), stepMode(false) {}

void VM::ensureGlobal(size_t index) {
    if (globals.size() <= index) {
        globals.resize(index + 1, 0);
    }
}

void VM::loadProgram(const std::vector<Instruction>& program) {
    code = program;
    pc = 0;
    stack.clear();
    globals.clear();
    callStack.clear();
    cpu = CPUState();
}

void VM::setStepMode(bool enabled) {
    stepMode = enabled;
}

void VM::executeASM(const std::string& asmCode) {
    std::istringstream iss(asmCode);
    std::string inst, arg1, arg2;

    while (iss >> inst) {
        std::transform(inst.begin(), inst.end(), inst.begin(), ::tolower);

        auto toRegister = [](const std::string& name) -> Register {
            if (name == "rax") return Register::RAX;
            if (name == "rbx") return Register::RBX;
            if (name == "rcx") return Register::RCX;
            if (name == "rdx") return Register::RDX;
            throw std::runtime_error("Unknown register '" + name + "'");
        };

        if (inst == "mov") {
            iss >> arg1 >> arg2;
            if (arg2[0] >= '0' && arg2[0] <= '9') {
                int64_t val = std::stoll(arg2);
                Register dstReg = toRegister(arg1);
                cpu.regs[dstReg] = val;
            } else {
                Register dstReg = toRegister(arg1);
                Register srcReg = toRegister(arg2);
                cpu.regs[dstReg] = cpu.regs[srcReg];
            }
        }
        else if (inst == "add") {
            iss >> arg1 >> arg2;
            int64_t val = std::stoll(arg2);
            Register dstReg = toRegister(arg1);
            cpu.regs[dstReg] += val;
            cpu.updateFlags(cpu.regs[dstReg]);
        }
        else if (inst == "sub") {
            iss >> arg1 >> arg2;
            int64_t val = std::stoll(arg2);
            Register dstReg = toRegister(arg1);
            cpu.regs[dstReg] -= val;
            cpu.updateFlags(cpu.regs[dstReg]);
        }
        else if (inst == "push") {
            iss >> arg1;
            Register reg = toRegister(arg1);
            stack.push_back(cpu.regs[reg]);
        }
        else if (inst == "pop") {
            iss >> arg1;
            if (!stack.empty()) {
                int64_t val = stack.back();
                stack.pop_back();
                Register reg = toRegister(arg1);
                cpu.regs[reg] = val;
            }
        }
        else if (inst == "inc") {
            iss >> arg1;
            Register reg = toRegister(arg1);
            cpu.regs[reg]++;
            cpu.updateFlags(cpu.regs[reg]);
        }
        else if (inst == "dec") {
            iss >> arg1;
            Register reg = toRegister(arg1);
            cpu.regs[reg]--;
            cpu.updateFlags(cpu.regs[reg]);
        }
    }
}

void VM::executeInstruction() {
    if (pc >= code.size()) {
        running = false;
        return;
    }

    Instruction inst = code[pc++];

    switch (inst.op) {
        case OpCode::PUSH:
            stack.push_back(inst.operand);
            break;
        case OpCode::POP:
            if (stack.empty()) throw std::runtime_error("Stack underflow on POP");
            stack.pop_back();
            break;
        case OpCode::ADD: {
            if (stack.size() < 2) throw std::runtime_error("Stack underflow on ADD");
            int64_t b = stack.back(); stack.pop_back();
            int64_t a = stack.back(); stack.pop_back();
            stack.push_back(a + b);
            break;
        }
        case OpCode::SUB: {
            if (stack.size() < 2) throw std::runtime_error("Stack underflow on SUB");
            int64_t b = stack.back(); stack.pop_back();
            int64_t a = stack.back(); stack.pop_back();
            stack.push_back(a - b);
            break;
        }
        case OpCode::MUL: {
            if (stack.size() < 2) throw std::runtime_error("Stack underflow on MUL");
            int64_t b = stack.back(); stack.pop_back();
            int64_t a = stack.back(); stack.pop_back();
            stack.push_back(a * b);
            break;
        }
        case OpCode::DIV: {
            if (stack.size() < 2) throw std::runtime_error("Stack underflow on DIV");
            int64_t b = stack.back(); stack.pop_back();
            int64_t a = stack.back(); stack.pop_back();
            stack.push_back(a / b);
            break;
        }
        case OpCode::STORE_GLOBAL: {
            if (stack.empty()) throw std::runtime_error("Stack underflow on STORE_GLOBAL");
            ensureGlobal(static_cast<size_t>(inst.operand));
            globals[static_cast<size_t>(inst.operand)] = stack.back();
            stack.pop_back();
            break;
        }
        case OpCode::LOAD_GLOBAL: {
            ensureGlobal(static_cast<size_t>(inst.operand));
            stack.push_back(globals[static_cast<size_t>(inst.operand)]);
            break;
        }
        case OpCode::STORE_LOCAL: {
            if (stack.empty()) throw std::runtime_error("Stack underflow on STORE_LOCAL");
            if (callStack.empty()) throw std::runtime_error("STORE_LOCAL without call frame");
            auto& locals = callStack.back().locals;
            size_t index = static_cast<size_t>(inst.operand);
            if (locals.size() <= index) locals.resize(index + 1, 0);
            locals[index] = stack.back();
            stack.pop_back();
            break;
        }
        case OpCode::LOAD_LOCAL: {
            if (callStack.empty()) throw std::runtime_error("LOAD_LOCAL without call frame");
            auto& locals = callStack.back().locals;
            size_t index = static_cast<size_t>(inst.operand);
            if (locals.size() <= index) locals.resize(index + 1, 0);
            stack.push_back(locals[index]);
            break;
        }
        case OpCode::CMP_EQ:
        case OpCode::CMP_NEQ:
        case OpCode::CMP_LT:
        case OpCode::CMP_GT:
        case OpCode::CMP_LEQ:
        case OpCode::CMP_GEQ: {
            if (stack.size() < 2) throw std::runtime_error("Stack underflow on comparison");
            int64_t b = stack.back(); stack.pop_back();
            int64_t a = stack.back(); stack.pop_back();
            int64_t result = 0;
            switch (inst.op) {
                case OpCode::CMP_EQ: result = (a == b); break;
                case OpCode::CMP_NEQ: result = (a != b); break;
                case OpCode::CMP_LT: result = (a < b); break;
                case OpCode::CMP_GT: result = (a > b); break;
                case OpCode::CMP_LEQ: result = (a <= b); break;
                case OpCode::CMP_GEQ: result = (a >= b); break;
                default: break;
            }
            stack.push_back(result);
            break;
        }
        case OpCode::JMP:
            pc = static_cast<size_t>(inst.operand);
            break;
        case OpCode::JMP_IF_FALSE: {
            if (stack.empty()) throw std::runtime_error("Stack underflow on JMP_IF_FALSE");
            int64_t value = stack.back();
            stack.pop_back();
            if (!value) {
                pc = static_cast<size_t>(inst.operand);
            }
            break;
        }
        case OpCode::CALL: {
            CallFrame frame;
            frame.returnAddress = pc;
            callStack.push_back(frame);
            pc = static_cast<size_t>(inst.operand);
            break;
        }
        case OpCode::RET: {
            if (callStack.empty()) throw std::runtime_error("RET without call frame");
            size_t returnAddress = callStack.back().returnAddress;
            callStack.pop_back();
            pc = returnAddress;
            break;
        }
        case OpCode::PRINT:
            if (stack.empty()) throw std::runtime_error("Stack underflow on PRINT");
            std::cout << stack.back() << std::endl;
            stack.pop_back();
            break;
        case OpCode::EXEC_ASM:
            executeASM(inst.text);
            break;
        case OpCode::HALT:
            running = false;
            break;
        default:
            break;
    }
}

void VM::step() {
    if (pc < code.size()) {
        executeInstruction();
        printState();
    }
}

void VM::run() {
    running = true;
    while (running && pc < code.size()) {
        executeInstruction();
        if (stepMode) {
            printState();
            std::cout << "Press ENTER to continue...";
            std::cin.get();
        }
    }
}

void VM::printState() {
    std::cout << "\n=== CPU State ===" << std::endl;
    std::cout << "PC: " << pc << std::endl;
    std::cout << "RAX: " << cpu.regs[Register::RAX] << std::endl;
    std::cout << "RBX: " << cpu.regs[Register::RBX] << std::endl;
    std::cout << "RCX: " << cpu.regs[Register::RCX] << std::endl;
    std::cout << "RDX: " << cpu.regs[Register::RDX] << std::endl;
    std::cout << "Flags: ZF=" << cpu.zf << " SF=" << cpu.sf << std::endl;
    std::cout << "Stack: [";
    for (size_t i = 0; i < stack.size(); i++) {
        std::cout << stack[i];
        if (i < stack.size() - 1) std::cout << ", ";
    }
    std::cout << "]" << std::endl;
}