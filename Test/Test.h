#ifndef MYCPP_TEST
#define MYCPP_TEST

// ��vs2105����  
// C++��׼��mutex���ٽ��������ܱȽϣ�thread��Windows Api�ıȽ�
// ���Խ����������׼������ܲ�������windows api, ��Ȼ��������
// Release������£�����Debug�����������鷴ת���ٽ�����ɱmutex
// �����������vs��Release����ĳЩ�Ż����¡�

// ����vs2013����
// ���ȴ���෴�� �ٽ�����������ȱ�׼���mutex�кܴ������,������debug����release

// ����һ���Ƽ�ʹ���ٽ������ڲ��������������¡�
void use_std_mutex();
void use_win_critical();
void use_win_thread();

#include <iostream>
#include <string>
#include "../Common/Meta.hpp"

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

#endif // MYCPP_TEST
