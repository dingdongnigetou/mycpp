#include <iostream>
#include <string>
#include <chrono>
#include "worksteal\demo.hpp"
#include "TestMutex.hpp"
#include "lockfree\lock_free_stack.hpp"
#include "ringbuffer\demo.hpp"

int main()
{
	//test_pcq();

	std::string input = "s";
	std::cin >> input;
	if (input == "s")
		test_steal_pool(true);
	else 
		test_mycpp_pool();

	system("pause");

	return 0;
}
