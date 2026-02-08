#include "InterpreteASTNode.h"

#include <cmath>
#include <unordered_map>
#include <stdexcept>

#include <functional>

//存在异常可能性的二元运算符号操作的函数
auto astdiv(double a, double b) -> ASTResult
{
	//除数不能为0
	if (b == 0)
	{
		return std::unexpected(ErrorState("error(bad syntax): divided by zero!\n"));
	}
	return a / b;
}
auto astpow(double a, double b) -> ASTResult
{
	//当底为0 幂为非正实数时幂操作无效
	if (a == 0 && b <= 0)
	{
		return std::unexpected(ErrorState("error(arithmatic): zero can not be power by non-positive value!\n"));
	}
	return pow(a, b);
}
auto astmod(double a, double b) -> ASTResult
{
	int ia = (int)a;
	if (ia != a)
	{
		return std::unexpected(ErrorState("error(arithmatic): non-integral number can not be modded!\n"));
	}
	int ib = (int)b;
	if (ib != b)
	{
		return std::unexpected(ErrorState("error(arithmatic): non-integral number can not be used to modded!\n"));
	}
	return ia % ib;
}

//解释取地址语法节点
auto interpreteRefAST(std::shared_ptr<ASTNode> ast, Environment* env) -> ASTResult
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
			return std::unexpected(ErrorState("error(ref): undefine symbol " + symbol + " !\n"));
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
		return std::unexpected(ErrorState("error(ref): has no address!\n"));
	}
}

//解引用辅助模板
//模板参数LVal表征解引用是当为左值还是右值 true时为左值 fasle时为右值(此时astR赋值nullptr)
//模板参数Self表征是否不是单纯的赋值而是存在额外自运算
//当Self为true(且LVal为true)时为自运算需要设置一个二元运算算子op 
//当Self为false(且LVal为fasle)时为普通赋值 op随便扔一个任意类型变量(例如数字0)即可利用constexpr的特性不会编译
template <bool LVal, bool Self, typename T, typename OP>
auto auxDeRefAST(double rval, UserAST astval, int iaddr, OP op) -> ASTResult
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
		TRY(rval, op(rval, std::get<T>(astval)));
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
//模板参数Self表征是否不是单纯的赋值而是存在额外自运算
//当Self为true(且LVal为true)时为自运算需要设置一个二元运算算子op 
//当Self为false(且LVal为fasle)时为普通赋值 op随便扔一个任意类型变量(例如数字0)即可利用constexpr的特性不会编译
template <bool LVal, bool Self, typename OP>
auto interpreteDeRefAST(std::shared_ptr<ASTNode> astL, std::shared_ptr<ASTNode> astR, Environment* env, OP op) -> ASTResult
{
	auto iter = astL->childs.begin();
	//计算表达式的值
	TRY_AUTO(daddr, interpreteAST(*iter, env));
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
				TRY(rval, interpreteAST(astR, env))
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
				return std::unexpected(ErrorState("error(Deref pointer): function can not be deref!\n"));
			}
		}
		else
		{
			return std::unexpected(ErrorState("error(Deref pointer): address out of stack top\n"));
		}
	}
	else
	{
		return std::unexpected(ErrorState("error(Deref pointer): address should be non negetive integral value\n"));
	}
}

//解释赋值符号语法节点
auto interpreteAssignAST(std::shared_ptr<ASTNode> ast, Environment* env) -> ASTResult
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
			TRY_AUTO(result, interpreteAST(*(++iter), env))
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
					return std::unexpected(ErrorState("error(assignment): address should be non negetive integral value!\n"));
				}
			}
			//变量 理论上不应该到达这个位置
			else 
			{
				return std::unexpected(ErrorState("error(assignment):can not assign a function!\n"));
			}

			return result;
		}
		else
		{
			return std::unexpected(ErrorState("error(assignment): undefine symbol!\n"));
		}
	}
}

