#ifndef __MYCPP_STRING_HPP__
#define __MYCPP_STRING_HPP__

#include <string>

#if defined(UNICODE) || defined(_UNICODE)
typedef std::wstring tstring;
#else
typedef std::string	tstring;
#endif


#endif// end __MYCPP_STRING_HPP__