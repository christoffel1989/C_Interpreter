#include "ASTNode.h"
#include "CreateASTNode.h"
#include "TranslateASTNode.h"

#include "AuxFacility.h"

#include <iostream>
#include <stdexcept>

int main(int argc, char* argv[])
{
	//构造全局环境
	Environment GlobalEnv;

	std::cout << "Hello! Welcome to use this small c-like code intepretor!" << std::endl;
	while (true)
	{
		std::string input;
		std::cout << ">> ";
		//获取一整行
		getline(std::cin, input);

		if (!input.empty())
		{
			try
			{
				//输入->语法树
				auto[ast, res] = createStatementASTNode(input);
				//语法数->输出
				auto result = executeAST(ast, &GlobalEnv);
				//根据语法树根节点的属性输出相应的信息
				//函数定义
				if (ast->tk.type == TokenType::DefProc)
				{
					//如果结尾不是;
					if (res == "")
					{
						//第二个子节点是函数的名字
						auto iter = ast->childs.begin();
						iter++;
						std::cout << "proc: " + std::get<std::string>((*iter)->tk.value) << " define complete!" << std::endl;
					}
					else
					{
						std::cout << "error(bad syntax)!" << std::endl;
					}
				}
				else if (ast->tk.type == TokenType::DefVar)
				{
					//如果结尾不是;
					if (res == "")
					{
						//查看变量名
						auto iter = ast->childs.begin();
						auto symbol = std::get<std::string>((*iter)->tk.value);
						std::cout << symbol << " = " << result << std::endl;
					}
				}
				//如果都不是 证明只是单纯的表达式计算 则输出计算结果
				else if (isnoneof(ast->tk.type, TokenType::If, TokenType::While, TokenType::For, TokenType::Block, TokenType::End))
				{
					//如果结尾不是;
					if (res == "")
					{
						//解析第一个字符
						Token tk;
						std::tie(tk, res) = parseToken(input);
						if (tk.type == TokenType::UserSymbol)
						{
							auto symbol = std::get<std::string>(tk.value);
							if (auto v = getEnvSymbol(symbol, &GlobalEnv))
							{
								//如果是变量
								if (std::holds_alternative<double>(v.value()))
								{
									std::cout << symbol << " = " << result << std::endl;
								}
								//函数调用
								else
								{
									std::cout << "ans = " << result << std::endl;
								}
							}
						}
						else
						{
							std::cout << "ans = " << result << std::endl;
						}
					}
					//证明存在残渣 即语法有错
					else if (res != ";")
					{
						std::cout << "error(bad syntax)!" << std::endl;
					}
				}
			}
			catch (std::exception& e)
			{
				std::cout << e.what();
			}
		}
	}

	return 0;
}