//解释由中括号包起来的语句块
auto interpreteBlockAST(std::shared_ptr<ASTNode> ast, Environment* env) -> ASTResult
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
		TRY(result, interpreteAST(*iter, &subenv));
	}
	return result;
}

//解释if语法节点
auto interpreteIfAST(std::shared_ptr<ASTNode> ast, Environment* env) -> ASTResult
{
	//构造一个调用函数新的环境
	Environment subenv;
	//他的父亲时env
	subenv.parent = env;

	//先计算条件
	auto iter = ast->childs.begin();
	double result = 0;
	TRY_AUTO(condition, interpreteAST(*iter, &subenv));
	if (condition != 0)
	{
		//执行if的语句块
		TRY(result, interpreteAST(*(++iter), &subenv));
	}
	else
	{
		//执行else的语句块
		++iter; ++iter;
		//可能不存再else分支所以要判断一下
		if (iter != ast->childs.end())
		{
			TRY(result, interpreteAST(*iter, &subenv));
		}
	}
	return result;
}

//解释while语法节点
auto interpreteWhileAST(std::shared_ptr<ASTNode> ast, Environment* env) -> ASTResult
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
	while (true)
	{
		TRY_AUTO(condition_result, interpreteAST(condition, &subenv));
		if (condition_result == 0)
		{
			break;
		}

		if (auto body_result = interpreteAST(body, &subenv); body_result.value())
		{
			result = body_result.value();
		}
		else
		{
			auto&& e = body_result.error();
			if (std::holds_alternative<ContinueState>(e)) 
			{
				continue;
			}
			else if (std::holds_alternative<BreakState>(e))
			{
				break;
			}
			else if (std::holds_alternative<ReturnState>(e))
			{
				return std::get<ReturnState>(e).value;
			}
			else if (std::holds_alternative<ErrorState>(e))
			{
				return std::unexpected(e);
			}
		}
		
	}

	return result;
}

//解释do while语法节点
auto interpreteDoWhileAST(std::shared_ptr<ASTNode> ast, Environment* env) -> ASTResult
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
	while (true)
	{
		if (auto body_result = interpreteAST(body, &subenv); body_result.value())
		{
			result = body_result.value();
		}
		else
		{
			auto&& e = body_result.error();
			if (std::holds_alternative<ContinueState>(e))
			{
				continue;
			}
			else if (std::holds_alternative<BreakState>(e))
			{
				break;
			}
			else if (std::holds_alternative<ReturnState>(e))
			{
				return std::get<ReturnState>(e).value;
			}
			else if (std::holds_alternative<ErrorState>(e))
			{
				return std::unexpected(e);
			}
		}
		TRY_AUTO(condition_result, interpreteAST(condition, &subenv));
		if (condition_result == 0)
		{
			break;
		}
	}

	return result;
}

//解释for语法节点
auto interpreteForAST(std::shared_ptr<ASTNode> ast, Environment* env) -> ASTResult
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
	TRY_IGNORE(interpreteAST(start, &subenv));
	while (true)
	{
		TRY_AUTO(condition_result, interpreteAST(end, &subenv));
		if (condition_result == 0)
		{
			break;
		}
		if (auto body_result = interpreteAST(body, &subenv); body_result.value())
		{
			result = body_result.value();
		}
		else
		{
			auto&& e = body_result.error();
			if (std::holds_alternative<ContinueState>(e))
			{
				continue;
			}
			else if (std::holds_alternative<BreakState>(e))
			{
				break;
			}
			else if (std::holds_alternative<ReturnState>(e))
			{
				return std::get<ReturnState>(e).value;
			}
			else if (std::holds_alternative<ErrorState>(e))
			{
				return std::unexpected(e);
			}
		}
		TRY_IGNORE(interpreteAST(increment, &subenv));
	}

	return result;
}

