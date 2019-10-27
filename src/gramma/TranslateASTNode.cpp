#include "TranslateASTNode.h"

#include <cmath>
#include <unordered_map>
#include <stdexcept>

#include <functional>

//翻译一元运算节点的模板
template<typename OP>
double translateOp1AST(std::shared_ptr<ASTNode> ast, Environment* env, OP op)
{
	double result;
	auto iter = ast->childs.begin();
	//获得变量名字
	auto symbol = std::get<std::string>((*iter)->tk.value);
	if (auto v = getEnvSymbol(symbol, env))
	{
		if (std::holds_alternative<double>(v.value()))
		{
			//计算变量值
			result = op(std::get<double>(v.value()), executeAST(*(++iter), env));
			//更新变量在环境中的值
			setEnvSymbol(symbol, result, env);
		}
		//变量类型错误
		else
		{
			throw std::runtime_error("error(assignment): the symbol is not variable!\n");
		}
	}
	//如果查不到这个变量则报错
	else
	{
		throw std::runtime_error("error(assignment): undefine symbol!\n");
	}

	return result;
}

//需要特殊处理的二元运算符号对应的函数
double astdiv(double a, double b)
{
	//除数不能为0
	if (b == 0)
	{
		throw std::runtime_error("error(bad syntax): divided by zero!\n");
	}
	return a / b;
}
double astpow(double a, double b)
{
	//当底为0 幂为非正实数时幂操作无效
	if (a == 0 && b <= 0)
	{
		throw std::runtime_error("error(arithmatic): zero can not be power by non-positive value!\n");
	}
	return pow(a, b);
}
double astmod(double a, double b)
{
	int ia = (int)a;
	if (ia != a)
	{
		throw std::runtime_error("error(arithmatic): non-integral number can not be modded!\n");
	}
	int ib = (int)b;
	if (ib != b)
	{
		throw std::runtime_error("error(arithmatic): non-integral number can not be used to modded!\n");
	}
	return ia % ib;
}

//翻译二元运算节点的模板
template<typename OP, typename PRED>
double translateOp2AST(std::shared_ptr<ASTNode> ast, Environment* env, OP op, PRED pred)
{
	auto iter = ast->childs.begin();
	if (ast->childs.size() == 2)
	{
		double val1 = executeAST(*iter, env);
		if (pred(val1)) return val1;
		double val2 = executeAST(*(++iter), env);
		return op(val1, val2);
	}
	else
	{
		auto val = executeAST(*iter, env);
		//结果取负
		if (ast->tk.type == TokenType::Minus)
		{
			val = -val;
		}
		return val;
	}
}

//翻译自增自运算节点的模板
template<bool post, typename OP>
double translateIncrementAST(std::shared_ptr<ASTNode> ast, Environment* env, OP op)
{
	double prev, after;
	//获得符号名字
	auto symbol = std::get<std::string>((*(ast->childs.begin()))->tk.value);
	if (auto v = getEnvSymbol(symbol, env))
	{
		if (std::holds_alternative<double>(v.value()))
		{
			//先赋值
			prev = std::get<double>(v.value());
			after = op(prev);
			//再加1更新变量在环境中的值
			setEnvSymbol(symbol, after, env);
		}
		//变量类型错误
		else
		{
			throw std::runtime_error("error(++ or --): the symbol is not variable!\n");
		}
	}
	//如果查不到这个变量则报错
	else
	{
		throw std::runtime_error("error(++ or --): undefine symbol!\n");
	}

	return post ? prev : after;
}

//翻译赋值符号节点
double translateAssignAST(std::shared_ptr<ASTNode> ast, Environment* env)
{
	auto iter = ast->childs.begin();
	//获得变量名字
	auto symbol = std::get<std::string>((*iter)->tk.value);
	//如果查不到这个变量则报错
	if (!getEnvSymbol(symbol, env))
	{
		throw std::runtime_error("error(assignment): undefine symbol!\n");
	}
	iter++;
	//计算变量定义式值
	auto result = executeAST(*iter, env);
	//更新变量在环境中的值
	setEnvSymbol(symbol, result, env);

	return result;
}

double translateBlockAST(std::shared_ptr<ASTNode> ast, Environment* env)
{
	auto& childs = ast->childs;
	//构造一个调用函数新的环境
	Environment subenv;
	//他的父亲时env
	subenv.parent = env;
	double result = 0;
	for (auto iter = childs.begin(); iter != childs.end(); iter++)
	{
		//逐行执行代码(在新环境中)
		result = executeAST(*iter, &subenv);
	}
	return result;
}

