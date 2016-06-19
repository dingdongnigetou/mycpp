
#ifndef __OCICONNECTION_DEF_H__
#define __OCICONNECTION_DEF_H__

#include "DBAccessApi.h"
#include "TypeDef.h"
#include "ocilib.h"

class COciConnection : public IConnection
{
public:
	// ���캯��
	COciConnection( const std::string& strHost,
		const std::string& strDataBase,
		const std::string& strUserName,
		const std::string& strPassword,
		UInt16 iPort = 1521 );

	// ��������
	virtual ~COciConnection() override;

	// �ͷ����ݼ�
	virtual	void ReleaseRecordSet( IRecordSet** pcsRecordSet ) override;

	// ׼�������� 
	// ����  sql��ʽ�� INSERT INTO TABLE(A,B,...) VALUES(?,?,...) 
	// ����  sql��ʽ�� UPDATE TABLE SET A=?,B=? WHERE C=123
	virtual	IRecordSet*	PrepareBind( const char* szSql ) override;

	// ִ�а󶨵�����
	virtual bool ExecuteBind( IRecordSet* pcsRecordSet ) override;

	// ִ��sql
	virtual	bool ExecuteSql( const char* szSql ) override;

	// ȡ����һ�� INSERT ���������� ID
	// oracle ����sequence�����У�����ȡ��������ID����Ҫָ��������
	// mysql ��������Ч
	virtual bool GetLastInsertID( const char* szSeqName, signed long long& lRowID );

	// ִ�в�ѯsql,�������ݼ�
	virtual	IRecordSet* ExecuteQuery( const char* szSql ) override;

	// ִ�з�ҳ��ѯsql,�������ݼ�     iStartRow: ��ʼ��, iRowNum: ��������
	virtual	IRecordSet* ExecutePageQuery( const char* szSql, int iStartRow, int iRowNum ) override;	

	// ��������
	virtual	bool BeginTrans() override;

	// �ع�����
	virtual	void Rollback() override;

	// �ύ����
	virtual	bool Commit() override;

	// ��ȡ������
	virtual EnumDBApiRet GetErrorCode() override;

	// ��ȡ������Ϣ
	virtual const char* GetErrorMessage() override;

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
	virtual	const char* ToTime(const char* szDateTime) override;
	// ������
	virtual	const char* ToDate(const char* szDateTime) override;
	// ������ʱ����
	virtual	const char* ToDateTime(const char* szDateTime) override;

	// ʱ����
	virtual	const char* TimeToStr(const char* szDateTime) override;
	// ������
	virtual	const char* DateToStr(const char* szDateTime) override;
	// ������ʱ����
	virtual	const char* DateTimeToStr(const char* szDateTime) override;

	// ��ȡ��ǰʱ��
	// ʱ����
	virtual const char* GetSysTime() override;
	// ������
	virtual const char* GetSysDate() override;
	// ������ʱ����
	virtual const char* GetSysDateTime() override;

public:
	// �������ݿ�
	bool ConnectDB();

	// ���������Ϣ
	void SetLastError();

private:
	// ��ʼ����Դ
	bool initialize();

	// �ͷ���Դ
	void cleanup();

	// �ر�����
	void disconnectDB();

	// �Ƿ�����
	bool isReconnect();

	// �������ӣ����ڲ���֤������
	bool reconnectDB();

	// ������
	void errorHandle();

	// ������Ϣ��ӡ
	void errorPrint();

	// �����sql���
	bool makeBindSql( const char *szSrcSql, std::string& strDstSql );

	// ��������״̬
	bool testConnectAlive();

	// ��ȫ�ͷ�Statement
	void safeToFreeStatement( OCI_Statement **pStmt );

private:
	// ��ʼ������
	static UInt16 iInitRefs_;

	// ���ݿ�������Ϣ
	std::string  strDB_;
	std::string  strUser_;
	std::string  strPwd_;

	// ���ݿ����Ӿ��
	OCI_Connection *pOciConn_ = nullptr;

	// OCI������Ϣ
	OCI_Error *pOciErr_ = nullptr;

	// ʱ�仺����
	char m_szDateTime[128];
};

#endif // __OCICONNECTION_DEF_H__
