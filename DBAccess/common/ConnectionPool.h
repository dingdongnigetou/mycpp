
#ifndef __GSCONNECTIONPOOL_DEF_H__
#define __GSCONNECTIONPOOL_DEF_H__

#include "DBAccessApi.h"
#include "TypeDef.h"
#include "MyMutex.hpp"

#include <vector>

class CConnectionPool : public IConnectionPool
{
public:
	// ���캯��
	CConnectionPool( EnumDriverType eType );

	// ��������
	virtual ~CConnectionPool( void );

	// ���ò���
	// szHost:     ���ݿ�����,�����������ݿ���NULL
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
	virtual IConnection* GetConnection( EnumDBApiRet* eError=NULL );

	// �ͷ�����
	virtual	void ReleaseConnection( IConnection** pcsConn );

	EnumDriverType GetDriverType();

private:
	// ������
	BOOL IsNoError( void );

	// ��������
	IConnection* CreateConnection( void );

	// ��������
	void DestroyConnection( IConnection* pcsConn );

	// ��ʼ���б�
	BOOL InitList( void );

	// ������ӵ������б�
	void AddList( IConnection* pcsConn );
	
	// ��������б�
	void ClearList( void );

	// �Ƿ����б���
	BOOL IsListItem( IConnection* pcsConn );

	// ��ȡ��������
	IConnection* GetIdle( void );
	
	// ��Ϊ��������
	void SetIdle( IConnection* pcsConn );

	// �б��Ƿ�Ϊ��
	BOOL IsListEmpty( void );

	// �Ƿ�ﵽ���������
	BOOL IsOverMaxLink( void );

private:
	EnumDriverType m_eType;

	std::string  m_strHost;
	std::string  m_strDataBase;
	std::string  m_strUserName;
	std::string  m_strPassword;
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

	// <����ָ��, �Ƿ����> TRUE����, FALSE�ǿ���
	typedef std::vector<StruConnInfo>  ConnectionList;
	ConnectionList m_vecConnectionList;

	// ��
	mycpp::MyMutex m_csMutex;
	mycpp::MyMutex m_csMutexInit;
};

#endif // __CONNECTIONPOOL_DEF_H__
