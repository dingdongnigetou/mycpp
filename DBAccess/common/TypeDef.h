
#ifndef TYPEDEF_DEF_H
#define TYPEDEF_DEF_H

#include "mytypes.h"
#include "mydefs.h"

#include <vector>
#include <map>

#if defined(UNICODE) || defined(_UNICODE)
typedef std::wstring							tstring;
#else
typedef std::string                             tstring;
#endif

#define MYDB_PRINT MYPRINTF

#define DB_POINTER_CHECK_RET(p,ret) do  \
{ \
	MYASSERT(p!=NULL); \
	if ( !p ) \
	return ret; \
} while (0)

#endif // TYPEDEF_DEF_H

