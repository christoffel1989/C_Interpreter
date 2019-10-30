#include "ASTNode.h"

#include <vector>

//解释器栈空间(变量和函数共享)
static std::vector<UserAST> StackMemory(MaxStackLen);

//栈顶起始为0
static VarAddress StackTop = 0;

//获得栈空间栈顶地址
VarAddress getStackTop()
{
	return StackTop;
}

//设置栈空间栈顶地址
void setStackTop(VarAddress top)
{
	//栈空间最大容量之内才能设置
	if (top < MaxStackLen)
	{
		StackTop = top;
	}
}

//注册新的变量或函数在当前环境
void registEnvSymbol(std::string symbol, UserAST value, Environment* env)
{
	//只在当前环境中设置
	auto top = getStackTop();
	env->address[symbol] = top;
	StackMemory[top] = value;
	//栈顶向上移动1
	setStackTop(top + 1);
}

//注册新的数组类型变量到当前环境 变量本身存储了数组首地址的指针
void registEnvSymbol(std::string symbol, std::vector<UserAST> values, Environment* env)
{
	auto top = getStackTop();
	//先存储元素
	for (const auto& value : values)
	{
		//当前栈顶设置新的值 设置完以后上移栈顶
		StackMemory[top++] = value;
	}
	//最后再存储symbol指针
	StackMemory[top] = getStackTop();
	env->address[symbol] = top;
	//设置新的栈顶
	setStackTop(top + 1);
}

//设置用户自定义符号(变量或函数)的值
void setEnvSymbol(std::string symbol, UserAST value, Environment* env)
{
	//查找
	auto iter = env->address.find(symbol);
	//如果存在
	if (iter != env->address.end())
	{
		StackMemory[env->address[symbol]] = value;
	}
	//到父亲中设置
	else if (env->parent)
	{
		setEnvSymbol(symbol, value, env->parent);
	}
}

//通过地址设置直接设置用户自定义符号(变量或函数)的值
bool setEnvSymbol(UserAST value, VarAddress addr)
{
	//如果在当前栈顶范围之内
	if (addr <= getStackTop())
	{
		//将地址addr处的值更新为value
		StackMemory[addr] = value;
		return true;
	}
	return false;
}

//获得特定名字的函数的实体
std::optional<UserAST> getEnvSymbol(std::string symbol, Environment* env, bool onlycurrent)
{
	//查找
	auto iter = env->address.find(symbol);
	//如果存在
	if (iter != env->address.end())
	{
		return StackMemory[iter->second];
	}
	//如果允许往更上一级找并且存在父环境 则继续查询父环境
	else if (!onlycurrent && env->parent)
	{
		return getEnvSymbol(symbol, env->parent);
	}
	else
	{
		//不存在时返回无效值
		return {};
	}
}

//通过地直接获得用户自定义符号(变量或函数)的值
std::optional<UserAST> getEnvSymbol(VarAddress addr)
{
	//如果在当前栈顶范围之内
	if (addr <= getStackTop())
	{
		//返回地址addr处的值
		return { StackMemory[addr] };
	}
	//栈顶范围之外返回无效值
	else
	{
		return {};
	}
}

//获得制定环境中用户自定义符号的地址
std::optional<VarAddress> getEnvSymbolAddr(std::string symbol, Environment* env)
{
	//查找
	auto iter = env->address.find(symbol);
	//如果存在
	if (iter != env->address.end())
	{
		return { iter->second };
	}
	//如果存在父环境 则继续查询父环境
	else if (!env->parent)
	{
		return getEnvSymbolAddr(symbol, env->parent);
	}
	else
	{
		//不存在时返回无效值
		return {};
	}
}