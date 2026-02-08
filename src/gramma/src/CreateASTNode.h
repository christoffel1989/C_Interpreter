#pragma once

#include "ASTNode.h"

//创建空语句的语法树节点
auto createNOpASTNode(std::string input) -> std::expected<std::tuple<std::shared_ptr<ASTNode>, std::string>, std::string>;

//创建解引用表达式的语法树节点
auto createDeRefASTNode(std::string input) -> std::expected<std::tuple<std::shared_ptr<ASTNode>, std::string>, std::string>;

//创建在现有表达式基础上添加++或--的后缀
auto createPostIncOrDecASTNode(std::shared_ptr<ASTNode> node, TokenType type) -> std::shared_ptr<ASTNode>;

//创建因子的语法树节点
auto createFactorASTNode(std::string input) -> std::expected<std::tuple<std::shared_ptr<ASTNode>, std::string>, std::string>;

//创建项的语法树节点
auto createTermASTNode(std::string input) -> std::expected<std::tuple<std::shared_ptr<ASTNode>, std::string>, std::string>;

//创建计算式的语法树节点
auto createArithmeticASTNode(std::string input) -> std::expected<std::tuple<std::shared_ptr<ASTNode>, std::string>, std::string>;

//创建关系比较的语法树节点
auto createRelationASTNode(std::string input) -> std::expected<std::tuple<std::shared_ptr<ASTNode>, std::string>, std::string>;

//创建逻辑运算的语法树节点
auto createLogicASTNode(std::string input) -> std::expected<std::tuple<std::shared_ptr<ASTNode>, std::string>, std::string>;

//在变量名节点的基础上创建数组语法节点(即从[开始继续解析)
auto createDefArrayASTNode(std::shared_ptr<ASTNode> node, std::string input) -> std::expected<std::tuple<std::shared_ptr<ASTNode>, std::string>, std::string>;

//创建表达式的语法树节点
auto createExpressionASTNode(std::string input) -> std::expected<std::tuple<std::shared_ptr<ASTNode>, std::string>, std::string>;

//创建语句块的语法树节点
auto createBlockASTNode(std::string input) -> std::expected<std::tuple<std::shared_ptr<ASTNode>, std::string>, std::string>;

//创建if语句的语法树节点
auto createIfASTNode(std::string input) -> std::expected<std::tuple<std::shared_ptr<ASTNode>, std::string>, std::string>;

//创建elseif语句的语法树节点
auto createElseIfASTNode(std::string input) -> std::expected<std::tuple<std::shared_ptr<ASTNode>, std::string>, std::string>;

//创建while语句的语法树节点
auto createWhileASTNode(std::string input) -> std::expected<std::tuple<std::shared_ptr<ASTNode>, std::string>, std::string>;

//创建DoWhile语句的语法树节点
auto createDoWhileASTNode(std::string input) -> std::expected<std::tuple<std::shared_ptr<ASTNode>, std::string>, std::string>;

//创建for语句的语法树节点
auto createForASTNode(std::string input) -> std::expected<std::tuple<std::shared_ptr<ASTNode>, std::string>, std::string>;

//创建break或者continue语句的语法树节点
auto createBreakorContinueASTNode(std::string input) -> std::expected<std::tuple<std::shared_ptr<ASTNode>, std::string>, std::string>;

//创建return语句的语法树节点
auto createReturnASTNode(std::string input) -> std::expected<std::tuple<std::shared_ptr<ASTNode>, std::string>, std::string>;

//创建定义过程的语法树节点
auto createDefProcASTNode(std::string input) -> std::expected<std::tuple<std::shared_ptr<ASTNode>, std::string>, std::string>;

//创建语法树节点(总入口)
auto createStatementASTNode(std::string input) -> std::expected<std::tuple<std::shared_ptr<ASTNode>, std::string>, std::string>;
