#include <stdio.h>
#include <stdlib.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include "../lib/chatProtocol.h"
#include "../lib/userList.h"
#include "simpleUI.h"
#include "chatClient.h"

int main()
{
	if (sockInit() != 0)
	{
		printf("\nsockInit failed!\n");
		sockClean();
		system("pause");
		return -1;
	}

	if (sockConnect() != 0)
	{
		printf("connect failed!\n");
		sockClean();
		system("pause");
		return -1;
	}
	printf("connect success!\n");
	system("pause");

	while (login() != 0)
		system("pause");

	// 创建接受服务器消息并处理的线程
	HANDLE recvThread = CreateThread(NULL, 0, RecvThread, NULL, 0, NULL);

	initUI();
	while (nextComRecv)
	{
		homePage();
	}

	if (recvThread != NULL)
	{
		WaitForSingleObject(recvThread, INFINITE);
		CloseHandle(recvThread);
	}

	return 0;
}