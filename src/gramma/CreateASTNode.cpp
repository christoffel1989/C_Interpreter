#include "CreateASTNode.h"

#include "token.h"

//定义一些简化书写的模板
//一个都不等
template <typename T>
bool isnoneof(T t, T arg)
{
	return t != arg;
}
template <typename T, typename... Ts>
bool isnoneof(T t, T arg, Ts... args)
{
	return (t != arg) && isnoneof(t, args...);
}
//等于其中一个
template <typename T>
bool isoneof(T t, T arg)
{
	return t == arg;
}
template <typename T, typename... Ts>
bool isoneof(T t, T arg, Ts... args)
{
	return (t == arg) || isoneof(t, args...);
}

//创建因子的语法树节点
std::tuple<std::shared_ptr<ASTNode>, std::string> createFactorASTNode(std::string input)
{
	std::shared_ptr<ASTNode> parent;

	//获得当前第一个token
	Token tk;
	tie(tk, input) = parseToken(input);

	//根据token的类型区别进行不同情况的处理
	if (tk.type == TokenType::Number)
	{
		//创建父节点
		parent = std::make_shared<ASTNode>();
		//设置父节点token
		parent->tk = tk;
	}
	//符号
	else if (tk.type == TokenType::Minus)
	{
		//创建父节点
		parent = std::make_shared<ASTNode>();
		//设置父节点token
		parent->tk = tk;
		//创建一个子节点是接下来的一个符号
		std::shared_ptr<ASTNode> child;
		std::tie(child, input) = createFactorASTNode(input);
		parent->childs.push_back(child);
	}
	//左括号
	else if (tk.type == TokenType::Lp)
	{
		//括号中的表达式
		tie(parent, input) = createExpressionASTNode(input);
		//解析右括号
		tie(tk, input) = parseToken(input);
		//没有右括号则报错
		if (tk.type != TokenType::Rp)
		{
			//报一个错误
			throw std::runtime_error("error(bad syntax): miss a )!\n");
		}
	}
	//Primitive Symbol
	else if (tk.type == TokenType::PrimitiveSymbol)
	{
		//创建父节点
		parent = std::make_shared<ASTNode>();
		//设置父节点token
		parent->tk = tk;

		//根据symbol的类型设置子节点
		auto symbol = std::get<std::string>(tk.value);
		auto primitive = getPrimitiveSymbol(symbol).value();
		//如果是1元函数
		if (std::holds_alternative<std::function<double(double)>>(primitive))
		{
			//解析左括号
			tie(tk, input) = parseToken(input);
			if (tk.type != TokenType::Lp)
			{
				//报一个错误
				throw std::runtime_error("error(bad syntax): function call miss a (!\n");
			}
			//解析括号中的函数输入参量
			std::shared_ptr<ASTNode> child;
			std::tie(child, input) = createExpressionASTNode(input);
			//解析右括号
			tie(tk, input) = parseToken(input);
			if (tk.type != TokenType::Rp)
			{
				//报一个错误
				throw std::runtime_error("error(bad syntax): not enough arguments for function call or function call miss a )!\n");
			}
			//将链接父和子节点
			parent->childs.push_back(child);
		}
	}
	//自定义Symbol
	else if (tk.type == TokenType::UserSymbol)
	{
		//获得symbol字符串
		auto symbol = std::get<std::string>(tk.value);

		//创建父节点
		parent = std::make_shared<ASTNode>();
		//设置父节点token
		parent->tk = tk;

		//解析下一个字符
		std::string res1, res2;
		tie(tk, res1) = parseToken(input);
		//如果是左括号
		if (tk.type == TokenType::Lp)
		{
			//解析下一个字符
			tie(tk, res2) = parseToken(res1);
			//不是右括号表示里面有函数输入参数
			if (tk.type != TokenType::Rp)
			{
				//解析出左括号后面的函数输入子节点
				do
				{
					//解析输入参数
					std::shared_ptr<ASTNode> child;
					tie(child, input) = createExpressionASTNode(res1);
					parent->childs.push_back(child);
					//解析逗号(如果不是逗号则循环中断)
					tie(tk, input) = parseToken(input);
				} while (tk.type == TokenType::Comma);

				//如果不是右括号则报错
				if (tk.type != TokenType::Rp)
				{
					//报一个错误
					throw std::runtime_error("error(bad syntax): not enough arguments for function call or function call miss a )!\n");
				}
			}
		}
	}
	else
	{
		//报一个错误
		throw std::runtime_error("error(bad syntax): need a number or a (!\n");
	}

	return { parent, input };
}

