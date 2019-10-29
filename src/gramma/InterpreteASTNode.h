#pragma once;

#include "ASTNode.h"

//翻译取地址语法节点
double interpreteRefAST(std::shared_ptr<ASTNode> ast, Environment* env);

//翻译赋值符号语法节点
double interpreteAssignAST(std::shared_ptr<ASTNode> ast, Environment* env);

//翻译由中括号包起来的语句块
double interpreteBlockAST(std::shared_ptr<ASTNode> ast, Environment* env);

//翻译if语法节点
double interpreteIfAST(std::shared_ptr<ASTNode> ast, Environment* env);

//翻译while语法节点
double interpreteWhileAST(std::shared_ptr<ASTNode> ast, Environment* env);

//翻译do while语法节点
double interpreteDoWhileAST(std::shared_ptr<ASTNode> ast, Environment* env);

//翻译for语法节点
double interpreteForAST(std::shared_ptr<ASTNode> ast, Environment* env);

//翻译定义变量语法节点
double interpreteDefVarAST(std::shared_ptr<ASTNode> ast, Environment* env);

//翻译定义指针变量语法节点
double interpreteDefPointerAST(std::shared_ptr<ASTNode> ast, Environment* env);

//翻译定义函数语法节点
double interpreteDefProcAST(std::shared_ptr<ASTNode> ast, Environment* env);

//翻译系统自定义符号语法节点
double interpretePrimitiveSymboAST(std::shared_ptr<ASTNode> ast, Environment* env);

//翻译用户自定义语法节点
double interpreteUserSymbolAST(std::shared_ptr<ASTNode> ast, Environment* env);

//翻译语法节点(总入口)
double interpreteAST(std::shared_ptr<ASTNode> ast, Environment* env);