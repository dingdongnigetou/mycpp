
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
	// 构造函数
	COciRecordSet( OCI_Statement *pStmt, COciConnection* pOciConn );

	// 析构函数
	virtual ~COciRecordSet() override;

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

	// 是否为记录集尾部
	bool bEof_ = false;

	// 绑定行数
	unsigned int iBindRows_ = 0;

	// 整型、字符等参数列表
	std::map<unsigned int, void*>  mapParamsList_;

	//  OCI_Date参数列表
	std::map<unsigned int, OCI_Date*> mapDateList_;

	// OCI_Lob参数列表
	std::map<unsigned int, OCI_Lob*> mapLobList_;

	//  OCI_Date参数数组列表
	std::map<unsigned int, OCI_Date**> mapDateArrayList_;

	// OCI_Lob参数数组列表
	std::map<unsigned int, OCI_Lob**> mapLobArrayList_;
};

#endif // __OCIRECORDSET_DEF_H__

