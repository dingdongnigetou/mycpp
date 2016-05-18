#ifndef __MYCPP_MYCOND_HPP__
#define __MYCPP_MYCOND_HPP__

#include "../mydefs.h"
#include "../mytypes.h"
#include "../noncopyable.h"
#include "MyMutex.hpp"
#include "../MSWinApi.hpp"

namespace mycpp
{
	class MyCond : noncopyable
	{
	private:
#ifdef _MSWINDOWS_
		bool isHadCondVar_;
		union {
			CONDITION_VARIABLE condVar;
			struct {
				unsigned int nWaitersCount;
				CRITICAL_SECTION lockerWaitersCount;
				HANDLE hSignalEvent;
				HANDLE hbroadcastEvent;
			}fallback;
		}hOS_;
#else
		pthread_cond_t hOS_;
		pthread_condattr_t attr_;
#endif

	public:
		typedef enum
		{
			COND_SUCCESS = 0,
			COND_ERR = -1,
			COND_TIMEOUT = -2,
		}CondErrno;

	public:
		MyCond()
		{
#ifdef _MSWINDOWS_	
			bzero(&hOS_, sizeof(hOS_));
			if (MSWinApi()->IsMSWndHaveCondVarApi())
			{
				MSWinApi()->InitializeConditionVariable(&hOS_.condVar);
				isHadCondVar_ = true;
			}
			else
			{
				isHadCondVar_ = false;

				hOS_.fallback.hSignalEvent = CreateEvent(nullptr,  /* no security */
					false, /* auto-reset event */
					false, /* non-signaled initially */
					nullptr); /* unnamed */

				MYABORTM(nullptr != hOS_.fallback.hSignalEvent,
					"MyCond::MyCond CreateEvent fail.");

				hOS_.fallback.hbroadcastEvent = CreateEvent(nullptr,
					true, false, nullptr);
				MYABORTM(nullptr != hOS_.fallback.hbroadcastEvent,
					"MyCond::MyCond CreateEvent fail.");

				/* initialize the count to 0. */

				InitializeCriticalSection(&hOS_.fallback.lockerWaitersCount);
			}
#else
			MYABORTM(0 == pthread_condattr_init(&attr_, nullptr),
				"MyCond::MyCond pthread_condattr_init fail.");

#define USE_CLOCK_MONOTONIC
#ifdef USE_CLOCK_MONOTONIC

			MYABORTM(0 == pthread_condattr_setclock(&pCond->stAttr, CLOCK_MONOTONIC),
				"MyCond::MyCond pthread_condattr_setclock CLOCK_MONOTONIC fail.");
#endif

			MYABORTM(0 == pthread_cond_init(&hOS_, &attr_),
				"MyCond::MyCond pthread_cond_init fail.");
#endif
		}

		~MyCond()
		{
#ifdef _MSWINDOWS_
			if (isHadCondVar_)
			{

			}
			else
			{
				MYASSERT(hOS_.fallback.nWaitersCount == 0);

				CloseHandle(hOS_.fallback.hbroadcastEvent);
				CloseHandle(hOS_.fallback.hSignalEvent);
				DeleteCriticalSection(&hOS_.fallback.lockerWaitersCount);
				hOS_.fallback.hbroadcastEvent = nullptr;
				hOS_.fallback.hSignalEvent = nullptr;
			}
#else
			pthread_cond_destroy(&hOS_);
			pthread_condattr_destroy(&attr_);
#endif

		}

		void Wait(MyMutex &mutex)
		{

#ifdef _MSWINDOWS_
			if (isHadCondVar_)
			{
				mutex.nLocked_--;
				MSWinApi()->SleepConditionVariableCS(&hOS_.condVar, &mutex.hOS_, INFINITE);
				mutex.nLocked_++;
			}
			else
			{
				CondWaitHelper(mutex, -1);
			}
#else
			mutex.nLocked_--;
			pthread_cond_wait(&hOS_, &mutex.hOS_);
			mutex.nLocked_++;
#endif
		}

