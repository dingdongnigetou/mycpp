#ifndef __MYCPP_TIMER_HPP__
#define __MYCPP_TIMER_HPP__

#include <functional>
#include <memory>

#include "../noncopyable.h"
#include "../Mutex/MyMutex.hpp"

namespace mycpp
{
	class TimerGlobal;
	struct StruTimerRunThread;

	class Timer : noncopyable
	{
	public:
		// ��ʱ���ص��հ�����
		using TimerCallBackType = std::function<void(void)>;

		Timer();
		~Timer();

	public: // �ӿ�

		// ��ʼ����ʱ��ģ��, nThreadNums �߳����� 
		// ���Ϊ 1 ����timerֻʹ��һ���̻߳ص��� ������timer ʹ�õ��Ƕ���̲߳����ص�
		// ��ʹ�������κνӿ�ǰ����
		static bool ModuleInit(int nThreadNums = 1);

		// �ͷż�ʱ����Դ
		static void ModuleUninit();

		// ��ʼ��ʱ����(ms)�Լ��ص�
		bool Init(int nInterval, TimerCallBackType fnCb);

		//��ʼ�ص�, isRunImmediately �Ƿ�����ִ��һ��
		bool Start(bool isRunImmediately);

		//bWait ������ڵ��ȣ� �Ƿ�ȴ���ǰ�������
		void Stop(bool bWait = false);

	private:
		void runCallBack();

	private:
		friend class TimerGlobal;
		static TimerGlobal timer_global_;

		std::shared_ptr<StruTimerRunThread> runThread_ = nullptr;
		long timerID_ = 0;
		bool isStarted_ = false;
		int interval_ = 10;
		MyMutex mutex_;
		TimeMeter meter_;
		TimerCallBackType callback_;
	};
}

#include "impl\Timer.ipp"

#endif // !__MYCPP_TIMER_HPP__
