
#ifndef __OCIRECORDSET_DEF_H__
#define __OCIRECORDSET_DEF_H__

#include "DBAccessApi.h"
#include "TypeDef.h"
#include "ocilib.h"

#include <map>

class COciConnection;

class COciRecordSet : public IRecordSet
{
public:
	// ���캯��
	COciRecordSet( OCI_Statement *pStmt, COciConnection* pOciConn );

	// ��������
	virtual ~COciRecordSet() override;

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

	OCI_Statement* GetOCIStatement() { return pStmt_; };

private:
	bool getLob( const char* szFieldName,
		void* pBuf, 
		unsigned int iBufSize, 
		unsigned int* iFactLen,
		bool bChar );

	bool getLob( unsigned int iColumn,
		void* pBuf, 
		unsigned int iBufSize, 
		unsigned int* iFactLen,
		bool bChar );

	std::string toBindName(unsigned int iValueIndex);

	EnumDataType getDataType( unsigned int iColumn, unsigned int iBufSize );

private:
	OCI_Statement *pStmt_ = nullptr;
	OCI_Resultset *pResultSet_ = nullptr;

	COciConnection* pOciConn_ = nullptr;

	// �Ƿ�Ϊ��¼��β��
	bool bEof_ = false;

	// ������
	unsigned int iBindRows_ = 0;

	// ���͡��ַ��Ȳ����б�
	std::map<unsigned int, void*>  mapParamsList_;

	//  OCI_Date�����б�
	std::map<unsigned int, OCI_Date*> mapDateList_;

	// OCI_Lob�����б�
	std::map<unsigned int, OCI_Lob*> mapLobList_;

	//  OCI_Date���������б�
	std::map<unsigned int, OCI_Date**> mapDateArrayList_;

	// OCI_Lob���������б�
	std::map<unsigned int, OCI_Lob**> mapLobArrayList_;
};

#endif // __OCIRECORDSET_DEF_H__

