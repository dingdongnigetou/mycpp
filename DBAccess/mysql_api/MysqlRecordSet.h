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
	virtual ~CMysqlRecordSet(void);

	// �Ƿ��¼��β��
	virtual	bool Eof( void );

	// �������һ����¼
	//virtual	bool MoveLast( void );

	// ������һ����¼
	virtual	bool MoveNext( void );

	// ��ȡ�ֶ�ֵ 
	// iFactLen: ʵ�ʳ��ȿ�����NULL
	// eType: ��������
	virtual bool GetValue( const char* szFieldName, 
		void* pBuf,
		unsigned int iBufSize,
		unsigned int* iFactLen,
		EnumDataType eType = DT_UNKNOWN );

	// ��ȡȡ�õ�ǰ�еĵ�i�е�ֵ
	// iFactLen: ʵ�ʳ��ȿ�����NULL
	// eType: ��������
	virtual bool GetValue( unsigned int iColumn, 
		void* pBuf,
		unsigned int iBufSize,
		unsigned int* iFactLen,
		EnumDataType eType = DT_UNKNOWN );

	// ��ȡ�ƶ�����
	virtual unsigned int GetRowsMoved( void );

	// ��ȡ����
	virtual unsigned int GetColumns( void );

	// ��ȡ����
	virtual const char* GetColumnName( unsigned int iColIndex );

	// ���ð󶨼�¼����
	virtual void SetBindRows( unsigned int iSize );

	// ��ȡ�󶨼�¼����
	virtual unsigned int GetBindRows( void );

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
		bool bNull = false );

	bool IsBindSuccess( void ) { return m_bBindSuccess; };

private:
	EnumDataType GetDataType( unsigned int iColumn, unsigned int iBufSize );
	bool  GetFieldList( void );
	bool  GetFieldIndex( const char* szFieldName,unsigned int& iIndex );   //�ж��Ƿ�Ϊ���һ���ֶ�

private:
	MYSQL*       m_pMysqlConn;
	MYSQL_RES*   m_pResult;
	MYSQL_STMT*  m_pStmt;
    
	//���Ƿ�ɹ�
	bool m_bBindSuccess;

	// �Ƿ�Ϊ��¼��β��
	bool m_bEof;

	unsigned int m_iMoveRows;

	// ������
	unsigned int m_iBindRows;

	// ������
	unsigned int m_iBindCols;
    
	//�󶨲�������
	MYSQL_BIND* m_pBindParam;
    
	//��¼�ֶ��������ֶ�λ��
	std::map<std::string, unsigned int>  m_mapFieldList;    
};

#endif // __MYSQLRECORDSET_DEF_H__
