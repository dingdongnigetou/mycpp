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

		//��ʼ�̣߳��ɹ�����true,ʧ�ܷ���false	
		// void run(mycpp::Thread& t, ...);
		template<typename Fun, typename... Args>
		bool Start(Fun&& fun, Args&&... args);	 

		//ֹͣ�̣߳��ɹ�����true,ʧ�ܷ���false	
		void Stop();	

		//�ж��߳��Ƿ��˳������̺߳�����ѭ���е��ã�ִ����ͣ�ͻ��Ѳ���	
		bool TestExit();	

		//�ȴ��߳̽���
		void Join();	

		//�߳���ͣ
		void Suspend();		

		//�����߳�,�ɹ�����true,ʧ�ܷ���false	
		bool Resume();		

		//�߳��Ƿ�����ͣ
		bool IsPause(); 

		//�����߳�����״̬��trueΪ�������У�falseΪδ����	
		bool IsRunning();

		//�����߳�ID��
		long GetThreadID(); 

		static long  GetCurrentThreadID(); //���ص�ǰ�߳�ID

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
