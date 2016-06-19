#include "DBAccessApi.h"
#include "ConnectionPool.h"

IConnectionPool* CreateDBConnectionPool( EnumDriverType eType )
{
#ifdef _MSWINDOWS
	if (eType == ADO)
	{
		::CoInitialize(nullptr);
	}
#endif

	IConnectionPool *pConnPool = nullptr;
	pConnPool = new CConnectionPool(eType);
	
	MYASSERT(pConnPool!=nullptr);

	return pConnPool;
}

void DestroyDBConnectionPool( IConnectionPool** pConnPool )
{
	MYASSERT(*pConnPool!=nullptr);

#ifdef _MSWINDOWS
	if ((*pConnPool)->GetDriverType() == ADO)
	{
		::CoUninitialize();
	}
#endif
	if ( *pConnPool )
	{
		delete *pConnPool;
		*pConnPool = nullptr;
	}
}
