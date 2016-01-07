
#ifndef __TYPEDEF_DEF_H__
#define __TYPEDEF_DEF_H__

#include <string>

#include "mytypes.h"
#include "mydefs.h"

#import "c:\program files\common files\system\ado\msado15.dll" \
	no_namespace rename("EOF", "adoEOF")

#define MYDB_PRINT MYPRINTF

#define DB_POINTER_CHECK_RET(p,ret) do  \
{ \
	MYASSERT(p!=NULL); \
	if ( !p ) \
	return ret; \
} while (0)

#endif // __TYPEDEF_DEF_H__

