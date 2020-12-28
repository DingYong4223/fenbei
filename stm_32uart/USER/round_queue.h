#ifndef __ROUND_QUEUE_H
#define __ROUND_QUEUE_H

#include "sys.h" 

/**
* Round queue for UAST data. Any question about this, please contact me for discuss.
* author delanding
* Last Modify: 2020.11.07
**/

//max receive number 200
#define USART_REC_LEN  		2000
typedef struct {
	uint16_t read_index;
	uint16_t write_index;
	uint8_t isfull;
	uint8_t buffer[USART_REC_LEN];
} rcv_buffer_t;

int16_t write2buffer(uint8_t data);
void resetBuffer(void);
uint16_t getBufferLen(void);
uint8_t readByte(void);
uint16_t tryReadShort(void);
uint16_t tryReadShortOffset(int);
void moveReaderPos(uint16_t);
uint64_t readLong(void);
uint16_t readShort(void);
uint16_t readFromBuffer(uint8_t*, uint16_t);
uint16_t readAtFromBuffer(uint8_t *at_buf, uint16_t length);

#endif
