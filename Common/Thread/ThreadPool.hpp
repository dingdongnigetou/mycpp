#ifndef __MYCPP_THREAD_POOL_HPP__
#define __MYCPP_THREAD_POOL_HPP__

#include <vector>
#include <list>
#include <memory>

#include "Thread.hpp"
#include "../Time/DateTime.hpp"

namespace mycpp
{
	class ThreadPool
	{
	public:
		/*
		minCapacity 线程池空闲时最小保留的线程数
		maxCapacity 线程池最大启动线程数
		idleTime 线程空闲多久后会销毁， 单位 秒 , <1 表示不退出
		*/
		ThreadPool(int minCapacity = 2, int maxCapacity = 16, int idleTime = 60);
		~ThreadPool();

		//增加或减少线程数，是否需要？
		void AddCapacity(int n);
		//返回线程池当前线程数
		int GetCapacity() const;
		//停止线程池
		void StopAll(); 
		//等待所有线程结束
		void JoinAll();

		template<typename Fun, typename... Args>
		bool Start(Fun&& fun, Args&&... args);

		bool IsSelfThread(long threaID);

		//返回等待调度的线程数
		int GetWaits();
	private:
		void threadEntry(Thread& thread);
		void addThread();

	private:
		std::list<std::function<void()>> tasks_;

		int minCapacity_ = 0;
		int maxCapacity_ = 0;
		int nCapacity_ = 0;
		int idleTime_ = 0; //单位秒
		MyMutex mutex_;
		MyCond cond_;
		std::vector<std::shared_ptr<Thread>> threads_;
		bool isrun_ = true;
		int idles_ = 0; //空闲线程数量
	};
} // end namespace 

#include "impl/ThreadPool.ipp"

#endif // !__MYCPP_THREAD_POOL_HPP__
