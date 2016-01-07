
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
		UInt16 iPort = 0 );

	// ��������
	virtual ~CAdoConnection();

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
	virtual	bool BeginTrans( );

	// �ع�����
	virtual	void Rollback( );

	// �ύ����
	virtual	bool Commit( );

	// ��ȡ������
	virtual EnumDBApiRet GetErrorCode( );

	// ��ȡ������Ϣ
	virtual const char* GetErrorMessage( );


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
	virtual const char* GetSysTime( );
	// ������
	virtual const char* GetSysDate( );
	// ������ʱ����
	virtual const char* GetSysDateTime( );


public:
	// �������ݿ�
	bool ConnectDB();

	// ���������Ϣ
	void SetLastError();

	// ��ȡԭʼado����ָ������
	_ConnectionPtr& GetRawConnRef();

private:
	// �ر�����
	bool Close();

	// �����Ƿ��
	bool IsOpen();

	// �Ƿ�����
	bool IsReconnect( );

	// �������ӣ����ڲ���֤������
	bool ReconnectDB( );

	// ������
	void ErrorHandle( );

	// ��������״̬
	bool TestConnectAlive( );

private:
	// ���ݿ�������Ϣ
	std::string  strDB_;
    std::string  strHost_;
	std::string  strUser_;
	std::string  strPwd_;
	UInt16    iPort_;

	// ���ݿ����Ӿ��
	_ConnectionPtr pConn_;

	// ������Ϣ
	ErrorPtr  pErr_;

	// ʱ�仺����
	char szDateTime_[256];
};

#endif // __ADOCONNECTION_DEF_H__
