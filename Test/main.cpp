
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

class TestThread
{
public:
	void Start()
	{
		mycpp::Thread t;
		t.Start([this](mycpp::Thread& t, void* p) { test(t, p); }, nullptr);
		Sleep(1000);
	}

private:
	void test(mycpp::Thread &thread, void* p)
	{
		while (!thread.TestExit())
		{
			std::cout << "hello" << std::endl;
		}
	}
};

#include <map>
#include <unordered_map>
#include <chrono>
#include "../Common/Thread/ThreadPool.hpp"


int main(int argc, char* argv[])
{
	mycpp::ThreadPool pool;
	pool.Start([] { while (1); });
	pool.Start([] { while (1); });

	std::cout << pool.GetCapacity() << std::endl;
	std::cout << pool.GetWaits() << std::endl;

	getchar();

	auto b = std::chrono::high_resolution_clock::now();
	std::map<std::string, std::string> m;
	for (int i = 0; i < 1000000; ++i)
		m.insert(std::make_pair(std::to_string(i)+"hello", "world"));
	auto e = std::chrono::high_resolution_clock::now();
	std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(e - b).count() << std::endl;


	getchar();

	return 0;
}