//解释定义变量语法节点
auto interpreteDefVarAST(std::shared_ptr<ASTNode> ast, Environment* env) -> ASTResult
{
	//自定义变量
	auto iter = ast->childs.begin();
	//获得变量名字
	auto symbol = std::get<std::string>((*iter)->tk.value);
	//如果在当前环境中已经定义则也报错
	if (getEnvSymbol(symbol, env, true))
	{
		return std::unexpected(ErrorState("error(Def var): " + symbol + " redefined!\n"));
	}
	//计算变量定义式值
	TRY_AUTO(result, interpreteAST(*(++iter), env));
	//注册变量至环境当中
	registEnvSymbol(symbol, result, env);
	
	return result;
}

//解释定义指针变量语法节点
auto interpreteDefPointerAST(std::shared_ptr<ASTNode> ast, Environment* env) -> ASTResult
{
	//自定义变量
	auto iter = ast->childs.begin();
	//获得变量名字
	auto symbol = std::get<std::string>((*iter)->tk.value);
	//如果在当前环境中已经定义则也报错
	if (getEnvSymbol(symbol, env, true))
	{
		return std::unexpected(ErrorState("error(Def pointer): " + symbol + " redefined!\n"));
	}
	//计算变量定义式值
	TRY_AUTO(daddr, interpreteAST(*(++iter), env));
	auto iaddr = (int)daddr;
	//如果result是非负整数
	if (iaddr >= 0 && iaddr == daddr)
	{
		//注册变量至环境当中
		registEnvSymbol(symbol, VarAddress(iaddr), env);
	}
	else
	{
		return std::unexpected(ErrorState("error(Def pointer): address should be non negetive integral value!\n"));
	}

	return daddr;
}

//解释定义数组变量的语法节点
auto interpreteDefArrayAST(std::shared_ptr<ASTNode> ast, Environment* env) -> ASTResult
{
	//自定义变量
	auto iter = ast->childs.begin();
	//获得变量名字
	auto symbol = std::get<std::string>((*iter)->tk.value);
	//如果在当前环境中已经定义则也报错
	if (getEnvSymbol(symbol, env, true))
	{
		return std::unexpected(ErrorState("error(Def array): " + symbol + " redefined!\n"));
	}
	//获取数组元素个数
	auto N = ast->childs.size() - 1;
	std::vector<UserAST> values(N);
	//计算每一个初始化元素的值
	for (decltype(N) i = 0; i < N; i++)
	{
		TRY_AUTO(value, interpreteAST(*(++iter), env));
		values[i] = value;
	}
	//注册变量
	registEnvSymbol(symbol, values, env);

	//返回数组最后一个元素
	return std::get<double>(values[N - 1]);
}

//解释定义函数语法节点
auto interpreteDefProcAST(std::shared_ptr<ASTNode> ast, Environment* env) -> ASTResult
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
		return std::unexpected(ErrorState("error(Def proc): " + symbol + " redefined!\n"));
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

//解释系统自定义符号语法节点
auto interpretePrimitiveSymboAST(std::shared_ptr<ASTNode> ast, Environment* env) -> ASTResult
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
		TRY_AUTO(arg, interpreteAST(*childs.begin(), env));
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

//解释用户自定义符号语法节点
auto interpreteUserSymbolAST(std::shared_ptr<ASTNode> ast, Environment* env) -> ASTResult
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
				return std::unexpected(ErrorState("error(bad syntax): mismatch argument counts for function call!\n"));
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
					TRY(result, interpreteAST(*iterast, env));
					//注册第i个实参至subenv中
					registEnvSymbol(*iterpara, VarAddress(result), &subenv);
				}
				//普通表达式
				else
				{
					//求解值
					TRY(result, interpreteAST(*iterast, env));
					//注册第i个实参至subenv中
					registEnvSymbol(*iterpara, result, &subenv);
				}
				//ast的迭代器步进1
				iterast++;
			}
			//执行body函数(在subenv下)
			TRY(result, interpreteAST(body, &subenv));
		}
	}
	else
	{
		return std::unexpected(ErrorState("error(bad syntax): undefine symbol!\n"));
	}

	return result;
}

