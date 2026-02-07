#pragma once

#include "Token.h"

#include <tuple>
#include <vector>
#include <list>
#include <unordered_map>
#include <memory>
#include <variant>
#include <expected>

//语法树节点
struct ASTNode
{
	//节点的词法类型
	Token tk;

	//子节点
	std::list<std::shared_ptr<ASTNode>> childs;
};

//栈空间定义
//地址类型
using VarAddress = unsigned int;

//栈空间的最大容量(目前设置为4k与实际windows的栈空间大小相同)
inline const VarAddress MaxStackLen = 4096;

//获得栈空间栈顶地址
VarAddress getStackTop();

//设置栈空间栈顶地址
void setStackTop(VarAddress top);

//环境表存储了一些定义了虚拟内存、符号与虚拟内存地址的映射哈希表、以及他的父环境指针
struct Environment
{
	//构造函数(初始化起始位置为当前栈顶)
	Environment() : start(getStackTop()) {}

	//析构函数(栈顶设置回环境的起始位置)
	~Environment() { setStackTop(start); }

	//每一个symbol对应的地址表
	std::unordered_map<std::string, VarAddress> address;

	//环境中变量在栈空间中的起始位置
	VarAddress start;

	//指向父类环境的指针
	Environment* parent = nullptr;
};

//用户自定义变量或函数或指针在解释器虚拟内存空间中的存储类型 为一个可变类型(即variant)
//当类型为double时,表明该处内存对应一个变量,存储了他的值
//当类型为std::tuple<std::list<std::string>, std::shared_ptr<ASTNode>>时，表明该处内存对应一个函数
//其第一个分量是函数的输入变量字符串构成的list 第二个分量是这个函数的语法树
//当类型为VarAddress时,表明该处内存对应一个指针,存储的是地址
using UserAST = std::variant<double, std::tuple<std::list<std::string>, std::shared_ptr<ASTNode>>, VarAddress>;

//注册新的变量或函数在当前环境
void registEnvSymbol(std::string symbol, UserAST value, Environment* env);

//注册新的数组类型变量到当前环境 变量本身存储了数组首地址的指针
void registEnvSymbol(std::string symbol, std::vector<UserAST> values, Environment* env);

//设置用户自定义符号(变量或函数)的值
void setEnvSymbol(std::string symbol, UserAST value, Environment* env);

//通过地址设置直接设置用户自定义符号(变量或函数)的值
bool setEnvSymbol(UserAST value, VarAddress addr);

//获得用户自定义符号(变量或函数)的值 当onlycurrent为true时只在当前环境搜索
std::optional<UserAST> getEnvSymbol(std::string symbol, Environment* env, bool onlycurrent = false);

//通过地直接获得用户自定义符号(变量或函数)的值
std::optional<UserAST> getEnvSymbol(VarAddress addr);

//获得制定环境中用户自定义符号的地址
std::optional<VarAddress> getEnvSymbolAddr(std::string symbol, Environment* env);




//流程状态
struct ContinueState {};
struct BreakState {};
struct ReturnState { double value; };
struct ErrorState { std::string message; };
using State = std::variant<ContinueState, BreakState, ReturnState, ErrorState>;
//单子
using ASTResult = std::expected<double, State>;

//单子解析宏
#define TRY_(expr) \
    if (!(expr)) [[unlikely]] { \
        return expr; \
    }

#define TRY(var, expr) \
    auto&& var##_result = (expr); \
    if (!var##_result) [[unlikely]] { \
        if (std::holds_alternative<ReturnState>(var##_result.error())) { return std::get<ReturnState>(var##_result.error()).value; } \
        return std::unexpected{var##_result.error()}; \
    } \
    var = std::move(var##_result).value();

#define TRY_AUTO(var, expr) \
    auto&& var##_result = (expr); \
    if (!var##_result) [[unlikely]] { \
        if (std::holds_alternative<ReturnState>(var##_result.error())) { return std::get<ReturnState>(var##_result.error()).value; } \
        return std::unexpected{var##_result.error()}; \
    } \
    auto var = std::move(var##_result).value();

//单子包装器
template<typename F>
concept UnaryBoolFunction = std::regular_invocable<F, double>&& std::same_as<std::invoke_result_t<F, bool>, bool>;
template<typename F>
concept UnaryDoubleFunction = std::regular_invocable<F, double>&& std::same_as<std::invoke_result_t<F, double>, double>;
template<typename F>
concept BinaryBoolFunction = std::regular_invocable<F, double, double>&& std::same_as<std::invoke_result_t<F, double, double>, bool>;
template<typename F>
concept BinaryDoubleFunction = std::regular_invocable<F, double, double>&& std::same_as<std::invoke_result_t<F, double, double>, double>;
template<UnaryBoolFunction UnaryOp>
auto ast_wrap(UnaryOp op) { return [op](bool a) -> ASTResult { return ASTResult(op(a)); }; }
template<UnaryDoubleFunction UnaryOp>
auto ast_wrap(UnaryOp op) { return [op](double a) -> ASTResult { return ASTResult(op(a)); }; }
template<BinaryBoolFunction BinaryOp>
auto ast_wrap(BinaryOp op) { return [op](double a, double b) -> ASTResult { return ASTResult(op(a, b)); }; }
template<BinaryDoubleFunction BinaryOp>
auto ast_wrap(BinaryOp op) { return [op](double a, double b) -> ASTResult { return ASTResult(op(a, b)); }; }