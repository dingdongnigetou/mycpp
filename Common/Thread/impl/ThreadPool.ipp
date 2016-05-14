#ifndef __MYCPP_THREAD_POOL_IPP__
#define __MYCPP_THREAD_POOL_IPP__



namespace mycpp
{
	ThreadPool::ThreadPool(int minCapacity, int maxCapacity, int idleTime)
		:minCapacity_(minCapacity),
		maxCapacity_(maxCapacity),
		idleTime_(idleTime * 1000)
	{
	}

	ThreadPool::~ThreadPool(void)
	{
		StopAll();
		JoinAll();
	}

	bool ThreadPool::IsSelfThread(long threaID)
	{
		MyAutoMutex l(mutex_);
		bool bret = false;
		for (auto& t : threads_)
		{
			if (t->GetThreadID() == threaID)
			{
				bret = true;
				break;
			}
		}
		return bret;
	}


	//增加或减少线程数，是否需要？
	void ThreadPool::AddCapacity(int n)
	{
		if (n > 0)
		{
			maxCapacity_ = n;
		}
	}

	int ThreadPool::GetCapacity(void) const
	{
		return nCapacity_;
	}

	void ThreadPool::StopAll(void)
	{
		MyAutoMutex lock(mutex_);
		isrun_ = false;
		for (auto& t : threads_)
			t->Join();

		cond_.Broadcast();
	}

	void ThreadPool::JoinAll(void)
	{
		for (auto& t : threads_)
			t->Join();
	}

	int ThreadPool::GetWaits(void)
	{
		MyAutoMutex locker(mutex_);
		return tasks_.size();
	}

	template<typename Fun, typename... Args>
	bool ThreadPool::Start(Fun&& fun, Args&&... args)
	{
		if (!isrun_)
		{
			return false;
		}
		mutex_.Lock();
		tasks_.push_back([fun, args...]{ fun(args...); });
		if (idles_)
		{
			cond_.Signal();
		}
		else if (nCapacity_ < maxCapacity_)
		{
			addThread();
		}
		mutex_.Unlock();
		return true;
	}

	void ThreadPool::addThread()
	{
		std::shared_ptr<Thread> p;
		bool isFound = false;
		for(auto& t : threads_)
		{
			p = t;
			if (!p->IsRunning())
			{
				isFound = true;
				break;
			}
		}

		if (!isFound)
		{
			p = std::make_shared<Thread>();
			MYABORT(p, "ThreadPool::addThread no memory.");
			threads_.push_back(p);
		}

		nCapacity_++;

		MYABORT(p->Start([this](Thread& t) { threadEntry(t); }), 
			"ThreadPool::addThread start fail.");
	}

	void ThreadPool::threadEntry(Thread &thread)
	{
		UInt32 ticks, cur;
		bool isIdle = false;
		MyAutoMutex locker(mutex_);
		while (isrun_)
		{
			if (tasks_.empty())
			{
				if (idleTime_ > 0)
				{
					if (isIdle)
					{
						cur = DateTime::GetTickCount();
						if (nCapacity_ > minCapacity_ &&
							idleTime_ > 0 &&
							DateTime::CountElapsed(ticks, cur) > (UInt32)idleTime_)
						{
							//退出进程
							MYPRINTF("threadpool thread  %ld idle exit.\n",
								thread.GetThreadID());
							break;
						}
					}
					else
					{
						ticks = DateTime::GetTickCount();
					}
				}
				idles_++;
				cond_.WaitTimeout(mutex_, 1000);
				idles_--;
				continue;
			}
			auto task = tasks_.front();
			tasks_.pop_front();
			mutex_.Unlock();
			task();
			mutex_.Lock();
		}
		thread.Stop();
		nCapacity_--;
	}

} // end namepsace					}

#endif // !__MYCPP_THREAD_POOL_IPP__