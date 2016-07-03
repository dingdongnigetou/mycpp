#pragma once

#include <iostream>
#include <string>
#include "../../Common/Meta.hpp"

void TestMeta()
{
	struct person
	{
		std::string name;
		int age;
		META(name, age) //����һ��֧�ֱ�ε�meta����
	};

	person p = { "tom", 20 };
    auto tp = p.meta();
	auto m = std::get<0>(tp);
	auto d = std::get<1>(tp);

	std::cout << m.first << ":" << m.second << std::endl;
	std::cout << d.first << ":" << d.second << std::endl;
}
