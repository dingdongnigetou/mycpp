
#include <sstream>

#include "AdoConnection.h"
#include "AdoRecordSet.h"


CAdoConnection::CAdoConnection( const std::string& strHost,
							   const std::string& strDataBase,
							   const std::string& strUserName,
							   const std::string& strPassword,
							   UInt16 iPort )
:strUser_(strUserName)
,strPwd_(strPassword)
,pConn_(NULL)
,pErr_(NULL)
{
	if ( iPort == 0 )
		iPort = 1433;    // sqlserver默认端口 

	char buf[512];
	sprintf_s(buf,"Provider=SQLOLEDB.1;Password=%s;    \
		Persist Security Info=True;User ID=%s;Initial Catalog=%s; \
		Data Source=%s",strPassword.c_str(),strUserName.c_str(),strDataBase.c_str(),strHost.c_str());

	strDB_ = std::string(buf);

	memset(szDateTime_, 0x0, sizeof(szDateTime_));

	pConn_.CreateInstance("ADODB.Connection");
}

CAdoConnection::~CAdoConnection(void)
{
	if (pConn_ != NULL)
	{
		if (IsOpen())
		{
			Close();
		}

		pConn_.Release();
		pConn_ = NULL;
	}
}

bool CAdoConnection::ConnectDB( )
{
	MYASSERT(pConn_ == NULL);

	try
	{
		pConn_->ConnectionString = strDB_.c_str();
		pConn_->ConnectionTimeout = 30;
		pConn_->Open(strDB_.c_str(), "", "", adConnectUnspecified);
	}
	catch (_com_error& e)
	{
		ErrorHandle();
		return false;
	}

	MYDB_PRINT("Server major    version : %s\n",   (char*)(_bstr_t)pConn_->GetVersion());

	return true;
}

bool CAdoConnection::Close()
{
	if (pConn_ == NULL)
	{
		return false;
	}

	if (!IsOpen())
	{
		return true;
	}

	try
	{
		if (pConn_ != NULL && IsOpen()) 
		{
			pConn_->Close();
		}
	}
	catch (_com_error e)
	{
		//MYDB_PRINT("Warning: 关闭数据库发生异常. 错误信息: %s; 文件: %s; 行: %d\n", e.ErrorMessage(), __FILE__, __LINE__);
		ErrorHandle();
		return false;
	}

	return true;
}

bool CAdoConnection::IsOpen()
{
	try
	{
		return (pConn_ != NULL && (pConn_->State & adStateOpen));
	}
	catch (_com_error e)
	{
		//MYDB_PRINT("Warning: IsOpen 方法发生异常. 错误信息: %s; 文件: %s; 行: %d\n"), e.ErrorMessage(), __FILE__, __LINE__);
		ErrorHandle();
		return false;
	} 

	return	true;
}

void CAdoConnection::ReleaseRecordSet( IRecordSet** pcsRecordSet )
{
	MYASSERT(*pcsRecordSet);
	
	if ( *pcsRecordSet )
	{
		delete *pcsRecordSet;
		*pcsRecordSet = NULL;
	}
}

IRecordSet* CAdoConnection::PrepareBind( const char* szSql )
{
	DB_POINTER_CHECK_RET(szSql, NULL);
}

bool CAdoConnection::ExecuteBind( IRecordSet* pcsRecordSet )
{
	DB_POINTER_CHECK_RET(pcsRecordSet, false);
}

bool CAdoConnection::ExecuteSql( const char* szSql )
{
	DB_POINTER_CHECK_RET(szSql, false);
	if ( !TestConnectAlive() )
		return false;

	try
	{
		if (IsOpen())
		{
			if (pConn_->Execute(_bstr_t(szSql), NULL, adCmdText) != NULL)
			{
				return	true;
			}
			else
			{
				return	false;
			}
		}
		else
		{
			ConnectDB();
			return false;
		}
	}
	catch (...)
	{
		ErrorHandle();
		return false;
	}
}

bool CAdoConnection::GetLastInsertID(const char* szSeqName, signed __int64& lRowID)
{
	DB_POINTER_CHECK_RET(szSeqName, false);
	if ( !TestConnectAlive() )
		return false;

	return true;
}

