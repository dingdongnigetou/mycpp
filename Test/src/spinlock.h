#pragma once
#include <atomic>

struct SpinLock
{
	volatile std::atomic_flag lck;

	SpinLock()
	{
		lck.clear();
	}

	void lock()
	{
		while (std::atomic_flag_test_and_set_explicit(&lck, std::memory_order_acquire));
	}

	bool try_lock()
	{
		bool ret = !std::atomic_flag_test_and_set_explicit(&lck, std::memory_order_acquire);
		return ret;
	}

	void unlock()
	{
		std::atomic_flag_clear_explicit(&lck, std::memory_order_release);
	}
};
