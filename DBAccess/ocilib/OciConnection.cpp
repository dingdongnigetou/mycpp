
#include <sstream>

#include "OciConnection.h"
#include "OciRecordSet.h"

UInt16 COciConnection::m_iInitRefs = 0; 

COciConnection::COciConnection( const std::string& strHost,
							   const std::string& strDataBase,
							   const std::string& strUserName,
							   const std::string& strPassword,
							   UInt16 iPort )
:m_strUser(strUserName)
,m_strPwd(strPassword)
,m_pOciConn(NULL)
,m_pOciErr(NULL)
{
	if ( iPort == 0 )
		iPort = 1521;    // oracleĬ�϶˿�

	// 172.7.14.6:1521/ORCL
	std::ostringstream ssOciDB;
	ssOciDB.str("");
	if ( !strHost.empty() )
		ssOciDB<<strHost;
	else
		ssOciDB<<"localhost";

	ssOciDB<<":"<<iPort<<"/"<<strDataBase;

	m_strDB = ssOciDB.str();

	Initialize();

	memset(m_szDateTime, 0x0, sizeof(m_szDateTime));
}

COciConnection::~COciConnection( void )
{
	DisconnectDB();
	Cleanup();
}

bool COciConnection::Initialize( void )
{
	if ( m_iInitRefs == 0 )
	{
		if ( !OCI_Initialize(NULL,
			NULL, 
			OCI_ENV_DEFAULT|OCI_ENV_CONTEXT|OCI_ENV_THREADED) )
		{
			MYDB_PRINT("OCI_Initialize() fail! Please check oci dll... \n");
			return false;
		}

		OCI_EnableWarnings(TRUE);
	}

	m_iInitRefs ++;

	return true;
}

void COciConnection::Cleanup( void )
{
	m_iInitRefs --;

	if ( m_iInitRefs == 0 )
	{
		OCI_Cleanup();
	}
}

bool COciConnection::ConnectDB( void )
{
	// ocilibδ��ʼ���򷵻�ʧ��
	if ( m_iInitRefs == 0 )
		return false;

	MYASSERT(m_pOciConn==NULL);

	OCI_Connection *pConn = NULL;
	pConn = OCI_ConnectionCreate( m_strDB.c_str(),
		m_strUser.c_str(),
		m_strPwd.c_str(),
		OCI_SESSION_DEFAULT );
	if ( !pConn )
	{
		SetLastError();
		ErrorPrint();
		return false;
	}

	MYDB_PRINT("Server major    version : %i\n",   OCI_GetServerMajorVersion(pConn));
	MYDB_PRINT("Server minor    version : %i\n",   OCI_GetServerMinorVersion(pConn));
	MYDB_PRINT("Server revision version : %i\n\n", OCI_GetServerRevisionVersion(pConn));
	MYDB_PRINT("Connection      version : %i\n\n", OCI_GetVersionConnection(pConn));

	// ����Ϊ�Զ��ύģʽ
	OCI_SetAutoCommit(pConn, TRUE);

	m_pOciConn = pConn;

	return true;
}

void COciConnection::DisconnectDB( void )
{
	if ( m_pOciConn )
	{
		OCI_ConnectionFree(m_pOciConn);
		m_pOciConn = NULL;
	}
}

void COciConnection::ReleaseRecordSet( IRecordSet** pcsRecordSet )
{
	MYASSERT(*pcsRecordSet);
	
	if ( *pcsRecordSet )
	{
		delete *pcsRecordSet;
		*pcsRecordSet = NULL;
	}
}

IRecordSet* COciConnection::PrepareBind( const char* szSql )
{
	DB_POINTER_CHECK_RET(szSql, NULL);
	//DB_POINTER_CHECK_RET(m_pOciConn,NULL);
	if ( !TestConnectAlive() )
		return NULL;

	// ����SQL���
	std::string strSql("");
	if ( !MakeBindSql(szSql, strSql) )
		return NULL;

 
	OCI_Statement *pStmt = NULL;
	pStmt = OCI_StatementCreate(m_pOciConn);
	MYASSERT(pStmt!=NULL);

	if ( !pStmt )
	{
		ErrorHandle();
		return NULL;
	}

	if ( !OCI_Prepare(pStmt, strSql.c_str()) )
	{
		ErrorHandle();
		SafeToFreeStatement(&pStmt);
		return NULL;
	}

	// pStmt ����COciRecordSet�ͷ�
	return  (new COciRecordSet(pStmt,this));
}

