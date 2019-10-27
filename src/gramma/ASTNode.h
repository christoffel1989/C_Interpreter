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

//栈空间定义
//地址类型
using VarAddress = unsigned int;

//栈空间的最大容量(目前设置为4k与实际windows的栈空间大小相同)
inline const VarAddress MaxStackLen = 4096;

//获得栈空间栈顶地址
VarAddress getStackTop();

//设置栈空间栈顶地址
void setStackTop(VarAddress top);

//环境表存储了一些定义了虚拟内存、符号与虚拟内存地址的映射哈希表、以及他的父环境指针
struct Environment
{
	//构造函数(初始化起始位置为当前栈顶)
	Environment() : start(getStackTop()) {}

	//析构函数(栈顶设置回环境的起始位置)
	~Environment() { setStackTop(start); }

	//每一个symbol对应的地址表
	std::unordered_map<std::string, VarAddress> address;

	//环境中变量在栈空间中的起始位置
	VarAddress start;

	//指向父类环境的指针
	Environment* parent = nullptr;
};

//用户自定义变量或函数或指针在解释器虚拟内存空间中的存储类型 为一个可变类型(即variant)
//当类型为double时,表明该处内存对应一个变量,存储了他的值
//当类型为std::tuple<std::list<std::string>, std::shared_ptr<ASTNode>>时，表明该处内存对应一个函数
//其第一个分量是函数的输入变量字符串构成的list 第二个分量是这个函数的语法树
//当类型为VarAddress时,表明该处内存对应一个指针,存储的是地址
using UserAST = std::variant<double, std::tuple<std::list<std::string>, std::shared_ptr<ASTNode>>, VarAddress>;

//注册新的变量或函数在当前环境
void registEnvSymbol(std::string symbol, UserAST value, Environment* env);

//设置用户自定义符号(变量或函数)的值
void setEnvSymbol(std::string symbol, UserAST value, Environment* env);

//通过地址设置直接设置用户自定义符号(变量或函数)的值
void setEnvSymbol(UserAST value, VarAddress addr);

//获得用户自定义符号(变量或函数)的值 当onlycurrent为true时只在当前环境搜索
std::optional<UserAST> getEnvSymbol(std::string symbol, Environment* env, bool onlycurrent = false);

//通过地直接获得用户自定义符号(变量或函数)的值
std::optional<UserAST> getEnvSymbol(VarAddress addr);

//获得制定环境中用户自定义符号的地址
std::optional<VarAddress> getEnvSymbolAddr(std::string symbol, Environment* env);