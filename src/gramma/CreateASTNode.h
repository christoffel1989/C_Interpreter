﻿#pragma once

#include "ASTNode.h"

//创建空语句的语法树节点
std::tuple<std::shared_ptr<ASTNode>, std::string> createNOpASTNode(std::string input);

//创建解引用表达式的语法树节点
std::tuple<std::shared_ptr<ASTNode>, std::string> createDeRefASTNode(std::string input);

//创建在现有表达式基础上添加++或--的后缀
std::shared_ptr<ASTNode> createPostIncOrDecASTNode(std::shared_ptr<ASTNode> node, TokenType type);

//创建因子的语法树节点
std::tuple<std::shared_ptr<ASTNode>, std::string> createFactorASTNode(std::string input);

//创建项的语法树节点
std::tuple<std::shared_ptr<ASTNode>, std::string> createTermASTNode(std::string input);

//创建计算式的语法树节点
std::tuple<std::shared_ptr<ASTNode>, std::string> createArithmeticASTNode(std::string input);

//创建关系比较的语法树节点
std::tuple<std::shared_ptr<ASTNode>, std::string> createRelationASTNode(std::string input);

//创建逻辑运算的语法树节点
std::tuple<std::shared_ptr<ASTNode>, std::string> createLogicASTNode(std::string input);

//在变量名节点的基础上创建数组语法节点(即从[开始继续解析)
std::tuple<std::shared_ptr<ASTNode>, std::string> createDefArrayASTNode(std::shared_ptr<ASTNode> node, std::string input);

//创建表达式的语法树节点
std::tuple<std::shared_ptr<ASTNode>, std::string> createExpressionASTNode(std::string input);

//创建语句块的语法树节点
std::tuple<std::shared_ptr<ASTNode>, std::string> createBlockASTNode(std::string input);

//创建if语句的语法树节点
std::tuple<std::shared_ptr<ASTNode>, std::string> createIfASTNode(std::string input);

//创建elseif语句的语法树节点
std::tuple<std::shared_ptr<ASTNode>, std::string> createElseIfASTNode(std::string input);

//创建while语句的语法树节点
std::tuple<std::shared_ptr<ASTNode>, std::string> createWhileASTNode(std::string input);

//创建DoWhile语句的语法树节点
std::tuple<std::shared_ptr<ASTNode>, std::string> createDoWhileASTNode(std::string input);

//创建for语句的语法树节点
std::tuple<std::shared_ptr<ASTNode>, std::string> createForASTNode(std::string input);

//创建break或者continue语句的语法树节点
std::tuple<std::shared_ptr<ASTNode>, std::string> createBreakorContinueASTNode(std::string input);

//创建return语句的语法树节点
std::tuple<std::shared_ptr<ASTNode>, std::string> createReturnASTNode(std::string input);

//创建定义过程的语法树节点
std::tuple<std::shared_ptr<ASTNode>, std::string> createDefProcASTNode(std::string input);

//创建语法树节点(总入口)
std::tuple<std::shared_ptr<ASTNode>, std::string> createStatementASTNode(std::string input);
