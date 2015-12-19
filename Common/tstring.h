
#ifndef MYCPP_TSTRING_H
#define MYCPP_TSTRING_H

#include <string>

#if defined(UNICODE) || defined(_UNICODE)
typedef std::wstring  tstring;
#else
typedef std::string   tstring;
#endif

#endif // MYCPP_TSTRING_H
