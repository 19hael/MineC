#pragma once
#include "ast.h"
#include "token.h"
#include "vm.h"
#include <vector>
#include <map>
#include <string>

struct VariableInfo {
    bool isGlobal;
    int index;
};

struct ScopeFrame {
    std::map<std::string, VariableInfo> variables;
};

struct FunctionContext {
    std::string name;
    bool isFunction;
    int nextLocalIndex;
};

class Compiler {
    std::vector<Instruction> code;
    VM* vm;
    std::vector<ScopeFrame> scopes;
    std::vector<FunctionContext> functionStack;
    int globalVarCounter;
    std::map<std::string, size_t> functionAddresses;
    std::vector<std::pair<size_t, std::string>> pendingCalls;

    void reset();
    void compileNode(ASTPtr node);
    void compileBlock(ASTPtr node);
    void compileFunction(ASTPtr node);
    void compileVarDecl(ASTPtr node);
    void compileIf(ASTPtr node);
    void compileWhile(ASTPtr node);
    void compileReturn(ASTPtr node);
    void compileAsm(ASTPtr node);
    void compileExprStmt(ASTPtr node);
    void compileExpr(ASTPtr node);
    void compileAssignment(ASTPtr node);
    void compileBinaryOp(ASTPtr node);
    void compileCall(ASTPtr node);
    void storeVariable(const VariableInfo& info);
    void loadVariable(const VariableInfo& info);
    VariableInfo declareVariable(const std::string& name);
    VariableInfo* resolveVariable(const std::string& name);
    void enterScope();
    void leaveScope();
    bool inFunction() const;
    void patchFunctionCalls(const std::string& name, size_t address);
    size_t emit(OpCode op, int64_t operand = 0, const std::string& text = "");

public:
    Compiler(VM* vmInstance);
    std::vector<Instruction> compile(ASTPtr ast);
};