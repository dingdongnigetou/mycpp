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

		//��ʼ�̣߳��ɹ�����TRUE,ʧ�ܷ���FALSE	
		//֧�ֱհ���ԭ������
		template<typename Fn, typename... Args>
		bool Start(Fn f, Args args);	 

		//ֹͣ�̣߳��ɹ�����TRUE,ʧ�ܷ���FALSE	
		void Stop();	

		//�ж��߳��Ƿ��˳������̺߳�����ѭ���е��ã�ִ����ͣ�ͻ��Ѳ���	
		bool TestExit();	

		//�ȴ��߳̽���
		void Join();	

		//�����߳�,������޷��ٴο����߳�
		void Detach();

		//�߳���ͣ
		void Suspend();		

		//�����߳�,�ɹ�����TRUE,ʧ�ܷ���FALSE	
		bool Resume();		

		//�߳��Ƿ�����ͣ
		bool IsPause(); 

		//�����߳�����״̬��TRUEΪ�������У�FALSEΪδ����	
		bool IsRunning();

		//�����߳�ID��
		long GetThreadID(); 

		static long  GetCurrentThreadID(); //���ص�ǰ�߳�ID

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
//�Ѿ�����
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
