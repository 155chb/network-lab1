#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <WinSock2.h>
#include <Ws2tcpip.h>
#include "../lib/chatProtocol.h"
#include "../lib/userList.h"

SOCKET sockSrv;
int connecting;
int nextConnAccp;
DWORD WINAPI InputThread(LPVOID param);
DWORD WINAPI ListenThread(LPVOID param);
DWORD WINAPI RequestThread(LPVOID param);
int sockInit();
void sockClean();
void login(SOCKET sockConn, _tagChatProtocol recvMsg);
void logout(_tagChatProtocol recvMsg);
void chat(_tagChatProtocol recvMsg);

// 线程函数，用于监听命令行输入
DWORD WINAPI InputThread(LPVOID param)
{
    char input[100];
    nextConnAccp = 1; // 设置允许连接标志
    while (1)
    {
        fgets(input, sizeof(input), stdin);
        if (strcmp(input, "exit\n") == 0)
        {
            nextConnAccp = 0; // 清除允许连接标志
            while (connecting); // 等待连接结束
            break;
        }
    }
    return 0;
}

// 线程函数，用于监听客户端连接请求
DWORD WINAPI ListenThread(LPVOID param)
{
    connecting = 0; // 标记未开始连接

    listen(sockSrv, 5);

    while (nextConnAccp)
    {
        SOCKET sockConn = accept(sockSrv, NULL, NULL);
        if (sockConn == INVALID_SOCKET)
        {
            printf("accept failed!\n");
            continue;
        }
        connecting = 1; // 设置连接标志
        HANDLE requestthread = CreateThread(NULL, 0, RequestThread, (LPVOID)sockConn, 0, NULL);
        if (requestthread != NULL)
        {
            CloseHandle(requestthread);
        }
        connecting = 0; // 清除连接标志
    }
    return 0;
}

// 线程函数，用于执行客户端连接请求的线程
DWORD WINAPI RequestThread(LPVOID param)
{
    SOCKET sockConn = (SOCKET)param;
    _tagChatProtocol recvMsg;
    while (true)
    {
        recv(sockConn, (char*)&recvMsg, PROTOCOL_SIZE, 0);
        simpleDecoding((uint16_t*)&recvMsg, PROTOCOL_SIZE);
        if (calculateChecksum((uint16_t*)&recvMsg) != 0)
            continue;
        if (strcmp(recvMsg.msgtype, "login") == 0)
        {
            login(sockConn, recvMsg);
        }
        else if (strcmp(recvMsg.msgtype, "logout") == 0)
		{
            logout(recvMsg);
            return 0;
		}
        else if (strcmp(recvMsg.msgtype, "chat") == 0)
        {
            chat(recvMsg);
        }
    }
    return 0;
}

int sockInit()
{
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData))
    {
        printf("WSAStartup failed!\n");
        return -1;
    }
    sockSrv = socket(AF_INET, SOCK_STREAM, 0);
    if (sockSrv == INVALID_SOCKET)
    {
        printf("socket failed!\n");
        return -1;
    }
    SOCKADDR_IN addrSrv;
    struct in_addr ipv4Address;
    if (inet_pton(AF_INET, "10.136.102.151", &ipv4Address) <= 0) {
        printf("Invalid IP address\n");
        system("pause");
        return -1;
    }
    addrSrv.sin_addr = ipv4Address;
    addrSrv.sin_family = AF_INET;
    addrSrv.sin_port = htons(6000);
    if (bind(sockSrv, (SOCKADDR*)&addrSrv, sizeof(SOCKADDR)) != 0)
    {
        printf("bind failed!\n");
        return -1;
    }

    char ipBuffer[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(addrSrv.sin_addr), ipBuffer, INET_ADDRSTRLEN);
    printf("Server IP: %s\n", ipBuffer);
    printf("Server Port: %d\n", ntohs(addrSrv.sin_port));

    return 0;
}

void sockClean()
{
    if (sockSrv != INVALID_SOCKET)
        closesocket(sockSrv);
    for (int i = 0; i < MAXUSER; i++)
	{
        if (globalUserList.list[i].id != -1)
        {
            closesocket(globalUserList.list[i].sock);
        }
	}
    WSACleanup();
}

