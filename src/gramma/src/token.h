#pragma once

#include <string>
#include <tuple>
#include <variant>
#include <optional>
#include <functional>
#include <expected>

//单子解析宏
#define TRY_PARSE(para1, para2, expr) if (auto result = expr) std::tie(para1, para2) = result.value(); else return std::unexpected(result.error());
#define TRY_PARSE_AUTO(para1, para2, expr) auto _______result = expr; if (!_______result.has_value()) return std::unexpected(_______result.error()); auto[para1, para2] = _______result.value();
#define TRY_PARSE_IGNORE(para, expr) if (auto result = expr) std::tie(std::ignore, para) = result.value(); else return std::unexpected(result.error());

//token类型
enum class TokenType
{
	//算术运算符
	Plus,
	Minus,
	Mul,
	Div,
	Pow,
	Mod,
	//自运算符
	SelfPlus,
	SelfMinus,
	SelfMul,
	SelfDiv,
	//自增自减
	PlusPlus,
	MinusMinus,
	Increment,
	PostIncrement,
	Decrement,
	PostDecrement,
	//关系运算符
	Less,
	Great,
	NotLess,
	NotGreat,
	NotEqual,
	Equal,
	//逻辑运算符
	Not,
	And,
	Or,
	//&&
	AndAnd,
	//||
	OrOr,
	//变量引用(获得指针)
	Ref,
	//指针解引用(获得变量)
	DeRef,
	//括号
	Lp,
	Rp,
	//中括号
	LBracket,
	RBracket,
	//大括号
	LBrace,
	RBrace,
	//逗号
	Comma,
	//分号
	End,
	//数字
	Number,
	//系统自带符号
	PrimitiveSymbol,
	//symbol的起始为下划线或者字母
	UserSymbol,
	//定义变量
	DefVar,
	//定义函数
	DefProc,
	//定义指针
	DefPointer,
	//定义数组
	DefArray,
	//赋值
	Assign,
	//条件分支
	If,
	ElseIf,
	Else,
	//循环
	For,
	While,
	Do,
	Break,
	Continue,
	//返回
	Return,
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
auto parseToken(std::string input) -> std::expected<std::tuple<Token, std::string>, std::string>;

//获得原生定义的符号
auto getPrimitiveSymbol(std::string symbol) -> std::optional<std::variant<double, std::function<std::expected<double, std::string>(double)>>>;