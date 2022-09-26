#include "TCP_client.h"

void TCP_client::initialize() {

	WSADATA data;

	if (WSAStartup(MAKEWORD(2, 2), &data) != 0)
	{
		std::cout << "WSAStartup error";
		system("pause");
		return;
	}

}


bool TCP_client::createSocketConnect()
{
	addrServer.sin_family = AF_INET;
	addrServer.sin_port = htons(11000);
	inet_pton(AF_INET, addrserver, (void*)&addrServer.sin_addr.S_un.S_addr);


	socketClient = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (socketClient == INVALID_SOCKET)
	{
		std::cout << "socket create error" << std::endl;
		//system("pause");
		return false;
	}

	if (connect(socketClient, (struct sockaddr*)&addrServer, sizeof(addrServer)) == INVALID_SOCKET)
	{
		std::cout << "connect error" << std::endl;
		//system("pause");
		return false;
	}
	else
	{
		return true;
	}
}



void TCP_client::sendMsg(char* data, int datasize)
{
	send(socketClient, data, datasize, 0);
}


void TCP_client::close()
{
	closesocket(socketClient);

	//WSACleanup();

}