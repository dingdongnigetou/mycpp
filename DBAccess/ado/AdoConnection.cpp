
#include <sstream>

#include "AdoConnection.h"
#include "AdoRecordSet.h"

#define ERROR_STR_SIZE 512

CAdoConnection::CAdoConnection( const std::string& strHost,
							   const std::string& strDataBase,
							   const std::string& strUserName,
							   const std::string& strPassword,
							   UInt16 iPort )
{
	pErr_ = new char[ERROR_STR_SIZE];
	ClearError();

	char buf[512];
	sprintf_s(buf,"Provider=SQLOLEDB.1;Password=%s;    \
		Persist Security Info=true;User ID=%s;Initial Catalog=%s; \
		Data Source=%s",strPassword.c_str(),strUserName.c_str(),strDataBase.c_str(),strHost.c_str());

	strDB_ = std::string(buf);

	memset(szDateTime_, 0x0, sizeof(szDateTime_));

	pConn_.CreateInstance("ADODB.Connection");
}

CAdoConnection::~CAdoConnection()
{
	if (pConn_ != nullptr)
	{
		if (isOpen())
		{
			close();
		}

		pConn_.Release();
		pConn_ = nullptr;
	}

	if (pErr_ != nullptr)
	{
		delete pErr_;
		pErr_ = nullptr;
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
		ClearError();

	    MYDB_PRINT("ADO version : %s\n",   (char*)(_bstr_t)pConn_->GetVersion());

		return true;
	}
	catch (_com_error& e)
	{
		errorHandle(e.Description());
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
		errorHandle(e.Description());
		return false;
	}
}

bool CAdoConnection::isOpen()
{
	try
	{
		if (pConn_ == nullptr || !(pConn_->State & adStateOpen))
			return false;
	}
	catch (_com_error e)
	{
		MYDB_PRINT("Warning: IsOpen 方法发生异常. 错误信息: %s; 文件: %s; 行: %d\n", e.ErrorMessage(), __FILE__, __LINE__);
		errorHandle(e.Description());
		return false;
	} 

	return true;
}

void CAdoConnection::ReleaseRecordSet( IRecordSet** pcsRecordSet )
{
	MYASSERT(*pcsRecordSet);
	
	if ( *pcsRecordSet )
	{
		delete *pcsRecordSet;
		*pcsRecordSet = nullptr;
	}
}

IRecordSet* CAdoConnection::PrepareBind( const char* szSql )
{
	DB_POINTER_CHECK_RET(szSql, nullptr);
	return nullptr;
}

bool CAdoConnection::ExecuteBind( IRecordSet* pcsRecordSet )
{
	DB_POINTER_CHECK_RET(pcsRecordSet, false);
	return true;
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
			if (pConn_->Execute(_bstr_t(szSql), nullptr, adCmdText) != nullptr)
			{
				ClearError();
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
	catch (_com_error& e)
	{
		errorHandle(e.Description());
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
	DB_POINTER_CHECK_RET(szSql, nullptr);
	if ( !testConnectAlive() )
		return nullptr;

	auto res = new CAdoRecordSet(szSql, this);

	if (res->IsOpen())
	{
		ClearError();
	}
	else
	{
		errorHandle();
		delete res;
		res = nullptr;
	}

	return res;
}

IRecordSet* CAdoConnection::ExecutePageQuery( const char* szSql, int iStartRow, int iRowNum )
{
	DB_POINTER_CHECK_RET(szSql, nullptr);
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
		 ClearError();
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
	catch(_com_error& e)
	{
		errorHandle(e.Description());
		return ;
	} 

}

bool CAdoConnection::Commit()
{
	if ( !pConn_ )
		return false;

	try
	{
		if (SUCCEEDED(pConn_->CommitTrans()))
		{
			ClearError();
			return	true;
		}		
		else
		{
			errorHandle();
			return false;
		}
	}
	catch(_com_error& e)
	{
		errorHandle(e.Description());
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

const char* CAdoConnection::GetErrorMessage()
{
	if (std::string(pErr_).empty())
		return "no error.";

	return pErr_;
}

const char* CAdoConnection::ToTime(const char* szDateTime)
{
	DB_POINTER_CHECK_RET(szDateTime, nullptr);

	return szDateTime_;

}

const char* CAdoConnection::ToDate(const char* szDateTime)
{
	DB_POINTER_CHECK_RET(szDateTime, nullptr);

	return szDateTime_;

}

const char* CAdoConnection::ToDateTime(const char* szDateTime)
{
	DB_POINTER_CHECK_RET(szDateTime, nullptr);

	return szDateTime_;
}

const char* CAdoConnection::TimeToStr(const char* szDateTime)
{
	DB_POINTER_CHECK_RET(szDateTime, nullptr);

	return szDateTime_;

}

const char* CAdoConnection::DateToStr(const char* szDateTime)
{
	DB_POINTER_CHECK_RET(szDateTime, nullptr);

	return szDateTime_;

}

const char* CAdoConnection::DateTimeToStr(const char* szDateTime)
{
	DB_POINTER_CHECK_RET(szDateTime, nullptr);

	return szDateTime_;
}

const char* CAdoConnection::GetSysTime()
{
	return szDateTime_;
}

const char* CAdoConnection::GetSysDate()
{
	return szDateTime_;
}

const char* CAdoConnection::GetSysDateTime()
{
	return szDateTime_;
}

bool CAdoConnection::isReconnect()
{
	// 若为网络连接错误则重连
	if (GetErrorCode() == RETCODE_NETWORK_FAIL_CONNECT)
		return true;

	return false;
}

bool CAdoConnection::reconnectDB()
{
	MYDB_PRINT("TRY RECONNECT DB( %s )... \n", strDB_.c_str());
	close();
	return ConnectDB();
}

void CAdoConnection::errorHandle(const char* err)
{
	SetLastError(err);
}

void CAdoConnection::SetLastError(const char* err)
{
	ClearError();
	if (err != nullptr)
	{
		sprintf_s(pErr_, ERROR_STR_SIZE, "%s", err);
		return;
	}

	ErrorPtr p = pConn_->Errors->GetItem(pConn_->Errors->GetCount() - 1);
	sprintf_s(pErr_, ERROR_STR_SIZE, "%s", (char*)p->Description + '\0');
}

bool CAdoConnection::testConnectAlive()
{
	if (!isOpen()
		|| std::string(pErr_) == "连接失败"
		|| std::string(pErr_).find("一般性网络错误") != std::string::npos)
	{
		if (!reconnectDB())
			return false;
	}

	return true;
}

_ConnectionPtr& CAdoConnection::GetRawConnRef()
{
	return pConn_;
}

void CAdoConnection::ClearError()
{
	memset(pErr_, 0x0, ERROR_STR_SIZE);
}

