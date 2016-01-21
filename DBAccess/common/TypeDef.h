
#ifndef __TYPEDEF_DEF_H__
#define __TYPEDEF_DEF_H__

#include <string>

#include "mytypes.h"
#include "mydefs.h"

#ifdef WIN_LOWER  /// Server2003或者Xp等低版本需要使用这个才能进行ADO连接
#import "c:\program files\common files\system\ado\msado60_Backcompat_i386.tlb" \
	no_namespace rename("EOF", "adoEOF")
#else	
#import "c:\program files\common files\system\ado\msado15.dll" \
	no_namespace rename("EOF", "adoEOF")
#endif

#define MYDB_PRINT MYPRINTF

#define DB_POINTER_CHECK_RET(p,ret) do  \
{ \
	MYASSERT(p!=NULL); \
	if ( !p ) \
	return ret; \
} while (0)

#endif // __TYPEDEF_DEF_H__

