#include "func.h"
#include "usart.h"

uint16_t readShortOff(uint8_t* b, uint16_t offset){
	uint16_t ret = (uint16_t)(b[offset + 1] & 0xFF) | (uint16_t)(b[offset + 0] & 0xFF) << 8;
	return ret;
}

uint32_t readIntOff(uint8_t* b, uint16_t offset) {
	uint32_t ret = (uint32_t)(b[offset + 3] & 0xFF) | (uint32_t)(b[offset + 2] & 0xFF) << 8 | (uint32_t)(b[offset + 1] & 0xFF) << 16 | (uint32_t)(b[offset + 0] & 0xFF) << 24;
	return ret;
}

char *del_char(char *str, char ch){
	unsigned char i=0,j=0;
	while(str[i] != '\0'){
		if(str[i] != ch){
			str[j++] = str[i];
		}
		i++;
	}
	str[j] = '\0';
	return str;
}
