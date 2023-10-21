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

    // ��������������߳�
    HANDLE inputThread = CreateThread(NULL, 0, InputThread, NULL, 0, NULL);

    // ���������ͻ�������������߳�
    HANDLE socketThread = CreateThread(NULL, 0, ListenThread, NULL, 0, NULL);

    // �ȴ������߳��˳�
    if (inputThread != NULL)
    {
        WaitForSingleObject(inputThread, INFINITE);
        CloseHandle(inputThread);
    }

    // �ȴ������߳��˳�
    if (socketThread != NULL)
    {
		CloseHandle(socketThread);
	}

    sockClean();

    return 0;
}