//翻译if语句节点
double translateIfAST(std::shared_ptr<ASTNode> ast, Environment* env)
{
	//构造一个调用函数新的环境
	Environment subenv;
	//他的父亲时env
	subenv.parent = env;

	//先计算条件
	auto iter = ast->childs.begin();
	double result = 0;
	double condition = executeAST(*iter, &subenv);
	if (condition != 0)
	{
		//执行if的语句块
		result = executeAST(*(++iter), &subenv);
	}
	else
	{
		//执行else的语句块
		++iter; ++iter;
		//可能不存再else分支所以要判断一下
		if (iter != ast->childs.end())
		{
			result = executeAST(*iter, &subenv);
		}
	}
	return result;
}

//continue跳转抛出的异常
struct ContinueState {};
//break跳转抛出的异常
struct BreakState {};

//翻译while语句节点
double translateWhileAST(std::shared_ptr<ASTNode> ast, Environment* env)
{
	//构造一个调用函数新的环境
	Environment subenv;
	//他的父亲时env
	subenv.parent = env;

	//获得循环条件
	auto iter = ast->childs.begin();
	auto condition = *iter;
	//获得循环体
	auto body = *(++iter);
	
	double result = 0;
	while (executeAST(condition, &subenv) != 0)
	{
		try
		{
			//执行循环体
			result = executeAST(body, &subenv);
		}
		//continue
		catch (ContinueState)
		{
			continue;
		}
		//break
		catch (BreakState)
		{
			break;
		}
		//错误代码 继续抛出
		catch (std::exception& e)
		{
			throw e;
		}
	}

	return result;
}

//翻译do while语句节点
double translateDoWhileAST(std::shared_ptr<ASTNode> ast, Environment* env)
{
	//构造一个调用函数新的环境
	Environment subenv;
	//他的父亲时env
	subenv.parent = env;

	//获得循环体
	auto iter = ast->childs.begin();
	auto body = *iter;
	//获得条件
	auto condition = *(++iter);

	double result = 0;
	do
	{
		try
		{
			//执行循环体
			result = executeAST(body, &subenv);
		}
		//continue
		catch (ContinueState)
		{
			continue;
		}
		//break
		catch (BreakState)
		{
			break;
		}
		//错误代码 继续抛出
		catch (std::exception& e)
		{
			throw e;
		}
	} while (executeAST(condition, &subenv) != 0);

	return result;
}

//翻译for语句节点
double translateForAST(std::shared_ptr<ASTNode> ast, Environment* env)
{
	//构造一个调用函数新的环境
	Environment subenv;
	//他的父亲时env
	subenv.parent = env;

	//获得循环起始
	auto iter = ast->childs.begin();
	auto start = *iter;
	//获得终止条件
	auto end = *(++iter);
	//获得步进
	auto increment = *(++iter);
	//获得循环体
	auto body = *(++iter);

	double result = 0;
	for (executeAST(start, &subenv); executeAST(end, &subenv) != 0; executeAST(increment, &subenv))
	{
		try
		{
			//执行循环体
			result = executeAST(body, &subenv);
		}
		//continue
		catch (ContinueState)
		{
			continue;
		}
		//break
		catch (BreakState)
		{
			break;
		}
		//错误代码 继续抛出
		catch (std::exception& e)
		{
			throw e;
		}
	}

	return result;
}

//翻译定义变量节点
double translateDefVarAST(std::shared_ptr<ASTNode> ast, Environment* env)
{
	//自定义变量
	auto iter = ast->childs.begin();
	//获得变量名字
	auto symbol = std::get<std::string>((*iter)->tk.value);
	//如果在当前环境中已经定义则也报错
	if (getEnvSymbolInCurrent(symbol, env))
	{
		throw std::runtime_error("error(Def var): " + symbol + " redefined!\n");
	}
	//计算变量定义式值
	auto result = executeAST(*(++iter), env);
	//注册变量至环境当中
	registEnvSymbol(symbol, result, env);
	
	return result;
}

//翻译定义函数节点
double translateDefProcAST(std::shared_ptr<ASTNode> ast, Environment* env)
{
	auto& childs = ast->childs;
	auto iter = childs.begin();
	//获得函数本体
	auto body = *iter;
	iter++;
	//获得函数体的名字
	auto symbol = std::get<std::string>((*iter)->tk.value);
	//如果在当前环境中已经定义则也报错
	if (getEnvSymbolInCurrent(symbol, env))
	{
		throw std::runtime_error("error(Def proc): " + symbol + " redefined!\n");
	}
	//获得各个参数的名字
	std::list<std::string> args;
	iter++;
	for (; iter != childs.end(); iter++)
	{
		args.push_back(std::get<std::string>((*iter)->tk.value));
	}
	//注册函数至环境当中
	registEnvSymbol(symbol, std::make_tuple(args, body), env);

	//函数定义返回值设置为0
	return 0;
}

