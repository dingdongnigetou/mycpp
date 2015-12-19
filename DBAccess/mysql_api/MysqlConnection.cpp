#include "MysqlConnection.h"
#include "MysqlRecordSet.h"

#include <sstream>

#include "errmsg.h"
#include "mysqld_error.h"


CMysqlConnection::CMysqlConnection( const tstring& strHost,
								   const tstring& strDataBase,
								   const tstring& strUserName,
								   const tstring& strPassword,
								   UInt16 iPort  )
								   
:m_strHost(strHost)
,m_strDB(strDataBase)
,m_strUser(strUserName)
,m_strPwd(strPassword)
,m_iPort(iPort)   // 默认填0
,m_pMysqlConn(NULL)
{
	if ( strHost.empty() )
		m_strHost = "localhost";

	memset(m_szDateTime, 0x0, sizeof(m_szDateTime));
}

CMysqlConnection::~CMysqlConnection(void)
{
	DisconnectDB();
}


bool CMysqlConnection::ConnectDB( void )
{
	MYASSERT( NULL == m_pMysqlConn );

	MYSQL *pConn = mysql_init(NULL);

	m_pMysqlConn = pConn;

	pConn = mysql_real_connect(m_pMysqlConn, 
		m_strHost.c_str(),
		m_strUser.c_str(),
		m_strPwd.c_str(),
		m_strDB.c_str(), 
		m_iPort,
		NULL,
		0);

	if ( !pConn )
	{
		ErrorPrint();
		m_pMysqlConn = NULL;
		return false;
	}

	mysql_set_character_set( pConn, "gb2312");    //设置默认字符集

	mysql_autocommit(pConn, 1);        // 设置为自动提交模式

	MYDB_PRINT("Server major    version : %i\n",   mysql_get_server_version(pConn) );
	MYDB_PRINT("Server proto    version : %i\n",   mysql_get_proto_info(pConn));

	return true;
}

void CMysqlConnection::ReleaseRecordSet( IRecordSet** pcsRecordSet )
{
	MY_ASSERT_RET(*pcsRecordSet);

	delete *pcsRecordSet;
	*pcsRecordSet = NULL;

}


IRecordSet*   CMysqlConnection::PrepareBind( const char* szSql )
{
	//GS_ASSERT_RET_VAL(m_pMysqlConn, false);
	MY_ASSERT_RET_VAL(szSql, NULL);

	if ( !TestConnectAlive() )
		return NULL;

	MYSQL_STMT *pStmt = NULL;
	pStmt = mysql_stmt_init(m_pMysqlConn);
	MYASSERT( NULL != pStmt );

	if ( !pStmt )
	{
		ErrorHandle();
		return NULL;
	}

	if ( 0 != mysql_stmt_prepare(pStmt, szSql,strlen(szSql)) )
	{
		ErrorHandle();
		mysql_stmt_close(pStmt);    // TODO:  需要测试!!!
		pStmt = NULL;
		return NULL;
	}
	//关闭自动提交
	if ( 0 != mysql_autocommit(m_pMysqlConn, 0) )
	{
		ErrorHandle();
		return false;
	}

	// pStmt 由类CMysqlRecordSet释放
	 return new CMysqlRecordSet(m_pMysqlConn, pStmt);
}

bool CMysqlConnection::ExecuteBind( IRecordSet* pcsRecordSet )
{
	//GS_ASSERT_RET_VAL(m_pMysqlConn, false);
	MY_ASSERT_RET_VAL(pcsRecordSet, false);

	if ( !TestConnectAlive() )
		return false;

    //开启自动提交
	if ( 0 != mysql_autocommit(m_pMysqlConn, 1) )
	{
		ErrorHandle();
		return false;
	}

	CMysqlRecordSet *pMysqlRecSet =(CMysqlRecordSet*)pcsRecordSet;

	if ( !pMysqlRecSet->IsBindSuccess() )
	{
		ErrorHandle();
		return false;
	}

	return true;
}

bool CMysqlConnection::ExecuteSql( const char* szSql )
{
	//GS_ASSERT_RET_VAL(m_pMysqlConn, false);
	MY_ASSERT_RET_VAL(szSql, false);
	
	if ( !TestConnectAlive() )
		return false;
    
    int res = mysql_query(m_pMysqlConn, szSql);

	if( 0 != res)
    {
		ErrorHandle();
		return false;
	}

	return true;
}

bool CMysqlConnection::GetLastInsertID( const char* szSeqName, signed __int64& lRowID )
{
	//GS_ASSERT_RET_VAL(m_pMysqlConn, false);
   
	if ( !TestConnectAlive() )
		return false;

	my_ulonglong lInsertID = mysql_insert_id(m_pMysqlConn);
	if( lInsertID == 0 )
	    return false;

	lRowID = lInsertID;

	return true;
}





