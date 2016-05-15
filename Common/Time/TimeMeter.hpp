
#ifndef  __MYCPP_TIMEMETER_HPP__
#define  __MYCPP_TIMEMETER_HPP__

#include "../mytypes.h"

namespace mycpp
{
	// ��ʱ��
	class TimeMeter
	{
	public:
		// �Ե�ǰϵͳ��ʱ�俪ʼ��ʱ
		void Reset();

		// ��iLastTvΪ��㿪ʼ��ʱ
		void Reset(UInt64 iLastTv);

		UInt64 GetElapsed() const; //�������ŵĺ�����

	public:
		TimeMeter();
		~TimeMeter();

	private:
		UInt64 lasttv_;

	};

}

#include "impl\TimeMeter.ipp"

#endif // ! __MYCPP_TIMEMETER_HPP__
