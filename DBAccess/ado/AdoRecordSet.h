
#ifndef __ADORECORDSET_DEF_H__
#define __ADORECORDSET_DEF_H__

#include "DBAccessApi.h"
#include "TypeDef.h"

#include <map>

class CAdoConnection;

class CAdoRecordSet : public IRecordSet
{
public:
	// ���캯��
	CAdoRecordSet(const char* szSql, CAdoConnection* pConn);

	// ��������
	virtual ~CAdoRecordSet() override;

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

	bool IsOpen();

private:

	bool open(const char * szSql);

	bool close();

	bool getEditMode();

	bool cancelUpdate();

	FieldsPtr getFields();

	FieldPtr getField(const char* szFieldName);

	std::string toBindName(unsigned int iValueIndex);

	EnumDataType getDataType(const char* szFieldName);

private:
	_RecordsetPtr   pRecordSet_ = nullptr;
	CAdoConnection* pConn_ = nullptr;
};

#endif // __ADORECORDSET_DEF_H__
