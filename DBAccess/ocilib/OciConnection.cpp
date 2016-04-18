
#include <sstream>

#include "OciConnection.h"
#include "OciRecordSet.h"

UInt16 COciConnection::iInitRefs_ = 0; 

COciConnection::COciConnection( const std::string& strHost,
							   const std::string& strDataBase,
							   const std::string& strUserName,
							   const std::string& strPassword,
							   UInt16 iPort )
:strUser_(strUserName)
,strPwd_(strPassword)
{
	if (0 == iPort)
		iPort = 1521;

	std::ostringstream ssOciDB;
	ssOciDB.str("");
	if ( !strHost.empty() )
		ssOciDB<<strHost;
	else
		ssOciDB<<"localhost";

	ssOciDB<<":"<<iPort<<"/"<<strDataBase;

	strDB_ = ssOciDB.str();

	initialize();

	memset(m_szDateTime, 0x0, sizeof(m_szDateTime));
}

COciConnection::~COciConnection()
{
	disconnectDB();
	cleanup();
}

bool COciConnection::initialize()
{
	if ( iInitRefs_ == 0 )
	{
		if ( !OCI_Initialize(nullptr,
			nullptr, 
			OCI_ENV_DEFAULT|OCI_ENV_CONTEXT|OCI_ENV_THREADED) )
		{
			MYDB_PRINT("OCI_Initialize() fail! Please check oci dll... \n");
			return false;
		}

		OCI_EnableWarnings(true);
	}

	iInitRefs_ ++;

	return true;
}

void COciConnection::cleanup()
{
	iInitRefs_ --;

	if ( iInitRefs_ == 0 )
	{
		OCI_Cleanup();
	}
}

bool COciConnection::ConnectDB()
{
	// ocilibδ��ʼ���򷵻�ʧ��
	if ( iInitRefs_ == 0 )
		return false;

	MYASSERT(pOciConn_==nullptr);

	OCI_Connection *pConn = nullptr;
	pConn = OCI_ConnectionCreate( strDB_.c_str(),
		strUser_.c_str(),
		strPwd_.c_str(),
		OCI_SESSION_DEFAULT );
	if ( !pConn )
	{
		SetLastError();
		errorPrint();
		return false;
	}

	MYDB_PRINT("OCI version : %i\n",   OCI_GetServerMajorVersion(pConn));

	// ����Ϊ�Զ��ύģʽ
	OCI_SetAutoCommit(pConn, true);

	pOciConn_ = pConn;

	return true;
}

void COciConnection::disconnectDB()
{
	if ( pOciConn_ )
	{
		OCI_ConnectionFree(pOciConn_);
		pOciConn_ = nullptr;
	}
}

void COciConnection::ReleaseRecordSet( IRecordSet** pcsRecordSet )
{
	MYASSERT(*pcsRecordSet);
	
	if ( *pcsRecordSet )
	{
		delete *pcsRecordSet;
		*pcsRecordSet = nullptr;
	}
}

IRecordSet* COciConnection::PrepareBind( const char* szSql )
{
	DB_POINTER_CHECK_RET(szSql, nullptr);
	if ( !testConnectAlive() )
		return nullptr;

	// ����SQL���
	std::string strSql("");
	if ( !makeBindSql(szSql, strSql) )
		return nullptr;

	OCI_Statement *pStmt = nullptr;
	pStmt = OCI_StatementCreate(pOciConn_);
	MYASSERT(pStmt!=nullptr);

	if ( !pStmt )
	{
		errorHandle();
		return nullptr;
	}

	if ( !OCI_Prepare(pStmt, strSql.c_str()) )
	{
		errorHandle();
		safeToFreeStatement(&pStmt);
		return nullptr;
	}

	// pStmt ����COciRecordSet�ͷ�
	return  (new COciRecordSet(pStmt,this));
}

bool COciConnection::ExecuteBind( IRecordSet* pcsRecordSet )
{
	DB_POINTER_CHECK_RET(pcsRecordSet, false);

	if ( !testConnectAlive() )
		return false;

	auto pOciRecSet =(COciRecordSet*)pcsRecordSet;

	if ( !OCI_Execute(pOciRecSet->GetOCIStatement()) )
	{
		errorHandle();
		return false;
	}

	return true;
}

bool COciConnection::ExecuteSql( const char* szSql )
{
	DB_POINTER_CHECK_RET(szSql, false);
	if ( !testConnectAlive() )
		return false;

	OCI_Statement *pStmt = nullptr;
	pStmt = OCI_StatementCreate(pOciConn_);
	MYASSERT(pStmt!=nullptr);

	if ( !pStmt )
	{
		errorHandle();
		return false;
	}

	if ( !OCI_ExecuteStmt(pStmt, szSql) )
	{
		errorHandle();
		safeToFreeStatement(&pStmt);
		return false;
	}
	safeToFreeStatement(&pStmt);
	return true;
}

