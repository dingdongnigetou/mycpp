
#ifndef OCIRECORDSET_DEF_H
#define OCIRECORDSET_DEF_H

#include "DBAccessApi.h"

#include "TypeDef.h"

#include "ocilib.h"


class COciConnection;

class COciRecordSet : public IRecordSet
{
public:
	// 构造函数
	COciRecordSet( OCI_Statement *pStmt, COciConnection* pOciConn );

	// 析构函数
	virtual ~COciRecordSet(void);

	// 是否记录集尾部
	virtual	bool Eof( void );

	// 移向最后一条记录
	//virtual	bool MoveLast( void );

	// 移向下一条记录
	virtual	bool MoveNext( void );

	// 获取字段值 
	// iFactLen: 实际长度可以填NULL
	// eType: 数据类型
	virtual bool GetValue( const char* szFieldName, 
		void* pBuf,
		unsigned int iBufSize,
		unsigned int* iFactLen,
		EnumDataType eType = DT_UNKNOWN );

	// 获取取得当前行的第i列的值
	// iFactLen: 实际长度可以填NULL
	// eType: 数据类型
	virtual bool GetValue( unsigned int iColumn, 
		void* pBuf,
		unsigned int iBufSize,
		unsigned int* iFactLen,
		EnumDataType eType = DT_UNKNOWN );

	// 获取移动行数
	virtual unsigned int GetRowsMoved( void );

	// 获取列数
	virtual unsigned int GetColumns( void );

	// 获取列名
	virtual const char* GetColumnName( unsigned int iColIndex );

	// 设置绑定记录行数
	virtual void SetBindRows( unsigned int iSize );

	// 获取绑定记录行数
	virtual unsigned int GetBindRows( void );

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

	// 是否为记录集尾部
	bool m_bEof;

	// 绑定行数
	unsigned int m_iBindRows;

	// 整型、字符等参数列表
	std::map<unsigned int, void*>  m_mapParamsList;

	//  OCI_Date参数列表
	std::map<unsigned int, OCI_Date*> m_mapDateList;

	// OCI_Lob参数列表
	std::map<unsigned int, OCI_Lob*> m_mapLobList;

	//  OCI_Date参数数组列表
	std::map<unsigned int, OCI_Date**> m_mapDateArrayList;

	// OCI_Lob参数数组列表
	std::map<unsigned int, OCI_Lob**> m_mapLobArrayList;
};

#endif // OCIRECORDSET_DEF_H
