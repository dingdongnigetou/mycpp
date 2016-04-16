#ifndef __MYCPP_COMMON_COMMON_H__
#define __MYCPP_COMMON_COMMON_H__

#include "mytypes.h"

namespace mycpp
{
	namespace common
	{
		// ÐÝÃßº¯Êý£¬µ¥Î»: ºÁÃë
		static INLINE MySleep(UInt32 milliSeconds)
		{
#if defined(_MSWINDOWS_)
			Sleep(millisSeconds);
#else
			usleep(millisSeconds*1000);
#endif
		}
	}
}


#endif // MYCPP_COMMON_COMMON_H 
