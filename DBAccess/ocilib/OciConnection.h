
#ifndef OCICONNECTION_DEF_H
#define OCICONNECTION_DEF_H

#include "DBAccessApi.h"

#include "TypeDef.h"

#include "ocilib.h"

class COciConnection : public IConnection
{
public:
	// ���캯��
	COciConnection( const tstring& strHost,
		const tstring& strDataBase,
		const tstring& strUserName,
		const tstring& strPassword,
		UInt16 iPort = 0 );

	// ��������
	virtual ~COciConnection(void);

	// �ͷ����ݼ�
	virtual	void ReleaseRecordSet( IRecordSet** pcsRecordSet );

	// ׼�������� 
	// ����  sql��ʽ�� INSERT INTO TABLE(A,B,...) VALUES(?,?,...) 
	// ����  sql��ʽ�� UPDATE TABLE SET A=?,B=? WHERE C=123
	virtual	IRecordSet*	PrepareBind( const char* szSql );

	// ִ�а󶨵�����
	virtual bool ExecuteBind( IRecordSet* pcsRecordSet );

	// ִ��sql
	virtual	bool ExecuteSql( const char* szSql );

	// ȡ����һ�� INSERT ���������� ID
	// oracle ����sequence�����У�����ȡ��������ID����Ҫָ��������
	// mysql ��������Ч
	virtual bool GetLastInsertID( const char* szSeqName, signed __int64& lRowID );

	// ִ�в�ѯsql,�������ݼ�
	virtual	IRecordSet* ExecuteQuery( const char* szSql );

	// ִ�з�ҳ��ѯsql,�������ݼ�     iStartRow: ��ʼ��, iRowNum: ��������
	virtual	IRecordSet* ExecutePageQuery( const char* szSql, int iStartRow, int iRowNum );	

	// ��������
	virtual	bool BeginTrans( void );

	// �ع�����
	virtual	void Rollback( void );

	// �ύ����
	virtual	bool Commit( void );

	// ��ȡ������
	virtual EnumDBApiRet GetErrorCode( void );

	// ��ȡ������Ϣ
	virtual const char* GetErrorMessage( void );


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
	virtual	const char* ToTime(const char* szDateTime);
	// ������
	virtual	const char* ToDate(const char* szDateTime);
	// ������ʱ����
	virtual	const char* ToDateTime(const char* szDateTime);


	// ʱ����
	virtual	const char* TimeToStr(const char* szDateTime);
	// ������
	virtual	const char* DateToStr(const char* szDateTime);
	// ������ʱ����
	virtual	const char* DateTimeToStr(const char* szDateTime);


	// ��ȡ��ǰʱ��
	// ʱ����
	virtual const char* GetSysTime( void );
	// ������
	virtual const char* GetSysDate( void );
	// ������ʱ����
	virtual const char* GetSysDateTime( void );


public:
	// �������ݿ�
	bool ConnectDB( void );

	// ���������Ϣ
	void SetLastError( void );

private:
	// ��ʼ����Դ
	bool Initialize( void );

	// �ͷ���Դ
	void Cleanup( void );

	// �ر�����
	void DisconnectDB( void );

	// �Ƿ�����
	bool IsReconnect( void );

	// �������ӣ����ڲ���֤������
	bool ReconnectDB( void );

	// ������
	void ErrorHandle( void );

	// ������Ϣ��ӡ
	void ErrorPrint( void );

	// �����sql���
	bool MakeBindSql( const char *szSrcSql, tstring& strDstSql );

	// ��������״̬
	bool TestConnectAlive( void );

	// ��ȫ�ͷ�Statement
	void SafeToFreeStatement( OCI_Statement **pStmt );

private:
	// ��ʼ������
	static UInt16 m_iInitRefs;

	// ���ݿ�������Ϣ
	tstring  m_strDB;
	tstring  m_strUser;
	tstring  m_strPwd;

	// ���ݿ����Ӿ��
	OCI_Connection *m_pOciConn;

	// OCI������Ϣ
	OCI_Error *m_pOciErr;

	// ʱ�仺����
	char m_szDateTime[128];
};

#endif // OCICONNECTION_DEF_H
