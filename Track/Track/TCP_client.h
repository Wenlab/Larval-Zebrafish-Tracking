#pragma once
#ifndef _TCP_CLIENT
#define _TCP_CLIENT

#define WIN32_LEAN_AND_MEAN
#include <iostream>
#include <WinSock2.h>
#include <windows.h>
#include <Ws2tcpip.h>
#include <cstring>
#include <string>
#pragma comment(lib,"ws2_32.lib")


class TCP_client
{
public:
	TCP_client(char* Addrserver) :addrserver(Addrserver) {

	}
	TCP_client()
	{
		addrserver = localhost;
	}
	void initialize();

	bool createSocketConnect();

	void sendMsg(char* data, int datasize);
	//bool recvMsg();

	void close();

	char recvBuff[1024];

	char localhost[50] = "169.254.157.190";;
	char* addrserver;
	SOCKADDR_IN addrServer;
	SOCKET socketClient;

};




























#endif