#pragma once
#include <time.h>
#include <string.h>
#include <cstdint>

#define MSGTYPE_SIZE 8
#define MESSAGE_SIZE 128
#define KEY 0x1234

struct _tagChatProtocol
{
	char msgtype[MSGTYPE_SIZE];
	char message[MESSAGE_SIZE];
	int sender_id;
	int receiver_id;
	time_t time;
	uint16_t checksum;
};

#define PROTOCOL_SIZE sizeof(_tagChatProtocol)

struct _tagChatProtocol createChatProtocol(const char* type, const char* message, int sender_id, int receiver_id);
int getChatProtocolSize();
void getMessage(struct _tagChatProtocol *tag, char* buffer);
int getSenderID(struct _tagChatProtocol* tag);
int getReceiverID(struct _tagChatProtocol* tag);
time_t getTime(struct _tagChatProtocol* tag);
uint16_t calculateChecksum(uint16_t* buf, int count);
void simpleEncoding(uint16_t* value, int length);
void simpleDecoding(uint16_t* value, int length);

struct _tagChatProtocol createChatProtocol(const char* type, const char* message, int sender_id, int receiver_id)
{
	struct _tagChatProtocol chatProtocol;
	memcpy_s(chatProtocol.msgtype, MSGTYPE_SIZE, type, MSGTYPE_SIZE);
	memcpy_s(chatProtocol.message, MESSAGE_SIZE, message, MESSAGE_SIZE);
	chatProtocol.sender_id = sender_id;
	chatProtocol.receiver_id = receiver_id;
	chatProtocol.time = time(NULL);
	chatProtocol.checksum = 0;
	chatProtocol.checksum = calculateChecksum((uint16_t*)&chatProtocol, PROTOCOL_SIZE);
	simpleEncoding((uint16_t*)&chatProtocol, PROTOCOL_SIZE);
	return chatProtocol;
}

int getChatProtocolSize()
{
	return sizeof(struct _tagChatProtocol);
}

void getMessage(struct _tagChatProtocol *tag, char* buffer)
{
	buffer = tag->message;
}

int getSenderID(struct _tagChatProtocol* tag)
{
	return tag->sender_id;
}

int getReceiverID(struct _tagChatProtocol* tag)
{
	return tag->receiver_id;
}

time_t getTime(struct _tagChatProtocol* tag)
{
	return tag->time;
}

uint16_t calculateChecksum(uint16_t* buf, int count = PROTOCOL_SIZE) {
	uint32_t sum = 0;
	count /= 2;
	while (count--) {
		sum += *buf++;
		if (sum & 0xFFFF0000) {
			sum &= 0xFFFF;
			sum++;
		}
	}
	return ~(sum & 0xFFFF);
}


void simpleEncoding(uint16_t* value, int length)
{
	uint16_t key = KEY;
	uint16_t* p = value;
	length /= sizeof(uint16_t);
	for (int i = 0; i < length; i++)
	{
		*p = *p ^ key;
		p++;
	}
	return;
}

void simpleDecoding(uint16_t* value, int length)
{
	uint16_t key = KEY;
	uint16_t* p = value;
	length /= sizeof(uint16_t);
	for (int i = 0; i < length; i++)
	{
		*p = *p ^ key;
		p++;
	}
	return;
}