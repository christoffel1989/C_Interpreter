#pragma once

#include "ASTNode.h"

//解释取地址语法节点
auto interpreteRefAST(std::shared_ptr<ASTNode> ast, Environment* env) -> ASTResult;

//解释赋值符号语法节点
auto interpreteAssignAST(std::shared_ptr<ASTNode> ast, Environment* env) -> ASTResult;

//解释由中括号包起来的语句块
auto interpreteBlockAST(std::shared_ptr<ASTNode> ast, Environment* env) -> ASTResult;

//解释if语法节点
auto interpreteIfAST(std::shared_ptr<ASTNode> ast, Environment* env) -> ASTResult;

//解释while语法节点
auto interpreteWhileAST(std::shared_ptr<ASTNode> ast, Environment* env) -> ASTResult;

//解释do while语法节点
auto interpreteDoWhileAST(std::shared_ptr<ASTNode> ast, Environment* env) -> ASTResult;

//解释for语法节点
auto interpreteForAST(std::shared_ptr<ASTNode> ast, Environment* env) -> ASTResult;

//解释定义变量语法节点
auto interpreteDefVarAST(std::shared_ptr<ASTNode> ast, Environment* env) -> ASTResult;

//解释定义指针变量语法节点
auto interpreteDefPointerAST(std::shared_ptr<ASTNode> ast, Environment* env) -> ASTResult;

//解释定义数组变量的语法节点
auto interpreteDefArrayAST(std::shared_ptr<ASTNode> ast, Environment* env) -> ASTResult;

//解释定义函数语法节点
auto interpreteDefProcAST(std::shared_ptr<ASTNode> ast, Environment* env) -> ASTResult;

//解释系统自定义符号语法节点
auto interpretePrimitiveSymboAST(std::shared_ptr<ASTNode> ast, Environment* env) -> ASTResult;

//解释用户自定义语法节点
auto interpreteUserSymbolAST(std::shared_ptr<ASTNode> ast, Environment* env) -> ASTResult;

//解释语法节点(总入口)
auto interpreteAST(std::shared_ptr<ASTNode> ast, Environment* env) -> ASTResult;