bool COciConnection::GetLastInsertID( const char* szSeqName, signed __int64& lRowID )
{
	DB_POINTER_CHECK_RET(szSeqName, false);
	if ( !testConnectAlive() )
		return false;

	otext szSql[128] = {0};
	MYSNPRINTF(szSql, sizeof(szSql) - 1, "SELECT %s.CURRVAL AS ID FROM DUAL", szSeqName);

	auto pRecSet = ExecuteQuery(szSql);
	DB_POINTER_CHECK_RET(pRecSet, false);

	if ( !pRecSet->GetValue("ID", &lRowID, sizeof(lRowID), nullptr, DT_INT64) )
	{
		ReleaseRecordSet(&pRecSet);
		errorHandle();
		return false;
	}

	ReleaseRecordSet(&pRecSet);

	return true;
}

IRecordSet* COciConnection::ExecuteQuery( const char* szSql )
{
	DB_POINTER_CHECK_RET(szSql, nullptr);
	if ( !testConnectAlive() )
		return false;

	OCI_Statement *pStmt = nullptr;
	pStmt = OCI_StatementCreate(pOciConn_);
	MYASSERT(pStmt!=nullptr);

	if ( !pStmt )
	{
		errorHandle();
		return nullptr;
	}

	if ( !OCI_ExecuteStmt(pStmt, szSql) )
	{
		errorHandle();
		safeToFreeStatement(&pStmt);
		return nullptr;
	}

	// pStmt ����COciRecordSet�ͷ�
	return  (new COciRecordSet(pStmt,this));
}

