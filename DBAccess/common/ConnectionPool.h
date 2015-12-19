
#ifndef GSCONNECTIONPOOL_DEF_H
#define GSCONNECTIONPOOL_DEF_H

#include "DBAccessApi.h"

#include "TypeDef.h"

class CConnectionPool : public IConnectionPool
{
public:
	// 构造函数
	CConnectionPool( EnumDriverType eType );

	// 析构函数
	virtual ~CConnectionPool( void );

	// 设置参数
	// szHost:     数据库主机,本地数据数据库填NULL
	// szDataBase: 数据库名
	// szUserName: 连接数据库用户名
	// szPassword: 连接数据库密码
	// iPort:      数据库监听端口,默认值0表示使用数据库默认端口
	// iMinConns:  最小连接数
	// iMaxConns:  最大连接数
	virtual void SetParams( const char* szHost,
		const char* szDataBase,
		const char* szUserName,
		const char* szPassword,
		unsigned short iPort = 0,
		unsigned int iMinConns = 1,
		unsigned int iMaxConns = 10 );

	// 获取连接
	// eError: 获取到连接为空时，可根据错误码来查看原因
	virtual IConnection* GetConnection( EnumDBApiRet* eError=NULL );

	// 释放连接
	virtual	void ReleaseConnection( IConnection** pcsConn );

private:
	// 错误检测
	BOOL IsNoError( void );

	// 创建连接
	IConnection* CreateConnection( void );

	// 销毁连接
	void DestroyConnection( IConnection* pcsConn );

	// 初始化列表
	BOOL InitList( void );

	// 添加连接到连接列表
	void AddList( IConnection* pcsConn );
	
	// 清除连接列表
	void ClearList( void );

	// 是否在列表中
	BOOL IsListItem( IConnection* pcsConn );

	// 获取空闲连接
	IConnection* GetIdle( void );
	
	// 设为空闲连接
	void SetIdle( IConnection* pcsConn );

	// 列表是否为空
	BOOL IsListEmpty( void );

	// 是否达到最大连接数
	BOOL IsOverMaxLink( void );

private:
	EnumDriverType m_eType;

	tstring  m_strHost;
	tstring  m_strDataBase;
	tstring  m_strUserName;
	tstring  m_strPassword;
	UInt16  m_iPort;
	UInt32  m_iMinConns;
	UInt32  m_iMaxConns;
	UInt32  m_iUsedConns;

	EnumDBApiRet m_eError;

	typedef struct StruConnInfo
	{
		BOOL bIdle;
		IConnection* pConn;
	};

	// <连接指针, 是否空闲> TRUE空闲, FALSE非空闲
	typedef std::vector<StruConnInfo>  ConnectionList;
	ConnectionList m_vecConnectionList;

	// 锁
	Mutex m_csMutex;

	Mutex m_csMutexInit;
};

#endif // CONNECTIONPOOL_DEF_H
