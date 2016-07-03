#ifndef __TEST_MUTEX_HPP__
#define __TEST_MUTEX_HPP__

// 在vs2105库下  
// C++标准库mutex与临界区的性能比较，thread与Windows Api的比较
// 测试结果表明，标准库的性能并不亚于windows api, 当然，这是在
// Release的情况下，而在Debug的情况下则剧情反转，临界区秒杀mutex
// 这可能是由于vs在Release做了某些优化导致。

// 而在vs2013库下
// 结果却是相反， 临界区的性能相比标准库的mutex有很大的提升,无论是debug还是release

#include "../../Common/mytypes.h"

#include <iostream>
#include <mutex>
#include <thread>
#include <list>

#include "spinlock.h"

#ifdef _MSWINDOWS_
#include <Windows.h>

static void test_critical()
{
	static int total = 0;
	static CRITICAL_SECTION lock;
	InitializeCriticalSection(&lock);

	std::list<std::shared_ptr<std::thread>> threads;
	auto b = std::chrono::high_resolution_clock::now();
	for (int i = 0; i < 5; ++i) {
		threads.push_back(std::make_shared<std::thread>([] {
			for (int i = 0; i < 1000000; ++i)
			{
				EnterCriticalSection(&lock);
				total += 1;
				LeaveCriticalSection(&lock);
			}
		}
		));
	}
	
	for (auto t : threads)
		t->join();

	auto e = std::chrono::high_resolution_clock::now();
	std::cout << "cirtical cost: " << std::chrono::duration_cast<std::chrono::milliseconds>(e - b).count()
		<< "ms" << std::endl;
}

#endif

static void test_mutex()
{
	static int total = 0;
	static std::mutex mtx;

	std::list<std::shared_ptr<std::thread>> threads;
	auto b = std::chrono::high_resolution_clock::now();
	for (int i = 0; i < 5; ++i) {
		threads.push_back(std::make_shared<std::thread>([] {
			for (int i = 0; i < 1000000; ++i)
			{
				std::unique_lock<std::mutex> lck(mtx);
				total += 1;
			}
		}
		));
	}

	for (auto t : threads)
		t->join();

	auto e = std::chrono::high_resolution_clock::now();
	std::cout << "mutex cost: " << std::chrono::duration_cast<std::chrono::milliseconds>(e - b).count()
		<< "ms" << std::endl;
}

static void test_spinlock()
{
	static int total = 0;
	static SpinLock mtx;

	std::list<std::shared_ptr<std::thread>> threads;
	auto b = std::chrono::high_resolution_clock::now();
	for (int i = 0; i < 5; ++i) {
		threads.push_back(std::make_shared<std::thread>([] {
			for (int i = 0; i < 1000000; ++i)
			{
				std::unique_lock<SpinLock> lck(mtx);
				total += 1;
			}
		}
		));
	}

	for (auto t : threads)
		t->join();

	auto e = std::chrono::high_resolution_clock::now();
	std::cout << "spinlock cost: " << std::chrono::duration_cast<std::chrono::milliseconds>(e - b).count()
		<< "ms" << std::endl;
}

#endif
