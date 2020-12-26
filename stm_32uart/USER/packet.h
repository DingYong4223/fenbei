#ifndef __PACKET_H
#define __PACKET_H

#include "sys.h" 

#define HEAD_LENGTH       14
#define HEAD_MAGIC        0x6473
#define DEF_VERSION       0x0001

#define BIT_QUNUE_FULL     0x0001     //qunue full
#define BIT_TCP_PACKET     0x0004     //tcp packet received

typedef struct {
	uint16_t magic;
	uint16_t version;
	uint16_t bodyLength;
	uint64_t msgID;
	uint16_t cmd;
	uint8_t* data;
} Packet;

void freePacket(Packet*);
uint16_t getPacketLength(Packet*);
void getPacketBytes(Packet*, uint8_t*);
void newPacketSendNullData(uint16_t cmd);
void newPacketSend(uint16_t cmd, const char* data);
void Send_Packet_And_Free(void);
void try2ParseTcpPacket(void);

#endif
