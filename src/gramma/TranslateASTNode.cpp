#include "TranslateASTNode.h"

#include "token.h"

#include <cmath>
#include <iostream>

//执行语法树
double executeAST(std::shared_ptr<ASTNode> ast, ASTEnvironment* env)
{
	//观察AST根节点的类型
	auto tk = ast->tk;
	auto type = tk.type;
	auto& childs = ast->childs;
	double result;

	//数字
	if (type == TokenType::Number)
	{
		result = std::get<double>(ast->tk.value);
	}
	//加法
	else if (type == TokenType::Plus)
	{
		auto iter = childs.begin();
		double val1 = executeAST(*iter, env);
		iter++;
		double val2 = executeAST(*iter, env);
		result = val1 + val2;
	}
	//减法
	else if (type == TokenType::Minus)
	{
		auto iter = childs.begin();
		double val1 = executeAST(*iter, env);
		iter++;
		double val2 = executeAST(*iter, env);
		result = val1 - val2;
	}
	//乘法
	else if (type == TokenType::Mul)
	{
		auto iter = childs.begin();
		double val1 = executeAST(*iter, env);
		iter++;
		double val2 = executeAST(*iter, env);
		result = val1 * val2;
	}
	//除法
	else if (type == TokenType::Div)
	{
		auto iter = childs.begin();
		double val1 = executeAST(*iter, env);
		iter++;
		double val2 = executeAST(*iter, env);
		if (val2 == 0)
		{
			//报一个错误
			throw std::runtime_error("error(bad syntax): divided by zero!\n");
		}
		result = val1 / val2;
	}
	//幂乘
	else if (type == TokenType::Pow)
	{
		auto iter = childs.begin();
		double val1 = executeAST(*iter, env);
		iter++;
		double val2 = executeAST(*iter, env);
		//当底为0 幂为非正实数时幂操作无效
		if (val1 == 0 && val2 <= 0)
		{
			throw std::runtime_error("error(arithmatic): zero can not be power by non-positive value!\n");
		}
		result = pow(val1, val2);
	}
	//小于
	else if (type == TokenType::Less)
	{
		auto iter = childs.begin();
		double val1 = executeAST(*iter, env);
		iter++;
		double val2 = executeAST(*iter, env);
		result = val1 < val2;
	}
	//大于
	else if (type == TokenType::Great)
	{
		auto iter = childs.begin();
		double val1 = executeAST(*iter, env);
		iter++;
		double val2 = executeAST(*iter, env);
		result = val1 > val2;
	}
	//大于等于
	else if (type == TokenType::NotLess)
	{
		auto iter = childs.begin();
		double val1 = executeAST(*iter, env);
		iter++;
		double val2 = executeAST(*iter, env);
		result = val1 >= val2;
	}
	//小于等于
	else if (type == TokenType::NotGreat)
	{
		auto iter = childs.begin();
		double val1 = executeAST(*iter, env);
		iter++;
		double val2 = executeAST(*iter, env);
		result = val1 <= val2;
	}
	//等于
	else if (type == TokenType::Equal)
	{
		auto iter = childs.begin();
		double val1 = executeAST(*iter, env);
		iter++;
		double val2 = executeAST(*iter, env);
		result = val1 == val2;
	}
	//条件语句
	else if (type == TokenType::If)
	{
		//先计算条件
		auto iter = childs.begin();
		double condition = executeAST(*iter, env);
		if (condition != 0)
		{
			//执行if的语句块
			iter++;
			result = executeAST(*iter, env);
		}
		else
		{
			//执行else的语句块
			iter++;
			iter++;
			result = executeAST(*iter, env);
		}
	}
	//原生变量及函数
	else if (type == TokenType::PrimitiveSymbol)
	{
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
	}
	//自定义变量
	else if (type == TokenType::DefVar)
	{
		auto iter = childs.begin();
		//获得变量名字
		auto symbol = std::get<std::string>((*iter)->tk.value);
		iter++;
		//计算变量定义式值
		result = executeAST(*iter, env);
		//注册变量至环境当中
		setASTEnvSymbol(symbol, { {}, result }, env);
	}
	//自定义函数
	else if (type == TokenType::DefProc)
	{
		auto iter = childs.begin();
		//获得函数本体
		auto body = *iter;
		iter++;
		//获得函数体的名字
		auto symbol = std::get<std::string>((*iter)->tk.value);
		//获得各个参数的名字
		std::list<std::string> args;
		iter++;
		for (; iter != childs.end(); iter++)
		{
			args.push_back(std::get<std::string>((*iter)->tk.value));
		}
		//注册函数至环境当中
		setASTEnvSymbol(symbol, { args, body }, env);
		//函数定义返回值设置为0
		result = 0;
	}
	//用户自定义的符号
	else if (type == TokenType::UserSymbol)
	{
		//获得符号名字
		auto symbol = std::get<std::string>(tk.value);
		//如果已经定义了
		if (auto val = getASTEnvSymbol(symbol, env))
		{
			//解包参数
			auto[paras, body] = val.value();
			//如果body是值类型则说明为变量
			if (std::holds_alternative<double>(body))
			{
				result = std::get<double>(body);
			}
			//函数类型
			else
			{
				//构造一个调用函数新的环境
				ASTEnvironment subenv;
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
					result = executeAST(*iterast, env);
					//注册第i个实参至subenv中
					setASTEnvSymbol(*iterpara, { {}, result }, &subenv);
					//ast的迭代器步进1
					iterast++;
				}
				//执行body函数(在subenv下)
				result = executeAST(std::get<std::shared_ptr<ASTNode>>(body), &subenv);
			}
		}
		else
		{
			//抛出异常
			throw std::runtime_error("error(bad syntax): undefine symbol!\n");
		}
	}
	else if (type == TokenType::Assign)
	{
		auto iter = childs.begin();
		//获得变量名字
		auto symbol = std::get<std::string>((*iter)->tk.value);
		//如果查不到这个变量则报错
		if (!getASTEnvSymbol(symbol, env))
		{
			throw std::runtime_error("error(assignment): undefine symbol!\n");
		}
		iter++;
		//计算变量定义式值
		result = executeAST(*iter, env);
		//注册变量至环境当中
		setASTEnvSymbol(symbol, { {}, result }, env);
	}
	//语句块
	else if (type == TokenType::Block)
	{
		for (auto iter = childs.begin(); iter != childs.end(); iter++)
		{
			//逐行执行代码
			result = executeAST(*iter, env);
		}
	}

	if (abs(result) < 1e-10)
	{
		result = 0;
	}

	return result;
}