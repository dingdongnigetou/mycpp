#ifndef __MYCPP_THREAD_HPP__
#define __MYCPP_THREAD_HPP__

#include <thread>
#include <functional>

#ifdef _LINUX
#include <pthread.h>
#endif

#include "../mydefs.h"
#include "../noncopyable.h"
#include "../Mutex/MyMutex.hpp"
#include "../Mutex/MyCond.hpp"


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
		bool Start(Fn f, Args ...args);	 

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
	};
}

#ifdef _MSWINDOWS_
#include <process.h>    /* _beginthread, _endthread */
#endif

#include "impl/Thread.ipp"

#endif // !__MYCPP_THREAD_HPP__
