
#ifndef __MYCPPP_MYMUTEX_HPP__
#define __MYCPPP_MYMUTEX_HPP__

#include "mytypes.h"
#include "noncopyable.h"

namespace mycpp
{
	class MyMutex : noncopyable
	{
	private:
		friend class MyCond;
		int m_nLocked;
#ifdef _MSWINDOWS_
		CRITICAL_SECTION m_hOS;
#else
		pthread_mutex_t m_hOS;
#endif
	public:
		MyMutex(void);
		~MyMutex(void);
		void	Lock();		//����
		void	Unlock();	//����
		bool  TryLock(); //true��ȡ���ɹ���falseʧ��
	};

	class MyAutoMutex : noncopyable
	{
	private:
		MyMutex &m_mutex;
	public:
		MyAutoMutex(MyMutex &mutex);

		~MyAutoMutex(void);
	};
} // namespace mycpp

#endif // !__MYCPP_MYMUTEX_HPP__
