#include "Token.h"

#include "AuxFacility.h"

#include <cmath>
#include <unordered_map>
#include <stdexcept>

//原始映射表
static std::unordered_map<std::string, std::variant<double, std::function<double(double)>>> PrimitiveTable;
//初始化原始符号表
bool initPrimitiveTable();
void registPrimitiveSymbol(std::string symbol, std::variant<double, std::function<double(double)>> value);
//利用静态变量的性质在程序启动时立刻初始化原始符号表
static bool init = initPrimitiveTable();
bool initPrimitiveTable()
{
	//////////////////初始化默认0元函数(即变量)////////////////////////
	registPrimitiveSymbol("PI", atan(1.0) * 4);
	registPrimitiveSymbol("e", exp(1.0));

	/////////////////////初始化默认1元函数////////////////////////////
	//不存在异常的函数
	registPrimitiveSymbol("sin", [](double val) { return sin(val); });
	registPrimitiveSymbol("cos", [](double val) { return cos(val); });
	registPrimitiveSymbol("tan", [](double val) { return tan(val); });
	registPrimitiveSymbol("atan", [](double val) { return atan(val); });
	registPrimitiveSymbol("exp", [](double val) { return exp(val); });
	//可能存在异常
	registPrimitiveSymbol("asin", [](double val)
	{
		if (val < -1 || val > 1)
		{
			throw std::runtime_error("error(function call): out of asin's domain!\n");
		}
		return asin(val);
	});
	registPrimitiveSymbol("acos", [](double val)
	{
		if (val < -1 || val > 1)
		{
			throw std::runtime_error("error(function call): out of acos's domain!\n");
		}
		return acos(val);
	});
	registPrimitiveSymbol("ln", [](double val)
	{
		if (val <= 0)
		{
			throw std::runtime_error("error(function call): out of ln's domain!\n");
		}
		return log(val);
	});
	registPrimitiveSymbol("log", [](double val)
	{
		if (val <= 0)
		{
			throw std::runtime_error("error(function call): out of log's domain!\n");
		}
		return log10(val);
	});
	registPrimitiveSymbol("sqrt", [](double val)
	{
		if (val < 0)
		{
			throw std::runtime_error("error(function call): out of sqrt's domain!\n");
		}
		return sqrt(val);
	});
	registPrimitiveSymbol("inv", [](double val)
	{
		if (val == 0)
		{
			throw std::runtime_error("error(function call): can not inverse zero!\n");
		}
		return 1 / val;
	});

	return true;
}

//单字符运算符的哈希表
static std::unordered_map<char, TokenType> Tk1Table
{
	{'=', TokenType::Assign},
	{'+', TokenType::Plus},
	{'-', TokenType::Minus},
	{'*', TokenType::Mul},
	{'/', TokenType::Div},
	{'^', TokenType::Pow},
	{'%', TokenType::Mod},
	{'(', TokenType::Lp},
	{')', TokenType::Rp},
	{'{', TokenType::LBrace},
	{'}', TokenType::RBrace},
	{',', TokenType::Comma},
	{';', TokenType::End},
	{'<', TokenType::Less},
	{'>', TokenType::Great},
	{'!', TokenType::Not},
	{'&', TokenType::And},
	{'&', TokenType::Or},
};

//双字符运算符的哈希表
static std::unordered_map<std::string, TokenType> TK2Table = 
{
	{"==", TokenType::Equal},
	{"!=", TokenType::NotEqual},
	{"<=", TokenType::NotGreat},
	{">=", TokenType::NotLess},
	{"+=", TokenType::SelfPlus},
	{"-=", TokenType::SelfMinus},
	{"*=", TokenType::SelfMul},
	{"/=", TokenType::SelfDiv},
	{"++", TokenType::PlusPlus},
	{"--", TokenType::MinusMinus},
	{"&&", TokenType::AndAnd},
	{"||", TokenType::OrOr}
};

//关键字表
static std::unordered_map<std::string, TokenType> KeywordTable =
{
	{"var", TokenType::DefVar},
	{"proc", TokenType::DefProc},
	{"if", TokenType::If},
	{"elseif", TokenType::ElseIf},
	{"else", TokenType::Else},
	{"for", TokenType::For},
	{"while", TokenType::While},
	{"do", TokenType::Do},
	{"break", TokenType::Break},
	{"continue", TokenType::Continue},
	{"return", TokenType::Return}
};

//从字符串中解析出第一个数值
std::tuple<double, std::string> parseNum(std::string input);
//从字符串中解析出第一个英文单词
std::tuple<std::string, std::string> parseSymbol(std::string input);

