#pragma once

#include <string>
#include <tuple>
#include <variant>
#include <optional>
#include <functional>

//token类型
enum TokenType
{
	//赋值
	Assign = '=',
	Plus = '+',
	Minus = '-',
	Mul = '*',
	Div = '/',
	Pow = '^',
	Fact = '!',
	Lp = '(',
	Rp = ')',
	LBrace = '{',
	RBrace = '}',
	Comma = ',',
	End = ';',
	Less = '<',
	Great = '>',
	NotLess,
	NotGreat,
	Equal,
	Number,
	PrimitiveSymbol,
	//symbol的起始为下划线或者字母
	UserSymbol,
	//定义变量
	DefVar,
	//定义函数
	DefProc,
	//条件分支
	If,
	ElseIf,
	Else,
	//循环
	For,
	While,
	//语句块
	Block,
	//错误类型
	BadType,
	//空
	Empty
};

//含有特定意义的词组
struct Token
{
	//类型
	TokenType type;
	//存储非符号类型Token的值的变量
	//std::variant类型 可能是double可能是std::string
	std::variant<double, std::string> value;
};

//词法解析
std::tuple<Token, std::string> parseToken(std::string input);

//获得原生定义的符号
std::optional<std::variant<double, std::function<double(double)>>> getPrimitiveSymbol(std::string symbol);