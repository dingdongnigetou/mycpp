
#ifndef __OCICONNECTION_DEF_H__
#define __OCICONNECTION_DEF_H__

#include "DBAccessApi.h"
#include "TypeDef.h"
#include "ocilib.h"

class COciConnection : public IConnection
{
public:
	// 构造函数
	COciConnection( const tstring& strHost,
		const tstring& strDataBase,
		const tstring& strUserName,
		const tstring& strPassword,
		UInt16 iPort = 0 );

	// 析构函数
	virtual ~COciConnection(void);

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

	// 保存错误信息
	void SetLastError( void );

private:
	// 初始化资源
	bool Initialize( void );

	// 释放资源
	void Cleanup( void );

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

	// 构造绑定sql语句
	bool MakeBindSql( const char *szSrcSql, tstring& strDstSql );

	// 测试连接状态
	bool TestConnectAlive( void );

	// 安全释放Statement
	void SafeToFreeStatement( OCI_Statement **pStmt );

private:
	// 初始化计数
	static UInt16 m_iInitRefs;

	// 数据库连接信息
	tstring  m_strDB;
	tstring  m_strUser;
	tstring  m_strPwd;

	// 数据库连接句柄
	OCI_Connection *m_pOciConn;

	// OCI错误信息
	OCI_Error *m_pOciErr;

	// 时间缓冲区
	char m_szDateTime[128];
};

#endif // __OCICONNECTION_DEF_H__
