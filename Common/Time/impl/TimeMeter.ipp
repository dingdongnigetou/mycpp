
#ifndef  __MYCPP_TIMEMETER_IPP__
#define  __MYCPP_TIMEMETER_IPP__

#include "../DateTime.hpp"

namespace mycpp
{
	void TimeMeter::Reset()
	{
		lasttv_ = DateTime::GetPerformanceCounter();
	}

	void TimeMeter::Reset(UInt64 iLastTv)
	{
		lasttv_ = iLastTv;
	}

	UInt64 TimeMeter::GetElapsed() const
	{
		auto cur = DateTime::GetPerformanceCounter();
		return cur - lasttv_;
	}

	TimeMeter::TimeMeter()
	{
		Reset();
	}
	TimeMeter::~TimeMeter()
	{
	}
}

#endif // ! __MYCPP_TIMEMETER_IPP__
