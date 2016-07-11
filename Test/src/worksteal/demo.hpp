#include "thread_pool.hpp"
#include "ThreadPool.hpp"
#include <atomic>
#include <thread>
#include <chrono>
#include <iostream>

void test_steal_pool(bool enable_steal)
{
	std::atomic_int total = 0;
	auto begin = std::chrono::high_resolution_clock::now();
	{
		thread_pool pool(std::thread::hardware_concurrency(), enable_steal);
		begin = std::chrono::high_resolution_clock::now();
		for (size_t i = 0; i < 1000; i++)
		{
			pool.add_task([&] {
				total++;
				std::this_thread::sleep_for(std::chrono::microseconds(50 * i));
			});
		}
	}

	auto end = std::chrono::high_resolution_clock::now();
	std::cout
		<< "thread pool " << enable_steal
		<< " result: " << total
		<< " cost : " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << "micros" << std::endl;
}

void test_mycpp_pool()
{
	std::atomic_int total = 0;
	auto begin = std::chrono::high_resolution_clock::now();
	{
		ThreadPool pool(std::thread::hardware_concurrency());
		begin = std::chrono::high_resolution_clock::now();
		for (size_t i = 0; i < 1000; i++)
		{
			pool.enqueue([&] {
				total++;
				std::this_thread::sleep_for(std::chrono::microseconds(50 * i));
			});
		}
	}
	auto end = std::chrono::high_resolution_clock::now();
	std::cout 
		<< "mycpp thread pool "
		<< "result: " << total 
		<< " cost : " <<
		std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << "micros" << std::endl;
}