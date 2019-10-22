#pragma once

#include "ASTNode.h"

//创建因子的语法树节点
std::tuple<std::shared_ptr<ASTNode>, std::string> createFactorASTNode(std::string input);

//创建项的语法树节点
std::tuple<std::shared_ptr<ASTNode>, std::string> createTermASTNode(std::string input);

//创建表达式的语法树节点
std::tuple<std::shared_ptr<ASTNode>, std::string> createExpressionASTNode(std::string input);

//创建赋值语句的语法树节点
std::tuple<std::shared_ptr<ASTNode>, std::string> createAssignmentASTNode(std::string input);

//创建逻辑判断的语法树节点
std::tuple<std::shared_ptr<ASTNode>, std::string> createLogicASTNode(std::string input);

//创建if语句的语法树
std::tuple<std::shared_ptr<ASTNode>, std::string> createIfASTNode(std::string input);

//创建elseif语句的语法树
std::tuple<std::shared_ptr<ASTNode>, std::string> createElseIfASTNode(std::string input);

//创建定义变量语句的语法树节点
std::tuple<std::shared_ptr<ASTNode>, std::string> createDefVarASTNode(std::string input);

//创建定义过程的语法树节点
std::tuple<std::shared_ptr<ASTNode>, std::string> createDefProcASTNode(std::string input);

//创建语句块的语法树
std::tuple<std::shared_ptr<ASTNode>, std::string> createBlocksASTNode(std::string input);

//创建一般语句的语法树
std::tuple<std::shared_ptr<ASTNode>, std::string> createStatementASTNode(std::string input);
