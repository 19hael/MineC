#include "compiler.h"

#include <stdexcept>

Compiler::Compiler(VM* vmInstance) : vm(vmInstance) {
    reset();
}

void Compiler::reset() {
    code.clear();
    scopes.clear();
    functionStack.clear();
    functionAddresses.clear();
    pendingCalls.clear();
    globalVarCounter = 0;
    scopes.push_back({});
    functionStack.push_back({"__global", false, 0});
}

size_t Compiler::emit(OpCode op, int64_t operand, const std::string& text) {
    code.emplace_back(op, operand, text);
    return code.size() - 1;
}

bool Compiler::inFunction() const {
    return !functionStack.empty() && functionStack.back().isFunction;
}

void Compiler::enterScope() {
    scopes.push_back({});
}

void Compiler::leaveScope() {
    if (!scopes.empty()) {
        scopes.pop_back();
    }
}

VariableInfo* Compiler::resolveVariable(const std::string& name) {
    for (auto it = scopes.rbegin(); it != scopes.rend(); ++it) {
        auto varIt = it->variables.find(name);
        if (varIt != it->variables.end()) {
            return &varIt->second;
        }
    }
    return nullptr;
}

VariableInfo Compiler::declareVariable(const std::string& name) {
    if (scopes.empty()) {
        throw std::runtime_error("Internal compiler error: missing scope");
    }

    if (scopes.back().variables.find(name) != scopes.back().variables.end()) {
        throw std::runtime_error("Variable '" + name + "' already declared in this scope");
    }

    VariableInfo info;
    if (inFunction()) {
        info.isGlobal = false;
        info.index = functionStack.back().nextLocalIndex++;
    } else {
        info.isGlobal = true;
        info.index = globalVarCounter++;
    }

    scopes.back().variables[name] = info;
    return info;
}

void Compiler::storeVariable(const VariableInfo& info) {
    if (info.isGlobal) {
        emit(OpCode::STORE_GLOBAL, info.index);
    } else {
        emit(OpCode::STORE_LOCAL, info.index);
    }
}

void Compiler::loadVariable(const VariableInfo& info) {
    if (info.isGlobal) {
        emit(OpCode::LOAD_GLOBAL, info.index);
    } else {
        emit(OpCode::LOAD_LOCAL, info.index);
    }
}

void Compiler::patchFunctionCalls(const std::string& name, size_t address) {
    for (auto it = pendingCalls.begin(); it != pendingCalls.end();) {
        if (it->second == name) {
            code[it->first].operand = static_cast<int64_t>(address);
            it = pendingCalls.erase(it);
        } else {
            ++it;
        }
    }
}

void Compiler::compileNode(ASTPtr node) {
    if (!node) return;

    switch (node->type) {
        case ASTType::PROGRAM:
            for (auto& child : node->children) {
                compileNode(child);
            }
            break;
        case ASTType::VAR_DECL:
            compileVarDecl(node);
            break;
        case ASTType::FUNC_DECL:
            compileFunction(node);
            break;
        case ASTType::BLOCK:
            compileBlock(node);
            break;
        case ASTType::PRINT:
            compileExpr(node->children[0]);
            emit(OpCode::PRINT);
            break;
        case ASTType::ASM_BLOCK:
            compileAsm(node);
            break;
        case ASTType::IF_STMT:
            compileIf(node);
            break;
        case ASTType::WHILE_STMT:
            compileWhile(node);
            break;
        case ASTType::RETURN:
            compileReturn(node);
            break;
        case ASTType::EXPR_STMT:
            compileExprStmt(node);
            break;
        default:
            throw std::runtime_error("Unsupported AST node in statement context");
    }
}

void Compiler::compileBlock(ASTPtr node) {
    enterScope();
    for (auto& child : node->children) {
        compileNode(child);
    }
    leaveScope();
}

void Compiler::compileVarDecl(ASTPtr node) {
    VariableInfo info = declareVariable(node->value);
    compileExpr(node->children[0]);
    storeVariable(info);
}

void Compiler::compileFunction(ASTPtr node) {
    std::string name = node->value;
    size_t skipIndex = emit(OpCode::JMP, 0);

    size_t entryPoint = code.size();
    functionAddresses[name] = entryPoint;
    patchFunctionCalls(name, entryPoint);

    functionStack.push_back({name, true, 0});
    compileBlock(node->children[0]);
    emit(OpCode::PUSH, 0);
    emit(OpCode::RET);
    functionStack.pop_back();

    size_t afterFunction = code.size();
    code[skipIndex].operand = static_cast<int64_t>(afterFunction);
}

