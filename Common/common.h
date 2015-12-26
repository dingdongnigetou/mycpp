#ifndef __MYCPP_COMMON_COMMON_H__
#define __MYCPP_COMMON_COMMON_H__

#include "mytypes.h"
#include "mydefs.h"

#define TEST_ASSERT(condition)  MYASSERT(condition)

#define TEST_ASSERT_RET(condition,ret) do  \
{ \
	MYASSERT(condition); \
	if ( !condition ) \
	return ret; \
} while (0)

#endif // MYCPP_COMMON_COMMON_H 
