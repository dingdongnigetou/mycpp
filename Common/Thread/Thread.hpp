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
	class Thread;
	typedef  void(*FunPtrThreadCallback)(Thread &thread, void *pThreadData);

	class Thread : noncopyable
	{
	public:
		Thread();
		virtual ~Thread();

		//��ʼ�̣߳��ɹ�����true,ʧ�ܷ���false	
		//֧�ֱհ���ԭ������
		bool Start(FunPtrThreadCallback fnOnEvent, void *pThreadDat);	 

		//ֹͣ�̣߳��ɹ�����true,ʧ�ܷ���false	
		void Stop();	

		//�ж��߳��Ƿ��˳������̺߳�����ѭ���е��ã�ִ����ͣ�ͻ��Ѳ���	
		bool TestExit();	

		//�ȴ��߳̽���
		void Join();	

		// ȡ��������̵߳İ�
		void Detach();

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
		void joinInner();

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
		FunPtrThreadCallback fnUser_ = nullptr;
		void *pFnThreadData_ = nullptr;
		void *pUserData_ = nullptr;
	};
}
#include "impl/Thread.ipp"

#endif // !__MYCPP_THREAD_HPP__
