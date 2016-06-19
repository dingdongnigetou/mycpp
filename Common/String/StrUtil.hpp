#ifndef MYCPP_STRUTIL_H
#define MYCPP_STRUTIL_H

#include "../mydefs.h"

#include <sstream>
#include <algorithm>
#include <vector>
#include <string>
#include <stdarg.h>

#include "../Singleton.hpp"

namespace mycpp
{
	class StrUtil : public Singleton<StrUtil>
	{
		friend class Singleton<StrUtil>;

	public:
		// �ַ�����ʽ��
		template <typename... Args>
		std::string Format(const char*s, Args... args)
		{
			std::stringstream os;
			doFormat(os, s, args...);
			return os.str();
		}

		// ɾ���ַ�����߿հ�
		std::string TrimLeft(const std::string& str)
		{
			std::string t = str;
			t.erase(0, t.find_first_not_of(" \t\n\r"));
			return t;
		}

		// ɾ���ַ����ұ߿հ�
		std::string TrimRight(const std::string& str)
		{
			std::string t = str;
			t.erase(t.find_last_not_of(" \t\n\r") + 1);
			return t;
		}

		// ɾ���ַ����������߿հ�
		std::string Trim(const std::string& str)
		{
			std::string t = str;
			t.erase(0, t.find_first_not_of(" \t\n\r"));
			t.erase(t.find_last_not_of(" \t\n\r") + 1);
			return t;
		}

		// ת��ΪСд�ַ���
		std::string ToLower(const std::string& str)
		{
			std::string t = str;
			std::transform(t.begin(), t.end(), t.begin(), ::tolower);
			return t;
		}

		// ת��Ϊ��д�ַ���
		std::string ToUpper(const std::string& str)
		{
			std::string t = str;
			std::transform(t.begin(), t.end(), t.begin(), ::toupper);
			return t;
		}

		// �ֽ��ַ���
		void Split(std::vector<std::string> &csResult, const std::string& str, const std::string& delimiters)
		{
			int iOffset = 0;
			std::string strToken;
			for (;;)
			{
				std::string::size_type i = str.find_first_not_of(delimiters, iOffset);
				if (i == std::string::npos) {
					iOffset = str.length();
					return;
				}

				// ���ұ�ʶ����λ��
				std::string::size_type j = str.find_first_of(delimiters, i);
				if (j == std::string::npos) {
					strToken = str.substr(i);
					iOffset = str.length();
					csResult.push_back(strToken);
				}
				else
				{
					strToken = str.substr(i, j - i);
					iOffset = j;
					csResult.push_back(strToken);
				}
			}
		}

		//���ִ�Сд�ıȽ�
		bool EqualsIgnoreCase(const std::string& strSrc, const std::string& strDest)
		{
			return ToLower(strSrc) == ToLower(strDest);
		}

		// ��ʮ�����ַ���ת��Ϊ��ֵ
		template<class T>
		T ToNumber(const std::string& str)
		{
			T value;
			std::istringstream iss(str.c_str());
			iss >> value;
			return value;
		}

		// ��ʮ�������ַ���ת��Ϊ��ֵ
		template<class T>
		T ToHexNumber(const std::string& str)
		{
			T value;
			std::istringstream iss(str.c_str());
			iss >> std::hex >> value;
			return value;
		}

		template<class T>
		std::string ToString(const T value)
		{
			std::ostringstream oss;
			oss << value;
			return oss.str();
		}

		template<class T>
		std::string ToHexString(const T value)
		{
			std::ostringstream oss;
			oss << "0x" << std::hex << value;
			return oss.str();
		}

		template<class T>
		int CheckValueRange(T &vValue, const T vMin, const T vMax)
		{
			if (vValue<vMin)
			{
				vValue = vMin;
				return -1;
			}
			else if (vValue > vMax)
			{
				vValue = vMax;
				return 1;
			}
			return 0;
		}

	private:
		void doFormat(std::ostream& os, const char* s)
		{
			while (*s)
			{
				if (*s == '%' && *(++s) != '%')
					throw std::logic_error("Format: ����̫��");

				os << *s++;
			}
		}

		template <typename T, typename... Args>
		void doFormat(std::ostream& os, const char *s, T value, Args... args)
		{
			while (*s)
			{
				if (*s == '%' && *(++s) != '%')
				{
					os << value;
					doFormat(os, *s ? ++s : s, args...);
					return;
				}
				os << *s++;
			}
			throw std::logic_error("Format: ��������");
		}
	};
}

#define STRUTIL() mycpp::StrUtil::Instance()

#endif //end MYCPP_STRUTIL_H
