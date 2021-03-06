#ifndef __MYCPP_DATATIME_H__
#define __MYCPP_DATATIME_H__

#include <time.h>
#include <stdio.h>
#include <string>

#include "../mydefs.h"

#ifndef _MSWINDOWS_
#include <unistd.h>
#include <sys/times.h>
#endif

namespace mycpp
{
	class DateTime{
		tm date_;// 日期
		time_t seconds_;// 自1970起的秒数
	public:
		explicit DateTime(void) : DateTime(time(0)) {}
		DateTime(time_t seconds) : seconds_(seconds)
		{
			localtime_all(&this->date_, &this->seconds_);
		}
		explicit DateTime(int year, int month, int day) : DateTime(year, month, day, 0, 0, 0) {}
		DateTime(int year, int month, int day, int hour, int minute, int second)
		{
			format(year, month, day, hour, minute, second);
		}
		explicit DateTime(const std::string& strTime) : DateTime(strTime, "%d-%d-%d %d:%d:%d") {}
		//日期字符串格式 年-月-日 时:分:秒 例:2008-02-03 9:30:20 
		explicit DateTime(const std::string& strTime, const std::string& strFormat)
		{
			format(strTime, strFormat);
		}

		// 拷贝构造
		DateTime(const DateTime& rhs)
		{
			*this = rhs;
		}
		const DateTime& operator=(const DateTime& rhs)
		{
			if (this == &rhs) {
				return *this;
			}
			this->seconds_ = rhs.seconds_;
			localtime_all(&this->date_, &this->seconds_);
			return *this;
		}
	public:
		// 获取时间戳
		time_t GetTimestamp(void) const { return seconds_; }

		// 获取时间
		int GetYear(void) const { return date_.tm_year + 1900; }
		int GetMonth(void) const { return date_.tm_mon + 1; }
		int GetDay(void) const { return date_.tm_mday; }
		int GetHour(void) const { return date_.tm_hour; }
		int GetMinute(void) const { return date_.tm_min; }
		int GetSecond(void) const { return date_.tm_sec; }

		// 添加时间
		void AddYears(int years)
		{
			auto year = date_.tm_year + years;
			if (!(year >= 1970 && year < 3000)) {
				return;
			}
			date_.tm_year += years;
			seconds_ = mktime(&date_);
		}
		void AddMonths(int months)
		{
			int year = (int)((date_.tm_mon + months) / 12);
			date_.tm_year += year;
			date_.tm_mon += (date_.tm_mon + months) % 12 - 1;
			seconds_ = mktime(&date_);
		}
		DateTime AddWeeks(int weeks) { return AddDays(weeks * 7); }
		DateTime AddDays(int days) { return AddHours(days * 24); }
		DateTime AddHours(int hours) { return AddMinutes(hours * 60); }
		DateTime AddMinutes(int minutes) { return AddSeconds(minutes * 60); }
		DateTime AddSeconds(int seconds)
		{
			seconds_ += seconds;
			localtime_all(&date_, &seconds_);

			return *this;
		}

		// 除减时间
		DateTime SubYears(int years)
		{
			date_.tm_year -= years;
			seconds_ = mktime(&date_);
			return *this;
		}
		DateTime SubMonths(int months)
		{
			int year = (int)((date_.tm_mon + months) / 12);
			date_.tm_year -= year;
			date_.tm_mon -= (date_.tm_mon + months) % 12 - 1;
			seconds_ = mktime(&date_);

			return *this;
		}
		DateTime SubWeeks(int weeks) { return SubDays(weeks * 7); }
		DateTime SubDays(int days) { return SubHours(days * 24); }
		DateTime SubHours(int hours) { return SubMinutes(hours * 60); }
		DateTime SubMinutes(int minutes) { return SubSeconds(minutes * 60); }
		DateTime SubSeconds(int seconds) 
		{ 
			seconds_ -= seconds;
			if (seconds_ < 0)
				seconds_ = 0;
			localtime_all(&date_, &seconds_);

			return *this;
		}

		// 判断上下午
		bool IsAM() const { return date_.tm_hour < 12; }
		bool IsPM() const { return date_.tm_hour >= 12; }

		//  获取此实例所表示的一年里面的第几天,从1月1日起,0-365
		int DayOfYear(void) const { return date_.tm_yday; }
		// 获取此实例所表示的日期是几号
		int DayOfMonth(void) const { return date_.tm_mday; }
		// 获取此实例所表示的日期是星期几
		int DayOfWeek(void) const { return date_.tm_wday; }

