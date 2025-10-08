#include "vm.h"
#include <iostream>
#include <sstream>
#include <algorithm>

VM::VM() : pc(0), running(false), stepMode(false) {}

void VM::loadProgram(const std::vector<Instruction>& program) {
    code = program;
    pc = 0;
}

void VM::setStepMode(bool enabled) {
    stepMode = enabled;
}

void VM::executeASM(const std::string& asmCode) {
    std::istringstream iss(asmCode);
    std::string inst, arg1, arg2;
    
    while (iss >> inst) {
        std::transform(inst.begin(), inst.end(), inst.begin(), ::tolower);
        
        if (inst == "mov") {
            iss >> arg1 >> arg2;
            if (arg2[0] >= '0' && arg2[0] <= '9') {
                int64_t val = std::stoll(arg2);
                if (arg1 == "rax") cpu.regs[Register::RAX] = val;
                else if (arg1 == "rbx") cpu.regs[Register::RBX] = val;
                else if (arg1 == "rcx") cpu.regs[Register::RCX] = val;
                else if (arg1 == "rdx") cpu.regs[Register::RDX] = val;
            } else {
                Register srcReg, dstReg;
                if (arg1 == "rax") dstReg = Register::RAX;
                else if (arg1 == "rbx") dstReg = Register::RBX;
                else if (arg1 == "rcx") dstReg = Register::RCX;
                else if (arg1 == "rdx") dstReg = Register::RDX;
                
                if (arg2 == "rax") srcReg = Register::RAX;
                else if (arg2 == "rbx") srcReg = Register::RBX;
                else if (arg2 == "rcx") srcReg = Register::RCX;
                else if (arg2 == "rdx") srcReg = Register::RDX;
                
                cpu.regs[dstReg] = cpu.regs[srcReg];
            }
        }
        else if (inst == "add") {
            iss >> arg1 >> arg2;
            int64_t val = std::stoll(arg2);
            if (arg1 == "rax") {
                cpu.regs[Register::RAX] += val;
                cpu.updateFlags(cpu.regs[Register::RAX]);
            }
            else if (arg1 == "rbx") {
                cpu.regs[Register::RBX] += val;
                cpu.updateFlags(cpu.regs[Register::RBX]);
            }
        }
        else if (inst == "sub") {
            iss >> arg1 >> arg2;
            int64_t val = std::stoll(arg2);
            if (arg1 == "rax") {
                cpu.regs[Register::RAX] -= val;
                cpu.updateFlags(cpu.regs[Register::RAX]);
            }
        }
        else if (inst == "push") {
            iss >> arg1;
            if (arg1 == "rax") stack.push_back(cpu.regs[Register::RAX]);
            else if (arg1 == "rbx") stack.push_back(cpu.regs[Register::RBX]);
            else if (arg1 == "rcx") stack.push_back(cpu.regs[Register::RCX]);
            else if (arg1 == "rdx") stack.push_back(cpu.regs[Register::RDX]);
        }
        else if (inst == "pop") {
            iss >> arg1;
            if (!stack.empty()) {
                int64_t val = stack.back();
                stack.pop_back();
                if (arg1 == "rax") cpu.regs[Register::RAX] = val;
                else if (arg1 == "rbx") cpu.regs[Register::RBX] = val;
                else if (arg1 == "rcx") cpu.regs[Register::RCX] = val;
                else if (arg1 == "rdx") cpu.regs[Register::RDX] = val;
            }
        }
        else if (inst == "inc") {
            iss >> arg1;
            if (arg1 == "rax") {
                cpu.regs[Register::RAX]++;
                cpu.updateFlags(cpu.regs[Register::RAX]);
            }
        }
        else if (inst == "dec") {
            iss >> arg1;
            if (arg1 == "rax") {
                cpu.regs[Register::RAX]--;
                cpu.updateFlags(cpu.regs[Register::RAX]);
            }
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
            if (!stack.empty()) stack.pop_back();
            break;
        case OpCode::ADD: {
            int64_t b = stack.back(); stack.pop_back();
            int64_t a = stack.back(); stack.pop_back();
            stack.push_back(a + b);
            break;
        }
        case OpCode::SUB: {
            int64_t b = stack.back(); stack.pop_back();
            int64_t a = stack.back(); stack.pop_back();
            stack.push_back(a - b);
            break;
        }
        case OpCode::MUL: {
            int64_t b = stack.back(); stack.pop_back();
            int64_t a = stack.back(); stack.pop_back();
            stack.push_back(a * b);
            break;
        }
        case OpCode::DIV: {
            int64_t b = stack.back(); stack.pop_back();
            int64_t a = stack.back(); stack.pop_back();
            stack.push_back(a / b);
            break;
        }
        case OpCode::STORE:
            globals["_tmp" + std::to_string(inst.operand)] = stack.back();
            stack.pop_back();
            break;
        case OpCode::LOAD:
            stack.push_back(globals["_tmp" + std::to_string(inst.operand)]);
            break;
        case OpCode::PRINT:
            std::cout << stack.back() << std::endl;
            stack.pop_back();
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