#include "InterpreteASTNode.h"

#include <cmath>
#include <unordered_map>
#include <stdexcept>

#include <functional>

//存在异常可能性的二元运算符号操作的函数
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

//翻译取地址语法节点
double interpreteRefAST(std::shared_ptr<ASTNode> ast, Environment* env)
{
	auto node = *(ast->childs.begin());
	//如果不是子节点不是usersymbol类型则返回(后续还应该继续支持如果子节点是deref的情形)
	if (node->tk.type == TokenType::UserSymbol)
	{
		//获得变量符号名字
		auto symbol = std::get<std::string>(node->tk.value);
		//如果查询变量的地址值
		if (auto val = getEnvSymbolAddr(symbol, env))
		{
			//转换成double返回
			return (double)val.value();
		}
		//如果没查询到
		else
		{
			throw std::runtime_error("error(ref): undefine symbol " + symbol + " !\n");
		}
	}
	//如果是解引用
	else if (node->tk.type == TokenType::DeRef)
	{
		//获取被解引用的表达式
		node = *(node->childs.begin());
		//计算表达式(计算结果即为地址)
		return interpreteAST(node, env);
	}
	//错误用法
	else
	{
		throw std::runtime_error("error(ref): has no address!\n");
	}
}

//解引用辅助模板
//模板参数LVal表征解引用是当为左值还是右值 true时为左值 fasle时为右值(此时astR赋值nullptr)
//模板参数Self表征是否不是单纯的赋值而是存在额外自运算 true(且LVal为true)时为自运算需要设置一个二元运算算子op false(且LVal为fasle)时为普通赋值 op随便扔一个lambda进去即可利用constexpr的特性不会编译
template <bool LVal, bool Self, typename T, typename OP>
double auxDeRefAST(double rval, UserAST astval, int iaddr, OP op)
{
	//如果是右值则先把rval赋值为左侧解引用得到的数值
	if constexpr (!LVal)
	{
		rval = std::get<T>(astval);
	}

	//如果是自运算则rval多一次运算
	if constexpr (Self)
	{
		//进入到此处时LVal不能是false
		static_assert(LVal, "self operation need deref be a L-value operation!\n");
		//计算结果值
		rval = op(rval, std::get<T>(astval));
	}

	//如果是左值则将求解结果(数值)赋值给地址为iddr处的内存
	if constexpr (LVal)
	{
		setEnvSymbol(T(rval), VarAddress(iaddr));
	}

	return rval;
}

//指针解引用运算(模板)
//模板参数LVal表征解引用是当为左值还是右值 true时为左值 fasle时为右值(此时astR赋值nullptr)
//模板参数Self表征是否不是单纯的赋值而是存在额外自运算 true(且LVal为true)时为自运算需要设置一个二元运算算子op false(且LVal为fasle)时为普通赋值 op随便扔一个lambda进去即可利用constexpr的特性不会编译
template <bool LVal, bool Self, typename OP>
double interpreteDeRefAST(std::shared_ptr<ASTNode> astL, std::shared_ptr<ASTNode> astR, Environment* env, OP op)
{
	auto iter = astL->childs.begin();
	//计算表达式的值
	auto daddr = interpreteAST(*iter, env);
	//查看是否是非负整数
	auto iaddr = (int)daddr;
	//如果result是非负整数
	if (iaddr >= 0 && iaddr == daddr)
	{
		//获取左侧地址的对应的变量值
		if (auto v = getEnvSymbol(VarAddress(iaddr)))
		{
			double rval = 0;
			//如果是左值则rval赋值为右侧表达式的计算结果
			if constexpr (LVal)
			{
				rval = interpreteAST(astR, env);
			}

			//如果计算结果是double
			if (std::holds_alternative<double>(v.value()))
			{
				return auxDeRefAST<LVal, Self, double>(rval, v.value(), iaddr, op);
			}
			//如果计算结果是VarAddress
			else if (std::holds_alternative<VarAddress>(v.value()))
			{
				return auxDeRefAST<LVal, Self, VarAddress>(rval, v.value(), iaddr, op);
			}
			//如果计算结果是函数
			else
			{
				//抛出异常
				throw std::runtime_error("error(Deref pointer): function can not be deref!\n");
			}
		}
		else
		{
			//抛出异常
			throw std::runtime_error("error(Deref pointer): address out of stack top\n");
		}
	}
	else
	{
		//抛出异常
		throw std::runtime_error("error(Deref pointer): address should be non negetive integral value\n");
	}
}

