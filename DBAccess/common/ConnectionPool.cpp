#include "ConnectionPool.h"

#include "OciConnection.h"
#include "MysqlConnection.h"

#define SET_ERROR_CODE(err) (m_eError = err)
#define GET_ERROR_CODE(perr) if ( perr ) {*perr = m_eError;}

using namespace mycpp;

CConnectionPool::CConnectionPool( EnumDriverType eType )
:m_eType(eType)
,m_strHost("")
,m_strDataBase("")
,m_strUserName("")
,m_strPassword("")
,m_iPort(0)
,m_iMinConns(0)
,m_iMaxConns(0)
,m_iUsedConns(0)
,m_eError(RETCODE_SUCCESS)
{

}

CConnectionPool::~CConnectionPool( void )
{
	ClearList();
}

void CConnectionPool::SetParams( const char* szHost,
								  const char* szDataBase,
								  const char* szUserName,
								  const char* szPassword,
								  unsigned short iPort,
								  unsigned int iMinConns,
								  unsigned int iMaxConns )
{
	//MYASSERT(szHost!=NULL);  //本地数据库可以为NULL
	MYASSERT(szDataBase!=NULL);
	MYASSERT(szUserName!=NULL);
	//MYASSERT(szPassword!=NULL); // 密码可以为NULL
	MYASSERT(iMinConns>0&&iMinConns<=iMaxConns);
	MYASSERT(iMinConns<=iMaxConns&&iMaxConns<=50);

	if ( !szUserName
		|| (iMinConns<0||iMinConns>iMaxConns)
		|| (iMaxConns<iMinConns||iMaxConns>50) )
	{
		MYWARNV(!m_strDataBase.empty(),"db name is null!");
		MYWARNV(!m_strUserName.empty(), "user name is null!");
		MYWARNV(iMinConns>0&&iMinConns<=iMaxConns, "min connections illegal");
		MYWARNV(iMinConns<=iMaxConns&&iMaxConns<=50, "max connections illegal");
		SET_ERROR_CODE(RETCODE_PARAMS_ERROR);
		return ;
	}
	
	if ( szHost ) m_strHost  = std::string(szHost);
	if ( szPassword ) m_strPassword = std::string(szPassword);

	m_strDataBase = std::string(szDataBase);
	m_strUserName = std::string(szUserName);

	m_iPort     = iPort;
	m_iMinConns = iMinConns;
	m_iMaxConns = iMaxConns;
}

IConnection* CConnectionPool::GetConnection( EnumDBApiRet* eError )
{
	IConnection *pConn = NULL;
	BOOL bRet = FALSE;

	MYASSERT(bRet=IsNoError());
	if ( !bRet )
	{
		GET_ERROR_CODE(eError);
		return pConn;
	}

	MYASSERT(bRet=InitList());
	if ( !bRet )
	{
		GET_ERROR_CODE(eError);
		return pConn;
	}

	pConn = GetIdle();
	if ( !pConn )
	{
		if ( !IsOverMaxLink() )
		{
			pConn = CreateConnection();
		}
		else
		{
			SET_ERROR_CODE(RETCODE_OVER_MAXLINK);
		}

		GET_ERROR_CODE(eError);
		
		return pConn;
	}

	GET_ERROR_CODE(eError);

	return pConn;
}

void CConnectionPool::ReleaseConnection( IConnection** pcsConn )
{
	MYASSERT(*pcsConn!=NULL);

	if ( !IsListItem(*pcsConn) )
		DestroyConnection(*pcsConn);
	else
		SetIdle(*pcsConn);

	*pcsConn = NULL;
}

BOOL CConnectionPool::IsNoError( void )
{
	return (m_eError==RETCODE_SUCCESS)?TRUE:FALSE;
}

