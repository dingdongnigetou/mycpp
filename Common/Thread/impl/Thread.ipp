#ifndef __MYCPP_THREAD_IPP__
#define __MYCPP_THREAD_IPP__

#include "../../Utils.hpp"

#ifdef _MSWINDOWS_
#include <process.h>    /* _beginthread, _endthread */
#endif

namespace mycpp
{
#define  THREAD_FLAGS_JOINABLE 	0x0001
#define  THREAD_FLAGS_EXIT_MARK	0x0002
#define  THREAD_FLAGS_PAUSE	   0x0004
#define  THREAD_FLAGS_JOINING	0x0008
#define  THREAD_FLAGS_STARTING 	0x0010
#define  THREAD_FLAGS_detachED  0x0020

	typedef struct _StruThreadArgs
	{
		std::function<void()> callback;
	}StruThreadArgs;


#if defined(_MSWINDOWS_)
	static unsigned   _stdcall _ThreadProxyEnter(HANDLE param)
#else
	static void *_ThreadProxyEnter(void *param)
#endif 
	{
		StruThreadArgs args;
		StruThreadArgs *p = (StruThreadArgs*)param;
		args = *p;
		delete p;
		p = nullptr;
		args.callback();
		return 0;
	}

	Thread::Thread(){ }
	Thread::~Thread() { Stop(); Join(); }

	template<typename Fun, typename... Args>
	bool Thread::Start(Fun&& fun, Args&&... args)
	{
		MyAutoMutex locker(mutex_);

		if (flags_&THREAD_FLAGS_STARTING)
		{
			//正在启动
			MYASSERT(0);
			return false;
		}
		flags_ |= THREAD_FLAGS_STARTING;

		if (flags_&THREAD_FLAGS_JOINABLE)
		{
			//上次线程执行过， 清理退出
			if (flags_&THREAD_FLAGS_EXIT_MARK)
			{
				//已经标注退出
				if (flags_&THREAD_FLAGS_JOINING)
				{
					//其他线程 正在  join
					MYASSERT(0);
					flags_ &= ~THREAD_FLAGS_STARTING;
					return false;
				}
				flags_ |= THREAD_FLAGS_JOINING;
				mutex_.Unlock();

				joinInner();
				mutex_.Lock();
				flags_ = THREAD_FLAGS_STARTING;
			}
			else
			{
				//线程没有退出
				MYASSERT(0);
				flags_ &= ~THREAD_FLAGS_STARTING;
				return false;
			}
		}

		flags_ = THREAD_FLAGS_JOINABLE;

		StruThreadArgs *pArgs = new StruThreadArgs;
		if (!pArgs)
		{
			MYASSERT(0);
			flags_ = 0;
			return false;
		}
		pArgs->callback = [this, fun, args...]() { fun(*this, args...);  };

#if  defined(_MSWINDOWS_)
		unsigned int id;
		hThread_ = (HANDLE)_beginthreadex(nullptr, 0, _ThreadProxyEnter, pArgs,
			0, (unsigned int *)(&id));
		if (hThread_ == nullptr)
		{
			MYASSERT(0);
			delete pArgs;
			flags_ = 0;
			return false;
		}
		threadId_ = id;
#else
		if (pthread_create(&hThread_, 0, _ThreadProxyEnter, (void*)pArgs))
		{
			MYASSERT(0);
			delete pArgs;
			flags_ = 0;
			hThread_ = 0;
			return false;
		}
		threadId_ = (long)hThread_;
#endif
		MYASSERT(hThread_);
		return true;
	}

	void Thread::Stop()
	{
		//判断线程是否在运行
		MyAutoMutex locker(mutex_);

		if (flags_&THREAD_FLAGS_JOINABLE ||
			flags_&THREAD_FLAGS_STARTING)
		{
			flags_ |= THREAD_FLAGS_EXIT_MARK;
			flags_ &= ~THREAD_FLAGS_PAUSE;
			cond_.Signal();
		}

	}

	bool Thread::TestExit()
	{
		MyAutoMutex locker(mutex_);
		if (!(flags_&THREAD_FLAGS_JOINABLE))
		{
			return true;
		}

		while (flags_&THREAD_FLAGS_PAUSE)
		{
			if (flags_&THREAD_FLAGS_EXIT_MARK)
			{
				break;
			}
			cond_.Wait(mutex_);
		}
		if (flags_&THREAD_FLAGS_EXIT_MARK)
		{
			return true;
		}
		return false;
	}

	void Thread::Join()
	{
		//判断线程是否在运行
		MyAutoMutex locker(mutex_);
		if (!(flags_&THREAD_FLAGS_JOINABLE))
		{
			return;
		}
		if (threadId_ == GetCurrentThreadID())
		{
			detach();
			return;
		}
		if (flags_&THREAD_FLAGS_JOINING)
		{
			//正在JOIN	
			do
			{
				mutex_.Unlock();
				Utils()->MySleep(10);
				mutex_.Lock();
			} while (flags_&THREAD_FLAGS_JOINING);
			return;
		}
		flags_ |= THREAD_FLAGS_JOINING;
		mutex_.Unlock();
		joinInner();
		mutex_.Lock();
		flags_ = 0;
	}

	void Thread::detach()
	{
		if (hThread_)
		{
#if  defined(_MSWINDOWS_) 
			CloseHandle(hThread_);
#else
			pthread_detach(hThread_);
#endif
			hThread_ = 0;
		}

		flags_ |= THREAD_FLAGS_detachED;
	}

	void Thread::Suspend()
	{
		MyAutoMutex locker(mutex_);

		if (flags_&THREAD_FLAGS_JOINABLE &&
			!(flags_&THREAD_FLAGS_EXIT_MARK))
		{
			flags_ |= THREAD_FLAGS_PAUSE;
		}
	}

	bool Thread::Resume()
	{
		MyAutoMutex locker(mutex_);
		flags_ &= ~THREAD_FLAGS_PAUSE;
		if (flags_&THREAD_FLAGS_JOINABLE)
		{
			//在运行
			cond_.Signal();
		}

		return (0 == (flags_&THREAD_FLAGS_EXIT_MARK) && flags_&THREAD_FLAGS_JOINABLE);
	}

	bool Thread::IsPause()
	{
		MyAutoMutex locker(mutex_);
		return  (flags_&(THREAD_FLAGS_JOINABLE | THREAD_FLAGS_PAUSE) &&
			0 == (flags_&THREAD_FLAGS_EXIT_MARK));
	}

	bool Thread::IsRunning()
	{
		MyAutoMutex locker(mutex_);
		return  (flags_&(THREAD_FLAGS_JOINABLE) &&
			0 == (flags_&THREAD_FLAGS_EXIT_MARK));
	}

	long Thread::GetThreadID()
	{
		return threadId_;
	}

	long  Thread::GetCurrentThreadID()
	{
#if defined(_MSWINDOWS_)
		return (long)GetCurrentThreadId();
#else
		return (long)pthread_self();
#endif
	}

	void Thread::joinInner()
	{
		int iRet;
		if (hThread_)
		{

#if  defined(_MSWINDOWS_) 
			iRet = WaitForSingleObject(hThread_, INFINITE);
			MYASSERT(iRet == WAIT_OBJECT_0);
			CloseHandle(hThread_);
#else
			iRet = pthread_join(hThread_, nullptr);
			MYASSERT(iRet == 0);
			hThread_ = (pthread_t)-1;
#endif
			hThread_ = 0;
		}
		threadId_ = -1;
	}


}

#endif // !__MYCPP_THREAD_IPP__