//翻译赋值符号语法节点
double interpreteAssignAST(std::shared_ptr<ASTNode> ast, Environment* env)
{
	auto iter = ast->childs.begin();
	//如果是引用类型
	if ((*iter)->tk.type == TokenType::DeRef)
	{
		//执行左值解引用赋值操作
		auto astL = *iter;
		auto astR = *(++iter);
		return interpreteDeRefAST<true, false>(astL, astR, env, []() {});
	}
	else
	{
		//获得变量名字
		auto symbol = std::get<std::string>((*iter)->tk.value);
		//查询这个变量
		if (auto v = getEnvSymbol(symbol, env))
		{
			//计算表达式结果
			auto result = interpreteAST(*(++iter), env);
			//如果是变量
			if (std::holds_alternative<double>(v.value()))
			{
				//更新变量在环境中的值
				setEnvSymbol(symbol, result, env);
			}
			//其他情况下是指针
			else if (std::holds_alternative<VarAddress>(v.value()))
			{
				auto addr = (int)result;
				//如果result是非负整数
				if (addr >= 0 && addr == result)
				{
					//更新指针在环境中的存储的地址值
					setEnvSymbol(symbol, VarAddress(addr), env);
				}
				else
				{
					//抛出异常
					throw std::runtime_error("error(assignment): address should be non negetive integral value!\n");
				}
			}
			//变量 理论上不应该到达这个位置
			else 
			{
				//抛出异常
				throw std::runtime_error("error(assignment):can not assign a function!\n");
			}

			return result;
		}
		else
		{
			//查不到这个变量报错
			throw std::runtime_error("error(assignment): undefine symbol!\n");
		}
	}
}

//翻译由中括号包起来的语句块
double interpreteBlockAST(std::shared_ptr<ASTNode> ast, Environment* env)
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
		result = interpreteAST(*iter, &subenv);
	}
	return result;
}

//翻译if语法节点
double interpreteIfAST(std::shared_ptr<ASTNode> ast, Environment* env)
{
	//构造一个调用函数新的环境
	Environment subenv;
	//他的父亲时env
	subenv.parent = env;

	//先计算条件
	auto iter = ast->childs.begin();
	double result = 0;
	double condition = interpreteAST(*iter, &subenv);
	if (condition != 0)
	{
		//执行if的语句块
		result = interpreteAST(*(++iter), &subenv);
	}
	else
	{
		//执行else的语句块
		++iter; ++iter;
		//可能不存再else分支所以要判断一下
		if (iter != ast->childs.end())
		{
			result = interpreteAST(*iter, &subenv);
		}
	}
	return result;
}

//continue跳转抛出的异常
struct ContinueState {};
//break跳转抛出的异常
struct BreakState {};

//翻译while语法节点
double interpreteWhileAST(std::shared_ptr<ASTNode> ast, Environment* env)
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
	while (interpreteAST(condition, &subenv) != 0)
	{
		try
		{
			//执行循环体
			result = interpreteAST(body, &subenv);
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

//翻译do while语法节点
double interpreteDoWhileAST(std::shared_ptr<ASTNode> ast, Environment* env)
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
			result = interpreteAST(body, &subenv);
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
	} while (interpreteAST(condition, &subenv) != 0);

	return result;
}

