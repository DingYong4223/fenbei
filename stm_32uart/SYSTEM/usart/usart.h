#ifndef __USART_H
#define __USART_H

#include "stdio.h"	
#include "sys.h" 

#define RATE_BOUND         115200     //bound rate
#define EN_USART1_RX 			 1		    //使能（1）/禁止（0）串口1接收
#define BIT_AT_END         0x0002   //at command end  

extern u16 USART_RX_STA;	
void uart_init(u32 bound);
void uart2_init(u32 bound);
void uart3_init(u32 bound);
//void UART1_Send(uint8_t* bytes, uint16_t len);
//void UART1_send_str(char* s);

void UART_Send(USART_TypeDef* USART, uint8_t* bytes, uint16_t len);
void UART_Send3(USART_TypeDef* USART, uint8_t* bytes, uint16_t len);
void UART_send_str(USART_TypeDef* USART, char* s);

#endif


