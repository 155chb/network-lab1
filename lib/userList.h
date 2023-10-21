#pragma once
#include <WinSock2.h>
#include <string.h>

#define MAXUSER 5
#define NAMELEN 16
#define TAGLEN 4

int currentID = 1;
int localID = 0;

struct userInfo
{
	int id;
	char name[NAMELEN];
	SOCKET sock;
};

struct userList
{
	struct userInfo list[MAXUSER];
	int userCount;
};

struct userInfoMsg
{
	char tag[TAGLEN];
	int userID;
	char userName[NAMELEN];
};

struct userList globalUserList;

void initUserList();
int addUser(int id, char* name, SOCKET sock);
int deleteUser(int id);
int getUserCount();
int getUserID(char* name);
int getUserName(int id, char* name);
void printUserList();

void initUserList()
{
	for (int i = 0; i < MAXUSER; i++)
	{
		globalUserList.list[i].id = -1;
		memset(globalUserList.list[i].name, 0, NAMELEN);
		globalUserList.list[i].sock = INVALID_SOCKET;
	}
	globalUserList.userCount = 0;
}

int addUser(int id, char* name, SOCKET sock)
{
	if (globalUserList.userCount == MAXUSER)
	{
		return -1;
	}
	for (int i = 0; i < MAXUSER; i++)
	{
		if (globalUserList.list[i].id == -1)
		{
			globalUserList.list[i].id = id;
			memcpy_s(globalUserList.list[i].name, NAMELEN, name, NAMELEN);
			globalUserList.list[i].sock = sock;
			break;
		}
	}
	globalUserList.userCount++;
	return 0;
}

int deleteUser(int id)
{
	for (int i = 0; i < MAXUSER; i++)
	{
		if (globalUserList.list[i].id == id)
		{
			globalUserList.list[i].id = -1;
			memset(globalUserList.list[i].name, 0, NAMELEN);
			globalUserList.list[i].sock = INVALID_SOCKET;
			globalUserList.userCount--;
			return 0;
		}
	}
	return -1;
}

int getUserCount()
{
	return globalUserList.userCount;
}

int getUserIndex(int id)
{
	for (int i = 0; i < MAXUSER; i++)
	{
		if (globalUserList.list[i].id == id)
		{
			return i;
		}
	}
	return -1;
}

int getUserID(char* name)
{
	for (int i = 0; i < MAXUSER; i++)
	{
		if (strcmp(globalUserList.list[i].name, name) == 0)
		{
			return globalUserList.list[i].id;
		}
	}
	return -1;
}

int getUserName(int id, char* name)
{
	for (int i = 0; i < MAXUSER; i++)
	{
		if (globalUserList.list[i].id == id)
		{
			memcpy_s(name, NAMELEN, globalUserList.list[i].name, NAMELEN);
			return 0;
		}
	}
	return -1;
}

SOCKET getUserSock(int id)
{
	for (int i = 0; i < MAXUSER; i++)
	{
		if (globalUserList.list[i].id == id)
		{
			return globalUserList.list[i].sock;
		}
	}
	return INVALID_SOCKET;
}

void printUserList()
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
				printf("%d:\t%s  <0>\n", onlineUser, globalUserList.list[i].name);
			}
			else
			{
				printf("%d:\t%s\n", onlineUser, globalUserList.list[i].name);
			}
		}
	}
	return;
}