//创建项的语法树节点
std::tuple<std::shared_ptr<ASTNode>, std::string> createTermASTNode(std::string input)
{
	std::shared_ptr<ASTNode> parent, child1, child2;
	//获得第一个项的节点并存储在父节点位置
	std::tie(parent, input) = createFactorASTNode(input);

	while (true)
	{
		//获取符号
		auto[op, str] = parseToken(input);
		//乘号、除号或者阶乘
		if (isoneof(op.type, TokenType::Mul, TokenType::Div, TokenType::Pow))
		{
			//把父节点搬移到子节点1的位置
			child1 = parent;
			//获取子节点2
			std::tie(child2, input) = createFactorASTNode(str);
			//创建父节点
			parent = std::make_shared<ASTNode>();
			//设置父节点token
			parent->tk = op;
			//子节点1和2与父节点相连
			parent->childs.push_back(child1);
			parent->childs.push_back(child2);
		}
		//其他token
		else
		{
			//退出循环
			break;
		}
	}

	return { parent, input };
}

//创建表达式的语法树节点
std::tuple<std::shared_ptr<ASTNode>, std::string> createExpressionASTNode(std::string input)
{
	std::shared_ptr<ASTNode> parent, child1, child2;
	//获得第一个项的节点并存储在父节点位置
	std::tie(parent, input) = createTermASTNode(input);

	while (true)
	{
		//获取符号
		auto[op, str] = parseToken(input);
		//加号或者减号
		if (isoneof(op.type, TokenType::Plus, TokenType::Minus))
		{
			//把父节点搬移到子节点1的位置
			child1 = parent;
			//获取子节点2
			std::tie(child2, input) = createTermASTNode(str);
			//创建父节点
			parent = std::make_shared<ASTNode>();
			//设置父节点token
			parent->tk = op;
			//子节点1和2与父节点相连
			parent->childs.push_back(child1);
			parent->childs.push_back(child2);
		}
		//其他token
		else
		{
			//退出循环
			break;
		}
	}

	return { parent, input };
}

//创建赋值语句的语法树节点
std::tuple<std::shared_ptr<ASTNode>, std::string> createAssignmentASTNode(std::string input)
{
	//获得前两个token
	Token tk1, tk2;
	std::string str;
	tie(tk1, str) = parseToken(input);
	tie(tk2, str) = parseToken(str);

	//第1、2个tk不同时是user symbo 和 = 号则报错
	if (tk1.type != TokenType::UserSymbol || tk2.type != TokenType::Assign)
	{
		//报一个错误
		throw std::runtime_error("error(assignment): const value can not be assign!\n");
	}

	//创建赋值父节点
	auto parent = std::make_shared<ASTNode>();
	//结点类型为赋值
	parent->tk.type = TokenType::Assign;
	//创建代表被赋值变量的子节点
	auto child = std::make_shared<ASTNode>();
	child->tk = tk1;
	//child与parent连接
	parent->childs.push_back(child);
	//创建代表赋值=号右边表达式的语句
	std::tie(child, input) = createExpressionASTNode(str);
	//child与parent连接
	parent->childs.push_back(child);

	return { parent, input };
}