IRecordSet*  CMysqlConnection::ExecuteQuery( const char* szSql )
{
	MY_ASSERT_RET_VAL(szSql, NULL);

	if ( !TestConnectAlive() )
		return false;

	if ( 0 != mysql_query(m_pMysqlConn,szSql) )
	{
		ErrorHandle();
		return NULL;
	}

	return new CMysqlRecordSet(m_pMysqlConn,NULL);
}



IRecordSet*  CMysqlConnection::ExecutePageQuery( const char* szSql, int iStartRow, int iRowNum )
{

	MY_ASSERT_RET_VAL(szSql, NULL);

	if ( !TestConnectAlive() )
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


bool  CMysqlConnection::BeginTrans( void )
{

	if ( !TestConnectAlive() )
		return false;

	MY_ASSERT_RET_VAL(m_pMysqlConn, false);

	
// 	if ( 0 != mysql_query(m_pMysqlConn,"START TRANSACTION")  )
// 	{
// 		ErrorHandle();
// 		return false;
// 	}

    //关闭自动提交
	if ( 0 != mysql_autocommit(m_pMysqlConn, 0) )
	{
		ErrorHandle();
		return false;
	}

	return true;
}


void  CMysqlConnection::Rollback( void )
{
	MY_ASSERT_RET(m_pMysqlConn);

	// 回滚
	if (  0 != mysql_rollback(m_pMysqlConn) )
	{
		ErrorHandle();
		return ;
	}

	// 设置自动提交
	if ( 0 != mysql_autocommit(m_pMysqlConn, 1) )
	{
		ErrorHandle();
		return ;
	}
}

bool CMysqlConnection::Commit( void )
{
	MY_ASSERT_RET_VAL(m_pMysqlConn, false);

	// 事务提交
	if (  0 != mysql_commit(m_pMysqlConn) )
	{
		ErrorHandle();
		return false;
	}

	//设置自动提交
	if ( 0 != mysql_autocommit(m_pMysqlConn, 1) )
	{
		ErrorHandle();
		return false;
	}

	return true;
}



// 获取错误码
EnumDBApiRet CMysqlConnection::GetErrorCode( void )
{
	if ( !m_pMysqlConn )
		return RETCODE_INITIALIZE_FAIL;
    
	UInt32 iErrCode = mysql_errno(m_pMysqlConn);
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


const char* CMysqlConnection::GetErrorMessage( void )
{
	//MYASSERT( NULL != m_pMysqlConn );
	if ( !m_pMysqlConn )
		return "mysql api initialize fail!";

	const char *pErr = mysql_error(m_pMysqlConn);
	if ( !pErr )
		return "no error.";

    return  pErr;
}


const char*  CMysqlConnection::ToTime(const char* szDateTime)
{

	MY_ASSERT_RET_VAL(szDateTime, NULL);

	memset(m_szDateTime, 0x0, sizeof(m_szDateTime));

	MYSNPRINTF(m_szDateTime, 
		sizeof(m_szDateTime)-1,
		"STR_TO_DATE('%s', '%%H:%%i:%%s')",
		szDateTime);


	return m_szDateTime;
}



const char*  CMysqlConnection::ToDate(const char* szDateTime)
{

	MY_ASSERT_RET_VAL(szDateTime, NULL);

	memset(m_szDateTime, 0x0, sizeof(m_szDateTime));

	MYSNPRINTF(m_szDateTime, 
		sizeof(m_szDateTime)-1,
		"STR_TO_DATE('%s', '%%Y-%%m-%%d')",
		szDateTime);

	return m_szDateTime;
}



const char*  CMysqlConnection::ToDateTime(const char* szDateTime)
{
	MY_ASSERT_RET_VAL(szDateTime, NULL);

	memset(m_szDateTime, 0x0, sizeof(m_szDateTime));

	MYSNPRINTF(m_szDateTime, 
		sizeof(m_szDateTime)-1,
		"STR_TO_DATE('%s', '%%Y-%%m-%%d %%H:%%i:%%s ')", 
		szDateTime);

	return m_szDateTime;
}

const char*  CMysqlConnection::TimeToStr(const char* szDateTime)
{

	MY_ASSERT_RET_VAL(szDateTime, NULL);

	memset(m_szDateTime, 0x0, sizeof(m_szDateTime));

	MYSNPRINTF(m_szDateTime, 
		sizeof(m_szDateTime)-1, 
		"TIME_FORMAT('%s', '%%H:%%i:%%s')",szDateTime);

	return m_szDateTime;
}

const char*  CMysqlConnection::DateToStr(const char* szDateTime)
{
	MY_ASSERT_RET_VAL(szDateTime, NULL);

	memset(m_szDateTime, 0x0, sizeof(m_szDateTime));

	MYSNPRINTF(m_szDateTime, 
		sizeof(m_szDateTime)-1,
		"DATE_FORMAT('%s', '%%Y-%%m-%%d')",szDateTime);

	return m_szDateTime;
}

const char*  CMysqlConnection::DateTimeToStr(const char* szDateTime)
{

	MY_ASSERT_RET_VAL(szDateTime, NULL);

	memset(m_szDateTime, 0x0, sizeof(m_szDateTime));

	MYSNPRINTF(m_szDateTime,
		sizeof(m_szDateTime)-1,
		"DATE_FORMAT('%s', '%%Y-%%m-%%d %%H:%%i:%%s')",szDateTime);

	return m_szDateTime;
}

const char*  CMysqlConnection::GetSysTime( void )
{
	memset(m_szDateTime, 0x0, sizeof(m_szDateTime));
	MYSNPRINTF(m_szDateTime,
		sizeof(m_szDateTime)-1,
		"SELECT CURTIME()");

	IRecordSet *pRescordSet = ExecuteQuery(m_szDateTime);
	if ( !pRescordSet )
	{
		ErrorHandle();
		return NULL;
	}

	memset(m_szDateTime, 0x0, sizeof(m_szDateTime));

	unsigned int iFactLen = 0;
	if ( !pRescordSet->GetValue( 1, m_szDateTime, sizeof(m_szDateTime), &iFactLen ) )
	{
		ReleaseRecordSet(&pRescordSet);
		ErrorHandle();
		return NULL;
	}

	ReleaseRecordSet(&pRescordSet);

	return m_szDateTime;
	
}

const char*  CMysqlConnection::GetSysDate( void )
{

	memset(m_szDateTime, 0x0, sizeof(m_szDateTime));
	MYSNPRINTF(m_szDateTime,
		sizeof(m_szDateTime)-1,
		"SELECT CURDATE()");
    
	IRecordSet *pRescordSet = ExecuteQuery(m_szDateTime);
	if ( !pRescordSet )
	{
		ErrorHandle();
		return false;
	}

	memset(m_szDateTime, 0x0, sizeof(m_szDateTime));

	unsigned int iFactLen = 0;
	if ( !pRescordSet->GetValue( 1, m_szDateTime, sizeof(m_szDateTime), &iFactLen ) )
	{
		ReleaseRecordSet(&pRescordSet);
		ErrorHandle();
		return NULL;
	}

	ReleaseRecordSet(&pRescordSet);

	return m_szDateTime;
}

const char*  CMysqlConnection::GetSysDateTime( void )
{
	memset(m_szDateTime, 0x0, sizeof(m_szDateTime));
	MYSNPRINTF(m_szDateTime,
		sizeof(m_szDateTime)-1,
		"SELECT NOW()");

	IRecordSet *pRescordSet = ExecuteQuery(m_szDateTime);
	if ( !pRescordSet )
	{
		ErrorHandle();
		return false;
	}

	memset(m_szDateTime, 0x0, sizeof(m_szDateTime));

	unsigned int iFactLen = 0;
	if ( !pRescordSet->GetValue( 1, m_szDateTime, sizeof(m_szDateTime), &iFactLen ) )
	{
		ReleaseRecordSet(&pRescordSet);
		ErrorHandle();
		return NULL;
	}

	ReleaseRecordSet(&pRescordSet);

	return m_szDateTime;
}

// 关闭连接
void  CMysqlConnection::DisconnectDB( void )
{
	if ( m_pMysqlConn )
	{
		mysql_close(m_pMysqlConn);
		m_pMysqlConn = NULL;
	}

}

// 是否重连
bool CMysqlConnection::IsReconnect( void )
{
	// 若为网络连接错误则重连
	if ( GetErrorCode() == RETCODE_NETWORK_FAIL_CONNECT )
		return true;

	return false;
}

// 重新连接（由内部保证重连）
bool CMysqlConnection::ReconnectDB( void )
{
	MYDB_PRINT("TRY RECONNECT DB( %s )... \n", m_strDB.c_str());
	DisconnectDB();
	return ConnectDB();
}

void CMysqlConnection::ErrorHandle( void )
{
	// 错误信息打印
	ErrorPrint();

	// 重连
	if ( IsReconnect() )
		ReconnectDB();
}

// 错误信息打印
void  CMysqlConnection::ErrorPrint( void )
{   
	MY_ASSERT_RET( NULL != m_pMysqlConn );

	MYDB_PRINT( "MYSQL ERROR :\n"
		"CODE : %d\n"
		"MSG  : %s\n",
		mysql_errno(m_pMysqlConn),
		mysql_error(m_pMysqlConn)
		);
}

// 测试连接状态
bool CMysqlConnection::TestConnectAlive( void )
{
	if( !m_pMysqlConn)
		if ( !ReconnectDB() )
			return false;

	return true;
}
