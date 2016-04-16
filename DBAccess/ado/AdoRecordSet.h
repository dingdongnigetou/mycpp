
#ifndef __ADORECORDSET_DEF_H__
#define __ADORECORDSET_DEF_H__

#include "DBAccessApi.h"
#include "TypeDef.h"

#include <map>

class CAdoConnection;

class CAdoRecordSet : public IRecordSet
{
public:
	// 构造函数
	CAdoRecordSet(const char* szSql, CAdoConnection* pConn);

	// 析构函数
	virtual ~CAdoRecordSet() override;

	// 是否记录集尾部
	virtual	bool Eof() override;

	// 移向最后一条记录
	//virtual	bool MoveLast();

	// 移向下一条记录
	virtual	bool MoveNext() override;

	// 获取字段值 
	// iFactLen: 实际长度可以填nullptr
	// eType: 数据类型
	virtual bool GetValue( const char* szFieldName, 
		void* pBuf,
		unsigned int iBufSize,
		unsigned int* iFactLen,
		EnumDataType eType = DT_UNKNOWN ) override;

	// 获取取得当前行的第i列的值
	// iFactLen: 实际长度可以填nullptr
	// eType: 数据类型
	virtual bool GetValue( unsigned int iColumn, 
		void* pBuf,
		unsigned int iBufSize,
		unsigned int* iFactLen,
		EnumDataType eType = DT_UNKNOWN ) override;

	// 获取移动行数
	virtual unsigned int GetRowsMoved() override;

	// 获取列数
	virtual unsigned int GetColumns() override;

	// 获取列名
	virtual const char* GetColumnName( unsigned int iColIndex ) override;

	// 设置绑定记录行数
	virtual void SetBindRows( unsigned int iSize ) override;

	// 获取绑定记录行数
	virtual unsigned int GetBindRows() override;

	// 绑定字段的信息
	// iRowIndex: 行序号由0开始
	// iValueIndex: 值序号由0开始
	// iBufSize: 字符串或者二进制数据类型时需要指定
	// iFactLen: 字符串或者二进制数据类型时需要指定
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
