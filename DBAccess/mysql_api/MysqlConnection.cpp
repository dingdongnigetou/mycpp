#include "MysqlConnection.h"
#include "MysqlRecordSet.h"

#include <sstream>

#include "errmsg.h"
#include "mysqld_error.h"

CMysqlConnection::CMysqlConnection( const std::string& strHost,
								   const std::string& strDataBase,
								   const std::string& strUserName,
								   const std::string& strPassword,
								   UInt16 iPort  )
:strHost_(strHost)
,strDB_(strDataBase)
,strUser_(strUserName)
,strPwd_(strPassword)
{
	if ( strHost.empty() )
		strHost_ = "localhost";

	memset(m_szDateTime, 0x0, sizeof(m_szDateTime));
}

CMysqlConnection::~CMysqlConnection()
{
	disconnectDB();
}

bool CMysqlConnection::ConnectDB()
{
	MYASSERT( nullptr == pMysqlConn_ );

	auto pConn = mysql_init(nullptr);

	pMysqlConn_ = pConn;

	pConn = mysql_real_connect(pMysqlConn_, 
		strHost_.c_str(),
		strUser_.c_str(),
		strPwd_.c_str(),
		strDB_.c_str(), 
		iPort_,
		nullptr,
		0);

	if ( !pConn )
	{
		errorPrint();
		pMysqlConn_ = nullptr;
		return false;
	}

	mysql_set_character_set( pConn, "gb2312");    //设置默认字符集

	mysql_autocommit(pConn, 1);        // 设置为自动提交模式

	MYDB_PRINT("MYSQL version : %i\n",   mysql_get_server_version(pConn) );

	return true;
}

void CMysqlConnection::ReleaseRecordSet( IRecordSet** pcsRecordSet )
{
	MY_ASSERT_RET(*pcsRecordSet);

	delete *pcsRecordSet;
	*pcsRecordSet = nullptr;
}

IRecordSet*   CMysqlConnection::PrepareBind( const char* szSql )
{
	MY_ASSERT_RET_VAL(szSql, nullptr);

	if ( !testConnectAlive() )
		return nullptr;

	MYSQL_STMT *pStmt = nullptr;
	pStmt = mysql_stmt_init(pMysqlConn_);
	MYASSERT( nullptr != pStmt );

	if ( !pStmt )
	{
		errorHandle();
		return nullptr;
	}

	if ( 0 != mysql_stmt_prepare(pStmt, szSql,strlen(szSql)) )
	{
		errorHandle();
		mysql_stmt_close(pStmt);    // TODO:  需要测试!!!
		pStmt = nullptr;
		return nullptr;
	}
	//关闭自动提交
	if ( 0 != mysql_autocommit(pMysqlConn_, 0) )
	{
		errorHandle();
		return false;
	}

	// pStmt 由类CMysqlRecordSet释放
	 return new CMysqlRecordSet(pMysqlConn_, pStmt);
}

bool CMysqlConnection::ExecuteBind( IRecordSet* pcsRecordSet )
{
	MY_ASSERT_RET_VAL(pcsRecordSet, false);

	if ( !testConnectAlive() )
		return false;

    //开启自动提交
	if ( 0 != mysql_autocommit(pMysqlConn_, 1) )
	{
		errorHandle();
		return false;
	}

	auto pMysqlRecSet =(CMysqlRecordSet*)pcsRecordSet;

	if ( !pMysqlRecSet->IsBindSuccess() )
	{
		errorHandle();
		return false;
	}

	return true;
}

bool CMysqlConnection::ExecuteSql( const char* szSql )
{
	MY_ASSERT_RET_VAL(szSql, false);
	
	if ( !testConnectAlive() )
		return false;
    
    auto res = mysql_query(pMysqlConn_, szSql);

	if( 0 != res)
    {
		errorHandle();
		return false;
	}

	return true;
}

bool CMysqlConnection::GetLastInsertID( const char* szSeqName, signed __int64& lRowID )
{
	if ( !testConnectAlive() )
		return false;

	auto lInsertID = mysql_insert_id(pMysqlConn_);
	if( lInsertID == 0 )
	    return false;

	lRowID = lInsertID;

	return true;
}

IRecordSet*  CMysqlConnection::ExecuteQuery( const char* szSql )
{
	MY_ASSERT_RET_VAL(szSql, nullptr);

	if ( !testConnectAlive() )
		return false;

	if ( 0 != mysql_query(pMysqlConn_,szSql) )
	{
		errorHandle();
		return nullptr;
	}

	return new CMysqlRecordSet(pMysqlConn_,nullptr);
}

IRecordSet*  CMysqlConnection::ExecutePageQuery( const char* szSql, int iStartRow, int iRowNum )
{

	MY_ASSERT_RET_VAL(szSql, nullptr);

	if ( !testConnectAlive() )
		return false;

	// 合成完整SQL语句
	std::ostringstream ssFullSql;

	ssFullSql << szSql;
	//添加SQL后面语句
    ssFullSql << " LIMIT ";
	ssFullSql <<  iStartRow; 
    ssFullSql <<  ","; 
	ssFullSql << iRowNum;

	return ExecuteQuery(ssFullSql.str().c_str());
}