		// 将当前 DateTime 对象的值转换为其等效的短日期字符串表示形式
		// "%Y-%m-%d %H:%M:%S" %Y=年 %m=月 %d=日 %H=时 %M=分 %S=秒
		// %Y-%m-%d
		std::string ToYmdString(void) const { return ToString("%Y-%m-%d"); }
		// %Y%m%d
		std::string ToYmdString2(void) const { return ToString("%Y%m%d"); }
		// %H:%M:%S
		std::string ToHmsString(void) const { return ToString("%H:%M:%S"); }
		// %H%M%S
		std::string ToHmsString2(void) const { return ToString("%H%M%S"); }
		//%Y-%m-%d %H:%M:%S
		std::string ToString(void) const { return ToString("%Y-%m-%d %H:%M:%S"); }
		//%Y%m%d %H%M%S
		std::string ToString2(void) const { return ToString("%Y%m%d%H%M%S"); }

		std::string ToString(const std::string& strFormat) const
		{
			if (seconds_ <= 0) {
				return std::string();
			}
			try {
				char str[40] = { 0 };
				strftime(str, 40, strFormat.c_str(), &date_);
				return std::string(str);
			}
			catch (...) {
			}
			return std::string();
		}

	public:
		// 运算符
		bool operator == (const DateTime& rhs) { return Equals(rhs); }
		bool operator > (const DateTime& rhs) { return 1 == Compare(rhs); }
		bool operator < (const DateTime& rhs) { return -1 == Compare(rhs); }
		bool operator >= (const DateTime& rhs) { return seconds_ >= rhs.seconds_ ? true : false; }
		bool operator <= (const DateTime& rhs) { return seconds_ <= rhs.seconds_ ? true : false; }
		bool operator != (const DateTime& rhs) { return 0 != Compare(rhs); }
		DateTime operator - (const DateTime& rhs)
		{
			auto seconds = seconds_ - rhs.seconds_;
			return DateTime(seconds);
		}

		// 比较时间大小
		int Compare(const DateTime& rhs) { return Compare(*this, rhs); }
		// 判断是否相等
		bool Equals(const DateTime& rhs) { return Equals(*this, rhs); }
	public:
		// 对两个DateTime 的实例进行比较，并返回一个指示第一个实例是早于、等于还是晚于第二个实例的整数 
		// < 0 此实例小于rhs
		// =0 此实例等于rhs 
		// > 0 此实例大于rhs 
		static int Compare(const DateTime& lhs, const DateTime& rhs)
		{
			if (lhs.seconds_ == rhs.seconds_) {
				return 0;
			}
			else if (lhs.seconds_ < rhs.seconds_) {
				return -1;
			}
			return 1;
		}
		// 对两个DateTime 的实例进行比较，相等返回true，否则false
		static bool Equals(const DateTime& lhs, const DateTime& rhs) { return lhs.seconds_ == rhs.seconds_; }

		/// 解析时间
		static DateTime Parse(time_t time) { return DateTime(time); }
		static DateTime Parse(const std::string& strTime) { return DateTime(strTime); }
		static DateTime Parse(const std::string& strTime, const std::string& strFormat) { return DateTime(strTime, strFormat); }

		// 返回当前日期
		static DateTime Now(void) { return DateTime(time(0)); }

		// 是否相同日期
		static bool IsSameDay(time_t lhs, time_t rhs) { return IsSameDay(DateTime(lhs), DateTime(rhs)); }
		static bool IsSameDay(const DateTime& lhs, const DateTime& rhs) { return lhs.GetYear() == rhs.GetYear() && lhs.GetMonth() == rhs.GetMonth() && lhs.GetDay() == rhs.GetDay(); }
		static bool IsSameMonth(time_t lhs, time_t rhs) { return IsSameMonth(DateTime(lhs), DateTime(rhs)); }
		static bool IsSameMonth(const DateTime& lhs, const DateTime& rhs){ return lhs.GetYear() == rhs.GetYear() && lhs.GetMonth() == rhs.GetMonth(); }
		static bool IsSameYear(time_t lhs, time_t rhs) { return IsSameYear(DateTime(lhs), DateTime(rhs)); }
		static bool IsSameYear(const DateTime& lhs, const DateTime& rhs){ return lhs.GetYear() == rhs.GetYear(); }
		static UInt32  GetTickCount()
		{
#ifdef _MSWINDOWS_
			return ::GetTickCount();
#else
			struct tms tmp = { 0 };
			UInt64 tcks = (UInt64)times(&tmp);
			UInt64 Hz = (UInt64)sysconf(_SC_CLK_TCK);
			if (Hz < 1)
			{
				return 0;
			}
			return  (UInt32)((UInt64)((tcks * 1000) / Hz) & 0xFFFFFFFF);
#endif
		}

