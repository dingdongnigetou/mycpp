
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
	virtual ~CAdoRecordSet(void);

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

	bool IsOpen();

private:

	bool Open(const char * szSql);

	bool Close();

	bool GetEditMode();

	bool CancelUpdate();

	FieldsPtr GetFields();

	FieldPtr GetField(const char* szFieldName);

	std::string ToBindName(unsigned int iValueIndex);

	EnumDataType GetDataType(const char* szFieldName);

private:
	_RecordsetPtr   pRecordSet_;
	CAdoConnection* pConn_;
};

#endif // __ADORECORDSET_DEF_H__

