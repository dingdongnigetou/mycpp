#ifndef __MYCPP_THREAD_IPP__
#define __MYCPP_THREAD_IPP__

namespace mycpp
{
#define  THREAD_FLAGS_JOINABLE 	0x0001
#define  THREAD_FLAGS_EXIT_MARK	0x0002
#define  THREAD_FLAGS_PAUSE	   0x0004
#define  THREAD_FLAGS_JOINING	0x0008
#define  THREAD_FLAGS_STARTING 	0x0010

	Thread::Thread(){ }
	Thread::~Thread() { Stop(); Join(); }

	template<typename Fn, typename... Args>
	bool Thread::Start(Fn f, Args ...args)
	{

	}

	void Thread::Stop()
	{
	}

	bool Thread::TestExit()
	{
	}

	void Thread::Join()
	{
	}

	void Thread::Detach()
	{
	}

	void Thread::Suspend()
	{
	}

	bool Thread::Resume()
	{
	}

	bool Thread::IsPause()
	{
	}

	bool Thread::IsRunning()
	{
	}

	long Thread::GetThreadID()
	{
	}

	long  Thread::GetCurrentThreadID()
	{
	}

	void Thread::joinInner()
	{
	}

	void Thread::detach()
	{
	}
}

#endif // !__MYCPP_THREAD_IPP__
