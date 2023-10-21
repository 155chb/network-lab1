#include <stdio.h>
#include <stdlib.h>
#include <WinSock2.h>
#include <Ws2tcpip.h>
#include "../lib/chatProtocol.h"
#include "../lib/userList.h"
#include "chatServer.h"

int main()
{
    if(sockInit() != 0)
	{
		printf("socket init failed\n");
        sockClean();
		return -1;
	}

    initUserList();

    // 创建监听输入的线程
    HANDLE inputThread = CreateThread(NULL, 0, InputThread, NULL, 0, NULL);

    // 创建监听客户端连接请求的线程
    HANDLE socketThread = CreateThread(NULL, 0, ListenThread, NULL, 0, NULL);

    // 等待输入线程退出
    if (inputThread != NULL)
    {
        WaitForSingleObject(inputThread, INFINITE);
        CloseHandle(inputThread);
    }

    // 等待连接线程退出
    if (socketThread != NULL)
    {
		CloseHandle(socketThread);
	}

    sockClean();

    return 0;
}
