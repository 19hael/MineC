#pragma once
#include "vm.h"
#include <string>

class Debugger {
    VM* vm;
    bool active;
    
public:
    Debugger(VM* vmInstance);
    void start();
    void handleCommand(const std::string& cmd);
    void printHelp();
};