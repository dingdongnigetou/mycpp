#ifndef __MYCPP_DATATIME_H__
#define __MYCPP_DATATIME_H__

#include <time.h>
#include "string.hpp"

namespace mycpp
{
	class DateTime{
		tm date_;// ����
		time_t seconds_;// ��1970�������
	public:
		explicit DateTime::DateTime() : DateTime(time(0)) {}
		DateTime::DateTime(time_t seconds) : seconds_(seconds)
		{
			localtime_s(&this->date_, &this->seconds_);
		}
		explicit DateTime::DateTime(int year, int month, int day) : DateTime(year, month, day, 0, 0, 0) {}
		DateTime::DateTime(int year, int month, int day, int hour, int minute, int second)
		{
			Format(year, month, day, hour, minute, second);
		}
		explicit DateTime::DateTime(const tstring& strTime) : DateTime(strTime, _T("%d-%d-%d %d:%d:%d")) {}
		//�����ַ�����ʽ ��-��-�� ʱ:��:�� ��:2008-02-03 9:30:20 
		explicit DateTime::DateTime(const tstring& strTime, const tstring& strFormat)
		{
			Format(strTime, strFormat);
		}

		// ��������
		DateTime::DateTime(const DateTime& rhs)
		{
			*this = rhs;
		}
		const DateTime& DateTime::operator=(const DateTime& rhs)
		{
			if (this == &rhs) {
				return *this;
			}
			this->seconds_ = rhs.seconds_;
			localtime_s(&this->date_, &this->seconds_);
			return *this;
		}
	public:
		// ��ȡʱ���
		time_t GetTimestamp() const { return seconds_; }

		// ��ȡʱ��
		int GetYear() const { return date_.tm_year + 1900; }
		int GetMonth() const { return date_.tm_mon + 1; }
		int GetDay() const { return date_.tm_mday; }
		int GetHour() const { return date_.tm_hour; }
		int GetMinute() const { return date_.tm_min; }
		int GetSecond() const { return date_.tm_sec; }

		// ���ʱ��
		void DateTime::AddYears(int years)
		{
			auto year = date_.tm_year + years;
			if (!(year >= 1970 && year < 3000)) {
				return;
			}
			date_.tm_year += years;
			seconds_ = mktime(&date_);
		}
		void DateTime::AddMonths(int months)
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
		DateTime DateTime::AddSeconds(int seconds)
		{
			seconds_ += seconds;
			localtime_s(&date_, &seconds_);

			return *this;
		}

		// ����ʱ��
		DateTime DateTime::SubYears(int years)
		{
			date_.tm_year -= years;
			seconds_ = mktime(&date_);
			return *this;
		}
		DateTime DateTime::SubMonths(int months)
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
			localtime_s(&date_, &seconds_);

			return *this;
		}

		// �ж�������
		bool IsAM() const { return date_.tm_hour < 12; }
		bool IsPM() const { return date_.tm_hour >= 12; }

		//  ��ȡ��ʵ������ʾ��һ������ĵڼ���,��1��1����,0-365
		int DayOfYear() const { return date_.tm_yday; }
		// ��ȡ��ʵ������ʾ�������Ǽ���
		int DayOfMonth() const { return date_.tm_mday; }
		// ��ȡ��ʵ������ʾ�����������ڼ�
		int DayOfWeek() const { return date_.tm_wday; }

		// ����ǰ DateTime �����ֵת��Ϊ���Ч�Ķ������ַ�����ʾ��ʽ
		// "%Y-%m-%d %H:%M:%S" %Y=�� %m=�� %d=�� %H=ʱ %M=�� %S=��
		// %Y-%m-%d
		tstring ToYmdString() const { return ToString(_T("%Y-%m-%d")); }
		std::string ToYmdStringA() const { return ToStringA("%Y-%m-%d"); }
		std::wstring ToYmdStringW() const { return ToStringW(L"%Y-%m-%d"); }
		// %H:%M:%S
		tstring ToHmsString() const { return ToString(_T("%H:%M:%S")); }
		std::string ToHmsStringA() const { return ToStringA("%H:%M:%S"); }
		std::wstring ToHmsStringW() const { return ToStringW(L"%H:%M:%S"); }

