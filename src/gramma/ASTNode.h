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

//AST执行的环境
//用户自定义变量或函数 本体是variant
//当类型为double时,表明symbol是变量 存储了他的值
//当类型为std::tuple<std::list<std::string>, std::shared_ptr<ASTNode>>时，表明symbol时函数
//其第一个分量是函数的输入变量字符串构成的list 第二个分量是这个函数的语法树
using UserAST = std::variant<double, std::tuple<std::list<std::string>, std::shared_ptr<ASTNode>>>;

//环境表存储了一些定义了的符号 以及 他的父环境指针
struct Environment
{
	//环境
	//每一个symbol对应
	std::unordered_map<std::string, UserAST> EnvMap;

	//指向父类环境的指针
	Environment* parent = nullptr;
};

//注册新的变量或函数在当前环境
void registEnvSymbol(std::string symbol, UserAST value, Environment* env);

//设置用户自定义符号(变量或函数)的值
void setEnvSymbol(std::string symbol, UserAST value, Environment* env);

//用户自定义符号是否在当前环境中已经存在（主要用于变量和函数的定义）
bool getEnvSymbolInCurrent(std::string symbol, Environment* env);

//获得特定名字的函数的实体
std::optional<UserAST> getEnvSymbol(std::string symbol, Environment* env);