void Compiler::compileIf(ASTPtr node) {
    compileExpr(node->children[0]);
    size_t jumpFalse = emit(OpCode::JMP_IF_FALSE, 0);
    compileBlock(node->children[1]);

    if (node->children.size() == 3) {
        size_t jumpEnd = emit(OpCode::JMP, 0);
        code[jumpFalse].operand = static_cast<int64_t>(code.size());
        compileBlock(node->children[2]);
        code[jumpEnd].operand = static_cast<int64_t>(code.size());
    } else {
        code[jumpFalse].operand = static_cast<int64_t>(code.size());
    }
}

void Compiler::compileWhile(ASTPtr node) {
    size_t loopStart = code.size();
    compileExpr(node->children[0]);
    size_t exitJump = emit(OpCode::JMP_IF_FALSE, 0);
    compileBlock(node->children[1]);
    emit(OpCode::JMP, static_cast<int64_t>(loopStart));
    code[exitJump].operand = static_cast<int64_t>(code.size());
}

void Compiler::compileReturn(ASTPtr node) {
    if (!inFunction()) {
        throw std::runtime_error("Return statement outside of function");
    }

    if (!node->children.empty()) {
        compileExpr(node->children[0]);
    } else {
        emit(OpCode::PUSH, 0);
    }
    emit(OpCode::RET);
}

void Compiler::compileAsm(ASTPtr node) {
    emit(OpCode::EXEC_ASM, 0, node->value);
}

void Compiler::compileExprStmt(ASTPtr node) {
    compileExpr(node->children[0]);
    emit(OpCode::POP);
}

void Compiler::compileExpr(ASTPtr node) {
    switch (node->type) {
        case ASTType::NUMBER:
            emit(OpCode::PUSH, std::stoll(node->value));
            break;
        case ASTType::IDENTIFIER: {
            VariableInfo* info = resolveVariable(node->value);
            if (!info) {
                throw std::runtime_error("Undefined variable '" + node->value + "'");
            }
            loadVariable(*info);
            break;
        }
        case ASTType::CALL:
            compileCall(node);
            break;
        case ASTType::BINARY_OP:
            compileBinaryOp(node);
            break;
        case ASTType::ASSIGN:
            compileAssignment(node);
            break;
        default:
            throw std::runtime_error("Unsupported expression");
    }
}

void Compiler::compileAssignment(ASTPtr node) {
    VariableInfo* info = resolveVariable(node->value);
    if (!info) {
        throw std::runtime_error("Undefined variable '" + node->value + "'");
    }
    compileExpr(node->children[0]);
    storeVariable(*info);
    loadVariable(*info);
}

void Compiler::compileBinaryOp(ASTPtr node) {
    compileExpr(node->children[0]);
    compileExpr(node->children[1]);

    const std::string& op = node->value;
    if (op == "+") emit(OpCode::ADD);
    else if (op == "-") emit(OpCode::SUB);
    else if (op == "*") emit(OpCode::MUL);
    else if (op == "/") emit(OpCode::DIV);
    else if (op == "==") emit(OpCode::CMP_EQ);
    else if (op == "!=") emit(OpCode::CMP_NEQ);
    else if (op == "<") emit(OpCode::CMP_LT);
    else if (op == "<=") emit(OpCode::CMP_LEQ);
    else if (op == ">") emit(OpCode::CMP_GT);
    else if (op == ">=") emit(OpCode::CMP_GEQ);
    else throw std::runtime_error("Unsupported binary operator '" + op + "'");
}

void Compiler::compileCall(ASTPtr node) {
    if (!node->children.empty()) {
        throw std::runtime_error("Function arguments are not supported yet");
    }

    auto it = functionAddresses.find(node->value);
    size_t index;
    if (it != functionAddresses.end()) {
        index = emit(OpCode::CALL, static_cast<int64_t>(it->second));
    } else {
        index = emit(OpCode::CALL, 0);
        pendingCalls.emplace_back(index, node->value);
    }
    (void)index;
}

std::vector<Instruction> Compiler::compile(ASTPtr ast) {
    reset();
    compileNode(ast);

    auto mainIt = functionAddresses.find("main");
    if (mainIt == functionAddresses.end()) {
        throw std::runtime_error("Entry point 'main' was not defined");
    }

    emit(OpCode::CALL, static_cast<int64_t>(mainIt->second));
    emit(OpCode::HALT);

    if (!pendingCalls.empty()) {
        throw std::runtime_error("Unresolved function call to '" + pendingCalls.front().second + "'");
    }

    return code;
}