//解释二元算数运算节点的模板
//SELF -- 标志是否左值自身作为二元运算的左操作数运算后再赋给自己
//SELF为false的运算有+ - * / ^ % && || < > != ==
//SELF为true的运算有+= -= *= /=
//SHORTCUT -- 是否是短路运算
//SHORTCUT为true的运算有&& ||
//OP -- 二元运算子 运算类型为(double, double)->double
//PRED -- 是否成功短路的判断谓词 类型(double)->bool 当SHORTCUT为true时 其余情况没用用通过if constexpr优化掉相关代码
template<bool SELF, bool SHORTCUT, typename OP, typename PRED>
auto interpreteOp2AST(std::shared_ptr<ASTNode> ast, Environment* env, OP op, PRED pred) -> ASTResult
{
	double result;
	auto iter = ast->childs.begin();
	//左值自身作为二元运算的左操作数运算后再赋给自己
	if constexpr (SELF)
	{
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
					TRY_AUTO(arg, interpreteAST(*(++iter), env));
					TRY(result, op(std::get<double>(v.value()), arg));
					//更新变量在环境中的值
					setEnvSymbol(symbol, result, env);
				}
				//变量类型错误
				else
				{
					return std::unexpected(ErrorState("error(assignment): the symbol is not variable!\n"));
				}
			}
			//如果查不到这个变量则报错
			else
			{
				return std::unexpected(ErrorState("error(assignment): undefine symbol!\n"));
			}
		}
	}
	//普通二元运算
	else
	{
		if (ast->childs.size() == 2)
		{
			TRY_AUTO(val1, interpreteAST(*iter, env));
			//是否短路运算
			if constexpr (SHORTCUT)
			{
				if (pred(val1)) return val1;
			}
			TRY_AUTO(val2, interpreteAST(*(++iter), env));
			TRY(result, op(val1, val2));
		}
		else
		{
			TRY(result, interpreteAST(*iter, env));
			//结果取负
			if (ast->tk.type == TokenType::Minus)
			{
				result = -result;
			}
		}
	}
	return result;
}