IConnection* CConnectionPool::CreateConnection( void )
{
	IConnection* pcsConn = NULL;

	switch ( m_eType )
	{
	case ODBC:
		MYWARNV(0, "Not support ODBC driver type!");
		break;
	case OCI:
		{
			COciConnection *p = new COciConnection(m_strHost,
				m_strDataBase,
				m_strUserName,
				m_strPassword,
				m_iPort);
			if ( !p->ConnectDB() )
			{
				SET_ERROR_CODE(p->GetErrorCode());
				delete p;
				p = NULL;
			}

			pcsConn = p;
			
			m_iUsedConns ++;
		}
		break;
	case MYSQL_API:
		{
			CMysqlConnection *p = new CMysqlConnection(m_strHost,
				m_strDataBase,
				m_strUserName,
				m_strPassword,
				m_iPort);
			if ( !p->ConnectDB() )
			{
				SET_ERROR_CODE(p->GetErrorCode());
				delete p;
				p = NULL;
			}

			pcsConn = p;

			m_iUsedConns ++;
		}
		break;
	default:
		MYASSERTV( (m_eType!=ODBC&&m_eType!=OCI&&m_eType!=MYSQL_API), m_eType );
		SET_ERROR_CODE(RETCODE_PARAMS_ERROR);
		break;
	}

	return pcsConn;
}

void CConnectionPool::DestroyConnection( IConnection* pcsConn )
{
	MYASSERT(pcsConn!=NULL);
	if ( pcsConn )
		delete pcsConn;

	m_iUsedConns--;
}

BOOL CConnectionPool::InitList( void )
{
	MyAutoMutex csAuto(m_csMutexInit);

	if ( !IsListEmpty() )
		return TRUE;

	// 创建连接
	for ( UInt32 i=0; i<m_iMinConns; i++ )
	{
		IConnection *p = CreateConnection();
		if ( !p )
		{
			ClearList();
			return FALSE;
		}
		AddList(p);
	}

	return TRUE;
}


void CConnectionPool::AddList( IConnection* pcsConn )
{
	BOOL bRet = FALSE;
	MYASSERT(!(bRet=IsListItem(pcsConn)));

	if ( !bRet )
	{
		// 加自动锁
		MyAutoMutex csAuto(m_csMutex);
		StruConnInfo stInfo;
		stInfo.bIdle = TRUE;
		stInfo.pConn = pcsConn;
		m_vecConnectionList.push_back(stInfo);
	}
}

void CConnectionPool::ClearList( void )
{
	// 加自动锁
	MyAutoMutex csAuto(m_csMutex);

	for ( ConnectionList::iterator iter = m_vecConnectionList.begin();
		iter != m_vecConnectionList.end();
		 )
	{
		if ( !(*iter).bIdle )
		{
			// 解锁
			m_csMutex.Unlock();
			MYWARNV(0, "db connection is busy. please wait...");
			Sleep(500);
			// 加锁
			m_csMutex.Lock();
		}
		else
		{
			DestroyConnection((*iter).pConn);
			iter ++;
		}
	}

	m_vecConnectionList.clear();
}

BOOL CConnectionPool::IsListItem( IConnection* pcsConn )
{
	// 加自动锁
	MyAutoMutex csAuto(m_csMutex);

	for ( ConnectionList::iterator iter = m_vecConnectionList.begin();
		iter != m_vecConnectionList.end();
		iter ++ )
	{
		if ( (*iter).pConn == pcsConn )
			return TRUE;
	}

	return FALSE;
}

IConnection* CConnectionPool::GetIdle( void )
{
	// 加自动锁
	MyAutoMutex csAuto(m_csMutex);

	for ( ConnectionList::iterator iter = m_vecConnectionList.begin();
		iter != m_vecConnectionList.end();
		iter ++ )
	{
		if ( (*iter).bIdle )
		{
			(*iter).bIdle = FALSE;
			return (*iter).pConn;
		}
	}

	return NULL;
}

void CConnectionPool::SetIdle( IConnection* pcsConn )
{
	// 加自动锁
	MyAutoMutex csAuto(m_csMutex);

	for ( ConnectionList::iterator iter = m_vecConnectionList.begin();
		iter != m_vecConnectionList.end();
		iter ++ )
	{
		if ( (*iter).pConn == pcsConn )
		{
			(*iter).bIdle = TRUE;
			break;
		}
	}
}

BOOL CConnectionPool::IsListEmpty( void )
{
	// 加自动锁
	MyAutoMutex csAuto(m_csMutex);

	return m_vecConnectionList.empty()?TRUE:FALSE;
}

BOOL CConnectionPool::IsOverMaxLink( void )
{
	return m_iUsedConns>=m_iMaxConns;
}
