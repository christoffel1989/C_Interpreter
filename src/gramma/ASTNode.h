#pragma once

#include "Token.h"

#include <tuple>
#include <list>
#include <unordered_map>
#include <memory>

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
//第二个分量是符号的本体variant类型
//当类型为double时,表明symbol是变量 存储了他的值
//当类型为std::shared_ptr<ASTNode>时，表明symbol时函数 存储了这个函数的语法树
using UserAST = std::tuple<std::list<std::string>, std::variant<double, std::shared_ptr<ASTNode>>>;

//环境表存储了一些定义了的符号 以及 他的父环境指针
struct ASTEnvironment
{
	//环境
	//每一个symbol对应
	std::unordered_map<std::string, UserAST> EnvMap;

	//指向父类环境的指针
	ASTEnvironment* parent = nullptr;
};

//注册新的变量或函数在当前环境
void registASTEnvSymbol(std::string symbol, UserAST value, ASTEnvironment* env);

//设置用户自定义符号(变量或函数)的值
void setASTEnvSymbol(std::string symbol, UserAST value, ASTEnvironment* env);

//获得特定名字的函数的实体
std::optional<UserAST> getASTEnvSymbol(std::string symbol, ASTEnvironment* env);