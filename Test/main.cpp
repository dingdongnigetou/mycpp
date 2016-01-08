
#include <time.h>
#include <iostream>
using namespace std;

#include "DBAccessApi.h"

#ifdef _DEBUG
#pragma comment(lib,"../DBAccess/bin/Debug/DBAccessD.lib")
#else
#pragma comment(lib,"../DBAccess/bin/Release/DBAccess.lib")
#endif

int main(int argc, char* argv[])
{
	IConnectionPool *pcsPool = CreateDBConnectionPool(ADO);
	
	// 设置连接参数
	pcsPool->SetParams( "192.168.26.24",
		"scdata_tz",
		"sa",
		"12345qwert",0,3,8);

	// 获取连接
	EnumDBApiRet eError = RETCODE_UNKNOWN_ERROR;
	IConnection *pcsConn = pcsPool->GetConnection(&eError);
	if ( !pcsConn )
	{
		printf("GetConnection() fail! error code: %d \n", eError);
	}


	auto b = clock();
	auto pSet = pcsConn->ExecuteQuery("select * from dic");
	cout << (clock() - b) << endl;

	double dVal = 0.0;
	float fVal = 0.0;
	int iVal = 0;
	char tVal = 0;
	char szResult[1024] = { 0 };
	unsigned int iFactLen = 0;
	while (!pSet->Eof())
	{
		if (pSet->GetValue("Name", szResult, sizeof(szResult), &iFactLen))
			printf("%s: ", szResult);
		::memset(szResult, 0x0, sizeof(szResult));

		if (pSet->GetValue("Adjust", &dVal, sizeof(dVal), &iFactLen))
			printf("%f ", dVal);

		if (pSet->GetValue("ModiValue", &tVal, sizeof(tVal), &iFactLen))
			printf("%d\n", tVal);

		getchar();
		pSet->MoveNext();
	}

	system("pause");

	return 0;
}
