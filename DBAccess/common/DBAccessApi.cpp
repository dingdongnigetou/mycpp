#include "DBAccessApi.h"
#include "ConnectionPool.h"

IConnectionPool* CreateDBConnectionPool( EnumDriverType eType )
{
	if (eType == ADO)
	{
		::CoInitialize(NULL);
	}

	IConnectionPool *pConnPool = NULL;
	pConnPool = new CConnectionPool(eType);
	
	MYASSERT(pConnPool!=NULL);

	return pConnPool;
}

void DestroyDBConnectionPool( IConnectionPool** pConnPool )
{
	MYASSERT(*pConnPool!=NULL);

	if ((*pConnPool)->GetDriverType() == ADO)
	{
		::CoUninitialize();
	}

	if ( *pConnPool )
	{
		delete *pConnPool;
		*pConnPool = NULL;
	}
}
