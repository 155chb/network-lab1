#pragma once
#include "chatClient.h"
#include <windows.h>

HANDLE hConsole;
CONSOLE_SCREEN_BUFFER_INFO csbi;
COORD outputPos;
COORD inputPos;
int chattingID = -1;

void initUI();
void clearUpper();
void clearLower();
void homePage();
void chat();
void view();
void exit();
void exeCommand(int commond);
void chatPage(int userID);
void refreshChatPage();

void initUI()
{
    // 获取标准输入和输出的句柄
    hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

    // 获取控制台窗口的大小
    GetConsoleScreenBufferInfo(hConsole, &csbi);

    // 计算上半部分和下半部分的高度
    int screenHeight = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
    int upperHeight = screenHeight * 3 / 4;
    int lowerHeight = screenHeight - upperHeight;

    outputPos.X = 0;
    outputPos.Y = 0;
    inputPos.X = 0;
    inputPos.Y = upperHeight;
}

void clearUpper()
{
	// 清空上半部分窗口
	COORD coord;
	coord.X = 0;
	coord.Y = 0;
	SetConsoleCursorPosition(hConsole, coord);
	for (int i = outputPos.Y; i < inputPos.Y; i++)
	{
		printf("\n");
	}
}

void clearLower()
{
	// 清空下半部分窗口
	COORD coord;
	coord.X = 0;
	coord.Y = inputPos.Y;
	SetConsoleCursorPosition(hConsole, coord);
	for (int i = inputPos.Y; i < csbi.srWindow.Bottom - csbi.srWindow.Top + 1; i++)
	{
		printf("\n");
	}
}

void homePage()
{
	system("cls");

	// 移动光标到上半部分窗口的顶部
    SetConsoleCursorPosition(hConsole, outputPos);

	printf("Welcome to the simpleUI!\n");
	printf("1. Chat\n");
	printf("2. View online users\n");
	printf("0. Exit\n");

    // 移动光标到下半部分窗口的顶部
    SetConsoleCursorPosition(hConsole, inputPos);
    int commond;
    printf("Please input your commond: ");
    scanf_s("%d", &commond);
    exeCommand(commond);
}

void exeCommand(int commond)
{
	switch (commond)
	{
	case 1:
		chat();
		break;
	case 2:
		view();
		break;
	case 0:
		exit();
		break;
	default:
		break;
	}
	return;
}

void chat()
{
	system("cls");
	SetConsoleCursorPosition(hConsole, outputPos);
	printUserListMsg();
	printf("0:\tALL USER(INPUT 0)\t(%d)\n", globalMsgBufferList[getUserIndex(localID)].unreadNum);
	SetConsoleCursorPosition(hConsole, inputPos);
	printf("Please input the user name you want to chat with: ");
	char name[20];
	int id;
	while (true)
	{
		scanf_s("%s", name, 20);
		name[19] = '\0';
		if  (strcmp(name, "0") == 0)
		{
			id = 0;
			break;
		}
		if (getUserID(name) == -1)
		{
			printf("Invalid User Name! Check and Reinput:");
			continue;
		}
		id = getUserID(name);
		if (id == localID)
		{
			printf("You can't chat with yourself! Please reinput:");
			continue;
		}
		break;
	}
	chattingID = id;
	chatPage(id);
	chattingID = -1;
	return;
}

void view()
{
	system("cls");
	printUserList();
	system("pause");
	return;
}

void exit()
{
	logout();
	nextMsgRecv = 0;
	nextComRecv = 0;
	system("cls");
	printf("Expecting for your next visit!\n");
	sockClean();
	system("pause");
	return;
}

void chatPage(int userID)
{
	char userName[NAMELEN];
	char localName[NAMELEN];
	if (getUserName(userID, userName) == -1 && userID != 0)
	{
		printf("Invalid User ID! User May Logout!\n");
		system("pause");
		return;
	}
	if (userID == 0)
	{
		strcpy_s(userName, NAMELEN, "ALL USER");
	}
	if (getUserName(localID, localName) == -1)
	{
		printf("Invalid Local User ID! Something Goes Wrong!\n");
		system("pause");
		return;
	}
	printf("success");
	char msg[MESSAGE_SIZE + 1];
	fgets(msg, MESSAGE_SIZE + 1, stdin);
	while (true)
	{
		system("cls");
		SetConsoleCursorPosition(hConsole, outputPos);

		printf("User Name: %s\n", userName);
		printMsgBuffer(userID);

		CONSOLE_SCREEN_BUFFER_INFO csbi_temp;
		GetConsoleScreenBufferInfo(hConsole, &csbi_temp);
		if (csbi_temp.dwCursorPosition.Y < inputPos.Y - 1)
		{
			SetConsoleCursorPosition(hConsole, inputPos);
		}
		printf("\n");
		printf("%s:>>\n", localName);
		fgets(msg, MESSAGE_SIZE + 1, stdin);
		if (strlen(msg) > MESSAGE_SIZE)
		{
			printf("Message too long! Please reinput:\n");
			continue;
		}
		if (strcmp(msg, "exit\n") == 0)
		{
			break;
		}
		_tagChatProtocol sendMsg = createChatProtocol("chat", msg, localID, userID);
		send(sockClient, (char*)&sendMsg, PROTOCOL_SIZE, 0);
		msgRecv = 0;
		for (int i = 0; i < 30; i++)
		{
			Sleep(100);
			if (msgRecv)
				break;
		}
		simpleDecoding((uint16_t*)&sendMsg, PROTOCOL_SIZE);
		addMsg(userID, &sendMsg, msgRecv);
	}
	return;
}

void refreshChatPage()
{
	CONSOLE_SCREEN_BUFFER_INFO csbi_temp;
	GetConsoleScreenBufferInfo(hConsole, &csbi_temp);
	COORD coord = { csbi_temp.dwCursorPosition.X, csbi_temp.dwCursorPosition.Y };
	clearUpper();
	SetConsoleCursorPosition(hConsole, outputPos);
	char chattingName[NAMELEN];
	if (chattingID != 0)
	{
		getUserName(chattingID, chattingName);
	}
	else
	{
		memcpy_s(chattingName, NAMELEN, "ALL USER", sizeof("ALL USER"));
	}

	printf("User Name: %s\n", chattingName);
	printMsgBuffer(chattingID);
	SetConsoleCursorPosition(hConsole, coord);
	return;
}