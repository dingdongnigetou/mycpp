#include "ConnectionPool.h"

#include "OciConnection.h"
#include "MysqlConnection.h"
#include "AdoConnection.h"

#define SET_ERROR_CODE(err) (eError_ = err)
#define GET_ERROR_CODE(perr) if ( perr ) {*perr = eError_;}

using namespace mycpp;

CConnectionPool::CConnectionPool( EnumDriverType eType )
:eType_(eType)
,strHost_("")
,strDataBase_("")
,strUserName_("")
,strPassword_("")
,iPort_(0)
,iMinConns_(0)
,iMaxConns_(0)
,iUsedConns_(0)
,eError_(RETCODE_SUCCESS)
{

}

CConnectionPool::~CConnectionPool()
{
	clearList();
}

EnumDriverType CConnectionPool::GetDriverType()
{
	return eType_;
}

void CConnectionPool::SetParams( const char* szHost,
								  const char* szDataBase,
								  const char* szUserName,
								  const char* szPassword,
								  unsigned short iPort,
								  unsigned int iMinConns,
								  unsigned int iMaxConns )
{
	//MYASSERT(szHost!=nullptr);  //本地数据库可以为nullptr
	MYASSERT(szDataBase!=nullptr);
	MYASSERT(szUserName!=nullptr);
	//MYASSERT(szPassword!=nullptr); // 密码可以为nullptr
	MYASSERT(iMinConns>0&&iMinConns<=iMaxConns);
	MYASSERT(iMinConns<=iMaxConns&&iMaxConns<=50);

	if ( !szUserName
		|| (iMinConns<0||iMinConns>iMaxConns)
		|| (iMaxConns<iMinConns||iMaxConns>50) )
	{
		MYWARNV(!strDataBase_.empty(),"db name is nullptr!");
		MYWARNV(!strUserName_.empty(), "user name is nullptr!");
		MYWARNV(iMinConns>0&&iMinConns<=iMaxConns, "min connections illegal");
		MYWARNV(iMinConns<=iMaxConns&&iMaxConns<=50, "max connections illegal");
		SET_ERROR_CODE(RETCODE_PARAMS_ERROR);
		return ;
	}
	
	if ( szHost ) strHost_  = std::string(szHost);
	if ( szPassword ) strPassword_ = std::string(szPassword);

	strDataBase_ = std::string(szDataBase);
	strUserName_ = std::string(szUserName);

	iPort_     = iPort;
	iMinConns_ = iMinConns;
	iMaxConns_ = iMaxConns;
}

