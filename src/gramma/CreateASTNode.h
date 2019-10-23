﻿#pragma once

#include "ASTNode.h"

//创建空语句
std::tuple<std::shared_ptr<ASTNode>, std::string> createNOpASTNode(std::string input);

//创建因子的语法树节点
std::tuple<std::shared_ptr<ASTNode>, std::string> createFactorASTNode(std::string input);

//创建项的语法树节点
std::tuple<std::shared_ptr<ASTNode>, std::string> createTermASTNode(std::string input);

//创建计算式的语法树节点
std::tuple<std::shared_ptr<ASTNode>, std::string> createArithmeticASTNode(std::string input);

//创建关系比较的语法树节点
std::tuple<std::shared_ptr<ASTNode>, std::string> createRelationASTNode(std::string input);

//创建表达式的语法树节点
std::tuple<std::shared_ptr<ASTNode>, std::string> createExpressionASTNode(std::string input);

//创建赋值语句的语法树节点
std::tuple<std::shared_ptr<ASTNode>, std::string> createAssignmentASTNode(std::string input);

//创建if语句的语法树
std::tuple<std::shared_ptr<ASTNode>, std::string> createIfASTNode(std::string input);

//创建elseif语句的语法树
std::tuple<std::shared_ptr<ASTNode>, std::string> createElseIfASTNode(std::string input);

//创建while语句的语法树
std::tuple<std::shared_ptr<ASTNode>, std::string> createWhileASTNode(std::string input);

//创建for语句的语法树
std::tuple<std::shared_ptr<ASTNode>, std::string> createForASTNode(std::string input);

//创建定义变量语句的语法树节点
std::tuple<std::shared_ptr<ASTNode>, std::string> createDefVarASTNode(std::string input);

//创建定义过程的语法树节点
std::tuple<std::shared_ptr<ASTNode>, std::string> createDefProcASTNode(std::string input);

//创建语句块的语法树
std::tuple<std::shared_ptr<ASTNode>, std::string> createBlockASTNode(std::string input);

//创建一般语句的语法树
std::tuple<std::shared_ptr<ASTNode>, std::string> createStatementASTNode(std::string input);
