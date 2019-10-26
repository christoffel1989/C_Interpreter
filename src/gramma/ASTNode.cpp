#include "ASTNode.h"

#include <iostream>

//注册新的变量或函数在当前环境
void registEnvSymbol(std::string symbol, UserAST value, Environment* env)
{
	//只在当前环境中设置
	env->EnvMap[symbol] = value;
}

//设置用户自定义符号(变量或函数)的值
void setEnvSymbol(std::string symbol, UserAST value, Environment* env)
{
	//查找
	auto iter = env->EnvMap.find(symbol);
	//如果存在
	if (iter != env->EnvMap.end())
	{
		env->EnvMap[symbol] = value;
	}
	//到父亲中设置
	else if (env->parent)
	{
		setEnvSymbol(symbol, value, env->parent);
	}
}

//用户自定义符号是否在当前环境中已经存在（主要用于变量和函数的定义）
bool getEnvSymbolInCurrent(std::string symbol, Environment* env)
{
	//只在当前环境找 不寻找父环境
	return env->EnvMap.find(symbol) != env->EnvMap.end();
}

//获得特定名字的函数的实体
std::optional<UserAST> getEnvSymbol(std::string symbol, Environment* env)
{
	//查找
	auto iter = env->EnvMap.find(symbol);
	//如果存在
	if (iter != env->EnvMap.end())
	{
		return { iter->second };
	}
	//如果存在父环境 则继续查询父环境
	else if (env->parent)
	{
		return getEnvSymbol(symbol, env->parent);
	}
	else
	{
		//不存在时返回无效值
		return {};
	}
}