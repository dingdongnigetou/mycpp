
#ifndef __ADOCONNECTION_DEF_H__
#define __ADOCONNECTION_DEF_H__


#include "DBAccessApi.h"
#include "TypeDef.h"
#include "noncopyable.h"

class CAdoConnection : mycpp::noncopyable, public IConnection
{
public:
	// ���캯��
	CAdoConnection( const std::string& strHost,
		const std::string& strDataBase,
		const std::string& strUserName,
		const std::string& strPassword,
		UInt16 iPort = 1433 );

	// ��������
	virtual ~CAdoConnection() override;

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
	virtual bool GetLastInsertID( const char* szSeqName, signed __int64& lRowID ) override;

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
	void SetLastError(const char* err = nullptr);

	// ���������Ϣ
	void ClearError();

	// ��ȡԭʼado����ָ������
	_ConnectionPtr& GetRawConnRef();

private:
	// �ر�����
	bool close();

	// �����Ƿ��
	bool isOpen();

	// �Ƿ�����
	bool isReconnect();

	// �������ӣ����ڲ���֤������
	bool reconnectDB();

	// ������
	void errorHandle(const char* err = nullptr);

	// ��������״̬
	bool testConnectAlive();

private:
	// ���ݿ������ַ���
	std::string  strDB_ = "";

	// ���ݿ����Ӿ��
	_ConnectionPtr pConn_ = nullptr;

	// ������Ϣ
	char* pErr_ = nullptr;

	// ʱ�仺����
	char szDateTime_[256];
};

#endif // __ADOCONNECTION_DEF_H__
