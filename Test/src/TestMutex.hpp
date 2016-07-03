#ifndef __TEST_MUTEX_HPP__
#define __TEST_MUTEX_HPP__

// ��vs2105����  
// C++��׼��mutex���ٽ��������ܱȽϣ�thread��Windows Api�ıȽ�
// ���Խ����������׼������ܲ�������windows api, ��Ȼ��������
// Release������£�����Debug�����������鷴ת���ٽ�����ɱmutex
// �����������vs��Release����ĳЩ�Ż����¡�

// ����vs2013����
// ���ȴ���෴�� �ٽ�����������ȱ�׼���mutex�кܴ������,������debug����release

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
