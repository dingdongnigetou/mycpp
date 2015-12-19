#ifndef MYCPP_COMMON_COMMON_H
#define MYCPP_COMMON_COMMON_H 

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
