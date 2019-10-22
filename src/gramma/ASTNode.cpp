#include "ASTNode.h"

#include <iostream>

//注册新的变量或函数在当前环境
void registASTEnvSymbol(std::string symbol, UserAST value, ASTEnvironment* env)
{
	//只在当前环境中设置
	env->EnvMap[symbol] = value;
}

//设置用户自定义符号(变量或函数)的值
void setASTEnvSymbol(std::string symbol, UserAST value, ASTEnvironment* env)
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
		setASTEnvSymbol(symbol, value, env->parent);
	}
}

//获得特定名字的函数的实体
std::optional<UserAST> getASTEnvSymbol(std::string symbol, ASTEnvironment* env)
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
		return getASTEnvSymbol(symbol, env->parent);
	}
	else
	{
		//不存在时返回无效值
		return {};
	}
}