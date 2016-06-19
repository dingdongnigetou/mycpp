
#ifndef __GSCONNECTIONPOOL_DEF_H__
#define __GSCONNECTIONPOOL_DEF_H__

#include "DBAccessApi.h"
#include "TypeDef.h"
#include "Mutex/MyMutex.hpp"

#include <vector>

class CConnectionPool : public IConnectionPool
{
public:
	// 构造函数
	CConnectionPool( EnumDriverType eType );

	// 析构函数
	virtual ~CConnectionPool();

	// 设置参数
	// szHost:     数据库主机,本地数据数据库填nullptr
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
	virtual IConnection* GetConnection( EnumDBApiRet* eError=nullptr );

	// 释放连接
	virtual	void ReleaseConnection( IConnection** pcsConn );
	
	EnumDriverType GetDriverType();

private:
	// 错误检测
	bool isNoError();

	// 创建连接
	IConnection* createConnection();

	// 销毁连接
	void destroyConnection( IConnection* pcsConn );

	// 初始化列表
	bool initList();

	// 添加连接到连接列表
	void addList( IConnection* pcsConn );
	
	// 清除连接列表
	void clearList();

	// 是否在列表中
	bool isListItem( IConnection* pcsConn );

	// 获取空闲连接
	IConnection* getIdle();
	
	// 设为空闲连接
	void setIdle( IConnection* pcsConn );

	// 列表是否为空
	bool isListEmpty();

	// 是否达到最大连接数
	bool isOverMaxLink();

private:
	EnumDriverType eType_;

	std::string  strHost_;
	std::string  strDataBase_;
	std::string  strUserName_;
	std::string  strPassword_;
	UInt16  iPort_;
	UInt32  iMinConns_;
	UInt32  iMaxConns_;
	UInt32  iUsedConns_;

	EnumDBApiRet eError_;

	struct StruConnInfo
	{
		bool bIdle;
		IConnection* pConn;
	};

	// <连接指针, 是否空闲> true空闲, false非空闲
	typedef std::vector<StruConnInfo>  ConnectionList;
	ConnectionList vecConnectionList_;

	// 锁
	mycpp::MyMutex csMutex_;
	mycpp::MyMutex csMutexInit_;
};

#endif // __CONNECTIONPOOL_DEF_H__
