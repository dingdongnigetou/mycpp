
#ifndef MYCPP_MYMUTEXRW_HPP
#define MYCPP_MYMUTEXRW_HPP

#include "mytypes.h"

#include "MyMutex.hpp"
#include "noncopyable.h"

#ifdef _LINUX
#include <unistd.h>
#include <pthread.h>
#endif

typedef struct _StruRWMutexDebug  StruRWMutexDebug;

namespace mycpp
{
	class MyMutexRW
	{
	private:

		//������ ��ȡ�������߳�id
		StruRWMutexDebug *m_pDBGThread;

#ifdef _MSWINDOWS_
		union {
			SRWLOCK srwlock;
			struct {
				CRITICAL_SECTION locker;
				HANDLE hReadEvent;
				HANDLE hWriteEvent;
				int nReaders; //���� �Ѿ������ĸ���
				int nWaitReaders; //���� �ȴ������ĸ���
				int nWriters; //д�� �ȴ������ĸ���+�Ѿ������ĸ���			
				int flags;
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

		void LockRead(void);//�Ӷ���
		void LockWrite(void);//��д��
		void Unlock(void); //����

	private:
		void UnlockRead(void);
		void UnlockWrite(void);
	};

	class MyAutoReadLock : public noncopyable
	{
	private:
		MyMutexRW &m_wrmutex;
	public:
		MyAutoReadLock(MyMutexRW &wrmutex);
		~MyAutoReadLock(void);
	};

	class MyAutoWriteLock : public noncopyable
	{
	private:
		MyMutexRW &m_wrmutex;
	public:
		MyAutoWriteLock(MyMutexRW &wrmutex);
		~MyAutoWriteLock(void);
	};
} // mycpp

#endif // MYCPP_MYMUTEXRW_HPP
