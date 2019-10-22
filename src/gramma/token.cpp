#include "token.h"

#include <cmath>
#include <map>
#include <stdexcept>

//原始映射表
static std::map<std::string, std::variant<double, std::function<double(double)>>> PrimitiveTable;

//初始化原始符号表
bool initPrimitiveTable();
void registPrimitiveSymbol(std::string symbol, std::variant<double, std::function<double(double)>> value);
//从字符串中解析出第一个数值
std::tuple<double, std::string> parseNum(std::string input);
//从字符串中解析出第一个英文单词
std::tuple<std::string, std::string> parseSymbol(std::string input);

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

std::tuple<double, std::string> parseNum(std::string input)
{

	// input "12.34a" numstr ""
	// ch 1 "2.34a" numstr "1"
	// ch 2 ".34a" numstr "12"
	// ch . "34a" numstr "12."
	char ch;
	std::string numstr;
	bool firstDot = true;

	while (1)
	{
		//是不是空
		if (input.empty())
		{
			break;
		}
		ch = *input.begin();
		if ((ch >= '0' && ch <= '9') || (ch == '.' && firstDot))
		{
			numstr.push_back(ch);
			input.erase(input.begin());
			if (ch == '.')
			{
				firstDot = false;
			}
		}
		//遇到了非数字或者是第二次遇到小数点的情况
		else
		{
			break;
		}
	}

	return { std::stod(numstr), input };
}

std::tuple<std::string, std::string> parseSymbol(std::string input)
{
	//根据调用关系 能确保这个程序的输入一定是字母或者下划线
	char ch;
	std::string name;

	while (1)
	{
		//是不是空
		if (input.empty())
		{
			break;
		}
		ch = *input.begin();
		//'Z'和'a'之间还有别的字符
		if (ch >= 'A' && ch <= 'Z' || ch >= 'a' && ch <= 'z' || ch >= '0' && ch <= '9' || ch == '_')
		{
			name.push_back(ch);
			input.erase(input.begin());
		}
		//遇到了非数字或者是第二次遇到小数点的情况
		else
		{
			break;
		}
	}

	return { name, input };
}

std::tuple<Token, std::string> parseToken(std::string input)
{
	char ch;
	Token tk;

	//把空格去掉
	do
	{
		//先判断一下是否是空
		if (input.empty())
		{
			tk.type = TokenType::Empty;
			return { tk, "" };
		}
		ch = *input.begin();
		input.erase(input.begin());
	} while (ch == ' ');

	switch (ch)
	{
	case '+':
	case '-':
	case '*':
	case '/':
	case '^':
	case '!':
	case '(':
	case ')':
	case '{':
	case '}':
	case ',':
	case ';':
		tk.type = TokenType(ch);
		break;
	case '<':
		tk.type = (*input.begin() != '=') ? TokenType(ch) : TokenType::NotGreat;
		break;
	case '>':
		tk.type = (*input.begin() != '=') ? TokenType(ch) : TokenType::NotLess;
		break;
	case '=':
		tk.type = (*input.begin() != '=') ? TokenType(ch) : TokenType::Equal;
		break;
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
		//刚才吃掉的数字补回去
		input.insert(input.begin(), ch);
		std::tie(tk.value, input) = parseNum(input);
		tk.type = TokenType::Number;
		break;
	default:
		//字母或者下划线的情况
		if (isalpha(ch) || ch == '_')
		{
			//刚才吃掉的字母补回去
			input.insert(input.begin(), ch);
			//解析出名字
			std::tie(tk.value, input) = parseSymbol(input);
			auto symbol = std::get<std::string>(tk.value);
			//如果是系统内部定义的关键字
			if (symbol == "var")
			{
				tk.type = TokenType::DefVar;
			}
			else if (symbol == "proc")
			{
				tk.type = TokenType::DefProc;
			}
			else if (symbol == "if")
			{
				tk.type = TokenType::If;
			}
			else if (symbol == "elseif")
			{
				tk.type = TokenType::ElseIf;
			}
			else if (symbol == "else")
			{
				tk.type = TokenType::Else;
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
			tk.type = TokenType::BadType;
		}
		break;
	}

	//这三种情况多吃了一个等号需要删掉
	if (tk.type == TokenType::NotGreat || tk.type == TokenType::NotLess || tk.type == TokenType::Equal)
	{
		input.erase(input.begin());
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