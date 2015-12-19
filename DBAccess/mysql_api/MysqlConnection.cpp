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
,m_iPort(iPort)   // Ĭ����0
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

	mysql_set_character_set( pConn, "gb2312");    //����Ĭ���ַ���

	mysql_autocommit(pConn, 1);        // ����Ϊ�Զ��ύģʽ

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
		mysql_stmt_close(pStmt);    // TODO:  ��Ҫ����!!!
		pStmt = NULL;
		return NULL;
	}
	//�ر��Զ��ύ
	if ( 0 != mysql_autocommit(m_pMysqlConn, 0) )
	{
		ErrorHandle();
		return false;
	}

	// pStmt ����CMysqlRecordSet�ͷ�
	 return new CMysqlRecordSet(m_pMysqlConn, pStmt);
}

bool CMysqlConnection::ExecuteBind( IRecordSet* pcsRecordSet )
{
	//GS_ASSERT_RET_VAL(m_pMysqlConn, false);
	MY_ASSERT_RET_VAL(pcsRecordSet, false);

	if ( !TestConnectAlive() )
		return false;

    //�����Զ��ύ
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

    //�ر��Զ��ύ
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

	// �ع�
	if (  0 != mysql_rollback(m_pMysqlConn) )
	{
		ErrorHandle();
		return ;
	}

	// �����Զ��ύ
	if ( 0 != mysql_autocommit(m_pMysqlConn, 1) )
	{
		ErrorHandle();
		return ;
	}
}

bool CMysqlConnection::Commit( void )
{
	MY_ASSERT_RET_VAL(m_pMysqlConn, false);

	// �����ύ
	if (  0 != mysql_commit(m_pMysqlConn) )
	{
		ErrorHandle();
		return false;
	}

	//�����Զ��ύ
	if ( 0 != mysql_autocommit(m_pMysqlConn, 1) )
	{
		ErrorHandle();
		return false;
	}

	return true;
}



// ��ȡ������
EnumDBApiRet CMysqlConnection::GetErrorCode( void )
{
	if ( !m_pMysqlConn )
		return RETCODE_INITIALIZE_FAIL;
    
	UInt32 iErrCode = mysql_errno(m_pMysqlConn);
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

// �ر�����
void  CMysqlConnection::DisconnectDB( void )
{
	if ( m_pMysqlConn )
	{
		mysql_close(m_pMysqlConn);
		m_pMysqlConn = NULL;
	}

}

// �Ƿ�����
bool CMysqlConnection::IsReconnect( void )
{
	// ��Ϊ�������Ӵ���������
	if ( GetErrorCode() == RETCODE_NETWORK_FAIL_CONNECT )
		return true;

	return false;
}

// �������ӣ����ڲ���֤������
bool CMysqlConnection::ReconnectDB( void )
{
	MYDB_PRINT("TRY RECONNECT DB( %s )... \n", m_strDB.c_str());
	DisconnectDB();
	return ConnectDB();
}

void CMysqlConnection::ErrorHandle( void )
{
	// ������Ϣ��ӡ
	ErrorPrint();

	// ����
	if ( IsReconnect() )
		ReconnectDB();
}

// ������Ϣ��ӡ
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

// ��������״̬
bool CMysqlConnection::TestConnectAlive( void )
{
	if( !m_pMysqlConn)
		if ( !ReconnectDB() )
			return false;

	return true;
}
