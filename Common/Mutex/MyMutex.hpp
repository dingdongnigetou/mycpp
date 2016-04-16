
#ifndef __MYCPPP_MYMUTEX_HPP__
#define __MYCPPP_MYMUTEX_HPP__

#include "mytypes.h"
#include "noncopyable.h"

namespace mycpp
{
	class Mycond;
	class MyMutex : noncopyable
	{
	private:
		friend class MyCond;
		int nLocked_;
#ifdef _MSWINDOWS_
		CRITICAL_SECTION hOS_;
#else
		pthread_mutex_t hOS_;
#endif

	public:
		MyMutex() : nLocked_(0)
		{
#ifdef _MSWINDOWS_
			InitializeCriticalSection(&hOS_);
#else
			MYABORTM(0 == pthread_mutex_init(&hOS_, nullptr), "MyMutex::MyMutex pthread_mutex_init fail.");
#endif
		}

		MyMutex::~MyMutex()
		{
#ifdef _MSWINDOWS_
			DeleteCriticalSection(&hOS_);
#else
			pthread_mutex_destroy(&hOS_);
#endif

		}
		void MyMutex::Lock()
		{
#ifdef _MSWINDOWS_
			EnterCriticalSection(&hOS_);
			MYABORTM(nLocked_ == 0, "MyMutex::Lock Recursive lock");
			nLocked_++;
#else
			MYABORTM(0 == pthread_mutex_lock(&hOS_), "MyMutex::Lock pthread_mutex_lock fail.\n");
			MYABORTM(nLocked_ == 0, "MyMutex::Lock Recursive lock");
			nLocked_++;
#endif
		}

		void MyMutex::Unlock()
		{
#ifdef _MSWINDOWS_
			MYABORTM(0 != nLocked_, "MyMutex::Unlock unlock no locked");
			nLocked_--;
			LeaveCriticalSection(&hOS_);

#else
			MYABORTM(0 != nLocked_, "MyMutex::Unlock unlock no locked");
			nLocked_--;
			MYABORTM(0 == pthread_mutex_unlock(&hOS_), "MyMutex::Unlock pthread_mutex_unlock fail.\n");
#endif
		}

		bool MyMutex::TryLock()
		{
#ifdef _MSWINDOWS_
			if (!TryEnterCriticalSection(&hOS_))
			{
				return false;
			}
			MYABORTM(nLocked_ == 0, "MyMutex::TryLock Recursive lock");
			nLocked_++;
#else
			if (0 != pthread_mutex_trylock(&hOS_))
			{
				return false;
			}
			MYABORTM(nLocked_ == 0, "MyMutex::TryLock Recursive lock");
			nLocked_++;
#endif
			return true;
		}

	}; // end class MyMutex

	class MyAutoMutex : noncopyable
	{
	private:
		MyMutex &mutex_;
	public:
		MyAutoMutex::MyAutoMutex(MyMutex &mutex)
			:mutex_(mutex)
		{
			mutex_.Lock();
		}

		MyAutoMutex::~MyAutoMutex()
		{
			mutex_.Unlock();
		}
	};
} // namespace mycpp

#endif // !__MYCPP_MYMUTEX_HPP__