bool COciConnection::ExecuteBind( IRecordSet* pcsRecordSet )
{
	//DB_POINTER_CHECK_RET(m_pOciConn, false);
	DB_POINTER_CHECK_RET(pcsRecordSet, false);

	if ( !TestConnectAlive() )
		return false;

	COciRecordSet *pOciRecSet =(COciRecordSet*)pcsRecordSet;

	if ( !OCI_Execute(pOciRecSet->GetOCIStatement()) )
	{
		ErrorHandle();
		return false;
	}

	return true;
}

bool COciConnection::ExecuteSql( const char* szSql )
{
	DB_POINTER_CHECK_RET(szSql, false);
	//DB_POINTER_CHECK_RET(m_pOciConn, false);
	if ( !TestConnectAlive() )
		return false;

	OCI_Statement *pStmt = NULL;
	pStmt = OCI_StatementCreate(m_pOciConn);
	MYASSERT(pStmt!=NULL);

	if ( !pStmt )
	{
		ErrorHandle();
		return false;
	}

	if ( !OCI_ExecuteStmt(pStmt, szSql) )
	{
		ErrorHandle();
		SafeToFreeStatement(&pStmt);
		return false;
	}
	SafeToFreeStatement(&pStmt);
	return true;
}

bool COciConnection::GetLastInsertID( const char* szSeqName, signed __int64& lRowID )
{
	DB_POINTER_CHECK_RET(szSeqName, false);
	//DB_POINTER_CHECK_RET(m_pOciConn, false);
	if ( !TestConnectAlive() )
		return false;

	otext szSql[128] = {0};
	MYSNPRINTF(szSql, sizeof(szSql) - 1, "SELECT %s.CURRVAL AS ID FROM DUAL", szSeqName);

	IRecordSet *pRecSet = ExecuteQuery(szSql);
	DB_POINTER_CHECK_RET(pRecSet, false);

	if ( !pRecSet->GetValue("ID", &lRowID, sizeof(lRowID), NULL, DT_INT64) )
	{
		ReleaseRecordSet(&pRecSet);
		ErrorHandle();
		return false;
	}

	ReleaseRecordSet(&pRecSet);

	return true;
}

IRecordSet* COciConnection::ExecuteQuery( const char* szSql )
{
	DB_POINTER_CHECK_RET(szSql, NULL);
	//DB_POINTER_CHECK_RET(m_pOciConn, NULL);
	if ( !TestConnectAlive() )
		return false;

	OCI_Statement *pStmt = NULL;
	pStmt = OCI_StatementCreate(m_pOciConn);
	MYASSERT(pStmt!=NULL);

	if ( !pStmt )
	{
		ErrorHandle();
		return NULL;
	}

	if ( !OCI_ExecuteStmt(pStmt, szSql) )
	{
		ErrorHandle();
		SafeToFreeStatement(&pStmt);
		return NULL;
	}

	// pStmt ����COciRecordSet�ͷ�
	return  (new COciRecordSet(pStmt,this));
}

IRecordSet* COciConnection::ExecutePageQuery( const char* szSql, int iStartRow, int iRowNum )
{
	DB_POINTER_CHECK_RET(szSql, NULL);
	//DB_POINTER_CHECK_RET(m_pOciConn, NULL);
	if ( !TestConnectAlive() )
		return false;

	// �ϳ�����SQL���
	std::ostringstream ssFullSql;

	//���SQLǰ�����
	ssFullSql << "SELECT  * FROM (SELECT ROWNUM R,GS_SELECT.* FROM ( ";

	ssFullSql << szSql;
	//���SQL�������
	ssFullSql << " ) GS_SELECT WHERE ROWNUM < ";
	ssFullSql <<  iStartRow + iRowNum; 
	ssFullSql << " ) WHERE R >= ";
	ssFullSql << iStartRow;

	return ExecuteQuery(ssFullSql.str().c_str());
}

bool COciConnection::BeginTrans( void )
{
	if ( !TestConnectAlive() )
		return false;

	DB_POINTER_CHECK_RET(m_pOciConn, false);

	// �����ֶ��ύ
	if ( !OCI_SetAutoCommit(m_pOciConn, FALSE) )
	{
		ErrorHandle();
		return false;
	}

	return true;
}

