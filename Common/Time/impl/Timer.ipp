#ifndef __MYCPP_TIMER_IPP__
#define __MYCPP_TIMER_IPP__

#include <set>
#include <atomic>

#include "../../Thread/ThreadPool.hpp"
#include "../../Utils.hpp"

namespace mycpp
{
	struct StruTimerRunThread
	{
		long ThreadID = 0;
		std::atomic_long NRefs = 1;
		bool IsRuning = false;
		std::atomic_int NOnlyOne = 0;

		StruTimerRunThread()
		{
		}
		~StruTimerRunThread()
		{
		}

		void Ref()
		{
			NRefs++;
		}
		void Unref()
		{
			if (--NRefs == 0)
			{
				delete this;
			}
		}
	};

	class TimerGlobal
	{
	private:
		std::shared_ptr<ThreadPool> threadPool_ = nullptr;
		std::atomic_long iRefs_ = 0;
		std::shared_ptr<Thread>  threadLoop_ = nullptr;
		MyMutex mutex_;
		std::set<Timer*> timerSet_;
	public:
		TimerGlobal()
		{
		}

		~TimerGlobal()
		{
			MYASSERT(iRefs_ == 0);

			threadLoop_->Stop();
			threadLoop_->Join();

			threadPool_->StopAll();
			threadPool_->JoinAll();
		}

		bool IsRun() const
		{
			return iRefs_ > 0;
		}

		bool Init(int nThreadNums)
		{
			if (++iRefs_ > 1)
			{
				return true;
			}
			if (nThreadNums < 1)
			{
				nThreadNums = std::thread::hardware_concurrency() * 2 + 3;
			}
			threadPool_ = std::make_shared<ThreadPool>(1, nThreadNums, 120);
			threadLoop_ = std::make_shared<Thread>();
			if (nullptr == threadPool_)
			{
				iRefs_--;
				return false;
			}
			if (!threadLoop_->Start([this](Thread& t) { Loop(); }))
			{
				iRefs_--;
				return false;
			}
			return true;
		}

		void Uninit()
		{
			long v = iRefs_--;
			MYABORT(v >= 0);
			if (v == 0)
			{
				threadLoop_->Stop();
				threadLoop_->Join();
				if (nullptr != threadPool_)
				{
					threadPool_->StopAll();
					threadPool_->JoinAll();
				}
			}
		}

		void AddTimer(Timer* p)
		{
			MyAutoMutex locker(mutex_);
			MYASSERT(timerSet_.find(p) == timerSet_.end());
			timerSet_.insert(p);
		}

		void RemoveTimer(Timer* p)
		{
			MyAutoMutex locker(mutex_);
			timerSet_.erase(p);
		}

		void Loop()
		{
			while (!threadLoop_->TestExit() && iRefs_ > 0)
			{
				{
					MyAutoMutex l(mutex_);
					for (auto timer : timerSet_)
					{
						if (nullptr == timer)
							continue;

						if (!timer->runThread_->IsRuning && timer->isStarted_ && timer->meter_.GetElapsed() >= timer->interval_)
						{
							timer->runThread_->IsRuning = true;
							timer->meter_.Reset();
							if (!threadPool_->Start([timer]{ timer->runCallBack(); }))
							{
								MYASSERT(0);
								timer->runThread_->IsRuning = false;
							}
						}
					}
				}

				Utils()->MySleep(1);
			}
		}

	}; // class TimerGlobal

	TimerGlobal Timer::timer_global_;
	Timer::Timer()
	{
		runThread_ = std::make_shared<StruTimerRunThread>();
	}
	Timer::~Timer()
	{
		Stop(false);

		if (nullptr == runThread_)
			runThread_->Unref();
	}

	bool Timer::ModuleInit(int nThreadNums /* =1 */)
	{
		return timer_global_.Init(nThreadNums);
	}

	void Timer::ModuleUninit()
	{
		timer_global_.Uninit();
	}

	bool Timer::Init(int nInterval, TimerCallBackType fnCb)
	{
		MyAutoMutex locker(mutex_);
		if (nInterval < 0 || fnCb == nullptr || callback_ != nullptr || runThread_ == nullptr)
		{
			MYASSERT(0);
			return false;
		}

		callback_ = fnCb;
		interval_ = nInterval;

		return true;
	}

	bool Timer::Start(bool isRunImmediately)
	{
		MyAutoMutex locker(mutex_);

		if (callback_ == nullptr)
		{
			return false;
		}
		if (isStarted_)
		{
			return true;
		}
		if (isRunImmediately)
		{
			meter_.Reset(0);
		}
		else
		{
			meter_.Reset();
		}

		isStarted_ = true;
		timer_global_.AddTimer(this);
		return isStarted_;
	}

	void Timer::Stop(bool bWait /* = false */)
	{
		{
			MyAutoMutex l(mutex_);
			if (!isStarted_)
			{
				return;
			}
			timer_global_.RemoveTimer(this);
			isStarted_ = false;
		}

		if (bWait)
		{
			if (runThread_->IsRuning)
			{
				if (runThread_->ThreadID == Thread::GetCurrentThreadID())
				{
					return;
				}

				while (runThread_->IsRuning && timer_global_.IsRun())
				{
					Utils()->MySleep(10);
				}
			}
		}
	}

	void Timer::runCallBack()
	{
		runThread_->Ref();
		if (++runThread_->NOnlyOne != 1)
		{
			MYASSERT(0);
			runThread_->NOnlyOne--;
			runThread_->Unref();
			return;
		}
		runThread_->ThreadID = Thread::GetCurrentThreadID();
		callback_();
		runThread_->ThreadID = 0;
		runThread_->NOnlyOne--;
		runThread_->IsRuning = false;
		runThread_->Unref();
	}



}

#endif // !__MYCPP_TIMER_IPP__