//创建逻辑判断的语法树节点
std::tuple<std::shared_ptr<ASTNode>, std::string> createLogicASTNode(std::string input)
{
	std::shared_ptr<ASTNode> child1, child2, parent;

	//读取表达式1
	std::tie(child1, input) = createExpressionASTNode(input);

	//读取逻辑运算符号
	Token tk;
	std::tie(tk, input) = parseToken(input);

	//读取表达式2
	std::tie(child2, input) = createExpressionASTNode(input);

	//构造父节点
	parent = std::make_shared<ASTNode>();
	parent->tk = tk;
	parent->childs.push_back(child1);
	parent->childs.push_back(child2);

	return { parent, input };
}

//创建条件语句的语法树
std::tuple<std::shared_ptr<ASTNode>, std::string> createIfASTNode(std::string input)
{
	//解析第一个tk
	Token tk;
	tie(tk, input) = parseToken(input);

	//如果不是if则报错
	if (tk.type != TokenType::If)
	{
		//报一个错误
		throw std::runtime_error("error(bad syntax): miss keyword if!\n");
	}

	//构建父节点
	auto parent = std::make_shared<ASTNode>();
	parent->tk = tk;

	//读取一个左括号
	tie(tk, input) = parseToken(input);
	if (tk.type != TokenType::Lp)
	{
		//报一个错误
		throw std::runtime_error("error(bad syntax): if condition need a (!\n");
	}

	//读取条件
	decltype(parent) ifcondition;
	tie(ifcondition, input) = createLogicASTNode(input);

	//添加为parent的第1个儿子
	parent->childs.push_back(ifcondition);

	//读取一个右括号
	tie(tk, input) = parseToken(input);
	if (tk.type != TokenType::Rp)
	{
		//报一个错误
		throw std::runtime_error("error(bad syntax): if condition need a )!\n");
	}

	//读取if成立的block
	decltype(parent) ifblock;
	tie(ifblock, input) = createBlocksASTNode(input);

	//添加为parent的第2个儿子
	parent->childs.push_back(ifblock);

	//读取下一个token
	std::string str;
	tie(tk, str) = parseToken(input);
	//如果是else则继续添加else分支的结果
	//如果是elseif则看作一个新的if递归即可
	if (tk.type == TokenType::ElseIf)
	{
		decltype(parent) elseifnode;
		tie(elseifnode, input) = createElseIfASTNode(input);
		//添加为parent的第3个儿子
		parent->childs.push_back(elseifnode);
	}
	else if (tk.type == TokenType::Else)
	{
		decltype(parent) elseblock;
		tie(elseblock, input) = createBlocksASTNode(str);
		//添加为parent的第3个儿子
		parent->childs.push_back(elseblock);
	}

	return { parent, input };
}

//创建条件语句的语法树
std::tuple<std::shared_ptr<ASTNode>, std::string> createElseIfASTNode(std::string input)
{
	//解析第一个tk
	Token tk;
	tie(tk, input) = parseToken(input);

	//如果不是if则报错
	if (tk.type != TokenType::ElseIf)
	{
		//报一个错误
		throw std::runtime_error("error(bad syntax): miss keyword elseif!\n");
	}

	//构建父节点
	auto parent = std::make_shared<ASTNode>();
	parent->tk.type = TokenType::If;

	//读取一个左括号
	tie(tk, input) = parseToken(input);
	if (tk.type != TokenType::Lp)
	{
		//报一个错误
		throw std::runtime_error("error(bad syntax): else if condition need a (!\n");
	}

	//读取条件
	decltype(parent) ifcondition;
	tie(ifcondition, input) = createLogicASTNode(input);

	//添加为parent的第1个儿子
	parent->childs.push_back(ifcondition);

	//读取一个右括号
	tie(tk, input) = parseToken(input);
	if (tk.type != TokenType::Rp)
	{
		//报一个错误
		throw std::runtime_error("error(bad syntax): else if condition need a )!\n");
	}

	//读取if成立的block
	decltype(parent) ifblock;
	tie(ifblock, input) = createBlocksASTNode(input);

	//添加为parent的第2个儿子
	parent->childs.push_back(ifblock);

	//读取下一个token
	std::string str;
	tie(tk, str) = parseToken(input);
	//如果是else则继续添加else分支的结果
	//如果是elseif则看作一个新的if递归即可
	if (tk.type == TokenType::ElseIf)
	{
		decltype(parent) elseifnode;
		tie(elseifnode, input) = createElseIfASTNode(str);
		//添加为parent的第3个儿子
		parent->childs.push_back(elseifnode);
	}
	else if (tk.type == TokenType::Else)
	{
		decltype(parent) elseblock;
		tie(elseblock, input) = createBlocksASTNode(str);
		//添加为parent的第3个儿子
		parent->childs.push_back(elseblock);
	}

	return { parent, input };
}

