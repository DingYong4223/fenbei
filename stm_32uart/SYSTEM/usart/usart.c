#include "sys.h"
#include "usart.h"
#include "round_queue.h"
#include "check_buffer.h"
  
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

#if EN_USART1_RX   //���ʹ���˽���
//����1�жϷ������
//ע��,��ȡUSARTx->SR�ܱ���Ī������Ĵ���
//����״̬
//bit15��	������ɱ�־
//bit14��	���յ�0x0d
//bit13~0��	���յ�����Ч�ֽ���Ŀ

extern rcv_buffer_t rcv_buf;
u16 USART_RX_STA=0;       //receive state
uint8_t byte;
uint16_t TCP_CMD_TO_SEND;
  
void uart_init(u32 bound){
  //GPIO�˿�����
  GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1|RCC_APB2Periph_GPIOA, ENABLE);	//ʹ��USART1��GPIOAʱ��
  
	//USART1_TX   GPIOA.9
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9; //PA.9
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//�����������
  GPIO_Init(GPIOA, &GPIO_InitStructure);//��ʼ��GPIOA.9
   
  //USART1_RX	  GPIOA.10��ʼ��
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;//PA10
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//��������
  GPIO_Init(GPIOA, &GPIO_InitStructure);//��ʼ��GPIOA.10  

  //Usart1 NVIC ����
  NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3 ;//��ռ���ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		//�����ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);	//����ָ���Ĳ�����ʼ��VIC�Ĵ���
  
   //USART ��ʼ������

	USART_InitStructure.USART_BaudRate = bound;//���ڲ�����
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//�ֳ�Ϊ8λ���ݸ�ʽ
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//һ��ֹͣλ
	USART_InitStructure.USART_Parity = USART_Parity_No;//����żУ��λ
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//��Ӳ������������
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//�շ�ģʽ

  USART_Init(USART1, &USART_InitStructure); //��ʼ������1
  USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);//�������ڽ����ж�
  USART_Cmd(USART1, ENABLE);                    //ʹ�ܴ���1
}

void uart2_init( u32 bound ){
	/*
	  GPIO_InitTypeDef GPIO_InitStrue;
		USART_InitTypeDef USART_InitStrue;
		NVIC_InitTypeDef NVIC_InitStrue;
		
		// ����ʹ��ʱ��
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2,ENABLE);
		USART_DeInit(USART2);  //��λ����2 -> ����û��
		
		// ��ʼ�� ���ڶ�ӦIO��  TX-PA2  RX-PA3
		GPIO_InitStrue.GPIO_Mode=GPIO_Mode_AF_PP;
		GPIO_InitStrue.GPIO_Pin=GPIO_Pin_2;
		GPIO_InitStrue.GPIO_Speed=GPIO_Speed_50MHz;
		GPIO_Init(GPIOA,&GPIO_InitStrue);
		
		GPIO_InitStrue.GPIO_Mode=GPIO_Mode_IN_FLOATING;
		GPIO_InitStrue.GPIO_Pin=GPIO_Pin_3;
	  GPIO_Init(GPIOA,&GPIO_InitStrue);
		
		// ��ʼ�� ����ģʽ״̬
		USART_InitStrue.USART_BaudRate=bound;//My_BaudRate; // ������
		USART_InitStrue.USART_HardwareFlowControl=USART_HardwareFlowControl_None; // Ӳ��������
		USART_InitStrue.USART_Mode=USART_Mode_Tx|USART_Mode_Rx; // ���� ���� ģʽ��ʹ��
		USART_InitStrue.USART_Parity=USART_Parity_No; // û����żУ��
		USART_InitStrue.USART_StopBits=USART_StopBits_1; // һλֹͣλ
		USART_InitStrue.USART_WordLength=USART_WordLength_8b; // ÿ�η������ݿ��Ϊ8λ
		USART_Init(USART2,&USART_InitStrue);
	
		// ��ʼ�� �ж����ȼ�
		NVIC_InitStrue.NVIC_IRQChannel=USART2_IRQn;
		NVIC_InitStrue.NVIC_IRQChannelCmd=ENABLE;
		NVIC_InitStrue.NVIC_IRQChannelPreemptionPriority=1;
		NVIC_InitStrue.NVIC_IRQChannelSubPriority=1;
		NVIC_Init(&NVIC_InitStrue);
		
		USART_Cmd(USART2,ENABLE);//ʹ�ܴ���
		USART_ITConfig(USART2,USART_IT_RXNE,ENABLE);//���������ж�
	*/
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

void uart3_init(u32 bound)
{
  //GPIO�˿�����
  GPIO_InitTypeDef GPIO_InitStructure;
  USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
   RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
	 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);	//ʹ��USART3��GPIOBʱ��
  
	//USART3_TX   GPIOB10.
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10; //PB.10
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//�����������
  GPIO_Init(GPIOB, &GPIO_InitStructure);//��ʼ��PB10
   
	  //USART13_RX	  GPIOB.11��ʼ��
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;//PB11
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//��������
	GPIO_Init(GPIOB, &GPIO_InitStructure);//��ʼ��PB11 
 
  //Usart3 NVIC ����
	NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0 ;//��ռ���ȼ�0
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		//�����ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);	//����ָ���Ĳ�����ʼ��VIC�Ĵ���
  
   //USART ��ʼ������
 
	USART_InitStructure.USART_BaudRate = bound;//���ڲ�����
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//�ֳ�Ϊ8λ���ݸ�ʽ
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//һ��ֹͣλ
	USART_InitStructure.USART_Parity = USART_Parity_No;//����żУ��λ
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//��Ӳ������������
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//�շ�ģʽ
 
  USART_Init(USART3, &USART_InitStructure); //��ʼ������3
  
  USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);//��������3�����ж�
	USART_Cmd(USART3, ENABLE);                    //ʹ�ܴ���3 
	  

}
void USART3_IRQHandler(void)           
 {              	//����3�жϷ������ 
	 u8 Res2;
		if(USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)  //mcu�������� �����ж�(���յ������ݱ�����0x0d 0x0a��β)
		{
			Res2 =USART_ReceiveData(USART3);	//mcu�������ݶ�ȡ���յ�������,һλһλ�Ľ��ܵģ�res2ָ����һλ�������S����ʾ��������Ϣ����ʼλ
			bufferPush(Res2);                  //�յ����ݴ��뻺����	
			//USART_SendData(USART1, Res2);      //for����
		}   		 
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

void UART_Putc3(USART_TypeDef* USART, unsigned char c)
{
	USART->DR = (u8)c;
	while((USART->SR & 0X40) == 0);
	
	USART_SendData(USART1, c);      //for����
}

//send bytes to uart
void UART_Send3(USART_TypeDef* USART, uint8_t* bytes, uint16_t len)
{
	uint16_t t;
	for(t=0; t < len; t++) {
		UART_Putc3(USART, bytes[t]);
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
	/*u8 res;
		if(USART_GetITStatus(USART2,USART_IT_RXNE)!= RESET) // �жϱ�־
   {
	     res= USART_ReceiveData(USART2);  // ����2 ����
	     USART_SendData(USART1,res);      // ����1 ����
	  }*/
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
