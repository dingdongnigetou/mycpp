#ifndef __MYSQLRECORDSET_DEF_H__
#define __MYSQLRECORDSET_DEF_H__

#include "DBAccessApi.h"
#include "TypeDef.h"
#include "mysql.h"
#include "noncopyable.h"

#include <map> 

class CMysqlRecordSet : mycpp::noncopyable, public IRecordSet
{
public:
	// ���캯��
	CMysqlRecordSet( MYSQL* pConn, MYSQL_STMT* pStmt );

	// ��������
	virtual ~CMysqlRecordSet() override;

	// �Ƿ��¼��β��
	virtual	bool Eof() override;

	// �������һ����¼
	//virtual	bool MoveLast();

	// ������һ����¼
	virtual	bool MoveNext() override;

	// ��ȡ�ֶ�ֵ 
	// iFactLen: ʵ�ʳ��ȿ�����nullptr
	// eType: ��������
	virtual bool GetValue( const char* szFieldName, 
		void* pBuf,
		unsigned int iBufSize,
		unsigned int* iFactLen,
		EnumDataType eType = DT_UNKNOWN ) override;

	// ��ȡȡ�õ�ǰ�еĵ�i�е�ֵ
	// iFactLen: ʵ�ʳ��ȿ�����nullptr
	// eType: ��������
	virtual bool GetValue( unsigned int iColumn, 
		void* pBuf,
		unsigned int iBufSize,
		unsigned int* iFactLen,
		EnumDataType eType = DT_UNKNOWN ) override;

	// ��ȡ�ƶ�����
	virtual unsigned int GetRowsMoved() override;

	// ��ȡ����
	virtual unsigned int GetColumns() override;

	// ��ȡ����
	virtual const char* GetColumnName( unsigned int iColIndex ) override;

	// ���ð󶨼�¼����
	virtual void SetBindRows( unsigned int iSize ) override;

	// ��ȡ�󶨼�¼����
	virtual unsigned int GetBindRows() override;

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
		bool bNull = false ) override;

	bool IsBindSuccess() { return bBindSuccess_; };

private:
	EnumDataType getDataType( unsigned int iColumn, unsigned int iBufSize );
	bool  getFieldList();
	bool  getFieldIndex( const char* szFieldName,unsigned int& iIndex );   //�ж��Ƿ�Ϊ���һ���ֶ�

private:
	MYSQL*       pMysqlConn_ = nullptr;
	MYSQL_RES*   pResult_ = nullptr;
	MYSQL_STMT*  pStmt_ = nullptr;
    
	//���Ƿ�ɹ�
	bool bBindSuccess_ = false;

	// �Ƿ�Ϊ��¼��β��
	bool bEof_ = false;

	unsigned int iMoveRows_ = 0;

	// ������
	unsigned int iBindRows_ = 0;

	// ������
	unsigned int iBindCols_ = 0;
    
	//�󶨲�������
	MYSQL_BIND* pBindParam_ = nullptr;
    
	//��¼�ֶ��������ֶ�λ��
	std::map<std::string, unsigned int>  mapFieldList_;    
};

#endif // __MYSQLRECORDSET_DEF_H__
