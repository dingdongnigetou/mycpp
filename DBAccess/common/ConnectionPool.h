
#ifndef __GSCONNECTIONPOOL_DEF_H__
#define __GSCONNECTIONPOOL_DEF_H__

#include "DBAccessApi.h"
#include "TypeDef.h"
#include "Mutex/MyMutex.hpp"

#include <vector>

class CConnectionPool : public IConnectionPool
{
public:
	// ���캯��
	CConnectionPool( EnumDriverType eType );

	// ��������
	virtual ~CConnectionPool();

	// ���ò���
	// szHost:     ���ݿ�����,�����������ݿ���nullptr
	// szDataBase: ���ݿ���
	// szUserName: �������ݿ��û���
	// szPassword: �������ݿ�����
	// iPort:      ���ݿ�����˿�,Ĭ��ֵ0��ʾʹ�����ݿ�Ĭ�϶˿�
	// iMinConns:  ��С������
	// iMaxConns:  ���������
	virtual void SetParams( const char* szHost,
		const char* szDataBase,
		const char* szUserName,
		const char* szPassword,
		unsigned short iPort = 0,
		unsigned int iMinConns = 1,
		unsigned int iMaxConns = 10 );

	// ��ȡ����
	// eError: ��ȡ������Ϊ��ʱ���ɸ��ݴ��������鿴ԭ��
	virtual IConnection* GetConnection( EnumDBApiRet* eError=nullptr );

	// �ͷ�����
	virtual	void ReleaseConnection( IConnection** pcsConn );
	
	EnumDriverType GetDriverType();

private:
	// ������
	bool isNoError();

	// ��������
	IConnection* createConnection();

	// ��������
	void destroyConnection( IConnection* pcsConn );

	// ��ʼ���б�
	bool initList();

	// ������ӵ������б�
	void addList( IConnection* pcsConn );
	
	// ��������б�
	void clearList();

	// �Ƿ����б���
	bool isListItem( IConnection* pcsConn );

	// ��ȡ��������
	IConnection* getIdle();
	
	// ��Ϊ��������
	void setIdle( IConnection* pcsConn );

	// �б��Ƿ�Ϊ��
	bool isListEmpty();

	// �Ƿ�ﵽ���������
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

	// <����ָ��, �Ƿ����> true����, false�ǿ���
	typedef std::vector<StruConnInfo>  ConnectionList;
	ConnectionList vecConnectionList_;

	// ��
	mycpp::MyMutex csMutex_;
	mycpp::MyMutex csMutexInit_;
};

#endif // __CONNECTIONPOOL_DEF_H__