void COciConnection::Rollback( void )
{
	//MYASSERT(m_pOciConn!=NULL);
	if ( !m_pOciConn )
		return ;

	// ����ع�
	if ( !OCI_Rollback(m_pOciConn) )
	{
		ErrorHandle();
		return ;
	}

	// ����Ϊ�Զ��ύ
	if ( !OCI_SetAutoCommit(m_pOciConn, TRUE) )
	{
		ErrorHandle();
		return ;
	}
}

bool COciConnection::Commit( void )
{
	//DB_POINTER_CHECK_RET(m_pOciConn, false);
	if ( !m_pOciConn )
		return false;

	// �����ύ
	if ( !OCI_Commit(m_pOciConn) )
	{
		ErrorHandle();
		return false;
	}

	// ����Ϊ�Զ��ύ
	if ( !OCI_SetAutoCommit(m_pOciConn, TRUE) )
	{
		ErrorHandle();
		return false;
	}

	return true;
}

EnumDBApiRet COciConnection::GetErrorCode( void )
{
	if ( m_iInitRefs == 0 )
		return RETCODE_INITIALIZE_FAIL;

	//MYASSERT(m_pOciErr!=NULL);
	if ( !m_pOciErr )
		return RETCODE_SUCCESS;

	int iErrCode = OCI_ErrorGetOCICode(m_pOciErr);
	switch ( iErrCode )
	{
	case 1:    // Υ��ΨһԼ������ (.)
		return RETCODE_UNIQUE_CONSTRAINT_VIOLATED;
	case 18:   //�������Ự��
	case 19:   //�������Ự�����
	case 20:   //������������ ()
		return RETCODE_OVER_MAXLINK;
	case 132:  //�﷨������޷��������������� ''
		return RETCODE_SQL_SYNTAX_ERROR;
	case 987:  //ȱ�ٻ���Ч�û���
	case 988:  //ȱ�ٻ���Ч����
	case 989:  //�������û����������
	case 990:  //ȱ�ٻ���ЧȨ��
		return RETCODE_USERNAME_PASSWORD_ERROR;

	case 3114:  //δ���ӵ�ORACLE
		return RETCODE_NETWORK_FAIL_CONNECT;

	default:
		break;
	}

	return RETCODE_UNKNOWN_ERROR;
}

const char* COciConnection::GetErrorMessage( void )
{
	if ( m_iInitRefs == 0 )
		return "ocilib initialize fail!";

	//MYASSERT(m_pOciErr!=NULL);
	if ( !m_pOciErr )
		return "no error.";

	return OCI_ErrorGetString(m_pOciErr);
}

const char* COciConnection::ToTime( const char* szDateTime )
{
	DB_POINTER_CHECK_RET(szDateTime, NULL);

	memset(m_szDateTime, 0x0, sizeof(m_szDateTime));

	MYSNPRINTF(m_szDateTime, 
		sizeof(m_szDateTime)-1,
		"TO_DATE('%s', 'HH24:Mi:SS')",
		szDateTime);

	return m_szDateTime;
}

const char* COciConnection::ToDate( const char* szDateTime )
{
	DB_POINTER_CHECK_RET(szDateTime, NULL);

	memset(m_szDateTime, 0x0, sizeof(m_szDateTime));

	MYSNPRINTF(m_szDateTime, 
		sizeof(m_szDateTime)-1,
		"TO_DATE('%s', 'YYYY-MM-DD')",
		szDateTime);

	return m_szDateTime;
}

const char* COciConnection::ToDateTime( const char* szDateTime )
{
	DB_POINTER_CHECK_RET(szDateTime, NULL);

	memset(m_szDateTime, 0x0, sizeof(m_szDateTime));

	MYSNPRINTF(m_szDateTime, 
		sizeof(m_szDateTime)-1,
		"TO_DATE('%s', 'YYYY-MM-DD HH24:Mi:SS')", 
		szDateTime);

	return m_szDateTime;
}

const char* COciConnection::TimeToStr( const char* szDateTime )
{
	DB_POINTER_CHECK_RET(szDateTime, NULL);

	memset(m_szDateTime, 0x0, sizeof(m_szDateTime));

	MYSNPRINTF(m_szDateTime, 
		sizeof(m_szDateTime)-1, 
		"TO_CHAR('%s', 'HH24:Mi:SS')",
		szDateTime);

	return m_szDateTime;
}

