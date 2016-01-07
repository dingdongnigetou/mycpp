#ifndef __DBACCESSAPI_DEF_H__
#define __DBACCESSAPI_DEF_H__

#include <cstdlib>

#ifdef _WIN32

#ifdef DBACCESS_STATIC
#define DB_API 
#else

#ifdef DBACCESS_EXPORTS
#define DB_API extern "C" __declspec(dllexport)
#else
#define DB_API extern "C" __declspec(dllimport)
#endif

#endif

#elif _LINUX

#define DB_API extern "C" 
#endif

typedef enum EnumDBApiRet
{
	RETCODE_SUCCESS = 0,
	RETCODE_INITIALIZE_FAIL,
	RETCODE_NETWORK_FAIL_CONNECT,
	RETCODE_PARAMS_ERROR,
	RETCODE_USERNAME_PASSWORD_ERROR,
	RETCODE_SQL_SYNTAX_ERROR,
	RETCODE_UNIQUE_CONSTRAINT_VIOLATED,
	RETCODE_OVER_MAXLINK,


	RETCODE_UNKNOWN_ERROR = 9999,
};


typedef enum EnumDriverType
{
	ODBC = 0,
	OCI,
	MYSQL_API,
	ADO,
	// to add:
};

typedef enum EnumDataType
{
	DT_NUMBER = 0,   // DT_INT8, DT_INT16, DT_INT32, DT_INT64, DT_FLOAT, DT_DOUBLE
	DT_INT8,
	DT_UINT8,
	DT_INT16,
	DT_UINT16,
	DT_INT32,
	DT_UINT32,
	DT_INT64,
	DT_UINT64,
	DT_TIME,
	DT_FLOAT,
	DT_DOUBLE,
	DT_STRING,
	DT_CLOB,
	DT_BLOB,
	DT_UNKNOWN
};


// 数据集  (非线程安全)
class IRecordSet
{
protected:
	IRecordSet(void){};

public:
	virtual ~IRecordSet(void){};

public:

	// 是否记录集尾部
	virtual	bool Eof( void ) = 0;

	// 移向最后一条记录
	//virtual	bool MoveLast( void ) = 0;

	// 移向下一条记录
	// 返回值: true 成功  false 需要判断Eof(),是结尾还是错误,错误则可以从GSIConnection获取错误码和错误信息
	virtual	bool MoveNext( void ) = 0;

	// 获取字段值 
	// iFactLen: 实际长度,可以填NULL
	// eType: 数据类型
	virtual bool GetValue( const char* szFieldName, 
		void* pBuf,
		unsigned int iBufSize,
		unsigned int* iFactLen,
		EnumDataType eType = DT_UNKNOWN ) = 0;

	// 获取取得当前行的第i列的值
	// iColumn: 列序号由1开始
	// iFactLen: 实际长度,可以填NULL
	// eType: 数据类型
	virtual bool GetValue( unsigned int iColumn, 
		void* pBuf,
		unsigned int iBufSize,
		unsigned int* iFactLen,
		EnumDataType eType = DT_UNKNOWN ) = 0;

	// 获取移动行数
	virtual unsigned int GetRowsMoved( void ) = 0;

	// 获取列数
	virtual unsigned int GetColumns( void ) = 0;

	// 获取列名
	virtual const char* GetColumnName( unsigned int iColIndex ) = 0;

public:
	// 设置绑定记录行数
	virtual void SetBindRows( unsigned int iSize ) = 0;

	// 获取绑定记录行数
	virtual unsigned int GetBindRows( void ) = 0;

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
		bool bNull = false ) = 0;
};

// 连接   (非线程安全)
class IConnection
{
protected:
	IConnection(void){};

public:
	virtual ~IConnection(void){};

public:

	// 释放数据集
	virtual	void ReleaseRecordSet( IRecordSet** pcsRecordSet ) = 0;

	// 准备绑定数据 
	// 插入  sql格式如 INSERT INTO TABLE(A,B,...) VALUES(?,?,...) 
	// 更新  sql格式如 UPDATE TABLE SET A=?,B=? WHERE C=?
	virtual	IRecordSet*	PrepareBind( const char* szSql ) = 0;