		CondErrno WaitTimeout(MyMutex &mutex, int millisecond)
		{
			CondErrno eRet = COND_ERR;
#ifdef _MSWINDOWS_
			MYASSERT(millisecond >= 0);
			if (isHadCondVar_)
			{
				mutex.nLocked_--;
				if (MSWinApi()->SleepConditionVariableCS(&hOS_.condVar, &mutex.hOS_, millisecond))
				{
					eRet = COND_SUCCESS;
				}
				if (GetLastError() == ERROR_TIMEOUT)
				{
					eRet = COND_TIMEOUT;
				}
				mutex.nLocked_++;
			}
			else
			{
				eRet = CondWaitHelper(mutex, millisecond);
			}
#else
			struct timespec ts;
			int ret;
			int mseconds = millisecond;

#ifndef USE_CLOCK_MONOTONIC
			struct timeval tv;
			struct timezone tz;
			int sec, usec;

			gettimeofday(&tv, &tz);
			sec = mseconds / 1000;
			mseconds = mseconds - (sec * 1000);
			FDT_ASSERT(mseconds < 1000);
			usec = mseconds * 1000;
			FDT_ASSERT(tv.tv_usec < 1000000);
			ts.tv_sec = tv.tv_sec + sec;
			ts.tv_nsec = (tv.tv_usec + usec) * 1000;
			FDT_ASSERT(ts.tv_nsec < 2000000000);
			if (ts.tv_nsec > 999999999)
			{
				ts.tv_sec++;
				ts.tv_nsec -= 1000000000;
			}
#else
			clock_gettime(CLOCK_MONOTONIC, &ts);
			long sec = mseconds / 1000;
			mseconds = mseconds - (sec * 1000);
			ts.tv_sec += sec;
			ts.tv_nsec += mseconds * 1000000;

			assert(ts.tv_nsec < 2000000000);
			if (ts.tv_nsec > 999999999)
			{
				ts.tv_sec++;
				ts.tv_nsec -= 1000000000;
			}
#endif // USE_CLOCK_MONOTONIC
			mutex.nLocked_--;
			ret = pthread_cond_timedwait(&hOS_, &mutex.hOS_, &ts);
			mutex.nLocked_++;
			if (ret)
			{
				if (ret == ETIMEDOUT)
				{
					eRet = COND_TIMEOUT;
				}
			}
			else
			{
				eRet = COND_SUCCESS;
			}
#endif
			return eRet;
		}

		void Signal()
		{
#ifdef _MSWINDOWS_
			if (isHadCondVar_)
			{
				MSWinApi()->WakeConditionVariable(&hOS_.condVar);
			}
			else
			{
				// 		bool haveWaiters;
				// 
				// 
				// 		EnterCriticalSection(&hOS_.fallback.lockerWaitersCount);
				// 		haveWaiters = hOS_.fallback.nWaitersCount > 0;
				// 		LeaveCriticalSection(&hOS_.fallback.lockerWaitersCount);
				// 
				// 		if (haveWaiters)
				// 		{
				SetEvent(hOS_.fallback.hSignalEvent);
				//		}
			}
#else
			MYABORTM(0 == pthread_cond_signal(&hOS_), "MyCond::Signal pthread_cond_signal fail.");
#endif
		}

		void Broadcast()
		{
#ifdef _MSWINDOWS_
			if (isHadCondVar_)
			{
				MSWinApi()->WakeAllConditionVariable(&hOS_.condVar);
			}
			else
			{
				// 		bool haveWaiters;
				// 
				// 
				// 		EnterCriticalSection(&hOS_.fallback.lockerWaitersCount);
				// 		haveWaiters = hOS_.fallback.nWaitersCount > 0;
				// 		LeaveCriticalSection(&hOS_.fallback.lockerWaitersCount);
				// 
				// 		if (haveWaiters)
				// 		{
				SetEvent(hOS_.fallback.hbroadcastEvent);
				//		}
			}
#else
			MYABORTM(0 == pthread_cond_broadcast(&hOS_), "MyCond::Signal pthread_cond_broadcast fail.");
#endif
		}

#ifdef _MSWINDOWS_	
		CondErrno CondWaitHelper(MyMutex &mutex, int millisecond)
		{
			int last_waiter = 0;
			HANDLE handles[2] =
			{
				hOS_.fallback.hSignalEvent,
				hOS_.fallback.hbroadcastEvent
			};

			EnterCriticalSection(&hOS_.fallback.lockerWaitersCount);
			hOS_.fallback.nWaitersCount++;
			LeaveCriticalSection(&hOS_.fallback.lockerWaitersCount);

			mutex.Unlock();

			DWORD result = WaitForMultipleObjects(2, handles, false,
				millisecond >= 0 ? (DWORD)millisecond : INFINITE);

			mutex.Lock();
			EnterCriticalSection(&hOS_.fallback.lockerWaitersCount);
			hOS_.fallback.nWaitersCount--;
			last_waiter = result == WAIT_OBJECT_0 + 1 && hOS_.fallback.nWaitersCount == 0;
			LeaveCriticalSection(&hOS_.fallback.lockerWaitersCount);

			if (last_waiter)
			{
				ResetEvent(hOS_.fallback.hbroadcastEvent);
			}

			if (result == WAIT_OBJECT_0 || result == WAIT_OBJECT_0 + 1)
				return COND_SUCCESS;

			if (result == WAIT_TIMEOUT)
				return COND_TIMEOUT;

			return COND_ERR;
		}
#endif

    };
} // namespace mycpp

#endif // !__MYCPP_MYCOND_HPP__
