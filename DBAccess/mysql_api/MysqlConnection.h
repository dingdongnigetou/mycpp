
#ifndef __MYSQLCONNECTION_DEF_H__
#define __MYSQLCONNECTION_DEF_H__

#include "DBAccessApi.h"
#include "TypeDef.h"
#include "mysql.h"
#include "noncopyable.h"

class CMysqlConnection : mycpp::noncopyable, public IConnection
{
public:
	// 构造函数
	CMysqlConnection( const std::string& strHost,
		const std::string& strDataBase,
		const std::string& strUserName,
		const std::string& strPassword,
		UInt16 iPort = 0 );

	// 析构函数
	virtual ~CMysqlConnection(void);

	// 释放数据集
	virtual	void ReleaseRecordSet( IRecordSet** pcsRecordSet );

	// 准备绑定数据 
	// 插入  sql格式如 INSERT INTO TABLE(A,B,...) VALUES(?,?,...) 
	// 更新  sql格式如 UPDATE TABLE SET A=?,B=? WHERE C=123
	virtual	IRecordSet*	PrepareBind( const char* szSql );

	// 执行绑定的数据
	virtual bool ExecuteBind( IRecordSet* pcsRecordSet );

	// 执行sql
	virtual	bool ExecuteSql( const char* szSql );

	// 取得上一步 INSERT 操作产生的 ID
	// oracle 采用sequence（序列）来获取最后插入行ID，需要指定序列名
	// mysql 序列名无效
	virtual bool GetLastInsertID( const char* szSeqName, signed __int64& lRowID );

	// 执行查询sql,返回数据集
	virtual	IRecordSet* ExecuteQuery( const char* szSql );

	// 执行分页查询sql,返回数据集     iStartRow: 起始行, iRowNum: 返回行数
	virtual	IRecordSet* ExecutePageQuery( const char* szSql, int iStartRow, int iRowNum );	

	// 开启事务
	virtual	bool BeginTrans( void );

	// 回滚事务
	virtual	void Rollback( void );

	// 提交事务
	virtual	bool Commit( void );

	// 获取错误码
	virtual EnumDBApiRet GetErrorCode( void );

	// 获取错误信息
	virtual const char* GetErrorMessage( void );


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
	virtual	const char* ToTime(const char* szDateTime);
	// 年月日
	virtual	const char* ToDate(const char* szDateTime);
	// 年月日时分秒
	virtual	const char* ToDateTime(const char* szDateTime);


	// 时分秒
	virtual	const char* TimeToStr(const char* szDateTime);
	// 年月日
	virtual	const char* DateToStr(const char* szDateTime);
	// 年月日时分秒
	virtual	const char* DateTimeToStr(const char* szDateTime);


	// 获取当前时间
	// 时分秒
	virtual const char* GetSysTime( void );
	// 年月日
	virtual const char* GetSysDate( void );
	// 年月日时分秒
	virtual const char* GetSysDateTime( void );


public:
	// 连接数据库
	bool ConnectDB( void );


private:
	// 关闭连接
	void DisconnectDB( void );

	// 是否重连
	bool IsReconnect( void );

	// 重新连接（由内部保证重连）
	bool ReconnectDB( void );

	// 错误处理
	void ErrorHandle( void );

	// 错误信息打印
	void ErrorPrint( void );

	// 测试连接状态
	bool TestConnectAlive( void );

private:
	// 数据库连接信息
	std::string  m_strDB;
    std::string  m_strHost;
	std::string  m_strUser;
	std::string  m_strPwd;
	UInt16    m_iPort;

	// 数据库连接句柄
	MYSQL *m_pMysqlConn;

	// 时间缓冲区
	char m_szDateTime[256];
};

#endif // __MYSQLCONNECTION_DEF_H__
