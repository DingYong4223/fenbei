#ifndef __FUNC_H
#define __FUNC_H

#include "sys.h" 
#include "config.h"

#define LOG_NOOP (void) 0
//#define logi(fmt, ...) printf("[%s:%u] %s: " # fmt, __FILE__, __LINE__, __PRETTY_FUNCTION__, ##__VA_ARGS__); \

#if _DEBUG == 1
#define logi(fmt, ...) printf(fmt, ##__VA_ARGS__); \
printf("\r\n")
#else
#define logi(fmt, ...) LOG_NOOP
#endif

//#if _DEBUG == 1
//#define logAt(at_buffer, at_len) UART_Send(USART_LOG, at_buffer, at_len);
//#else
//#define logAt(fmt, ...) LOG_NOOP
//#endif

uint16_t readShortOff(uint8_t* data, uint16_t offset);
uint32_t readIntOff(uint8_t* data, uint16_t offset);
char *del_char(char *str, char ch);

#endif