IConnection* CConnectionPool::GetConnection( EnumDBApiRet* eError )
{
	IConnection *pConn = nullptr;
	bool bRet = false;

	MYASSERT(bRet=isNoError());
	if ( !bRet )
	{
		GET_ERROR_CODE(eError);
		return pConn;
	}

	MYASSERT(bRet=initList());
	if ( !bRet )
	{
		GET_ERROR_CODE(eError);
		return pConn;
	}

	pConn = getIdle();
	if ( !pConn )
	{
		if ( !isOverMaxLink() )
		{
			pConn = createConnection();
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
	MYASSERT(*pcsConn!=nullptr);

	if ( !isListItem(*pcsConn) )
		destroyConnection(*pcsConn);
	else
		setIdle(*pcsConn);

	*pcsConn = nullptr;
}

bool CConnectionPool::isNoError()
{
	return (eError_==RETCODE_SUCCESS)?true:false;
}

IConnection* CConnectionPool::createConnection()
{
	IConnection* pcsConn = nullptr;

	switch ( eType_ )
	{
	case ODBC:
		MYWARNV(0, "Not support ODBC driver type!");
		break;
	case OCI:
		{
			COciConnection *p = new COciConnection(strHost_,
				strDataBase_,
				strUserName_,
				strPassword_,
				iPort_);
			if ( !p->ConnectDB() )
			{
				SET_ERROR_CODE(p->GetErrorCode());
				delete p;
				p = nullptr;
			}

			pcsConn = p;
			
			iUsedConns_ ++;
		}
		break;
	case MYSQL_API:
		{
			CMysqlConnection *p = new CMysqlConnection(strHost_,
				strDataBase_,
				strUserName_,
				strPassword_,
				iPort_);
			if ( !p->ConnectDB() )
			{
				SET_ERROR_CODE(p->GetErrorCode());
				delete p;
				p = nullptr;
			}

			pcsConn = p;

			iUsedConns_ ++;
		}
	case ADO:
       {
			CAdoConnection *p = new CAdoConnection(strHost_,
				strDataBase_,
				strUserName_,
				strPassword_,
				iPort_);
			if ( !p->ConnectDB() )
			{
				SET_ERROR_CODE(p->GetErrorCode());
				delete p;
				p = nullptr;
			}

			pcsConn = p;

			iUsedConns_ ++;
       }
		break;
	default:
		MYASSERTV( (eType_!=ODBC&&eType_!=OCI&&eType_!=MYSQL_API&&eType_!=ADO), eType_ );
		SET_ERROR_CODE(RETCODE_PARAMS_ERROR);
		break;
	}

	return pcsConn;
}

void CConnectionPool::destroyConnection( IConnection* pcsConn )
{
	MYASSERT(pcsConn!=nullptr);
	if ( pcsConn )
		delete pcsConn;

	iUsedConns_--;
}

bool CConnectionPool::initList()
{
	MyAutoMutex csAuto(csMutexInit_);

	if ( !isListEmpty() )
		return true;

	// 创建连接
	for ( UInt32 i=0; i<iMinConns_; i++ )
	{
		auto p = createConnection();
		if ( !p )
		{
			clearList();
			return false;
		}
		addList(p);
	}

	return true;
}


void CConnectionPool::addList( IConnection* pcsConn )
{
	bool bRet = false;
	MYASSERT(!(bRet=isListItem(pcsConn)));

	if ( !bRet )
	{
		// 加自动锁
		MyAutoMutex csAuto(csMutex_);
		StruConnInfo stInfo;
		stInfo.bIdle = true;
		stInfo.pConn = pcsConn;
		vecConnectionList_.push_back(stInfo);
	}
}

void CConnectionPool::clearList()
{
	// 加自动锁
	MyAutoMutex csAuto(csMutex_);

	for ( ConnectionList::iterator iter = vecConnectionList_.begin();
		iter != vecConnectionList_.end();
		 )
	{
		if ( !(*iter).bIdle )
		{
			// 解锁
			csMutex_.Unlock();
			MYWARNV(0, "db connection is busy. please wait...");
			Sleep(500);
			// 加锁
			csMutex_.Lock();
		}
		else
		{
			destroyConnection((*iter).pConn);
			iter ++;
		}
	}

	vecConnectionList_.clear();
}

bool CConnectionPool::isListItem( IConnection* pcsConn )
{
	// 加自动锁
	MyAutoMutex csAuto(csMutex_);

	for ( ConnectionList::iterator iter = vecConnectionList_.begin();
		iter != vecConnectionList_.end();
		iter ++ )
	{
		if ( (*iter).pConn == pcsConn )
			return true;
	}

	return false;
}

IConnection* CConnectionPool::getIdle()
{
	// 加自动锁
	MyAutoMutex csAuto(csMutex_);

	for ( ConnectionList::iterator iter = vecConnectionList_.begin();
		iter != vecConnectionList_.end();
		iter ++ )
	{
		if ( (*iter).bIdle )
		{
			(*iter).bIdle = false;
			return (*iter).pConn;
		}
	}

	return nullptr;
}

void CConnectionPool::setIdle( IConnection* pcsConn )
{
	// 加自动锁
	MyAutoMutex csAuto(csMutex_);

	for ( ConnectionList::iterator iter = vecConnectionList_.begin();
		iter != vecConnectionList_.end();
		iter ++ )
	{
		if ( (*iter).pConn == pcsConn )
		{
			(*iter).bIdle = true;
			break;
		}
	}
}

bool CConnectionPool::isListEmpty()
{
	// 加自动锁
	MyAutoMutex csAuto(csMutex_);

	return vecConnectionList_.empty()?true:false;
}

bool CConnectionPool::isOverMaxLink()
{
	return iUsedConns_>=iMaxConns_;
}