//创建定义变量语句的语法数节点
std::tuple<std::shared_ptr<ASTNode>, std::string> createDefVarASTNode(std::string input)
{
	//获得前三个token
	Token tk1, tk2, tk3;
	std::string str;
	tie(tk1, str) = parseToken(input);
	tie(tk2, str) = parseToken(str);
	tie(tk3, str) = parseToken(str);

	//第1、2、3个tk不同时是DefVar、user symbo 和 = 号则报错
	if (tk1.type != TokenType::DefVar || tk2.type != TokenType::UserSymbol || tk3.type != TokenType::Assign)
	{
		//报一个错误
		throw std::runtime_error("error(Def var): illegal syntax for define variable!\n");
	}

	//创建赋值父节点
	auto parent = std::make_shared<ASTNode>();
	//结点类型为赋值
	parent->tk.type = TokenType::DefVar;
	//创建代表被赋值变量的子节点
	auto child = std::make_shared<ASTNode>();
	child->tk = tk2;
	//child与parent连接
	parent->childs.push_back(child);
	//创建代表赋值=号右边表达式的语句
	std::tie(child, input) = createExpressionASTNode(str);
	//child与parent连接
	parent->childs.push_back(child);

	return { parent, input };
}

//创建定义过程的语法树节点
std::tuple<std::shared_ptr<ASTNode>, std::string> createDefProcASTNode(std::string input)
{
	//获得前三个token
	Token tk1, tk2, tk3;
	std::tie(tk1, input) = parseToken(input);
	std::tie(tk2, input) = parseToken(input);
	std::tie(tk3, input) = parseToken(input);

	//第1、2、3个tk不同时是DefProc、user symbo 和 ( 号则报错
	if (tk1.type != TokenType::DefProc || tk2.type != TokenType::UserSymbol || tk3.type != TokenType::Lp)
	{
		//报一个错误
		throw std::runtime_error("error(Def proc): illegal syntax for define pro!\n");
	}

	//读取函数输入形式参量
	std::list<Token> paras;
	auto[tk, res] = parseToken(input);

	//0输入参量
	if (tk.type == TokenType::Rp)
	{
		input = res;
	}
	//大于等1个输入参量
	else
	{
		do
		{
			//读取参量
			std::tie(tk, input) = parseToken(input);
			//不是用户自定义变量则报错
			if (tk.type != TokenType::UserSymbol)
			{
				throw std::runtime_error("error(define proc): need a symbo for proc para name!\n");
			}
			//注册信息
			paras.push_back(tk);
			//再读取一个token
			std::tie(tk, input) = parseToken(input);
		} while (tk.type == TokenType::Comma);

		//判断停止while后的符号是不是右括号 如果不是则报错
		if (tk.type != TokenType::Rp)
		{
			throw std::runtime_error("error(define var): miss a )!\n");
		}
	}

	//读取赋值符号
	tie(tk, input) = parseToken(input);
	//如果类型不是=号则报错
	if (tk.type != TokenType::Assign)
	{
		throw std::runtime_error("error(define var): need a =!\n");
	}

	//构建节点
	auto parent = std::make_shared<ASTNode>();
	parent->tk.type = TokenType::DefProc;
	//获取并添加函数体
	decltype(parent) body;
	std::tie(body, input) = createBlocksASTNode(input);
	parent->childs.push_back(body);
	//添加函数名字节点
	auto procname = std::make_shared<ASTNode>();
	procname->tk = tk2;
	parent->childs.push_back(procname);
	//添加函数参数
	for (auto iter = paras.begin(); iter != paras.end(); iter++)
	{
		auto para = std::make_shared<ASTNode>();
		para->tk = *iter;
		parent->childs.push_back(para);
	}
	return { parent, input };
}

