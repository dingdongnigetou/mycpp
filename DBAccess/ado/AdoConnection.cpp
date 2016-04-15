
#include <sstream>

#include "AdoConnection.h"
#include "AdoRecordSet.h"

#define ERROR_STR_SIZE 512

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

	pErr_ = new char[ERROR_STR_SIZE];
	clearError();

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
		if (isOpen())
		{
			close();
		}

		pConn_.Release();
		pConn_ = NULL;
	}

	if (pErr_ != NULL)
	{
		delete pErr_;
		pErr_ = NULL;
	}
}

bool CAdoConnection::ConnectDB( )
{
	DB_POINTER_CHECK_RET(pConn_, false);

	try
	{
		pConn_->ConnectionString = strDB_.c_str();
		pConn_->ConnectionTimeout = 30;
		pConn_->Open(strDB_.c_str(), "", "", adConnectUnspecified);

	    MYDB_PRINT("ADO version : %s\n",   (char*)(_bstr_t)pConn_->GetVersion());

		return true;
	}
	catch (_com_error& e)
	{
		errorHandle();
		return false;
	}
}

bool CAdoConnection::close()
{
	DB_POINTER_CHECK_RET(pConn_, false);

	if (!isOpen())
	{
		return true;
	}

	try
	{
		pConn_->Close();
		return true;
	}
	catch (_com_error e)
	{
		MYDB_PRINT("Warning: 关闭数据库发生异常. 错误信息: %s; 文件: %s; 行: %d\n", e.ErrorMessage(), __FILE__, __LINE__);
		errorHandle();
		return false;
	}
}

bool CAdoConnection::isOpen()
{
	try
	{
		return (pConn_ != NULL && (pConn_->State & adStateOpen));
	}
	catch (_com_error e)
	{
		MYDB_PRINT("Warning: IsOpen 方法发生异常. 错误信息: %s; 文件: %s; 行: %d\n", e.ErrorMessage(), __FILE__, __LINE__);
		errorHandle();
		return false;
	} 
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
	if ( !testConnectAlive() )
		return false;

	try
	{
		if (isOpen())
		{
			if (pConn_->Execute(_bstr_t(szSql), NULL, adCmdText) != NULL)
			{
				clearError();
				return	true;
			}
			else
			{
				errorHandle();
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
		errorHandle();
		return false;
	}
}

bool CAdoConnection::GetLastInsertID(const char* szSeqName, signed __int64& lRowID)
{
	DB_POINTER_CHECK_RET(szSeqName, false);
	if ( !testConnectAlive() )
		return false;

	return true;
}

IRecordSet* CAdoConnection::ExecuteQuery( const char* szSql )
{
	DB_POINTER_CHECK_RET(szSql, NULL);
	if ( !testConnectAlive() )
		return NULL;

	auto res = new CAdoRecordSet(szSql, this);

	if (res->IsOpen())
	{
		clearError();
	}
	else
	{
		errorHandle();
		delete res;
		res = NULL;
	}

	return res;
}

IRecordSet* CAdoConnection::ExecutePageQuery( const char* szSql, int iStartRow, int iRowNum )
{
	DB_POINTER_CHECK_RET(szSql, NULL);
	//DB_POINTER_CHECK_RET(pConn_, NULL);
	if ( !testConnectAlive() )
		return false;

	// 合成完整SQL语句
	std::ostringstream ssFullSql;

	return ExecuteQuery(ssFullSql.str().c_str());
}

bool CAdoConnection::BeginTrans()
{
	if ( !testConnectAlive() )
		return false;

	DB_POINTER_CHECK_RET(pConn_, false);

	try
	{
		 pConn_->BeginTrans();
		 clearError();
		 return true;
	}
	catch (_com_error e)
	{
		MYDB_PRINT("Warning: BeginTrans 方法发生异常. 错误信息: %s; 文件: %s; 行: %d\n", e.ErrorMessage(), __FILE__, __LINE__);
		errorHandle();
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
		errorHandle();
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
			clearError();
			return	true;
		}		
		else
		{
			errorHandle();
			return false;
		}
	}
	catch(...)
	{
		errorHandle();
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
	if (std::string(pErr_).empty())
		return "no error.";

	return pErr_;
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

bool CAdoConnection::isReconnect( )
{
	// 若为网络连接错误则重连
	if ( GetErrorCode() == RETCODE_NETWORK_FAIL_CONNECT )
		return true;

	return false;
}

bool CAdoConnection::reconnectDB( )
{
	MYDB_PRINT("TRY RECONNECT DB( %s )... \n", strDB_.c_str());
	close();
	return ConnectDB();
}

void CAdoConnection::errorHandle()
{
	SetLastError();
}

void CAdoConnection::SetLastError()
{
	clearError();
	ErrorPtr p = pConn_->Errors->GetItem(pConn_->Errors->GetCount() - 1);
	sprintf_s(pErr_, ERROR_STR_SIZE, "%s", (char*)p->Description +'\0');
}

bool CAdoConnection::testConnectAlive( void )
{
	if ( !isOpen())
		if ( !reconnectDB() )
			return false;

	return true;
}

_ConnectionPtr& CAdoConnection::GetRawConnRef()
{
	return pConn_;
}

void CAdoConnection::clearError()
{
	memset(pErr_, 0x0, ERROR_STR_SIZE);
}