const char* COciConnection::DateToStr( const char* szDateTime )
{
	DB_POINTER_CHECK_RET(szDateTime, NULL);

	memset(m_szDateTime, 0x0, sizeof(m_szDateTime));

	MYSNPRINTF(m_szDateTime, 
		sizeof(m_szDateTime)-1,
		"TO_CHAR('%s', 'YYYY-MM-DD')", 
		szDateTime);

	return m_szDateTime;
}

const char* COciConnection::DateTimeToStr( const char* szDateTime )
{
	DB_POINTER_CHECK_RET(szDateTime, NULL);

	memset(m_szDateTime, 0x0, sizeof(m_szDateTime));

	MYSNPRINTF(m_szDateTime,
		sizeof(m_szDateTime)-1,
		"TO_CHAR('%s', 'YYYY-MM-DD HH24:Mi:SS')", 
		szDateTime);

	return m_szDateTime;
}

const char* COciConnection::GetSysTime( void )
{
	memset(m_szDateTime, 0x0, sizeof(m_szDateTime));

	MYSNPRINTF(m_szDateTime,
		sizeof(m_szDateTime)-1,
		"SELECT TO_CHAR(sysdate, 'HH24:Mi:SS') FROM DUAL");

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

const char* COciConnection::GetSysDate( void )
{
	memset(m_szDateTime, 0x0, sizeof(m_szDateTime));

	MYSNPRINTF(m_szDateTime,
		sizeof(m_szDateTime)-1,
		"SELECT TO_CHAR(sysdate, 'YYYY-MM-DD') FROM DUAL");

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

const char* COciConnection::GetSysDateTime( void )
{   
	memset(m_szDateTime, 0x0, sizeof(m_szDateTime));

	MYSNPRINTF(m_szDateTime,
		sizeof(m_szDateTime)-1,
		"SELECT TO_CHAR(sysdate, 'YYYY-MM-DD HH24:Mi:SS') FROM DUAL");

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

bool COciConnection::IsReconnect( void )
{
	// ��Ϊ�������Ӵ���������
	if ( GetErrorCode() == RETCODE_NETWORK_FAIL_CONNECT )
		return true;

	return false;
}

bool COciConnection::ReconnectDB( void )
{
	MYDB_PRINT("TRY RECONNECT DB( %s )... \n", m_strDB.c_str());
	DisconnectDB();
	return ConnectDB();
}

void COciConnection::ErrorHandle( void )
{
	// ���������Ϣ	
	SetLastError();

	// ������Ϣ��ӡ
	ErrorPrint();
	
	// ����
	if ( IsReconnect() )
		ReconnectDB();
}

void COciConnection::ErrorPrint( void )
{
	MYDB_PRINT( "OCI %s INFO:\n"
		"CODE : ORA-%05i\n"
		"MSG  : %s\n"
		"SQL  : %s\n",
		(OCI_ErrorGetType(m_pOciErr)==OCI_ERR_WARNING?"WARNING":"ERROR"),
		OCI_ErrorGetOCICode(m_pOciErr),
		OCI_ErrorGetString(m_pOciErr),
		OCI_GetSql(OCI_ErrorGetStatement(m_pOciErr)) );
}

void COciConnection::SetLastError( void )
{
	m_pOciErr = OCI_GetLastError();
	MYASSERT(m_pOciErr!=NULL);
}

bool COciConnection::MakeBindSql( const char *szSrcSql, std::string& strDstSql )
{
	std::ostringstream strSql;
	strSql.str("");
	int iIndex=0;
	while ( *szSrcSql!='\0' )
	{
		if ( *szSrcSql == '?' )
		{
			strSql<<":"<<iIndex;
			iIndex ++;
		}
		else
		{
			strSql<<(*szSrcSql);
		}

		szSrcSql ++;
		
	}

	strDstSql = strSql.str();

	return true;
}

bool COciConnection::TestConnectAlive( void )
{
	if ( !m_pOciConn )
		if ( !ReconnectDB() )
			return false;

	return true;
}

void COciConnection::SafeToFreeStatement( OCI_Statement **pStmt )
{
	if ( !IsReconnect() && m_pOciConn )   // ���������󣬲����ͷ�pStmt
		OCI_StatementFree(*pStmt);

	*pStmt = NULL;
} 