void login(SOCKET sockConn, _tagChatProtocol recvMsg)
{
    _tagChatProtocol sendMsg;
    if (globalUserList.userCount == MAXUSER)
    {
        sendMsg = createChatProtocol("login", "ful", 0, -1);
        send(sockConn, (char*)&sendMsg, PROTOCOL_SIZE, 0);
        return;
    }
    if (getUserID(recvMsg.message) != -1)
    {
        sendMsg = createChatProtocol("login", "err", 0, -1);
        send(sockConn, (char*)&sendMsg, PROTOCOL_SIZE, 0);
        return;
    }
    addUser(currentID, recvMsg.message, sockConn);
    currentID++;
    struct userInfoMsg message[MESSAGE_SIZE / sizeof(userInfoMsg)];
    memset(message, 0, sizeof(message));
    int messageUsed = 0;
    for (int i = 0; i < MAXUSER; i++)
	{
        if (globalUserList.list[i].id != -1)
        {
            memcpy_s(message[messageUsed].tag, TAGLEN, "add", TAGLEN);
            message[messageUsed].userID = globalUserList.list[i].id;
            memcpy_s(message[messageUsed].userName, NAMELEN, globalUserList.list[i].name, NAMELEN);
            messageUsed++;
            if (messageUsed == MESSAGE_SIZE / sizeof(userInfoMsg))
            {
                sendMsg = createChatProtocol("login", (char*)message, 0, getUserID(recvMsg.message));
                send(sockConn, (char*)&sendMsg, PROTOCOL_SIZE, 0);
                messageUsed = 0;
            }
        }
	}
    memcpy_s(&message[messageUsed].tag, TAGLEN, "end", TAGLEN);
    sendMsg = createChatProtocol("login", (char*)message, 0, getUserID(recvMsg.message));
    send(sockConn, (char*)&sendMsg, PROTOCOL_SIZE, 0);

    for (int i = 0; i < MAXUSER; i++)
    {
		if (globalUserList.list[i].id != -1 && globalUserList.list[i].id != getUserID(recvMsg.message))
		{
            memcpy_s(message[0].tag, TAGLEN, "add", TAGLEN);
			message[0].userID = getUserID(recvMsg.message);
			memcpy_s(message[0].userName, NAMELEN, recvMsg.message, NAMELEN);
			sendMsg = createChatProtocol("update", (char*)message, 0, globalUserList.list[i].id);
			send(globalUserList.list[i].sock, (char*)&sendMsg, PROTOCOL_SIZE, 0);
		}
	}
    return;
}

void logout(_tagChatProtocol recvMsg)
{
    int senderID = recvMsg.sender_id;
	_tagChatProtocol sendMsg = createChatProtocol("accept", "", 0, senderID);
    send(getUserSock(senderID), (char*)&sendMsg, PROTOCOL_SIZE, 0);
    closesocket(getUserSock(senderID));
    deleteUser(senderID);
    struct userInfoMsg message;
    for (int i = 0; i < MAXUSER; i++)
    {
        if (globalUserList.list[i].id != -1)
        {
            memcpy_s(message.tag, TAGLEN, "del", TAGLEN);
            message.userID = senderID;
            memcpy_s(message.userName, NAMELEN, "", NAMELEN);
            sendMsg = createChatProtocol("update", (char*)&message, 0, globalUserList.list[i].id);
            send(globalUserList.list[i].sock, (char*)&sendMsg, PROTOCOL_SIZE, 0);
        }
    }
    return;
}

void chat(_tagChatProtocol recvMsg)
{
    int senderID = recvMsg.sender_id;
    int receiverID = recvMsg.receiver_id;
    SOCKET sendSock = getUserSock(senderID);
    SOCKET recvSock;
    _tagChatProtocol sendMsg = createChatProtocol("accept", "", 0, senderID);
    send(sendSock, (char*)&sendMsg, PROTOCOL_SIZE, 0);
    simpleEncoding((uint16_t*)&recvMsg, PROTOCOL_SIZE);
    if (receiverID == 0)
	{
		for (int i = 0; i < MAXUSER; i++)
		{
			if (globalUserList.list[i].id != -1 && globalUserList.list[i].id != senderID)
			{
                recvSock = globalUserList.list[i].sock;
				send(recvSock, (char*)&recvMsg, PROTOCOL_SIZE, 0);
			}
		}
	}
    else
    {
        recvSock = getUserSock(receiverID);
        send(recvSock, (char*)&recvMsg, PROTOCOL_SIZE, 0);
    }

    return;
}