		static UInt64 GetPerformanceCounter()
		{
#ifdef _MSWINDOWS_
			LARGE_INTEGER frequency = { 0 };
			LARGE_INTEGER counter = { 0 };

			QueryPerformanceFrequency(&frequency);
			QueryPerformanceCounter(&counter);

			if (frequency.QuadPart == 0)
				return 0;

			return (UInt64)(counter.QuadPart * 1000 / frequency.QuadPart);
#else

			UInt64 tcks = (UInt64)times(NULL);
			UInt64 Hz = (UInt64)sysconf(_SC_CLK_TCK);
			if (Hz < 1)
			{
				return 0;
			}
			return  (UInt64)(tcks * 1000 / Hz);
#endif
		}

		static UInt32  CountElapsed(UInt32 nowTick, UInt32 oldTick)
		{
			if (nowTick >= oldTick)
			{
				return nowTick - oldTick;
			}
			return (MAX_UINT32 - oldTick + nowTick);
		}

		//返回系统 UTC 时间，自 1970-01-01 00:00:00 的秒数 等于 Timestamp
		static UInt64 GetEpochMilliseconds(void)
		{
			struct _TIMEVAL tv;
			if (gettimeofday(&tv, NULL))
			{
				return MAX_UINT64;
			}
			return (UInt64)(tv.tv_sec) * 1000 + tv.tv_usec / 1000;
		}

	private:
		void format(const std::string& strTime, const std::string& strFormat)
		{
			int year = 0, month = 0, day = 0, hour = 0, minute = 0, second = 0;
			try {
				//sscanf_s(strTime.c_str(), strFormat.c_str(), &year, &month, &day, &hour, &minute, &second);/// _stscanf_s	
				sscanf(strTime.c_str(), strFormat.c_str(), &year, &month, &day, &hour, &minute, &second);/// _stscanf_s	
			}
			catch (...) {
				return;
			}

			month--;

			if (year < 1900) year = 1970;
			if (month < 0) month = 1;
			if (day < 0) day = 1;
			if (hour < 0) hour = 0;
			if (minute < 0) minute = 0;
			if (second < 0) second = 0;
			format(year, month, day, hour, minute, second);
		}
		void format(int year, int month, int day, int hour, int minute, int second)
		{
			tm t;
			t.tm_year = year - 1900;
			t.tm_mon = month/* - 1*/;
			t.tm_mday = day;
			t.tm_hour = hour;
			t.tm_min = minute;
			t.tm_sec = second;
			time_t sec = mktime(&t);
			seconds_ = sec < 0 ? 0 : sec;
			localtime_all(&date_, &seconds_);
		}

	private:

		void localtime_all(tm* date, time_t* seconds)
		{
#ifdef _MSWINDOWS_
			localtime_s(date, seconds);
#else
			localtime_r(seconds, date);
#endif 
		}
		struct _TIMEVAL
		{
			Int64 tv_sec;
				long  tv_usec;
		};
		
			
			// Based on: http://www.google.com/codesearch/p?hl=en#dR3YEbitojA/os_win32.c&q=GetSystemTimeAsFileTime%20license:bsd
			// See COPYING for copyright information.
			static int gettimeofday(struct _TIMEVAL *tv, void* tz) 
			{
#define EPOCHFILETIME (116444736000000000ULL)
				FILETIME ft;
					LARGE_INTEGER li;
					UInt64 tt;
					
					GetSystemTimeAsFileTime(&ft);
					li.LowPart = ft.dwLowDateTime;
					li.HighPart = ft.dwHighDateTime;
					tt = (li.QuadPart - EPOCHFILETIME) / 10;
					tv->tv_sec = tt / 1000000;
					tv->tv_usec = tt % 1000000;
					return 0;
			}
	};// end class

}// end mycpp

#endif //! __MYCPP_DATATIME_H__
