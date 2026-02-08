#include "ASTNode.h"
#include "CreateASTNode.h"
#include "InterpreteASTNode.h"

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
			//输入->语法树
			if (auto result1 = createStatementASTNode(input))
			{
				auto [ast, res] = result1.value();
				//语法数->输出
				if (auto result = interpreteAST(ast, &GlobalEnv))
				{
					//根据语法树根节点的属性输出相应的信息
					//函数定义
					if (ast->tk.type == TokenType::DefProc)
					{
						//如果结尾不存在空格或分号以外的字符
						if (containOnly(res, ' ', ';'))
						{
							//如果不存在分号(即只存在空格)则输出函数定义完毕的提示
							if (containOnly(res, ' '))
							{
								//第二个子节点是函数的名字
								auto iter = ast->childs.begin();
								iter++;
								std::cout << "proc: " + std::get<std::string>((*iter)->tk.value) << " define complete!" << std::endl;
							}
						}
						//存在不符合语法的字符语法有错
						else
						{
							std::cout << "error(bad syntax)!" << std::endl;
						}
					}
					else if (ast->tk.type == TokenType::DefVar)
					{
						//如果结尾不存在空格或分号以外的字符
						if (containOnly(res, ' ', ';'))
						{
							//如果不存在分号(即只存在空格)则输出该变量的名字和值
							if (containOnly(res, ' '))
							{
								//查看变量名
								auto iter = ast->childs.begin();
								auto symbol = std::get<std::string>((*iter)->tk.value);
								std::cout << symbol << " = " << result.value() << std::endl;
							}
						}
						//存在不符合语法的字符语法有错
						else
						{
							std::cout << "error(bad syntax)!" << std::endl;
						}
					}
					//如果都不是 证明只是单纯的表达式计算 则输出计算结果
					else if (isnoneof(ast->tk.type, TokenType::If, TokenType::While, TokenType::For, TokenType::Block, TokenType::End))
					{
						//如果结尾不存在空格或分号以外的字符
						if (containOnly(res, ' ', ';'))
						{
							//如果不存在分号(即只存在空格)
							//如果是变量赋值则输出变量名 = 数值
							//否则输出 ans = 数值
							if (containOnly(res, ' '))
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
											std::cout << symbol << " = " << result.value() << std::endl;
										}
										//函数调用
										else
										{
											std::cout << "ans = " << result.value() << std::endl;
										}
									}
								}
								else
								{
									std::cout << "ans = " << result.value() << std::endl;
								}
							}
						}
						//存在不符合语法的字符语法有错
						else
						{
							std::cout << "error(bad syntax)!" << std::endl;
						}
					}
				}
				else if (std::holds_alternative<ErrorState>(result.error()))
				{
					std::cout << std::get<ErrorState>(result.error()).message;
				}
			}
			else
			{
				std::cout << result1.error();
			}
		}
	}

	return 0;
}