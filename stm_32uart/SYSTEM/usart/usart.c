#include "sys.h"
#include "usart.h"
#include "round_queue.h"
  
#if 1
#pragma import(__use_no_semihosting)                 
struct __FILE {
	int handle; 
}; 

FILE __stdout;
_sys_exit(int x) { 
	x = x; 
}

//__use_no_semihosting was requested, but _ttywrch was 
_ttywrch(int ch) {
  ch = ch;
}

int fputc(int ch, FILE *f) {
	while((USART1->SR&0X40)==0);
    USART1->DR = (u8) ch;      
	return ch;
}
#endif 

#if EN_USART1_RX   //如果使能了接收
//串口1中断服务程序
//注意,读取USARTx->SR能避免莫名其妙的错误
//接收状态
//bit15，	接收完成标志
//bit14，	接收到0x0d
//bit13~0，	接收到的有效字节数目

extern rcv_buffer_t rcv_buf;
u16 USART_RX_STA=0;       //receive state
uint8_t byte;
uint16_t TCP_CMD_TO_SEND;
  
void uart_init(u32 bound){
  //GPIO端口设置
  GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1|RCC_APB2Periph_GPIOA, ENABLE);	//使能USART1，GPIOA时钟
  
	//USART1_TX   GPIOA.9
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9; //PA.9
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//复用推挽输出
  GPIO_Init(GPIOA, &GPIO_InitStructure);//初始化GPIOA.9
   
  //USART1_RX	  GPIOA.10初始化
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;//PA10
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//浮空输入
  GPIO_Init(GPIOA, &GPIO_InitStructure);//初始化GPIOA.10  

  //Usart1 NVIC 配置
  NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3 ;//抢占优先级3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		//子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器
  
   //USART 初始化设置

	USART_InitStructure.USART_BaudRate = bound;//串口波特率
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式

  USART_Init(USART1, &USART_InitStructure); //初始化串口1
  USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);//开启串口接受中断
  USART_Cmd(USART1, ENABLE);                    //使能串口1
}

void uart2_init( u32 bound ){
    GPIO_InitTypeDef GPIO_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    // Enable the USART2 Pins Software Remapping
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_AFIO, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2|RCC_APB2Periph_AFIO, ENABLE);
    
    // Configure USART2 Rx (PA.03) as input floating
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;    
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    // Configure USART2 Tx (PA.02) as alternate function push-pull
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);    
    
    USART_InitStructure.USART_BaudRate = bound;                
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;     
    USART_InitStructure.USART_Parity = USART_Parity_No;        
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;      
    
    USART_Init(USART2, &USART_InitStructure);
    USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
    USART_Cmd(USART2, ENABLE);
}

void UART_Putc(USART_TypeDef* USART, unsigned char c){
	USART->DR = (u8)c;
	while((USART->SR & 0X40) == 0);
}

void UART_Send_Enter(USART_TypeDef* USART){
	UART_Putc(USART, 0x0d);
	UART_Putc(USART, 0x0a);
}

//send bytes to uart
void UART_Send(USART_TypeDef* USART, uint8_t* bytes, uint16_t len){
	uint16_t t;
	for(t=0; t < len; t++) {
		UART_Putc(USART, bytes[t]);
	}
}

//send a string, which end with \n.
void UART_send_str(USART_TypeDef* USART, char* s){
	USART_ClearFlag(USART, USART_FLAG_TC);
	for(;*s;s++){
		if(*s=='\n'){
			UART_Send_Enter(USART);
			return;
		}else{
			UART_Putc(USART, *s);
		}
	}
}

void USART1_IRQHandler(void) {}

void USART2_IRQHandler(void) {
	if(USART_GetFlagStatus(USART2, USART_FLAG_RXNE) == SET){
		USART_ClearITPendingBit(USART2, USART_IT_TXE);
		if(rcv_buf.isfull == 1){ //quene full, can't write into
			USART_RX_STA = USART_RX_STA | 0x0001;
			rcv_buf.read_index = 0;
			rcv_buf.write_index = 0;
			rcv_buf.isfull = 0;
		}
		write2buffer(byte = USART_ReceiveData(USART2));
		//detect 0x0d, 0x0a end
		if((USART_RX_STA & BIT_AT_END) == 0){
			if((USART_RX_STA & 0x0800) != 0){
				if(byte == 0x0a){
					USART_RX_STA |= BIT_AT_END;
				} else {
					USART_RX_STA &= ~0x0800;
				}
			} else if(byte == 0x0d){
				USART_RX_STA |= 0x0800;
			}
	  }
	}
}

#endif
