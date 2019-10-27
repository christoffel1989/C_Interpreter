#include "ASTNode.h"

//注册新的变量或函数在当前环境
void registEnvSymbol(std::string symbol, UserAST value, Environment* env)
{
	//只在当前环境中设置
	env->address[symbol] = env->top;
	env->memory[env->top] = value;
	//栈顶指针后移一位
	env->top++;
}

//设置用户自定义符号(变量或函数)的值
void setEnvSymbol(std::string symbol, UserAST value, Environment* env)
{
	//查找
	auto iter = env->address.find(symbol);
	//如果存在
	if (iter != env->address.end())
	{
		env->memory[env->address[symbol]] = value;
	}
	//到父亲中设置
	else if (env->parent)
	{
		setEnvSymbol(symbol, value, env->parent);
	}
}

//获得特定名字的函数的实体
std::optional<UserAST> getEnvSymbol(std::string symbol, Environment* env, bool onlycurrent)
{
	//只在当前环境找 不寻找父环境
	if (onlycurrent)
	{
		auto iter = env->address.find(symbol);
		if (iter != env->address.end())
		{
			return { iter->second };
		}
		else
		{
			return {};
		}
	}
	else
	{
		//查找
		auto iter = env->address.find(symbol);
		//如果存在
		if (iter != env->address.end())
		{
			return { env->memory[iter->second] };
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
}