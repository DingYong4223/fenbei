#include "round_queue.h"
#include <stdlib.h>
#include <string.h>

rcv_buffer_t rcv_buf;

void resetBuffer() {
	rcv_buf.read_index = rcv_buf.write_index = 0;
	rcv_buf.isfull=0;
}

uint16_t getBufferLen() {
	int16_t ret;
	if(rcv_buf.isfull) {
		return USART_REC_LEN;
	}
	ret = rcv_buf.write_index - rcv_buf.read_index;
	if(ret >= 0) {
		return ret;
	}
	return ret + USART_REC_LEN;
}

/**skip from buffer*/
void moveReaderPos(uint16_t length) {
	if(length <= 0 || getBufferLen() == 0) {
		return;
	}
	rcv_buf.read_index = (rcv_buf.read_index + length) % USART_REC_LEN;
}

int16_t write2buffer(uint8_t data) {
	if(rcv_buf.isfull) {
		return 0;
	}
	rcv_buf.buffer[rcv_buf.write_index++] = data;
	rcv_buf.write_index %= USART_REC_LEN;
	rcv_buf.isfull = rcv_buf.write_index == rcv_buf.read_index ? 1 : 0;
	return 1;
}

/**
* just try read, not move the position.
*/
uint16_t tryReadFromBuffer(uint8_t *buf, uint16_t length) {
	uint16_t read_len = length;
	if(length == 0 || getBufferLen() < length) {
		return 0;
	}
	/*if(getBufferLen() >= length) {
		read_len = length;
	} else {
		read_len = getBufferLen();
	}*/
	if(rcv_buf.read_index + read_len < USART_REC_LEN) {
		memcpy(buf, rcv_buf.buffer + rcv_buf.read_index, read_len);
		//rcv_buf.read_index += read_len;	
	} else {
		uint8_t rlft = USART_REC_LEN - rcv_buf.read_index; //left length
		memcpy(buf, rcv_buf.buffer + rcv_buf.read_index, rlft);
		memcpy(buf + rlft, rcv_buf.buffer, read_len - rlft);
		//rcv_buf.read_index = (rcv_buf.read_index + read_len) % USART_REC_LEN;
	}
	//rcv_buf.isfull = 0;
	return read_len;
}

uint16_t tryReadFromBufferOffset(uint8_t *buf, uint16_t length, uint16_t offset) {
	uint16_t read;
	uint16_t befor_read_index = rcv_buf.read_index;
	moveReaderPos(offset);
	read = tryReadFromBuffer(buf, length);
	rcv_buf.read_index = befor_read_index;
	return read;
}

/***read from buffer backwards offset*/
uint16_t tryReadFromBufferBackOffset(uint8_t *buf, uint16_t length) {
	uint16_t read, befor_read_index;
	uint16_t buffer_length = getBufferLen();
	if(buffer_length < length) {
		return 0;
	}
	befor_read_index = rcv_buf.read_index;
	moveReaderPos(buffer_length - length);
	read = tryReadFromBuffer(buf, length);
	rcv_buf.read_index = befor_read_index;
	return read;
}

/**get the last 1 or 2*/
uint8_t tryLastIndexOf1Or2(uint16_t index) {
	uint8_t ret;
	uint8_t buf[2];
	if(tryReadFromBufferBackOffset(buf, 2) == 2) {
		ret = index == 1 ? buf[1] : buf[0];
	}
	return ret;
}

uint16_t readAtFromBuffer(uint8_t *at_buf, uint16_t length) {
	uint16_t read_len = readFromBuffer(at_buf, length);
	at_buf[length] = 0;
	return read_len;
}

uint16_t readFromBuffer(uint8_t *buf, uint16_t length) {
	uint16_t read_len = tryReadFromBuffer(buf, length);
	if(read_len > 0) {
		moveReaderPos(read_len);
		rcv_buf.isfull = 0;
	}
	return read_len;
}

uint16_t tryReadShort() {
	int16_t ret = 0;
	if(getBufferLen() >= 2) {
		uint8_t buf[2];
		tryReadFromBuffer(buf, 2);
		ret = ((buf[0] & 0xFF) << 8) | (buf[1] & 0xFF);
	}
	return ret;
}

uint16_t tryReadShortOffset(int offset) {
	uint16_t ret = 0;
	if(getBufferLen() >= offset + 2) {
		uint16_t befor_read_index = rcv_buf.read_index;
		moveReaderPos(offset);
		ret = tryReadShort();
		rcv_buf.read_index = befor_read_index;
	}
	return ret;
}

uint16_t readShort() {
	int16_t ret = tryReadShort();
	moveReaderPos(2);
	return ret;
}

uint8_t readByte() {
	uint8_t ret;
	if(getBufferLen() <= 0) {
		return 0;
	}
	ret = rcv_buf.buffer[rcv_buf.read_index];
	rcv_buf.read_index = ++rcv_buf.read_index % USART_REC_LEN;
	return ret;
}

uint64_t tryReadLong() {
	uint64_t ret = 0;
	if(getBufferLen() >= 8) {
		uint8_t b[8];
		tryReadFromBuffer(b, 8);
		/*for (count = 0; count < 8; ++count) {
			int shift = (7 - count) << 3;
			ret |= ((long) 0xff << shift) & ((long) b[count] << shift);
		}*/
		ret = (uint64_t)(b[7] & 0xFF) | (uint64_t)(b[6] & 0xFF) << 8 | (uint64_t)(b[5] & 0xFF) << 16 | (uint64_t)(b[4] & 0xFF) << 24
        | (uint64_t)(b[3] & 0xFF) << 32 | (uint64_t)(b[2] & 0xFF) << 40 | (uint64_t)(b[1] & 0xFF) << 48 | (uint64_t)(b[0] & 0xFF) << 56;
	}
	return ret;
}

uint64_t tryReadLongOffset(int offset) {
	uint64_t ret;
	if(getBufferLen() >= offset + 8) {
		uint16_t befor_read_index = rcv_buf.read_index;
		moveReaderPos(offset);
		ret = tryReadLong();
		rcv_buf.read_index = befor_read_index;
	}
	return ret;
}

uint64_t readLong() {
	uint64_t ret = tryReadLong();
	moveReaderPos(8);
	return ret;
}
