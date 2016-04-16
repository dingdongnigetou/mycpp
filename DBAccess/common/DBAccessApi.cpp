#include "DBAccessApi.h"
#include "ConnectionPool.h"

IConnectionPool* CreateDBConnectionPool( EnumDriverType eType )
{
	if (eType == ADO)
	{
		::CoInitialize(nullptr);
	}

	IConnectionPool *pConnPool = nullptr;
	pConnPool = new CConnectionPool(eType);
	
	MYASSERT(pConnPool!=nullptr);

	return pConnPool;
}

void DestroyDBConnectionPool( IConnectionPool** pConnPool )
{
	MYASSERT(*pConnPool!=nullptr);

	if ((*pConnPool)->GetDriverType() == ADO)
	{
		::CoUninitialize();
	}

	if ( *pConnPool )
	{
		delete *pConnPool;
		*pConnPool = nullptr;
	}
}
