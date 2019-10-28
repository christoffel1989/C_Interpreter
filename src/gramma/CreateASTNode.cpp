﻿#include "CreateASTNode.h"

#include "AuxFacility.h"

#include <unordered_map>
#include <stdexcept>

//变长参数模板 没有获得预期type的token则报错 报错内容为error(以异常的形式抛出)
template <typename... TS>
inline std::tuple<Token, std::string> expectToken(std::string input, std::string error, TS... expects) noexcept(false)
{
	auto pack = parseToken(std::move(input));
	if (isnoneof(std::get<0>(pack).type, expects...))
	{
		throw std::runtime_error(error + "\n");
	}
	return pack;
}

//创建空语句
std::tuple<std::shared_ptr<ASTNode>, std::string> createNOpASTNode(std::string input)
{
	//读取分号
	std::tie(std::ignore, input) = expectToken(input, "error(bad syntax): not ;!\n", TokenType::End);

	//创建父节点
	auto parent = std::make_shared<ASTNode>();
	//设置父节点token
	parent->tk.type = TokenType::End;

	return { parent, input };
}

//创建解引用表达式
std::tuple<std::shared_ptr<ASTNode>, std::string> createDeRefASTNode(std::string input)
{
	Token tk;

	//读取解引用符号(*)
	std::tie(tk, input) = expectToken(input, "error(bad syntax): miss a *!", TokenType::Mul);

	//读取自定义变量或者左括号
	std::tie(tk, input) = expectToken(input, "error(bad syntax): dereference(*) must be followed by variable or a (arithmetic expression)!", TokenType::UserSymbol, TokenType::Lp);

	//创建父节点
	auto parent = std::make_shared<ASTNode>();
	//设置父节点token
	parent->tk.type = TokenType::DeRef;
	//定义子节点变量
	decltype(parent) child;

	//如果读取出来的是自定义变量
	if (tk.type == TokenType::UserSymbol)
	{
		//子节点存储自加的变量名
		child = std::make_shared<ASTNode>();
		child->tk = tk;
	}
	//读出来是括号
	else
	{
		//解析一个算数表达式
		std::tie(child, input) = createArithmeticASTNode(input);
		//再解析一个右括号
		std::tie(tk, input) = expectToken(input, "error(bad syntax): miss a )!", TokenType::Rp);
	}

	parent->childs.push_back(child);

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
	//++
	else if (tk.type == TokenType::PlusPlus)
	{
		//读取自定义变量
		std::tie(tk, input) = expectToken(input, "error(bad syntax): ++ must be followed by variable!", TokenType::UserSymbol);

		//创建父节点
		parent = std::make_shared<ASTNode>();
		//设置父节点token
		parent->tk.type = TokenType::Increment;
		//创建一个子节点存储自加的变量名
		auto child = std::make_shared<ASTNode>();
		child->tk = tk;
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
	//--
	else if (tk.type == TokenType::MinusMinus)
	{
		//读取自定义变量
		std::tie(tk, input) = expectToken(input, "error(bad syntax): -- must be followed by variable!", TokenType::UserSymbol);

		//创建父节点
		parent = std::make_shared<ASTNode>();
		//设置父节点token
		parent->tk.type = TokenType::Decrement;
		//创建一个子节点存储自加的变量名
		auto child = std::make_shared<ASTNode>();
		child->tk = tk;
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
	//&变量求地址
	else if (tk.type == TokenType::And)
	{
		//读取自定义变量
		std::tie(tk, input) = expectToken(input, "error(bad syntax): reference(&) must be followed by variable!", TokenType::UserSymbol);

		//创建父节点
		parent = std::make_shared<ASTNode>();
		//设置父节点token
		parent->tk.type = TokenType::Ref;
		//创建一个子节点存储自加的变量名
		auto child = std::make_shared<ASTNode>();
		child->tk = tk;
		parent->childs.push_back(child);
	}
	//*变量解引用
	else if (tk.type == TokenType::Mul)
	{
		//把*号补回去再解析
		std::tie(parent, input) = createDeRefASTNode("*" + input);
	}
	//左括号
	else if (tk.type == TokenType::Lp)
	{
		//括号中的表达式
		tie(parent, input) = createExpressionASTNode(input);

		//读取右括号
		std::tie(tk, input) = expectToken(input, "error(bad syntax): miss a )!", TokenType::Rp);
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
			//读取左括号
			std::tie(tk, input) = expectToken(input, "error(bad syntax): function call miss a (!", TokenType::Lp);

			//解析括号中的函数输入参量
			std::shared_ptr<ASTNode> child;
			std::tie(child, input) = createExpressionASTNode(input);

			//读取右括号
			std::tie(tk, input) = expectToken(input, "error(bad syntax): not enough arguments for function call or function call miss a )!", TokenType::Rp);

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
			//无输入参数 0元函数
			else
			{
				input = res2;
			}
		}
		//如果是++
		else if (isoneof(tk.type, TokenType::PlusPlus, TokenType::MinusMinus))
		{
			auto child = parent;
			parent = std::make_shared<ASTNode>();
			parent->tk.type = (tk.type == TokenType::PlusPlus) ? TokenType::PostIncrement : TokenType::PostDecrement;
			parent->childs.push_back(child);
			//补回丢掉的变量
			input = res1;
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
		if (isoneof(op.type, TokenType::AndAnd, TokenType::OrOr))
		{
			//把父节点搬移到子节点1的位置
			child1 = parent;
			//获取子节点2
			std::tie(child2, input) = createRelationASTNode(str);
			//创建父节点
			parent = std::make_shared<ASTNode>();
			//设置父节点token (parse和translate的时候And、Or的含义不同 AndAnd变成与(And) And变成取地址(Address))
			parent->tk.type = (op.type == TokenType::AndAnd) ? TokenType::And : TokenType::Or;
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
	//获得第一个token
	Token tk;
	std::string str;
	std::tie(tk, str) = parseToken(input);

	std::shared_ptr<ASTNode> parent;

	//如果是变量定义符号
	if (tk.type == TokenType::DefVar)
	{
		//读取定义的变量名或者是*  当为*时代表定义的是指针类型变量
		std::tie(tk, input) = expectToken(str, "error(Def var or pointer): need ( or a symbol to reprensent variable name !", TokenType::UserSymbol, TokenType::Mul);

		//定义非指针类型变量
		if (tk.type == TokenType::UserSymbol)
		{
			//读取赋值符号
			std::tie(std::ignore, input) = expectToken(input, "error(Def var): miss = for defining varible!", TokenType::Assign);

			//创建赋值父节点
			parent = std::make_shared<ASTNode>();
			//结点类型为赋值
			parent->tk.type = TokenType::DefVar;
			//创建代表被赋值变量的子节点
			auto child = std::make_shared<ASTNode>();
			child->tk = tk;
			//child与parent连接
			parent->childs.push_back(child);
			//创建代表赋值=号右边表达式的语句
			std::tie(child, input) = createExpressionASTNode(input);
			//child与parent连接
			parent->childs.push_back(child);
		}
		//定义指针类型变量
		else if (tk.type == TokenType::Mul)
		{
			//读取定义的变量名
			std::tie(tk, input) = expectToken(input, "error(Def pointer): need a symbol to reprensent pointer name!", TokenType::UserSymbol);

			//读取赋值符号
			std::tie(std::ignore, input) = expectToken(input, "error(Def var): miss = for defining varible!", TokenType::Assign);

			//创建赋值父节点
			parent = std::make_shared<ASTNode>();
			//结点类型为赋值
			parent->tk.type = TokenType::DefPointer;
			//创建代表被赋值变量的子节点
			auto child = std::make_shared<ASTNode>();
			child->tk = tk;
			//child与parent连接
			parent->childs.push_back(child);
			//创建代表赋值=号右边算表达式语句
			std::tie(child, input) = createArithmeticASTNode(input);
			//child与parent连接
			parent->childs.push_back(child);
		}
	}
	//如果是用户自定义的符号 
	else if (tk.type == TokenType::UserSymbol)
	{
		//则继续再读一个字符
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
	//如果是Mul(即指针解引用)
	else if (tk.type == TokenType::Mul)
	{
		//解析出解引用节点
		auto[child, str] = createDeRefASTNode("*" + input);
		//则继续再读一个字符
		Token tktemp;
		std::tie(tktemp, str) = parseToken(str);
		//如果是=、+=、-=、*=、/=
		if (isoneof(tktemp.type, TokenType::Assign, TokenType::SelfPlus, TokenType::SelfMinus, TokenType::SelfMul, TokenType::SelfDiv))
		{
			//创建赋值父节点
			parent = std::make_shared<ASTNode>();
			//结点类型为赋值
			parent->tk.type = tktemp.type;
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

//创建语句块的语法树
std::tuple<std::shared_ptr<ASTNode>, std::string> createBlockASTNode(std::string input)
{
	Token tk;
	std::string str;

	//读取左大括号
	std::tie(tk, input) = expectToken(input, "error(bad syntax): miss a {!", TokenType::LBrace);

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
		if (isnoneof(child->tk.type, TokenType::Block, TokenType::If, TokenType::While, TokenType::Do, TokenType::For))
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

//创建条件语句的语法树
std::tuple<std::shared_ptr<ASTNode>, std::string> createIfASTNode(std::string input)
{
	Token tk;

	//读取关键字if
	std::tie(tk, input) = expectToken(input, "error(bad syntax): miss keyword if!", TokenType::If);

	//构建父节点
	auto parent = std::make_shared<ASTNode>();
	parent->tk.type = TokenType::If;

	//读取左括号
	std::tie(tk, input) = expectToken(input, "error(bad syntax): if condition need a (!", TokenType::Lp);

	//读取条件
	decltype(parent) ifcondition;
	std::tie(ifcondition, input) = createExpressionASTNode(input);

	//添加为parent的第1个儿子
	parent->childs.push_back(ifcondition);

	//读取右括号
	std::tie(tk, input) = expectToken(input, "error(bad syntax): if condition need a )!", TokenType::Rp);

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
	Token tk;

	//读取关键字elseif
	std::tie(tk, input) = expectToken(input, "error(bad syntax): miss keyword elseif!", TokenType::ElseIf);

	//构建父节点
	auto parent = std::make_shared<ASTNode>();
	parent->tk.type = TokenType::If;

	//读取左括号
	std::tie(tk, input) = expectToken(input, "error(bad syntax): elseif condition need a (!", TokenType::Lp);

	//读取条件
	decltype(parent) ifcondition;
	std::tie(ifcondition, input) = createExpressionASTNode(input);

	//添加为parent的第1个儿子
	parent->childs.push_back(ifcondition);

	//读取右括号
	std::tie(tk, input) = expectToken(input, "error(bad syntax): elseif condition need a )!", TokenType::Rp);

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
	Token tk;

	//读取关键字while
	std::tie(tk, input) = expectToken(input, "error(bad syntax): miss keyword while!", TokenType::While);

	//构建父节点
	auto parent = std::make_shared<ASTNode>();
	parent->tk.type = TokenType::While;

	//读取左括号
	std::tie(tk, input) = expectToken(input, "error(bad syntax): while loop condition need a (!", TokenType::Lp);

	//循环条件
	decltype(parent) loopcondition;
	std::tie(loopcondition, input) = createExpressionASTNode(input);

	//添加为parent的第1个儿子
	parent->childs.push_back(loopcondition);

	//读取右括号
	std::tie(tk, input) = expectToken(input, "error(bad syntax): while loop condition need a )", TokenType::Rp);

	//读取while的循环体
	decltype(parent) loopblock;
	std::tie(loopblock, input) = createBlockASTNode(input);

	//添加为parent的第2个儿子
	parent->childs.push_back(loopblock);

	return { parent, input };
}

//创建DoWhile语句的语法树
std::tuple<std::shared_ptr<ASTNode>, std::string> createDoWhileASTNode(std::string input)
{
	Token tk;

	//读取关键字do
	std::tie(tk, input) = expectToken(input, "error(bad syntax): miss keyword do!", TokenType::Do);

	//构建父节点
	auto parent = std::make_shared<ASTNode>();
	parent->tk.type = TokenType::Do;

	//读取do while的循环体
	decltype(parent) loopblock;
	std::tie(loopblock, input) = createBlockASTNode(input);

	//添加为parent的第1个儿子
	parent->childs.push_back(loopblock);

	//读取关键字while
	std::tie(tk, input) = expectToken(input, "error(bad syntax): miss keyword while!", TokenType::While);

	//读取左括号
	std::tie(tk, input) = expectToken(input, "error(bad syntax): do while loop condition need a (!", TokenType::Lp);

	//循环条件
	decltype(parent) loopcondition;
	std::tie(loopcondition, input) = createExpressionASTNode(input);

	//添加为parent的第2个儿子
	parent->childs.push_back(loopcondition);

	//读取右括号
	std::tie(tk, input) = expectToken(input, "error(bad syntax): do while loop condition need a )", TokenType::Rp);

	//读取分号
	std::tie(tk, input) = expectToken(input, "error(bad syntax): miss a ;", TokenType::End);

	return { parent, input };
}

//创建for语句的语法树
std::tuple<std::shared_ptr<ASTNode>, std::string> createForASTNode(std::string input)
{
	Token tk;

	//读取关键字for
	std::tie(tk, input) = expectToken(input, "error(bad syntax): miss keyword for!", TokenType::For);

	//构建父节点
	auto parent = std::make_shared<ASTNode>();
	parent->tk.type = TokenType::For;

	//读取左括号
	std::tie(tk, input) = expectToken(input, "error(bad syntax): for loop condition need a (!", TokenType::Lp);

	//循环起始
	decltype(parent) start;
	std::tie(start, input) = createExpressionASTNode(input);

	//添加为parent的第1个儿子
	parent->childs.push_back(start);

	//读取分号
	std::tie(tk, input) = expectToken(input, "error(bad syntax): for loop need a ;!", TokenType::End);

	//循环终止条件
	decltype(parent) end;
	std::tie(end, input) = createExpressionASTNode(input);

	//添加为parent的第2个儿子
	parent->childs.push_back(end);

	//读取分号
	std::tie(tk, input) = expectToken(input, "error(bad syntax): for loop need a ;!", TokenType::End);

	//循环步进
	decltype(parent) increment;
	std::tie(increment, input) = createExpressionASTNode(input);

	//添加为parent的第3个儿子
	parent->childs.push_back(increment);

	//读取右括号
	std::tie(tk, input) = expectToken(input, "error(bad syntax): for loop condition need a )!", TokenType::Rp);

	//读取for的循环体
	decltype(parent) loopblock;
	std::tie(loopblock, input) = createBlockASTNode(input);

	//添加为parent的第4个儿子
	parent->childs.push_back(loopblock);

	return { parent, input };
}

//创建break或者continue语句的语法树
std::tuple<std::shared_ptr<ASTNode>, std::string> createBreakorContinueASTNode(std::string input)
{
	Token tk;

	//读取关键字break或者continue
	std::tie(tk, input) = expectToken(input, "error(bad syntax): miss keyword break or continue!", TokenType::Break, TokenType::Continue);

	//构建父节点
	auto parent = std::make_shared<ASTNode>();
	parent->tk = tk;

	return { parent, input };
}

//创建return语句的语法树
std::tuple<std::shared_ptr<ASTNode>, std::string> createReturnASTNode(std::string input)
{
	//读取关键字return
	std::tie(std::ignore, input) = expectToken(input, "error(bad syntax): miss keyword return!", TokenType::Return);

	//构建父节点
	auto parent = std::make_shared<ASTNode>();
	parent->tk.type = TokenType::Return;

	//解析一个表达式
	decltype(parent) child;
	std::tie(child, input) = createExpressionASTNode(input);

	//添加为子节点
	parent->childs.push_back(child);

	return { parent, input };
}

//创建定义过程的语法树节点
std::tuple<std::shared_ptr<ASTNode>, std::string> createDefProcASTNode(std::string input)
{
	Token tkfun, tkarg;

	//读取关键字proc
	std::tie(std::ignore, input) = expectToken(input, "error(Def proc): miss keyword proc!", TokenType::DefProc);
	//读取变量名
	std::tie(tkfun, input) = expectToken(input, "error(Def proc): miss function name!", TokenType::UserSymbol);
	//读取左括号
	std::tie(std::ignore, input) = expectToken(input, "error(Def proc): miss a (", TokenType::Lp);

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
			//读取函数输入参数名
			std::tie(tkarg, input) = expectToken(input, "need a symbo for function para name!", TokenType::UserSymbol);

			//注册信息
			paras.push_back(tkarg);
			//再读取一个token
			std::tie(tk, input) = parseToken(input);
		} while (tk.type == TokenType::Comma);

		//判断停止while后的符号是不是右括号 如果不是则报错
		if (tk.type != TokenType::Rp)
		{
			throw std::runtime_error("error(define proc): miss a )!\n");
		}
	}

	//读取赋值号
	std::tie(std::ignore, input) = expectToken(input, "error(define proc): need a =!", TokenType::Assign);

	//构建节点
	auto parent = std::make_shared<ASTNode>();
	parent->tk.type = TokenType::DefProc;
	//获取并添加函数体
	decltype(parent) body;
	std::tie(body, input) = createBlockASTNode(input);
	parent->childs.push_back(body);
	//添加函数名字节点
	auto procname = std::make_shared<ASTNode>();
	procname->tk = tkfun;
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

//构建一个映射表
static std::unordered_map<TokenType, std::tuple<std::shared_ptr<ASTNode>, std::string>(*)(std::string)> ASTNodeMap = 
{
	{ TokenType::DefProc, createDefProcASTNode },
	{ TokenType::If, createIfASTNode },
	{ TokenType::While, createWhileASTNode },
	{ TokenType::Do, createDoWhileASTNode },
	{ TokenType::For, createForASTNode },
	{ TokenType::Return, createReturnASTNode },
	{ TokenType::LBrace, createBlockASTNode },
	{ TokenType::Break, createBreakorContinueASTNode },
	{ TokenType::Continue, createBreakorContinueASTNode },
	{ TokenType::End, createNOpASTNode }
};

//创建一般语句的语法树
std::tuple<std::shared_ptr<ASTNode>, std::string> createStatementASTNode(std::string input)
{
	std::shared_ptr<ASTNode> parent;

	//解析第一个tk
	auto[tk, res] = parseToken(input);

	//如果在映射表中
	if (ASTNodeMap.find(tk.type) != ASTNodeMap.end())
	{
		std::tie(parent, input) = ASTNodeMap[tk.type](input);
	}
	else
	{
		//其他情况下用表达式语句解析
		std::tie(parent, input) = createExpressionASTNode(input);
	}

	return { parent, input };
}