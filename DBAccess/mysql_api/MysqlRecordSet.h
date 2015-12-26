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
	// 构造函数
	CMysqlRecordSet( MYSQL* pConn, MYSQL_STMT* pStmt );

	// 析构函数
	virtual ~CMysqlRecordSet(void);

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

	bool IsBindSuccess( void ) { return m_bBindSuccess; };

private:
	EnumDataType GetDataType( unsigned int iColumn, unsigned int iBufSize );
	bool  GetFieldList( void );
	bool  GetFieldIndex( const char* szFieldName,unsigned int& iIndex );   //判断是否为表的一个字段

private:
	MYSQL*       m_pMysqlConn;
	MYSQL_RES*   m_pResult;
	MYSQL_STMT*  m_pStmt;
    
	//绑定是否成功
	bool m_bBindSuccess;

	// 是否为记录集尾部
	bool m_bEof;

	unsigned int m_iMoveRows;

	// 绑定行数
	unsigned int m_iBindRows;

	// 绑定列数
	unsigned int m_iBindCols;
    
	//绑定参数数组
	MYSQL_BIND* m_pBindParam;
    
	//记录字段名字与字段位置
	std::map<std::string, unsigned int>  m_mapFieldList;    
};

#endif // __MYSQLRECORDSET_DEF_H__
