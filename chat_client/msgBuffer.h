#pragma once
#include <stdio.h>
#include <stdlib.h>
#include "../lib/userList.h"

#define MSGBUFFER_SIZE 20

struct chatMsg
{
	char msg[MESSAGE_SIZE];
	int senderID;
	int receiverID;
	int sendSuccess;
};

struct msgBuffer
{
	struct chatMsg buffer[MSGBUFFER_SIZE];
	int storeNum;
	int unreadNum;
};

struct msgBuffer globalMsgBufferList[MAXUSER];

void initMsgBuffer()
{
	for (int i = 1; i < MAXUSER; i++)
	{
		globalMsgBufferList[i].storeNum = 0;
		globalMsgBufferList[i].unreadNum = 0;
	}
}

int addMsg(int userID, _tagChatProtocol* msg, int sendSuccess)
{
	int index = userID == 0 ? getUserIndex(localID) : getUserIndex(userID);
	if (index == -1)
	{
		printf("addMsg: user not exist\n");
		return -1;
	}
	int startPos = globalMsgBufferList[index].storeNum % MSGBUFFER_SIZE;
	memcpy_s(globalMsgBufferList[index].buffer[startPos].msg, MESSAGE_SIZE, msg->message, MESSAGE_SIZE);
	globalMsgBufferList[index].buffer[startPos].senderID = msg->sender_id;
	globalMsgBufferList[index].buffer[startPos].receiverID = msg->receiver_id;
	globalMsgBufferList[index].buffer[startPos].sendSuccess = sendSuccess;
	globalMsgBufferList[index].storeNum++;
	globalMsgBufferList[index].unreadNum++;
	return 0;
}

int clearBuffer(int userID)
{
	int index = getUserIndex(userID);
	if (index == -1)
	{
		printf("addMsg: user not exist\n");
		return -1;
	}
	globalMsgBufferList[index].storeNum = 0;
	return 0;
}

int printMsgBuffer(int userID)
{
	int index = userID == 0 ? getUserIndex(localID) : getUserIndex(userID);
	if (index == -1)
	{
		printf("addMsg: user not exist\n");
		return -1;
	}

	int printNum = globalMsgBufferList[index].storeNum < 20 ? globalMsgBufferList[index].storeNum : 20;
	int printIndex = globalMsgBufferList[index].storeNum < 20 ? 0 : globalMsgBufferList[index].storeNum % MSGBUFFER_SIZE;
	char senderName[NAMELEN];

	for (int i = 0; i < printNum; i++)
	{
		if (getUserName(globalMsgBufferList[index].buffer[printIndex].senderID, senderName) == -1)
		{
			memcpy_s(senderName, NAMELEN, "unknown", strlen("unknown"));
		}
		if (globalMsgBufferList[index].buffer[printIndex].sendSuccess)
		{
			printf("%s>\t\n%s", senderName, globalMsgBufferList[index].buffer[printIndex].msg);
		}
		else
		{
			printf("%s>\t @@ Send Failed!\n%s", senderName, globalMsgBufferList[index].buffer[printIndex].msg);
		}
		printIndex = (printIndex + 1) % MSGBUFFER_SIZE;
	}
	globalMsgBufferList[index].unreadNum = 0;
	return 0;
}

void printUserListMsg()
{
	printf("User List:\t%d/%d\n", globalUserList.userCount, MAXUSER);
	int onlineUser = 0;
	for (int i = 0; i < MAXUSER; i++)
	{
		if (globalUserList.list[i].id != -1)
		{
			onlineUser++;
			if (globalUserList.list[i].id == localID)
			{
				printf("%d:\t%s\t\t\t<0>\n", onlineUser, globalUserList.list[i].name);
			}
			else
			{
				printf("%d:\t%s\t\t\t(%d)\n", onlineUser, globalUserList.list[i].name, globalMsgBufferList[i].unreadNum);
			}
		}
	}
	return;
}