//创建语句块的语法树
std::tuple<std::shared_ptr<ASTNode>, std::string> createBlocksASTNode(std::string input)
{
	//解析第一个tk
	Token tk;
	tie(tk, input) = parseToken(input);
	std::string str;

	//如果不是左大括号则报错
	if (tk.type != TokenType::LBrace)
	{
		//报一个错误
		throw std::runtime_error("error(bad syntax): miss a {!\n");
	}

	auto parent = std::make_shared<ASTNode>();
	parent->tk.type = TokenType::Block;
	//不停的解析语句
	do
	{
		std::shared_ptr<ASTNode> child;
		std::tie(child, input) = createStatementASTNode(input);
		parent->childs.push_back(child);
		//再解析一个tk
		tie(tk, str) = parseToken(input);
		//不是Block、IF、While、For等特殊语句的情况下
		if (isnoneof(child->tk.type, TokenType::Block, TokenType::If, TokenType::While, TokenType::For))
		{
			if (tk.type != TokenType::End)
			{
				//结尾不是;语法错误
				throw std::runtime_error("error(bad syntax): miss a ;!\n");
			}
			else
			{
				input = str;
			}
		}
		//再解析一个tk
		tie(tk, str) = parseToken(input);
	} while (tk.type != TokenType::RBrace);

	//如果不是右大括号则报错
	if (tk.type != TokenType::RBrace)
	{
		//报一个错误
		throw std::runtime_error("error(bad syntax): miss a }!\n");
	}

	input = str;

	return { parent, input };
}

//创建以;号结尾的一般语句的语法树
std::tuple<std::shared_ptr<ASTNode>, std::string> createStatementASTNode(std::string input)
{
	std::shared_ptr<ASTNode> parent;

	//解析第一个tk
	Token tk;
	std::string str;
	tie(tk, str) = parseToken(input);

	//定义变量
	if (tk.type == TokenType::DefVar)
	{
		//原输入重新用定义变量语句去解析
		std::tie(parent, input) = createDefVarASTNode(input);
	}
	//定义函数
	else if (tk.type == TokenType::DefProc)
	{
		std::tie(parent, input) = createDefProcASTNode(input);
	}
	//if
	else if (tk.type == TokenType::If)
	{
		//用条件语句的语法解析
		std::tie(parent, input) = createIfASTNode(input);
	}
	//语句块
	else if (tk.type == TokenType::LBrace)
	{
		//用语句块的语法解析
		std::tie(parent, input) = createBlocksASTNode(input);
	}
	else
	{
		//如果是用户自定义的符号 
		if (tk.type == TokenType::UserSymbol)
		{
			//则继续再度一个字符
			tie(tk, str) = parseToken(str);
			//如果是赋值
			if (tk.type == Assign)
			{
				//原输入重新用赋值语句去解析
				std::tie(parent, input) = createAssignmentASTNode(input);
			}
			else
			{
				//其他情况下用表达式语句解析
				std::tie(parent, input) = createExpressionASTNode(input);
			}
		}
		else
		{
			//其他情况下用表达式语句解析
			std::tie(parent, input) = createExpressionASTNode(input);
		}
	}

	return { parent, input };
}