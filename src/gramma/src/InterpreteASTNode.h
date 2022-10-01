#pragma once

#include "ASTNode.h"

//解释取地址语法节点
double interpreteRefAST(std::shared_ptr<ASTNode> ast, Environment* env);

//解释赋值符号语法节点
double interpreteAssignAST(std::shared_ptr<ASTNode> ast, Environment* env);

//解释由中括号包起来的语句块
double interpreteBlockAST(std::shared_ptr<ASTNode> ast, Environment* env);

//解释if语法节点
double interpreteIfAST(std::shared_ptr<ASTNode> ast, Environment* env);

//解释while语法节点
double interpreteWhileAST(std::shared_ptr<ASTNode> ast, Environment* env);

//解释do while语法节点
double interpreteDoWhileAST(std::shared_ptr<ASTNode> ast, Environment* env);

//解释for语法节点
double interpreteForAST(std::shared_ptr<ASTNode> ast, Environment* env);

//解释定义变量语法节点
double interpreteDefVarAST(std::shared_ptr<ASTNode> ast, Environment* env);

//解释定义指针变量语法节点
double interpreteDefPointerAST(std::shared_ptr<ASTNode> ast, Environment* env);

//解释定义数组变量的语法节点
double interpreteDefArrayAST(std::shared_ptr<ASTNode> ast, Environment* env);

//解释定义函数语法节点
double interpreteDefProcAST(std::shared_ptr<ASTNode> ast, Environment* env);

//解释系统自定义符号语法节点
double interpretePrimitiveSymboAST(std::shared_ptr<ASTNode> ast, Environment* env);

//解释用户自定义语法节点
double interpreteUserSymbolAST(std::shared_ptr<ASTNode> ast, Environment* env);

//解释语法节点(总入口)
double interpreteAST(std::shared_ptr<ASTNode> ast, Environment* env);