#ifndef __MYCPP_TIMER_HPP__
#define __MYCPP_TIMER_HPP__

#include <function>

#include "../noncopyable.h"

namespace mycpp
{
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

		// ��ʼ��ʱ�����Լ��ص�
		bool Init(int nInterval, TimerCallBackType fnCb);

		//��ʼ�ص�, isRunImmediately �Ƿ�����ִ��һ��
		bool Start(bool isRunImmediately);

		//bWait ������ڵ��ȣ� �Ƿ�ȴ���ǰ�������
		void Stop(bool bWait = true);

	};
}

#endif // !__MYCPP_TIMER_HPP__
