#include "DBAccessApi.h"
#include "ConnectionPool.h"

IConnectionPool* CreateDBConnectionPool( EnumDriverType eType )
{
	IConnectionPool *pConnPool = NULL;
	pConnPool = new CConnectionPool(eType);
	
	MYASSERT(pConnPool!=NULL);

	return pConnPool;
}

void DestroyDBConnectionPool( IConnectionPool** pConnPool )
{
	MYASSERT(*pConnPool!=NULL);
	
	if ( *pConnPool )
	{
		delete *pConnPool;
		*pConnPool = NULL;
	}
}
