#include "packet.h"
#include "round_queue.h"
#include <stdlib.h>
#include "usart.h"
#include <string.h>
#include "config.h"
#include "delay.h"

extern rcv_buffer_t rcv_buf;
extern u16 USART_RX_STA;
Packet packetRecv;  //packet received!
Packet packetSend;  //packet to send!

void buildPacket(Packet* packet, uint16_t bodyLength, uint64_t msgId, uint16_t cmd, uint8_t* data){
	if(packet->magic != HEAD_MAGIC) {
		packet->magic=HEAD_MAGIC;
	}
	if(packet->version != DEF_VERSION) {
		packet->version=DEF_VERSION;
	}
	packet->bodyLength=bodyLength;
	packet->msgID=msgId;
	packet->cmd=cmd;
	if(data != NULL) {
		if(packet->data != NULL) {
			free(packet->data);
		}
		packet->data=malloc(bodyLength-2);
		memcpy(packet->data, data, bodyLength-2);
	}
}

void newPacketSendNullData(uint16_t cmd){
	buildPacket(&packetSend, 2, rand(), cmd, NULL);
}

void newPacketSend(uint16_t cmd, const char* data){
	buildPacket(&packetSend, 2 + strlen(data), rand(), cmd, (uint8_t *)data);
}

uint16_t getPacketLength(Packet* packet){
	return HEAD_LENGTH + packet->bodyLength;
}

void getPacketBytes(Packet* packet, uint8_t* bytes){
	uint16_t index;
	bytes[0] = (packet->magic >> 8) & 0xFF;
	bytes[1] = packet->magic & 0xFF;
	
	bytes[2] = (packet->version >> 8) & 0xFF;
	bytes[3] = packet->version & 0xFF;
	
	bytes[4] = (packet->bodyLength >> 8) & 0xFF;
	bytes[5] = packet->bodyLength & 0xFF;
	
	bytes[6] = (packet->msgID >> 56) & 0xFF;
	bytes[7] = (packet->msgID >> 48) & 0xFF;
	bytes[8] = (packet->msgID >> 40) & 0xFF;
	bytes[9] = (packet->msgID >> 32) & 0xFF;
	bytes[10] = (packet->msgID >> 24) & 0xFF;
	bytes[11] = (packet->msgID >> 16) & 0xFF;
	bytes[12] = (packet->msgID >> 8) & 0xFF;
	bytes[13] = packet->msgID & 0xFF;
	
	bytes[14] = (packet->cmd >> 8) & 0xFF;
	bytes[15] = packet->cmd & 0xFF;
	
	for(index = 0; index < packet->bodyLength - 2; index++) {
		bytes[index + 16] = packet->data[index];
	}
}

void freePacket(Packet* packet){
	packet->cmd=0;
	if(packet->data != NULL) {
		free(packet->data);
		packet->data=NULL;
	}
}

void Send_Packet_And_Free(){
	uint8_t* bytes;
	uint16_t len;
	len = getPacketLength(&packetSend);
	bytes = malloc(len);
	getPacketBytes(&packetSend, bytes);
	UART_Send(USART_8266, bytes, len);
	free(bytes);
	freePacket(&packetSend);
}

void try2ParseTcpPacket() {
	u16 body_length, cmd, buf_len, i;
	uint64_t msgID;
	/***********TCP Package*************/
	if((USART_RX_STA & BIT_TCP_PACKET) == 0) {
		buf_len = getBufferLen();
		if(buf_len<6) {
			return;
		}
		for(i=0; i<buf_len-2; i++){
			if(tryReadShortOffset(i) == HEAD_MAGIC 
						&& tryReadShortOffset(i + 2) == DEF_VERSION) {
					break;
			}
		}
		if(i>=buf_len-2) {
			return;
		}
		moveReaderPos(i+4);
		body_length = readShort();
		while(getBufferLen() < body_length){
			delay_ms(10);
			if(++i >= 100) {
				resetBuffer(); //tcp frame drop, reset
				return;//time out
			}
		}
		//TCP server Package received.
		msgID=readLong();
		cmd=readShort();
		if(body_length - 2 > 0) {
			uint8_t *data = malloc(body_length - 2);
			readFromBuffer(data, body_length - 2);
			buildPacket(&packetRecv, body_length, msgID, cmd, data);
			free(data);
		} else {
			buildPacket(&packetRecv, body_length, msgID, cmd, NULL);
		}
		resetBuffer(); //tcp frame drop, reset
		USART_RX_STA |= BIT_TCP_PACKET;
	}
}