		tstring ToYmdString2() const { return ToString(_T("%Y%m%d")); }
		std::string ToYmdStringA2() const { return ToStringA("%Y%m%d"); }
		std::wstring ToYmdStringW2() const { return ToStringW(L"%Y%m%d"); }

		tstring ToHmsString2() const { return ToString(_T("%H%M%S")); }
		std::string ToHmsStringA2() const { return ToStringA("%H%M%S"); }
		std::wstring ToHmsStringW2() const { return ToStringW(L"%H%M%S"); }

		tstring ToString() const { return ToString(_T("%Y-%m-%d %H:%M:%S")); }
		std::string ToStringA() const { return ToStringA("%Y-%m-%d %H:%M:%S"); }
		std::wstring ToStringW() const { return ToStringW(L"%Y-%m-%d %H:%M:%S"); }

		tstring ToString2() const { return ToString(_T("%Y%m%d%H%M%S")); }
		std::string ToStringA2() const { return ToStringA("%Y%m%d%H%M%S"); }
		std::wstring ToStringW2() const { return ToStringW(L"%Y%m%d%H%M%S"); }

		
		tstring DateTime::ToString(const tstring& strFormat) const
		{
#if defined(UNICODE) || defined(_UNICODE)
			return ToStringW(strFormat);
#else
			return ToStringA(strFormat);
#endif
		}
		std::string DateTime::ToStringA(const std::string& strFormat) const
		{
			if (seconds_ <= 0) {
				return std::string();
			}
			try {
				char str[40] = { 0 };
				strftime(str, 40, strFormat.c_str(), &date_);
				return std::move(std::string(str));
			}
			catch (...) {
			}
			return std::string();
		}
		std::wstring DateTime::ToStringW(const std::wstring& strFormat) const
		{
			if (seconds_ <= 0) {
				return std::wstring();
			}
			try {
				wchar_t str[40] = { 0 };
				wcsftime(str, sizeof(str), strFormat.c_str(), &date_);
				return std::move(std::wstring(str));
			}
			catch (...) {
			}
			return std::wstring();
		}

	public:
		// �����
		bool operator == (const DateTime& rhs) { return Equals(rhs); }
		bool operator > (const DateTime& rhs) { return 1 == Compare(rhs); }
		bool operator < (const DateTime& rhs) { return -1 == Compare(rhs); }
		bool operator >= (const DateTime& rhs) { return seconds_ >= rhs.seconds_ ? true : false; }
		bool operator <= (const DateTime& rhs) { return seconds_ <= rhs.seconds_ ? true : false; }
		bool operator != (const DateTime& rhs) { return 0 != Compare(rhs); }
		DateTime&& DateTime::operator - (const DateTime& rhs)
		{
			auto seconds = seconds_ - rhs.seconds_;
			return std::move(DateTime(seconds));
		}

		// �Ƚ�ʱ���С
		int Compare(const DateTime& rhs) { return Compare(*this, rhs); }
		// �ж��Ƿ����
		bool Equals(const DateTime& rhs) { return Equals(*this, rhs); }
	public:
		// ������DateTime ��ʵ�����бȽϣ�������һ��ָʾ��һ��ʵ�������ڡ����ڻ������ڵڶ���ʵ�������� 
		// < 0 ��ʵ��С��rhs
		// =0 ��ʵ������rhs 
		// > 0 ��ʵ������rhs 
		static int DateTime::Compare(const DateTime& lhs, const DateTime& rhs)
		{
			if (lhs.seconds_ == rhs.seconds_) {
				return 0;
			}
			else if (lhs.seconds_ < rhs.seconds_) {
				return -1;
			}
			return 1;
		}
		// ������DateTime ��ʵ�����бȽϣ���ȷ���true������false
		static bool Equals(const DateTime& lhs, const DateTime& rhs) { return lhs.seconds_ == rhs.seconds_; }