bool  CMysqlConnection::BeginTrans()
{
	if ( !testConnectAlive() )
		return false;

	MY_ASSERT_RET_VAL(pMysqlConn_, false);

    //关闭自动提交
	if ( 0 != mysql_autocommit(pMysqlConn_, 0) )
	{
		errorHandle();
		return false;
	}

	return true;
}

void  CMysqlConnection::Rollback()
{
	MY_ASSERT_RET(pMysqlConn_);

	// 回滚
	if (  0 != mysql_rollback(pMysqlConn_) )
	{
		errorHandle();
		return ;
	}

	// 设置自动提交
	if ( 0 != mysql_autocommit(pMysqlConn_, 1) )
	{
		errorHandle();
		return ;
	}
}

bool CMysqlConnection::Commit()
{
	MY_ASSERT_RET_VAL(pMysqlConn_, false);

	// 事务提交
	if (  0 != mysql_commit(pMysqlConn_) )
	{
		errorHandle();
		return false;
	}

	//设置自动提交
	if ( 0 != mysql_autocommit(pMysqlConn_, 1) )
	{
		errorHandle();
		return false;
	}

	return true;
}

// 获取错误码
EnumDBApiRet CMysqlConnection::GetErrorCode()
{
	if ( !pMysqlConn_ )
		return RETCODE_INITIALIZE_FAIL;
    
	auto iErrCode = mysql_errno(pMysqlConn_);
	switch ( iErrCode )
	{
	case ER_ACCESS_DENIED_ERROR: //不能连接数据库，用户名或密码错误
	case ER_DBACCESS_DENIED_ERROR:  //当前用户没有访问数据库的权限
		return RETCODE_USERNAME_PASSWORD_ERROR;
		
	case ER_CON_COUNT_ERROR: // 连接过多
	case ER_TOO_MANY_USER_CONNECTIONS: // 用户%s已有了超过'max_user_connections'的活动连接
		return RETCODE_OVER_MAXLINK;

	case  ER_SYNTAX_ERROR: // 存在SQL语法错误
		return RETCODE_SQL_SYNTAX_ERROR;

	case ER_DUP_UNIQUE: // 唯一性限制
		return RETCODE_UNIQUE_CONSTRAINT_VIOLATED;

	//未连接到mysql
	case ER_NET_READ_ERROR: //网络错误，出现读错误，请检查网络连接状况
	case ER_NET_READ_INTERRUPTED: //网络错误，读超时，请检查网络连接状况
	case ER_NET_ERROR_ON_WRITE: //网络错误，出现写错误，请检查网络连接状况
	case ER_NET_WRITE_INTERRUPTED: //网络错误，写超时，请检查网络连接状况
	case CR_SERVER_GONE_ERROR: //2006 MySQL服务器不可用		
	case CR_SERVER_LOST: //2013 查询过程中丢失了与MySQL服务器的连接
		return RETCODE_NETWORK_FAIL_CONNECT;

	default:
		break;
	}

	return RETCODE_UNKNOWN_ERROR;
}

const char* CMysqlConnection::GetErrorMessage()
{
	if ( !pMysqlConn_ )
		return "mysql api initialize fail!";

	auto pErr = mysql_error(pMysqlConn_);
	if ( !pErr )
		return "no error.";

    return  pErr;
}

const char*  CMysqlConnection::ToTime(const char* szDateTime)
{

	MY_ASSERT_RET_VAL(szDateTime, nullptr);

	memset(m_szDateTime, 0x0, sizeof(m_szDateTime));

	MYSNPRINTF(m_szDateTime, 
		sizeof(m_szDateTime)-1,
		"STR_TO_DATE('%s', '%%H:%%i:%%s')",
		szDateTime);

	return m_szDateTime;
}

const char*  CMysqlConnection::ToDate(const char* szDateTime)
{
	MY_ASSERT_RET_VAL(szDateTime, nullptr);

	memset(m_szDateTime, 0x0, sizeof(m_szDateTime));

	MYSNPRINTF(m_szDateTime, 
		sizeof(m_szDateTime)-1,
		"STR_TO_DATE('%s', '%%Y-%%m-%%d')",
		szDateTime);

	return m_szDateTime;
}

const char*  CMysqlConnection::ToDateTime(const char* szDateTime)
{
	MY_ASSERT_RET_VAL(szDateTime, nullptr);

	memset(m_szDateTime, 0x0, sizeof(m_szDateTime));

	MYSNPRINTF(m_szDateTime, 
		sizeof(m_szDateTime)-1,
		"STR_TO_DATE('%s', '%%Y-%%m-%%d %%H:%%i:%%s ')", 
		szDateTime);

	return m_szDateTime;
}

const char*  CMysqlConnection::TimeToStr(const char* szDateTime)
{
	MY_ASSERT_RET_VAL(szDateTime, nullptr);

	memset(m_szDateTime, 0x0, sizeof(m_szDateTime));

	MYSNPRINTF(m_szDateTime, 
		sizeof(m_szDateTime)-1, 
		"TIME_FORMAT('%s', '%%H:%%i:%%s')",szDateTime);

	return m_szDateTime;
}

