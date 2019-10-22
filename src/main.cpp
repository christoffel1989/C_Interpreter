#include "ASTNode.h"
#include "CreateASTNode.h"
#include "TranslateASTNode.h"

#include <iostream>
#include <stdexcept>

int main(int argc, char* argv[])
{
	//构造全局环境
	ASTEnvironment GlobalEnv;

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
				std::cout << "ans = " << result << std::endl;
			}
			catch (std::exception& e)
			{
				std::cout << e.what();
			}
		}
	}

	return 0;
}