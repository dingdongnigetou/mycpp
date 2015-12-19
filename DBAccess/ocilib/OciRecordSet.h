
#ifndef OCIRECORDSET_DEF_H
#define OCIRECORDSET_DEF_H

#include "DBAccessApi.h"

#include "TypeDef.h"

#include "ocilib.h"


class COciConnection;

class COciRecordSet : public IRecordSet
{
public:
	// ���캯��
	COciRecordSet( OCI_Statement *pStmt, COciConnection* pOciConn );

	// ��������
	virtual ~COciRecordSet(void);

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

	OCI_Statement* GetOCIStatement( void ) { return m_pStmt; };

private:
	bool GetLob( const char* szFieldName,
		void* pBuf, 
		unsigned int iBufSize, 
		unsigned int* iFactLen,
		bool bChar );

	bool GetLob( unsigned int iColumn,
		void* pBuf, 
		unsigned int iBufSize, 
		unsigned int* iFactLen,
		bool bChar );

	tstring ToBindName(unsigned int iValueIndex);

	EnumDataType GetDataType( unsigned int iColumn, unsigned int iBufSize );

private:
	OCI_Statement *m_pStmt;
	OCI_Resultset *m_pResultSet;

	COciConnection* m_pOciConn;

	// �Ƿ�Ϊ��¼��β��
	bool m_bEof;

	// ������
	unsigned int m_iBindRows;

	// ���͡��ַ��Ȳ����б�
	std::map<unsigned int, void*>  m_mapParamsList;

	//  OCI_Date�����б�
	std::map<unsigned int, OCI_Date*> m_mapDateList;

	// OCI_Lob�����б�
	std::map<unsigned int, OCI_Lob*> m_mapLobList;

	//  OCI_Date���������б�
	std::map<unsigned int, OCI_Date**> m_mapDateArrayList;

	// OCI_Lob���������б�
	std::map<unsigned int, OCI_Lob**> m_mapLobArrayList;
};

#endif // OCIRECORDSET_DEF_H
