#include "CreateASTNode.h"

#include "AuxFacility.h"
#include <stdexcept>

//创建空语句
std::tuple<std::shared_ptr<ASTNode>, std::string> createNOpASTNode(std::string input)
{
	std::shared_ptr<ASTNode> parent;

	//获得当前第一个token
	Token tk;
	tie(tk, input) = parseToken(input);

	if (tk.type == TokenType::End)
	{
		//创建父节点
		parent = std::make_shared<ASTNode>();
		//设置父节点token
		parent->tk = tk;
	}
	else
	{
		//报一个错误
		throw std::runtime_error("error(bad syntax): not ;!\n");
	}

	return { parent, input };
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
	//正号
	else if (tk.type == TokenType::Plus)
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
	//负号
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
	//!号
	else if (tk.type == TokenType::Not)
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
				input = res1;
				//解析出左括号后面的函数输入子节点
				do
				{
					//解析输入参数
					std::shared_ptr<ASTNode> child;
					tie(child, input) = createExpressionASTNode(input);
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
		if (isoneof(op.type, TokenType::Mul, TokenType::Div, TokenType::Pow, TokenType::Mod))
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
std::tuple<std::shared_ptr<ASTNode>, std::string> createArithmeticASTNode(std::string input)
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

//创建关系比较的语法树节点
std::tuple<std::shared_ptr<ASTNode>, std::string> createRelationASTNode(std::string input)
{
	std::shared_ptr<ASTNode> parent, child1, child2;
	//获得第一个项的节点并存储在父节点位置
	std::tie(parent, input) = createArithmeticASTNode(input);

	while (true)
	{
		//获取符号
		auto[op, str] = parseToken(input);
		//加号或者减号
		if (isoneof(op.type, TokenType::Less, TokenType::Great, TokenType::NotLess, TokenType::NotGreat, TokenType::Equal, TokenType::NotEqual))
		{
			//把父节点搬移到子节点1的位置
			child1 = parent;
			//获取子节点2
			std::tie(child2, input) = createArithmeticASTNode(str);
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

//创建逻辑运算的语法树节点
std::tuple<std::shared_ptr<ASTNode>, std::string> createLogicASTNode(std::string input)
{
	std::shared_ptr<ASTNode> parent, child1, child2;
	//获得第一个项的节点并存储在父节点位置
	std::tie(parent, input) = createRelationASTNode(input);

	while (true)
	{
		//获取符号
		auto[op, str] = parseToken(input);
		//加号或者减号
		if (isoneof(op.type, TokenType::And, TokenType::Or))
		{
			//把父节点搬移到子节点1的位置
			child1 = parent;
			//获取子节点2
			std::tie(child2, input) = createRelationASTNode(str);
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
	//获得前两个token
	Token tk;
	std::string str;
	std::tie(tk, str) = parseToken(input);

	std::shared_ptr<ASTNode> parent;

	//如果是变量定义符号
	if (tk.type == TokenType::DefVar)
	{
		//读取定义的变量名
		Token tkname;
		std::tie(tkname, input) = parseToken(str);
		//如果不是用户自定义的符号
		if (tkname.type != TokenType::UserSymbol)
		{
			//报一个错误
			throw std::runtime_error("error(Def var): need a symbol to reprensent variable name!\n");
		}

		//读取赋值符号
		std::tie(tk, input) = parseToken(input);

		//如果不是赋值符号
		if (tk.type != TokenType::Assign)
		{
			//报一个错误
			throw std::runtime_error("error(Def var): miss = for defining varible!\n");
		}

		//创建赋值父节点
		parent = std::make_shared<ASTNode>();
		//结点类型为赋值
		parent->tk.type = TokenType::DefVar;
		//创建代表被赋值变量的子节点
		auto child = std::make_shared<ASTNode>();
		child->tk = tkname;
		//child与parent连接
		parent->childs.push_back(child);
		//创建代表赋值=号右边表达式的语句
		std::tie(child, input) = createExpressionASTNode(input);
		//child与parent连接
		parent->childs.push_back(child);
	}
	//如果是用户自定义的符号 
	else if (tk.type == TokenType::UserSymbol)
	{
		//则继续再度一个字符
		Token tktemp;
		std::tie(tktemp, str) = parseToken(str);
		//如果是=、+=、-=、*=、/=
		if (isoneof(tktemp.type, TokenType::Assign, TokenType::SelfPlus, TokenType::SelfMinus, TokenType::SelfMul, TokenType::SelfDiv))
		{
			//创建赋值父节点
			parent = std::make_shared<ASTNode>();
			//结点类型为赋值
			parent->tk.type = tktemp.type;
			//创建代表被赋值变量的子节点
			auto child = std::make_shared<ASTNode>();
			child->tk = tk;
			//child与parent连接
			parent->childs.push_back(child);
			//创建代表赋值=号右边表达式的语句
			std::tie(child, input) = createExpressionASTNode(str);
			//child与parent连接
			parent->childs.push_back(child);
		}
		else
		{
			//其他情况下用逻辑运算语句解析
			std::tie(parent, input) = createLogicASTNode(input);
		}
	}
	else
	{
		//其他情况下用逻辑运算语句解析
		std::tie(parent, input) = createLogicASTNode(input);
	}

	return { parent, input };
}

//创建条件语句的语法树
std::tuple<std::shared_ptr<ASTNode>, std::string> createIfASTNode(std::string input)
{
	//解析第一个tk
	Token tk;
	std::tie(tk, input) = parseToken(input);

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
	std::tie(tk, input) = parseToken(input);
	if (tk.type != TokenType::Lp)
	{
		//报一个错误
		throw std::runtime_error("error(bad syntax): if condition need a (!\n");
	}

	//读取条件
	decltype(parent) ifcondition;
	std::tie(ifcondition, input) = createExpressionASTNode(input);

	//添加为parent的第1个儿子
	parent->childs.push_back(ifcondition);

	//读取一个右括号
	std::tie(tk, input) = parseToken(input);
	if (tk.type != TokenType::Rp)
	{
		//报一个错误
		throw std::runtime_error("error(bad syntax): if condition need a )!\n");
	}

	//读取if成立的block
	decltype(parent) ifblock;
	std::tie(ifblock, input) = createBlockASTNode(input);

	//添加为parent的第2个儿子
	parent->childs.push_back(ifblock);

	//读取下一个token
	std::string str;
	std::tie(tk, str) = parseToken(input);
	//如果是else则继续添加else分支的结果
	//如果是elseif则看作一个新的if递归即可
	if (tk.type == TokenType::ElseIf)
	{
		decltype(parent) elseifnode;
		std::tie(elseifnode, input) = createElseIfASTNode(input);
		//添加为parent的第3个儿子
		parent->childs.push_back(elseifnode);
	}
	else if (tk.type == TokenType::Else)
	{
		decltype(parent) elseblock;
		std::tie(elseblock, input) = createBlockASTNode(str);
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
	std::tie(tk, input) = parseToken(input);

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
	std::tie(tk, input) = parseToken(input);
	if (tk.type != TokenType::Lp)
	{
		//报一个错误
		throw std::runtime_error("error(bad syntax): else if condition need a (!\n");
	}

	//读取条件
	decltype(parent) ifcondition;
	std::tie(ifcondition, input) = createExpressionASTNode(input);

	//添加为parent的第1个儿子
	parent->childs.push_back(ifcondition);

	//读取一个右括号
	std::tie(tk, input) = parseToken(input);
	if (tk.type != TokenType::Rp)
	{
		//报一个错误
		throw std::runtime_error("error(bad syntax): else if condition need a )!\n");
	}

	//读取if成立的block
	decltype(parent) ifblock;
	std::tie(ifblock, input) = createBlockASTNode(input);

	//添加为parent的第2个儿子
	parent->childs.push_back(ifblock);

	//读取下一个token
	std::string str;
	std::tie(tk, str) = parseToken(input);
	//如果是else则继续添加else分支的结果
	//如果是elseif则看作一个新的if递归即可
	if (tk.type == TokenType::ElseIf)
	{
		decltype(parent) elseifnode;
		std::tie(elseifnode, input) = createElseIfASTNode(str);
		//添加为parent的第3个儿子
		parent->childs.push_back(elseifnode);
	}
	else if (tk.type == TokenType::Else)
	{
		decltype(parent) elseblock;
		std::tie(elseblock, input) = createBlockASTNode(str);
		//添加为parent的第3个儿子
		parent->childs.push_back(elseblock);
	}

	return { parent, input };
}

//创建while语句的语法树
std::tuple<std::shared_ptr<ASTNode>, std::string> createWhileASTNode(std::string input)
{
	//解析第一个tk
	Token tk;
	std::tie(tk, input) = parseToken(input);

	//如果不是while则报错
	if (tk.type != TokenType::While)
	{
		//报一个错误
		throw std::runtime_error("error(bad syntax): miss keyword while!\n");
	}

	//构建父节点
	auto parent = std::make_shared<ASTNode>();
	parent->tk = tk;

	//读取一个左括号
	std::tie(tk, input) = parseToken(input);
	if (tk.type != TokenType::Lp)
	{
		//报一个错误
		throw std::runtime_error("error(bad syntax): while loop need a (!\n");
	}

	//循环条件
	decltype(parent) loopcondition;
	std::tie(loopcondition, input) = createExpressionASTNode(input);

	//添加为parent的第1个儿子
	parent->childs.push_back(loopcondition);

	//读取一个右括号
	std::tie(tk, input) = parseToken(input);
	if (tk.type != TokenType::Rp)
	{
		//报一个错误
		throw std::runtime_error("error(bad syntax): while loop need a )!\n");
	}

	//读取while的循环体
	decltype(parent) loopblock;
	std::tie(loopblock, input) = createBlockASTNode(input);

	//添加为parent的第2个儿子
	parent->childs.push_back(loopblock);

	return { parent, input };
}

//创建for语句的语法树
std::tuple<std::shared_ptr<ASTNode>, std::string> createForASTNode(std::string input)
{
	//解析第一个tk
	Token tk;
	std::tie(tk, input) = parseToken(input);

	//如果不是while则报错
	if (tk.type != TokenType::For)
	{
		//报一个错误
		throw std::runtime_error("error(bad syntax): miss keyword for!\n");
	}

	//构建父节点
	auto parent = std::make_shared<ASTNode>();
	parent->tk = tk;

	//读取一个左括号
	std::tie(tk, input) = parseToken(input);
	if (tk.type != TokenType::Lp)
	{
		//报一个错误
		throw std::runtime_error("error(bad syntax): for loop need a (!\n");
	}

	//循环起始
	decltype(parent) start;
	std::tie(start, input) = createExpressionASTNode(input);

	//添加为parent的第1个儿子
	parent->childs.push_back(start);

	//读取一个;
	std::tie(tk, input) = parseToken(input);
	if (tk.type != TokenType::End)
	{
		//报一个错误
		throw std::runtime_error("error(bad syntax): for loop need a ;!\n");
	}

	//循环终止条件
	decltype(parent) end;
	std::tie(end, input) = createExpressionASTNode(input);

	//添加为parent的第2个儿子
	parent->childs.push_back(end);

	//读取一个;
	std::tie(tk, input) = parseToken(input);
	if (tk.type != TokenType::End)
	{
		//报一个错误
		throw std::runtime_error("error(bad syntax): for loop need a ;!\n");
	}

	//循环步进
	decltype(parent) increment;
	std::tie(increment, input) = createExpressionASTNode(input);

	//添加为parent的第3个儿子
	parent->childs.push_back(increment);

	//读取一个右括号
	std::tie(tk, input) = parseToken(input);
	if (tk.type != TokenType::Rp)
	{
		//报一个错误
		throw std::runtime_error("error(bad syntax): for loop need a )!\n");
	}

	//读取for的循环体
	decltype(parent) loopblock;
	std::tie(loopblock, input) = createBlockASTNode(input);

	//添加为parent的第4个儿子
	parent->childs.push_back(loopblock);

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
			throw std::runtime_error("error(define proc): miss a )!\n");
		}
	}

	//读取赋值符号
	std::tie(tk, input) = parseToken(input);
	//如果类型不是=号则报错
	if (tk.type != TokenType::Assign)
	{
		throw std::runtime_error("error(define proc): need a =!\n");
	}

	//构建节点
	auto parent = std::make_shared<ASTNode>();
	parent->tk.type = TokenType::DefProc;
	//获取并添加函数体
	decltype(parent) body;
	std::tie(body, input) = createBlockASTNode(input);
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
std::tuple<std::shared_ptr<ASTNode>, std::string> createBlockASTNode(std::string input)
{
	//解析第一个tk
	Token tk;
	std::tie(tk, input) = parseToken(input);
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
		std::tie(tk, str) = parseToken(input);
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
		std::tie(tk, str) = parseToken(input);
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
	std::tie(tk, str) = parseToken(input);

	//定义函数
	if (tk.type == TokenType::DefProc)
	{
		std::tie(parent, input) = createDefProcASTNode(input);
	}
	//if
	else if (tk.type == TokenType::If)
	{
		//用if语句的语法解析
		std::tie(parent, input) = createIfASTNode(input);
	}
	//while
	else if (tk.type == TokenType::While)
	{
		//用while语句的语法解析
		std::tie(parent, input) = createWhileASTNode(input);
	}
	//for
	else if (tk.type == TokenType::For)
	{
		//用For语句的语法解析
		std::tie(parent, input) = createForASTNode(input);
	}
	//语句块
	else if (tk.type == TokenType::LBrace)
	{
		//用语句块的语法解析
		std::tie(parent, input) = createBlockASTNode(input);
	}
	//;号
	else if (tk.type == TokenType::End)
	{
		//用语句块的语法解析
		std::tie(parent, input) = createNOpASTNode(input);
	}
	else
	{
		//其他情况下用表达式语句解析
		std::tie(parent, input) = createExpressionASTNode(input);
	}

	return { parent, input };
}