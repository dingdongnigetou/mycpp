#ifndef __DBACCESSAPI_DEF_H__
#define __DBACCESSAPI_DEF_H__

#include <cstdlib>

#ifdef _WIN32

#ifdef DBACCESS_STATIC
#define DB_API 
#else

#ifdef DBACCESS_EXPORTS
#define DB_API extern "C" __declspec(dllexport)
#else
#define DB_API extern "C" __declspec(dllimport)
#endif

#endif

#elif _LINUX

#define DB_API extern "C" 
#endif

typedef enum EnumDBApiRet
{
	RETCODE_SUCCESS = 0,
	RETCODE_INITIALIZE_FAIL,
	RETCODE_NETWORK_FAIL_CONNECT,
	RETCODE_PARAMS_ERROR,
	RETCODE_USERNAME_PASSWORD_ERROR,
	RETCODE_SQL_SYNTAX_ERROR,
	RETCODE_UNIQUE_CONSTRAINT_VIOLATED,
	RETCODE_OVER_MAXLINK,


	RETCODE_UNKNOWN_ERROR = 9999,
};


typedef enum EnumDriverType
{
	ODBC = 0,
	OCI,
	MYSQL_API,
	ADO,
	// to add:
};

typedef enum EnumDataType
{
	DT_NUMBER = 0,   // DT_INT8, DT_INT16, DT_INT32, DT_INT64, DT_FLOAT, DT_DOUBLE
	DT_INT8,
	DT_UINT8,
	DT_INT16,
	DT_UINT16,
	DT_INT32,
	DT_UINT32,
	DT_INT64,
	DT_UINT64,
	DT_TIME,
	DT_FLOAT,
	DT_DOUBLE,
	DT_STRING,
	DT_CLOB,
	DT_BLOB,
	DT_UNKNOWN
};


// ���ݼ�  (���̰߳�ȫ)
class IRecordSet
{
protected:
	IRecordSet(void){};

public:
	virtual ~IRecordSet(void){};

public:

	// �Ƿ��¼��β��
	virtual	bool Eof( void ) = 0;

	// �������һ����¼
	//virtual	bool MoveLast( void ) = 0;

	// ������һ����¼
	// ����ֵ: true �ɹ�  false ��Ҫ�ж�Eof(),�ǽ�β���Ǵ���,��������Դ�GSIConnection��ȡ������ʹ�����Ϣ
	virtual	bool MoveNext( void ) = 0;

	// ��ȡ�ֶ�ֵ 
	// iFactLen: ʵ�ʳ���,������NULL
	// eType: ��������
	virtual bool GetValue( const char* szFieldName, 
		void* pBuf,
		unsigned int iBufSize,
		unsigned int* iFactLen,
		EnumDataType eType = DT_UNKNOWN ) = 0;

	// ��ȡȡ�õ�ǰ�еĵ�i�е�ֵ
	// iColumn: �������1��ʼ
	// iFactLen: ʵ�ʳ���,������NULL
	// eType: ��������
	virtual bool GetValue( unsigned int iColumn, 
		void* pBuf,
		unsigned int iBufSize,
		unsigned int* iFactLen,
		EnumDataType eType = DT_UNKNOWN ) = 0;

	// ��ȡ�ƶ�����
	virtual unsigned int GetRowsMoved( void ) = 0;

	// ��ȡ����
	virtual unsigned int GetColumns( void ) = 0;

	// ��ȡ����
	virtual const char* GetColumnName( unsigned int iColIndex ) = 0;

public:
	// ���ð󶨼�¼����
	virtual void SetBindRows( unsigned int iSize ) = 0;

	// ��ȡ�󶨼�¼����
	virtual unsigned int GetBindRows( void ) = 0;

	// ���ֶε���Ϣ
	// iRowIndex: �������0��ʼ
	// iValueIndex: ֵ�����0��ʼ
	// iBufSize: �ַ������߶�������������ʱ��Ҫָ��
	// iFactLen: �ַ������߶�������������ʱ��Ҫָ��
	virtual bool BindField( unsigned int iRowIndex, 
		unsigned int iValueIndex,
		EnumDataType eType, 
		void *pBuf, 
		unsigned int iBufSize = 0, 
		unsigned int iFactLen = 0,
		bool bNull = false ) = 0;
};

// ����   (���̰߳�ȫ)
class IConnection
{
protected:
	IConnection(void){};

public:
	virtual ~IConnection(void){};

public:

	// �ͷ����ݼ�
	virtual	void ReleaseRecordSet( IRecordSet** pcsRecordSet ) = 0;