IRecordSet* COciConnection::ExecutePageQuery( const char* szSql, int iStartRow, int iRowNum )
{
	DB_POINTER_CHECK_RET(szSql, nullptr);
	if ( !testConnectAlive() )
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

bool COciConnection::BeginTrans()
{
	if ( !testConnectAlive() )
		return false;

	DB_POINTER_CHECK_RET(pOciConn_, false);

	// �����ֶ��ύ
	if ( !OCI_SetAutoCommit(pOciConn_, false) )
	{
		errorHandle();
		return false;
	}

	return true;
}

void COciConnection::Rollback()
{
	//MYASSERT(pOciConn_!=nullptr);
	if ( !pOciConn_ )
		return ;

	// ����ع�
	if ( !OCI_Rollback(pOciConn_) )
	{
		errorHandle();
		return ;
	}

	// ����Ϊ�Զ��ύ
	if ( !OCI_SetAutoCommit(pOciConn_, true) )
	{
		errorHandle();
		return ;
	}
}

bool COciConnection::Commit()
{
	//DB_POINTER_CHECK_RET(pOciConn_, false);
	if ( !pOciConn_ )
		return false;

	// �����ύ
	if ( !OCI_Commit(pOciConn_) )
	{
		errorHandle();
		return false;
	}

	// ����Ϊ�Զ��ύ
	if ( !OCI_SetAutoCommit(pOciConn_, true) )
	{
		errorHandle();
		return false;
	}

	return true;
}

EnumDBApiRet COciConnection::GetErrorCode()
{
	if ( iInitRefs_ == 0 )
		return RETCODE_INITIALIZE_FAIL;

	//MYASSERT(pOciErr_!=nullptr);
	if ( !pOciErr_ )
		return RETCODE_SUCCESS;

	int iErrCode = OCI_ErrorGetOCICode(pOciErr_);
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

const char* COciConnection::GetErrorMessage()
{
	if ( iInitRefs_ == 0 )
		return "ocilib initialize fail!";

	if ( !pOciErr_ )
		return "no error.";

	return OCI_ErrorGetString(pOciErr_);
}

const char* COciConnection::ToTime( const char* szDateTime )
{
	DB_POINTER_CHECK_RET(szDateTime, nullptr);

	memset(m_szDateTime, 0x0, sizeof(m_szDateTime));

	MYSNPRINTF(m_szDateTime, 
		sizeof(m_szDateTime)-1,
		"TO_DATE('%s', 'HH24:Mi:SS')",
		szDateTime);

	return m_szDateTime;
}

const char* COciConnection::ToDate( const char* szDateTime )
{
	DB_POINTER_CHECK_RET(szDateTime, nullptr);

	memset(m_szDateTime, 0x0, sizeof(m_szDateTime));

	MYSNPRINTF(m_szDateTime, 
		sizeof(m_szDateTime)-1,
		"TO_DATE('%s', 'YYYY-MM-DD')",
		szDateTime);

	return m_szDateTime;
}

const char* COciConnection::ToDateTime( const char* szDateTime )
{
	DB_POINTER_CHECK_RET(szDateTime, nullptr);

	memset(m_szDateTime, 0x0, sizeof(m_szDateTime));

	MYSNPRINTF(m_szDateTime, 
		sizeof(m_szDateTime)-1,
		"TO_DATE('%s', 'YYYY-MM-DD HH24:Mi:SS')", 
		szDateTime);

	return m_szDateTime;
}

const char* COciConnection::TimeToStr( const char* szDateTime )
{
	DB_POINTER_CHECK_RET(szDateTime, nullptr);

	memset(m_szDateTime, 0x0, sizeof(m_szDateTime));

	MYSNPRINTF(m_szDateTime, 
		sizeof(m_szDateTime)-1, 
		"TO_CHAR('%s', 'HH24:Mi:SS')",
		szDateTime);

	return m_szDateTime;
}

const char* COciConnection::DateToStr( const char* szDateTime )
{
	DB_POINTER_CHECK_RET(szDateTime, nullptr);

	memset(m_szDateTime, 0x0, sizeof(m_szDateTime));

	MYSNPRINTF(m_szDateTime, 
		sizeof(m_szDateTime)-1,
		"TO_CHAR('%s', 'YYYY-MM-DD')", 
		szDateTime);

	return m_szDateTime;
}

const char* COciConnection::DateTimeToStr( const char* szDateTime )
{
	DB_POINTER_CHECK_RET(szDateTime, nullptr);

	memset(m_szDateTime, 0x0, sizeof(m_szDateTime));

	MYSNPRINTF(m_szDateTime,
		sizeof(m_szDateTime)-1,
		"TO_CHAR('%s', 'YYYY-MM-DD HH24:Mi:SS')", 
		szDateTime);

	return m_szDateTime;
}

const char* COciConnection::GetSysTime()
{
	memset(m_szDateTime, 0x0, sizeof(m_szDateTime));

	MYSNPRINTF(m_szDateTime,
		sizeof(m_szDateTime)-1,
		"SELECT TO_CHAR(sysdate, 'HH24:Mi:SS') FROM DUAL");

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

const char* COciConnection::GetSysDate()
{
	memset(m_szDateTime, 0x0, sizeof(m_szDateTime));

	MYSNPRINTF(m_szDateTime,
		sizeof(m_szDateTime)-1,
		"SELECT TO_CHAR(sysdate, 'YYYY-MM-DD') FROM DUAL");

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

const char* COciConnection::GetSysDateTime()
{   
	memset(m_szDateTime, 0x0, sizeof(m_szDateTime));

	MYSNPRINTF(m_szDateTime,
		sizeof(m_szDateTime)-1,
		"SELECT TO_CHAR(sysdate, 'YYYY-MM-DD HH24:Mi:SS') FROM DUAL");

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

bool COciConnection::isReconnect()
{
	// ��Ϊ�������Ӵ���������
	if ( GetErrorCode() == RETCODE_NETWORK_FAIL_CONNECT )
		return true;

	return false;
}

bool COciConnection::reconnectDB()
{
	MYDB_PRINT("TRY RECONNECT DB( %s )... \n", strDB_.c_str());
	disconnectDB();
	return ConnectDB();
}

void COciConnection::errorHandle()
{
	// ���������Ϣ	
	SetLastError();

	// ������Ϣ��ӡ
	errorPrint();
	
	// ����
	if ( isReconnect() )
		reconnectDB();
}

void COciConnection::errorPrint()
{
	MYDB_PRINT( "OCI %s INFO:\n"
		"CODE : ORA-%05i\n"
		"MSG  : %s\n"
		"SQL  : %s\n",
		(OCI_ErrorGetType(pOciErr_)==OCI_ERR_WARNING?"WARNING":"ERROR"),
		OCI_ErrorGetOCICode(pOciErr_),
		OCI_ErrorGetString(pOciErr_),
		OCI_GetSql(OCI_ErrorGetStatement(pOciErr_)) );
}

void COciConnection::SetLastError()
{
	pOciErr_ = OCI_GetLastError();
	MYASSERT(pOciErr_!=nullptr);
}

bool COciConnection::makeBindSql( const char *szSrcSql, std::string& strDstSql )
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

bool COciConnection::testConnectAlive()
{
	if ( !pOciConn_ )
		if ( !reconnectDB() )
			return false;

	return true;
}

void COciConnection::safeToFreeStatement( OCI_Statement **pStmt )
{
	if ( !isReconnect() && pOciConn_ )   // ���������󣬲����ͷ�pStmt
		OCI_StatementFree(*pStmt);

	*pStmt = nullptr;
} 
