
#include <time.h>
#include <iostream>
using namespace std;

#include "../DBAccess/common/DBAccessApi.h"

#ifdef _DEBUG
#pragma comment(lib,"../DBAccess/bin/Debug/DBAccessD.lib")
#else
#pragma comment(lib,"../DBAccess/bin/Release/DBAccess.lib")
#endif

#include "../Common/Time/DateTime.hpp"
#include "../Common/Utils.hpp"
#include "../Common/String/StrUtil.hpp"

#include <thread>
#include <iostream>

#include "Test.h"
#include "../Common/Sigleton_c11.hpp"


void dosome(int a, int b)
{
	std::cout << a + b << std::endl;
}

template<typename Fn, typename... Args>
void test(Fn f, Args... args)
{
	f(args...);
}

class Test
{

public:
	Test()
	{
	}

	Test(int a, std::string c)
	{
		std::cout << a << " " << c << std::endl;
	}

	void dosome() { std::cout << "hello" << std::endl; }

};

int main(int argc, char* argv[])
{
	TestMeta();

	mycpp::Sigleton<Test>::Instance().dosome();
	mycpp::Sigleton<Test>::Instance(1, "hello").dosome();

	getchar();

	return 0;
}
