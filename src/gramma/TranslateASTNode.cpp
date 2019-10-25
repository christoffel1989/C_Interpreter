#include "TranslateASTNode.h"

#include <cmath>
#include <unordered_map>
#include <stdexcept>

//////////////////////////////////////////////构建二元运算符号的表//////////////////////////////////////////////////////////
//二元运算符号对应的函数
double astadd(double a, double b) { return a + b; }
double astminus(double a, double b) { return a - b; }
double astmul(double a, double b) { return a * b; }
double astless(double a, double b) { return a < b; }
double astgreat(double a, double b) { return a > b; }
double astnotless(double a, double b) { return a >= b; }
double astnotgreat(double a, double b) { return a <= b; }
double astequal(double a, double b) { return a == b; }
double astnotequal(double a, double b) { return a != b; }
double astand(double a, double b) { return a && b; }
double astor(double a, double b) { return a || b; }
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
//二元运算的映射表
static std::unordered_map<TokenType, double(*)(double, double)> Op2Table =
{
	{TokenType::Plus, astadd},
	{TokenType::Minus, astminus},
	{TokenType::Mul, astmul},
	{TokenType::Div, astdiv},
	{TokenType::Pow, astpow},
	{TokenType::Mod, astmod},
	{TokenType::Less, astless},
	{TokenType::Great, astgreat},
	{TokenType::NotLess, astnotless},
	{TokenType::NotGreat, astnotgreat},
	{TokenType::Equal, astequal},
	{TokenType::NotEqual, astnotequal},
	{TokenType::And, astand},
	{TokenType::Or, astor},
};
//一元自运算的映射表
static std::unordered_map<TokenType, double(*)(double, double)> Op1SelfTable =
{
	{TokenType::SelfPlus, astadd},
	{TokenType::SelfMinus, astminus},
	{TokenType::SelfMul, astmul},
	{TokenType::SelfDiv, astdiv},
};

//continue跳转抛出的异常
struct ContinueState {};
//break跳转抛出的异常
struct BreakState {};

