#include "Token.h"

#include "AuxFacility.h"

#include <cmath>
#include <unordered_map>
#include <stdexcept>

//原始映射表
static std::unordered_map<std::string, std::variant<double, std::function<double(double)>>> PrimitiveTable;

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

//token映射表
static std::unordered_map<char, TokenType> TokenTypeTable
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
	{'&', TokenType::Address}
};

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
	double result = 0;
	//先提取整数部分
	while (!input.empty())
	{
		//获取第一个字符
		auto iter = input.begin();
		auto ch = *iter;
		//如果是数字0至9则整数部分进一位新的到数字放在个位
		if (ch >= '0' && ch <= '9')
		{
			result = result * 10 + (ch - '0');
			//删除第一个字符
			input.erase(iter);
		}
		else
		{
			break;
		}
	}
	//如果退出循环后第一个元素是小数点则继续提取小数部分
	if (auto iter = input.begin(); iter != input.end() && *iter == '.')
	{
		//删除.
		input.erase(iter);
		double divisor = 10;
		//提取小数部分
		while (!input.empty())
		{
			//获取第一个字符
			iter = input.begin();
			auto ch = *iter;
			//如果是数字0至9则整数部分最后一位再退一位放入
			if (ch >= '0' && ch <= '9')
			{
				result += (ch - '0') / divisor;
				divisor *= 10;
				//删除第一个字符
				input.erase(iter);
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
	if (auto iter = input.begin(); iter != input.end() && isoneof(*iter, 'E', 'e'))
	{
		//删除e或E
		input.erase(iter);
		//标识指数正负的因子(初始位1)
		int sign = 1;
		//如果接下来的字符是+或者-
		if (iter = input.begin(); iter != input.end() && isoneof(*iter, '+', '-'))
		{
			//如果是-号则将标识正负的因子变为-1
			if (*iter == '-')
			{
				sign = -1;
			}		
			//删除-
			input.erase(iter);
		}
		//获取剩下的指数部分(整数)
		int exp = 0;
		bool hasin = false;
		while (!input.empty())
		{
			//获取第一个字符
			auto iter = input.begin();
			auto ch = *iter;
			//如果是数字0至9则整数部分进一位新的到数字放在个位
			if (ch >= '0' && ch <= '9')
			{
				exp = exp * 10 + (ch - '0');
				//删除第一个字符
				input.erase(iter);
			}
			else
			{
				break;
			}
		}
		//把result加上后面的幂数
		result = result * pow(10, sign * exp);
	}

	return { result, input };
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
		//如果是字母或者数字或者下划线
		if (isalpha(ch) || ch >= '0' && ch <= '9' || ch == '_')
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
	} while (ch == ' ' || ch == '\n');

	switch (ch)
	{
	case '^':
	case '%':
	case '(':
	case ')':
	case '{':
	case '}':
	case ',':
	case ';':
		tk.type = TokenTypeTable[ch];
		break;
	case '+':
		if (*input.begin() == '=')
		{
			tk.type = TokenType::SelfPlus;
		}
		else if (*input.begin() == '+')
		{
			tk.type = TokenType::PlusPlus;
		}
		else
		{
			tk.type = TokenType::Plus;
		}
		break;
	case '-':
		if (*input.begin() == '=')
		{
			tk.type = TokenType::SelfMinus;
		}
		else if (*input.begin() == '-')
		{
			tk.type = TokenType::MinusMinus;
		}
		else
		{
			tk.type = TokenType::Minus;
		}
		break;
	case '*':
		tk.type = (*input.begin() != '=') ? TokenTypeTable[ch] : TokenType::SelfMul;
		break;
	case '/':
		tk.type = (*input.begin() != '=') ? TokenTypeTable[ch] : TokenType::SelfDiv;
		break;
	case '!':
		tk.type = (*input.begin() != '=') ? TokenTypeTable[ch] : TokenType::NotEqual;
		break;
	case '<':
		tk.type = (*input.begin() != '=') ? TokenTypeTable[ch] : TokenType::NotGreat;
		break;
	case '>':
		tk.type = (*input.begin() != '=') ? TokenTypeTable[ch] : TokenType::NotLess;
		break;
	case '=':
		tk.type = (*input.begin() != '=') ? TokenTypeTable[ch] : TokenType::Equal;
		break;
	case '&':
		tk.type = (*input.begin() != '&') ? TokenTypeTable[ch] : TokenType::And;
		break;
	case '|':
		tk.type = (*input.begin() != '|') ? TokenType::BadType : TokenType::Or;
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
			tk.type = TokenType::BadType;
		}
		break;
	}

	//这三种情况多吃了一个等号需要删掉
	if (isoneof(tk.type, TokenType::NotLess, TokenType::NotGreat, TokenType::Equal, TokenType::NotEqual, TokenType::And, 
						 TokenType::Or, TokenType::SelfPlus, TokenType::SelfMinus, TokenType::SelfMul, TokenType::SelfDiv, 
						 TokenType::MinusMinus, TokenType::PlusPlus))
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