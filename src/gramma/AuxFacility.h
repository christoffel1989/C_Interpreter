#pragma once

//一些通用的小模板函数
//一个都不等
template <typename T>
bool isnoneof(T t, T arg)
{
	return t != arg;
}
template <typename T, typename... Ts>
bool isnoneof(T t, T arg, Ts... args)
{
	return (t != arg) && isnoneof(t, args...);
}

//等于其中一个
template <typename T>
bool isoneof(T t, T arg)
{
	return t == arg;
}
template <typename T, typename... Ts>
bool isoneof(T t, T arg, Ts... args)
{
	return (t == arg) || isoneof(t, args...);
}