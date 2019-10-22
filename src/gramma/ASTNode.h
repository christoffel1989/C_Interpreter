#pragma once

#include "token.h"

#include <tuple>
#include <list>
#include <map>
#include <memory>

//前置声明
struct ASTEnvironment;

//语法树节点
struct ASTNode
{
	//节点的词法类型
	Token tk;

	//子节点
	std::list<std::shared_ptr<ASTNode>> childs;
};

//存储AST的环境
//用户自定义变量或函数
//是一个元组
//第一个分量是由输入变量字符串构成的list
//第二个分量是本体
//当输入参量个数为0时 第二分量具体类型是double
//当输入参量个数大于1时 第二分量具体类型是string 存储了表达式
using UserAST = std::tuple<std::list<std::string>, std::variant<double, std::shared_ptr<ASTNode>>>;

//环境表存储了一些定义了的符号 以及 他的父环境指针
struct ASTEnvironment
{
	//环境
	//每一个symbol对应
	std::map<std::string, UserAST> EnvMap;

	//指向父类环境的指针
	ASTEnvironment* parent = nullptr;
};

//将新的变量或函数注册到解释器环境里
void setASTEnvSymbol(std::string symbol, UserAST value, ASTEnvironment* env);

//获得特定名字的函数的实体
std::optional<UserAST> getASTEnvSymbol(std::string symbol, ASTEnvironment* env);