const char*  CMysqlConnection::DateToStr(const char* szDateTime)
{
	MY_ASSERT_RET_VAL(szDateTime, nullptr);

	memset(m_szDateTime, 0x0, sizeof(m_szDateTime));

	MYSNPRINTF(m_szDateTime, 
		sizeof(m_szDateTime)-1,
		"DATE_FORMAT('%s', '%%Y-%%m-%%d')",szDateTime);

	return m_szDateTime;
}

const char*  CMysqlConnection::DateTimeToStr(const char* szDateTime)
{
	MY_ASSERT_RET_VAL(szDateTime, nullptr);

	memset(m_szDateTime, 0x0, sizeof(m_szDateTime));

	MYSNPRINTF(m_szDateTime,
		sizeof(m_szDateTime)-1,
		"DATE_FORMAT('%s', '%%Y-%%m-%%d %%H:%%i:%%s')",szDateTime);

	return m_szDateTime;
}

const char*  CMysqlConnection::GetSysTime()
{
	memset(m_szDateTime, 0x0, sizeof(m_szDateTime));
	MYSNPRINTF(m_szDateTime,
		sizeof(m_szDateTime)-1,
		"SELECT CURTIME()");

	auto pRescordSet = ExecuteQuery(m_szDateTime);
	if ( !pRescordSet )
	{
		errorHandle();
		return nullptr;
	}

	memset(m_szDateTime, 0x0, sizeof(m_szDateTime));

	unsigned int iFactLen = 0;
	if ( !pRescordSet->GetValue( 1, m_szDateTime, sizeof(m_szDateTime), &iFactLen ) )
	{
		ReleaseRecordSet(&pRescordSet);
		errorHandle();
		return nullptr;
	}

	ReleaseRecordSet(&pRescordSet);

	return m_szDateTime;
}

const char*  CMysqlConnection::GetSysDate()
{
	memset(m_szDateTime, 0x0, sizeof(m_szDateTime));
	MYSNPRINTF(m_szDateTime,
		sizeof(m_szDateTime)-1,
		"SELECT CURDATE()");
    
	auto pRescordSet = ExecuteQuery(m_szDateTime);
	if ( !pRescordSet )
	{
		errorHandle();
		return false;
	}

	memset(m_szDateTime, 0x0, sizeof(m_szDateTime));

	unsigned int iFactLen = 0;
	if ( !pRescordSet->GetValue( 1, m_szDateTime, sizeof(m_szDateTime), &iFactLen ) )
	{
		ReleaseRecordSet(&pRescordSet);
		errorHandle();
		return nullptr;
	}

	ReleaseRecordSet(&pRescordSet);

	return m_szDateTime;
}

const char*  CMysqlConnection::GetSysDateTime()
{
	memset(m_szDateTime, 0x0, sizeof(m_szDateTime));
	MYSNPRINTF(m_szDateTime,
		sizeof(m_szDateTime)-1,
		"SELECT NOW()");

	auto pRescordSet = ExecuteQuery(m_szDateTime);
	if ( !pRescordSet )
	{
		errorHandle();
		return false;
	}

	memset(m_szDateTime, 0x0, sizeof(m_szDateTime));

	unsigned int iFactLen = 0;
	if ( !pRescordSet->GetValue( 1, m_szDateTime, sizeof(m_szDateTime), &iFactLen ) )
	{
		ReleaseRecordSet(&pRescordSet);
		errorHandle();
		return nullptr;
	}

	ReleaseRecordSet(&pRescordSet);

	return m_szDateTime;
}

// 关闭连接
void  CMysqlConnection::disconnectDB()
{
	if ( pMysqlConn_ )
	{
		mysql_close(pMysqlConn_);
		pMysqlConn_ = nullptr;
	}

}

// 是否重连
bool CMysqlConnection::isReconnect()
{
	// 若为网络连接错误则重连
	if ( GetErrorCode() == RETCODE_NETWORK_FAIL_CONNECT )
		return true;

	return false;
}

// 重新连接（由内部保证重连）
bool CMysqlConnection::reconnectDB()
{
	MYDB_PRINT("TRY RECONNECT DB( %s )... \n", strDB_.c_str());
	disconnectDB();
	return ConnectDB();
}

void CMysqlConnection::errorHandle()
{
	// 错误信息打印
	errorPrint();

	// 重连
	if ( isReconnect() )
		reconnectDB();
}

// 错误信息打印
void  CMysqlConnection::errorPrint()
{   
	MY_ASSERT_RET( nullptr != pMysqlConn_ );

	MYDB_PRINT( "MYSQL ERROR :\n"
		"CODE : %d\n"
		"MSG  : %s\n",
		mysql_errno(pMysqlConn_),
		mysql_error(pMysqlConn_)
		);
}

// 测试连接状态
bool CMysqlConnection::testConnectAlive()
{
	if( !pMysqlConn_)
		if ( !reconnectDB() )
			return false;

	return true;
}