//执行语法树
double executeAST(std::shared_ptr<ASTNode> ast, ASTEnvironment* env)
{
	//观察AST根节点的类型
	auto tk = ast->tk;
	auto type = tk.type;
	auto& childs = ast->childs;
	double result;

	//用户自定义的符号
	if (type == TokenType::UserSymbol)
	{
		//获得符号名字
		auto symbol = std::get<std::string>(tk.value);
		//如果已经定义了
		if (auto val = getASTEnvSymbol(symbol, env))
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
					registASTEnvSymbol(*iterpara, result, &subenv);
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
	}
	//查询根节点是否在二元运算表中
	else if (Op2Table.find(type) != Op2Table.end())
	{
		auto iter = childs.begin();
		if (childs.size() == 2)
		{
			//计算第1个操作数
			double val1 = executeAST(*iter, env);
			//如果是&&或者||则需要判断是否进行短路运算
			if (type == TokenType::And)
			{
				if (val1 == 0)
				{
					return 0;
				}
			}
			else if (type == TokenType::Or)
			{
				if (val1 != 0)
				{
					return 1;
				}
			}
			//迭代器步进1
			iter++;
			//计算第2个操作数
			double val2 = executeAST(*iter, env);
			result = Op2Table[type](val1, val2);
		}
		//特殊情况为+ 和 -可能是一元运算
		else
		{
			result = executeAST(*iter, env);
			//结果取负
			if (type == TokenType::Minus)
			{
				result = -result;
			}
		}
	}
	//查询根节点是否在一元运算表中
	else if (Op1SelfTable.find(type) != Op1SelfTable.end())
	{
		auto iter = childs.begin();
		//获得变量名字
		auto symbol = std::get<std::string>((*iter)->tk.value);
		if (auto v = getASTEnvSymbol(symbol, env))
		{
			if (std::holds_alternative<double>(v.value()))
			{
				result = std::get<double>(v.value());
				//计算变量值
				iter++;
				result = Op1SelfTable[type](result, executeAST(*iter, env));
				//更新变量在环境中的值
				setASTEnvSymbol(symbol, result, env);
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
		//更新变量在环境中的值
		setASTEnvSymbol(symbol, result, env);
	}
	else if (type == TokenType::Increment)
	{
		//获得符号名字
		auto symbol = std::get<std::string>((*childs.begin())->tk.value);
		if (auto v = getASTEnvSymbol(symbol, env))
		{
			if (std::holds_alternative<double>(v.value()))
			{
				//先加1再赋值
				result = std::get<double>(v.value()) + 1;
				//更新变量在环境中的值
				setASTEnvSymbol(symbol, result, env);
			}
			//变量类型错误
			else
			{
				throw std::runtime_error("error(++): the symbol is not variable!\n");
			}
		}
		//如果查不到这个变量则报错
		else
		{
			throw std::runtime_error("error(++): undefine symbol!\n");
		}
	}
	else if (type == TokenType::PostIncrement)
	{
		//获得符号名字
		auto symbol = std::get<std::string>((*childs.begin())->tk.value);
		if (auto v = getASTEnvSymbol(symbol, env))
		{
			if (std::holds_alternative<double>(v.value()))
			{
				//先赋值
				result = std::get<double>(v.value());
				//再加1更新变量在环境中的值
				setASTEnvSymbol(symbol, result + 1, env);
			}
			//变量类型错误
			else
			{
				throw std::runtime_error("error(++): the symbol is not variable!\n");
			}
		}
		//如果查不到这个变量则报错
		else
		{
			throw std::runtime_error("error(++): undefine symbol!\n");
		}
	}
	else if (type == TokenType::Decrement)
	{
		//获得符号名字
		auto symbol = std::get<std::string>((*childs.begin())->tk.value);
		if (auto v = getASTEnvSymbol(symbol, env))
		{
			if (std::holds_alternative<double>(v.value()))
			{
				//先加1再赋值
				result = std::get<double>(v.value()) - 1;
				//更新变量在环境中的值
				setASTEnvSymbol(symbol, result, env);
			}
			//变量类型错误
			else
			{
				throw std::runtime_error("error(--): the symbol is not variable!\n");
			}
		}
		//如果查不到这个变量则报错
		else
		{
			throw std::runtime_error("error(--): undefine symbol!\n");
		}
	}
	else if (type == TokenType::PostDecrement)
	{
		//获得符号名字
		auto symbol = std::get<std::string>((*childs.begin())->tk.value);
		if (auto v = getASTEnvSymbol(symbol, env))
		{
			if (std::holds_alternative<double>(v.value()))
			{
				//先赋值
				result = std::get<double>(v.value());
				//再减1更新变量在环境中的值
				setASTEnvSymbol(symbol, result - 1, env);
			}
			//变量类型错误
			else
			{
				throw std::runtime_error("error(--): the symbol is not variable!\n");
			}
		}
		//如果查不到这个变量则报错
		else
		{
			throw std::runtime_error("error(--): undefine symbol!\n");
		}
	}
	//数字
	else if (type == TokenType::Number)
	{
		result = std::get<double>(ast->tk.value);
	}
	//非
	else if (type == TokenType::Not)
	{
		auto iter = childs.begin();
		result = !executeAST(*iter, env);
	}
	//语句块
	else if (type == TokenType::Block)
	{
		//构造一个调用函数新的环境
		ASTEnvironment subenv;
		//他的父亲时env
		subenv.parent = env;
		for (auto iter = childs.begin(); iter != childs.end(); iter++)
		{
			//逐行执行代码(在新环境中)
			result = executeAST(*iter, &subenv);
		}
	}
	//条件语句
	else if (type == TokenType::If)
	{
		//构造一个调用函数新的环境
		ASTEnvironment subenv;
		//他的父亲时env
		subenv.parent = env;

		//先计算条件
		auto iter = childs.begin();
		double condition = executeAST(*iter, &subenv);
		if (condition != 0)
		{
			//执行if的语句块
			iter++;
			result = executeAST(*iter, &subenv);
		}
		else
		{
			//执行else的语句块
			iter++;
			iter++;
			//可能不存再else分支所以要判断一下
			if (iter != childs.end())
			{
				result = executeAST(*iter, &subenv);
			}
		}
	}
	//While语句
	else if (type == TokenType::While)
	{
		//构造一个调用函数新的环境
		ASTEnvironment subenv;
		//他的父亲时env
		subenv.parent = env;

		//获得循环条件
		auto iter = childs.begin();
		auto condition = *iter;
		//获得循环体
		iter++;
		auto body = *iter;
		result = 0;

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
	}
	//for语句
	else if (type == TokenType::For)
	{
		//构造一个调用函数新的环境
		ASTEnvironment subenv;
		//他的父亲时env
		subenv.parent = env;

		//获得循环起始
		auto iter = childs.begin();
		auto start = *iter;
		//获得终止条件
		iter++;
		auto end = *iter;
		//获得步进
		iter++;
		auto increment = *iter;
		//获得循环体
		iter++;
		auto body = *iter;
		result = 0;
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
	}
	//break语句
	else if (type == TokenType::Break)
	{
		throw BreakState();
	}
	//break语句
	else if (type == TokenType::Continue)
	{
		throw ContinueState();
	}
	//return语句
	else if (type == TokenType::Return)
	{
		//获得循环起始
		auto iter = childs.begin();
		//以抛出异常的形式返回
		throw executeAST(*iter, env);
	}
	//无操作
	else if (type == TokenType::End)
	{
	}
	//自定义变量
	else if (type == TokenType::DefVar)
	{
		auto iter = childs.begin();
		//获得变量名字
		auto symbol = std::get<std::string>((*iter)->tk.value);
		//如果在当前环境中已经定义则报错
		if (getASTEnvSymbolInCurrent(symbol, env))
		{
			throw std::runtime_error("error(Def var): " + symbol + " redefined!\n");
		}
		iter++;
		//计算变量定义式值
		result = executeAST(*iter, env);
		//注册变量至环境当中
		registASTEnvSymbol(symbol, result, env);
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
		//如果在当前环境中已经定义则报错
		if (getASTEnvSymbolInCurrent(symbol, env))
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
		registASTEnvSymbol(symbol, std::make_tuple(args, body), env);
		//函数定义返回值设置为0
		result = 0;
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
	else
	{
		throw std::runtime_error("error(bad syntax):\n");
	}

	if (abs(result) < 1e-10)
	{
		result = 0;
	}

	return result;
}