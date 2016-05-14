
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
#include <functional>

#include "Test.h"
//#include "../Common/Sigleton_c11.hpp"
#include "../Common/Thread/Thread.hpp"


void test(mycpp::Thread &thread, void* p)
{
	while (!thread.TestExit())
	{
		std::cout << "hello" << std::endl;
	}
}

int main(int argc, char* argv[])
{
	mycpp::Thread t;
	t.Start(test, nullptr);
	t.Suspend();
	getchar();
	auto d = t.Resume();
	t.Detach();
	getchar();

	return 0;
}
