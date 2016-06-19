#ifndef __MYCPP_MYMUTEXRW_HPP__
#define __MYCPP_MYMUTEXRW_HPP__

#include "../mytypes.h"
#include "../noncopyable.h"

namespace mycpp
{
	class MyMutexRW : noncopyable
	{
	private:
#ifdef _MSWINDOWS_
		union {
			SRWLOCK srwlock;
			struct {
				CRITICAL_SECTION locker;
				HANDLE hReadEvent;
				HANDLE hWriteEvent;
				int nReaders; //读锁 已经加锁的个数
				int nWaitReaders; //读锁 等待加锁的个数
				int nWriters; //写锁 等待加锁的个数+已经加锁的个数			
				int flaMy;
			}fallback;
		}hOS_;
		bool sHadSRWlock_;
		int isWLocked_;
#else
		pthread_rwlock_t hOS_;
		pthread_rwlockattr_t attr_;
#endif

	public:
		MyMutexRW()
		{
#ifdef _MSWINDOWS_
			isWLocked_ = 0;
			bzero(&hOS_, sizeof(hOS_));
			if (IsMSWndHaveSRWLockApi())
			{
				pInitializeSRWLock(&hOS_.srwlock);
				sHadSRWlock_ = true;
			}
			else
			{
				sHadSRWlock_ = false;
				InitializeCriticalSection(&hOS_.fallback.locker);
				hOS_.fallback.hReadEvent = CreateEvent(nullptr, true, false, nullptr);
				MYABORTM(nullptr != hOS_.fallback.hReadEvent,
					"MyMutexRW::MyMutexRW CreateEvent fail.");
				hOS_.fallback.hWriteEvent = CreateEvent(nullptr, false, false, nullptr);
				MYABORTM(nullptr != hOS_.fallback.hWriteEvent,
					"MyMutexRW::MyMutexRW CreateEvent fail.");
			}
#else
			MYABORTM(0 == pthread_rwlockattr_init(&attr_, nullptr),
				"MyMutexRW::MyMutexRW pthread_rwlockattr_init fail.");

			//设置写锁优先
			pthread_rwlockattr_setkind_np(&attr_,
				PTHREAD_RWLOCK_PREFER_WRITER_NONRECURSIVE_NP);
			MYABORTM(0 == pthread_rwlock_init(&hOS_, &attr_),
				"MyMutexRW::MyMutexRW pthread_rwlock_init fail.");
#endif
		}

		~MyMutexRW()
		{
#ifdef _MSWINDOWS_
			if (sHadSRWlock_)
			{

			}
			else
			{
				MYASSERT(0 == hOS_.fallback.nReaders &&
					0 == hOS_.fallback.nWriters);

				CloseHandle(hOS_.fallback.hReadEvent);
				CloseHandle(hOS_.fallback.hWriteEvent);
				hOS_.fallback.hReadEvent = nullptr;
				hOS_.fallback.hWriteEvent = nullptr;
				DeleteCriticalSection(&hOS_.fallback.locker);
			}
#else
			pthread_rwlock_destroy(&hOS_);
			pthread_rwlockattr_destroy(&attr_);
#endif
		}

		void LockRead()
		{
#ifdef _MSWINDOWS_
			if (sHadSRWlock_)
			{
				pAcquireSRWLockShared(&hOS_.srwlock);
			}
			else
			{
				EnterCriticalSection(&hOS_.fallback.locker);
				while (1)
				{
					if (hOS_.fallback.nWriters)
					{
						//等待读锁事件信号							
						hOS_.fallback.nWaitReaders++;
						LeaveCriticalSection(&hOS_.fallback.locker);
						WaitForSingleObject(hOS_.fallback.hReadEvent, INFINITE);
						EnterCriticalSection(&hOS_.fallback.locker);
						hOS_.fallback.nWaitReaders--;
					}
					else
					{
						hOS_.fallback.nReaders++;
						LeaveCriticalSection(&hOS_.fallback.locker);
						break;
					}
				}
			}
#else
			MYABORTM(0 == pthread_rwlock_rdlock(&hOS_),
				"MyMutexRW::LockRead pthread_rwlock_rdlock fail.");
#endif
			MYASSERT(isWLocked_ == 0);
		}