	// ׼�������� 
	// ����  sql��ʽ�� INSERT INTO TABLE(A,B,...) VALUES(?,?,...) 
	// ����  sql��ʽ�� UPDATE TABLE SET A=?,B=? WHERE C=?
	virtual	IRecordSet*	PrepareBind( const char* szSql ) = 0;

	// ִ�а󶨵�����
	virtual bool ExecuteBind( IRecordSet* pcsRecordSet ) = 0;

	// ִ��sql
	virtual	bool ExecuteSql( const char* szSql ) = 0;

	// ִ��sql,����lRowID
	// INSERT ���ز����к�
	// UPDATE ������Ӱ������
	// DELETE ������Ӱ������
	//virtual	bool ExecuteSql( const char* szSql, const char* szTable,  signed __int64& lRowID ) = 0;


	// ȡ����һ�� INSERT ���������� ID
	// oracle ����sequence�����У�����ȡ��������ID����Ҫָ��������
	// mysql ��������Ч
	virtual bool GetLastInsertID( const char* szSeqName, signed __int64& lRowID ) = 0;


	// ִ�в�ѯsql,�������ݼ�
	virtual	IRecordSet* ExecuteQuery( const char* szSql ) = 0;

	// ִ�з�ҳ��ѯsql,�������ݼ�    szSql:�� SELECT * FROM TB , iStartRow: ��ʼ��, iRowNum: ��������
	virtual	IRecordSet* ExecutePageQuery( const char* szSql, int iStartRow, int iRowNum ) = 0;	

	// ��������
	virtual	bool BeginTrans( void ) = 0;

	// �ع�����
	virtual	void Rollback( void ) = 0;

	// �ύ����
	virtual	bool Commit( void ) = 0;

	// ��ȡ������
	virtual EnumDBApiRet GetErrorCode( void ) = 0;

	// ��ȡ������Ϣ
	virtual const char* GetErrorMessage( void ) = 0;


	// ���ַ���ת���ɱ�׼�����ݿ��ʱ���ַ���
	// Input:
	//					ToTime:HH:MI:SS
	//				    ToDate:YYYY-MM-DD
	//					ToDateTime:YYYY-MM-DD HH:MI:SS
	// Output: 
	//					ToTime:HH:MI:SS
	//					ToDate:YYYY-MM-DD
	//					ToDateTime:YYYY-MM-DD HH:MI:SS
	// ʱ����
	virtual	const char* ToTime(const char* szDateTime) = 0;
	// ������
	virtual	const char* ToDate(const char* szDateTime) = 0;
	// ������ʱ����
	virtual	const char* ToDateTime(const char* szDateTime) = 0;


	// ʱ����
	virtual	const char* TimeToStr(const char* szDateTime) = 0;
	// ������
	virtual	const char* DateToStr(const char* szDateTime) = 0;
	// ������ʱ����
	virtual	const char* DateTimeToStr(const char* szDateTime) = 0;


	// ��ȡ��ǰʱ��
	virtual const char* GetSysTime( void ) = 0;
	virtual const char* GetSysDate( void ) = 0;
	virtual const char* GetSysDateTime( void ) = 0;

};


// ���ӳ�  (�̰߳�ȫ)
class IConnectionPool 
{
protected:
	IConnectionPool(void){};

public:
	virtual ~IConnectionPool(void){};

public:
	// ���ò���
	// szHost:     ���ݿ�����,�����������ݿ���NULL
	// szDataBase: ���ݿ���
	// szUserName: �������ݿ��û���
	// szPassword: �������ݿ�����
	// iPort:      ���ݿ�����˿�,Ĭ��ֵ0��ʾʹ�����ݿ�Ĭ�϶˿�
	// iMinConns:  ��С������
	// iMaxConns:  ���������
	virtual void SetParams( const char* szHost,
		const char* szDatabase,
		const char* szUserName,
		const char* szPassword,
		unsigned short iPort = 0,
		unsigned int iMinConns = 1,
		unsigned int iMaxConns = 10 ) = 0;

	// ��ȡ����
	// eError: ��ȡ������Ϊ��ʱ���ɸ��ݴ��������鿴ԭ��
	virtual IConnection* GetConnection( EnumDBApiRet* eError=NULL ) = 0;

	// �ͷ�����
	virtual	void ReleaseConnection( IConnection** pcsConn ) = 0;

	virtual EnumDriverType GetDriverType() = 0;
};

// �������ݿ����ӳ�
DB_API IConnectionPool* CreateDBConnectionPool( EnumDriverType eType );

// �������ݿ����ӳ�
DB_API void DestroyDBConnectionPool( IConnectionPool** pConnPool );

#endif // __DBACCESSAPI_DEF_H__

