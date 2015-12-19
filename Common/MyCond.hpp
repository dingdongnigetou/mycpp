
#ifndef MYCPP_MYCOND_HPP
#define MYCPP_MYCOND_HPP

#include "mytypes.h"

#include "MyMutex.hpp"
#include "noncopyable.h"

#ifdef _LINUX
#include <unistd.h>
#include <pthread.h>
#endif

namespace mycpp
{
	class MyCond : public noncopyable
	{
	private:
#ifdef _MSWINDOWS_
		bool m_isHadCondVar;
		union {
			CONDITION_VARIABLE condVar;
			struct {
				unsigned int nWaitersCount;
				CRITICAL_SECTION lockerWaitersCount;
				HANDLE hSignalEvent;
				HANDLE hbroadcastEvent;
			}fallback;
		}m_hOS;
#else
		pthread_cond_t m_hOS;
		pthread_condattr_t m_attr;
#endif

	public:
		typedef enum
		{
			COND_SUCCESS = 0,
			COND_ERR = -1,
			COND_TIMEOUT = -2,
		}CondErrno;
		MyCond(void);
		~MyCond(void);

		void Wait(MyMutex &mutex);
		MyCond::CondErrno  WaitTimeout(MyMutex &mutex, int millisecond);

		void Signal(void);
		void Broadcast(void);

	private:
#ifdef _MSWINDOWS_
		MyCond::CondErrno CondWaitHelper(MyMutex &mutex, int millisecond);
#endif
	};

} // mycpp

#endif // MYCPP_MYCOND_HPP