	// 执行绑定的数据
	virtual bool ExecuteBind( IRecordSet* pcsRecordSet ) = 0;

	// 执行sql
	virtual	bool ExecuteSql( const char* szSql ) = 0;

	// 执行sql,返回lRowID
	// INSERT 返回插入行号
	// UPDATE 返回受影响行数
	// DELETE 返回受影响行数
	//virtual	bool ExecuteSql( const char* szSql, const char* szTable,  signed __int64& lRowID ) = 0;


	// 取得上一步 INSERT 操作产生的 ID
	// oracle 采用sequence（序列）来获取最后插入行ID，需要指定序列名
	// mysql 序列名无效
	virtual bool GetLastInsertID( const char* szSeqName, signed __int64& lRowID ) = 0;


	// 执行查询sql,返回数据集
	virtual	IRecordSet* ExecuteQuery( const char* szSql ) = 0;

	// 执行分页查询sql,返回数据集    szSql:如 SELECT * FROM TB , iStartRow: 起始行, iRowNum: 返回行数
	virtual	IRecordSet* ExecutePageQuery( const char* szSql, int iStartRow, int iRowNum ) = 0;	

	// 开启事务
	virtual	bool BeginTrans( void ) = 0;

	// 回滚事务
	virtual	void Rollback( void ) = 0;

	// 提交事务
	virtual	bool Commit( void ) = 0;

	// 获取错误码
	virtual EnumDBApiRet GetErrorCode( void ) = 0;

	// 获取错误信息
	virtual const char* GetErrorMessage( void ) = 0;


	// 将字符串转换成标准的数据库的时间字符串
	// Input:
	//					ToTime:HH:MI:SS
	//				    ToDate:YYYY-MM-DD
	//					ToDateTime:YYYY-MM-DD HH:MI:SS
	// Output: 
	//					ToTime:HH:MI:SS
	//					ToDate:YYYY-MM-DD
	//					ToDateTime:YYYY-MM-DD HH:MI:SS
	// 时分秒
	virtual	const char* ToTime(const char* szDateTime) = 0;
	// 年月日
	virtual	const char* ToDate(const char* szDateTime) = 0;
	// 年月日时分秒
	virtual	const char* ToDateTime(const char* szDateTime) = 0;


	// 时分秒
	virtual	const char* TimeToStr(const char* szDateTime) = 0;
	// 年月日
	virtual	const char* DateToStr(const char* szDateTime) = 0;
	// 年月日时分秒
	virtual	const char* DateTimeToStr(const char* szDateTime) = 0;


	// 获取当前时间
	virtual const char* GetSysTime( void ) = 0;
	virtual const char* GetSysDate( void ) = 0;
	virtual const char* GetSysDateTime( void ) = 0;

};


// 连接池  (线程安全)
class IConnectionPool 
{
protected:
	IConnectionPool(void){};

public:
	virtual ~IConnectionPool(void){};

public:
	// 设置参数
	// szHost:     数据库主机,本地数据数据库填NULL
	// szDataBase: 数据库名
	// szUserName: 连接数据库用户名
	// szPassword: 连接数据库密码
	// iPort:      数据库监听端口,默认值0表示使用数据库默认端口
	// iMinConns:  最小连接数
	// iMaxConns:  最大连接数
	virtual void SetParams( const char* szHost,
		const char* szDatabase,
		const char* szUserName,
		const char* szPassword,
		unsigned short iPort = 0,
		unsigned int iMinConns = 1,
		unsigned int iMaxConns = 10 ) = 0;

	// 获取连接
	// eError: 获取到连接为空时，可根据错误码来查看原因
	virtual IConnection* GetConnection( EnumDBApiRet* eError=NULL ) = 0;

	// 释放连接
	virtual	void ReleaseConnection( IConnection** pcsConn ) = 0;

	virtual EnumDriverType GetDriverType() = 0;
};

// 创建数据库连接池
DB_API IConnectionPool* CreateDBConnectionPool( EnumDriverType eType );

// 销毁数据库连接池
DB_API void DestroyDBConnectionPool( IConnectionPool** pConnPool );

#endif // __DBACCESSAPI_DEF_H__