		/// ����ʱ��
		static DateTime Parse(time_t time) { return std::move(DateTime(time)); }
		static DateTime Parse(const tstring& strTime) { return std::move(DateTime(strTime)); }
		static DateTime Parse(const tstring& strTime, const tstring& strFormat) { return std::move(DateTime(strTime, strFormat)); }
		static DateTime DateTime::ParseA(const std::string& strTime)
		{
			DateTime lhs;
			lhs.FormatA(strTime, "%d-%d-%d %d:%d:%d");
			return std::move(lhs);
		}
		static DateTime DateTime::ParseW(const std::wstring& strTime)
		{
			DateTime lhs;
			lhs.FormatW(strTime, L"%d-%d-%d %d:%d:%d");
			return std::move(lhs);
		}
		static DateTime DateTime::ParseA(const std::string& strTime, const std::string& strFormat)
		{
			DateTime lhs;
			lhs.FormatA(strTime, strFormat);
			return std::move(lhs);
		}
		static DateTime DateTime::ParseW(const std::wstring& strTime, const std::wstring& strFormat)
		{
			DateTime lhs;
			lhs.FormatW(strTime, strFormat);
			return std::move(lhs);
		}

		// ���ص�ǰ����
		static DateTime Now() { return std::move(DateTime(time(0))); }

		// �Ƿ���ͬ����
		static bool IsSameDay(time_t lhs, time_t rhs) { return IsSameDay(DateTime(lhs), DateTime(rhs)); }
		static bool IsSameDay(const DateTime& lhs, const DateTime& rhs) { return lhs.GetYear() == rhs.GetYear() && lhs.GetMonth() == rhs.GetMonth() && lhs.GetDay() == rhs.GetDay(); }
		static bool IsSameMonth(time_t lhs, time_t rhs) { return IsSameMonth(DateTime(lhs), DateTime(rhs)); }
		static bool IsSameMonth(const DateTime& lhs, const DateTime& rhs){ return lhs.GetYear() == rhs.GetYear() && lhs.GetMonth() == rhs.GetMonth(); }
		static bool IsSameYear(time_t lhs, time_t rhs) { return IsSameYear(DateTime(lhs), DateTime(rhs)); }
		static bool IsSameYear(const DateTime& lhs, const DateTime& rhs){ return lhs.GetYear() == rhs.GetYear(); }
	private:
		void DateTime::Format(const tstring& strTime, const tstring& strFormat)
		{
#if defined(UNICODE) || defined(_UNICODE)
			return FormatW(strTime, strFormat);
#else
			return FormatA(strTime, strFormat);
#endif
		}
		void DateTime::FormatA(const std::string& strTime, const std::string& strFormat)
		{
			int year = 0, month = 0, day = 0, hour = 0, minute = 0, second = 0;
			try {
				sscanf_s(strTime.c_str(), strFormat.c_str(), &year, &month, &day, &hour, &minute, &second);/// _stscanf_s	
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
			Format(year, month, day, hour, minute, second);
		}
		void DateTime::FormatW(const std::wstring& strTime, const std::wstring& strFormat)
		{
			int year = 0, month = 0, day = 0, hour = 0, minute = 0, second = 0;
			try {
				swscanf_s(strTime.c_str(), strFormat.c_str(), &year, &month, &day, &hour, &minute, &second);/// sscanf_s
			}
			catch (...) {
				return;
			}
			if (year < 1900) year = 1970;
			if (month < 0) month = 1;
			if (day < 0) day = 1;
			if (hour < 0) hour = 0;
			if (minute < 0) minute = 0;
			if (second < 0) second = 0;
			Format(year, month, day, hour, minute, second);
		}
		void DateTime::Format(int year, int month, int day, int hour, int minute, int second)
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
			localtime_s(&date_, &seconds_);
		}
	};// end class

}// end mycpp

#endif //! __MYCPP_DATATIME_H__
