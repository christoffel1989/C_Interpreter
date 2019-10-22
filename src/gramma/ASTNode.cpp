#include "ASTNode.h"

#include <iostream>

//将新的变量或函数注册到解释器环境里
void setASTEnvSymbol(std::string symbol, UserAST value, ASTEnvironment* env)
{
	//先不搞太复杂的
	//就更新在当前环境 不考虑父环境
	//更新
	env->EnvMap[symbol] = value;
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