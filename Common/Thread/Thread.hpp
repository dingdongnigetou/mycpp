#ifndef __MYCPP_THREAD_HPP__
#define __MYCPP_THREAD_HPP__

#include <thread>
#include <function>

#ifdef _LINUX
#include <pthread.h>
#endif

#include "../mydefs.h"
#include "../noncopyable.h"
#include "../Mutex/MyMutex.hpp"


namespace mycpp
{
	class Thread : noncopyable
	{
	public:
		Thread();
		virtual ~Thread();

		//开始线程，成功返回TRUE,失败返回FALSE	
		//支持闭包，原生函数
		template<typename Fn, typename... Args>
		bool Start(Fn f, Args args);	 

		//停止线程，成功返回TRUE,失败返回FALSE	
		void Stop();	

		//判断线程是否退出，在线程函数的循环中调用，执行暂停和唤醒操作	
		bool TestExit();	

		//等待线程结束
		void Join();	

		//分离线程,分离后无法再次控制线程
		void Detach();

		//线程暂停
		void Suspend();		

		//唤醒线程,成功返回TRUE,失败返回FALSE	
		bool Resume();		

		//线程是否在暂停
		bool IsPause(); 

		//返回线程运行状态，TRUE为正在运行，FALSE为未运行	
		bool IsRunning();

		//返回线程ID号
		long GetThreadID(); 

		static long  GetCurrentThreadID(); //返回当前线程ID

	private:
		void joinInner();
		void detach();

	protected:
#if  defined(_MSWINDOWS_)
		HANDLE hThread_ = nullptr;
#else
		pthread_t hThread_ = nullptr;
#endif
		int flags_ = 0;
		MyMutex mutex_;
		MyCond cond_;
		long threadId_ = -1;

		std::function fn_;
	};
}

#ifdef _MSWINDOWS_
#include <process.h>    /* _beginthread, _endthread */
#endif

// impl
namespace mycpp
{
//已经启动
#define  THREAD_FLAGS_JOINABLE 	0x0001
#define  THREAD_FLAGS_EXIT_MARK	0x0002
#define  THREAD_FLAGS_PAUSE	   0x0004
#define  THREAD_FLAGS_JOINING	0x0008
#define  THREAD_FLAGS_STARTING 	0x0010

	Thread::Thread()
	{
	}

	Thread::~Thread() { Stop(); Join(); }
}


#endif // !__MYCPP_THREAD_HPP__