//解释自增自运算(类似++、--这种)节点的模板
template<bool post, typename OP>
auto interpreteIncrementAST(std::shared_ptr<ASTNode> ast, Environment* env, OP op) -> ASTResult
{
	double prev, after;

	//获得子节点
	auto node = *(ast->childs.begin());

	//如果是用户自定义变量
	if (node->tk.type == TokenType::UserSymbol)
	{
		//获得符号名字
		auto symbol = std::get<std::string>(node->tk.value);
		if (auto v = getEnvSymbol(symbol, env))
		{
			//数值变量
			if (std::holds_alternative<double>(v.value()))
			{
				//先赋值
				prev = std::get<double>(v.value());
				after = op(prev);
				//再加1或减1更新变量在环境中的值
				setEnvSymbol(symbol, after, env);
			}
			//地址变量
			else if (std::holds_alternative<VarAddress>(v.value()))
			{
				//先赋值
				prev = std::get<VarAddress>(v.value());
				after = op(prev);
				//加1或减1更新变量在环境中的值
				setEnvSymbol(symbol, VarAddress(after), env);
			}
			//变量类型错误
			else
			{
				return std::unexpected(ErrorState("error(++ or --): operation is only supported for variable, pointer and deref expression!\n"));
			}
		}
		//如果查不到这个变量则报错
		else
		{
			return std::unexpected(ErrorState("error(++ or --): operation is only supported for variable, pointer and deref expression!\n"));
		}
	}
	//解引用
	else if (node->tk.type == TokenType::DeRef)
	{
		//获得地址表达式节点
		node = *(node->childs.begin());
		//计算表达式的值(地址)
		TRY_AUTO(daddr, interpreteAST(node, env));
		//查看是否是非负整数
		auto iaddr = (int)daddr;
		//如果daddr是非负整数
		if (iaddr >= 0 && iaddr == daddr)
		{
			//获取左侧地址的对应的变量值
			if (auto v = getEnvSymbol(VarAddress(iaddr)))
			{
				//如果计算结果是double
				if (std::holds_alternative<double>(v.value()))
				{
					//先赋值
					prev = std::get<double>(v.value());
					after = op(prev);
					//加1或减1更新变量在环境中的值
					setEnvSymbol(double(after), VarAddress(iaddr));
				}
				//如果计算结果是VarAddress
				else if (std::holds_alternative<VarAddress>(v.value()))
				{
					//先赋值
					prev = std::get<VarAddress>(v.value());
					after = op(prev);
					//加1或减1更新变量在环境中的值
					setEnvSymbol(VarAddress(after), VarAddress(iaddr));
				}
				//如果计算结果是函数
				else
				{
					return std::unexpected(ErrorState("error(Deref pointer): function can not be deref!\n"));
				}
			}
			else
			{
				return std::unexpected(ErrorState("error(Deref pointer): address out of stack top\n"));
			}
		}
		else
		{
			return std::unexpected(ErrorState("error(Deref pointer): address should be non negetive integral value\n"));
		}

	}
	//错误情形
	else
	{
		return std::unexpected(ErrorState("error(++ or --): operation is only supported for variable, pointer and deref expression!\n"));
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

//简写
using PAST = std::shared_ptr<ASTNode>;
using PENV = Environment*;
//语法节点解释映射表
static std::unordered_map<TokenType, std::function<ASTResult(PAST, PENV)>> ASTInterpreteTable
{
	{ TokenType::Plus, [](PAST ast, PENV env) { return interpreteOp2AST<false, false>(ast, env, ast_wrap(std::plus<double>()), 0); }},
	{ TokenType::Minus, [](PAST ast, PENV env) { return interpreteOp2AST<false, false>(ast, env, ast_wrap(std::minus<double>()), 0); } },
	{ TokenType::Mul, [](PAST ast, PENV env) { return interpreteOp2AST<false, false>(ast, env, ast_wrap(std::multiplies<double>()), 0); } },
	{ TokenType::Div, [](PAST ast, PENV env) { return interpreteOp2AST<false, false>(ast, env, astdiv, 0); } },
	{ TokenType::Pow, [](PAST ast, PENV env) { return interpreteOp2AST<false, false>(ast, env, astpow, 0); } },
	{ TokenType::Mod, [](PAST ast, PENV env) { return interpreteOp2AST<false, false>(ast, env, astmod, 0); } },
	{ TokenType::Less, [](PAST ast, PENV env) { return interpreteOp2AST<false, false>(ast, env, ast_wrap(std::less<double>()), 0); } },
	{ TokenType::Great, [](PAST ast, PENV env) { return interpreteOp2AST<false, false>(ast, env, ast_wrap(std::greater<double>()), 0); } },
	{ TokenType::NotLess, [](PAST ast, PENV env) { return interpreteOp2AST<false, false>(ast, env, ast_wrap(std::not_fn(std::less<double>())), 0); } },
	{ TokenType::NotGreat, [](PAST ast, PENV env) { return interpreteOp2AST<false, false>(ast, env, ast_wrap(std::not_fn(std::greater<double>())), 0); } },
	{ TokenType::Equal, [](PAST ast, PENV env) { return interpreteOp2AST<false, false>(ast, env, ast_wrap(std::equal_to<double>()), 0); } },
	{ TokenType::NotEqual, [](PAST ast, PENV env) { return interpreteOp2AST<false, false>(ast, env, ast_wrap(std::not_equal_to<double>()), 0); } },
	{ TokenType::And, [](PAST ast, PENV env) { return interpreteOp2AST<false, true>(ast, env, ast_wrap(std::logical_and<double>()), ast_wrap(std::logical_not<double>())); } },
	{ TokenType::Or, [](PAST ast, PENV env) { return interpreteOp2AST<false, true>(ast, env, ast_wrap(std::logical_or<double>()), ast_wrap(std::not_fn(std::logical_not<double>()))); } },
	{ TokenType::SelfPlus, [](PAST ast, PENV env) { return interpreteOp2AST<true, false>(ast, env, ast_wrap(std::plus<double>()), 0); } },
	{ TokenType::SelfMinus, [](PAST ast, PENV env) { return interpreteOp2AST<true, false>(ast, env, ast_wrap(std::minus<double>()), 0); } },
	{ TokenType::SelfMul, [](PAST ast, PENV env) { return interpreteOp2AST<true, false>(ast, env, ast_wrap(std::multiplies<double>()), 0); } },
	{ TokenType::SelfDiv, [](PAST ast, PENV env) { return interpreteOp2AST<true, false>(ast, env, astdiv, 0); } },
	{ TokenType::Increment, [](PAST ast, PENV env) { return interpreteIncrementAST<false>(ast, env, [](double a) { return a + 1; }); } },
	{ TokenType::PostIncrement, [](PAST ast, PENV env) { return interpreteIncrementAST<true>(ast, env, [](double a) { return a + 1; }); } },
	{ TokenType::Decrement, [](PAST ast, PENV env) { return interpreteIncrementAST<false>(ast, env, [](double a) { return a - 1; }); } },
	{ TokenType::PostDecrement, [](PAST ast, PENV env) { return interpreteIncrementAST<true>(ast, env, [](double a) { return a - 1; }); } },
	{ TokenType::Not, [](PAST ast, PENV env) -> ASTResult { TRY_AUTO(logical_result, interpreteAST(*(ast->childs.begin()), env)); return logical_result != 0;  } },
	{ TokenType::Ref, interpreteRefAST },
	{ TokenType::DeRef,[](PAST ast, PENV env) { return interpreteDeRefAST<false, false>(ast, nullptr, env, 0); } },
	{ TokenType::Assign, interpreteAssignAST },
	{ TokenType::Block, interpreteBlockAST },
	{ TokenType::If, interpreteIfAST },
	{ TokenType::While, interpreteWhileAST },
	{ TokenType::Do, interpreteDoWhileAST },
	{ TokenType::For, interpreteForAST },
	{ TokenType::DefVar, interpreteDefVarAST },
	{ TokenType::DefPointer, interpreteDefPointerAST },
	{ TokenType::DefArray, interpreteDefArrayAST },
	{ TokenType::DefProc, interpreteDefProcAST },
	{ TokenType::Number, [](PAST ast, PENV env) { return std::get<double>(ast->tk.value); } },
	{ TokenType::PrimitiveSymbol, interpretePrimitiveSymboAST },
	{ TokenType::UserSymbol, interpreteUserSymbolAST },
	{ TokenType::Break, [](PAST, PENV) -> ASTResult {  return std::unexpected(BreakState()); } },
	{ TokenType::Continue, [](PAST, PENV) -> ASTResult { return std::unexpected(ContinueState()); } },
	{ TokenType::Return, [](PAST ast, PENV env) -> ASTResult { TRY_AUTO(result, interpreteAST(*(ast->childs.begin()), env)); return std::unexpected(ReturnState(result)); }},
	{ TokenType::End, [](PAST, PENV) -> ASTResult { return 0; } },
};

//解释语法节点(总入口)
auto interpreteAST(std::shared_ptr<ASTNode> ast, Environment* env) -> ASTResult
{
	double result;
	//根据驱动表执行执行对应的函数
	if (auto iter = ASTInterpreteTable.find(ast->tk.type); iter != ASTInterpreteTable.end())
	{
		TRY(result, iter->second(ast, env));
	}
	else
	{
		return std::unexpected(ErrorState("error(bad syntax):\n"));
	}
	return abs(result) < 1e-10 ? 0 : result;
}