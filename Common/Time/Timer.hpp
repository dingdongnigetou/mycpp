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
		// 定时器回调闭包类型
		using TimerCallBackType = std::function<void(void)>;

		Timer();
		~Timer();

	public: // 接口

		// 初始化定时器模块, nThreadNums 线程数， 
		// 如果为 1 所有timer只使用一个线程回调， 否则多个timer 使用的是多个线程并发回调
		// 在使用以下任何接口前调用
		static bool ModuleInit(int nThreadNums = 1);

		// 释放计时器资源
		static void ModuleUninit();

		// 初始化时间间隔(ms)以及回调
		bool Init(int nInterval, TimerCallBackType fnCb);

		//开始回调, isRunImmediately 是否立刻执行一次
		bool Start(bool isRunImmediately);

		//bWait 如果正在调度， 是否等待当前任务结束
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

#include "impl/Timer.ipp"

#endif // !__MYCPP_TIMER_HPP__
