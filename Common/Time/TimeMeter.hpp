
#ifndef  __MYCPP_TIMEMETER_HPP__
#define  __MYCPP_TIMEMETER_HPP__

#include "../mytypes.h"

namespace mycpp
{
	// 计时器
	class TimeMeter
	{
	public:
		// 以当前系统的时间开始计时
		void Reset();

		// 以iLastTv为起点开始计时
		void Reset(UInt64 iLastTv);

		UInt64 GetElapsed() const; //返回流逝的毫秒数

	public:
		TimeMeter();
		~TimeMeter();

	private:
		UInt64 lasttv_;

	};

}

#include "impl\TimeMeter.ipp"

#endif // ! __MYCPP_TIMEMETER_HPP__
