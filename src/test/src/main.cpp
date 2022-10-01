#include <gtest/gtest.h>

#include "ASTNode.h"
#include "CreateASTNode.h"
#include "InterpreteASTNode.h"


TEST(test_definitionn, primitive_var_and_function)
{
    Environment GlobalEnv;
    interpreteAST(std::get<0>(createStatementASTNode("proc f1(a) = { return sin(a) ^ 2 + cos(a) ^ 2; }")), &GlobalEnv);
    interpreteAST(std::get<0>(createStatementASTNode("proc f2(a, b) = { return sqrt(a ^ 2 + b ^ 2); }")), &GlobalEnv);
    interpreteAST(std::get<0>(createStatementASTNode("proc g(f, a, b) = { return f(a, b) + f1(a); }")), &GlobalEnv);
    interpreteAST(std::get<0>(createStatementASTNode("var a = 3")), &GlobalEnv);
    EXPECT_EQ(interpreteAST(std::get<0>(createStatementASTNode("f2(a, 4)")), &GlobalEnv), 5);
    EXPECT_EQ(interpreteAST(std::get<0>(createStatementASTNode("g(f2, 3, 4)")), &GlobalEnv), 6);
}

TEST(test_definitionn, array_and_loop)
{
    Environment GlobalEnv;
    interpreteAST(std::get<0>(createStatementASTNode("proc sum(a, N) = { var acc = 0; for (var i = 0; i < N; i++) { acc += arr[i]; } return acc; }")), &GlobalEnv);
    interpreteAST(std::get<0>(createStatementASTNode("var arr[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };")), &GlobalEnv);
    EXPECT_EQ(interpreteAST(std::get<0>(createStatementASTNode("sum(arr, 10)")), &GlobalEnv), 55);
}

TEST(test_recursive_function, frac)
{
    Environment GlobalEnv;
    interpreteAST(std::get<0>(createStatementASTNode("proc frac(n) = { if (n > 1) { return n * frac(n - 1); } else { return 1; } }")), &GlobalEnv);
    EXPECT_EQ(interpreteAST(std::get<0>(createStatementASTNode("frac(6)")), &GlobalEnv), 720);
}

TEST(test_recursive_function, fibbo)
{
    Environment GlobalEnv;
    interpreteAST(std::get<0>(createStatementASTNode("proc fib(n) = { if (n > 2) { return fib(n - 1) + fib(n - 2); } elseif (n == 2) { return 1; } else { return 1; } }")), &GlobalEnv);
    EXPECT_EQ(interpreteAST(std::get<0>(createStatementASTNode("fib(7)")), &GlobalEnv), 13);
}

TEST(test_pointer, reference_and_dereference)
{
    Environment GlobalEnv;
    interpreteAST(std::get<0>(createStatementASTNode("proc swap(a, b) = { var temp = *a; *a = *b; *b = temp; }")), &GlobalEnv);
    interpreteAST(std::get<0>(createStatementASTNode("var a = 3;")), &GlobalEnv);
    interpreteAST(std::get<0>(createStatementASTNode("var b = 4;")), &GlobalEnv);
    interpreteAST(std::get<0>(createStatementASTNode("swap(&a, &b);")), &GlobalEnv);
    EXPECT_EQ(interpreteAST(std::get<0>(createStatementASTNode("a;")), &GlobalEnv), 4);
    EXPECT_EQ(interpreteAST(std::get<0>(createStatementASTNode("b;")), &GlobalEnv), 3);
}

TEST(test_pointer, self_increment_and_decrement)
{
    Environment GlobalEnv;
    interpreteAST(std::get<0>(createStatementASTNode("var a = 3;")), &GlobalEnv);
    interpreteAST(std::get<0>(createStatementASTNode("var b = 4;")), &GlobalEnv);
    interpreteAST(std::get<0>(createStatementASTNode("var* pa = &a;")), &GlobalEnv);
    interpreteAST(std::get<0>(createStatementASTNode("var* pb = &b;")), &GlobalEnv);
    interpreteAST(std::get<0>(createStatementASTNode("*pa += 3;")), &GlobalEnv);
    EXPECT_EQ(interpreteAST(std::get<0>(createStatementASTNode("a;")), &GlobalEnv), 6);
    interpreteAST(std::get<0>(createStatementASTNode("*pb = a++;")), &GlobalEnv);
    EXPECT_EQ(interpreteAST(std::get<0>(createStatementASTNode("b;")), &GlobalEnv), 6);
    EXPECT_EQ(interpreteAST(std::get<0>(createStatementASTNode("a;")), &GlobalEnv), 7);
}

TEST(test_others, scope_hidden)
{
    Environment GlobalEnv;
    interpreteAST(std::get<0>(createStatementASTNode("proc fun() = { var a = 3; var b = 0; {var a = 2; b = a; } return b; }")), &GlobalEnv);
    EXPECT_EQ(interpreteAST(std::get<0>(createStatementASTNode("fun();")), &GlobalEnv), 2);
}