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

	mysql_set_character_set( pConn, "gb2312");    //����Ĭ���ַ���

	mysql_autocommit(pConn, 1);        // ����Ϊ�Զ��ύģʽ

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
		mysql_stmt_close(pStmt);    // TODO:  ��Ҫ����!!!
		pStmt = nullptr;
		return nullptr;
	}
	//�ر��Զ��ύ
	if ( 0 != mysql_autocommit(pMysqlConn_, 0) )
	{
		errorHandle();
		return false;
	}

	// pStmt ����CMysqlRecordSet�ͷ�
	 return new CMysqlRecordSet(pMysqlConn_, pStmt);
}

bool CMysqlConnection::ExecuteBind( IRecordSet* pcsRecordSet )
{
	MY_ASSERT_RET_VAL(pcsRecordSet, false);

	if ( !testConnectAlive() )
		return false;

    //�����Զ��ύ
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

	// �ϳ�����SQL���
	std::ostringstream ssFullSql;

	ssFullSql << szSql;
	//���SQL�������
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

    //�ر��Զ��ύ
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

	// �ع�
	if (  0 != mysql_rollback(pMysqlConn_) )
	{
		errorHandle();
		return ;
	}

	// �����Զ��ύ
	if ( 0 != mysql_autocommit(pMysqlConn_, 1) )
	{
		errorHandle();
		return ;
	}
}

bool CMysqlConnection::Commit()
{
	MY_ASSERT_RET_VAL(pMysqlConn_, false);

	// �����ύ
	if (  0 != mysql_commit(pMysqlConn_) )
	{
		errorHandle();
		return false;
	}

	//�����Զ��ύ
	if ( 0 != mysql_autocommit(pMysqlConn_, 1) )
	{
		errorHandle();
		return false;
	}

	return true;
}

// ��ȡ������
EnumDBApiRet CMysqlConnection::GetErrorCode()
{
	if ( !pMysqlConn_ )
		return RETCODE_INITIALIZE_FAIL;
    
	auto iErrCode = mysql_errno(pMysqlConn_);
	switch ( iErrCode )
	{
	case ER_ACCESS_DENIED_ERROR: //�����������ݿ⣬�û������������
	case ER_DBACCESS_DENIED_ERROR:  //��ǰ�û�û�з������ݿ��Ȩ��
		return RETCODE_USERNAME_PASSWORD_ERROR;
		
	case ER_CON_COUNT_ERROR: // ���ӹ���
	case ER_TOO_MANY_USER_CONNECTIONS: // �û�%s�����˳���'max_user_connections'�Ļ����
		return RETCODE_OVER_MAXLINK;

	case  ER_SYNTAX_ERROR: // ����SQL�﷨����
		return RETCODE_SQL_SYNTAX_ERROR;

	case ER_DUP_UNIQUE: // Ψһ������
		return RETCODE_UNIQUE_CONSTRAINT_VIOLATED;

	//δ���ӵ�mysql
	case ER_NET_READ_ERROR: //������󣬳��ֶ�����������������״��
	case ER_NET_READ_INTERRUPTED: //������󣬶���ʱ��������������״��
	case ER_NET_ERROR_ON_WRITE: //������󣬳���д����������������״��
	case ER_NET_WRITE_INTERRUPTED: //�������д��ʱ��������������״��
	case CR_SERVER_GONE_ERROR: //2006 MySQL������������		
	case CR_SERVER_LOST: //2013 ��ѯ�����ж�ʧ����MySQL������������
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

// �ر�����
void  CMysqlConnection::disconnectDB()
{
	if ( pMysqlConn_ )
	{
		mysql_close(pMysqlConn_);
		pMysqlConn_ = nullptr;
	}

}

// �Ƿ�����
bool CMysqlConnection::isReconnect()
{
	// ��Ϊ�������Ӵ���������
	if ( GetErrorCode() == RETCODE_NETWORK_FAIL_CONNECT )
		return true;

	return false;
}

// �������ӣ����ڲ���֤������
bool CMysqlConnection::reconnectDB()
{
	MYDB_PRINT("TRY RECONNECT DB( %s )... \n", strDB_.c_str());
	disconnectDB();
	return ConnectDB();
}

void CMysqlConnection::errorHandle()
{
	// ������Ϣ��ӡ
	errorPrint();

	// ����
	if ( isReconnect() )
		reconnectDB();
}

// ������Ϣ��ӡ
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

// ��������״̬
bool CMysqlConnection::testConnectAlive()
{
	if( !pMysqlConn_)
		if ( !reconnectDB() )
			return false;

	return true;
}