std::tuple<double, std::string> parseNum(std::string input)
{
	double result = 0;
	auto iter = input.begin();
	//先提取整数部分
	while (iter != input.end())
	{
		auto ch = *iter;
		//如果是数字0至9则整数部分进一位新的到数字放在个位
		if (ch >= '0' && ch <= '9')
		{
			result = result * 10 + (ch - '0');
			//迭代器前进一位
			++iter;
		}
		else
		{
			break;
		}
	}
	//如果退出循环后第一个元素是小数点则继续提取小数部分
	if (iter != input.end() && *iter == '.')
	{
		//迭代器前进一位(去掉小数点)
		++iter;
		double divisor = 10;
		//提取小数部分
		while (iter != input.end())
		{
			//获取第一个字符
			auto ch = *iter;
			//如果是数字0至9则整数部分最后一位再退一位放入
			if (ch >= '0' && ch <= '9')
			{
				result += (ch - '0') / divisor;
				divisor *= 10;
				//迭代器前进一位
				++iter;
			}
			else
			{
				break;
			}
		}
		//说明小数点后第一个字符不是数字 说明输入有问题
		if (divisor == 10)
		{
			//抛出异常
			throw std::runtime_error("error(bad syntax): illegal format of input number!\n");
		}
	}
	//如果退出循环后第一个e或者E则继续提取指数的部分
	if (iter != input.end() && isoneof(*iter, 'E', 'e'))
	{
		//迭代器前进一位(去掉e或E)
		++iter;
		//标识指数正负的因子(初始位1)
		int sign = 1;
		//如果接下来的字符是+或者-
		if (iter != input.end() && isoneof(*iter, '+', '-'))
		{
			//如果是-号则将标识正负的因子变为-1
			if (*iter == '-')
			{
				sign = -1;
			}
			//迭代器前进一位(删除+或-)
			++iter;
		}
		//获取剩下的指数部分(整数)
		int exp = 0;
		bool hasin = false;
		while (iter != input.end())
		{
			//获取第一个字符
			auto ch = *iter;
			//如果是数字0至9则整数部分进一位新的到数字放在个位
			if (ch >= '0' && ch <= '9')
			{
				exp = exp * 10 + (ch - '0');
				//迭代器前进一位
				++iter;
			}
			else
			{
				break;
			}
		}
		//把result加上后面的幂数
		result = result * pow(10, sign * exp);
	}

	//切掉input的[begin,iter)的字符
	input.erase(input.begin(), iter);

	return { result, input };
}

std::tuple<std::string, std::string> parseSymbol(std::string input)
{
	//根据调用关系 能确保这个程序的输入一定是字母或者下划线
	auto iter = input.begin();
	while (iter != input.end())
	{
		auto ch = *iter;
		//如果是字母或者数字或者下划线
		if (isalpha(ch) || ch >= '0' && ch <= '9' || ch == '_')
		{
			//迭代器步进1
			++iter;
		}
		//遇到了非数字或者是第二次遇到小数点的情况
		else
		{
			break;
		}
	}

	//分割字符
	auto name = input.substr(0, iter - input.begin());
	//去掉前面的部分
	input.erase(input.begin(), iter);

	return { name, input };
}

std::tuple<Token, std::string> parseToken(std::string input)
{
	char ch;
	Token tk;
	auto iter = input.begin();
	//去掉字符串开头的空格和换行符号
	while (iter != input.end())
	{
		ch = *iter;
		if (ch == ' ' || ch == '\n')
		{
			//迭代器往前
			++iter;
		}
		else
		{
			break;
		}
	}
	//如果迭代器已经走到末尾了则说明是空字符串 直接返回
	if (iter == input.end())
	{
		tk.type = TokenType::Empty;
		return { tk, "" };
	}

	//裁剪input区间在[begin, iter)范围内的字符(不是空格就是换行)
	input.erase(input.begin(), iter);

	//如果当前字符ch是下面这些可能由两个字符构成的运算的第一个字符
	if (isoneof(ch, '=', '+', '-', '*', '/', '&', '|', '!', '<', '>'))
	{
		bool exist = false;
		//如果当前至少还有两个字符
		if (input.size() > 1)
		{
			//构造一个字符串包含input的前两个字符
			auto str = input.substr(0, 2);
			//如果str在二元表中
			if (auto it = TK2Table.find(str); exist = it != TK2Table.end())
			{
				//解析出来的token的类型为该表中str映射的类型
				tk.type = it->second;
				//裁剪掉前两个字符
				input.erase(input.begin(), input.begin() + 2);
			}
		}
		//如果上述查找失败了 则说明在当前输入字串中只是单独的一个ch 所以只需在一元表中查询即可
		if (!exist)
		{
			//根据一元二元表的构造结构 一定能查到所以不需要用find
			tk.type = Tk1Table[ch];
			//裁剪第一个字符
			input.erase(input.begin());
		}
	}
	//因为到了这里时ch不一定在一元表中 所以需要用find查找
	else if (auto it = Tk1Table.find(ch); it != Tk1Table.end())
	{
		//解析出来的token的类型为该表中ch映射的类型
		tk.type = it->second;
		//裁剪第一个字符
		input.erase(input.begin());
	}
	//如果是数字
	else if (ch >= '0' && ch <= '9')
	{
		//执行解析字符串开头数字的函数
		std::tie(tk.value, input) = parseNum(input);
		tk.type = TokenType::Number;
	}
	//字母或者下划线的情况
	else if (isalpha(ch) || ch == '_')
	{
		//解析出名字
		std::tie(tk.value, input) = parseSymbol(input);
		auto symbol = std::get<std::string>(tk.value);
		//如果是系统内部定义的关键字
		if (auto iter = KeywordTable.find(symbol); iter != KeywordTable.end())
		{
			tk.type = iter->second;
		}
		//内部提前定义好的符号
		else if (auto v = getPrimitiveSymbol(symbol))
		{
			tk.type = TokenType::PrimitiveSymbol;
		}
		//用户自定义符号
		else
		{
			tk.type = TokenType::UserSymbol;
		}
	}
	else
	{
		//坏类型
		tk.type = TokenType::BadType;
	}
	return { tk, input };
}

void registPrimitiveSymbol(std::string symbol, std::variant<double, std::function<double(double)>> value)
{
	PrimitiveTable[symbol] = value;
}

std::optional<std::variant<double, std::function<double(double)>>> getPrimitiveSymbol(std::string symbol)
{
	//查找
	auto iter = PrimitiveTable.find(symbol);

	if (iter != PrimitiveTable.end())
	{
		return { iter->second };
	}
	return {};
}