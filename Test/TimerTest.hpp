#include "../Common/Time/TimeMeter.hpp"
#include "../Common//Time/Timer.hpp"

#include <iostream>
#include <memory>

static mycpp::Timer g_t;

void TestTimer()
{
	mycpp::Timer::ModuleInit();
	g_t.Init(5000, [] { std::cout << "times up...\n";  });
	g_t.Start(false);
	getchar();
	g_t.Stop();
	mycpp::Timer::ModuleUninit();
}