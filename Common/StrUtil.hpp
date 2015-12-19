#ifndef MYCPP_STRUTIL_H
#define MYCPP_STRUTIL_H

#include "mydefs.h"
#include "tstring.h"

#include <sstream>
#include <algorithm>
#include <vector>
#include <stdarg.h>

namespace mycpp
{
	namespace StrUtil
	{
		// ɾ���ַ�����߿հ�
		INLINE static tstring TrimLeft(const tstring& str)
		{
			tstring t = str;
			t.erase(0, t.find_first_not_of(" \t\n\r"));
			return t;
		}

		// ɾ���ַ����ұ߿հ�
		INLINE static tstring TrimRight(const tstring& str)
		{
			tstring t = str;
			t.erase(t.find_last_not_of(" \t\n\r") + 1);
			return t;
		}

		// ɾ���ַ����������߿հ�
		INLINE static tstring Trim(const tstring& str)
		{
			tstring t = str;
			t.erase(0, t.find_first_not_of(" \t\n\r"));
			t.erase(t.find_last_not_of(" \t\n\r") + 1);
			return t;
		}

		// ת��ΪСд�ַ���
		INLINE static tstring ToLower(const tstring& str)
		{
			tstring t = str;
			std::transform(t.begin(), t.end(), t.begin(), ::tolower);
			return t;
		}

		// ת��Ϊ��д�ַ���
		INLINE static tstring ToUpper(const tstring& str)
		{
			tstring t = str;
			std::transform(t.begin(), t.end(), t.begin(), ::toupper);
			return t;
		}

		// �ֽ��ַ���
		INLINE static void Split(std::vector<tstring> &csResult, const tstring& str, const tstring& delimiters)
		{
			int iOffset = 0;
			tstring strToken;
			for (;;)
			{
				tstring::size_type i = str.find_first_not_of(delimiters, iOffset);
				if (i == tstring::npos) {
					iOffset = str.length();
					return;
				}

				// ���ұ�ʶ����λ��
				tstring::size_type j = str.find_first_of(delimiters, i);
				if (j == tstring::npos) {
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
		INLINE static bool EqualsIgnoreCase(const tstring& strSrc, const tstring& strDest)
		{
			return ToLower(strSrc) == ToLower(strDest);
		}


		// �ַ�������ת��ģ�庯��
		// �ַ�������ת��ģ�庯��
		template<class T> T ToNumber(const tstring& str);
		template<class T> T ToHexNumber(const tstring& str);
		template<class T> tstring ToString(const T value);
		template<class T> tstring ToHexString(const T value);

		// ��ʮ�����ַ���ת��Ϊ��ֵ
		template<class T>
		T ToNumber(const tstring& str)
		{
			T value;
			std::istringstream iss(str.c_str());
			iss >> value;
			return value;
		}

		// ��ʮ�������ַ���ת��Ϊ��ֵ
		template<class T>
		T ToHexNumber(const tstring& str)
		{
			T value;
			std::istringstream iss(str.c_str());
			iss >> std::hex >> value;
			return value;
		}

		template<class T>
		tstring ToString(const T value)
		{
			std::ostringstream oss;
			oss << value;
			return oss.str();
		}

		template<class T>
		tstring ToHexString(const T value)
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

#define MAX_STRING_LENGHT  (100<<20)

		INLINE static  bool VFormat(tstring &strOString, const char *czFormat, va_list ap)
		{
			char sBuffer[256];
			va_list apStart;
			char *pBuffer;
			int n, size = 256;


			strOString.clear();
#ifdef _MSWINDOWS_
			apStart = ap;
#else
			va_copy(apStart, ap);
#endif
			pBuffer = sBuffer;
			while (pBuffer) {
#ifdef _MSWINDOWS_
				ap = apStart;
#else
				va_copy(ap, apStart);
#endif
				n = MYVSNPRINTF(pBuffer, size, czFormat, ap);
				if (n > -1 && n < size)
				{
					//�ɹ���ʽ��
					//pBuffer[n] = '\0';
					strOString = pBuffer;
					if (pBuffer != sBuffer)
					{
						::free(pBuffer);
					}
					return true;
				}
				if (pBuffer != sBuffer)
				{
					::free(pBuffer);
				}
				pBuffer = NULL;
				size *= 2;
				if (size > MAX_STRING_LENGHT)
				{
					MYASSERT(0);
					return false;
				}

				pBuffer = (char*) ::malloc(size);
				MYABORT(pBuffer != NULL);
			}
			if (pBuffer && pBuffer != sBuffer)
			{
				::free(pBuffer);
			}
			return false;
		}

		INLINE static  bool Format(tstring &strOString, const char* czFormat, ...)
		{
			bool bRet;
			va_list ap;
			va_start(ap, czFormat);
			bRet = StrUtil::VFormat(strOString, czFormat, ap);
			va_end(ap);
			return  bRet;

		}

		INLINE static  bool AppendWithFormat(tstring &strIOString, const char* czFormat, ...)
		{
			bool bRet;
			tstring strTemp;
			va_list ap;
			va_start(ap, czFormat);
			bRet = StrUtil::VFormat(strTemp, czFormat, ap);
			va_end(ap);

			if (bRet)
			{
				strIOString += strTemp;
			}
			return bRet;
		}
	}
} // mycpp

#endif //end MYCPP_STRUTIL_H