IRecordSet* CAdoConnection::ExecuteQuery( const char* szSql )
{
	DB_POINTER_CHECK_RET(szSql, NULL);
	if ( !TestConnectAlive() )
		return false;

	return (new CAdoRecordSet(szSql, this));
}

IRecordSet* CAdoConnection::ExecutePageQuery( const char* szSql, int iStartRow, int iRowNum )
{
	DB_POINTER_CHECK_RET(szSql, NULL);
	//DB_POINTER_CHECK_RET(pConn_, NULL);
	if ( !TestConnectAlive() )
		return false;

	// 合成完整SQL语句
	std::ostringstream ssFullSql;

	return ExecuteQuery(ssFullSql.str().c_str());
}

bool CAdoConnection::BeginTrans()
{
	if ( !TestConnectAlive() )
		return false;

	DB_POINTER_CHECK_RET(pConn_, false);

	try
	{
		 pConn_->BeginTrans();
		 return true;
	}
	catch (_com_error e)
	{
		//MYDB_PRINT("Warning: BeginTrans 方法发生异常. 错误信息: %s; 文件: %s; 行: %d\n", e.ErrorMessage(), __FILE__, __LINE__);
		ErrorHandle();
		return false;
	}
}

void CAdoConnection::Rollback()
{
	if ( !pConn_ )
		return ;
	try
	{
		if (SUCCEEDED(pConn_->RollbackTrans()))
		{
			return;
		}		
	}
	catch(...)
	{
		ErrorHandle();
		return ;
	} 

}

bool CAdoConnection::Commit( void )
{
	//DB_POINTER_CHECK_RET(pConn_, false);
	if ( !pConn_ )
		return false;

	try
	{
		if (SUCCEEDED(pConn_->CommitTrans()))
		{
			return	true;
		}		
	}
	catch(...)
	{
		ErrorHandle();
		return false;
	} 

	return false;
}

EnumDBApiRet CAdoConnection::GetErrorCode()
{
	if ( !pErr_ )
		return RETCODE_SUCCESS;
	return RETCODE_UNKNOWN_ERROR;
}

const char* CAdoConnection::GetErrorMessage( void )
{
	if ( !pErr_ )
		return "no error.";

	return pErr_->GetDescription();
}

const char* CAdoConnection::ToTime( const char* szDateTime )
{
	DB_POINTER_CHECK_RET(szDateTime, NULL);

	return szDateTime_;

}

const char* CAdoConnection::ToDate( const char* szDateTime )
{
	DB_POINTER_CHECK_RET(szDateTime, NULL);

		return szDateTime_;

}

const char* CAdoConnection::ToDateTime( const char* szDateTime )
{
	DB_POINTER_CHECK_RET(szDateTime, NULL);


		return szDateTime_;
}

const char* CAdoConnection::TimeToStr( const char* szDateTime )
{
	DB_POINTER_CHECK_RET(szDateTime, NULL);

		return szDateTime_;

}

const char* CAdoConnection::DateToStr( const char* szDateTime )
{
	DB_POINTER_CHECK_RET(szDateTime, NULL);

		return szDateTime_;

}

const char* CAdoConnection::DateTimeToStr(const char* szDateTime)
{
	DB_POINTER_CHECK_RET(szDateTime, NULL);

		return szDateTime_;
}

const char* CAdoConnection::GetSysTime( void )
{
		return szDateTime_;
}

const char* CAdoConnection::GetSysDate( void )
{
		return szDateTime_;
}

const char* CAdoConnection::GetSysDateTime( void )
{   
	return szDateTime_;
}

bool CAdoConnection::IsReconnect( )
{
	// 若为网络连接错误则重连
	if ( GetErrorCode() == RETCODE_NETWORK_FAIL_CONNECT )
		return true;

	return false;
}

bool CAdoConnection::ReconnectDB( )
{
	MYDB_PRINT("TRY RECONNECT DB( %s )... \n", strDB_.c_str());
	Close();
	return ConnectDB();
}

void CAdoConnection::ErrorHandle( )
{
	SetLastError();
}

void CAdoConnection::SetLastError()
{
	pErr_ = pConn_->Errors;
}

bool CAdoConnection::TestConnectAlive( void )
{
	if ( !IsOpen())
		if ( !ReconnectDB() )
			return false;

	return true;
}

_ConnectionPtr& CAdoConnection::GetRawConnRef()
{
	return pConn_;
}