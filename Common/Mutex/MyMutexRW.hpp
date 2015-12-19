#ifndef __MYCPP_MYMUTEXRW_HPP__
#define __MYCPP_MYMUTEXRW_HPP__

#include "mytypes.h"
#include "noncopyable.h"

namespace mycpp
{
	typedef struct _StruRWMutexDebug  StruRWMutexDebug;

	class MyMutexRW : noncopyable
	{
	private:

		//调试下 获取到锁的线程id
		StruRWMutexDebug *m_pDBGThread;

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
		}m_hOS;
		bool m_isHadSRWlock;
		int m_isWLocked;
#else
		pthread_rwlock_t m_hOS;
		pthread_rwlockattr_t m_attr;
#endif

	public:

		MyMutexRW(void);
		~MyMutexRW(void);

		void LockRead(void);//加读锁
		void LockWrite(void);//加写锁
		void Unlock(void); //解锁


	private:
		void UnlockRead(void);
		void UnlockWrite(void);
	};

	class MyAutoReadLock : noncopyable
	{
	private:
		MyMutexRW &m_wrmutex;
	public:
		MyAutoReadLock(MyMutexRW &wrmutex);
		~MyAutoReadLock(void);
	};

	class MyAutoWriteLock : noncopyable
	{
	private:
		MyMutexRW &m_wrmutex;
	public:
		MyAutoWriteLock(MyMutexRW &wrmutex);
		~MyAutoWriteLock(void);
	};
} // namespace mycpp

#endif // !__MYCPP_MYMUTEXRW_HPP__

