#ifndef __MYCPP_THREAD_HPP__
#define __MYCPP_THREAD_HPP__

#include <thread>
#include <functional>

#ifdef _LINUX
#include <pthread.h>
#endif

#include "../mydefs.h"
#include "../mytypes.h"
#include "../noncopyable.h"
#include "../Mutex/MyMutex.hpp"
#include "../Mutex/MyCond.hpp"

namespace mycpp
{
	// void run(mycpp::Thread& t, ...);
	class Thread : noncopyable
	{
	public:
		Thread();
		virtual ~Thread();

		//开始线程，成功返回true,失败返回false	
		// void run(mycpp::Thread& t, ...);
		template<typename Fun, typename... Args>
		bool Start(Fun&& fun, Args&&... args);	 

		//停止线程，成功返回true,失败返回false	
		void Stop();	

		//判断线程是否退出，在线程函数的循环中调用，执行暂停和唤醒操作	
		bool TestExit();	

		//等待线程结束
		void Join();	

		//线程暂停
		void Suspend();		

		//唤醒线程,成功返回true,失败返回false	
		bool Resume();		

		//线程是否在暂停
		bool IsPause(); 

		//返回线程运行状态，true为正在运行，false为未运行	
		bool IsRunning();

		//返回线程ID号
		long GetThreadID(); 

		static long  GetCurrentThreadID(); //返回当前线程ID

	private:
		void detach();
		void joinInner();

	protected:
#if  defined(_MSWINDOWS_)
		HANDLE hThread_ = nullptr;
#else
		pthread_t hThread_ = nullptr;
#endif
		volatile int flags_ = 0;
		MyMutex mutex_;
		MyCond cond_;
		long threadId_ = -1;

	};
}
#include "impl/Thread.ipp"

#endif // !__MYCPP_THREAD_HPP__