//翻译系统自定义符号节点
double translatePrimitiveSymboAST(std::shared_ptr<ASTNode> ast, Environment* env)
{
	auto tk = ast->tk;
	auto& childs = ast->childs;
	double result;

	auto symbol = std::get<std::string>(tk.value);
	auto primitive = getPrimitiveSymbol(symbol).value();
	//函数
	if (std::holds_alternative<std::function<double(double)>>(primitive))
	{
		//获得函数体
		auto fun = std::get<std::function<double(double)>>(primitive);
		//获得函数输入参数 只有单输入参数
		auto arg = executeAST(*childs.begin(), env);
		//执行函数
		result = fun(arg);
	}
	//常量
	else
	{
		result = std::get<double>(primitive);
	}

	return result;
}

//翻译usersymbol节点
double translateUserSymbolAST(std::shared_ptr<ASTNode> ast, Environment* env)
{
	auto tk = ast->tk;
	auto& childs = ast->childs;
	double result;

	//获得符号名字
	auto symbol = std::get<std::string>(tk.value);
	//如果已经定义了
	if (auto val = getEnvSymbol(symbol, env))
	{
		//如果body是值类型则说明为变量
		if (std::holds_alternative<double>(val.value()))
		{
			result = std::get<double>(val.value());
		}
		//函数类型
		else
		{
			//解包参数
			auto[paras, body] = std::get<std::tuple<std::list<std::string>, std::shared_ptr<ASTNode>>>(val.value());

			//构造一个调用函数新的环境
			Environment subenv;
			//他的父亲时env
			subenv.parent = env;

			//如果形参和实参数量不匹配则报错
			if (paras.size() != childs.size())
			{
				//报一个错误
				throw std::runtime_error("error(bad syntax): mismatch argument counts for function call!\n");
			}

			//求各个函数输入参数的值
			auto iterast = childs.begin();
			for (auto iterpara = paras.begin(); iterpara != paras.end(); iterpara++)
			{
				//求第i个实参
				//如果第i个实参是用户自定义符号
				if ((*iterast)->tk.type == TokenType::UserSymbol)
				{
					if (auto v = getEnvSymbol(std::get<std::string>((*iterast)->tk.value), env))
					{
						//如果是数值变量
						if (std::holds_alternative<double>(v.value()))
						{
							//获得实参数值
							result = std::get<double>(v.value());
							//注册第i个实参至subenv中
							registEnvSymbol(*iterpara, result, &subenv);
						}
						//如果是函数变量
						else
						{
							//将这个函数注册到调用的环境中
							registEnvSymbol(*iterpara, std::get<std::tuple<std::list<std::string>, std::shared_ptr<ASTNode>>>(v.value()), &subenv);
						}
					}
				}
				//其他情况
				else
				{
					//求解值
					result = executeAST(*iterast, env);
					//注册第i个实参至subenv中
					registEnvSymbol(*iterpara, result, &subenv);
				}
				//ast的迭代器步进1
				iterast++;
			}
			//执行body函数(在subenv下)
			try
			{
				result = executeAST(body, &subenv);
			}
			//返回值
			catch (double d)
			{
				result = d;
			}
			//错误代码 继续抛出
			catch (std::exception& e)
			{
				throw e;
			}
		}
	}
	else
	{
		//抛出异常
		throw std::runtime_error("error(bad syntax): undefine symbol!\n");
	}

	return result;
}

