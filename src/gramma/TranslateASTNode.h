#pragma once;

#include "ASTNode.h"

//翻译取地址
double translateRefAST(std::shared_ptr<ASTNode> ast, Environment* env);

//指针解引用(作为左值)
double translateLDeRefAST(std::shared_ptr<ASTNode> astL, std::shared_ptr<ASTNode> astR, Environment* env);

//指针解引用(作为右值)
double translateRDeRefAST(std::shared_ptr<ASTNode> ast, Environment* env);

//翻译赋值符号节点
double translateAssignAST(std::shared_ptr<ASTNode> ast, Environment* env);

//翻译由中括号包起来的语句块
double translateBlockAST(std::shared_ptr<ASTNode> ast, Environment* env);

//翻译if语句节点
double translateIfAST(std::shared_ptr<ASTNode> ast, Environment* env);

//翻译while语句节点
double translateWhileAST(std::shared_ptr<ASTNode> ast, Environment* env);

//翻译do while语句节点
double translateDoWhileAST(std::shared_ptr<ASTNode> ast, Environment* env);

//翻译for语句节点
double translateForAST(std::shared_ptr<ASTNode> ast, Environment* env);

//翻译定义变量节点
double translateDefVarAST(std::shared_ptr<ASTNode> ast, Environment* env);

//翻译定义指针变量的节点
double translateDefPointerAST(std::shared_ptr<ASTNode> ast, Environment* env);

//翻译定义函数节点
double translateDefProcAST(std::shared_ptr<ASTNode> ast, Environment* env);

//翻译系统自定义符号节点
double translatePrimitiveSymboAST(std::shared_ptr<ASTNode> ast, Environment* env);

//翻译usersymbol节点
double translateUserSymbolAST(std::shared_ptr<ASTNode> ast, Environment* env);

//执行语法树
double executeAST(std::shared_ptr<ASTNode> ast, Environment* env);