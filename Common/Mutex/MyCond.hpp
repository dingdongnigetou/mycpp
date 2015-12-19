#define __MYCPP_MYCOND_HPP__
#define __MYCPP_MYCOND_HPP__

#include "mytypes.h"
#include "noncopyable.h"
#include "MyMutex.hpp"

namespace mycpp
{
	class MyCond : noncopyable
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
} // namespace mycpp

#endif // !__MYCPP_MYCOND_HPP__