//翻译for语法节点
double interpreteForAST(std::shared_ptr<ASTNode> ast, Environment* env)
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
	for (interpreteAST(start, &subenv); interpreteAST(end, &subenv) != 0; interpreteAST(increment, &subenv))
	{
		try
		{
			//执行循环体
			result = interpreteAST(body, &subenv);
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

//翻译定义变量语法节点
double interpreteDefVarAST(std::shared_ptr<ASTNode> ast, Environment* env)
{
	//自定义变量
	auto iter = ast->childs.begin();
	//获得变量名字
	auto symbol = std::get<std::string>((*iter)->tk.value);
	//如果在当前环境中已经定义则也报错
	if (getEnvSymbol(symbol, env, true))
	{
		throw std::runtime_error("error(Def var): " + symbol + " redefined!\n");
	}
	//计算变量定义式值
	auto result = interpreteAST(*(++iter), env);
	//注册变量至环境当中
	registEnvSymbol(symbol, result, env);
	
	return result;
}

//翻译定义指针变量语法节点
double interpreteDefPointerAST(std::shared_ptr<ASTNode> ast, Environment* env)
{
	//自定义变量
	auto iter = ast->childs.begin();
	//获得变量名字
	auto symbol = std::get<std::string>((*iter)->tk.value);
	//如果在当前环境中已经定义则也报错
	if (getEnvSymbol(symbol, env, true))
	{
		throw std::runtime_error("error(Def pointer): " + symbol + " redefined!\n");
	}
	//计算变量定义式值
	auto daddr = interpreteAST(*(++iter), env);
	auto iaddr = (int)daddr;
	//如果result是非负整数
	if (iaddr >= 0 && iaddr == daddr)
	{
		//注册变量至环境当中
		registEnvSymbol(symbol, VarAddress(iaddr), env);
	}
	else
	{
		//抛出异常
		throw std::runtime_error("error(Def pointer): address should be non negetive integral value!\n");
	}

	return daddr;
}

//翻译定义函数语法节点
double interpreteDefProcAST(std::shared_ptr<ASTNode> ast, Environment* env)
{
	auto& childs = ast->childs;
	auto iter = childs.begin();
	//获得函数本体
	auto body = *iter;
	iter++;
	//获得函数体的名字
	auto symbol = std::get<std::string>((*iter)->tk.value);
	//如果在当前环境中已经定义则也报错
	if (getEnvSymbol(symbol, env, true))
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

//翻译系统自定义符号语法节点
double interpretePrimitiveSymboAST(std::shared_ptr<ASTNode> ast, Environment* env)
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
		auto arg = interpreteAST(*childs.begin(), env);
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

//翻译用户自定义符号语法节点
double interpreteUserSymbolAST(std::shared_ptr<ASTNode> ast, Environment* env)
{
	auto tk = ast->tk;
	auto& childs = ast->childs;
	double result;

	//获得符号名字
	auto symbol = std::get<std::string>(tk.value);
	//如果已经定义了
	if (auto val = getEnvSymbol(symbol, env))
	{
		//double类型则说明是值类型变量
		if (std::holds_alternative<double>(val.value()))
		{
			result = std::get<double>(val.value());
		}
		//VarAddress类型则说明是指针类型变量
		else if (std::holds_alternative<VarAddress>(val.value()))
		{
			result = std::get<VarAddress>(val.value());
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
				auto node = *iterast;
				//如果第i个实参是用户自定义符号
				if (node->tk.type == TokenType::UserSymbol)
				{
					if (auto v = getEnvSymbol(std::get<std::string>(node->tk.value), env))
					{
						//注册第i个实参(可能是变量指针或者函数)至subenv中
						registEnvSymbol(*iterpara, v.value(), &subenv);
					}
				}
				//如果是引用(获取地址)
				else if (node->tk.type == TokenType::Ref)
				{
					//求解值
					result = interpreteAST(*iterast, env);
					//注册第i个实参至subenv中
					registEnvSymbol(*iterpara, VarAddress(result), &subenv);
				}
				//普通表达式
				else
				{
					//求解值
					result = interpreteAST(*iterast, env);
					//注册第i个实参至subenv中
					registEnvSymbol(*iterpara, result, &subenv);
				}
				//ast的迭代器步进1
				iterast++;
			}
			//执行body函数(在subenv下)
			try
			{
				result = interpreteAST(body, &subenv);
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

//翻译一元运算节点的模板
template<typename OP>
double interpreteOp1AST(std::shared_ptr<ASTNode> ast, Environment* env, OP op)
{
	double result;
	auto iter = ast->childs.begin();
	//引用自加
	if ((*iter)->tk.type == TokenType::DeRef)
	{
		//执行左值解引用赋值操作
		auto astL = *iter;
		auto astR = *(++iter);
		//执行解引用(自运算的左值)
		return interpreteDeRefAST<true, true>(astL, astR, env, op);
	}
	else
	{
		//获得变量名字
		auto symbol = std::get<std::string>((*iter)->tk.value);
		if (auto v = getEnvSymbol(symbol, env))
		{
			if (std::holds_alternative<double>(v.value()))
			{
				//计算变量值
				result = op(std::get<double>(v.value()), interpreteAST(*(++iter), env));
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
	}

	return result;
}

//翻译二元运算节点的模板
template<typename OP, typename PRED>
double interpreteOp2AST(std::shared_ptr<ASTNode> ast, Environment* env, OP op, PRED pred)
{
	auto iter = ast->childs.begin();
	if (ast->childs.size() == 2)
	{
		double val1 = interpreteAST(*iter, env);
		if (pred(val1)) return val1;
		double val2 = interpreteAST(*(++iter), env);
		return op(val1, val2);
	}
	else
	{
		auto val = interpreteAST(*iter, env);
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
double interpreteIncrementAST(std::shared_ptr<ASTNode> ast, Environment* env, OP op)
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

	//根据post的是true还是false确定是返回自加之前还是自加之后的值
	if constexpr (post)
	{
		return prev;
	}
	else
	{
		return after;
	}
}

//谓词忽略
static auto ignorepred = [](double) { return false; };
//简写
using PAST = std::shared_ptr<ASTNode>;
using PENV = Environment*;
//映射表
static std::unordered_map<TokenType, std::function<double(PAST, PENV)>> ASTTable
{
	{ TokenType::Plus, [](PAST ast, PENV env) {return interpreteOp2AST(ast, env, [](double a, double b) { return a + b; }, ignorepred); } },
	{ TokenType::Minus, [](PAST ast, PENV env) {return interpreteOp2AST(ast, env, [](double a, double b) { return a - b; }, ignorepred); } },
	{ TokenType::Mul, [](PAST ast, PENV env) {return interpreteOp2AST(ast, env, [](double a, double b) { return a * b; }, ignorepred); } },
	{ TokenType::Div, [](PAST ast, PENV env) {return interpreteOp2AST(ast, env, astdiv, ignorepred); } },
	{ TokenType::Pow, [](PAST ast, PENV env) {return interpreteOp2AST(ast, env, astpow, ignorepred); } },
	{ TokenType::Mod, [](PAST ast, PENV env) {return interpreteOp2AST(ast, env, astmod, ignorepred); } },
	{ TokenType::Less, [](PAST ast, PENV env) {return interpreteOp2AST(ast, env, [](double a, double b) { return a < b; }, ignorepred); } },
	{ TokenType::Great, [](PAST ast, PENV env) {return interpreteOp2AST(ast, env, [](double a, double b) { return a > b; }, ignorepred); } },
	{ TokenType::NotLess, [](PAST ast, PENV env) {return interpreteOp2AST(ast, env, [](double a, double b) { return a >= b; }, ignorepred); } },
	{ TokenType::NotGreat, [](PAST ast, PENV env) {return interpreteOp2AST(ast, env, [](double a, double b) { return a <= b; }, ignorepred); } },
	{ TokenType::Equal, [](PAST ast, PENV env) {return interpreteOp2AST(ast, env, [](double a, double b) { return a == b; }, ignorepred); } },
	{ TokenType::NotEqual, [](PAST ast, PENV env) {return interpreteOp2AST(ast, env, [](double a, double b) { return a != b; }, ignorepred); } },
	{ TokenType::And, [](PAST ast, PENV env) {return interpreteOp2AST(ast, env, [](double a, double b) { return a && b; }, [](double v) -> bool { return !v; }); } },
	{ TokenType::Or, [](PAST ast, PENV env) {return interpreteOp2AST(ast, env, [](double a, double b) { return a || b; }, [](double v) -> bool { return v; }); } },
	{ TokenType::SelfPlus, [](PAST ast, PENV env) {return interpreteOp1AST(ast, env, [](double a, double b) { return a + b; }); } },
	{ TokenType::SelfMinus, [](PAST ast, PENV env) {return interpreteOp1AST(ast, env, [](double a, double b) { return a - b; }); } },
	{ TokenType::SelfMul, [](PAST ast, PENV env) {return interpreteOp1AST(ast, env, [](double a, double b) { return a * b; }); } },
	{ TokenType::SelfDiv, [](PAST ast, PENV env) {return interpreteOp1AST(ast, env, astdiv); } },
	{ TokenType::Increment, [](PAST ast, PENV env) { return interpreteIncrementAST<false>(ast, env, [](double a) { return a + 1; }); } },
	{ TokenType::PostIncrement, [](PAST ast, PENV env) { return interpreteIncrementAST<true>(ast, env, [](double a) { return a + 1; }); } },
	{ TokenType::Decrement, [](PAST ast, PENV env) { return interpreteIncrementAST<false>(ast, env, [](double a) { return a - 1; }); } },
	{ TokenType::PostDecrement, [](PAST ast, PENV env) { return interpreteIncrementAST<true>(ast, env, [](double a) { return a - 1; }); } },
	{ TokenType::Not, [](PAST ast, PENV env) { return !interpreteAST(*(ast->childs.begin()), env); } },
	{ TokenType::Ref, interpreteRefAST },
	{ TokenType::DeRef,[](PAST ast, PENV env) { return interpreteDeRefAST<false, false>(ast, nullptr, env, []() {}); } },
	{ TokenType::Assign, interpreteAssignAST },
	{ TokenType::Block, interpreteBlockAST },
	{ TokenType::If, interpreteIfAST },
	{ TokenType::While, interpreteWhileAST },
	{ TokenType::Do, interpreteDoWhileAST },
	{ TokenType::For, interpreteForAST },
	{ TokenType::DefVar, interpreteDefVarAST },
	{ TokenType::DefPointer, interpreteDefPointerAST },
	{ TokenType::DefProc, interpreteDefProcAST },
	{ TokenType::Number, [](PAST ast, PENV env) { return std::get<double>(ast->tk.value); } },
	{ TokenType::PrimitiveSymbol, interpretePrimitiveSymboAST },
	{ TokenType::UserSymbol, interpreteUserSymbolAST },
	{ TokenType::Break, [](PAST, PENV) -> double { throw BreakState(); } },
	{ TokenType::Continue, [](PAST, PENV) -> double { throw ContinueState(); } },
	{ TokenType::Return, [](PAST ast, PENV env) -> double { throw interpreteAST(*(ast->childs.begin()), env); } },
	{ TokenType::End, [](PAST, PENV) { return 0; } },
};

//翻译语法节点(总入口)
double interpreteAST(std::shared_ptr<ASTNode> ast, Environment* env)
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