//谓词忽略
static auto ignorepred = [](double) { return false; };
//简写
using PAST = std::shared_ptr<ASTNode>;
using PENV = Environment*;
//映射表
static std::unordered_map<TokenType, std::function<double(PAST, PENV)>> ASTTable
{
	{ TokenType::Plus, [](PAST ast, PENV env) {return translateOp2AST(ast, env, [](double a, double b) { return a + b; }, ignorepred); } },
	{ TokenType::Minus, [](PAST ast, PENV env) {return translateOp2AST(ast, env, [](double a, double b) { return a - b; }, ignorepred); } },
	{ TokenType::Mul, [](PAST ast, PENV env) {return translateOp2AST(ast, env, [](double a, double b) { return a * b; }, ignorepred); } },
	{ TokenType::Div, [](PAST ast, PENV env) {return translateOp2AST(ast, env, astdiv, ignorepred); } },
	{ TokenType::Pow, [](PAST ast, PENV env) {return translateOp2AST(ast, env, astpow, ignorepred); } },
	{ TokenType::Mod, [](PAST ast, PENV env) {return translateOp2AST(ast, env, astmod, ignorepred); } },
	{ TokenType::Less, [](PAST ast, PENV env) {return translateOp2AST(ast, env, [](double a, double b) { return a < b; }, ignorepred); } },
	{ TokenType::Great, [](PAST ast, PENV env) {return translateOp2AST(ast, env, [](double a, double b) { return a > b; }, ignorepred); } },
	{ TokenType::NotLess, [](PAST ast, PENV env) {return translateOp2AST(ast, env, [](double a, double b) { return a >= b; }, ignorepred); } },
	{ TokenType::NotGreat, [](PAST ast, PENV env) {return translateOp2AST(ast, env, [](double a, double b) { return a <= b; }, ignorepred); } },
	{ TokenType::Equal, [](PAST ast, PENV env) {return translateOp2AST(ast, env, [](double a, double b) { return a == b; }, ignorepred); } },
	{ TokenType::NotEqual, [](PAST ast, PENV env) {return translateOp2AST(ast, env, [](double a, double b) { return a != b; }, ignorepred); } },
	{ TokenType::And, [](PAST ast, PENV env) {return translateOp2AST(ast, env, [](double a, double b) { return a && b; }, [](double v) -> bool { return !v; }); } },
	{ TokenType::Or, [](PAST ast, PENV env) {return translateOp2AST(ast, env, [](double a, double b) { return a || b; }, [](double v) -> bool { return v; }); } },
	{ TokenType::SelfPlus, [](PAST ast, PENV env) {return translateOp1AST(ast, env, [](double a, double b) { return a + b; }); } },
	{ TokenType::SelfMinus, [](PAST ast, PENV env) {return translateOp1AST(ast, env, [](double a, double b) { return a - b; }); } },
	{ TokenType::SelfMul, [](PAST ast, PENV env) {return translateOp1AST(ast, env, [](double a, double b) { return a * b; }); } },
	{ TokenType::SelfDiv, [](PAST ast, PENV env) {return translateOp1AST(ast, env, astdiv); } },
	{ TokenType::Increment, [](PAST ast, PENV env) { return translateIncrementAST<false>(ast, env, [](double a) { return a + 1; }); } },
	{ TokenType::PostIncrement, [](PAST ast, PENV env) { return translateIncrementAST<true>(ast, env, [](double a) { return a + 1; }); } },
	{ TokenType::Decrement, [](PAST ast, PENV env) { return translateIncrementAST<false>(ast, env, [](double a) { return a - 1; }); } },
	{ TokenType::PostDecrement, [](PAST ast, PENV env) { return translateIncrementAST<true>(ast, env, [](double a) { return a - 1; }); } },
	{ TokenType::Not, [](PAST ast, PENV env) { return !executeAST(*(ast->childs.begin()), env); } },
	{ TokenType::Assign, translateAssignAST },
	{ TokenType::Block, translateBlockAST },
	{ TokenType::If, translateIfAST },
	{ TokenType::While, translateWhileAST },
	{ TokenType::Do, translateDoWhileAST },
	{ TokenType::For, translateForAST },
	{ TokenType::DefVar, translateDefVarAST },
	{ TokenType::DefProc, translateDefProcAST },
	{ TokenType::Number, [](PAST ast, PENV env) { return std::get<double>(ast->tk.value); } },
	{ TokenType::PrimitiveSymbol, translatePrimitiveSymboAST },
	{ TokenType::UserSymbol, translateUserSymbolAST },
	{ TokenType::Break, [](PAST, PENV) -> double { throw BreakState(); } },
	{ TokenType::Continue, [](PAST, PENV) -> double { throw ContinueState(); } },
	{ TokenType::Return, [](PAST ast, PENV env) -> double { throw executeAST(*(ast->childs.begin()), env); } },
	{ TokenType::End, [](PAST, PENV) { return 0; } },
};

//执行语法树
double executeAST(std::shared_ptr<ASTNode> ast, Environment* env)
{
	double result;
	//根据驱动表执行执行对应的函数
	if (auto iter = ASTTable.find(ast->tk.type); iter != ASTTable.end())
	{
		result = iter->second(ast, env);
	}
	else
	{
		throw std::runtime_error("error(bad syntax):\n");
	}
	return abs(result) < 1e-10 ? 0 : result;
}