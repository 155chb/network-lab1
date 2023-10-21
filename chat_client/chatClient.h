#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include "../lib/chatProtocol.h"
#include "../lib/userList.h"
#include "msgBuffer.h"
#include "simpleUI.h"
extern int chattingID;
extern void refreshChatPage();

SOCKET sockClient = INVALID_SOCKET;
SOCKADDR_IN addrSrv;
int nextMsgRecv = 1;
int nextComRecv = 1;
int msgRecv = 0;

DWORD WINAPI RecvThread(LPVOID param);
int sockInit();
void sockClean();
int sockConnect();
int login();
int logout();
void update(_tagChatProtocol recvMsg);
void recvChatMsg(_tagChatProtocol recvMsg);

// 线程函数，用于接收服务端消息并处理的线程
DWORD WINAPI RecvThread(LPVOID param)
{
	_tagChatProtocol recvMsg;
	while (nextMsgRecv)
	{
		if (recv(sockClient, (char*)&recvMsg, PROTOCOL_SIZE, 0) == SOCKET_ERROR && WSAGetLastError() == WSAETIMEDOUT)
		{
			continue;
		}
		simpleDecoding((uint16_t*)&recvMsg, PROTOCOL_SIZE);
		if (calculateChecksum((uint16_t*)&recvMsg) != 0)
		{
			printf("Checksum error! Received a wrong Msg!\n");
			continue;
		}
		if (strcmp(recvMsg.msgtype, "update") == 0)
		{
			update(recvMsg);
		}
		else if (strcmp(recvMsg.msgtype, "accept") == 0)
		{
			msgRecv = 1;
		}
		else if (strcmp(recvMsg.msgtype, "chat") == 0)
		{
			recvChatMsg(recvMsg);
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
	sockClient = socket(AF_INET, SOCK_STREAM, 0);
	if (sockClient == INVALID_SOCKET)
	{
		printf("socket failed! sockClient\n");
		return -1;
	}


	struct in_addr ipv4Address;
	if (inet_pton(AF_INET, "10.136.102.151", &ipv4Address) <= 0) {
		printf("Invalid IP address\n");
		return -1;
	}
	addrSrv.sin_addr = ipv4Address;
	addrSrv.sin_family = AF_INET;
	addrSrv.sin_port = htons(6000);

	int timeout = 5000;
	setsockopt(sockClient, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(int));

	char ipBuffer[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &(addrSrv.sin_addr), ipBuffer, INET_ADDRSTRLEN);
	printf("Server IP: %s\n", ipBuffer);
	printf("Server Port: %d\n", ntohs(addrSrv.sin_port));

	return 0;
}

void sockClean()
{
	if (sockClient != INVALID_SOCKET)
		closesocket(sockClient);
	WSACleanup();
}

int sockConnect()
{
	return connect(sockClient, (SOCKADDR*)&addrSrv, sizeof(SOCKADDR));
}

int login()
{
	char name[20];

	printf("Enter your name: ");
	scanf_s("%s", name, 20);
	name[19] = '\0';
	if (strnlen_s(name, 20) > 15)
	{
		printf("Name too long! Please reinput\n");
		return -1;
	}

	struct _tagChatProtocol sendMsg = createChatProtocol("login", name, 0, 0);
	send(sockClient, (char*)&sendMsg, PROTOCOL_SIZE, 0);
	struct _tagChatProtocol recvMsg;
	initUserList();
	while (true)
	{
		if (recv(sockClient, (char*)&recvMsg, PROTOCOL_SIZE, 0) == SOCKET_ERROR && WSAGetLastError() == WSAETIMEDOUT)
		{
			printf("Recv timeout!\n");
			return -1;
		}
		simpleDecoding((uint16_t*)&recvMsg, PROTOCOL_SIZE);
		if (calculateChecksum((uint16_t*)&recvMsg) != 0)
		{
			printf("Checksum error! Received a wrong Msg!\n");
			return -1;
		}
		struct userInfoMsg* userInfoMsgs = (struct userInfoMsg*)&recvMsg.message;
		for (int i = 0; i < MESSAGE_SIZE / sizeof(userInfoMsg); i++, userInfoMsgs++)
		{
			if (strcmp(userInfoMsgs->tag, "err") == 0)
			{
				printf("Name is taken\n");
				return -1;
			}
			else if (strcmp(userInfoMsgs->tag, "ful") == 0)
			{
				printf("Online capacity has been reached!\n");
				return -1;
			}
			else if (strcmp(userInfoMsgs->tag, "end") == 0)
			{
				return 0;
			}
			else if (strcmp(userInfoMsgs->tag, "add") == 0)
			{
				addUser(userInfoMsgs->userID, userInfoMsgs->userName, INVALID_SOCKET);
				if (strcmp(userInfoMsgs->userName, name) == 0)
				{
					localID = userInfoMsgs->userID;
				}
			}
		}
	}
}

int logout()
{
	struct _tagChatProtocol sendMsg = createChatProtocol("logout", "", localID, 0);
	msgRecv = 0;
	while (!msgRecv)
	{
		if (send(sockClient, (char*)&sendMsg, sizeof(_tagChatProtocol), 0) == SOCKET_ERROR && WSAGetLastError() == WSAETIMEDOUT)
		{
			printf("Send timeout!\n");
			return 0;
		}
		Sleep(100);
	}
	return 0;
}

void update(_tagChatProtocol recvMsg)
{
	userInfoMsg* message = (userInfoMsg*)&recvMsg.message;
	if (strcmp(message->tag, "add") == 0)
	{
		addUser(message->userID, message->userName, INVALID_SOCKET);
	}
	else if (strcmp(message->tag, "del") == 0)
	{
		deleteUser(message->userID);
	}
	return;
}

void recvChatMsg(_tagChatProtocol recvMsg)
{
	int senderID = recvMsg.sender_id;
	int receiverID = recvMsg.receiver_id;
	if (receiverID == localID)
	{
		addMsg(senderID, &recvMsg, 1);
	}
	else if (receiverID == 0)
	{
		addMsg(0, &recvMsg, 1);
		if (chattingID == 0)
		{
			refreshChatPage();
		}
	}
	if (chattingID == senderID)
	{
		refreshChatPage();
	}
	return;
}