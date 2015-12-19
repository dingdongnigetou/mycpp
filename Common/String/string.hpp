#ifndef __STRING_HPP__
#define __STRING_HPP__

#include <tchar.h>
#include <string>

#if defined(UNICODE) || defined(_UNICODE)
typedef std::wstring tstring;
#else
typedef std::string	tstring;
#endif


#endif// end __STRING_HPP__