		void LockWrite()
		{
#ifdef _MSWINDOWS_
			if (sHadSRWlock_)
			{
				pAcquireSRWLockExclusive(&hOS_.srwlock);
			}
			else
			{
				EnterCriticalSection(&hOS_.fallback.locker);
				if (hOS_.fallback.flags&WRMUTEX_RD_EVT_EXIST)
				{
					//清除读信号
					ResetEvent(hOS_.fallback.hReadEvent);
					hOS_.fallback.flags &= ~WRMUTEX_RD_EVT_EXIST;
				}
				hOS_.fallback.nWriters++;

				if (hOS_.fallback.nReaders || hOS_.fallback.nWriters > 1)
				{
					LeaveCriticalSection(&hOS_.fallback.locker);
					WaitForSingleObject(hOS_.fallback.hWriteEvent, INFINITE);
				}
				else
				{
					LeaveCriticalSection(&hOS_.fallback.locker);
				}
			}
#else
			MYABORTM(0 == pthread_rwlock_wrlock(&hOS_),
				"MyMutexRW::LockWrite pthread_rwlock_wrlock fail.");
#endif
			isWLocked_++;
		}

		void Unlock()
		{
			if (isWLocked_)
			{
				isWLocked_--;
				MYABORTM(0 == isWLocked_, "MyMutexRW::Unlock assert.");
				UnlockWrite();
			}
			else
			{
				UnlockRead();
			}
		}

	private:
		void UnlockRead()
		{
#ifdef _MSWINDOWS_
			if (sHadSRWlock_)
			{
				pReleaseSRWLockShared(&hOS_.srwlock);
			}
			else
			{
				EnterCriticalSection(&hOS_.fallback.locker);
				hOS_.fallback.nReaders--;
				if (0 == hOS_.fallback.nReaders  &&
					hOS_.fallback.nWriters)
				{
					//唤醒写线程
					LeaveCriticalSection(&hOS_.fallback.locker);
					SetEvent(hOS_.fallback.hWriteEvent);
				}
				else
				{
					LeaveCriticalSection(&hOS_.fallback.locker);
				}
			}
#else
			MYABORTM(0 == pthread_rwlock_unlock(&hOS_),
				"MyMutexRW::UnlockRead pthread_rwlock_unlock fail.");
#endif
		}

		void UnlockWrite()
		{
#ifdef _MSWINDOWS_
			if (sHadSRWlock_)
			{
				pReleaseSRWLockExclusive(&hOS_.srwlock);
			}
			else
			{
				EnterCriticalSection(&hOS_.fallback.locker);
				hOS_.fallback.nWriters--;
				if (hOS_.fallback.nWriters == 0)
				{

					if (hOS_.fallback.nWaitReaders &&
						!(hOS_.fallback.flags&WRMUTEX_RD_EVT_EXIST))
					{
						hOS_.fallback.flags |= WRMUTEX_RD_EVT_EXIST;
						LeaveCriticalSection(&hOS_.fallback.locker);
						SetEvent(hOS_.fallback.hReadEvent);
					}
					else
					{
						LeaveCriticalSection(&hOS_.fallback.locker);
					}
				}
				else
				{
					SetEvent(hOS_.fallback.hWriteEvent);
					LeaveCriticalSection(&hOS_.fallback.locker);
				}


			}
#else
			MYABORTM(0 == pthread_rwlock_unlock(&hOS_),
				"MyMutexRW::UnlockWrite pthread_rwlock_unlock fail.");
#endif
		};

		class MyAutoReadLock : noncopyable
		{
	private:
		MyMutexRW &wrmutex_;
	public:
		MyAutoReadLock(MyMutexRW &wrmutex) : wrmutex_(wrmutex)
		{
			wrmutex.LockRead();
		}

		~MyAutoReadLock()
		{
			wrmutex_.Unlock();
		}
	};

	class MyAutoWriteLock : noncopyable
	{
	private:
		MyMutexRW &wrmutex_;
	public:
		MyAutoWriteLock::MyAutoWriteLock(MyMutexRW &wrmutex) : wrmutex_(wrmutex)
		{
			wrmutex.LockWrite();
		}

		MyAutoWriteLock::~MyAutoWriteLock()
		{
			wrmutex_.Unlock();
		}
	};
} // namespace mycpp

#endif // !__MYCPP_MYMUTEXRW_HPP__
