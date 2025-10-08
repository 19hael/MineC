#include "debugger.h"
#include <iostream>
#include <sstream>

Debugger::Debugger(VM* vmInstance) : vm(vmInstance), active(false) {}

void Debugger::printHelp() {
    std::cout << "\n=== MicroC Debugger Commands ===" << std::endl;
    std::cout << "step (s)    - Execute one instruction" << std::endl;
    std::cout << "continue (c) - Run until breakpoint" << std::endl;
    std::cout << "regs (r)    - Show CPU registers" << std::endl;
    std::cout << "stack       - Show stack contents" << std::endl;
    std::cout << "quit (q)    - Exit debugger" << std::endl;
    std::cout << "help (h)    - Show this help" << std::endl;
}

void Debugger::handleCommand(const std::string& cmd) {
    if (cmd == "s" || cmd == "step") {
        vm->step();
    }
    else if (cmd == "c" || cmd == "continue") {
        vm->run();
    }
    else if (cmd == "r" || cmd == "regs") {
        vm->printState();
    }
    else if (cmd == "q" || cmd == "quit") {
        active = false;
    }
    else if (cmd == "h" || cmd == "help") {
        printHelp();
    }
    else {
        std::cout << "Unknown command. Type 'help' for commands." << std::endl;
    }
}

void Debugger::start() {
    active = true;
    vm->setStepMode(true);
    printHelp();
    
    std::string line;
    while (active) {
        std::cout << "\n(microc-db) ";
        std::getline(std::cin, line);
        if (!line.empty()) {
            handleCommand(line);
        }
    }
}