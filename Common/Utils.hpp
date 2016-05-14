#ifndef __MYCPP_COMMON_COMMON_H__
#define __MYCPP_COMMON_COMMON_H__

#include "mytypes.h"

#if defined(_MSWINDOWS_)
#include <WinSock2.h>
#include <WS2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")
#else
#include <unistd.h>
#include <stdarg.h>
#include <string.h>
#include <arpa/inet.h>
#endif

#include "Singleton.hpp"

namespace mycpp
{
	class Utils : public Singleton<Utils>
	{
		friend class Singleton<Utils>;

	public:
		// 休眠函数，单位: 毫秒
		void MySleep(UInt32 milliSeconds)
		{
#if defined(_MSWINDOWS_)
			Sleep(milliSeconds);
#else
			usleep(milliSeconds*1000);
#endif
		}

		// 查找本地端口占用情况
		bool IsPortBusy(int port, const std::string& ip = "")
		{
#ifdef  _MSWINDOWS_
			WSADATA wsa = { 0 };
			if (0 != WSAStartup(MAKEWORD(2, 2), &wsa))
			{
				return false;
			}
#endif
			SOCKET sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
			if (INVALID_SOCKET == sockfd)
			{
#ifdef _MSWINDOWS_
				WSACleanup();
#endif
				return false;
			}
			sockaddr_in service;
			service.sin_family = AF_INET;
			if (ip.empty())
			{
				service.sin_addr.s_addr = ::htonl(INADDR_ANY);
			}
			else
			{
				struct in_addr s;
				inet_pton(AF_INET, ip.c_str(), (void*)&s);
				service.sin_addr.s_addr = s.S_un.S_addr;
			}
			service.sin_port = htons(port);
			int ret = ::bind(sockfd, (struct sockaddr*)&service, sizeof(service));
#ifdef _MSWINDOWS_
			closesocket(sockfd);
			WSACleanup();
#else
			close(sockfd);
#endif
			return (0 != ret) ? true : false;
		}

	};
}

// 单例缩写语法糖
#define UTILS() mycpp::Utils::Instance()

#endif // MYCPP_COMMON_COMMON_H 
