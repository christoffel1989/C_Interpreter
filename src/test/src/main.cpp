#include <gtest/gtest.h>

#include "ASTNode.h"
#include "CreateASTNode.h"
#include "InterpreteASTNode.h"


TEST(ExampleTest, test_basic1)
{
    Environment GlobalEnv;
    EXPECT_TRUE(interpreteAST(std::get<0>(createStatementASTNode("1 + 1")), &GlobalEnv) == 2);
}
TEST(ExampleTest, test_basic2)
{
    Environment GlobalEnv;
    interpreteAST(std::get<0>(createStatementASTNode("var a = 3")), &GlobalEnv);
    interpreteAST(std::get<0>(createStatementASTNode("var b = 4")), &GlobalEnv);
    EXPECT_TRUE(interpreteAST(std::get<0>(createStatementASTNode("a + b * a")), &GlobalEnv) == 12);
}
TEST(ExampleTest, test_basic3)
{
    Environment GlobalEnv;
    EXPECT_TRUE(interpreteAST(std::get<0>(createStatementASTNode("4 + 3")), &GlobalEnv) == 3);
}